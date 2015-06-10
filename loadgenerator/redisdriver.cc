#include <hiredis.h>
#include <hirediscluster.h>

#include <string>
#include <cstring>
#include <cstdio>
#include <cmath>

#include "imembench.h"

using namespace std;
using namespace rediscluster;

bool RedisDriver::init(BenchConfig *config)
{
  reset();
  if (config == NULL) {
    fprintf(stderr, "NULL RedisBenchConfig\n");
    return false;
  }
  m_client = new RedisCluster();
  if (!m_client->init(config)) {
    delete m_client;
    m_client = NULL;
    return false;
  }
  if (!m_client->buildSlots()) {
    printf("fail to build slots\n");
    delete m_client;
    m_client = NULL;
    return false;
  }
  m_config = config;
  m_initialized = true;
  return true;
}

void RedisDriver::reset()
{
  if (m_client != NULL)
    delete m_client;
  if (m_config != NULL)
    delete m_config;
  m_initialized = false;
  m_config = NULL;
}

void RedisDriver::write(const char *key, uint32_t keylen,
        const char *value, uint32_t valuelen)
{
  redisContext *c = m_client->getClientForKey(key, keylen);
  if (c != NULL) {
    redisReply *reply = m_client->executeCommand(c,"SET %s %s", 
      string(key, keylen).c_str(), value);
    freeReplyObject(reply);
  }
}

int RedisDriver::read(const char *key, uint32_t keylen, 
        char *buff, uint32_t bufflen)
{
  int rd = -1;
  redisContext *c = m_client->getClientForKey(key, keylen);
  if (c != NULL) {
    redisReply *reply = m_client->executeCommand(c, 
        "GET %s", string(key, keylen).c_str());
    if (reply->type == REDIS_REPLY_STRING) {
      strncpy(buff, reply->str, bufflen);
      rd = (int) strnlen(buff, bufflen);
    }
    freeReplyObject(reply);
  }
  return rd;
}

