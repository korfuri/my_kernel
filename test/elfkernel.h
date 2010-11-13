#ifndef ELFKERNEL_H_
#define ELFKERNEL_H_

#include <libC.h>

void elf_init(void* shdr, unsigned int num, unsigned int shstrndx);
char* elf_get_sym_name_before(uintptr_t eip, size_t* diff);

#endif
