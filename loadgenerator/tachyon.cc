#include <Tachyon.h>
#include <string.h>
#include <stdio.h>
#include <sstream>

#include "imembench.h"

using namespace tachyon;

bool TachyonDriver::init(ConnectionConfig *config)
{
  const char *host = config->getHost();
  int port = config->getPort();
  std::stringstream ss;
  ss << host << ":" << port;
  m_client = TachyonClient::createClient(ss.str().c_str());
  if (m_client == NULL) {
    fprintf(stderr, "fail to create tachyon client\n");
    return false;
  }

  m_config = config;
  m_initialized = true;
  return true;
}

void TachyonDriver::write(const char *key, const char *value, uint32_t len)
{

}

void TachyonDriver::read(const char *key, char *buff, uint32_t len)
{

}
