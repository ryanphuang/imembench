#ifndef __IMEMBENCH_H_
#define __IMEMBENCH_H_

#include <cstdlib>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <utility>

#include "benchconfig.h"

// some forward declarations
namespace RAMCloud {
  class RamCloud;
  class Buffer;
}
namespace tachyon {
  class TachyonClient;
  class TachyonKV;
}
namespace rediscluster {
  class RedisCluster;
}

////////////////////////////////////////////
//    Basic benchmark driver, should
//    define a driver based on this
//    for each system.
// /////////////////////////////////////////
class BenchDriverBase {
  public:
    BenchDriverBase(const char *name) {
      m_name = name;
      m_config = NULL;
      m_initialized = false;
    }

    virtual ~BenchDriverBase() {}

    virtual bool init(BenchConfig *config) = 0;
    virtual void reset() = 0;

    virtual void write(const char *key, uint32_t keylen, const char *value, uint32_t valuelen) = 0;
    virtual int read(const char *key, uint32_t keylen, char *buff, uint32_t bufflen) = 0;

    virtual const char *getName() { return m_name; }
    virtual void *getClient() = 0;

  protected:
    const char *m_name;
    bool m_initialized;
    BenchConfig *m_config;
    // void *m_client;
};

// represents each type of benchmark
struct BenchMark {
  const char*name;
  void (*run)(BenchDriverBase *driver);
};

void runBenchMarks(BenchDriverBase **drivers, int ndriver, 
      const char **benchnames, int nbench);

////////////////////////////////////////////
// Benchmark drivers for ramcloud, tachyon,
// redis, etc.
////////////////////////////////////////////

class RamCloudDriver : public BenchDriverBase {
  public:
    RamCloudDriver(const char *name) : BenchDriverBase(name) {}
    ~RamCloudDriver() { reset(); }

    bool init(BenchConfig *config);
    void reset();
  
    void write(const char *key, uint32_t keylen, const char *value, uint32_t valuelen);
    int read(const char *key, uint32_t keylen, char *buff, uint32_t bufflen);
    void *getClient() { return m_client; }

  protected:
    RAMCloud::RamCloud *m_client;
    RAMCloud::Buffer *m_buffer;
    uint64_t m_test_table_id;
};

class TachyonDriver : public BenchDriverBase {
  public:
    TachyonDriver(const char *name) : BenchDriverBase(name) {}
    ~TachyonDriver() { reset(); }

    bool init(BenchConfig *config);
    void reset();

    void write(const char *key, uint32_t keylen, const char *value, uint32_t valuelen);
    int read(const char *key, uint32_t keylen, char *buff, uint32_t bufflen);
    void *getClient() { return m_client; }

  protected:
    tachyon::TachyonKV *m_client;
};

class RedisDriver : public BenchDriverBase {
  public:
    RedisDriver(const char *name) : BenchDriverBase(name) {}
    ~RedisDriver() { reset(); }

    bool init(BenchConfig *config);
    void reset();

    void write(const char *key, uint32_t keylen, const char *value, uint32_t valuelen);
    int read(const char *key, uint32_t keylen, char *buff, uint32_t bufflen);
    void *getClient()  { return m_client; } 

  protected:
    rediscluster::RedisCluster *m_client;
};

#endif /* __IMEMBENCH_H_ */

/* vim: set ts=4 sw=4 : */
