#ifndef __REDISCLUSTER_H_
#define __REDISCLUSTER_H_

#include <map>
#include <utility>
#include <vector>

#include "config.h"

class redisContext;

#define CLUSTER_SLOTS_CMD    "CLUSTER SLOTS"

struct KeySlot;
struct KeySlotComp;

typedef std::vector<redisContext *> RClients;
typedef std::map<std::pair<const char *, int>, redisContext *> RClientConnMap;
typedef std::map<KeySlot, redisContext *, KeySlotComp> RClientSlotMap;

typedef struct KeySlot {
  unsigned int start;
  unsigned int end;
} KeySlot;

typedef struct KeySlotComp {
  bool operator() (const KeySlot &lhs, const KeySlot &rhs) const 
  { 
    return lhs.start < rhs.start; 
  }
} KeySlotComp;

class RedisCluster {
  public:
    RedisCluster() { m_initialized = false; }
    ~RedisCluster();

    bool init(ConnectionConfig *config);
    redisContext *connect(const char *host, int port, struct timeval *tv);
    bool buildSlots();
    void getClients(std::vector<redisContext *> &clients);
    redisContext *getClientForKey(const char *key, unsigned int keylen);

  private:
    bool m_initialized;
    RClientConnMap m_clientconn_map;
    RClientSlotMap m_clientslot_map;
};



#endif /* __REDISCLUSTER_H_ */

/* vim: set ts=4 sw=4 : */
