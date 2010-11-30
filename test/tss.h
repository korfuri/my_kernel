#ifndef TSS_H_
#define TSS_H_

struct tss {
  volatile unsigned short   link;
  volatile unsigned short   link_h;

  volatile unsigned long   esp0;
  volatile unsigned short   ss0;
  volatile unsigned short   ss0_h;

  volatile unsigned long   esp1;
  volatile unsigned short   ss1;
  volatile unsigned short   ss1_h;

  volatile unsigned long   esp2;
  volatile unsigned short   ss2;
  volatile unsigned short   ss2_h;

  volatile unsigned long   cr3;
  volatile unsigned long   eip;
  volatile unsigned long   eflags;

  volatile unsigned long   eax;
  volatile unsigned long   ecx;
  volatile unsigned long   edx;
  volatile unsigned long    ebx;

  volatile unsigned long   esp;
  volatile unsigned long   ebp;

  volatile unsigned long   esi;
  volatile unsigned long   edi;

  volatile unsigned short   es;
  volatile unsigned short   es_h;

  volatile unsigned short   cs;
  volatile unsigned short   cs_h;

  volatile unsigned short   ss;
  volatile unsigned short   ss_h;

  volatile unsigned short   ds;
  volatile unsigned short   ds_h;

  volatile unsigned short   fs;
  volatile unsigned short   fs_h;

  volatile unsigned short   gs;
  volatile unsigned short   gs_h;

  volatile unsigned short   ldt;
  volatile unsigned short   ldt_h;

  volatile unsigned short   trap;
  volatile unsigned short   iomap;
};

#endif
