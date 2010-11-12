#include <stdarg.h>
#include <tty.h>
#include <libC.h>

#define COLS 80
#define LINES 25

static unsigned char* const vram = (unsigned char*)0xb8000;
static size_t vram_offset = 0;

void puts(char* str) {
  putstr(str);
  putchar('\n');
}

void putstr(char* str) {
  for (unsigned int i = 0; str[i]; i++) {
    putchar(str[i]);
  }
}

void putstrn(char* str, size_t s) {
  for (size_t i = 0; i < s; i++) {
    putchar(str[i]);
  }
}


static void putnbr_r(unsigned int n) {
  if (n > 9)
    putnbr_r(n / 10);
  putchar('0' + (n % 10));
}

void putnbr(int n) {
  if (n < 0) {
    putchar('-');
    putnbr_r(-n);
  } else {
    putnbr_r(n);
  }
}

static void putnbr16_r(unsigned int n) {
  if (n > 15)
    putnbr16_r(n / 16);
  if (n % 16 < 10)
    putchar('0' + (n % 16));
  else
    putchar('A' - 10 + (n % 16));
}

void putnbr16(unsigned int n) {
  putchar('0');
  putchar('x');
  putnbr16_r(n);
}

void clear_screen(void) {
  // Resets the whole screen to zero
  memset(vram, 0, COLS * LINES * 2);
}

void putchar(char c) {
  if (c == '\n') {
    // + COLS * 2 : for line feed
    // - (vram_offset % (COLS * 2)) : for carriage return
    vram_offset += (COLS * 2) - (vram_offset % (COLS * 2));
  } else {
    vram[vram_offset++] = c;
    vram[vram_offset++] = 0x07;
  }
  // Scrolls one line from the screen
  if (vram_offset > (LINES - 1) * COLS * 2) {
    memcpy(vram, vram + COLS * 2, (LINES - 1) * COLS * 2);
    vram[(LINES - 1) * COLS * 2] = ' ';
    vram_offset -= COLS * 2;
  }
}

void printf(char* format, ...) {
  va_list arg;
  
  va_start(arg, format);
  for (size_t i = 0; format[i] != '\0'; i++) {
    if (format[i] == '%') {
      i++;
      switch (format[i]) {
      case 's':
	putstr(va_arg(arg, char*));
	break;
      case 'i':
      case 'd':
	putnbr(va_arg(arg, int));
      break;
      case 'x':
      case 'X':
      case 'p':
	putnbr16(va_arg(arg, unsigned int));
      break;
      case '\0':
      default:
	i--;
      case '%':
	putchar('%');
      }
    } else {
      putchar(format[i]);
    }
  }
}
