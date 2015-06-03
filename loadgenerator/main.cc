#include "imembench.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <sys/stat.h>

const char * program_name;

BenchConfigParser gParser;
RamCloudDriver gRamDriver;
TachyonDriver gTacDriver;
RedisDriver gRedDriver;

void testConfigs(const char *configfile)
{
  BenchConfigParser parser;
  parser.parse(configfile);
  BenchConfigSections sections;
  parser.getSections(sections);
  BenchConfigSections::iterator sit;
  for (sit = sections.begin(); sit != sections.end(); ++sit) {
    printf("section: %s\n", sit->c_str());
    BenchConfigMap configs = parser.getSectionConfigs(sit->c_str());
    BenchConfigMap::iterator cit;
    for (cit = configs.begin(); cit != configs.end(); ++cit) {
      printf("%s=%s\n", cit->first.c_str(), cit->second.c_str());
    }
  }
}

void usage() 
{
  fprintf(stderr, "\n\tUsage: %s configfile\n\n", program_name);
}

const char *getConfig(BenchConfigMap &configs, const char *section, 
      const char *key, bool noCheck=false, bool checkEmpty=true) 
{
  BenchConfigMap::iterator cit;
  cit = configs.find(key);
  if (cit == configs.end()) {
    if (!noCheck) {
      fprintf(stderr, "Error: no '%s' setting for %s\n", key, section);
    }
    return NULL;
  }
  if (cit->second.length() == 0) {
    if (!noCheck && checkEmpty) {
      fprintf(stderr, "Error: '%s' for %s has empty value\n", key, section);
      return NULL;
    }
  }
  return cit->second.c_str();
}

void setupRamCloudDriver()
{
  bool ok;
  const char *host = NULL;
  const char *sport = NULL;
  const char *transport = NULL;
  const char *clusterName = NULL;
  const char *testTable = NULL;
  int port; 

  BenchConfigMap rc_configs;
  rc_configs  = gParser.getSectionConfigs("ramcloud");
  if (rc_configs.size() == 0) {
    fprintf(stderr, "Error: empty ramcloud configurations\n");
    exit(1);
  }

  BenchConfigMap::iterator cit;
  cit = rc_configs.find("host");
  if (cit == rc_configs.end()) {
    fprintf(stderr, "Error: no 'host' setting for ramcloud\n");
    exit(1);
  }
  host = getConfig(rc_configs, "ramcloud", "host");
  if (host == NULL)
    exit(1);
  sport = getConfig(rc_configs, "ramcloud", "port");
  if (sport == NULL)
    exit(1);
  std::stringstream ss(sport);
  ss >> port;
  if (!ss) {
    fprintf(stderr, "Error: 'port'=%s is not an integer\n", sport);
    exit(1);
  }
  transport = getConfig(rc_configs, "ramcloud", "transport");
  if (transport == NULL)
    exit(1);

  clusterName = getConfig(rc_configs, "ramcloud", "cluster_name", true);
  testTable = getConfig(rc_configs, "ramcloud", "test_table", true);
  if (testTable == NULL)
    testTable = "test";
  ConnectionConfig * rcconf = new ConnectionConfig(host, port, transport, 
      clusterName, testTable);
  ok = gRamDriver.init(rcconf);
  if (!ok) {
    printf("fail to initialize ramcloud driver\n");
    exit(1);
  }
  printf("ramcloud driver for %s successfully initialized\n", rcconf->getLocator());
  // char buf[256];
  // rcdriver.write("fooo", "bar", 4); // also store the '\0' in value
  // rcdriver.read("fooo", buf, sizeof(buf));
  // printf("['%s']=%s\n", "fooo", buf);
}

void setupTachyonDriver()
{
  bool ok;
  const char *host = NULL;
  const char *sport = NULL;
  const char *kvstorePrefix = NULL;
  int port; 
  BenchConfigMap tc_configs;
  BenchConfigMap::iterator cit;

  tc_configs = gParser.getSectionConfigs("tachyon");
  if (tc_configs.size() == 0) {
    fprintf(stderr, "Error: empty tachyon configurations\n");
    exit(1);
  }

  host = getConfig(tc_configs, "tachyon", "host");
  if (host == NULL)
    exit(1);
  sport = getConfig(tc_configs, "tachyon", "port");
  if (sport == NULL)
    exit(1);
  std::stringstream ss(sport);
  ss >> port;
  if (!ss) {
    fprintf(stderr, "Error: 'port'=%s is not an integer\n", sport);
    exit(1);
  }
  kvstorePrefix = getConfig(tc_configs, "tachyon", "kv_store_prefix");

  ConnectionConfig * tcconf = new ConnectionConfig(host, port, NULL, NULL, 
      NULL, kvstorePrefix);
  ok = gTacDriver.init(tcconf);
  if (!ok) {
    printf("fail to initialize tachyon driver\n");
    exit(1);
  }
  printf("tachyon driver successfully initialized\n");

  char buf[256];
  gTacDriver.write("hello", "bar", 4); // also store the '\0' in value
  gTacDriver.read("hello", buf, sizeof(buf));
  printf("['%s']=%s\n", "hello", buf);
}

void setupRedisDriver()
{
  BenchConfigMap rd_configs;
  rd_configs = gParser.getSectionConfigs("redis");
  if (rd_configs.size() == 0) {
    fprintf(stderr, "Error: empty redis configurations\n");
    exit(1);
  }
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
  // testConfigs(configfile);
  ok = gParser.parse(configfile);
  if (!ok) {
    fprintf(stderr, "Error: invalid configuration file\n");
    exit(1);
  }
  // setupRamCloudDriver();
  setupTachyonDriver();
  return 0;
}
