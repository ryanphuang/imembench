#include <RamCloud.h>
#include <string.h>
#include <stdio.h>

#include "imembench.h"

using namespace RAMCloud;

void RamCloudDriver::reset()
{
  RAMCloudBenchConfig * rconfig = dynamic_cast<RAMCloudBenchConfig *>(m_config);
  if (m_initialized && m_client != NULL && rconfig != NULL) {
    const char *testTableName = rconfig->getTestTableName();
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
  if (rconfig != NULL) {
    delete rconfig;
  }

  m_initialized = false;
  m_config = NULL;

  m_client = NULL;
  m_buffer = NULL;
  m_test_table_id = 0;
}

bool RamCloudDriver::init(BenchConfig *config) 
{
  reset();
  RAMCloudBenchConfig * rconfig = dynamic_cast<RAMCloudBenchConfig *>(config);
  if (rconfig == NULL) {
    fprintf(stderr, "NULL RAMCloudBenchConfig\n");
    return false;
  }
  m_buffer = new Buffer();
  const char *cname = rconfig->getClusterName();
  if (cname == NULL || strlen(cname) == 0)
    m_client = new RamCloud(rconfig->getLocator());
  else
    m_client = new RamCloud(rconfig->getLocator(), cname);
  m_config = rconfig;

  // the r/w will happen inside the test table if no table name
  // is specified!
  const char *tableName = rconfig->getTestTableName();
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

inline void RamCloudDriver::write(const char *key, uint32_t keylen, 
      const char *value, uint32_t valuelen)
{
  m_client->write(m_test_table_id, key, downCast<uint16_t>(keylen), 
                    value, valuelen);
}

inline int RamCloudDriver::read(const char *key, uint32_t keylen,
      char *buf, uint32_t bufflen)
{
  m_client->read(m_test_table_id, key, downCast<uint16_t>(keylen), 
      m_buffer);
  uint32_t sz = m_buffer->size();
  strncpy(buf, static_cast<const char *>(m_buffer->getRange(0, sz)), bufflen);
  return sz > bufflen ? bufflen : sz;
}

