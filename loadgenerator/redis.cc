#include <hiredis.h>

#include <string.h>
#include <stdio.h>
#include <math.h>

#include "imembench.h"

using namespace std;

bool RedisDriver::init(ConnectionConfig *config)
{
  reset();

  HostList hosts = config->getCluster();
  int port = config->getPort();
  double timeout = config->getTimeout();
  int sec = (int) floor(timeout);
  int usec = (int) ((timeout - sec) * 1000000);
  struct timeval tv = {sec, usec};

  HostList::iterator hit;
  for (hit = hosts.begin(); hit != hosts.end(); ++hit) {
    redisContext* ctx;
    if (sec == 0 && usec == 0)
      ctx = redisConnect(hit->c_str(), port); // no timeout specified
    else
      ctx = redisConnectWithTimeout(hit->c_str(), port, tv); 
    if (ctx == NULL || ctx->err) {
      if (ctx) {
        printf("Error connecting '%s:%d': %s\n", hit->c_str(), port, ctx->errstr);
        redisFree(ctx);
      } else {
        printf("Fail to create redis context for '%s:%d'\n", hit->c_str(), port);
      }
      return false;
    }
    printf("Created redis context for %s:%d\n", hit->c_str(), port);
    
    m_clients.insert(make_pair(make_pair(ctx->tcp.host, 
        ctx->tcp.port), ctx));
  }

  m_config = config;
  m_initialized = true;
  return true;
}

void RedisDriver::reset()
{
  map<pair<const char *, int>, redisContext *>::iterator rit;
  for (rit = m_clients.begin(); rit != m_clients.end(); ++rit) {
    redisContext *ctx = rit->second;
    if (ctx != NULL)
      redisFree(ctx);
  }
  m_clients.clear();

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

void RedisDriver::getClients(vector<void *> &clients)
{
  map<pair<const char *, int>, redisContext *>::iterator rit;
  for (rit = m_clients.begin(); rit != m_clients.end(); ++rit) {
    redisContext *ctx = rit->second;
    if (ctx != NULL)
      clients.push_back(ctx);
  }
}
