#ifndef __IMEMBENCH_H_
#define __IMEMBENCH_H_

#include <cstdlib>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <utility>

#include "config.h"

// some forward declarations
namespace RAMCloud {
  class RamCloud;
  class Buffer;
}
namespace tachyon {
  class TachyonClient;
}
class RedisCluster;

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

    virtual bool init(ConnectionConfig *config) = 0;
    virtual void reset() = 0;

    virtual void write(const char *key, const char *value, uint32_t len) = 0;
    virtual int read(const char *key, char *buff, uint32_t len) = 0;

    virtual const char *getName() { return m_name; }
    virtual void *getClient() = 0;

  protected:
    const char *m_name;
    bool m_initialized;
    ConnectionConfig *m_config;
    // void *m_client;
};

// represents each type of benchmark
struct BenchMark {
  const char*name;
  void (*run)(BenchDriverBase *driver);
};

////////////////////////////////////////////
// Benchmark drivers for ramcloud, tachyon,
// redis, etc.
////////////////////////////////////////////

class RamCloudDriver : public BenchDriverBase {
  public:
    RamCloudDriver(const char *name) : BenchDriverBase(name) {}
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

class TachyonDriver : public BenchDriverBase {
  public:
    TachyonDriver(const char *name) : BenchDriverBase(name) {}
    ~TachyonDriver() { reset(); }

    bool init(ConnectionConfig *config);
    void reset();

    void write(const char *key, const char *value, uint32_t len);
    int read(const char *key, char *buff, uint32_t len);
    void *getClient() { return m_client; }

  protected:
    tachyon::TachyonClient *m_client;
};

class RedisDriver : public BenchDriverBase {
  public:
    RedisDriver(const char *name) : BenchDriverBase(name) {}
    ~RedisDriver() { reset(); }

    bool init(ConnectionConfig *config);
    void reset();

    void write(const char *key, const char *value, uint32_t len);
    int read(const char *key, char *buff, uint32_t len);
    void *getClient()  { return m_client; } 

  protected:
    RedisCluster *m_client;
};

void runBenchMarks();

extern BenchMark gBenchmarks[];
extern size_t gNbench;
extern BenchDriverBase *gDrivers[];
extern size_t gNdriver;

#endif /* __IMEMBENCH_H_ */

/* vim: set ts=4 sw=4 : */
