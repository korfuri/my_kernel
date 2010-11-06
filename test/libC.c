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
