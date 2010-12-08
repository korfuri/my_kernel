#include <stdio.h>
#include <stdlib.h>

void switch_task_asm(void* esp);
unsigned long new_task_asm(unsigned long stackbase, void* retpoint);

unsigned int current_coroutine = 0;
unsigned long coroutines[10];

void a(void) {
  printf("In a\n");
}

void b(void) {
  printf("In b\n");
}

void coroutine_entry(void* old_stack, void (*entry)(void)) {
  printf("New coroutine ! %p\n", entry);
  (*entry)();
  printf("Out of coroutine %p\n", entry);
  switch_task_asm(old_stack);
  printf("We should have switched stacks by now... at %p\n", entry);
}

/* void end_of_coroutine(void* s) { */
/*   printf("A coroutine died (stack = %p)\n", s); */
/*   sleep(2); */
/*   build_stack((unsigned long)((unsigned long*)malloc(4000) + 1000), &a); */
/* } */

void new_task(void) {
  unsigned long* stack = malloc(4000);
  new_task_asm((unsigned long)(stack + 1000), &b);
}

int main(void) {
  printf("In main\n");
  a();
  printf("main:2\n");
  new_task();
  printf("Out of main\n");
  return 0;
}
