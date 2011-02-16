#ifndef REGISTERS_H_
#define REGISTERS_H_

struct registers {
  const unsigned long edi;
  const unsigned long esi;
  const unsigned long ebp;
  const unsigned long esp;
  const unsigned long ebx;
  const unsigned long edx;
  const unsigned long ecx;
  unsigned long eax;
  const unsigned long eflags;
};

#endif
