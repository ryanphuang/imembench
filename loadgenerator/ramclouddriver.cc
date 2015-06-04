#include <RamCloud.h>
#include <string.h>
#include <stdio.h>

#include "imembench.h"

using namespace RAMCloud;

void RamCloudDriver::reset()
{
  if (m_initialized && m_client != NULL && m_config != NULL) {
    const char *testTableName = m_config->getTestTableName();
    if (strlen(testTableName) > 0) {
      m_client->dropTable(testTableName);
      printf("test table '%s' dropped\n", testTableName);
    }
  }
  // TODO: WTF, we can't destroy the client!!
  // if (m_client != NULL)
  //  delete m_client;
  if (m_buffer != NULL)
    delete m_buffer;
  if (m_config != NULL)
    delete m_config;

  m_initialized = false;
  m_config = NULL;

  m_client = NULL;
  m_buffer = NULL;
  m_test_table_id = 0;
}

bool RamCloudDriver::init(ConnectionConfig *config) 
{
  reset();
  m_buffer = new Buffer();

  const char *cname = config->getClusterName();
  if (cname == NULL || strlen(cname) == 0)
    m_client = new RamCloud(config->getLocator());
  else
    m_client = new RamCloud(config->getLocator(), cname);
  m_config = config;

  // the r/w will happen inside the test table if no table name
  // is specified!
  const char *tableName = config->getTestTableName();
  if (tableName != NULL && strlen(tableName) > 0) {
    try {
      RamCloud *rc = (RamCloud *)m_client;
      printf("creating test table '%s'...\n", tableName);
      rc->createTable(tableName);
      m_test_table_id = rc->getTableId(tableName);
      printf("test table '%s' created.\n", tableName);
      m_initialized = true;
      return true;
    } catch (TableDoesntExistException tde) {
      m_initialized = false;
      printf("failed to create test table '%s'.", tableName);
      return false;
    }
  }
  return true;
}

inline void RamCloudDriver::write(const char *key, const char *value, uint32_t len)
{
  m_client->write(m_test_table_id, key, downCast<uint16_t>(strlen(key)), 
                    value, len);
}

inline int RamCloudDriver::read(const char *key, char *buf, uint32_t len)
{
  m_client->read(m_test_table_id, key, downCast<uint16_t>(strlen(key)), 
                    m_buffer);
  uint32_t sz = m_buffer->size();
  strncpy(buf, static_cast<const char *>(m_buffer->getRange(0, sz)), len);
  return sz > len ? len : sz;
}

