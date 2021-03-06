/*
** Real memory manager
**
** This file provides a tool to manage the available real (i.e. physical) RAM
*/

#ifndef RMM_H_
#define RMM_H_

#include <libC.h>
#include <paging.h>
#include <multiboot.h>

/*
** Information about a 4kiB chunk, in physical memory
** This structure must not exceed 32 bits
*/
struct rmm_pageinfo {
  uint16_t		ref_count;
} __attribute__((packed));

/*
** Information about a chunk. A chunk is 1024 contiguous (4kiB) pages
**   1k * 4kiB = 4MiB
*/
struct rmm_chunkinfo {
  uint16_t		free_pages_count;
} __attribute__((packed));

/*
** Global information about the physical memory
** This structure must not exceed 4 MiB
*/
struct rmm_internal {
  struct rmm_pageinfo	page[1024*1024]; // 1Mi * 2B = 2MiB
  struct rmm_chunkinfo	chunk[1024]; // 1ki * 2B = 2kiB
} __attribute__((packed)); // total = 2050 kiB

// Initializes the RMM. This should be called once and only once, in a
// non-paginated environment, preferably at the very start of the
// kernel. It takes the multiboot_info as a parameter to identify free
// regions of the RAM, and returns the total available memory.
size_t	rmm_init(struct multiboot_info* mbi);

// Allocates a single physical page and returns its address.
// Caution : The page may not be accessible in the current paging context
// This function panic()'s if no memory is available
uintptr_t rmm_allocate_page(void);

// Allocates 1024 continuous pages in the same chunk.
// Caution : The pages may not be accessible in the current paging context
// This function panic()'s if no memory is available
// The pages can be reclaimed through individual calls to
// rmm_reclaim_page or a global call to rmm_reclaim_chunk
uintptr_t rmm_allocate_chunk(void);

// Gives back a physical page to the RMM. This takes any address in
// the page, but this may have to change some day.
void rmm_reclaim_page(uintptr_t page);

// Gives back a physical chunk to the RMM. This takes any address in
// the chunk, but this may have to change some day.
// This can only reclaim a chunk that is totally allocated, i.e. all
// pages it contains are referenced at least once.
// Trying to reclaim a not-totally-allocated chunk causes panic() to
// be called.
// This function decrements the ref_count for each individual page.
// If some pages in this chunk are shared, they won't count as free
// pages for the chunk.
void rmm_reclaim_chunk(uintptr_t chunk);

// Return the boundaries of the reserved space. Space between is
// allocated through RMM functions. Space out of these boundaries
// should always be identity paged.
uintptr_t rmm_min_physical_addr(void);
uintptr_t rmm_max_physical_addr(void);

#ifdef RMM_DEBUG
// Debug utility : dumps the memory map (as provided by Multiboot) to the screen
void dump_memory_map(struct multiboot_info* mbi);
#endif

#define CHUNK_SIZE (1024 * PAGE_SIZE)

#endif
