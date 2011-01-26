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

void unlocked_putchar(char c);
void unlocked_clear_screen(void);
void unlocked_puts(char* str);
void unlocked_putstr(char* str);
void unlocked_putstrn(char* str, size_t s);
void unlocked_putnbr16(uint64_t n);
void unlocked_putnbr(int64_t n);
void unlocked_printf(char* format, ...);

#endif
