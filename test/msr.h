#ifndef MSR_H_
#define MSR_H_

#include <libC.h>

typedef uint32_t msrid;

void getmsr(msrid msr, uint32_t* lo, uint32_t* hi);
void setmsr(msrid msr, uint32_t lo, uint32_t hi);

#define MSR_SYSENTER_CS		0x174
#define MSR_SYSENTER_ESP	0x175
#define MSR_SYSENTER_EIP	0x176

#endif
