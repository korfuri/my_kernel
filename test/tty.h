#ifndef TTY_H_
#define TTY_H_

#include <libC.h>

void putchar(char c);
void clear_screen(void);
void puts(char* str);
void putstr(char* str);
void putstrn(char* str, size_t s);
void putnbr16(uint64_t n);
void putnbr(int64_t n);
void printf(char* format, ...);

#endif
