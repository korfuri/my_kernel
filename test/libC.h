#ifndef LIBC_H_
#define LIBC_H_

/* typedef unsigned int uint32_t; */
/* typedef unsigned long long int uint64_t; */
/* typedef unsigned short int uint16_t; */
/* typedef int int32_t; */
/* typedef long long int int64_t; */
/* typedef short int int16_t; */
/* typedef char int8_t; */
/* typedef unsigned char uint8_t; */
/* typedef unsigned char uchar; */

#include <stdint.h>

typedef unsigned int size_t;

#define NULL ((void*)0)

void* memcpy(void* dest, void* src, size_t size);
void* memset(void* memory, unsigned char c, size_t size);
int strcmp(char* s1, char* s2);

#define DO_PRAGMA(x) _Pragma (#x)
#define TODO(who, what) DO_PRAGMA(message ("TODO(" #who ") - " #what))

#endif
