#include "benchconfig.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <algorithm>
#include <utility>
#include <cstdio>

#include <Tachyon.h>

using namespace std;

static inline bool NotSpace(char c)
{
  return !isspace(c);
}

static const char *getAndCheckConfig(INISectionConfigs *sec_configs, const char *section, 
      const char *key, bool checkNone = true, bool checkEmpty = true)
{
  INISectionConfigs::iterator cit;
  cit = sec_configs->find(key);
  if (cit == sec_configs->end()) {
    if (checkNone) {
      fprintf(stderr, "Error: no '%s' setting for %s\n", key, section);
      exit(1);
    } else {
      return NULL;
    }
  }
  if (cit->second.length() == 0) {
    if (checkNone && checkEmpty) {
      fprintf(stderr, "Error: '%s' for %s has empty value\n", key, section);
      exit(1);
    }
  }
  return cit->second.c_str();
}

static INISectionConfigs * getAndCheckSection(INIConfigDict *configs, const char *section)
{
  INIConfigDict::iterator it = configs->find(section);
  if (it == configs->end()) {
    fprintf(stderr, "Error: empty '%s' section in configuration\n", section);
    exit(1);
  }
  return &(it->second);
}

string& INIConfigParser::trim(string &s)
{
  string::iterator it = find_if(s.begin(), s.end(), NotSpace);
  s.erase(s.begin(), it);
  it = find_if(s.rbegin(), s.rend(), NotSpace).base();
  s.erase(it, s.end());
  return s;
}

bool INIConfigParser::parse(const char *filename)
{
  m_configs.clear(); // reset configs
  string line;
  ifstream ifs(filename);
  if (ifs.is_open()) {
    size_t pos;
    string section;
    while (getline(ifs, line)) {
      line = trim(line);
      if (line.empty() || line[0] == ';')
        continue; // comment or empty line

     /* in standard INI, ';' means comment only if it's at the beginning of the line
      * so we shouldn't be more smart about it here.
      *
      *  pos = line.find(';');
      *  if (pos != string::npos) {
      *    line = line.substr(0, pos); // ignore chars after ';'
      *  }
      */
    
      if (line[0] == '[' && line[line.length() - 1] == ']') {
        // it's a section
        section = line.substr(1, line.length() - 2);
        continue;
      }
      
      pos = line.find('=');
      if ((pos != string::npos) && (pos + 1 < line.length())) {
        string s1 = line.substr(0, pos);
        string s2 = line.substr(pos + 1);
        INIConfigKV config(trim(s1), trim(s2));
        if (!section.empty() && 
            !config.first.empty() && 
            !config.second.empty()) {
          INISectionConfigs &map = m_configs[section];
          map.insert(config);
        }
      }
    }
    ifs.close();
    return true;
  } else {
    return false;
  }
}

const char *INIConfigParser::getConfig(const char *section, const char *key, 
        const char *value)
{
  INIConfigDict::iterator msit;
  msit = m_configs.find(section);
  if (msit != m_configs.end()) {
    INISectionConfigs::iterator mit;
    mit = msit->second.find(key);
    if (mit != msit->second.end()) {
      return mit->second.c_str();
    }
  }
  return NULL;
}

void INIConfigParser::getConfigKeys(INIFullKeys &skeys)
{
  INIConfigDict::iterator msit;
  for (msit = m_configs.begin(); msit != m_configs.end(); ++msit) {
    INISectionConfigs::iterator mit;
    for (mit = msit->second.begin(); mit != msit->second.end(); ++mit) {
      skeys.push_back(make_pair(msit->first, mit->first));
    }
  }
}

void INIConfigParser::getConfigKeys(const char *section, INIShortKeys &keys)
{
  INIConfigDict::iterator msit;
  msit = m_configs.find(section);
  if (msit == m_configs.end())
    return;

  INISectionConfigs::iterator mit;
  for (mit = msit->second.begin(); mit != msit->second.end(); ++mit) {
    keys.push_back(mit->first);
  }
}

void INIConfigParser::getSections(INISections &sections)
{
  INIConfigDict::iterator msit;
  for (msit = m_configs.begin(); msit != m_configs.end(); msit++) {
    sections.push_back(msit->first);
  }
}


void RAMCloudBenchConfig::rebuildLocator()
{
  stringstream ss;
  ss << m_transport << ":host=" << m_host << ",port=" << m_port;
  m_locator = ss.str();
}

RAMCloudBenchConfig * RAMCloudBenchConfig::fromINI(INIConfigDict *configs)
{
  const char *host = NULL;
  const char *sport = NULL;
  const char *transport = NULL;
  const char *clusterName = NULL;
  const char *testTable = NULL;
  int port; 
  INISectionConfigs * sec_configs;

  sec_configs = getAndCheckSection(configs, RAMCLOUD_CONFIG_SECTION);
  host = getAndCheckConfig(sec_configs, RAMCLOUD_CONFIG_SECTION, RAMCLOUD_HOST_CONFIG);
  sport = getAndCheckConfig(sec_configs, RAMCLOUD_CONFIG_SECTION, RAMCLOUD_PORT_CONFIG);
  std::stringstream ss(sport);
  ss >> port;
  if (!ss) {
    fprintf(stderr, "Error: 'port'=%s is not an integer\n", sport);
    exit(1);
  }
  transport = getAndCheckConfig(sec_configs, RAMCLOUD_CONFIG_SECTION, 
        RAMCLOUD_TRANS_CONFIG, false);
  clusterName = getAndCheckConfig(sec_configs, RAMCLOUD_CONFIG_SECTION, 
        RAMCLOUD_CLSTN_CONFIG, false);
  testTable = getAndCheckConfig(sec_configs, RAMCLOUD_CONFIG_SECTION,
        RAMCLOUD_TTABLE_CONFIG, false);
  if (testTable == NULL)
    testTable = "test";

  RAMCloudBenchConfig * rambench_config = new RAMCloudBenchConfig(host, port);
  (*rambench_config).setTransport(transport)
           .setClusterName(clusterName)
           .setTestTable(testTable);
  return rambench_config;
}

TachyonBenchConfig * TachyonBenchConfig::fromINI(INIConfigDict *configs)
{
  const char *host, *sport, *kvstorePrefix, *sreadType, *swriteType;
  int port, readType = tachyon::CACHE, writeType = tachyon::MUST_CACHE;
  INISectionConfigs * sec_configs;

  sec_configs = getAndCheckSection(configs, TACHYON_CONFIG_SECTION);
  host = getAndCheckConfig(sec_configs, TACHYON_CONFIG_SECTION, TACHYON_HOST_CONFIG);
  sport = getAndCheckConfig(sec_configs, TACHYON_CONFIG_SECTION, TACHYON_PORT_CONFIG);
  std::stringstream ss(sport);
  ss >> port;
  if (!ss) {
    fprintf(stderr, "Error: 'port'=%s is not an integer\n", sport);
    exit(1);
  }
  kvstorePrefix = getAndCheckConfig(sec_configs, TACHYON_CONFIG_SECTION, TACHYON_KVSTORE_CONFIG);
  sreadType = getAndCheckConfig(sec_configs, TACHYON_CONFIG_SECTION, TACHYON_RDTYPE_CONFIG);
  swriteType = getAndCheckConfig(sec_configs, TACHYON_CONFIG_SECTION, TACHYON_WTTYPE_CONFIG);
  //TODO: parse read/write type

  TachyonBenchConfig * tachyon_config = new TachyonBenchConfig(host, port);
  (*tachyon_config).setKVStore(kvstorePrefix)
           .setReadType(readType)
           .setWriteType(writeType);
  return tachyon_config;
}

RedisBenchConfig * RedisBenchConfig::fromINI(INIConfigDict *configs)
{
  const char *cluster, *sport, *stimeout;
  int port;
  double timeout;
  INISectionConfigs * sec_configs;

  sec_configs = getAndCheckSection(configs, REDIS_CONFIG_SECTION);
  sport = getAndCheckConfig(sec_configs, REDIS_CONFIG_SECTION, REDIS_PORT_CONFIG);
  std::stringstream ss(sport);
  ss >> port;
  if (!ss) {
    fprintf(stderr, "Error: 'port'=%s is not an integer\n", sport);
    exit(1);
  }
  cluster = getAndCheckConfig(sec_configs, REDIS_CONFIG_SECTION, REDIS_CLUSTER_CONFIG);
  stimeout = getAndCheckConfig(sec_configs, REDIS_CONFIG_SECTION, REDIS_TIMEOUT_CONFIG);
  if (stimeout != NULL) {
    std::stringstream ss(stimeout);
    ss >> timeout;
    if (!ss) {
      fprintf(stderr, "Error: 'timeout'=%s is not a numeric\n", stimeout);
      exit(1);
    }
    if (timeout <= 0) {
      fprintf(stderr, "Error: 'timeout'=%s is not positive\n", stimeout);
      exit(1);
    }
  } else {
    timeout = 1.5;
  }

  RedisBenchConfig * redis_config = new RedisBenchConfig("", port);
  (*redis_config).setCluster(cluster)
           .setTimeout(timeout);
  return redis_config;
}

