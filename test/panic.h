/*
** panic.h
**
** Provides tools to handle critical situations (i.e. forced kernel crash)
*/

#ifndef PANIC_H_
#define PANIC_H_

#include <libC.h>

void panic(char* error_msg);

#endif
