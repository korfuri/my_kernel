#ifndef THREADS_H_
#define THREADS_H_

void schedule(void);
void new_thread(void (*fct)(void* data), void* data);
void start_threads(void (*startfunction)(void));

#endif
