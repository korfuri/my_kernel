#include <elf.h>
#define HAS_INTPTR_T

#include <tty.h>
#include <libC.h>

static char* shtrtab = NULL;
static char* strtab = NULL;
static Elf32_Sym* symtab = NULL;
static unsigned int symcount = 0;

void test_elf(Elf32_Shdr* shdr, unsigned int num, unsigned int shstrndx) {
  shtrtab = shdr[shstrndx].sh_addr;

  for (unsigned int i = 0; i < num; i++) {
    if (!strcmp(shtrtab + shdr[i].sh_name, ".symtab")) {
      symtab = shdr[i].sh_addr;
      symcount = shdr[i].sh_size / sizeof(Elf32_Sym);
    } else if (!strcmp(shtrtab + shdr[i].sh_name, ".strtab")) {
      strtab = shdr[i].sh_addr;
    }
    puts(shtrtab + shdr[i].sh_name);
  }
}

char* elf_get_sym_name_before(intptr_t eip, size_t* diff) {
  intptr_t	best_value = 0;
  char*		best_string = "<unknown>";
  
  for (unsigned int i = 0; i < symcount; i++) {
    if (eip >= symtab[i].st_value){// && (symtab[i].st_info & STT_FUNC)) {
      if (symtab[i].st_value > best_value) {
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
