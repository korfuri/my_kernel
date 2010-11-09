#include <rmm.h>
#include <libC.h>
#include <paging.h> // for PAGE_SIZE
#include <panic.h>

static struct rmm_internal rmm_info;

// Below this address, physical memory is reserved : 0xa0000 - 0xfffff
// is hardware-specific, below is kernel-reserved
static inline intptr_t rmm_min_physical_addr(void) {
  return 0x1000000;
}

static inline intptr_t rmm_max_physical_addr(void) {
  return 0xffffffff;
}

static void rmm_allocate_pagetabinfo(intptr_t p, struct rmm_pagetabinfo* pti) {
  p &= 0xffc00000; // Rounds the pointer to the beginning of the 4MB chunk
  pti->pages = (struct rmm_pageinfo*)p; // Stores the metadata at the beginning of the chunk.
  if (p >= rmm_min_physical_addr()) { // Don't add metadata in critical first pages
    memset(pti->pages, '\0', 1024 * sizeof(struct rmm_pageinfo)); // That's 2kB
    pti->pages[0].ref_count = 1; // This chunk is filled with metadata
  }
  pti->free_pages_count = 1023;
}

static inline struct rmm_pagetabinfo* rmm_get_pagetabinfo(intptr_t p) {
  intptr_t ptid = p / (1024 * 4096);

  if (rmm_info.pagetabs[ptid].pages == NULL)
    rmm_allocate_pagetabinfo(p, &(rmm_info.pagetabs[ptid]));
  return &(rmm_info.pagetabs[ptid]);
}

static inline struct rmm_pageinfo* rmm_get_pageinfo(intptr_t p) {
  struct rmm_pagetabinfo* pti = rmm_get_pagetabinfo(p);
  intptr_t pid = (p / 4096) % 1024;

  return &(pti->pages[pid]);
}

void	rmm_init(void) {
  intptr_t	p;
  intptr_t	min_physical_addr = rmm_min_physical_addr();
  
  memset(&rmm_info, '\0', sizeof(rmm_info));
  for (p = 0; p < min_physical_addr; p += PAGE_SIZE) {
    if (rmm_get_pageinfo(p)->ref_count == 0)
      rmm_get_pagetabinfo(p)->free_pages_count--;
    rmm_get_pageinfo(p)->ref_count++; // Page busy
  }
}

static intptr_t rmm_allocate_page_in_chunk(unsigned int chunkID) {
  struct rmm_pagetabinfo* pti = &(rmm_info.pagetabs[chunkID]);

  for (unsigned int i = 1; i < 1024; i++) { // i = 1 : We can always
    // skip the first page anyway because it's filled with metadata   
    if (pti->pages[i].ref_count == 0) {
      pti->pages[i].ref_count = 1;
      pti->free_pages_count--;
      return (chunkID * 1024 + i) * 4096;
    }
  }
  return 0;
}

intptr_t rmm_allocate_page(void) {
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
