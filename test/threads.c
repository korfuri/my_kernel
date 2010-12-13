#include <libC.h>
#include <tty.h>
#include <panic.h>
#include <kmalloc.h>

#define NCR 10

void switch_thread_asm(void* esp, volatile unsigned long* where_to_save_esp);
unsigned long new_thread_asm(unsigned long stackbase, void* retpoint, volatile unsigned long* where_to_save_esp, void* data);

static volatile int current_thread;
static volatile int max_thread;
static volatile unsigned long threads[NCR];

#ifdef THREADS_DEBUG
#define thr_printf printf
#else
void thr_printf(char* format, ...) {
  format = 0;
}
#endif

void schedule(void) {
  //  threads[current_thread] = (unsigned long)esp;
  int old_thread = current_thread;
  if (max_thread == -1) {
    panic("Last thread died\n");
  }
  thr_printf("We are at thread #%d\n", current_thread);
  int n = 0;
  do {
    current_thread++;
    if (current_thread > max_thread)
      current_thread = 0;
    thr_printf("Testing corouting %d : %p\n", current_thread, threads[current_thread]);
    n++;
    if (n > max_thread + 1) {
      panic("No more alive threads\n");
    }
  } while (threads[current_thread] == 0);
  thr_printf("We are going to thread #%d [%p]\n", current_thread, (void*)(threads[current_thread]));
  if (current_thread != old_thread) {
    if (old_thread == -1)
      switch_thread_asm((void*)(threads[current_thread]), NULL);
    else
      switch_thread_asm((void*)(threads[current_thread]), &(threads[old_thread]));
  }
}

void thread_entry(void* old_stack, void (*entry)(void* data), void* data) {
  thr_printf("New thread ! %p We were previously in thread %d\n", entry, current_thread);

  threads[current_thread] = (unsigned long)old_stack;
  max_thread++;
  current_thread = max_thread;
  
  thr_printf("This thread has id #%d\n", current_thread);

  (*entry)(data);
  thr_printf("Out of thread %p\n", entry);

  threads[current_thread] = 0;
  current_thread = -1;
  for (;;) schedule();
  thr_printf("We should have switched stacks by now... at %p\n", entry);
}

void new_thread(void (*fct)(void* data), void* data) {
  unsigned long* stack = kmalloc(4000);
  new_thread_asm((unsigned long)(stack + 1000), fct, &(threads[current_thread]), data);
  thr_printf("WTF at end of new_thread\n");
}

void start_threads(void (*startfunction)(void*)) {
  current_thread = 0;
  max_thread = -1;
  for (unsigned int i = 0; i < NCR; i++)
    threads[i] = 0;
  new_thread(startfunction, NULL);
  schedule();
}
