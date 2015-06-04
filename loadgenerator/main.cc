#include "imembench.h"
#include "config.h"
#include "rediscluster.h"

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

  ConnectionConfig * rcconf = new ConnectionConfig(host, port);

  (*rcconf).setTransport(transport)
           .setClusterName(clusterName)
           .setTestTable(testTable);

  ok = gRamDriver.init(rcconf);
  if (!ok) {
    printf("fail to initialize ramcloud driver\n");
    exit(1);
  }
  printf("ramcloud driver for %s successfully initialized\n", rcconf->getLocator());
  char buf[64];
  gRamDriver.write("fooo", "bar", 4); // also store the '\0' in value
  gRamDriver.read("fooo", buf, sizeof(buf));
  printf("['%s']=%s\n", "fooo", buf);
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

  ConnectionConfig * tcconf = new ConnectionConfig(host, port);
  (*tcconf).setKVStore(kvstorePrefix);

  ok = gTacDriver.init(tcconf);
  if (!ok) {
    printf("fail to initialize tachyon driver\n");
    exit(1);
  }
  printf("tachyon driver successfully initialized\n");

  char buf[32];
  gTacDriver.write("fooo", "bar", 4); // also store the '\0' in value
  gTacDriver.read("fooo", buf, sizeof(buf));
  printf("['%s']=%s\n", "hello", buf);
}

void setupRedisDriver()
{
  bool ok;
  const char *cluster = NULL;
  const char *sport = NULL;
  const char *stimeout = NULL;
  int port; 
  double timeout;

  BenchConfigMap rd_configs;
  rd_configs = gParser.getSectionConfigs("redis");
  if (rd_configs.size() == 0) {
    fprintf(stderr, "Error: empty redis configurations\n");
    exit(1);
  }
  cluster = getConfig(rd_configs, "redis", "cluster");
  if (cluster == NULL)
    exit(1);
  sport = getConfig(rd_configs, "redis", "port");
  if (sport == NULL)
    exit(1);
  std::stringstream ss(sport);
  ss >> port;
  if (!ss) {
    fprintf(stderr, "Error: 'port'=%s is not an integer\n", sport);
    exit(1);
  }

  stimeout = getConfig(rd_configs, "redis", "timeout");
  if (stimeout != NULL) {
    std::stringstream ss(stimeout);
    ss >> timeout;
    if (!ss) {
      fprintf(stderr, "Error: 'port'=%s is not an integer\n", sport);
      exit(1);
    }
    if (timeout <= 0) {
      fprintf(stderr, "Error: 'timeout'=%s is not positive\n", stimeout);
      exit(1);
    }
  } else {
    timeout = 1.5;
  }
  
  ConnectionConfig * rdconf = new ConnectionConfig("", port); // use cluster instead of host
  (*rdconf).setCluster(cluster).setTimeout(timeout);

  HostList hosts = rdconf->getCluster();
  HostList::iterator hit;
  for (hit = hosts.begin(); hit != hosts.end(); ++hit) {
    printf("%s\n", hit->c_str());
  }
  ok = gRedDriver.init(rdconf);
  if (!ok) {
    printf("fail to initialize redis driver\n");
    exit(1);
  }
  RedisCluster *client = (RedisCluster *) gRedDriver.getClient();
  client->getClientForKey("hello", 5);
  client->getClientForKey("world", 5);
  client->getClientForKey("foo", 3);
  client->getClientForKey("bar", 3);
  client->getClientForKey("barrrr", 6);
  printf("redis driver successfully initialized\n");
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
  // setupTachyonDriver();
  setupRedisDriver();
  return 0;
}
