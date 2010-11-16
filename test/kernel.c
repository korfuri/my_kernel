#include <tty.h>
#include <libC.h>
#include <paging.h>
#include <rmm.h>
#include <panic.h>
#include <multiboot.h>
#include <elfkernel.h>

void kmain(struct multiboot_info* mbi, unsigned int magic)
{
  if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
      panic("Wrong multiboot magic");
    }

  clear_screen();

  puts("\n\n         *** Booting my_kernel ***\n\n");



  elf_init((void*)mbi->u.elf_sec.addr, mbi->u.elf_sec.num, mbi->u.elf_sec.shndx);

  size_t total_free_memory = rmm_init(mbi);
  printf("We have %x bytes available (%d MiB)\n",
	 total_free_memory,
	 total_free_memory / (1024*1024));


  printf("Is paging enabled ? %s\n", is_paging_enabled() ? "no" : "yes");
  paging_context kernel_paging_context = init_paging();
  printf("Is paging enabled ? %s\n", is_paging_enabled() ? "no" : "yes");

  uintptr_t p1 = rmm_allocate_page();
  add_range_paging_protected(current_paging_context_virtual(), 0x0ff00000, p1, PAGE_SIZE);

  volatile int* ptr = (int*)0x0ff00000;
  *ptr = 42;

  paging_context ctx1 = fork_paging_context();
  uintptr_t p2 = rmm_allocate_page();
  add_range_paging_protected(current_paging_context_virtual(), 0x0ff00000, p2, PAGE_SIZE);

  *ptr = 21;

  restore_paging_context(kernel_paging_context);
  printf("In kernel paging context, *ptr = %d\n", *ptr);
  restore_paging_context(ctx1);
  printf("In ctx1 paging context, *ptr = %d\n", *ptr);
  panic("End of kmain()");
}
