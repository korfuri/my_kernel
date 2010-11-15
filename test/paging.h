#ifndef PAGING_H_
#define PAGING_H_

#include <libC.h>

// Both a page table and a page directory are contiguous arrays of
// 1024 32-bit fields.
// These fields are represented as int32_t because they contain a
// (rounded) pointer to physical memory, and some flags in the low
// bits.
typedef uint32_t page_table_t[1024];
typedef uint32_t page_directory_t[1024];

// This structure aggregates a page directory and 1023 page
// tables. This means at least one virtual chunk can't be allocated
// (because there are actually 1024 chunks) : we use the last chunk
// for this purpose. This means you cannot allocate virtual addresses
// in the 0xfff0 0000 - 0xffff ffff range.
// This structure MUST have a 4 MiB size exactly. (= 1 chunk)
struct pager {
  page_directory_t pd; // 1 * 4 kiB = 4 kiB
  page_table_t pt[1023]; // 1023 * 4 kiB = 4092 kiB
} __attribute__((packed)); // total = 4092 + 4 kiB = 4096 kiB = 4 MiB

struct page_directory {
  unsigned int* page_tables[1024];
} __attribute__((deprecated));

// Returns 0 if paging is enabled, 1 otherwise. Once paging has been
// enabled, it is not a good idea to have it disabled.
int is_paging_enabled(void);

void init_paging(void);

#define PAGE_SIZE 0x1000
#define MAX_PAGED_ADDRESS (0xffffffff - CHUNK_SIZE + 1)

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
