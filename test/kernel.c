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
  for (;;) {
    puts("a");
    for (unsigned int i = 0; i < 20000000; i++);
  }
  /* puts("a"); */
  /* puts("a"); */
  /* puts("a"); */

}
