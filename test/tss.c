#include <tss.h>
#include <segmentation.h>

static struct tss_entry_t tss_entry;

static char tmp_stack[10000];
static char* tmp_stack_top = tmp_stack + sizeof(tmp_stack) - 1;

uintptr_t write_tss(void) {
  // Firstly, let's compute the base and limit of our entry into the GDT.
  uint32_t base = (uintptr_t) &tss_entry;
  uint32_t limit = base + sizeof(struct tss_entry_t);

  // Ensure the descriptor is initially zero.
  memset(&tss_entry, 0, sizeof(tss_entry));

  tss_entry.ss0 = segment_kernel_stack; // Set the kernel stack segment.
  tss_entry.esp0 = tmp_stack_top; // Set the kernel stack pointer.

  // Here we set the cs, ss, ds, es, fs and gs entries in the TSS. These specify what
  // segments should be loaded when the processor switches to kernel mode. Therefore
  // they are just our normal kernel code/data segments - 0x08 and 0x10 respectively,
  // but with the last two bits set, making 0x0b and 0x13. The setting of these bits
  // sets the RPL (requested privilege level) to 3, meaning that this TSS can be used
  // to switch to kernel mode from ring 3.
  tss_entry.cs = (segment_kernel_code | 3);
  tss_entry.ss =
    tss_entry.ds =
    tss_entry.es =
    tss_entry.fs =
    tss_entry.gs = (segment_kernel_stack | 3);

  // Now, add our TSS descriptor's address to the GDT.
  uintptr_t ret = gdt_set_gate(base, limit, 0xE9, 0x00);

  return ret;
}
