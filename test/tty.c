#include <tty.h>
#include <libC.h>

#define COLS 80
#define LINES 25

static unsigned char* const vram = 0xb8000;
static size_t vram_offset = 0;

void puts(char* str) {
  for (unsigned int i = 0; str[i]; i++) {
    putchar(str[i]);
  }
  putchar('\n');
}

static void putnbr_r(int n) {
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

static void putnbr16_r(int n) {
  if (n > 15)
    putnbr16_r(n / 16);
  if (n % 16 < 10)
    putchar('0' + (n % 16));
  else
    putchar('A' - 10 + (n % 16));
}

void putnbr16(int n) {
  putchar('0');
  putchar('x');
  if (n < 0) {
    putchar('-');
    putnbr16_r(-n);
  } else {
    putnbr16_r(n);
  }
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
    vram_offset -= COLS * 2;
  }
}
