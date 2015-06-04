#include <hiredis.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <algorithm>

#include "rediscluster.h"
#include "hash.h"

using namespace std;

RedisCluster::~RedisCluster()
{
  RClientConnMap::iterator rit;
  for (rit = m_clientconn_map.begin(); rit != m_clientconn_map.end(); ++rit) {
    redisContext *ctx = rit->second;
    if (ctx != NULL)
      redisFree(ctx);
  }
  m_clientconn_map.clear();
}

bool RedisCluster::init(ConnectionConfig *config)
{
  if (m_initialized) {
    printf("redis cluster is already set\n");
    return false;
  }

  HostList hosts = config->getCluster();
  if (hosts.empty()) {
    printf("Empty hosts to create redis cluster\n");
    return false;
  }

  int port = config->getPort();
  double timeout = config->getTimeout();
  int sec = (int) floor(timeout);
  int usec = (int) ((timeout - sec) * 1000000);
  struct timeval tv = {sec, usec};

  HostList::iterator hit;
  for (hit = hosts.begin(); hit != hosts.end(); ++hit) {
    redisContext* ctx = connect(hit->c_str(), port, &tv);
    if (ctx == NULL)
      return false;
  }
  m_initialized = true;
  return true;
}

redisContext *RedisCluster::connect(const char *host, int port, struct timeval *tv)
{
  redisContext* ctx;
  if (tv == NULL || (tv->tv_sec == 0 && tv->tv_usec == 0))
    ctx = redisConnect(host, port); // no timeout specified
  else
    ctx = redisConnectWithTimeout(host, port, *tv); 
  if (ctx == NULL || ctx->err) {
    if (ctx) {
      printf("Error connecting '%s:%d': %s\n", host, port, ctx->errstr);
      redisFree(ctx);
    } else {
      printf("Fail to create redis context for '%s:%d'\n", host, port);
    }
    return NULL;
  }
  printf("Created redis context for %s:%d\n", host, port);
  // update the connection map
  m_clientconn_map.insert(make_pair(make_pair(
      ctx->tcp.host, ctx->tcp.port), ctx));
  return ctx;
}

bool RedisCluster::buildSlots()
{
  if (!m_initialized) {
    printf("Cannot build slots: cluster uninitialized\n");
    return false;
  }
  if (m_clientconn_map.empty()) {
    printf("Cannot build slots: empty cluster\n");
    return false;
  }
  RClientConnMap::iterator rit;
  bool found = false;
  for (rit = m_clientconn_map.begin(); rit != m_clientconn_map.end(); ++rit) {
    redisContext *ctx = rit->second;
    if (ctx == NULL)
        continue;
    redisReply *reply;
    reply = (redisReply *) redisCommand(ctx, CLUSTER_SLOTS_CMD);
    if (reply->type == REDIS_REPLY_ARRAY) {
      unsigned int i;
      for (i = 0; i < reply->elements; ++i) {
        redisReply *rele = reply->element[i];
        if (rele->type == REDIS_REPLY_ARRAY && 
            rele->elements >= 3 && 
            rele->element[0]->type == REDIS_REPLY_INTEGER &&
            rele->element[1]->type == REDIS_REPLY_INTEGER &&
            rele->element[2]->type == REDIS_REPLY_ARRAY &&
            rele->element[2]->element[0]->type == REDIS_REPLY_STRING &&
            rele->element[2]->element[1]->type == REDIS_REPLY_INTEGER) {
          KeySlot slot;
          RClientConnMap::iterator cit;

          slot.start = (unsigned int) (rele->element[0]->integer);
          slot.end = (unsigned int) (rele->element[1]->integer);
          const char * host = rele->element[2]->element[0]->str;
          int port = (int) (rele->element[2]->element[1]->integer);

          printf("slot [%u-%u] is served by %s:%d\n", 
              slot.start, slot.end, host, port);

          redisContext *sc = NULL;
          cit = m_clientconn_map.find(make_pair(host, port));

          // if no context object, we need to connect
          if (cit == m_clientconn_map.end())
            sc = connect(host, port, NULL);  
          else
            sc = cit->second;
          if (sc != NULL) {
            m_clientslot_map.insert(make_pair(slot, sc));
            found = true;
          }
        } 
      }
    }
    freeReplyObject(reply);
    if (found)
      break;
  }

  RClientSlotMap::iterator rsit;
  int i = 0;
  for (rsit = m_clientslot_map.begin(); rsit != m_clientslot_map.end(); ++rsit) {
    printf("slot_map.%d = [%u-%u]\n", i, rsit->first.start, rsit->first.end);
    ++i;
  }
  return found;
}

void RedisCluster::getClients(vector<redisContext *> &clients)
{
  RClientConnMap::iterator rit;
  for (rit = m_clientconn_map.begin(); rit != m_clientconn_map.end(); ++rit) {
    redisContext *ctx = rit->second;
    if (ctx != NULL)
      clients.push_back(ctx);
  }
}

redisContext *RedisCluster::getClientForKey(const char *key, unsigned int keylen)
{
  if (m_clientslot_map.empty()) {
    printf("Error: empty slot map\n");
    return NULL;
  }

  unsigned int slot = HASH_SLOT(key, keylen);
  printf("slot('%s') = %u\n", key, slot);
  RClientSlotMap::iterator it;
  
  it = m_clientslot_map.lower_bound(KeySlot{slot, slot});
  if (it ==m_clientslot_map.end() || it->first.start > slot)
    --it;
  if (it->first.start <= slot && slot <= it->first.end) {
    redisContext *ctx = it->second;
    printf("slot %u is served by %s:%d [%u-%u]\n", slot, ctx->tcp.host, 
        ctx->tcp.port, it->first.start, it->first.end);
    return ctx;
  } else {
    printf("slot %u is not in near range [%u-%u]\n", slot, it->first.start, 
        it->first.end);
    return NULL;
  }
}
