#include <stdio.h>
#include <stdlib.h>

#define NCR 10

void switch_task_asm(void* esp, volatile unsigned long* where_to_save_esp);
unsigned long new_task_asm(unsigned long stackbase, void* retpoint, volatile unsigned long* where_to_save_esp);

void schedule(void);
void end_of_schedule(unsigned long old_coroutine);

volatile unsigned int current_coroutine;
volatile unsigned int max_coroutine;
volatile unsigned long coroutines[NCR];

void new_task(void (*fct)(void));

void a(void);
void b(void);

void a(void) {
  printf("In a\n");
  schedule();
  new_task(b);
  printf("In a2\n");
}

void b(void) {
  printf("In b\n");
  schedule();
}

void schedule() {
  //  coroutines[current_coroutine] = (unsigned long)esp;
  unsigned long old_coroutine = current_coroutine;
  if (max_coroutine == -1) {
    printf("Last coroutine died\n");
    exit(0);
  }
  printf("We are at coroutine #%d\n", current_coroutine);
  int n = 0;
  do {
    current_coroutine++;
    if (current_coroutine > max_coroutine)
      current_coroutine = 0;
    printf("Testing corouting %d : %p\n", current_coroutine, coroutines[current_coroutine]);
    n++;
    if (n > max_coroutine + 1) {
      printf("No more alive coroutines\n");
      exit(1);
    }
  } while (coroutines[current_coroutine] == 0);
  printf("We are going to coroutine #%d [%p]\n", current_coroutine, (void*)(coroutines[current_coroutine]));
  if (current_coroutine != old_coroutine) {
    if (old_coroutine == -1)
      switch_task_asm((void*)(coroutines[current_coroutine]), NULL);
    else
      switch_task_asm((void*)(coroutines[current_coroutine]), &(coroutines[old_coroutine]));
  }
}

void coroutine_entry(void* old_stack, void (*entry)(void)) {
  printf("New coroutine ! %p We were previously in coroutine %d\n", entry, current_coroutine);

  coroutines[current_coroutine] = (unsigned long)old_stack;
  max_coroutine++;
  current_coroutine = max_coroutine;
  
  printf("This coroutine has id #%d\n", current_coroutine);
  
  (*entry)();
  printf("Out of coroutine %p\n", entry);

  coroutines[current_coroutine] = 0;
  current_coroutine = -1;
  for (;;) schedule();
  printf("We should have switched stacks by now... at %p\n", entry);
}

void new_task(void (*fct)(void)) {
  unsigned long* stack = malloc(4000);
  new_task_asm((unsigned long)(stack + 1000), fct, &(coroutines[current_coroutine]));
  printf("WTF at end of new_task\n");
}

void xmain(void) {
  printf("In xmain\n");
  new_task(a);
  new_task(a);
  new_task(a);
  new_task(a);
  printf("Out of xmain\n");
}

void start_coroutines(void) {
  current_coroutine = 0;
  max_coroutine = -1;
  for (unsigned int i = 0; i < NCR; i++)
    coroutines[i] = 0;
  new_task(xmain);
  schedule();
}

int main(void) {
  printf("In main\n");
  start_coroutines();
  printf("Out of coroutines mode, back to main\n");
  return 0;
}
