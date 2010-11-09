#include <panic.h>
#include <tty.h>
#include <libC.h>

void backtrace(void) {
  intptr_t eip;
  intptr_t ebp;
  
  asm volatile("mov %%ebp, %0": "=b"(ebp));

  for (;;) {
    eip = *(intptr_t*)(ebp + 12);
    ebp = *(intptr_t*)(ebp + 8);

    putnbr16(eip);
    puts(" - eip");

    putnbr16(ebp);
    puts(" - ebp");

    puts("=====");
    for (unsigned int i = 0; i < 200000000; i++);
  } 
}

void kmain();

void panic(char* error_msg) {
  puts("");
  puts("======= PANIC =======");
  putnbr16(kmain);
  putnbr16(panic);
  puts(error_msg);
  backtrace();
  for (;;);
}

