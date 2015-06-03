#ifndef __IMEMBENCH_H_
#define __IMEMBENCH_H_

#include <cstdlib>
#include <string>
#include <sstream>

#include "config.h"

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
class BenchDriverBase {
  public:
    BenchDriverBase() {
      m_config = NULL;
      m_initialized = false;
      // m_client = NULL;
    }

    virtual ~BenchDriverBase() {}

    virtual bool init(ConnectionConfig *config) = 0;
    virtual void reset() = 0;

    virtual void write(const char *key, const char *value, uint32_t len) = 0;
    virtual int read(const char *key, char *buff, uint32_t len) = 0;
    virtual void *getClient() = 0;

  protected:
    bool m_initialized;
    ConnectionConfig *m_config;
    // void *m_client;
};

class RamCloudDriver : BenchDriverBase {
  public:
    ~RamCloudDriver() { reset(); }

    bool init(ConnectionConfig *config);
    void reset();
  
    void write(const char *key, const char *value, uint32_t len);
    int read(const char *key, char *buff, uint32_t len);
    void *getClient() { return m_client; }

  protected:
    RAMCloud::RamCloud *m_client;
    RAMCloud::Buffer *m_buffer;
    uint64_t m_test_table_id;
};

class TachyonDriver : BenchDriverBase {
  public:
    ~TachyonDriver() { reset(); }

    bool init(ConnectionConfig *config);
    void reset();

    void write(const char *key, const char *value, uint32_t len);
    int read(const char *key, char *buff, uint32_t len);
    void *getClient() { return m_client; }

  protected:
    tachyon::TachyonClient *m_client;
};

class RedisDriver : BenchDriverBase {
  public:
    ~RedisDriver() { reset(); }

    bool init(ConnectionConfig *config);
    void reset();

    void write(const char *key, const char *value, uint32_t len);
    int read(const char *key, char *buff, uint32_t len);
    void *getClient() { return m_client; }

  protected:
    redisContext *m_client;
};

#endif /* __IMEMBENCH_H_ */

/* vim: set ts=4 sw=4 : */
