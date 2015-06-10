#include <hirediscluster.h>

#include "imembench.h"
#include "benchconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <sstream>

const char * program_name;

INIConfigParser gParser;

const char * supported_targets [] = {
  "ramcloud",
  "tachyon",
  "redis",
  0,
};

RamCloudDriver gRamDriver("ramcloud");
TachyonDriver gTacDriver("tachyon");
RedisDriver gRedDriver("redis");

static struct option long_options[] = {
  {"help",            no_argument,         0,     'h'},
  {"list",            no_argument,         0,     'l'},
  {"config_file",     required_argument,   0,     'c'},
  {"workload_file",   required_argument,   0,     'w'},
  {"target",          required_argument,   0,     't'},
  {"benchmark",       required_argument,   0,     'b'},
  {0,  0,  0,  0},
};

void testConfigs(const char *configfile)
{
  printf("========================================================\n");
  printf("               test benchmark configurations            \n");
  printf("========================================================\n");
  INIConfigParser parser;
  parser.parse(configfile);
  INISections sections;
  parser.getSections(sections);
  INISections::iterator sit;
  for (sit = sections.begin(); sit != sections.end(); ++sit) {
    printf("section: %s\n", sit->c_str());
    INISectionConfigs* configs = parser.getSectionConfigs(sit->c_str());
    INISectionConfigs::iterator cit;
    for (cit = configs->begin(); cit != configs->end(); ++cit) {
      printf("%s=%s\n", cit->first.c_str(), cit->second.c_str());
    }
    printf("\n");
  }
  printf("--------------------------------------------------------\n");
  RAMCloudBenchConfig * rcconf = RAMCloudBenchConfig::fromINI(parser.getConfigs());
  if (rcconf == NULL) {
    fprintf(stderr, "Fail to parse RAMCloud configurations\n");
  } else {
    printf("RAMCloud configurations parsed\n");
  }
  TachyonBenchConfig * tcconf = TachyonBenchConfig::fromINI(parser.getConfigs());
  if (tcconf == NULL) {
    fprintf(stderr, "Fail to parse Tachyon configurations\n");
  } else {
    printf("Tachyon configurations parsed\n");
  }
  RedisBenchConfig *rdconf = RedisBenchConfig::fromINI(parser.getConfigs());
  if (rdconf == NULL) {
    fprintf(stderr, "Fail to parse Redis configurations\n");
  } else {
    printf("Redis configurations parsed\n");
  }
  printf("========================================================\n");
}

void usage() 
{
  printf("\nUsage: %s [OPTIONS] [tachyon|ramcloud|redis|all] \n\n", program_name);
  printf("  OPTION\n");
  printf("\t-l, --list                   list available benchmarks and supported targets.\n");
  printf("\t-c, --config_file            configuration file (default imembench.ini) for the benchmark\n");
  printf("\t-w, --workload_file          workload trace file to use instead of the default benchmark\n");
  printf("\t-b, --benchmark              comma-separated list of benchmarks to run on the target systems\n");
  printf("\t-t, --target                 comma-separated list of target system to evaluate\n");
  printf("\n");
  printf("  EXAMPLE\n");
  printf("\t%s -c imembench.ini --benchmark randomRW redis\n", program_name);
  printf("\t%s -c imembench.ini --workload_file trace.ycsb --benchmark ycsbReplay --target redis,ramcloud\n\n", program_name);
}

void setupRamCloudDriver()
{
  bool ok;
  RAMCloudBenchConfig * rcconf = RAMCloudBenchConfig::fromINI(gParser.getConfigs());
  ok = gRamDriver.init(rcconf);
  if (!ok) {
    printf("fail to initialize ramcloud driver\n");
    exit(1);
  }
  printf("ramcloud driver for %s successfully initialized\n", rcconf->getLocator());

  printf("========================================================\n");
  printf("               smoke test ramcloud                      \n");
  printf("========================================================\n");
  char buf[64];
  gRamDriver.write("fooo", 4, "bar", 4); // also store the '\0' in value
  gRamDriver.read("fooo", 4, buf, sizeof(buf));
  printf("[fooo]=%s\n", buf);
  printf("========================================================\n");
}

void setupTachyonDriver()
{
  bool ok;
  TachyonBenchConfig * tcconf = TachyonBenchConfig::fromINI(gParser.getConfigs());
  ok = gTacDriver.init(tcconf);
  if (!ok) {
    printf("fail to initialize tachyon driver\n");
    exit(1);
  }
  printf("tachyon driver successfully initialized\n");

  printf("========================================================\n");
  printf("               smoke test tachyon                       \n");
  printf("========================================================\n");
  char buf[32];
  gTacDriver.write("fooo", 4, "bar", 4); // also store the '\0' in value
  gTacDriver.read("fooo", 4, buf, sizeof(buf));
  printf("['fooo']=%s\n", buf);
  printf("========================================================\n");
}

void setupRedisDriver()
{
  bool ok;
  RedisBenchConfig *rdconf = RedisBenchConfig::fromINI(gParser.getConfigs());
  ok = gRedDriver.init(rdconf);
  if (!ok) {
    printf("fail to initialize redis driver\n");
    exit(1);
  }

  rediscluster::RedisCluster *client = (rediscluster::RedisCluster *) 
        gRedDriver.getClient();
  client->getClientForKey("hello", 5);
  client->getClientForKey("world", 5);
  client->getClientForKey("foo", 3);
  client->getClientForKey("bar", 3);
  client->getClientForKey("barrrr", 6);
  printf("redis driver successfully initialized\n");

  printf("========================================================\n");
  printf("               smoke test redis                         \n");
  printf("========================================================\n");
  char buf[32];
  gRedDriver.write("world", 5, "hello", 6); // also store the '\0' in value
  gRedDriver.read("world", 5, buf, sizeof(buf));
  printf("[world]=%s\n", buf);
  printf("========================================================\n");
}


void parse_list(const char *str, std::vector<std::string> & result)
{
  std::istringstream iss(str);
  std::string part;
  while (getline(iss, part, ',')) {
    result.push_back(part);
  }
}


int main(int argc, char ** argv)
{
  program_name = argv[0];
  const char *configFile = "imembench.ini";
  const char *workloadFile = NULL;
  const char *benchList = "all";
  const char *targetList = "all";
  std::vector<std::string> targetV;
  std::vector<std::string> benchV;
  std::vector<std::string>::iterator vit;

  int i, c;
  int option_index;
  while ((c = getopt_long(argc, argv, "c:w:b:t:hl", long_options, &option_index)) != -1) {
    switch (c) {
      case 'c': 
        configFile = optarg;
        break;
      case 'w':
        workloadFile = optarg;
        break;
      case 'b':
        benchList = optarg;
        break;
      case 't':
        targetList = optarg;
        break;
      case 'h':
        usage();
        exit(0);
        break;
      case 'l':
        printf("Supported targets:\n");
        i = 0;
        while (supported_targets[i] != NULL) {
          printf("%s\n", supported_targets[i]);
          i++;
        }
        printf("\nSupported benchmarks:\n");
        for (i = 0; i < gNbench; i++) {
          printf("%s\n", gBenchmarks[i].name);
        }
        exit(0);
      case '?':
      default:
        usage();
        exit(1);
    }
  }
  if (optind < argc) {
    targetList = argv[optind];
  }

  struct stat st;
  if (stat(configFile, &st) != 0) {
    fprintf(stderr, "configuration file '%s' does not exist\n", configFile);
    exit(1);
  }
  bool ok;
  ok = gParser.parse(configFile);
  if (!ok) {
    fprintf(stderr, "Error: invalid configuration file '%s'\n", configFile);
    exit(1);
  }
  testConfigs(configFile);
  parse_list(targetList, targetV);
  bool all = false, ramcloud = false, tachyon = false, redis = false;
  int targets = 0;
  for (vit = targetV.begin(); vit != targetV.end(); ++vit) {
    if (vit->compare("all") == 0) {
      all = true;
      targets = 3;
      break;
    }
    if (vit->compare("ramcloud") == 0) {
      if (!ramcloud) {
        targets++;
        ramcloud = true;
      }
    } else if (vit->compare("tachyon") == 0) {
      if (!tachyon) {
        targets++;
        tachyon = true;
      }
    } else if (vit->compare("redis") == 0) {
      if (!redis) {
        targets++;
        redis = true;
      }
    }
  }
  if (targets == 0) {
    printf("no targets specified for benchmarks, exit\n");
    exit(0);
  }
  printf("%d targets specified for benchmarks: %s\n", targets, targetList);

  BenchDriverBase **drivers = new BenchDriverBase *[targets];
  int ndriver = 0;
  if (all || ramcloud) {
    setupRamCloudDriver();
    drivers[ndriver++] = &gRamDriver;
  }
  if (all || tachyon) {
    setupTachyonDriver();
    drivers[ndriver++] = &gTacDriver;
  }
  if (all || redis) {
    setupRedisDriver();
    drivers[ndriver++] = &gRedDriver;
  }

  parse_list(benchList, benchV);
  const char **benchnames;
  int nbench;
  if (benchV.size() == 0) {
    printf("no benchmarks specified\n");
    exit(0);
  } 

  printf("%d benchmarks specified: %s\n", (int) benchV.size(), benchList);
  nbench = 0;
  benchnames = new const char *[benchV.size()];
  for (vit = benchV.begin(); vit != benchV.end(); ++vit) {
    benchnames[nbench++] = vit->c_str();
  }
  runBenchMarks(drivers, ndriver, benchnames, nbench, workloadFile);
  return 0;
}
