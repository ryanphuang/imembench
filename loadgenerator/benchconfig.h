#ifndef __CONFIG_H_
#define __CONFIG_H_

#include <string>
#include <map>
#include <vector>
#include <connconfig.h>

typedef std::pair<std::string, std::string> INIConfigKV;
typedef std::map<std::string, std::string> INISectionConfigs;
typedef std::map<std::string, INISectionConfigs> INIConfigDict; 
typedef std::vector<std::string> INIShortKeys;
typedef std::vector<std::pair<std::string, std::string>> INIFullKeys;
typedef std::vector<std::string> INISections;

class INIConfigParser {
  public:
    bool parse(const char *filename);

    INIConfigDict * getConfigs() { return &m_configs; }
    INISectionConfigs * getSectionConfigs(const char *section) 
    { 
      INIConfigDict::iterator it = m_configs.find(section);
      if (it == m_configs.end()) {
        return NULL;
      }
      return &(it->second);
    }

    const char *getConfig(const char *section, const char *key, const char *value);

    void getConfigKeys(INIFullKeys & skeys);
    void getConfigKeys(const char *section, INIShortKeys & keys);
    void getSections(INISections & sections);

  public:
    static std::string& trim(std::string &s);

  private:
    INIConfigDict m_configs;
};

class BenchConfig : public rediscluster::ConnectionConfig {
  public:
    BenchConfig(const char *host, int port) : 
        rediscluster::ConnectionConfig(host, port) {}

    virtual BenchConfig & setHost(const char *host)
    {
      rediscluster::ConnectionConfig::setHost(host);
      return *this;
    }

    virtual BenchConfig & setPort(int port)
    {
      rediscluster::ConnectionConfig::setPort(port);
      return *this;
    }

    virtual BenchConfig & setCluster(const char *cluster)
    {
      rediscluster::ConnectionConfig::setCluster(cluster);
      return *this;
    }
};


#define RAMCLOUD_CONFIG_SECTION              "ramcloud"
#define RAMCLOUD_HOST_CONFIG                 "host"
#define RAMCLOUD_PORT_CONFIG                 "port"
#define RAMCLOUD_TRANS_CONFIG                "transport"
#define RAMCLOUD_CLSTN_CONFIG                "cluster_name"
#define RAMCLOUD_CLSTN_CONFIG                "cluster_name"
#define RAMCLOUD_TTABLE_CONFIG               "test_table"

// add configuration specific for ramcloud
class RAMCloudBenchConfig : public BenchConfig {
  public:
    RAMCloudBenchConfig(const char *host, int port) : 
        BenchConfig(host, port) {}

    static RAMCloudBenchConfig * fromINI(INIConfigDict *configs);

    inline RAMCloudBenchConfig & setHost(const char *host) 
    {
      if (host != NULL) {
        m_host.assign(host);
        rebuildLocator();
      }
      return *this;
    }

    inline RAMCloudBenchConfig & setPort(int port)
    {
      m_port = port;
      rebuildLocator();
      return *this;
    }

    inline RAMCloudBenchConfig & setTransport(const char *transport)
    {
      if (transport != NULL) {
        m_transport.assign(transport);
        rebuildLocator();
      }
      return *this;
    }

    inline RAMCloudBenchConfig& setClusterName(const char *clusterName)
    {
      if (clusterName != NULL)
        m_cluster_name.assign(clusterName);
      return *this;
    }

    inline RAMCloudBenchConfig& setTestTable(const char *testTableName)
    {
      if (testTableName != NULL)
        m_test_table.assign(testTableName); 
      return *this;
    }

    const char *getTransport() { return m_transport.c_str(); }
    const char *getLocator() { return m_locator.c_str(); }
    const char *getClusterName() { return m_cluster_name.c_str(); }
    const char *getTestTableName() { return m_test_table.c_str(); }

    void rebuildLocator();

  protected:
    std::string m_transport; // for ramcloud
    std::string m_cluster_name; // for ramcloud
    std::string m_locator; // for ramcloud
    std::string m_test_table; // for ramcloud
};

#define TACHYON_CONFIG_SECTION            "tachyon"
#define TACHYON_HOST_CONFIG               "host"
#define TACHYON_PORT_CONFIG               "port"
#define TACHYON_KVSTORE_CONFIG            "kv_store_prefix"
#define TACHYON_RDTYPE_CONFIG             "read_type"
#define TACHYON_WTTYPE_CONFIG             "write_type"

// add tachyon specific configuration 
class TachyonBenchConfig : public BenchConfig {
  public:
    TachyonBenchConfig(const char *host, int port) : 
        BenchConfig(host, port) {}

    static TachyonBenchConfig * fromINI(INIConfigDict *configs);

    TachyonBenchConfig& setKVStore(const char *kvStorePrefix) 
    {
      if (kvStorePrefix != NULL) {
        m_kvstore_prefix.assign(kvStorePrefix);
        // make sure the prefix ends with a slash
        if (!m_kvstore_prefix.empty() && 
            m_kvstore_prefix[m_kvstore_prefix.length() - 1] != '/') {
          m_kvstore_prefix += '/';
        }
      }
      return *this;
    }

    TachyonBenchConfig&  setReadType(int readType) { m_read_type = readType; return *this; }
    TachyonBenchConfig&  setWriteType(int writeType) { m_write_type = writeType; return *this; }
    
    const char *getKVStore() { return m_kvstore_prefix.c_str(); }
    int getReadType() { return m_read_type; }
    int getWriteType() { return m_write_type; }

  protected:
    std::string m_kvstore_prefix; // for tachyon
    int m_read_type; // for tachyon
    int m_write_type; // for tachyon
};

#define REDIS_CONFIG_SECTION            "redis"
#define REDIS_HOST_CONFIG               "host"
#define REDIS_PORT_CONFIG               "port"
#define REDIS_CLUSTER_CONFIG            "cluster"
#define REDIS_TIMEOUT_CONFIG            "timeout"

//  add redis specific configuration
class RedisBenchConfig : public BenchConfig {
  public:
    RedisBenchConfig(const char *host, int port) : 
        BenchConfig(host, port) {}

    static RedisBenchConfig * fromINI(INIConfigDict *configs);
};


#endif /* __CONFIG_H_ */

/* vim: set ts=4 sw=4 : */
