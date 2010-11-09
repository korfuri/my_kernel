#ifndef LIBC_H_
#define LIBC_H_

typedef unsigned int uint32_t;
typedef unsigned short int uint16_t;

typedef unsigned int size_t;
typedef unsigned int intptr_t;

#define NULL ((void*)0)

void* memcpy(void* dest, void* src, size_t size);
void* memset(void* memory, unsigned char c, size_t size);

#endif
