#include <elf.h>
#include <tty.h>

void test_elf(Elf32_Shdr* shdr, unsigned int num, unsigned int shstrndx) {
  char* strings = shdr[shstrndx].sh_addr;
  
  puts("Nb sections : ");
  putnbr(num);
  for (unsigned int i = 0; i < num; i++) {
    puts("\n\n== Section ==");

    putnbr(shdr[i].sh_name);
    puts(" - name (offset)");

    puts(strings + shdr[i].sh_name);
    
    putnbr(i);
    puts(" - num");
 
    putnbr16(shdr[i].sh_addr);
    puts(" - addr");

    for (unsigned int sleep = 0; sleep < 200000000; sleep++);
 }
}
