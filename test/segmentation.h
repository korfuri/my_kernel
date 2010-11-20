#ifndef SEGMENTATION_H_
#define SEGMENTATION_H_

// This file defines how segmentation is handled in this kernel. This
// is pretty straightforward.
//
// This file is very much inspired from
//   http://www.jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html

#include <libC.h>

struct		gdt_entry
{
  uint16_t	limit_low;	// The lower 16 bits of the limit.
  uint16_t	base_low;	// The lower 16 bits of the base.
  uint8_t	base_middle;	// The next 8 bits of the base.
  uint8_t	access;		// Access flags, determine what ring this segment can be used in.
  uint8_t	granularity;
  uint8_t	base_high;	// The last 8 bits of the base.
} __attribute__((packed));

struct		gdt_ptr
{
  uint16_t	limit;		// The upper 16 bits of all selector limits.
  uint32_t	base;		// The address of the first gdt_entry_t struct.
} __attribute__((packed));

void segmentation_init(void);

//. ASM
void gdt_flush(void* gdt);

#endif
