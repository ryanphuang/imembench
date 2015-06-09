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
  reset();
  const char *host = config->getHost();
  int port = config->getPort();
  std::stringstream ss;
  ss << host << ":" << port;
  TachyonClient *client = TachyonClient::createClient(ss.str().c_str());
  if (client == NULL) {
    fprintf(stderr, "fail to create tachyon client\n");
    return false;
  }
  const char *kvprefix = config->getKVStorePrefix();
  m_client = TachyonKV::createKV(client, CACHE, MUST_CACHE, 
      DEFAULT_KV_BLOCK_BYTES, kvprefix); 
  delete client; // don't need tachyon client any more, only need kvstore
  if (m_client == NULL) {
    fprintf(stderr, "fail to create tachyon kvstore\n");
    return false;
  }
  if (!m_client->init()) {
    fprintf(stderr, "fail to initialize tachyon kvstore\n");
    return false;
  }
  /*
  bool ok = m_client->mkdir(kvprefix);
  if (!ok) {
    fprintf(stderr, "fail to create kvstore directory %s\n", kvprefix);
    return false;
  }
  */
  m_config = config;
  m_initialized = true;
  return true;
}

void TachyonDriver::reset()
{
  if (m_initialized && m_client != NULL) {
    const char *kvprefix = m_config->getKVStorePrefix();
    if (strlen(kvprefix) > 0) {
      bool ok = m_client->getClient()->deletePath(kvprefix, true);
      if (!ok) {
        fprintf(stderr, "fail to create kvstore directory %s\n", kvprefix);
      }
    }
  }
  if (m_config != NULL)
    delete m_config;

  m_initialized = false;
  m_config = NULL;
}

inline void TachyonDriver::write(const char *key, uint32_t keylen,
        const char *value, uint32_t valuelen)
{
  m_client->set(key, keylen, value, valuelen);
  /*
  const char *kvprefix = m_config->getKVStorePrefix();
  string s1(kvprefix);
  s1.append(key, keylen);
  jTachyonFile jfile = m_client->getFile(s1.c_str());
  if (jfile != NULL) {
    delete jfile;
    if (!m_client->deletePath(s1.c_str(), false)) {
      return;
    }
  }
  int fid = m_client->createFile(s1.c_str());
  if (fid > 0)
    jfile = m_client->getFile(fid);
  if (jfile != NULL) {
    jOutStream ostream = jfile->getOutStream(MUST_CACHE);
    if (ostream != NULL) {
      ostream->write(value, valuelen);
      ostream->close();
      delete ostream;
    }
    delete jfile;
  }
  */
}

inline int TachyonDriver::read(const char *key, uint32_t keylen, 
        char *buff, uint32_t valuelen)
{
  return m_client->get(key, keylen, buff, valuelen);
  /*
  int sz = -1;
  const char *kvprefix = m_config->getKVStorePrefix();
  string s1(kvprefix);
  s1.append(key, keylen);
  jTachyonFile jfile = m_client->getFile(s1.c_str());
  if (jfile == NULL || jfile->getJObj() == NULL)
    return sz;
  jInStream istream = jfile->getInStream(CACHE);
  if (istream != NULL) {
    sz = istream->read(buff, valuelen);
    istream->close();
    delete istream;
  }
  delete jfile;
  return sz;
  */
}
