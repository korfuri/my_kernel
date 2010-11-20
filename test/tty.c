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


static void putnbr_r(int64_t n) {
  if (n > 9)
    putnbr_r(n / 10);
  putchar('0' + (n % 10));
}

void putnbr(int64_t n) {
  if (n < 0) {
    putchar('-');
    putnbr_r(-n);
  } else {
    putnbr_r(n);
  }
}

static void putnbr16_r(uint64_t n) {
  if (n > 15)
    putnbr16_r(n / 16);
  if (n % 16 < 10)
    putchar('0' + (n % 16));
  else
    putchar('A' - 10 + (n % 16));
}

void putnbr16(uint64_t n) {
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
  } else if (c == '\t') {
    // Rounds up to 8
    vram_offset = (vram_offset + 8) & ~7;
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
  char	type[10];
  unsigned int j = 0;
  uint64_t value;
  
  va_start(arg, format);
  for (size_t i = 0; format[i] != '\0'; i++) {
    if (format[i] == '%') {
      i++;
      type[0] = '\0';
      if (format[i] == '(') {
	i++;
	j = 0;
	while (j < 9 && format[i] != '\0' && format[i] != ')') {
	  type[j] = format[i];
	  i++;
	  j++;
	}
	type[j] = '\0';
	if (format[i] == ')')
	  i++;
      }
      if (!strcmp(type, "int64_t") ||
	  !strcmp(type, "uint64_t")) {
	value = va_arg(arg, uint64_t);
      }
      else if (!strcmp(type, "int16_t") ||
	       !strcmp(type, "uint16_t"))
	value = va_arg(arg, uint32_t); // int16 are promoted to int32 when passed through ...
      else if (!strcmp(type, "int8_t") ||
	       !strcmp(type, "uint8_t") ||
	       !strcmp(type, "char") ||
	       !strcmp(type, "uchar"))
	value = va_arg(arg, uint32_t); // int8 are promoted to int32 when passed through ...
      else
	value = va_arg(arg, uint32_t);
      
      switch (format[i]) {
      case 's':
	putstr((char*)(uint32_t)value);
	break;
      case 'i':
      case 'd':
	putnbr((int64_t)value);
      break;
      case 'x':
      case 'X':
      case 'p':
	putnbr16((uint64_t)value);
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
