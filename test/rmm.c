#include <rmm.h>
#include <libC.h>
#include <paging.h>
#include <panic.h>
#include <multiboot.h>
#include <tty.h>
#include <elf.h>
#include <kmalloc.h>

static struct rmm_internal* rmm_gl_metadata_addr = 0;
static uintptr_t rmm_gl_min_physical_addr = 0;
static uintptr_t rmm_gl_max_physical_addr = 0;

uintptr_t rmm_min_physical_addr(void) {
  return rmm_gl_min_physical_addr;
}

uintptr_t rmm_max_physical_addr(void) {
  return rmm_gl_max_physical_addr;
}

uintptr_t rmm_allocate_chunk(void) {
  for (uint32_t chunkID = 0; chunkID < 1024; chunkID++) {
    if (rmm_gl_metadata_addr->chunk[chunkID].free_pages_count == 1024) {
      for (uint32_t pageID = chunkID * 1024; pageID < (chunkID + 1) * 1024; pageID++) {
	rmm_gl_metadata_addr->page[pageID].ref_count = 1;
      }
      rmm_gl_metadata_addr->chunk[chunkID].free_pages_count = 0;
      return chunkID * 1024 * 4096;
    }
  }
  panic("No physical memory available");
  return 0;  
}

uintptr_t rmm_allocate_page(void) {
  for (uint32_t chunkID = 0; chunkID < 1024; chunkID++) {
    if (rmm_gl_metadata_addr->chunk[chunkID].free_pages_count > 0) {
      for (uint32_t pageID = chunkID * 1024; pageID < (chunkID + 1) * 1024; pageID++) {
	if (rmm_gl_metadata_addr->page[pageID].ref_count == 0) {
	  rmm_gl_metadata_addr->page[pageID].ref_count++;
	  rmm_gl_metadata_addr->chunk[chunkID].free_pages_count--;
	  return pageID * 4096;
	}
      }
      panic("Chunk has free pages but all pages are referenced");
    }
  }
  panic("No physical memory available");
  return 0;
}

void rmm_reclaim_page(uintptr_t page) {
  uint32_t pageID = page / 4096;

  rmm_gl_metadata_addr->page[pageID].ref_count--;
  if (rmm_gl_metadata_addr->page[pageID].ref_count == 0) {
    rmm_gl_metadata_addr->chunk[pageID / 1024].free_pages_count++;
  }
}

void rmm_reclaim_chunk(uintptr_t chunk) {
  uint32_t chunkID = chunk / (1024 * 4096);

  if (rmm_gl_metadata_addr->chunk[chunkID].free_pages_count != 0)
    panic("rmm_reclaim_chunk called on a non-totally-free chunk");
  for (uint32_t pageID = chunkID * 1024; pageID < (chunkID + 1) * 1024; pageID++) {
    rmm_gl_metadata_addr->page[pageID].ref_count--;
    if (rmm_gl_metadata_addr->page[pageID].ref_count == 0)
      rmm_gl_metadata_addr->chunk[chunkID].free_pages_count++;
  }
}

size_t	rmm_init(struct multiboot_info* mbi) {
  uint64_t	max_contiguous_size = 0;
  multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)mbi->mmap_addr;
  
  // Sanity checks
  if (sizeof(struct rmm_internal) > 4096*1024)
    panic("struct rmm_internal is larger than a chunk");
  if (sizeof(struct rmm_pageinfo) > 4)
    panic("struct rmm_pageinfo is larger than 32 bits");
  
  // Scans the memory map given by Multiboot to find the largest
  // continuous chunk. We will use this chunk, and only this chunk,
  // for allocation.
  while((uint32_t)mmap < mbi->mmap_addr + mbi->mmap_length) {
    if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
      if (mmap->len > max_contiguous_size) {
	max_contiguous_size = mmap->len;
	rmm_gl_max_physical_addr = mmap->addr + mmap->len;
	rmm_gl_min_physical_addr = mmap->addr;
      }
    }
    mmap = (multiboot_memory_map_t*)((unsigned int)mmap + mmap->size + sizeof(unsigned int));
  }
  if (max_contiguous_size == 0)
    panic("No available physical memory according to Multiboot");

  // Scans the ELF sections to remove our kernel's memory from the
  // available pool (this also includes the kernel's stack)
  Elf32_Shdr* shdr = (Elf32_Shdr*)mbi->u.elf_sec.addr;
  unsigned int num = mbi->u.elf_sec.num;
  for (unsigned int i = 0; i < num; i++) {
    // Case 1 : the section overlaps only the beginning of our memory area
    if (shdr[i].sh_addr <= rmm_gl_min_physical_addr &&
	shdr[i].sh_addr + shdr[i].sh_size > rmm_gl_min_physical_addr)
      rmm_gl_min_physical_addr = shdr[i].sh_addr + shdr[i].sh_size;
    // Case 2 : the section overlaps only the end of our memory area
    else if (shdr[i].sh_addr < rmm_gl_max_physical_addr &&
	     shdr[i].sh_addr + shdr[i].sh_size >= rmm_gl_max_physical_addr)
      rmm_gl_max_physical_addr = shdr[i].sh_addr;
    // Case 3 : the section is totally contained in our memory area
    else if (shdr[i].sh_addr >= rmm_gl_min_physical_addr &&
	     shdr[i].sh_addr + shdr[i].sh_size <= rmm_gl_max_physical_addr) {
      // Case 3a : we have more space AFTER the section
      if (rmm_gl_max_physical_addr - (shdr[i].sh_addr + shdr[i].sh_size) >
	  shdr[i].sh_addr - rmm_gl_min_physical_addr)
	rmm_gl_min_physical_addr = shdr[i].sh_addr + shdr[i].sh_size;
      // Case 3b we habe more space BEFORE the section
      else
	rmm_gl_max_physical_addr = shdr[i].sh_addr;
    }
  }

  // Rounds the boundaries of the available memory to CHUNK_SIZE (4M)
  if (rmm_gl_min_physical_addr & (~0xffc00000))
    rmm_gl_min_physical_addr = (rmm_gl_min_physical_addr + CHUNK_SIZE) & 0xffc00000;
  rmm_gl_max_physical_addr = rmm_gl_max_physical_addr & 0xffc00000;

  // Puts the metadata for RMM at the beginning of the available memory
  rmm_gl_metadata_addr = (struct rmm_internal*)rmm_gl_min_physical_addr;
  rmm_gl_min_physical_addr += CHUNK_SIZE;

  // Puts the kernel kmalloc()'d data after RMM's data
  size_t kmalloc_size = KMALLOC_REQUIRED_SPACE;
  // kmalloc_size must be aligned on CHUNK_SIZE
  if (kmalloc_size & (~0xffc00000))
    kmalloc_size = (kmalloc_size + CHUNK_SIZE) & 0xffc00000;
  kmalloc_size = kmalloc_size & 0xffc00000;  
  kmalloc_init(rmm_gl_min_physical_addr, kmalloc_size);
  rmm_gl_min_physical_addr += kmalloc_size;
  
  // Sanity check - do we still have some memory left ?
  if (rmm_gl_max_physical_addr <= rmm_gl_min_physical_addr)
    panic("No free physical memory");
  
  // Now we can fill in the metadata
  // First we set the whole structure to 0, just in case
  memset(rmm_gl_metadata_addr, '\0', sizeof(struct rmm_internal));
  // Then we set each chunk as having 1024 free pages
  for (uint32_t chunkID = 0; chunkID < 1024; chunkID++)
    rmm_gl_metadata_addr->chunk[chunkID].free_pages_count = 1024;
  // Finally, we iterate on each page
  for (uint32_t pageID = 0; pageID < 1024*1024; pageID++) {
    // Mark all the protected pages as referenced even though no
    // paging context use them
    if (pageID * PAGE_SIZE < rmm_gl_min_physical_addr) {
      rmm_gl_metadata_addr->page[pageID].ref_count = 1;
      // We also mark the associated chunk as having one less free page
      rmm_gl_metadata_addr->chunk[pageID / 1024].free_pages_count--;
    }
    if (pageID * PAGE_SIZE >= rmm_gl_max_physical_addr) {
      rmm_gl_metadata_addr->page[pageID].ref_count = 1;
      // We also mark the associated chunk as having one less free page
      rmm_gl_metadata_addr->chunk[pageID / 1024].free_pages_count--;
    }
  }
  return rmm_gl_max_physical_addr - rmm_gl_min_physical_addr;
}







#ifdef RMM_DEBUG
void dump_memory_map(struct multiboot_info* mbi) {
  multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)mbi->mmap_addr;
  
  printf("==== BEGIN MEMORY MAP ====\n");
  while((uint32_t)mmap < mbi->mmap_addr + mbi->mmap_length) {
    printf("%(uint64_t)p : length=%(uint64_t)x, type=%s, size=%x\n",
	   mmap->addr,
	   mmap->len,
	   ((mmap->type == MULTIBOOT_MEMORY_AVAILABLE) ?
	    ("available") :
	    ((mmap->type == MULTIBOOT_MEMORY_RESERVED) ?
	     ("reserved") :
	     ("other"))),
	   mmap->size);
    mmap = (multiboot_memory_map_t*)((unsigned int)mmap + mmap->size + sizeof(unsigned int));
  }
  printf("==== END MEMORY MAP ====\n");
}
#endif
