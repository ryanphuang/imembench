#include <hiredis.h>

#include <string.h>
#include <stdio.h>

#include "imembench.h"

bool RedisDriver::init(ConnectionConfig *config)
{
  return false;
}

void RedisDriver::reset()
{
}

void RedisDriver::write(const char *key, const char *value, uint32_t len)
{

}

int RedisDriver::read(const char *key, char *buff, uint32_t len)
{
  return -1;
}
