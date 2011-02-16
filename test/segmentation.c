#include <segmentation.h>
#include <panic.h>
#include <tss.h>

#define MAX_GDT_ENTRIES 6

static struct gdt_entry		gdte[MAX_GDT_ENTRIES];
static struct gdt_ptr		gdtptr;

uintptr_t	segment_null;
uintptr_t	segment_kernel_code;
uintptr_t	segment_kernel_stack;
uintptr_t	segment_user_code;
uintptr_t	segment_user_stack;

static uint32_t nb_gdt_entries = 0;

uintptr_t gdt_set_gate(uint32_t base,
		       uint32_t limit,
		       uint8_t access,
		       uint8_t gran) {

  if (nb_gdt_entries == MAX_GDT_ENTRIES)
    panic("No more space for GDT entries");
  gdte[nb_gdt_entries].base_low    = (base & 0xFFFF);
  gdte[nb_gdt_entries].base_middle = (base >> 16) & 0xFF;
  gdte[nb_gdt_entries].base_high   = (base >> 24) & 0xFF;
  
  gdte[nb_gdt_entries].limit_low   = (limit & 0xFFFF);
  gdte[nb_gdt_entries].granularity = (limit >> 16) & 0x0F;

  gdte[nb_gdt_entries].granularity |= gran & 0xF0;
  gdte[nb_gdt_entries].access      = access;

  uintptr_t ret = nb_gdt_entries << 3;
  nb_gdt_entries++;
  return ret;
}

void segmentation_init(void) {
  segment_null = gdt_set_gate(0, 0, 0, 0);                // Null segment
  segment_kernel_code = gdt_set_gate(0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
  segment_kernel_stack = gdt_set_gate(0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
  segment_user_code = gdt_set_gate(0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
  segment_user_stack = gdt_set_gate(0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment

  write_tss();
  
  do_gdt_flush();
  load_tss();
}

void do_gdt_flush(void) {
  gdtptr.limit = (sizeof(struct gdt_entry) * nb_gdt_entries) - 1;
  gdtptr.base  = (uint32_t)&gdte;
  
  gdt_flush(&gdtptr);
}
