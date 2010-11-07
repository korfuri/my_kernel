#include <tty.h>
#include <libC.h>
#include <paging.h>

void kmain( void* mbd, unsigned int magic )
{
  if ( magic != 0x2BADB002 )
    {
      /* Something went not according to specs. Print an error */
      /* message and halt, but do *not* rely on the multiboot */
      /* data structure. */
    }

  clear_screen();
  puts("Korfuri\nOcian\nKernel 0.0.0.0.0.0.0.0.1");
  puts("Is paging enabled (0 = yes) ?");
  putnbr(is_paging_enabled());
  putchar('\n');
  set_pages();
  puts("Paging should be enabled !");
  puts("Is paging enabled (0 = yes) ?");
  putnbr(is_paging_enabled());
  putchar('\n');
}
