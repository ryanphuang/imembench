#include "imembench.h"

#include <Cycles.h>
#include <hirediscluster.h>
#include <Common.h>

#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <vector>
#include <algorithm>
#include <sstream>


// From RamCloud src/ClusterPerf.c
//
// Used to return results about the distribution of times for a
// particular operation.
struct TimeDist {
    double min;                   // Fastest time seen, in seconds.
    double p50;                   // Median time per operation, in seconds.
    double p90;                   // 90th percentile time/op, in seconds.
    double p99;                   // 99th percentile time/op, in seconds.
    double p999;                  // 99.9th percentile time/op, in seconds.
    double p9999;                 // 99.99th percentile time/op, in seconds,
                                  // or 0 if no such measurement.
    double p99999;                // 99.999th percentile time/op, in seconds,
                                  // or 0 if no such measurement.
    double bandwidth;             // Average throughput in bytes/sec., or 0
                                  // if no such measurement.
};

void uniformRandomRW(BenchDriverBase *driver, const char *traceFile);
void zipfRandomRW(BenchDriverBase *driver, const char *traceFile);

void ycsbReplay(BenchDriverBase *driver, const char *traceFile);
void readIntensive(BenchDriverBase *driver, const char *traceFile);
void writeOnly(BenchDriverBase *driver, const char *traceFile);

void getDist(std::vector<uint64_t>& times, TimeDist* dist);
std::string formatTime(double seconds);
void printBandwidth(const char* name, double bandwidth, const char* description);

BenchMark gBenchmarks[] = {
  {"ycsbReplay", ycsbReplay},
  {"randomRW", uniformRandomRW},
  {"zipfRandomRW", zipfRandomRW},
  {"readIntensive", readIntensive},
  {"writeOnly", writeOnly},
};

int gNbench = 5;

bool YCSBTraceParser::nextLog(TraceLog *log)
{
  if (log == NULL) {
    return false;
  }
  std::string line;
  if (std::getline(m_ifs, line)) {
    const char *p = line.c_str();
    const char *start = p;
    while (*p != '\0' && *p != '|') p++;
    if (*p == '\0') {
      fprintf(stderr, "invalid trace log: %s\n", line.c_str());
      return false;
    }
    size_t max_len = p - start;
    if (strncmp(start, "UPDATE", max_len) == 0) {
      log->op = T_OP_UPDATE;
    } else if (strncmp(start, "GET", max_len) == 0) {
      log->op = T_OP_GET;
    } else if (strncmp(start, "DELETE", max_len) == 0) {
      log->op = T_OP_DELETE;
    } else {
      fprintf(stderr, "invalid operation in trace log: %s\n", line.c_str());
      return false;
    }
    ++p;
    start = p;
    while (*p != '\0' && *p != '|') p++;
    max_len = p - start;
    if (max_len >= MAX_TRACE_KEY) {
      max_len = MAX_TRACE_KEY - 1;
    }
    strncpy(log->key, start, max_len);
    log->key[max_len] = '\0'; // need to pad '\0'
    if (*p == '\0') {
      if (log->op == T_OP_UPDATE) {
        // update needs a value
        fprintf(stderr, "update operation in trace log missing value: %s\n", line.c_str());
        return false;
      }
      strcpy(log->value, ""); // should clear the value!
      return true;
    }
    ++p;
    strncpy(log->value, p, MAX_TRACE_VALUE);
    return true;
  } else {
    if (m_ifs.is_open()) {
      m_ifs.close();
    }
    return false;
  }
}

/**
 * Used to generate zipfian distributed random numbers where the distribution is
 * skewed toward the lower integers; e.g. 0 will be the most popular, 1 the next
 * most popular, etc.
 *
 * This class implements the core algorithm from YCSB's ZipfianGenerator; it, in
 * turn, uses the algorithm from "Quickly Generating Billion-Record Synthetic
 * Databases", Jim Gray et al, SIGMOD 1994.
 */
class ZipfianGenerator {
  public:
    /**
     * Construct a generator.  This may be expensive if n is large.
     *
     * \param n
     *      The generator will output random numbers between 0 and n-1.
     * \param theta
     *      The zipfian parameter where 0 < theta < 1 defines the skew; the
     *      smaller the value the more skewed the distribution will be. Default
     *      value of 0.99 comes from the YCSB default value.
     */
    explicit ZipfianGenerator(uint64_t n, double theta = 0.99)
        : n(n)
        , theta(theta)
        , alpha(1 / (1 - theta))
        , zetan(zeta(n, theta))
        , eta((1 - pow(2.0 / static_cast<double>(n), 1 - theta)) /
              (1 - zeta(2, theta) / zetan))
    {}

    /**
     * Return the zipfian distributed random number between 0 and n-1.
     */
    uint64_t nextNumber()
    {
        double u = static_cast<double>(generateRandom()) /
                   static_cast<double>(~0UL);
        double uz = u * zetan;
        if (uz < 1)
            return 0;
        if (uz < 1 + std::pow(0.5, theta))
            return 1;
        return 0 + static_cast<uint64_t>(static_cast<double>(n) *
                                         std::pow(eta*u - eta + 1.0, alpha));
    }

  private:
    const uint64_t n;       // Range of numbers to be generated.
    const double theta;     // Parameter of the zipfian distribution.
    const double alpha;     // Special intermediate result used for generation.
    const double zetan;     // Special intermediate result used for generation.
    const double eta;       // Special intermediate result used for generation.

    /**
     * Returns the nth harmonic number with parameter theta; e.g. H_{n,theta}.
     */
    static double zeta(uint64_t n, double theta)
    {
        double sum = 0;
        for (uint64_t i = 0; i < n; i++) {
            sum = sum + 1.0/(std::pow(i+1, theta));
        }
        return sum;
    }
};

void fillData(BenchDriverBase *driver, int numObjects, uint16_t keyLength, uint32_t valueLength)
{
    char value[valueLength];
    memset(value, 'x', valueLength);

    // Each iteration through the following loop adds one object to
    // the current multi-write, and invokes the multi-write if needed.
    uint64_t writeTime = 0;
    for (int i = 0; i < numObjects; i++) {
        std::stringstream  ss;
        ss << i;
        std::string key = ss.str();
        uint64_t start = RAMCloud::Cycles::rdtsc();
        driver->write(key.c_str(), (uint32_t) key.length(), &value[0], valueLength);
        writeTime += RAMCloud::Cycles::rdtsc() - start;
    }
    double rate = numObjects/RAMCloud::Cycles::toSeconds(writeTime);
    printf("total objects written = %d\n", numObjects);
    printf("total bytes written = %u\n", numObjects * (keyLength+valueLength));
    printf("write rate for %d-byte objects: %.1f kobjects/sec,"
          " %.1f MB/sec\n", valueLength, rate/1e03, rate*(keyLength+valueLength)/1e06);
}

// From RAMCloud, src/ClusterPerf.cc
/**
 * Read randomly-chosen objects from a single table and return information about
 * the distribution of read times.
 *
 * \param tableId
 *      Table containing the object.
 * \param numObjects
 *      Total number of objects in the table. Keys must have been generated
 *      by makeKey(i, keyLength, ...), where 0 <= i < numObjects.
 * \param keyLength
 *      Size in bytes of keys.
 * \param count
 *      Read the object this many times, unless time runs out.
 * \param timeLimit
 *      Maximum time (in seconds) to spend on this test: if this much
 *      time elapses, then fewer than count iterations will be run.
 *
 * \return
 *      Information about how long the reads took.
 */
TimeDist
readRandomObjects(BenchDriverBase *driver, ZipfianGenerator *generator, 
          uint32_t numObjects, uint16_t keyLength, uint32_t maxValueLength, 
          int count, double timeLimit)
{
    uint64_t total = 0;
    std::vector<uint64_t> times(count);
    char value[maxValueLength];
    uint64_t stopTime = RAMCloud::Cycles::rdtsc() + RAMCloud::Cycles::fromSeconds(timeLimit);
    double totalBytes = 0;
    for (int i = 0; i < count; i++) {
        uint64_t start = RAMCloud::Cycles::rdtsc();
        if (start >= stopTime) {
            fprintf(stderr, "time expired after %d iterations\n", i);
            times.resize(i);
            break;
        }

        int ri; 
        if (generator == NULL) {
          ri = RAMCloud::downCast<int>(generateRandom()%numObjects);
        } else {
          ri = RAMCloud::downCast<int>(generator->nextNumber());
        }
        std::stringstream ss;
        ss << ri;
        std::string key = ss.str();
        int rd = driver->read(key.c_str(), (uint32_t) key.length(), value, maxValueLength);
        if (rd > 0)
          totalBytes = rd;
        uint64_t interval = RAMCloud::Cycles::rdtsc() - start;
        total += interval;
        times.at(i) = interval;
    }
    printf("read = %lf\n", totalBytes);
    TimeDist result;
    getDist(times, &result);
    totalBytes *= RAMCloud::downCast<int>(times.size());
    result.bandwidth = (double) totalBytes/RAMCloud::Cycles::toSeconds(total);
    printf("total bytes = %lf\n", totalBytes);
    return result;
}

/**
 * Write randomly-chosen objects in a single table and return information about
 * the distribution of write times.
 *
 * \param tableId
 *      Table containing the object.
 * \param numObjects
 *      Total number of objects in the table. Keys must have been generated
 *      by makeKey(i, keyLength, ...), where 0 <= i < numObjects.
 * \param keyLength
 *      Size in bytes of each key.
 * \param valueLength
 *      Number of bytes of data to write into each object; the actual values
 *      will be randomly chosen.
 * \param count
 *      Write this many objects.
 * \param timeLimit
 *      Maximum time (in seconds) to spend on this test: if this much
 *      time elapses, then less than count iterations will be run.
 *
 * \return
 *      Information about how long the writes took.
 */
TimeDist
writeRandomObjects(BenchDriverBase *driver, ZipfianGenerator *generator,
          uint32_t numObjects, uint16_t keyLength,
          uint32_t valueLength, int count, double timeLimit)
{
    uint64_t total = 0;
    std::vector<uint64_t> times(count);
    char value[valueLength];
    memset(value, 'x', valueLength);
    uint64_t stopTime = RAMCloud::Cycles::rdtsc() + RAMCloud::Cycles::fromSeconds(timeLimit);

    for (int i = 0; i < count; i++) {
        int ri;
        if (generator == NULL) {
          ri = RAMCloud::downCast<int>(generateRandom()%numObjects);
        } else {
          ri = RAMCloud::downCast<int>(generator->nextNumber());
        }
        std::stringstream ss;
        ss << ri;
        std::string key = ss.str();
        uint64_t start = RAMCloud::Cycles::rdtsc();
        if (start >= stopTime) {
            fprintf(stderr, "time expired after %d iterations\n", i);
            times.resize(i);
            break;
        }
        driver->write(key.c_str(), (uint32_t) key.length(), value, valueLength);
        uint64_t interval = RAMCloud::Cycles::rdtsc() - start;
        total += interval;
        times.at(i) = interval;
    }
    TimeDist result;
    getDist(times, &result);
    double totalBytes = valueLength;
    totalBytes *= RAMCloud::downCast<int>(times.size());
    result.bandwidth = totalBytes/RAMCloud::Cycles::toSeconds(total);
    printf("write total bytes = %lf\n", totalBytes);
    return result;
}

void ycsbReplay(BenchDriverBase *driver, const char *traceFile)
{
  if (traceFile == NULL) {
    fprintf(stderr, "no trace file specified for ycsbReplay\n");
    return;
  }
  YCSBTraceParser parser;
  if (!parser.init(traceFile)) {
    fprintf(stderr, "fail to init trace file '%s' for ycsbReplay\n", traceFile);
    return;
  }
  struct TraceLog log;

  uint64_t total = 0;
  std::vector<uint64_t> times;
  double totalBytes = 0;
  size_t keyLen, valLen;
  while (parser.nextLog(&log)) {
    bool ok = true;
    uint64_t start, interval;
    switch (log.op) {
      case T_OP_GET:
        keyLen = strlen(log.key);
        start = RAMCloud::Cycles::rdtsc();
        driver->read(log.key, (uint32_t) keyLen, log.value, MAX_TRACE_VALUE);
        interval = RAMCloud::Cycles::rdtsc() - start;
        totalBytes += (uint32_t) strlen(log.value);
        // printf("GET %s = %s\n", log.key, log.value);
        break;
      case T_OP_UPDATE:
        keyLen = strlen(log.key);
        valLen = strlen(log.value);
        start = RAMCloud::Cycles::rdtsc();
        driver->write(log.key, (uint32_t) strlen(log.key), log.value, (uint32_t) valLen);
        interval = RAMCloud::Cycles::rdtsc() - start;
        totalBytes += (uint32_t) strlen(log.value);
        // printf("PUT %s = %s\n", log.key, log.value);
        break;
      case T_OP_DELETE:
        start = RAMCloud::Cycles::rdtsc();
        driver->write(log.key, (uint32_t) strlen(log.key), "", 0);
        interval = RAMCloud::Cycles::rdtsc() - start;
        // printf("DEL %s\n", log.key);
        break;
      default:
        fprintf(stderr, "UNKNOWN log\n");
        ok = false;
    }
    if (!ok) {
      continue;
    }
    total += interval;
    times.push_back(interval);
  }

  printf("total bytes = %lf\n", totalBytes);
  TimeDist dist;
  getDist(times, &dist);
  dist.bandwidth = totalBytes / RAMCloud::Cycles::toSeconds(total);

  const char *name = "YCSB";
  const char *description = "simple get/put workload";

  printf("%-20s %s     %s median\n", name, formatTime(dist.p50).c_str(),
          description);
  printf("%-20s %s     %s minimum\n", name, formatTime(dist.min).c_str(),
          description);
  printf("%-20s %s     %s 90%%\n", name, formatTime(dist.p90).c_str(),
          description);
  if (dist.p99 != 0) {
      printf("%-20s %s     %s 99%%\n", name,
              formatTime(dist.p99).c_str(), description);
  }
  if (dist.p999 != 0) {
      printf("%-20s %s     %s 99.9%%\n", name,
              formatTime(dist.p999).c_str(), description);
  }
  printBandwidth(name, dist.bandwidth, "bandwidth");
}

// Modified from RAMCloud
//
void _randomRW(BenchDriverBase *driver, const char *traceFile, bool zipfRandom, 
              uint32_t objSize, uint16_t keySize, uint32_t dataSize, 
              uint32_t operations, double readPercent, double writePercent, 
              TimeDist *readDist, TimeDist *writeDist)
{

    int readCount = (int) (operations * readPercent);
    int writeCount = (int) (operations * writePercent);

    // The "20" below accounts for additional overhead per object beyond the
    // key and value.
    uint32_t numObjects = dataSize / (objSize + keySize + 20);

    ZipfianGenerator *generator = NULL;
    if (zipfRandom) {
      generator = new ZipfianGenerator(numObjects);
    }
    fillData(driver, numObjects, keySize, objSize);
    *readDist = readRandomObjects(driver, generator, numObjects, keySize,
            objSize, readCount, 5.0);
    *writeDist =  writeRandomObjects(driver, generator, numObjects, keySize,
            objSize, writeCount, 5.0);
    if (generator != NULL) {
      delete generator;
    }
}

void printTimeDist(const char *operation, TimeDist dists[], const char *ids[], 
                  int n, int keyLength)
{
    char name[50], description[50];
    // Print out the results (in a different order):
    for (int i = 0; i < n; i++) {
        TimeDist* dist = &dists[i];
        snprintf(description, sizeof(description),
                "%s random %sB object (%uB key)", operation, ids[i], keyLength);
        snprintf(name, sizeof(name), "basic.%s%s", operation, ids[i]);
        printf("%-20s %s     %s median\n", name, formatTime(dist->p50).c_str(),
                description);
        snprintf(name, sizeof(name), "basic.%s%s.min", operation, ids[i]);
        printf("%-20s %s     %s minimum\n", name, formatTime(dist->min).c_str(),
                description);
        snprintf(name, sizeof(name), "basic.%s%s.9", operation, ids[i]);
        printf("%-20s %s     %s 90%%\n", name, formatTime(dist->p90).c_str(),
                description);
        if (dist->p99 != 0) {
            snprintf(name, sizeof(name), "basic.%s%s.99", operation, ids[i]);
            printf("%-20s %s     %s 99%%\n", name,
                    formatTime(dist->p99).c_str(), description);
        }
        if (dist->p999 != 0) {
            snprintf(name, sizeof(name), "basic.%s%s.999", operation, ids[i]);
            printf("%-20s %s     %s 99.9%%\n", name,
                    formatTime(dist->p999).c_str(), description);
        }
        snprintf(name, sizeof(name), "basic.%sBw%s", operation, ids[i]);
        snprintf(description, sizeof(description),
                "bandwidth %s %sB objects (%uB key)", operation, ids[i], keyLength);
        printBandwidth(name, dist->bandwidth, description);
    }
}

void uniformRandomRW(BenchDriverBase *driver, const char *traceFile)
{
    uint16_t keyLength = 30;
    uint32_t dataSize = 200000000;
    uint32_t operations = 10000;
    double readPercent = 0.5;
    double writePercent = 0.5;

#define NUM_SIZES 5
    int sizes[] = {100, 1000, 10000, 100000, 1000000};
    TimeDist readDists[NUM_SIZES], writeDists[NUM_SIZES];
    const char* ids[] = {"100", "1K", "10K", "100K", "1M"};

    // Each iteration through the following loop measures random reads and
    // writes of a particular object size. Start with the largest object
    // size and work down to the smallest (this way, each iteration will
    // replace all of the objects created by the previous iteration).
    for (int i = NUM_SIZES - 1; i >= 0; --i) {
      _randomRW(driver, traceFile, false, sizes[i], keyLength, dataSize,
                operations, readPercent, writePercent, 
                &readDists[i], &writeDists[i]);
    }
    printTimeDist("read", readDists, ids, NUM_SIZES, keyLength);
    printTimeDist("write", writeDists, ids, NUM_SIZES, keyLength);
}

void zipfRandomRW(BenchDriverBase *driver, const char *traceFile)
{
    uint16_t keyLength = 30;
    uint32_t dataSize = 200000000;
    uint32_t operations = 10000;
    double readPercent = 0.5;
    double writePercent = 0.5;

#define NUM_SIZES 5
    int sizes[] = {100, 1000, 10000, 100000, 1000000};
    TimeDist readDists[NUM_SIZES], writeDists[NUM_SIZES];
    const char* ids[] = {"100", "1K", "10K", "100K", "1M"};

    // Each iteration through the following loop measures random reads and
    // writes of a particular object size. Start with the largest object
    // size and work down to the smallest (this way, each iteration will
    // replace all of the objects created by the previous iteration).
    for (int i = NUM_SIZES - 1; i >= 0; --i) {
      _randomRW(driver, traceFile, true, sizes[i], keyLength, dataSize,
                operations, readPercent, writePercent, 
                &readDists[i], &writeDists[i]);
    }
    printTimeDist("read", readDists, ids, NUM_SIZES, keyLength);
    printTimeDist("write", writeDists, ids, NUM_SIZES, keyLength);
}

void readIntensive(BenchDriverBase *driver, const char *traceFile)
{

}

void writeOnly(BenchDriverBase *driver, const char *traceFile)
{


}

// From RAMCloud
/**
 * Given a vector of time values, sort it and return information
 * about various percentiles.
 *
 * \param times
 *      Interval lengths in Cycles::rdtsc units.
 * \param[out] dist
 *      The various percentile values in this structure will be
 *      filled in with times in seconds.
 */
void getDist(std::vector<uint64_t>& times, TimeDist* dist)
{
    int count = RAMCloud::downCast<int>(times.size());
    std::sort(times.begin(), times.end());
    dist->min = RAMCloud::Cycles::toSeconds(times[0]);
    int index = count/2;
    if (index < count) {
        dist->p50 = RAMCloud::Cycles::toSeconds(times.at(index));
    } else {
        dist->p50 = 0;
    }
    index = count - (count+5)/10;
    if (index < count) {
        dist->p90 = RAMCloud::Cycles::toSeconds(times.at(index));
    } else {
        dist->p90 = 0;
    }
    index = count - (count+50)/100;
    if (index < count) {
        dist->p99 = RAMCloud::Cycles::toSeconds(times.at(index));
    } else {
        dist->p99 = 0;
    }
    index = count - (count+500)/1000;
    if (index < count) {
        dist->p999 = RAMCloud::Cycles::toSeconds(times.at(index));
    } else {
        dist->p999 = 0;
    }
    index = count - (count+5000)/10000;
    if (index < count) {
        dist->p9999 = RAMCloud::Cycles::toSeconds(times.at(index));
    } else {
        dist->p9999 = 0;
    }
    index = count - (count+50000)/100000;
    if (index < count) {
        dist->p99999 = RAMCloud::Cycles::toSeconds(times.at(index));
    } else {
        dist->p99999 = 0;
    }
}

// From RAMCloud src/ClusterPerf.cc
/**
 * Given a time value, return a string representation of the value
 * with an appropriate scale factor.
 *
 * \param seconds
 *      Time value, in seconds.
 * \result
 *      A string corresponding to the time value, such as "4.2ms".
 *      Appropriate units will be chosen, ranging from nanoseconds to
 *      seconds.
 */
std::string formatTime(double seconds)
{
    if (seconds < 1.0e-06) {
        return RAMCloud::format("%5.1f ns", 1e09*seconds);
    } else if (seconds < 1.0e-03) {
        return RAMCloud::format("%5.1f us", 1e06*seconds);
    } else if (seconds < 1.0) {
        return RAMCloud::format("%5.1f ms", 1e03*seconds);
    } else {
        return RAMCloud::format("%5.1f s ", seconds);
    }
}

/**
 * Print a performance measurement consisting of a bandwidth.
 *
 * \param name
 *      Symbolic name for the measurement, in the form test.value
 *      where \c test is the name of the test that generated the result,
 *      \c value is a name for the particular measurement.
 * \param bandwidth
 *      Measurement in units of bytes/second.
 * \param description
 *      Longer string (but not more than 20-30 chars) with a human-
 *      readable explanation of what the value refers to.
 */
void
printBandwidth(const char* name, double bandwidth, const char* description)
{
    double gb = 1024.0*1024.0*1024.0;
    double mb = 1024.0*1024.0;
    double kb = 1024.0;
    printf("%-20s ", name);
    if (bandwidth > gb) {
        printf("%5.1f GB/s ", bandwidth/gb);
    } else if (bandwidth > mb) {
        printf("%5.1f MB/s ", bandwidth/mb);
    } else if (bandwidth >kb) {
        printf("%5.1f KB/s ", bandwidth/kb);
    } else {
        printf("%5.1f B/s  ", bandwidth);
    }
    printf("  %s\n", description);
}

void runBenchMarks(BenchDriverBase **drivers, int ndriver, 
      const char **benchnames, int nbench, const char *traceFile)
{
  int i, j, k;
  // test if bennames are valid first!
  for (j = 0; j < nbench; ++j) {
    for (k = 0; k < gNbench; ++k) {
      if (strcmp(gBenchmarks[k].name, benchnames[j]) == 0
          || strcmp("all", benchnames[j]) == 0) {
        break;
      }
    }
    if (k >= gNbench) {
      fprintf(stderr, "Error: cannot find benchmark '%s'\n", benchnames[j]);
      return;
    }
  }
  bool *benchrun = new bool[gNbench];
  for (i = 0; i < ndriver; ++i) {
    BenchDriverBase *driver = drivers[i];
    memset(benchrun, 0, gNbench * sizeof(bool));
    for (j = 0; j < nbench; ++j) {
      for (k = 0; k < gNbench; ++k) {
        if (strcmp(gBenchmarks[k].name, benchnames[j]) == 0
            || strcmp("all", benchnames[j]) == 0) {
          if (!benchrun[k]) {
            BenchMark *bench = &gBenchmarks[k];
            printf("========================================================\n");
            printf("running benchmark '%s' on '%s'\n", bench->name, driver->getName());
            bench->run(driver, traceFile);
            printf("========================================================\n");
            benchrun[k] = true;
          }
          break;
        }
      }
    }
  }
  delete []benchrun;
}
