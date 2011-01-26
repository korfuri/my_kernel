#include <keyboard.h>

char	keyboard_buffer[256];
char*	keyboard_readPtr = keyboard_buffer;
char*	keyboard_writePtr = keyboard_buffer;


size_t keyboard_read(char* buf, size_t len) {
  // FIXME UGLY

  size_t i;
  
  if (len > KEYBOARD_BUFFER_SIZE)
    len = KEYBOARD_BUFFER_SIZE;
  for (i = 0; i < len && keyboard_readPtr != keyboard_writePtr; i++) {
    buf[i] = *keyboard_readPtr++;
    if (keyboard_readPtr > keyboard_buffer + KEYBOARD_BUFFER_SIZE)
      keyboard_readPtr = keyboard_buffer;
  }
  return i;
}
