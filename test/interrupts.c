#include <libC.h>
#include <interrupts_handlers.h>
#include <interrupts.h>
#include <tty.h>
#include <panic.h>

static struct idt_entry idt_entries[IDT_SIZE];
static struct idt_ptr idt_ptr;

static void idt_set_gate(uint8_t num, void (*base)(void), uint16_t sel, uint8_t flags) {
  idt_entries[num].base_lo = (uint32_t)base & 0xFFFF;
  idt_entries[num].base_hi = ((uint32_t)base >> 16) & 0xFFFF;
  
  idt_entries[num].sel = sel;
  idt_entries[num].always0 = 0;
  // We must uncomment the OR below when we get to using user-mode.
  // It sets the interrupt gate`s privilege level to 3.
  idt_entries[num].flags = flags /* | 0x60 */;
}

static void idt_set_gates(void);
#include "./idt_set_gates.c"

void interrupts_init(void) {
  idt_ptr.limit = sizeof(struct idt_entry) * IDT_SIZE - 1;
  idt_ptr.base = idt_entries;
  
  memset(idt_entries, 0, sizeof(struct idt_entry) * IDT_SIZE);
  
  idt_set_gates();
  
  idt_flush(&idt_ptr);
}

// Adding interrupt_handler_n here will automatically generate the
// associated ISR and register it at startup.

void interrupt_handler_3(void) {
  printf("Interrupt 3\n");
  backtrace();
}

void interrupt_handler_13(void) {
  panic("General protection fault");
}

void interrupt_handler_14(void) {
  panic("Page fault");
}
