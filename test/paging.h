#ifndef PAGING_H_
#define PAGING_H_

#include <libC.h>

// Both a page table and a page directory are contiguous arrays of
// 1024 32-bit fields.
// These fields are represented as int32_t because they contain a
// (rounded) pointer to physical memory, and some flags in the low
// bits.
typedef volatile uint32_t page_table_t[1024];
typedef volatile uint32_t page_directory_t[1024];

// This is the type we expose to other modules. This should mostly be
// helpful to the scheduler, which can use is to restore a paging
// context when giving the control flow to a thread.
// This type represents a PHYSICAL address of a struct pager (which is
// also the address of its associated page directory)
// Applying it is as simple as putting it back in cr3.
typedef uintptr_t paging_context;

// This structure aggregates a page directory and 1023 page
// tables. This means at least one virtual chunk can't be allocated
// (because there are actually 1024 chunks) : we use the last chunk
// for this purpose. This means you cannot allocate virtual addresses
// in the 0xfff0 0000 - 0xffff ffff range.
// This structure MUST have a 4 MiB size exactly. (= 1 chunk)
struct pager {
  volatile page_directory_t pd; // 1 * 4 kiB = 4 kiB
  volatile page_table_t pt[1023]; // 1023 * 4 kiB = 4092 kiB
} __attribute__((packed)); // total = 4092 + 4 kiB = 4096 kiB = 4 MiB

// Returns 0 if paging is enabled, 1 otherwise. Once paging has been
// enabled, it is not a good idea to have it disabled.
int is_paging_enabled(void);

// This creates the first paging contexts, mapping only the bare
// necessary (i.e. identity paging what the kernel needs).
// This should be called once and only once, after rmm has been
// initialized.
// This enables the paging before returning. We should never leave the
// paged mode afterwards.
// This returns the paging context that was applied.
paging_context init_paging(void);

// This creates a new paging context, and applies it. It then returns
// the new paging context.
// The newly created paging context is empty, except for the basics :
// identity paging of protected areas, plus a paging for the struct
// pager and the alternative struct pager.
paging_context fork_paging_context(void);

// Destroys the current paging context, and switches back to the given
// paging_context
// All physical pages between rmm_min_physical_addr and
// rmm_max_physical_addr will see their ref_count decremented
void destroy_current_paging_context(paging_context fallback);

// This function returns the physical address associated to a given
// virtual address in the given struct pager.
// The address doesn't need to be rounded to PAGE_SIZE, and the
// returned address preserves the offset in the page.
uintptr_t virtual_to_physical(struct pager* pager, uintptr_t virtual_address);

// This function adds a new paging range to the given struct pager. If
// the struct pager is the one currently applied, the changes take
// effect immediately.
// This function doesn't check if it is overwriting previous mappings.
void add_range_paging_protected(struct pager* pager,
				uintptr_t virtual_address,
				uintptr_t physical_address,
				size_t size);

// Remove a user-allocated page from a paging context, and have rmm
// reclaim it
void remove_page_from_paging_context(struct pager* pager,
				     uintptr_t virtual_address);

// This returns the currently used struct pager (virtual address)
// This is a constant value.
struct pager* current_paging_context_virtual(void);


// This return the currently applied paging_context (physical address)
paging_context current_paging_context_physical(void);


// This adds an identity paging for the given page (in address). This
// requires both the struct pager and its associated paging_context
// because this function is used during the creating of the context,
// when the PD is not yet mapped in itself. The pager and the context
// MUST correspond, and no checks are made (nor can they be).
void add_identity_paging(struct pager* pager, paging_context context, uintptr_t address);

// This restores a paging_context. This can be used by the scheduler
// to switch to a thread's paging_context before handing it the
// control flow back.
void restore_paging_context(paging_context ctx);

#define PAGE_SIZE 0x1000

// Due to technical limitations of the struct pager, the last chunk
// can't be mapped. This is the highest mappable address.
#define MAX_PAGED_ADDRESS (0xffffffff - CHUNK_SIZE)




//
// The macros below are directly related to the signification of the
// control bits of PDE/PTE. See Intel IA-32 manual to know more about
// the significance of each bit.
//

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


#define PAGING_PTE_DEFAULT_FLAGS (PAGING_PTE_PRESENT_FALSE | PAGING_PTE_CACHE_DISABLE)
#define PAGING_PDE_DEFAULT_FLAGS (PAGING_PDE_PRESENT_FALSE | PAGING_PDE_CACHE_DISABLE)

#endif
