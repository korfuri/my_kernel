#include <libC.h>
#include <interrupts_handlers.h>
#include <interrupts.h>
#include <tty.h>
#include <panic.h>
#include <registers.h>
#include <ports.h>
#include <threads.h>
#include <keyboard.h>
#include <syscalls.h>

static struct idt_entry idt_entries[IDT_SIZE];
static struct idt_ptr idt_ptr;
static unsigned int ticks = 0;

static void end_of_interrupt(void) {
  outportb(0x20, 0x20);
}

static void idt_set_gate(uint8_t num, void (*base)(void), uint16_t sel, uint8_t flags) {
  idt_entries[num].base_lo = (uint32_t)base & 0xFFFF;
  idt_entries[num].base_hi = ((uint32_t)base >> 16) & 0xFFFF;
  
  idt_entries[num].sel = sel;
  idt_entries[num].always0 = 0;
  idt_entries[num].flags = flags | 0x60;
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

unsigned int getticks(void) {
  return ticks;
}

// Adding interrupt_handler_n here will automatically generate the
// associated ISR and register it at startup.

void interrupt_handler_3(struct registers regs) {
  unlocked_printf("Interrupt 3\n");
  dump_regs(&regs);
  backtrace();
}

void interrupt_handler_10(void) {
  panic("Invalid TSS");
}

void interrupt_handler_13(struct registers regs, unsigned long info, unsigned long eip) {
  backtrace();
  dump_regs(&regs);
  printf("General protection fault [ec=%d] [eip=%p]\n", info, eip);
  thread_destroy_current();
  schedule();
}

void interrupt_handler_14(struct registers regs, unsigned long info, unsigned long eip) {
  uintptr_t at;
  asm volatile("movl %%cr2, %0": "=b"(at));
  printf("at : %p, info : %d, eip : %p\n", at, info, eip);
  printf("Error details :\n"
	 "\tType : %s\n"
	 "\tRead/Write : %s\n"
	 "\tUser/Kernel : %s\n"
	 "\tDue to reserved bits ? : %s\n"
	 "\tInstruction fetch : %s\n",
	 (info & PAGEFAULT_IS_PROTECTION_VIOLATION) ? "Protection violation" : "Nonpresent page",
	 (info & PAGEFAULT_IS_WRITE) ? "Write" : "Read",
	 (info & PAGEFAULT_IS_FROM_USERMODE) ? "User" : "Kernel",
	 (info & PAGEFAULT_IS_RESERVEDBITS) ? "Yes" : "No",
	 (info & PAGEFAULT_IS_INSTRFETCH) ? "Yes" : "No");
  backtrace();
  dump_regs(&regs);
  printf("Page fault");
  thread_destroy_current();
  schedule(); // unreachable
}

// Interrupt 32 is IRQ 0, i.e. the timer
void interrupt_handler_32(void) {
  ticks++;
  end_of_interrupt();
  schedule();
}

void interrupt_handler_33(void) {
  unsigned int c = inportb(0x60);
  keyboard_write(c);
  end_of_interrupt();
  schedule();
}

void interrupt_handler_128(struct registers regs, uintptr_t eip) {
  handle_syscall(&regs, eip);
  schedule();
}

void interrupt_handler_8(void) {
  panic("Double fault");
  schedule();
}
