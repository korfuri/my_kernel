#ifndef USER_SYSCALL_H_
#define USER_SYSCALL_H_

#include <libC.h>
#include <user/errno.h>

int sys_write(char* str, size_t size);
int sys_read(char* str, size_t size);
int sys_exit(int status);
int sys_switch_to_user_mode(void);
int sys_sleep(unsigned int ticks);

#endif
