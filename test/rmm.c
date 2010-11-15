#include <rmm.h>
#include <libC.h>
#include <paging.h>
#include <panic.h>
#include <multiboot.h>
#include <tty.h>

static struct rmm_internal rmm_info;

// Below this address, physical memory is reserved : 0x1000 0000 - 0xffff ffff
// is hardware-specific, below is kernel-reserved
static inline uintptr_t rmm_min_physical_addr(void) {
  return 0x1000000;
}

static inline uintptr_t rmm_max_physical_addr(void) {
  return 0xffffffff;
}

static void rmm_allocate_pagetabinfo(uintptr_t p, struct rmm_pagetabinfo* pti) {
  p &= 0xffc00000; // Rounds the pointer to the beginning of the 4MB chunk
  pti->pages = (struct rmm_pageinfo*)p; // Stores the metadata at the beginning of the chunk.
  if (p >= rmm_min_physical_addr()) { // Don't add metadata in critical first pages
    memset(pti->pages, '\0', 1024 * sizeof(struct rmm_pageinfo)); // That's 2kB
    pti->pages[0].ref_count = 1; // This chunk is filled with metadata
  }
  pti->free_pages_count = 1023;
}

static inline struct rmm_pagetabinfo* rmm_get_pagetabinfo(uintptr_t p) {
  uintptr_t ptid = p / (1024 * 4096);

  if (rmm_info.pagetabs[ptid].pages == NULL)
    rmm_allocate_pagetabinfo(p, &(rmm_info.pagetabs[ptid]));
  return &(rmm_info.pagetabs[ptid]);
}

static inline struct rmm_pageinfo* rmm_get_pageinfo(uintptr_t p) {
  struct rmm_pagetabinfo* pti = rmm_get_pagetabinfo(p);
  uintptr_t pid = (p / 4096) % 1024;

  return &(pti->pages[pid]);
}

void	rmm_init(void) {
  uintptr_t	p;
  uintptr_t	min_physical_addr = rmm_min_physical_addr();
  
  memset(&rmm_info, '\0', sizeof(rmm_info));
  for (p = 0; p < min_physical_addr; p += PAGE_SIZE) {
    if (rmm_get_pageinfo(p)->ref_count == 0)
      rmm_get_pagetabinfo(p)->free_pages_count--;
    rmm_get_pageinfo(p)->ref_count++; // Page busy
  }
}

static uintptr_t rmm_allocate_page_in_chunk(unsigned int chunkID) {
  struct rmm_pagetabinfo* pti = &(rmm_info.pagetabs[chunkID]);

  for (unsigned int i = 1; i < 1024; i++) { // i = 1 : We can always
    // skip the first page anyway because it's filled with metadata   
    if (pti->pages[i].ref_count == 0) {
      pti->pages[i].ref_count = 1;
      pti->free_pages_count--;
      return (chunkID * 1024 + i) * 4096;
    }
  }
  panic("No free page in allocate_page_in_chunk when pti->free_page_count was > 0");
  return 0;
}

uintptr_t rmm_allocate_page(void) {
  for (unsigned int i = rmm_min_physical_addr() / CHUNK_SIZE; i < 1024; i++) {
    if (rmm_info.pagetabs[i].free_pages_count > 0) {
      return rmm_allocate_page_in_chunk(i);
    }
    else if (rmm_info.pagetabs[i].pages == NULL) {
      rmm_allocate_pagetabinfo(i * 4096 * 1024, &(rmm_info.pagetabs[i]));
      return rmm_allocate_page_in_chunk(i);
    }
  }
  panic("Out of physical memory");
  return 0;
}

unsigned int* rmm_get_rmmpage(void) {
  static unsigned int* p = 0;
  if (p)
    return p;
  p = (unsigned int*)rmm_allocate_page();
  for (unsigned int i = 0; i < 1024; i++)
    p[i] = i * 4096 | PAGING_PTE_ACCESS_RW | PAGING_PTE_LEVEL_SUPERVISOR | PAGING_PTE_PRESENT_TRUE;
  return p;
}

void dump_memory_map(struct multiboot_info* mbi) {
  multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)mbi->mmap_addr;
  
  printf("==== BEGIN MEMORY MAP ====\n");
  while((uint32_t)mmap < mbi->mmap_addr + mbi->mmap_length) {
    printf("%(uint64_t)p : length=%(uint64_t)x, type=%s, size=%x\n",
	   mmap->addr,
	   mmap->len,
	   ((mmap->type == MULTIBOOT_MEMORY_AVAILABLE) ?
	    ("available") :
	    ((mmap->type == MULTIBOOT_MEMORY_RESERVED) ?
	     ("reserved") :
	     ("other"))),
	   mmap->size);
    mmap = (multiboot_memory_map_t*)((unsigned int)mmap + mmap->size + sizeof(unsigned int));
  }
  printf("==== END MEMORY MAP ====\n");
}
