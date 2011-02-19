#ifndef SYSCALLS_H_
#define SYSCALLS_H_

#include <libC.h>
#include <registers.h>

void handle_syscall(struct registers*, uintptr_t saved_eip);

struct ksys_args_empty {};

struct ksys_write_args {
  char*		str;
  size_t	len;
};
int ksys_write(struct ksys_write_args*); // 1

struct ksys_read_args {
  char*		str;
  size_t	len;
};
int ksys_read(struct ksys_read_args*); // 2

struct ksys_exit_args {
  int		status;
};
int ksys_exit(struct ksys_exit_args*); // 3

int ksys_switch_to_user_mode(struct ksys_args_empty* args, struct registers* regs, uintptr_t eip); // 4

struct ksys_sleep_args {
  unsigned int	ticks;
};
int ksys_sleep(struct ksys_sleep_args*); // 5

#endif
