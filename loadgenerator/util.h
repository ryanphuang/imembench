#ifndef __IMEMBENCH_UTIL_H_
#define __IMEMBENCH_UTIL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t crc16(const char *key, unsigned int keylen);
unsigned int HASH_SLOT(const char *key, unsigned int keylen);

bool resolve(const char *hostname, char *ip, int iplen);

#ifdef __cplusplus
}
#endif

#endif /* __IMEMBENCH_UTIL_H_ */

/* vim: set ts=4 sw=4 : */
