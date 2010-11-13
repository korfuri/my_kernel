#ifndef LIBC_H_
#define LIBC_H_

typedef unsigned int uint32_t;
typedef unsigned short int uint16_t;

typedef unsigned int size_t;
#ifndef HAS_UINTPTR_T
typedef unsigned int uintptr_t;
#endif

#define NULL ((void*)0)

void* memcpy(void* dest, void* src, size_t size);
void* memset(void* memory, unsigned char c, size_t size);
int strcmp(char* s1, char* s2);

#endif
