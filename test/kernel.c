#include <tty.h>
#include <libC.h>
#include <paging.h>
#include <segmentation.h>
#include <interrupts.h>
#include <rmm.h>
#include <panic.h>
#include <multiboot.h>
#include <elfkernel.h>
#include <threads.h>
#include <ports.h>
#include <keyboard.h>
#include <user/syscalls.h>
#include <demo.h>


void process_init(void* p __attribute__((unused))) {
  enable_interrupts();
  printf("Starting threaded mode...\n");
  new_thread(initsh, NULL);
  for (;;) {
    enable_interrupts();
    asm volatile("hlt");
  }
}

void kmain(struct multiboot_info* mbi, unsigned int magic)
{
  if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
      panic("Wrong multiboot magic");
    }

  unlocked_clear_screen();

  unlocked_puts("\n\n\t\t\t*** Booting my_kernel ***\n\n");



  elf_init((void*)mbi->u.elf_sec.addr, mbi->u.elf_sec.num, mbi->u.elf_sec.shndx);

  size_t total_free_memory = rmm_init(mbi);
  unlocked_printf("We have %x bytes available (%d MiB)\n",
		  total_free_memory,
		  total_free_memory / (1024*1024));
  
  segmentation_init();

  unsigned short divisor = 1193180 / 20;       /* Calculate our divisor */
  outportb(0x43, 0x36);             /* Set our command byte */
  outportb(0x40, divisor & 0xFF);   /* Set low byte of divisor */
  outportb(0x40, divisor >> 8);     /* Set high byte of divisor */

  interrupts_init();
  

  /*
    printf("Is paging enabled ? %s\n", is_paging_enabled() ? "no" : "yes");
    printf("Is paging enabled ? %s\n", is_paging_enabled() ? "no" : "yes");
    
    uintptr_t p1 = rmm_allocate_page();
    printf("Mapping %p to 0x0ff0 0000 in this context (kernel)\n", p1);
    add_range_paging_protected(current_paging_context_virtual(), 0x0ff00000, p1, PAGE_SIZE);
    
    volatile int* ptr = (int*)0x0ff00000;
    *ptr = 42;
    
    paging_context ctx1 = fork_paging_context();
    uintptr_t p2 = rmm_allocate_page();
    printf("Mapping %p to 0x0ff0 0000 in this context (ctx1)\n", p2);
    add_range_paging_protected(current_paging_context_virtual(), 0x0ff00000, p2, PAGE_SIZE);
    
    *ptr = 21;
    
    restore_paging_context(kernel_paging_context);
    printf("In kernel paging context, *ptr = %d\n", *ptr);
    restore_paging_context(ctx1);
    printf("In ctx1 paging context, *ptr = %d\n", *ptr);
    
    destroy_current_paging_context(kernel_paging_context);
    printf("In kernel paging context, *ptr = %d\n", *ptr);
  */
  
  //  tss_init();
  
  //  asm volatile("int $3");
  //  asm volatile("hlt");
  
  //  process_init();
  //  struct process* pid1 = process_new();
  //  process_first_sysexit(pid1->esp, pid1->eip);
  //  process_schedule(pid1);

  /* paging_context kernel_paging_context = */ init_paging();
  

  start_threads(process_init);

  panic("End of kmain()");
}
