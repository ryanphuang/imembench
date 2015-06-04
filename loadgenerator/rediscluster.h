#ifndef __REDISCLUSTER_H_
#define __REDISCLUSTER_H_

#include <map>
#include <utility>
#include <vector>
#include <string.h>
#include <stdint.h>

#include "config.h"

struct redisContext;
struct redisReply;

#define CLUSTER_SLOTS_CMD    "CLUSTER SLOTS"

struct KeySlot;
struct KeySlotComp;
struct KeyHostComp;

typedef std::pair<const char *, int> KeyHost;
typedef std::vector<redisContext *> RClients;
typedef std::map<KeyHost, redisContext *, KeyHostComp> RClientConnMap;
typedef std::map<KeySlot, redisContext *, KeySlotComp> RClientSlotMap;

typedef struct KeySlot {
  uint32_t start;
  uint32_t end;
} KeySlot;

typedef struct KeySlotComp {
  bool operator() (const KeySlot &lhs, const KeySlot &rhs) const 
  { 
    return lhs.start < rhs.start; 
  }
} KeySlotComp;

typedef struct KeyHostComp {
  bool operator() (const KeyHost &lhs, const KeyHost &rhs) const
  {
    int cmp = strcmp(lhs.first, rhs.first); 
    if (cmp == 0)
      return lhs.second < rhs.second;
    return cmp < 0;
  }

} KeyHostComp;

class RedisCluster {
  public:
    RedisCluster() { m_initialized = false; }
    ~RedisCluster();

    bool init(ConnectionConfig *config);
    redisContext *connect(const char *host, int port, struct timeval *tv);
    bool buildSlots();
    void getClients(std::vector<redisContext *> &clients);
    redisContext *getClientForKey(const char *key, uint32_t keylen);
    redisReply *retryMovedCommand(redisContext *context, const char *format, ...);

  private:
    bool m_initialized;
    RClientConnMap m_clientconn_map;
    RClientSlotMap m_clientslot_map;
};



#endif /* __REDISCLUSTER_H_ */

/* vim: set ts=4 sw=4 : */
