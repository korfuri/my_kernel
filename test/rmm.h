/*
** Real memory manager
**
** This file provides a tool to manage the available real (i.e. physical) RAM
*/

#ifndef RMM_H_
#define RMM_H_

#include <libC.h>
#include <paging.h> // For PAGE_SIZE
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

size_t	rmm_init(struct multiboot_info* mbi);
uintptr_t rmm_allocate_page(void);
void dump_memory_map(struct multiboot_info* mbi);

#define CHUNK_SIZE (1024 * PAGE_SIZE)

#endif
