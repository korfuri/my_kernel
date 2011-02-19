#ifndef REGISTERS_H_
#define REGISTERS_H_

struct registers {
  volatile unsigned long edi;
  volatile unsigned long esi;
  volatile unsigned long ebp;
  volatile unsigned long esp;
  volatile unsigned long ebx;
  volatile unsigned long edx;
  volatile unsigned long ecx;
  volatile unsigned long eax;
  volatile unsigned long eflags;
};

#endif
