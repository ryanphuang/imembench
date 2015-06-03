#ifndef __HASH_H_
#define __HASH_H_

#ifdef __cplusplus
extern "C" {
#endif

unsigned int HASH_SLOT(char *key, unsigned int keylen);
unsigned int crc16(char *key, unsigned int keylen);

#ifdef __cplusplus
}
#endif

#endif /* __HASH_H_ */

/* vim: set ts=4 sw=4 : */
