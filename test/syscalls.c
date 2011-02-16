#include <syscalls.h>
#include <user/errno.h>
#include <registers.h>

typedef int (*syscallhandler_t)();

static int ksys_zero(void* unused __attribute__((unused))) { return 0; }

static const syscallhandler_t handlers[] = {
  ksys_zero, // 0
  ksys_write, // 1
  ksys_blu // 2
};

static const uint32_t nb_syscalls = sizeof(handlers) / sizeof(handlers[0]);

#define _return(x) do{regs->eax=(x);goto end;}while(0)

#include <tty.h>
int ksys_write(struct ksys_write_args* args) {
  printf("write: %p %d\n", args->str, args->len);
  putstrn(args->str, args->len);
  return OK;
}

int ksys_blu(struct ksys_args_empty* args __attribute__((unused))) {
  printf("blu !!!@#\n");
  return OK;
}

void handle_syscall(struct registers* regs) {
  uint32_t sysnum = regs->eax;
  if (sysnum > nb_syscalls) {
    regs->eax = EBADSYSCALL;
    return;
  } 
  uint32_t retVal = (*(handlers[sysnum]))(regs->esp + 20);
  regs->eax = retVal;
}
