#ifndef THREADS_H_
#define THREADS_H_

#include <paging.h>

typedef int32_t tid;

typedef struct thread_s {
  unsigned long esp;
  paging_context pctx;
} thread_t;

// This will ask the scheduler to schedule another thread, if another
// thread exists. The function will appear to return after a
// while. Registers are preserved. This should be called every once in
// a while to avoid keeping the CPU at one task all the time.
void schedule(void);

// This creates a new thread, making it call fct and pass it data as
// an argument. The thread might be scheduled right now.
void new_thread(void (*fct)(void* data), void* data);

// This enables threading. This function never returns. It takes a
// continuation function (startfunction) that will be called in thread
// 0. It is good practice that this function loops indefinitely,
// calling schedule() at each turn.
void start_threads(void (*startfunction)(void*));

// Destroys a given thread. It won't be scheduled anymore. This
// doesn't handle destroying the associated ressources, if any.
// You should disable interrupts before calling this.
void thread_destroy(unsigned int thrid);

// Destroys the current thread and calls schedule. This never returns.
void thread_destroy_current(void);

// Switches the current thread to usermode (ring 3)
void switch_to_user_mode();

#endif
