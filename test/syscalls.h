#ifndef SYSCALLS_H_
#define SYSCALLS_H_

#include <libC.h>
#include <registers.h>

void handle_syscall(struct registers*);

struct ksys_args_empty {};

struct ksys_write_args {
  char*		str;
  size_t	len;
};

int ksys_write(struct ksys_write_args*);

int ksys_blu(struct ksys_args_empty*);

#endif
