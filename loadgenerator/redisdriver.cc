#include <hiredis.h>

#include <string>
#include <cstring>
#include <cstdio>
#include <cmath>

#include "imembench.h"
#include "rediscluster.h"

using namespace std;

bool RedisDriver::init(ConnectionConfig *config)
{
  reset();
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

void RedisDriver::write(const char *key, const char *value, uint32_t len)
{
  redisContext *c = m_client->getClientForKey(key, (uint32_t) strlen(key));
  if (c != NULL) {
    redisReply *reply = m_client->retryMovedCommand(c,"SET %s %s", key, value);
    freeReplyObject(reply);
  }
}

int RedisDriver::read(const char *key, char *buff, uint32_t len)
{
  int rd = -1;
  redisContext *c = m_client->getClientForKey(key, (uint32_t) strlen(key));
  if (c != NULL) {
    redisReply *reply = m_client->retryMovedCommand(c, "GET %s", key);
    if (reply->type == REDIS_REPLY_STRING) {
      strncpy(buff, reply->str, len);
      rd = (int) strnlen(buff, len);
    }
    freeReplyObject(reply);
  }
  return rd;
}

