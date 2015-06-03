#include <Tachyon.h>
#include <string>
#include <cstring>
#include <cstdio>
#include <sstream>

#include "imembench.h"

using namespace tachyon;
using namespace std;

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
  const char *kvprefix = config->getKVStorePrefix();
  if (strlen(kvprefix) > 0) {
    bool ok = m_client->mkdir(kvprefix);
    if (!ok) {
      fprintf(stderr, "fail to create kvstore directory %s\n", kvprefix);
      return false;
    }
  }
  m_config = config;
  m_initialized = true;
  return true;
}

void TachyonDriver::reset()
{
  if (m_initialized && m_client != NULL) {
    const char *kvprefix = m_config->getKVStorePrefix();
    if (strlen(kvprefix) > 0) {
      // bool ok = m_client->deletePath(kvprefix, true);
      // if (!ok) {
      //  fprintf(stderr, "fail to create kvstore directory %s\n", kvprefix);
      // }
    }
  }
  if (m_config != NULL)
    delete m_config;

  m_initialized = false;
  m_config = NULL;
}

inline void TachyonDriver::write(const char *key, const char *value, uint32_t len)
{
  const char *kvprefix = m_config->getKVStorePrefix();
  string s1(kvprefix);
  s1.append(key);
  jTachyonFile jfile = m_client->getFile(s1.c_str());
  if (jfile == NULL) {
    int fid = m_client->createFile(s1.c_str());
    if (fid > 0)
      jfile = m_client->getFile(fid);
  }
  if (jfile != NULL) {
    jOutStream ostream = jfile->getOutStream(MUST_CACHE);
    if (ostream != NULL) {
      ostream->write(value, len);
      ostream->close();
      delete ostream;
    }
    delete jfile;
  }
}

inline int TachyonDriver::read(const char *key, char *buff, uint32_t len)
{
  int sz = -1;
  const char *kvprefix = m_config->getKVStorePrefix();
  string s1(kvprefix);
  s1.append(key);
  jTachyonFile jfile = m_client->getFile(s1.c_str());
  if (jfile == NULL)
    return sz;
  jInStream istream = jfile->getInStream(CACHE);
  if (istream != NULL) {
    sz = istream->read(buff, len);
    delete istream;
  }
  delete jfile;
  return sz;
}
