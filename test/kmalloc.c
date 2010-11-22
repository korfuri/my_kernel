#include <panic.h>
#include <kmalloc.h>

static uintptr_t kmalloc_base;
static size_t kmalloc_spacesize;

void kmalloc_init(uintptr_t base, size_t size) {
  kmalloc_base = base;
  kmalloc_spacesize = size;
}

void*	kmalloc(size_t s) {
  uintptr_t ret = kmalloc_base;
  kmalloc_base += s;
  return (void*)ret;
}

void*	kcalloc(size_t s) {
  return kmalloc(s);
}

void	kfree(void* addr) {
  addr = addr;
}

void*	krealloc(void* addr, size_t s) {
  panic("krealloc not implemented");
  addr += s;
  return NULL;
}

