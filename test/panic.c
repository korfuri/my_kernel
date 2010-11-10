#include <panic.h>
#include <tty.h>
#include <libC.h>

void backtrace(void) {
  intptr_t eip = 0;
  intptr_t ebp = 0;
  unsigned int i = 0;
  
  asm volatile("mov %%ebp, %0": "=b"(ebp));

  while (i++ < MAX_BACKTRACE_DEPTH) {
    eip = *(intptr_t*)(ebp + 4);
    ebp = *(intptr_t*)(ebp);

    if (ebp == 0 || eip == 0)
      break;

    size_t diff;
    char* symname = elf_get_sym_name_before(eip, &diff);
    
    putnbr16(eip);
    puts(" - eip");
    putnbr16(symname);
    puts(symname);
    puts(" + ");
    putnbr16(diff);
    
    /* putnbr16(ebp); */
    /* puts(" - ebp"); */

    puts("");

  }
  puts("End of backtrace");
}

void kmain();

void panic(char* error_msg) {
  puts("");
  puts("======= PANIC =======");
  puts(error_msg);
  backtrace();
  for (;;);
}

