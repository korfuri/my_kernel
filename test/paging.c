#include <paging.h>

int is_paging_enabled(void) {
  unsigned int cr0;

  // Fetches cr0
  asm volatile("mov %%cr0, %0": "=b"(cr0));
  // Tests paging bit
  return !(cr0 & 0x80000000);
}

static void enable_paging(void* page_directory) {
  unsigned int cr0;

  puts("AA");
  // Sets cr3
  asm volatile("mov %0, %%cr3":: "b"(page_directory));
  puts("AB");
  // Fetches cr0
  asm volatile("mov %%cr0, %0": "=b"(cr0));
  puts("AC");
  putnbr16(cr0);
  putchar('\n');
  // Sets paging bit
  cr0 |= 0x80000000;
  putnbr16(cr0);
  putchar('\n');
  puts("AD");
  // Updates cr0
  asm volatile("mov %0, %%cr0":: "b"(cr0));
  puts("AE");
}

void set_pages(void) {
  unsigned int *page_directory = (unsigned int*) 0x9C000;
  puts("a");
  for (unsigned i = 0; i < 1024; i++)
    {
      //attribute: supervisor level, read/write, not present.
      page_directory[i] = 0 | 2; 
    }
  puts("b");
  //our first page table comes right after the page directory
  unsigned int *first_page_table = page_directory + 0x1000;

    puts("c");
  // holds the physical address where we want to start mapping these pages to.
  // in this case, we want to map these pages to the very beginning of memory.
  unsigned int address = 0; 
  unsigned int i;
  
  //we will fill all 1024 entries, mapping 4 megabytes
  for(i = 0; i < 1024; i++)
    {
      first_page_table[i] = address | 3; // attributes: supervisor level, read/write, present.
      address = address + 4096; //advance the address to the next page boundary
    }
  puts("d");
  page_directory[0] = first_page_table; 
  page_directory[0] |= 3;// attributes: supervisor level, read/write, present
  puts("e");
  enable_paging(page_directory);
  puts("f");
}
