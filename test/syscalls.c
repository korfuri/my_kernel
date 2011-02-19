#include <syscalls.h>
#include <user/errno.h>
#include <registers.h>
#include <threads.h>

typedef int (*syscallhandler_t)();

static int ksys_zero(void* unused __attribute__((unused))) { return 0; }

static const syscallhandler_t handlers[] = {
  ksys_zero, // 0
  ksys_write, // 1
  ksys_blu, // 2
  ksys_exit, // 3
  ksys_switch_to_user_mode // 4
};

static const uint32_t nb_syscalls = sizeof(handlers) / sizeof(handlers[0]);

void handle_syscall(struct registers* regs, uintptr_t saved_eip) {
  uint32_t sysnum = regs->eax;
  if (sysnum > nb_syscalls) {
    regs->eax = EBADSYSCALL;
    return;
  } 
  uint32_t retVal = (*(handlers[sysnum]))(regs->esp + 20, regs, saved_eip);
  regs->eax = retVal;
}

#include <tty.h>
int ksys_write(struct ksys_write_args* args) {
  printf("write: %p %d\n", args->str, args->len);
  putstrn(args->str, args->len);
  return OK;
}

int ksys_blu(struct ksys_args_empty* args __attribute__((unused))) {
  printf("blu !!!@#\n");
  volatile int i = 0;
  for (i = 0; i != 200000000; i++);
  printf("blu !!!@#\n");
  return OK;
}

int ksys_exit(struct ksys_exit_args* args) {
  printf("Exiting with status=%d\n", args->status);
  thread_destroy_current();
  schedule(); // unreachable
  return OK;
}

int ksys_switch_to_user_mode(struct ksys_args_empty* args __attribute__((unused)),
			     struct registers* regs, uintptr_t eip) {
  printf("Switching the current thread to user mode [eip=%p, esp=%p]\n", eip, regs->esp);
  
  /* char* v = regs; */
  /* v += sizeof(struct registers); */
  /* uintptr_t* stack = v; */
  /* stack += 2; */
  /* stack[0] = eip; // eip */
  /* stack[1] = 0x18 | 0x3; // cs */
  /* stack[2] = regs->eflags | 0x200; // eflags */
  /* stack[3] = regs->esp; // esp */
  /* stack[4] = 0x20 | 0x3; // ss */
  /* apply_usersegment(); */
  return OK;
}
