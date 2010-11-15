/*
** panic.h
**
** Provides tools to handle critical situations (i.e. forced kernel crash)
*/

#ifndef PANIC_H_
#define PANIC_H_

#include <libC.h>

#define MAX_BACKTRACE_DEPTH 4
void panic(char* error_msg)  __attribute__((noreturn));

#endif
