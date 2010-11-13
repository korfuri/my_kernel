#include <tty.h>
#include <libC.h>
#include <paging.h>
#include <rmm.h>
#include <panic.h>
#include <multiboot.h>
#include <elfkernel.h>

void kmain(struct multiboot_info* mbi, unsigned int magic )
{
  if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
      panic("Wrong multiboot magic");
    }
  
  clear_screen();
  elf_init((void*)mbi->u.elf_sec.addr, mbi->u.elf_sec.num, mbi->u.elf_sec.shndx);
  rmm_init();
  
  puts("           Booting my_kernel");
  
  /* puts("Is paging enabled (0 = yes) ?"); */
  /* putnbr(is_paging_enabled()); */
  /* putchar('\n'); */
  /* set_pages(); */
  /* puts("Paging should be enabled !"); */
  /* puts("Is paging enabled (0 = yes) ?"); */
  /* putnbr(is_paging_enabled()); */
  /* putchar('\n'); */

  /* memcpy(0x3FE000, "coucou les amis", sizeof("coucou les amis")); */
  /* puts(0x3FF000); */

  /* rmm_init(); */
  /* for (;;) { */
  /*   putnbr16(rmm_allocate_page()); */
  /*   puts(""); */
  /* } */


  panic("End of kmain()");
}
