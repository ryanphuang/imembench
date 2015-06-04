#ifndef __HASH_H_
#define __HASH_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t crc16(const char *key, unsigned int keylen);
unsigned int HASH_SLOT(const char *key, unsigned int keylen);

#ifdef __cplusplus
}
#endif

#endif /* __HASH_H_ */

/* vim: set ts=4 sw=4 : */
