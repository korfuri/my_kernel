#ifndef THREADS_H_
#define THREADS_H_

#include <paging.h>

typedef int32_t tid;

typedef struct thread_s {
  unsigned long esp;
  paging_context pctx;
} thread_t;

void schedule(void);
void new_thread(void (*fct)(void* data), void* data);
void start_threads(void (*startfunction)(void*));

#endif
