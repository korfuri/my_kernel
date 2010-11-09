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

static inline struct rmm_pagetabinfo* rmm_get_pagetabinfo(intptr_t p) {
  intptr_t ptid = p / (1024 * 4096);

  if (rmm_info.pagetabs[ptid].pages == NULL)
    rmm_allocate_pagetabinfo(p, &(rmm_info.pagetabs[ptid]));
  return &(rmm_info.pagetabs[ptid]);
}

static inline struct rmm_pageinfo* rmm_get_pageinfo(intptr_t p) {
  struct rmm_pagetabinfo* pti = rmm_get_pagetabinfo(p);
  intptr_t pid = (p / 4096) % 1024;

  return &(rmm_info.pagetabs[pid].pages[pid]);
}

void	rmm_init(void) {
  intptr_t	p;
  intptr_t	min_physical_addr = rmm_min_physical_addr();
  
  memset(&rmm_info, '\0', sizeof(rmm_info));
  for (p = NULL; p < min_physical_addr; p += PAGE_SIZE) {
    rmm_get_pageinfo(p)->ref_count = 1; // Page busy
  }
}

static void rmm_allocate_pagetabinfo(intptr_t p, struct rmm_pagetabinfo* pti) {
  p &= 0xffc00000; // Rounds the pointer to the beginning of the 4MB chunk
  pti->pages = (struct rmm_pageinfo*)p; // Stores the metadata at the beginning of the chunk.
  memset(pti->pages, '\0', 1024 * sizeof(struct rmm_pageinfo)); // That's 2kB
  pti->pages[0].ref_count = 1; // This chunk is filled with metadata
  pti->free_pages_count = 1023;
}

intptr_t rmm_allocate_page(void) {
  for (unsigned int i = 0; i < 1024; i++) {
    if (rmm_info.pagetabs[i].free_pages_count > 0)
      return rmm_allocate_page_in_chunk(&(rmm_info.pagetabs[i]));
    else if (rmm_info.pagetabs[i].pages == NULL) {
      rmm_allocate_pagetabinfo(i * 4096 * 1024, &(rmm_info.pagetabs[i]));
      return rmm_allocate_page_in_chunk(&(rmm_info.pagetabs[i]));
    } else {
      panic("Out of physical memory");
      return NULL;
    }
  }
}
