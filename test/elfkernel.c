#include <elf.h>
#define HAS_UINTPTR_T

#include <elfkernel.h>
#include <tty.h>
#include <libC.h>
#include <panic.h>

static char* shtrtab = NULL;
static char* strtab = NULL;
static Elf32_Sym* symtab = NULL;
static unsigned int symcount = 0;
static uintptr_t text_begin = 0;
static uintptr_t text_end = 0;


void elf_init(void* vshdr, unsigned int num, unsigned int shstrndx) {
  Elf32_Shdr* shdr = vshdr;
  shtrtab = (char*)(shdr[shstrndx].sh_addr);

  for (unsigned int i = 0; i < num; i++) {
    if (!strcmp(shtrtab + shdr[i].sh_name, ".symtab")) {
      symtab = (Elf32_Sym*)(shdr[i].sh_addr);
      symcount = shdr[i].sh_size / sizeof(Elf32_Sym);
    } else if (!strcmp(shtrtab + shdr[i].sh_name, ".strtab")) {
      strtab = (char*)(shdr[i].sh_addr);
    } else if (!strcmp(shtrtab + shdr[i].sh_name, ".text")) {
      text_begin = shdr[i].sh_addr;
      text_end = shdr[i].sh_addr + shdr[i].sh_size;
    }
  }
  if (text_end == 0 || text_begin == 0 || strtab == NULL || symtab == NULL || symcount == 0)
    panic("Misformed ELF kernel");
}

char* elf_get_sym_name_before(uintptr_t eip, size_t* diff) {
  uintptr_t	best_value = 0;
  char*		best_string = "<unknown>";
  
  for (unsigned int i = 0; i < symcount; i++) {
    if (eip >= (uintptr_t)symtab[i].st_value && (uintptr_t)symtab[i].st_value >= text_begin && (uintptr_t)symtab[i].st_value < text_end) {
      if ((uintptr_t)symtab[i].st_value > best_value) {
	best_value = symtab[i].st_value;
	best_string = strtab + symtab[i].st_name;
      }
    }
  }
  if (diff)
    *diff = eip - best_value;
  return best_string;
}
