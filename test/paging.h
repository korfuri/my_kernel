#ifndef PAGING_H_
#define PAGING_H_

struct page_directory {
  unsigned int* page_tables[1024];
};

// Returns 0 if paging is enabled, 1 otherwise
int is_paging_enabled(void);

// Applies the given page_directory and enables paging if needed
void page_directory_apply(struct page_directory* pd);

#define PAGE_SIZE 0x1000

// Page directory entries control bits

#define PAGING_PDE_PAGESIZE_4K 0
#define PAGING_PDE_PAGESIZE_4M (1 << 7)

// Bit 6 is ignored

#define PAGING_PDE_ACCESSED_FALSE 0
#define PAGING_PDE_ACCESSED_TRUE (1 << 5)

#define PAGING_PDE_CACHE_ENABLE 0
#define PAGING_PDE_CACHE_DISABLE (1 << 4)

#define PAGING_PDE_WRITETHROUGH_DISABLE 0
#define PAGING_PDE_WRITETHROUGH_ENABLE (1 << 3)

#define PAGING_PDE_LEVEL_USER (1 << 2)
#define PAGING_PDE_LEVEL_SUPERVISOR 0

#define PAGING_PDE_ACCESS_RW (1 << 1)
#define PAGING_PDE_ACCESS_RO 0

#define PAGING_PDE_PRESENT_TRUE 1
#define PAGING_PDE_PRESENT_FALSE 0



// Page table entries control bits

#define PAGING_PTE_GLOBAL_FALSE 0
#define PAGING_PTE_GLOBAL_TRUE (1 << 8)

// bit 7 is ignored

#define PAGING_PTE_DIRTY_FALSE 0
#define PAGING_PTE_DIRTY_TRUE (1 << 6)

#define PAGING_PTE_ACCESSED_FALSE 0
#define PAGING_PTE_ACCESSED_TRUE (1 << 5)

#define PAGING_PTE_CACHE_ENABLE 0
#define PAGING_PTE_CACHE_DISABLE (1 << 4)

#define PAGING_PTE_WRITETHROUGH_DISABLE 0
#define PAGING_PTE_WRITETHROUGH_ENABLE (1 << 3)

#define PAGING_PTE_LEVEL_USER (1 << 2)
#define PAGING_PTE_LEVEL_SUPERVISOR 0

#define PAGING_PTE_ACCESS_RW (1 << 1)
#define PAGING_PTE_ACCESS_RO 0

#define PAGING_PTE_PRESENT_TRUE 1
#define PAGING_PTE_PRESENT_FALSE 0

#endif
