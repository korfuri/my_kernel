#include <panic.h>

void panic(char* error_msg) {
  puts(error_msg);
  for (;;);
}
