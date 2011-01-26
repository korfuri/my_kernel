#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include <libC.h>

#define KEYBOARD_BUFFER_SIZE	256

extern char	keyboard_buffer[256];
extern char*	keyboard_readPtr;
extern char*	keyboard_writePtr;
size_t keyboard_read(char* buf, size_t len);

#endif
