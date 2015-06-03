#ifndef __CONFIG_H_
#define __CONFIG_H_

#include<string>
#include<map>
#include<vector>

typedef std::pair<std::string, std::string> BenchConfig;
typedef std::map<std::string, std::string> BenchConfigMap;
typedef std::map<std::string, BenchConfigMap> BenchConfigMaps; 
typedef std::vector<std::string> BenchConfigKeys;
typedef std::vector<std::pair<std::string, std::string>> BenchConfigSectionKeys;
typedef std::vector<std::string> BenchConfigSections;

class BenchConfigParser {
  public:
    bool parse(const char *filename);

    BenchConfigMaps & getConfigs() { return m_configs; }
    BenchConfigMap & getSectionConfigs(const char *section) { return m_configs[section]; }

    const char *getConfig(const char *section, const char *key, const char *value);

    void getConfigKeys(BenchConfigSectionKeys & skeys);
    void getConfigKeys(const char *section, BenchConfigKeys & keys);
    void getSections(BenchConfigSections & sections);

  public:
    static std::string& trim(std::string &s);

  private:
    BenchConfigMaps m_configs;
};

class ConnectionConfig {
  public:
     ConnectionConfig(const char *host, int port, const char *transport = NULL, 
          const char *clusterName = NULL, const char *testTableName = NULL, 
          const char *kvStorePrefix = NULL, double timeout = 1.5);

    const char *getHost() { return m_host.c_str(); }
    int getPort() { return m_port; }
    const char *getTransport() { return m_transport.c_str(); }
    const char *getLocator() { return m_locator.c_str(); }
    const char *getClusterName() { return m_cluster_name.c_str(); }
    const char *getTestTableName() { return m_test_table.c_str(); }
    const char *getKVStorePrefix() { return m_kvstore_prefix.c_str(); }
    double getTimeout() { return m_timeout; }

  private:
    std::string m_host;
    int m_port;

    std::string m_transport; // for ramcloud
    std::string m_cluster_name; // for ramcloud
    std::string m_locator; // for ramcloud
    std::string m_test_table; // for ramcloud

    std::string m_kvstore_prefix; // for tachyon

    double m_timeout; // for redis
};


#endif /* __CONFIG_H_ */

/* vim: set ts=4 sw=4 : */
