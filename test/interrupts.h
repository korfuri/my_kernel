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

#endif
