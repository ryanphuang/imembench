#include <hirediscluster.h>

#include "imembench.h"
#include "benchconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <sys/stat.h>

const char * program_name;

INIConfigParser gParser;

RamCloudDriver gRamDriver("ramcloud");
TachyonDriver gTacDriver("tachyon");
RedisDriver gRedDriver("redis");

BenchDriverBase *gDrivers[] = {
//  &gRamDriver,
  &gTacDriver,
//    &gRedDriver,
};

size_t gNdriver = sizeof(gDrivers) / sizeof(gDrivers[0]);

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
  fprintf(stderr, "\n\tUsage: %s configfile\n\n", program_name);
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

int main(int argc, char ** argv)
{
  program_name = argv[0];
  if (argc != 2) {
    usage();
    exit(1);
  }
  bool ok;
  const char *configfile;
  configfile  = argv[1];
  struct stat st;
  if (stat(configfile, &st) != 0) {
    fprintf(stderr, "configuration file '%s' does not exist\n", configfile);
    exit(1);
  }
  ok = gParser.parse(configfile);
  if (!ok) {
    fprintf(stderr, "Error: invalid configuration file\n");
    exit(1);
  }
  // testConfigs(configfile);
  // setupRamCloudDriver();
  // setupTachyonDriver();
  setupRedisDriver();
  runBenchMarks();
  return 0;
}
