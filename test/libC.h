#ifndef LIBC_H_
#define LIBC_H_

typedef unsigned int size_t;

void putchar(char c);
void* memcpy(void* dest, void* src, size_t size);
void* memset(void* memory, unsigned char c, size_t size);

#endif
