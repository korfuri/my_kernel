/*
** Real memory manager
**
** This file provides a tool to manage the available real (i.e. physical) RAM
*/

#ifndef RMM_H_
#define RMM_H_

#include <libC.h>
#include <paging.h> // For PAGE_SIZE

/*
** Information about a 4k chunk, in physical memory
*/
struct rmm_pageinfo {
  uint16_t		ref_count;
};

/*
** Information about 1024 contiguous 4k chunks
**   1k * 4k = 4M
*/
struct rmm_pagetabinfo {
  struct rmm_pageinfo	*pages;
  uint16_t		free_pages_count;
};

/*
** Global information about the physical memory
*/
struct rmm_internal {
  struct rmm_pagetabinfo	pagetabs[1024]; // TODO(korfuri): Should this be fixed ? Maybe use the RAM size from multiboot instead ?
};

void	rmm_init(void);
uintptr_t rmm_allocate_page(void);

#define CHUNK_SIZE (1024 * PAGE_SIZE)

#endif
