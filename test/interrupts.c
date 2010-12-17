#include <libC.h>
#include <interrupts_handlers.h>
#include <interrupts.h>
#include <tty.h>
#include <panic.h>
#include <registers.h>
#include <ports.h>

static struct idt_entry idt_entries[IDT_SIZE];
static struct idt_ptr idt_ptr;

static void end_of_interrupt(void) {
  outportb(0x20, 0x20);
}

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
  
  // Remaps IRQ 0-15 to IDT entries 32-47 so there is no conflict
  // between an IRQ and an exception
  // Explanation and original source :
  // http://www.osdever.net/bkerndev/Docs/irqs.htm
  // I have no idea what these ports and magic values are exactly.
  outportb(0x20, 0x11);
  outportb(0xA0, 0x11);
  outportb(0x21, 0x20);
  outportb(0xA1, 0x28);
  outportb(0x21, 0x04);
  outportb(0xA1, 0x02);
  outportb(0x21, 0x01);
  outportb(0xA1, 0x01);
  outportb(0x21, 0x0);
  outportb(0xA1, 0x0);

  idt_set_gates();
  
  idt_flush(&idt_ptr);
  //  enable_interrupts();
}

// Adding interrupt_handler_n here will automatically generate the
// associated ISR and register it at startup.

void interrupt_handler_3(struct registers regs) {
  printf("Interrupt 3\n");
  dump_regs(&regs);
  backtrace();
}

void interrupt_handler_13(void) {
  panic("General protection fault");
}

void interrupt_handler_14(struct registers regs, unsigned long info, unsigned long eip) {
  printf("at : %p, info: %p\n", eip, info);
  dump_regs(&regs);
  panic("Page fault");
}

void interrupt_handler_32(void) {
  end_of_interrupt();
  schedule();
}

void interrupt_handler_33(void) {
  unsigned char c = inportb(0x60);
  end_of_interrupt();
  printf("keyboard ! %d\n", c);
}

void interrupt_handler_8(void) {
  panic("Double fault");
}
