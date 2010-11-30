#include <segmentation.h>
#include <tss.h>
#include <libC.h>
#include <kmalloc.h>

static uintptr_t segment_tss;

static struct tss tss_gl;

void tss_init(void) {
  memset(&tss_gl, '\0', sizeof(tss_gl));
  tss_gl.esp0 = (uint32_t)kmalloc(4096);
  tss_gl.ss0 = segment_kernel_stack;
  tss_gl.iomap = sizeof(tss_gl);
  segment_tss = gdt_set_gate((uintptr_t)&tss_gl, sizeof(tss_gl), 0x89 /*present|exec|accessed*/, 0x40);
  do_gdt_flush();

  asm volatile("ltr %%ax" :: "a" (segment_tss | 0x3));
}
