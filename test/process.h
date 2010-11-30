#ifndef PROCESS_H_
#define PROCESS_H_

#include <libC.h>
#include <paging.h>

struct process {
  uint16_t		pid;
  void*			eip;
  void*			esp;
};

void process_init(void);
void sysenter_entry_C(void* esp, void* eip);
void process_schedule(struct process* p);


// ASM
void sysenter_entry(void);
void process_first_sysexit(void* esp, void* eip);
void process_schedule_asm(void* esp, void* eip);
void syscall(void);

#endif
