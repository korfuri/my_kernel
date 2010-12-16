#include <libC.h>
#include <tty.h>
#include <panic.h>
#include <kmalloc.h>
#include <threads.h>

#define MAXPROCS 256

void switch_thread_asm(void* esp, volatile unsigned long* where_to_save_esp);
unsigned long new_thread_asm(unsigned long stackbase, void* retpoint, volatile unsigned long* where_to_save_esp, void* data);

static volatile int current_thread;
static volatile int max_thread;
static volatile thread_t threads[MAXPROCS];

#ifdef THREADS_DEBUG
#define thr_printf printf
#else
void thr_printf(char* format, ...) {
  format = 0;
}
#endif

void schedule(void) {
  int old_thread = current_thread;
  if (max_thread == -1) {
    // FIXME is this even reachable ?!
    panic("Last thread died\n");
  }
  int n = 0;
  do {
    current_thread++;
    if (current_thread > max_thread)
      current_thread = 0;
    n++;
    if (n > max_thread + 1) {
      panic("No more alive threads\n");
    }
  } while (threads[current_thread].esp == 0);
  if (current_thread != old_thread) {
    if (old_thread == -1)
      switch_thread_asm((void*)(threads[current_thread].esp), NULL);
    else
      switch_thread_asm((void*)(threads[current_thread].esp), &(threads[old_thread].esp));
  }
}

void thread_entry(void* old_stack, void (*entry)(void* data), void* data) {
  threads[current_thread].esp = (unsigned long)old_stack;
  max_thread++;
  current_thread = max_thread;
  
  (*entry)(data);

  threads[current_thread].esp = 0;
  threads[current_thread].pctx = 0;
  current_thread = -1;
  for (;;) schedule();
}

static unsigned long thread_new_stack(void) {
  uintptr_t physical = rmm_allocate_page();
  add_identity_paging(current_paging_context_virtual(), current_paging_context_physical(), physical);
  restore_paging_context(current_paging_context_physical());
  return (unsigned long*)(physical + PAGE_SIZE);
}

void new_thread(void (*fct)(void* data), void* data) {
  unsigned long stack = thread_new_stack();
  new_thread_asm(stack, fct, &(threads[current_thread].esp), data);
}

void start_threads(void (*startfunction)(void*)) {
  current_thread = 0;
  max_thread = -1;
  for (unsigned int i = 0; i < MAXPROCS; i++) {
    threads[i].esp = 0;
    threads[i].pctx = 0;
  }
  new_thread(startfunction, NULL);
  schedule();
}
