#include "imembench.h"
#include <Cycles.h>
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

void randomRW(BenchDriverBase *driver);
void readIntensive(BenchDriverBase *driver);
void writeOnly(BenchDriverBase *driver);
void getDist(std::vector<uint64_t>& times, TimeDist* dist);
std::string formatTime(double seconds);
void printBandwidth(const char* name, double bandwidth, const char* description);

BenchMark gBenchmarks[] = {
  {"randomRW", randomRW},
  {"readIntensive", readIntensive},
  {"writeOnly", writeOnly},
};

size_t gNbench = sizeof(gBenchmarks) / sizeof(gBenchmarks[0]);

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
readRandomObjects(BenchDriverBase *driver, uint32_t numObjects, uint16_t keyLength, 
          uint32_t maxValueLength, int count, double timeLimit)
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

        std::stringstream ss;
        int ri = RAMCloud::downCast<int>(generateRandom() % numObjects);
        ss << ri;
        std::string key = ss.str();
        int rd = driver->read(key.c_str(), (uint32_t) key.length(), value, maxValueLength);
        if (rd > 0)
          totalBytes = rd;
        uint64_t interval = RAMCloud::Cycles::rdtsc() - start;
        total += interval;
        times.at(i) = interval;
    }
    printf("read %lf\n", totalBytes);
    TimeDist result;
    getDist(times, &result);
    totalBytes *= RAMCloud::downCast<int>(times.size());
    result.bandwidth = (double) totalBytes/RAMCloud::Cycles::toSeconds(total);
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
writeRandomObjects(BenchDriverBase *driver, uint32_t numObjects, uint16_t keyLength,
          uint32_t valueLength, int count, double timeLimit)
{
    uint64_t total = 0;
    std::vector<uint64_t> times(count);
    char value[valueLength];
    memset(value, 'x', valueLength);
    uint64_t stopTime = RAMCloud::Cycles::rdtsc() + RAMCloud::Cycles::fromSeconds(timeLimit);

    for (int i = 0; i < count; i++) {
        int ri = RAMCloud::downCast<int>(generateRandom()%numObjects);
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

    return result;
}

// Modified from RAMCloud
//
void randomRW(BenchDriverBase *driver)
{
#define NUM_SIZES 5
    int sizes[] = {100, 1000, 10000, 100000, 1000000};
    TimeDist readDists[NUM_SIZES], writeDists[NUM_SIZES];
    const char* ids[] = {"100", "1K", "10K", "100K", "1M"};
    uint16_t keyLength = 30;
    char name[50], description[50];

    // Each iteration through the following loop measures random reads and
    // writes of a particular object size. Start with the largest object
    // size and work down to the smallest (this way, each iteration will
    // replace all of the objects created by the previous iteration).
    for (int i = NUM_SIZES-1; i >= 0; i--) {
        int size = sizes[i];

        // Generate roughly 500MB of data of the current size. The "20"
        // below accounts for additional overhead per object beyond the
        // key and value.
        uint32_t numObjects = 200000000/(size + keyLength + 20);
        fillData(driver, numObjects, keyLength, size);
        readDists[i] = readRandomObjects(driver, numObjects, keyLength,
                size, 10000, 10.0);
        writeDists[i] =  writeRandomObjects(driver, numObjects, keyLength,
                size, 10000, 10.0);
    }

    // Print out the results (in a different order):
    for (int i = 0; i < NUM_SIZES; i++) {
        TimeDist* dist = &readDists[i];
        snprintf(description, sizeof(description),
                "read random %sB object (%uB key)", ids[i], keyLength);
        snprintf(name, sizeof(name), "basic.read%s", ids[i]);
        printf("%-20s %s     %s median\n", name, formatTime(dist->p50).c_str(),
                description);
        snprintf(name, sizeof(name), "basic.read%s.min", ids[i]);
        printf("%-20s %s     %s minimum\n", name, formatTime(dist->min).c_str(),
                description);
        snprintf(name, sizeof(name), "basic.read%s.9", ids[i]);
        printf("%-20s %s     %s 90%%\n", name, formatTime(dist->p90).c_str(),
                description);
        if (dist->p99 != 0) {
            snprintf(name, sizeof(name), "basic.read%s.99", ids[i]);
            printf("%-20s %s     %s 99%%\n", name,
                    formatTime(dist->p99).c_str(), description);
        }
        if (dist->p999 != 0) {
            snprintf(name, sizeof(name), "basic.read%s.999", ids[i]);
            printf("%-20s %s     %s 99.9%%\n", name,
                    formatTime(dist->p999).c_str(), description);
        }
        snprintf(name, sizeof(name), "basic.readBw%s", ids[i]);
        snprintf(description, sizeof(description),
                "bandwidth reading %sB objects (%uB key)", ids[i], keyLength);
        printBandwidth(name, dist->bandwidth, description);
    }

    for (int i = 0; i < NUM_SIZES; i++) {
        TimeDist* dist = &writeDists[i];
        snprintf(description, sizeof(description),
                "write random %sB object (%uB key)", ids[i], keyLength);
        snprintf(name, sizeof(name), "basic.write%s", ids[i]);
        printf("%-20s %s     %s median\n", name, formatTime(dist->p50).c_str(),
                description);
        snprintf(name, sizeof(name), "basic.write%s.min", ids[i]);
        printf("%-20s %s     %s minimum\n", name, formatTime(dist->min).c_str(),
                description);
        snprintf(name, sizeof(name), "basic.write%s.9", ids[i]);
        printf("%-20s %s     %s 90%%\n", name, formatTime(dist->p90).c_str(),
                description);
        if (dist->p99 != 0) {
            snprintf(name, sizeof(name), "basic.write%s.99", ids[i]);
            printf("%-20s %s     %s 99%%\n", name,
                    formatTime(dist->p99).c_str(), description);
        }
        if (dist->p999 != 0) {
            snprintf(name, sizeof(name), "basic.write%s.999", ids[i]);
            printf("%-20s %s     %s 99.9%%\n", name,
                    formatTime(dist->p999).c_str(), description);
        }
        snprintf(name, sizeof(name), "basic.writeBw%s", ids[i]);
        snprintf(description, sizeof(description),
                "bandwidth writing %sB objects (%uB key)", ids[i], keyLength);
        printBandwidth(name, dist->bandwidth, description);
    }
}

void readIntensive(BenchDriverBase *driver)
{

}

void writeOnly(BenchDriverBase *driver)
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

void runBenchMarks()
{
  size_t i, j;
  for (i = 0; i < gNdriver; ++i) {
    BenchDriverBase *driver = gDrivers[i];
    for (j = 0; j < gNbench; ++j) {
      BenchMark bench = gBenchmarks[j];
      printf("=============================================\n");
      printf("running benchmark '%s' on '%s'\n", bench.name, driver->getName());
      bench.run(driver);
      printf("==========================================\n");
    }
  }
}
