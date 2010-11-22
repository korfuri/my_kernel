#ifndef KMALLOC_H_
#define KMALLOC_H_

#include <libC.h>
#include <rmm.h>

#define KMALLOC_REQUIRED_SPACE CHUNK_SIZE

void kmalloc_init(uintptr_t base, size_t size);
void*	kmalloc(size_t s);
void*	kcalloc(size_t s);
void	kfree(void* addr);
void*	krealloc(void* addr, size_t s);

#endif
