#ifndef USER_SYSCALL_H_
#define USER_SYSCALL_H_

#include <libC.h>
#include <user/errno.h>

int sys_write(char* str, size_t size);
int sys_blu(void);

#endif
