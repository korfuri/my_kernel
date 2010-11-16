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
