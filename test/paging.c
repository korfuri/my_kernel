#include <paging.h>
#include <rmm.h>
#include <tty.h>

int is_paging_enabled(void) {
  unsigned int cr0;

  // Fetches cr0
  asm volatile("mov %%cr0, %0": "=b"(cr0));
  // Tests paging bit
  return !(cr0 & 0x80000000);
}

static void enable_paging(void* page_directory) {
  unsigned int cr0;

  // Sets cr3
  asm volatile("mov %0, %%cr3":: "b"(page_directory));
  // Fetches cr0
  asm volatile("mov %%cr0, %0": "=b"(cr0));
  // Sets paging bit
  cr0 |= 0x80000000;
  // Updates cr0
  asm volatile("mov %0, %%cr0":: "b"(cr0));
}

void set_first_pages(void) {
  unsigned int *page_directory = (unsigned int*)rmm_allocate_page();
  unsigned int *first_page_table = (unsigned int*)rmm_allocate_page();
  unsigned int *second_page_table = (unsigned int*)rmm_allocate_page();
  unsigned int *third_page_table = (unsigned int*)rmm_get_rmmpage();

  printf("pde = %p\npte0 = %p\npte1 = %p\n", page_directory, first_page_table, second_page_table);
  
  for (unsigned i = 0; i < 1024; i++)
    {
      page_directory[i] =
	PAGING_PDE_LEVEL_SUPERVISOR |
	PAGING_PDE_PRESENT_FALSE |
	PAGING_PDE_ACCESS_RW;
    }

  // holds the physical address where we want to start mapping these pages to.
  // in this case, we want to map these pages to the very beginning of memory.
  unsigned int address = 0;
  unsigned int i;
  
  //we will fill all 1024 entries, mapping 4 megabytes
  for(i = 0; i < 1024; i++)
    {
      first_page_table[i] = address |
	PAGING_PTE_LEVEL_SUPERVISOR |
	PAGING_PTE_ACCESS_RW |
	PAGING_PTE_PRESENT_TRUE |
	PAGING_PTE_GLOBAL_TRUE;
      address = address + 4096; //advance the address to the next page boundary
    }

  second_page_table[0] = (unsigned int)page_directory |
    PAGING_PTE_PRESENT_TRUE |
    PAGING_PTE_ACCESS_RW |
    PAGING_PTE_LEVEL_SUPERVISOR;

  second_page_table[1] = (unsigned int)first_page_table |
    PAGING_PTE_PRESENT_TRUE |
    PAGING_PTE_ACCESS_RW |
    PAGING_PTE_LEVEL_SUPERVISOR;
  
  second_page_table[2] = (unsigned int)second_page_table |
    PAGING_PTE_PRESENT_TRUE |
    PAGING_PTE_ACCESS_RW |
    PAGING_PTE_LEVEL_SUPERVISOR;
  
  for(i = 3; i < 1024; i++)
    {
      second_page_table[i] =
	PAGING_PTE_PRESENT_FALSE |
	PAGING_PTE_ACCESS_RW |
	PAGING_PTE_LEVEL_SUPERVISOR;
    }

  page_directory[0] = (unsigned int)first_page_table |
    PAGING_PDE_LEVEL_SUPERVISOR |
    PAGING_PDE_ACCESS_RW |
    PAGING_PDE_PRESENT_TRUE;

  page_directory[1] = (unsigned int)second_page_table |
    PAGING_PDE_LEVEL_SUPERVISOR |
    PAGING_PDE_ACCESS_RW |
    PAGING_PDE_PRESENT_TRUE;

  enable_paging(page_directory);
}
