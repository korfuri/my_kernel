#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_

#include <libC.h>

// A struct describing an interrupt gate.
struct		idt_entry
{
  uint16_t	base_lo;	// The lower 16 bits of the address to
				// jump to when this interrupt fires.
  uint16_t	sel;		// Kernel segment selector.
  uint8_t	always0;	// This must always be zero.
  uint8_t	flags;		// More flags. See documentation.
  uint16_t	base_hi;	// The upper 16 bits of the address to
				// jump to.
} __attribute__((packed));

// A struct describing a pointer to an array of interrupt handlers.
// This is in a format suitable for giving to 'lidt'.
struct			idt_ptr
{
  uint16_t		limit;
  struct idt_entry*	base;	// The address of the first element in
				// our idt_entry_t array.
} __attribute__((packed));

void interrupts_init(void);

// ASM
void idt_flush(void*);

#define IDT_SIZE	256

#include <interrupts_handlers.h>

static inline void disable_interrupts(void) {
  asm volatile("cli");
}

static inline void enable_interrupts(void) {
  asm volatile("sti");
}

// The macros below are to test the error code on a page fault. See
// the Intel IA-32 manual volume 3A, page 6-55 for further details.

// A page fault is either a protection violation or a non-present page
#define PAGEFAULT_IS_PROTECTION_VIOLATION 1

// A page fault happens either on a read or on a write
#define PAGEFAULT_IS_WRITE 2

// A page fault happens either from user or supervisor mode
#define PAGEFAULT_IS_FROM_USERMODE 4

// A page fault can be due to reserved bits being set in the PDE
#define PAGEFAULT_IS_RESERVEDBITS 8

// A page fault can happen on an instruction fetch
#define PAGEFAULT_IS_INSTRFETCH 16


#endif
