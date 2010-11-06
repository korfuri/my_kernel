#include <libC.h>

#define COLS 80
#define LINES 25

static unsigned char* const vram = 0xb8000;
static size_t vram_offset = 0;

void* memcpy(void* dest, void* src, size_t size) {
  unsigned char* cdest = dest;
  unsigned char* csrc = src;
  for (unsigned int i = 0; i < size; i++) {
    cdest[i] = csrc[i];
  }
  return dest;
}

void* memset(void* memory, unsigned char c, size_t size) {
  unsigned char* cmemory = memory;
  for (unsigned int i = 0; i < size; i++) {
    cmemory[i] = c;
  }
  return memory;
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

void puts(char* str) {
  for (unsigned int i = 0; str[i]; i++) {
    putchar(str[i]);
  }
  putchar('\n');
}

void clear_screen(void) {
  // Resets the whole screen to zero
  memset(vram, 0, COLS * LINES * 2);
}
