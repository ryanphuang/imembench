#ifndef __IMEMBENCH_H_
#define __IMEMBENCH_H_

#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
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

enum TraceOp {
  T_OP_GET,
  T_OP_UPDATE,
  T_OP_DELETE,
};

#define MAX_TRACE_KEY        128
#define MAX_TRACE_VALUE      4096

struct TraceLog {
  TraceOp op;
  char key[MAX_TRACE_KEY];
  char value[MAX_TRACE_VALUE];
};

class TraceLogParser {
  public:
    virtual bool init(const char *traceFile)
    {
      m_ifs.open(traceFile, std::ifstream::in);
      return m_ifs.is_open();
    }

    virtual bool nextLog(TraceLog *log) = 0;

    virtual void done()
    {
      if (m_ifs.is_open()) {
        m_ifs.close();
      }
    }

  protected:
    std::ifstream m_ifs;
};

class YCSBTraceParser : public TraceLogParser 
{
  public:
    bool nextLog(TraceLog *log); 
};

// represents each type of benchmark
struct BenchMark {
  const char *name;
  void (*run)(BenchDriverBase *driver, const char *traceFile);
};

struct WorkloadParameters {
 uint16_t keyLength;  
 uint32_t dataSize; 
 uint32_t operations;
 double readPercent;
 double writePercent;
};

extern BenchMark gBenchmarks[];
extern int gNbench;

void runBenchMarks(BenchDriverBase **drivers, int ndriver, 
      const char **benchnames, int nbench, const char *traceFile);

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
