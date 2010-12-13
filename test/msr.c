#include <msr.h>

void getmsr(msrid msr, uint32_t* lo, uint32_t* hi) {
  asm volatile("rdmsr" :
	       "=a"(*lo), "=d"(*hi):
	       "c"(msr));
}

void setmsr(msrid msr, uint32_t lo, uint32_t hi) {
  asm volatile("wrmsr":
	       :
	       "a"(lo),"d"(hi),"c"(msr));
}
