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

static void enable_paging(uintptr_t physical_page_directory) {
  unsigned int cr0;

  // Sets cr3
  asm volatile("mov %0, %%cr3":: "b"(physical_page_directory));
  // Fetches cr0
  asm volatile("mov %%cr0, %0": "=b"(cr0));
  // Sets paging bit
  cr0 |= 0x80000000;
  // Updates cr0
  asm volatile("mov %0, %%cr0":: "b"(cr0));
}

uintptr_t virtual_to_physical(struct pager* pager, uintptr_t virtual_address) {
  uint32_t pageID = virtual_address / PAGE_SIZE;
  uint32_t chunkID = pageID / 1024;
  uint32_t page_in_chunk = pageID - (chunkID * 1024);

  // This can be hugely optimized with some bit hacks. gcc doesn't
  // seem to do it by itself.
  return ((pager->pt[chunkID][page_in_chunk] / PAGE_SIZE) * PAGE_SIZE) + (virtual_address % PAGE_SIZE);
}

void add_range_paging_protected(struct pager* pager,
				uintptr_t virtual_address,
				uintptr_t physical_address,
				size_t size) {
  uint32_t pageID = virtual_address / 4096;
  uint32_t chunkID = pageID / 1024;
  uint32_t page_in_chunk = pageID - (chunkID * 1024);

  while (size > 0) {
    // Enable the chunk's PDE for protected mapping
    pager->pd[chunkID] = virtual_to_physical(current_paging_context_virtual(), (uintptr_t)(pager->pt[chunkID])) |
      PAGING_PDE_DEFAULT_FLAGS |
      PAGING_PDE_PRESENT_TRUE |
      PAGING_PDE_LEVEL_SUPERVISOR |
      PAGING_PDE_ACCESS_RW;

    // Enable the page's PTE for protected mapping
    pager->pt[chunkID][page_in_chunk] = (physical_address) |
      PAGING_PTE_DEFAULT_FLAGS |
      PAGING_PTE_PRESENT_TRUE |
      PAGING_PTE_LEVEL_SUPERVISOR |
      PAGING_PTE_ACCESS_RW;

    // Advance to the next page
    pageID++;
    page_in_chunk++;
    if (page_in_chunk == 1024) {
      page_in_chunk = 0;
      chunkID++;
    }
    physical_address += PAGE_SIZE;
    size -= PAGE_SIZE;
  }
}

void add_identity_paging(struct pager* pager, paging_context context, uintptr_t address) {
  uint32_t pageID = address / 4096;
  uint32_t chunkID = pageID / 1024;
  uint32_t page_in_chunk = pageID - (chunkID * 1024);

  if (address > MAX_PAGED_ADDRESS)
    panic("address > MAX_PAGED_ADDRESS");

  // Enable the chunk's PDE for identity mapping
  pager->pd[chunkID] = ((uintptr_t)(pager->pt[chunkID]) - (uintptr_t)pager + context) |
    PAGING_PDE_DEFAULT_FLAGS |
    PAGING_PDE_PRESENT_TRUE |
    PAGING_PDE_LEVEL_SUPERVISOR |
    PAGING_PDE_ACCESS_RW;

  // Enable the page's PTE for identity mapping
  pager->pt[chunkID][page_in_chunk] = (pageID * 4096) |
    PAGING_PTE_DEFAULT_FLAGS |
    PAGING_PTE_PRESENT_TRUE |
    PAGING_PTE_LEVEL_SUPERVISOR |
    PAGING_PTE_ACCESS_RW;
}

void build_basic_paging(struct pager* pager, paging_context context) {
  uintptr_t min = rmm_min_physical_addr();
  uintptr_t max = rmm_max_physical_addr();

  // Initializes the PD and each PT
  for (uint32_t i = 0; i < 1024; i++) {
    pager->pd[i] = PAGING_PDE_PRESENT_FALSE | PAGING_PDE_DEFAULT_FLAGS;
    // There are only 1023 PT, even though there are 1024 entries in
    // the PD (see comments on the definition of struct pager to
    // understand why)
    if (i != 1023) {
      for (uint32_t j = 0; j < 1024; j++) {
	pager->pt[i][j] = PAGING_PTE_PRESENT_FALSE | PAGING_PTE_DEFAULT_FLAGS;
      }
    }
  }
  // These 2 loops force an identity mapping of, respectively :
  // - The protected addresses, including our kernel, the
  // hardware-reserved stuff, and RMM's metadata
  for (uintptr_t address = 0; address < min; address += PAGE_SIZE)
    add_identity_paging(pager, context, address);
  // - The top of the memory. This may include both protected memory,
  // and non-existing physical memory (we always map up to 4G, even
  // though we may not have that much memory available)
  for (uintptr_t address = max; address < MAX_PAGED_ADDRESS; address += PAGE_SIZE)
    add_identity_paging(pager, context, address);
}

paging_context init_paging(void) {
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

  // Sets up the mandatory identity paged chunks
  build_basic_paging(page_directory, (paging_context)page_directory);

  // Now as this is the first paging context we're building, we need
  // to setup identity paging for the pager itself, and the
  // alternative pager
  // - The pager we are building
  for (uintptr_t address = (uintptr_t)page_directory;
       address < ((uintptr_t)page_directory) + CHUNK_SIZE;
       address += PAGE_SIZE)
    add_identity_paging(page_directory, (paging_context)page_directory, address);
  // - The current alternative pager
  for (uintptr_t address = (uintptr_t)alternative_page_directory;
       address < ((uintptr_t)alternative_page_directory) + CHUNK_SIZE;
       address += PAGE_SIZE)
    add_identity_paging(page_directory, (paging_context)page_directory, address);

  // And start the paging. We should never come back from this.
  enable_paging((uintptr_t)page_directory);

  // As we are using identity mapping, we can return the virtual
  // address as a physical one
  return (paging_context)page_directory;
}

paging_context fork_paging_context(void) {
  // We are going to build a full paging context in
  // alternative_page_directory without leaving the current context,
  // then we will switch to the new context.

  // We are going to lose our page directory at some point. Let's save
  // its physical address
  uintptr_t physical_new_page_directory = virtual_to_physical(page_directory, (uintptr_t)alternative_page_directory);

  // Sets up the mandatory identity paged chunks
  build_basic_paging(alternative_page_directory, physical_new_page_directory);

  // Sets up the paging to have page_directory and
  // alternative_page_directory mapped to their corresponding physical
  // chunks in the new page directory
  add_range_paging_protected(alternative_page_directory, (uintptr_t)page_directory, virtual_to_physical(page_directory, (uintptr_t)alternative_page_directory), CHUNK_SIZE);
  add_range_paging_protected(alternative_page_directory, (uintptr_t)alternative_page_directory, rmm_allocate_chunk(), CHUNK_SIZE);

  // Let's replace our alternative page directory in the old context,
  // we don't want this old context to change it now
  add_range_paging_protected(page_directory, (uintptr_t)alternative_page_directory, rmm_allocate_chunk(), CHUNK_SIZE);

  // Applies the changes
  enable_paging(physical_new_page_directory);

  // This returns the new paging_context
  return physical_new_page_directory;
}

void destroy_current_paging_context(paging_context fallback) {
  paging_context paging_context_to_destroy = current_paging_context_physical();

  // We won't fork this paging context. Let's get rid of the
  // alternative_page_directory
  rmm_reclaim_chunk(virtual_to_physical(current_paging_context_virtual(), (uintptr_t)alternative_page_directory));

  // Now we can use the alternative_page_directory to map our
  // fallback context
  add_range_paging_protected(page_directory, (uintptr_t)alternative_page_directory, fallback, CHUNK_SIZE);
  // Force the flush of the TLB - suboptimal, we could use INVLPGs
  restore_paging_context(paging_context_to_destroy);

  // Now we are going to do something tricky. We will edit our
  // fallback context (through alternative_page_directory) to map the
  // current paging context in its own
  // alternative_page_directory. Then we will context switch to
  // fallback, from where we will be able to destroy the original
  // context.

  // First step : save the alternative_page_directory of our fallback
  // context
  paging_context save_alternative_pd = virtual_to_physical(alternative_page_directory, (uintptr_t)alternative_page_directory);

  // Now we can map the current context as the
  // alternative_page_directory of the fallback context
  // This overwrites the mapping of alternative_page_directory to
  // save_alternative_pd in this context
  add_range_paging_protected(alternative_page_directory, (uintptr_t)alternative_page_directory, paging_context_to_destroy, CHUNK_SIZE);

  // We can now context-switch to our fallback context
  restore_paging_context(fallback);

  // Now we can destroy all mappings from our paging_context_to_destroy
  // We don't need to touch the protected areas as the ref counting is
  // disabled in those

  // We can first cleanup all user allocated pages
  struct pager* pager = alternative_page_directory;
  // We iterate on each chunk of the mapping
  for (uint32_t chunkID = (rmm_min_physical_addr() / CHUNK_SIZE) + 2;
       chunkID < (rmm_max_physical_addr() / CHUNK_SIZE) && chunkID < 1023;
       chunkID++) {
    // If the chunk is present, we iterate on each page
    if (pager->pd[chunkID] & PAGING_PDE_PRESENT_TRUE) {
      for (uint32_t pageIDc = 0; pageIDc < 1024; pageIDc++) {
	if (pager->pt[chunkID][pageIDc] & PAGING_PTE_PRESENT_TRUE) {
	  // We only ask RMM to reclaim the page, there's no need to
	  // update the present bit - we are going to destroy this
	  // mapping right after
	  rmm_reclaim_page(pager->pt[chunkID][pageIDc]);
	}
      }
    }
  }
  
  // Finally we can reclaim the page directory itself as we don't need
  // it anymore
  rmm_reclaim_chunk(virtual_to_physical(alternative_page_directory, (uintptr_t)page_directory));

  // Now that we're done with the old page directory, we can reset our
  // alternative_page_directory to a blank one :
  add_range_paging_protected(page_directory, (uintptr_t)alternative_page_directory, save_alternative_pd, CHUNK_SIZE);
  // Force the flush of the TLB - suboptimal, we could use INVLPGs
  restore_paging_context(current_paging_context_physical());

  // And we're done.
}


void remove_page_from_paging_context(struct pager* pager,
				     uintptr_t virtual_address) {
  uint32_t pageID = virtual_address / PAGE_SIZE;
  uint32_t chunkID = pageID / 1024;
  uint32_t page_in_chunk = pageID - (chunkID * 1024);

  // Rounds the virtual address to PAGE_SIZE
  virtual_address /= 4096;
  virtual_address *= 4096;
  // Decrements the ref counter to this page
  rmm_reclaim_page(pager->pt[chunkID][page_in_chunk]);
  // Unmap this page
  pager->pt[chunkID][page_in_chunk] = PAGING_PTE_DEFAULT_FLAGS | PAGING_PTE_PRESENT_FALSE;
  // invlpg invalidates a page entry in the TLB
  asm volatile("invlpg %0":: "m"(*(uintptr_t*)virtual_address));
  // Look in this chunk for another mapped page
  for (uint32_t p = 0; p < 1024; p++)
    if (pager->pt[chunkID][p] & PAGING_PTE_PRESENT_TRUE)
      return;
  // If none was found, clear the whole PDE
  pager->pd[chunkID] = PAGING_PDE_DEFAULT_FLAGS | PAGING_PDE_PRESENT_FALSE;
}

void restore_paging_context(paging_context ctx) {
  // To restore a paging context we just need to apply it to CR3
  enable_paging(ctx);
}

struct pager* current_paging_context_virtual(void) {
  // This is a constant address
  return page_directory;
}

paging_context current_paging_context_physical(void) {
  struct pager* v = current_paging_context_virtual();

  return virtual_to_physical(v, (uintptr_t)v);
}
