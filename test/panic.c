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

  printf("   eip\t\t   symbol + offset\t\t   ebp\n");
  while (i++ < MAX_BACKTRACE_DEPTH) {
    eip = *(uintptr_t*)(ebp + 4);
    ebp = *(uintptr_t*)(ebp);
    if (ebp == 0 || eip == 0)
      break;

    symname = elf_get_sym_name_before(eip, &diff);
    
    printf("%p\t%s<%x>+%x\t\%p\n", eip, symname, eip - diff, diff, ebp);
  }
  if (i == MAX_BACKTRACE_DEPTH)
    printf("<Backtrace aborted after %d stackframes>\n", MAX_BACKTRACE_DEPTH);
  puts("End of backtrace\n");
}

void kmain(void);

void panic(char* error_msg) {
  printf("\n========= PANIC =========\n\nCaused by : %s\n\nCaused at:\n", error_msg);
  backtrace();
  for (;;)
    asm volatile("hlt");
}

void dump_regs(struct registers* regs) {
  printf("eax: %p\tebx: %p\tecx: %p\tedx: %p\n", regs->eax, regs->ebx, regs->ecx, regs->edx);
  printf("esp: %p\tebp: %p\tesi: %p\tedi: %p\n", regs->esp, regs->ebp, regs->esi, regs->edi);
}
