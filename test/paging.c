#include <paging.h>
#include <rmm.h>
#include <tty.h>
#include <panic.h>

static struct pager* page_directory = 0;
static struct pager* alternative_page_directory = 0;

int is_paging_enabled(void) {
  unsigned int cr0;

  // Fetches cr0
  asm volatile("mov %%cr0, %0": "=b"(cr0));
  // Tests paging bit
  return !(cr0 & 0x80000000);
}

static void enable_paging(uintptr_t page_directory) {
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

void add_identity_paging(struct pager* pager, uintptr_t address) {
  uint32_t pageID = address / 4096;
  uint32_t chunkID = pageID / 1024;
  uint32_t page_in_chunk = pageID - (chunkID * 1024);

  pager->pd[chunkID] = ((uint32_t)(pager->pt[chunkID])) |
    PAGING_PDE_PRESENT_TRUE |
    PAGING_PDE_LEVEL_SUPERVISOR |
    PAGING_PDE_ACCESS_RW;
  pager->pt[chunkID][page_in_chunk] = (pageID * 4096) |
    PAGING_PTE_PRESENT_TRUE |
    PAGING_PTE_LEVEL_SUPERVISOR |
    PAGING_PTE_ACCESS_RW;
}

void build_basic_paging(struct pager* pager) {
  uintptr_t min = rmm_min_physical_addr();
  uintptr_t max = rmm_max_physical_addr();

  // Inits the PD and each PT
  for (uint32_t i = 0; i < 1024; i++) {
    pager->pd[i] = PAGING_PDE_PRESENT_FALSE;
    if (i != 1024) {
      for (uint32_t j = 0; j < 1024; j++) {
	pager->pt[i][j] = PAGING_PTE_PRESENT_FALSE;
      }
    }
  }

  for (uintptr_t address = 0; address < min; address += PAGE_SIZE)
    add_identity_paging(pager, address);
  printf("Mapped %p to %p\n", 0, min);
  for (uintptr_t address = max; address < MAX_PAGED_ADDRESS; address += PAGE_SIZE)
    add_identity_paging(pager, address);
  printf("Mapped %p to %p\n", max, MAX_PAGED_ADDRESS);
  for (uintptr_t address = (uintptr_t)page_directory;
       address < ((uintptr_t)page_directory) + CHUNK_SIZE;
       address += PAGE_SIZE)
    add_identity_paging(pager, address);
  printf("Mapped %p to %p\n", (uintptr_t)page_directory, ((uintptr_t)page_directory) + CHUNK_SIZE);
  for (uintptr_t address = (uintptr_t)alternative_page_directory;
       address < ((uintptr_t)alternative_page_directory) + CHUNK_SIZE;
       address += PAGE_SIZE)
    add_identity_paging(pager, address);
  printf("Mapped %p to %p\n", (uintptr_t)alternative_page_directory, ((uintptr_t)alternative_page_directory) + CHUNK_SIZE);

  printf("Test. Address = 0x12345. ChunkID = %d, PageID = %d\n", 0x12345 / (1024 * 4096), 0x12345 / 4096);
  printf("PDE : %x\n", pager->pd[0x12345 / (1024 * 4096)]);
  printf("PTE : %x\n", pager->pt[0x12345 / (1024 * 4096)][0x12345 / 4096]);

  printf("Something on the stack : %p\n", &min);
  
  printf("Finished building a basic pagination\n");
  //  panic("test");
}

void init_paging(void) {
  // Sanity check
  if (sizeof(struct pager) != 1024 * 4096)
    panic("sizeof(struct pager) is not equal to one chunk");

  // This has two purposes : this creates the two first pagers, and
  // this sets the fixed addresses that will be used by every pagers
  // in every context.
  // The first pagers will be identity-mapped : this will not be the
  // case later.
  page_directory = (struct pager*)rmm_allocate_chunk();
  alternative_page_directory = (struct pager*)rmm_allocate_chunk();

  build_basic_paging(page_directory);
  enable_paging((uintptr_t)page_directory);
}

/* void set_first_pages(void) { */
/*   unsigned int *page_directory = (unsigned int*)rmm_allocate_page(); */
/*   unsigned int *first_page_table = (unsigned int*)rmm_allocate_page(); */
/*   unsigned int *second_page_table = (unsigned int*)rmm_allocate_page(); */

/*   printf("pde = %p\npte0 = %p\npte1 = %p\n", page_directory, first_page_table, second_page_table); */

/*   for (unsigned i = 0; i < 1024; i++) */
/*     { */
/*       page_directory[i] = */
/* 	PAGING_PDE_LEVEL_SUPERVISOR | */
/* 	PAGING_PDE_PRESENT_FALSE | */
/* 	PAGING_PDE_ACCESS_RW; */
/*     } */

/*   // holds the physical address where we want to start mapping these pages to. */
/*   // in this case, we want to map these pages to the very beginning of memory. */
/*   unsigned int address = 0; */
/*   unsigned int i; */

/*   //we will fill all 1024 entries, mapping 4 megabytes */
/*   for(i = 0; i < 1024; i++) */
/*     { */
/*       first_page_table[i] = address | */
/* 	PAGING_PTE_LEVEL_SUPERVISOR | */
/* 	PAGING_PTE_ACCESS_RW | */
/* 	PAGING_PTE_PRESENT_TRUE | */
/* 	PAGING_PTE_GLOBAL_TRUE; */
/*       address = address + 4096; //advance the address to the next page boundary */
/*     } */

/*   second_page_table[0] = (unsigned int)page_directory | */
/*     PAGING_PTE_PRESENT_TRUE | */
/*     PAGING_PTE_ACCESS_RW | */
/*     PAGING_PTE_LEVEL_SUPERVISOR; */

/*   second_page_table[1] = (unsigned int)first_page_table | */
/*     PAGING_PTE_PRESENT_TRUE | */
/*     PAGING_PTE_ACCESS_RW | */
/*     PAGING_PTE_LEVEL_SUPERVISOR; */

/*   second_page_table[2] = (unsigned int)second_page_table | */
/*     PAGING_PTE_PRESENT_TRUE | */
/*     PAGING_PTE_ACCESS_RW | */
/*     PAGING_PTE_LEVEL_SUPERVISOR; */

/*   for(i = 3; i < 1024; i++) */
/*     { */
/*       second_page_table[i] = */
/* 	PAGING_PTE_PRESENT_FALSE | */
/* 	PAGING_PTE_ACCESS_RW | */
/* 	PAGING_PTE_LEVEL_SUPERVISOR; */
/*     } */

/*   page_directory[0] = (unsigned int)first_page_table | */
/*     PAGING_PDE_LEVEL_SUPERVISOR | */
/*     PAGING_PDE_ACCESS_RW | */
/*     PAGING_PDE_PRESENT_TRUE; */

/*   page_directory[1] = (unsigned int)second_page_table | */
/*     PAGING_PDE_LEVEL_SUPERVISOR | */
/*     PAGING_PDE_ACCESS_RW | */
/*     PAGING_PDE_PRESENT_TRUE; */

/*   enable_paging(page_directory); */
/* } */
