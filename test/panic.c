#include <panic.h>
#include <tty.h>
#include <libC.h>
#include <elfkernel.h>

void backtrace(void) {
  uintptr_t eip = 0;
  uintptr_t ebp = 0;
  unsigned int i = 0;
  size_t diff;
  char* symname;

  asm volatile("mov %%ebp, %0": "=b"(ebp));

  while (i++ < MAX_BACKTRACE_DEPTH) {
    eip = *(uintptr_t*)(ebp + 4);
    ebp = *(uintptr_t*)(ebp);
    if (ebp == 0 || eip == 0)
      break;

    symname = elf_get_sym_name_before(eip, &diff);
    
    printf("%p <%s+%x> (ebp = %p)\n", eip, symname, diff, ebp);
  }
  puts("End of backtrace");
}

void kmain();

void panic(char* error_msg) {
  puts("\n======= PANIC =======");
  puts(error_msg);
  backtrace();
  for (;;);
}

