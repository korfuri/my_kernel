#include <elf.h>
#define HAS_UINTPTR_T

#include <elfkernel.h>
#include <tty.h>
#include <libC.h>

static char* shtrtab = NULL;
static char* strtab = NULL;
static Elf32_Sym* symtab = NULL;
static unsigned int symcount = 0;

void elf_init(void* vshdr, unsigned int num, unsigned int shstrndx) {
  Elf32_Shdr* shdr = vshdr;
  shtrtab = (char*)(shdr[shstrndx].sh_addr);

  for (unsigned int i = 0; i < num; i++) {
    if (!strcmp(shtrtab + shdr[i].sh_name, ".symtab")) {
      symtab = (Elf32_Sym*)(shdr[i].sh_addr);
      symcount = shdr[i].sh_size / sizeof(Elf32_Sym);
    } else if (!strcmp(shtrtab + shdr[i].sh_name, ".strtab")) {
      strtab = (char*)(shdr[i].sh_addr);
    }
  }
}

char* elf_get_sym_name_before(uintptr_t eip, size_t* diff) {
  uintptr_t	best_value = 0;
  char*		best_string = "<unknown>";
  
  for (unsigned int i = 0; i < symcount; i++) {
    if (eip >= (uintptr_t)symtab[i].st_value) { // && (symtab[i].st_info & STT_FUNC)) {
      if ((uintptr_t)symtab[i].st_value > best_value) {
	best_value = symtab[i].st_value;
	if (symtab[i].st_name > 0)
	  best_string = strtab + symtab[i].st_name;
	else
	  best_string = "<unknown>";
      }
    }
  }
  if (diff)
    *diff = eip - best_value;
  return best_string;
}
