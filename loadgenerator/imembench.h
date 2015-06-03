#ifndef __IMEMBENCH_H_
#define __IMEMBENCH_H_

#include <cstdlib>
#include <string>
#include <sstream>

// some forward declarations
namespace RAMCloud {
  class RamCloud;
  class Buffer;
}
namespace tachyon {
  class TachyonClient;
}
class redisContext;
//////////////////////////////


class ConnectionConfig {
  public:
     ConnectionConfig(const char *host, int port, const char *transport = NULL, 
          const char *clusterName = NULL, const char *testTableName = NULL, 
          const char *kvStorePrefix = NULL) {
      if (host != NULL)
        m_host.assign(host);
      m_port = port;
      if (transport != NULL) {
        m_transport.assign(transport);
        std::stringstream ss;
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
    }

    const char *getHost() { return m_host.c_str(); }
    int getPort() { return m_port; }
    const char *getTransport() { return m_transport.c_str(); }
    const char *getLocator() { return m_locator.c_str(); }
    const char *getClusterName() { return m_cluster_name.c_str(); }
    const char *getTestTableName() { return m_test_table.c_str(); }
    const char *getKVStorePrefix() { return m_kvstore_prefix.c_str(); }

  private:
    std::string m_host;
    int m_port;

    std::string m_transport; // for ramcloud
    std::string m_cluster_name; // for ramcloud
    std::string m_locator; // for ramcloud
    std::string m_test_table; // for ramcloud

    std::string m_kvstore_prefix; // for tachyon
};

class BenchDriverBase {
  public:
    BenchDriverBase() {
      m_config = NULL;
      m_initialized = false;
      // m_client = NULL;
    }

    virtual ~BenchDriverBase() {}

    virtual bool init(ConnectionConfig *config) = 0;
    virtual void write(const char *key, const char *value, uint32_t len) = 0;
    virtual void read(const char *key, char *buff, uint32_t len) = 0;
    virtual void *getClient() = 0;

  protected:
    bool m_initialized;
    ConnectionConfig *m_config;
    // void *m_client;
};

class RamCloudDriver : BenchDriverBase {
  public:
    RamCloudDriver();
    ~RamCloudDriver();

    bool init(ConnectionConfig *config);
    void write(const char *key, const char *value, uint32_t len);
    void read(const char *key, char *buff, uint32_t len);
    void *getClient() { return m_client; }

  protected:
    RAMCloud::RamCloud *m_client;
    RAMCloud::Buffer *m_buffer;
    uint64_t m_test_table_id;
};

class TachyonDriver : BenchDriverBase {
  public:
    bool init(ConnectionConfig *config);
    void write(const char *key, const char *value, uint32_t len);
    void read(const char *key, char *buff, uint32_t len);
    void *getClient() { return m_client; }

  protected:
    tachyon::TachyonClient *m_client;
};

class RedisDriver : BenchDriverBase {
  public:
    bool init(ConnectionConfig *config);
    void write(const char *key, const char *value, uint32_t len);
    void read(const char *key, char *buff, uint32_t len);
    void *getClient() { return m_client; }

  protected:
    redisContext *m_client;
};

#endif /* __IMEMBENCH_H_ */

/* vim: set ts=4 sw=4 : */
