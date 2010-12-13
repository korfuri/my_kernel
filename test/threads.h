#ifndef THREADS_H_
#define THREADS_H_

void schedule(void);
void new_thread(void (*fct)(void));
void start_threads(void (*startfunction)(void));

#endif
