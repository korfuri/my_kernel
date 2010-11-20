#include <segmentation.h>

static struct gdt_entry		gdte[5];
static struct gdt_ptr		gdtptr;

static void gdt_set_gate(uint32_t num,
			 uint32_t base,
			 uint32_t limit,
			 uint8_t access,
			 uint8_t gran) {
  gdte[num].base_low    = (base & 0xFFFF);
  gdte[num].base_middle = (base >> 16) & 0xFF;
  gdte[num].base_high   = (base >> 24) & 0xFF;
  
  gdte[num].limit_low   = (limit & 0xFFFF);
  gdte[num].granularity = (limit >> 16) & 0x0F;

  gdte[num].granularity |= gran & 0xF0;
  gdte[num].access      = access;
}

void segmentation_init(void) {
  gdtptr.limit = (sizeof(struct gdt_entry) * 5) - 1;
  gdtptr.base  = (uint32_t)&gdte;

  gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
  gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
  gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
  gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
  gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment

  gdt_flush(&gdtptr);
}
