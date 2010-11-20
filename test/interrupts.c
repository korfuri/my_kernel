#include <libC.h>
#include <interrupts_handlers.h>
#include <interrupts.h>
#include <tty.h>

static struct idt_entry idt_entries[IDT_SIZE];
static struct idt_ptr idt_ptr;

static void idt_set_gate(uint8_t num, void (*base)(void), uint16_t sel, uint8_t flags) {
  idt_entries[num].base_lo = (uint32_t)base & 0xFFFF;
  idt_entries[num].base_hi = ((uint32_t)base >> 16) & 0xFFFF;

  idt_entries[num].sel = sel;
  idt_entries[num].always0 = 0;
  // We must uncomment the OR below when we get to using user-mode.
  // It sets the interrupt gate's privilege level to 3.
  idt_entries[num].flags = flags /* | 0x60 */;
}

void interrupts_init(void) {
  idt_ptr.limit = sizeof(struct idt_entry) * IDT_SIZE - 1;
  idt_ptr.base = idt_entries;

  memset(&idt_entries, 0, sizeof(struct idt_entry) * IDT_SIZE);

  idt_set_gate(0, &isr0, 0x08, 0x8E);
  idt_set_gate(1, &isr1, 0x08, 0x8E);
  idt_set_gate(2, &isr2, 0x08, 0x8E);
  idt_set_gate(3, isr3, 0x08, 0x8E);
  idt_set_gate(4, isr4, 0x08, 0x8E);
  idt_set_gate(5, isr5, 0x08, 0x8E);
  idt_set_gate(6, isr6, 0x08, 0x8E);
  idt_set_gate(7, isr7, 0x08, 0x8E);
  idt_set_gate(8, isr8, 0x08, 0x8E);
  idt_set_gate(9, isr9, 0x08, 0x8E);
  idt_set_gate(10, isr10, 0x08, 0x8E);
  idt_set_gate(11, isr11, 0x08, 0x8E);
  idt_set_gate(12, isr12, 0x08, 0x8E);
  idt_set_gate(13, isr13, 0x08, 0x8E);
  idt_set_gate(14, isr14, 0x08, 0x8E);
  idt_set_gate(15, isr15, 0x08, 0x8E);
  idt_set_gate(16, isr16, 0x08, 0x8E);
  idt_set_gate(17, isr17, 0x08, 0x8E);
  idt_set_gate(18, isr18, 0x08, 0x8E);
  idt_set_gate(19, isr19, 0x08, 0x8E);
  idt_set_gate(20, isr20, 0x08, 0x8E);
  idt_set_gate(21, isr21, 0x08, 0x8E);
  idt_set_gate(22, isr22, 0x08, 0x8E);
  idt_set_gate(23, isr23, 0x08, 0x8E);
  idt_set_gate(24, isr24, 0x08, 0x8E);
  idt_set_gate(25, isr25, 0x08, 0x8E);
  idt_set_gate(26, isr26, 0x08, 0x8E);
  idt_set_gate(27, isr27, 0x08, 0x8E);
  idt_set_gate(28, isr28, 0x08, 0x8E);
  idt_set_gate(29, isr29, 0x08, 0x8E);
  idt_set_gate(30, isr30, 0x08, 0x8E);
  idt_set_gate(31, isr31, 0x08, 0x8E);
  
  idt_flush(&idt_ptr);
}

void interrupt_handler_0(void) { printf("Interrupt 0\n"); }
void interrupt_handler_1(void) { printf("Interrupt 1\n"); }
void interrupt_handler_2(void) { printf("Interrupt 2\n"); }
void interrupt_handler_3(void) { printf("Interrupt 3\n"); }
void interrupt_handler_4(void) { printf("Interrupt 4\n"); }
void interrupt_handler_5(void) { printf("Interrupt 5\n"); }
void interrupt_handler_6(void) { printf("Interrupt 6\n"); }
void interrupt_handler_7(void) { printf("Interrupt 7\n"); }
void interrupt_handler_8(void) { printf("Interrupt 8\n"); }
void interrupt_handler_9(void) { printf("Interrupt 9\n"); }
void interrupt_handler_10(void) { printf("Interrupt 10\n"); }
void interrupt_handler_11(void) { printf("Interrupt 11\n"); }
void interrupt_handler_12(void) { printf("Interrupt 12\n"); }
void interrupt_handler_13(void) { printf("Interrupt 13\n"); }
void interrupt_handler_14(void) { printf("Interrupt 14\n"); }
void interrupt_handler_15(void) { printf("Interrupt 15\n"); }
void interrupt_handler_16(void) { printf("Interrupt 16\n"); }
void interrupt_handler_17(void) { printf("Interrupt 17\n"); }
void interrupt_handler_18(void) { printf("Interrupt 18\n"); }
void interrupt_handler_19(void) { printf("Interrupt 19\n"); }
void interrupt_handler_20(void) { printf("Interrupt 20\n"); }
void interrupt_handler_21(void) { printf("Interrupt 21\n"); }
void interrupt_handler_22(void) { printf("Interrupt 22\n"); }
void interrupt_handler_23(void) { printf("Interrupt 23\n"); }
void interrupt_handler_24(void) { printf("Interrupt 24\n"); }
void interrupt_handler_25(void) { printf("Interrupt 25\n"); }
void interrupt_handler_26(void) { printf("Interrupt 26\n"); }
void interrupt_handler_27(void) { printf("Interrupt 27\n"); }
void interrupt_handler_28(void) { printf("Interrupt 28\n"); }
void interrupt_handler_29(void) { printf("Interrupt 29\n"); }
void interrupt_handler_30(void) { printf("Interrupt 30\n"); }
void interrupt_handler_31(void) { printf("Interrupt 31\n"); }

