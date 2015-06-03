#include <hiredis.h>

#include <string.h>
#include <stdio.h>
#include <math.h>

#include "imembench.h"

bool RedisDriver::init(ConnectionConfig *config)
{
  reset();
  const char *host = config->getHost();
  int port = config->getPort();
  double timeout = config->getTimeout();
  int sec = (int) floor(timeout);
  int usec = (int) ((timeout - sec) * 1000000);
  struct timeval tv = {sec, usec};
  redisContext* ctx = redisConnectWithTimeout(host, port, tv); 
  if (ctx == NULL || ctx->err) {
    if (ctx) {
      printf("Connection error: %s\n", ctx->errstr);
      redisFree(ctx);
    } else {
      printf("Fail to create redis context object\n");
    }
    return false;
  }
  m_client = ctx;
  m_config = config;
  m_initialized = true;
  return true;
}

void RedisDriver::reset()
{
  if (m_client != NULL)
    redisFree(m_client);

  if (m_config != NULL)
    delete m_config;

  m_initialized = false;
  m_config = NULL;
}

void RedisDriver::write(const char *key, const char *value, uint32_t len)
{

}

int RedisDriver::read(const char *key, char *buff, uint32_t len)
{
  return -1;
}
