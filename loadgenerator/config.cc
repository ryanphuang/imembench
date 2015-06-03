#include "config.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <algorithm>
#include <utility>
#include <cstdio>

using namespace std;

ConnectionConfig::ConnectionConfig(const char *host, int port, 
    const char *transport, const char *clusterName, 
    const char *testTableName, const char *kvStorePrefix, 
    double timeout) {
      if (host != NULL)
        m_host.assign(host);
      m_port = port;
      if (transport != NULL) {
        m_transport.assign(transport);
        stringstream ss;
        ss << m_transport << ":host=" << m_host << ",port=" << port;
        m_locator = ss.str();
      }
      if (clusterName != NULL)
        m_cluster_name.assign(clusterName);
      if (testTableName != NULL)
        m_test_table.assign(testTableName); 
      if (kvStorePrefix != NULL) {
        m_kvstore_prefix.assign(kvStorePrefix);
        // make sure the prefix ends with a slash
        if (!m_kvstore_prefix.empty() && 
            m_kvstore_prefix[m_kvstore_prefix.length() - 1] != '/') {
          m_kvstore_prefix += '/';
        }
      }
      m_timeout = timeout;
}
inline bool NotSpace(char c)
{
  return !isspace(c);
}

string& BenchConfigParser::trim(string &s)
{
  string::iterator it = find_if(s.begin(), s.end(), NotSpace);
  s.erase(s.begin(), it);
  it = find_if(s.rbegin(), s.rend(), NotSpace).base();
  s.erase(it, s.end());
  return s;
}

bool BenchConfigParser::parse(const char *filename)
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

      pos = line.find(';');
      if (pos != string::npos) {
        line = line.substr(0, pos); // ignore chars after ';'
      }
    
      if (line[0] == '[' && line[line.length() - 1] == ']') {
        // it's a section
        section = line.substr(1, line.length() - 2);
        continue;
      }
      
      pos = line.find('=');
      if ((pos != string::npos) && (pos + 1 < line.length())) {
        string s1 = line.substr(0, pos);
        string s2 = line.substr(pos + 1);
        BenchConfig config(trim(s1), trim(s2));
        if (!section.empty() && 
            !config.first.empty() && 
            !config.second.empty()) {
          BenchConfigMap &map = m_configs[section];
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

const char *BenchConfigParser::getConfig(const char *section, const char *key, 
        const char *value)
{
  BenchConfigMaps::iterator msit;
  msit = m_configs.find(section);
  if (msit != m_configs.end()) {
    BenchConfigMap::iterator mit;
    mit = msit->second.find(key);
    if (mit != msit->second.end()) {
      return mit->second.c_str();
    }
  }
  return NULL;
}

void BenchConfigParser::getConfigKeys(BenchConfigSectionKeys &skeys)
{
  BenchConfigMaps::iterator msit;
  for (msit = m_configs.begin(); msit != m_configs.end(); ++msit) {
    BenchConfigMap::iterator mit;
    for (mit = msit->second.begin(); mit != msit->second.end(); ++mit) {
      skeys.push_back(make_pair(msit->first, mit->first));
    }
  }
}

void BenchConfigParser::getConfigKeys(const char *section, BenchConfigKeys &keys)
{
  BenchConfigMaps::iterator msit;
  msit = m_configs.find(section);
  if (msit == m_configs.end())
    return;

  BenchConfigMap::iterator mit;
  for (mit = msit->second.begin(); mit != msit->second.end(); ++mit) {
    keys.push_back(mit->first);
  }
}

void BenchConfigParser::getSections(BenchConfigSections &sections)
{
  BenchConfigMaps::iterator msit;
  for (msit = m_configs.begin(); msit != m_configs.end(); msit++) {
    sections.push_back(msit->first);
  }
}
