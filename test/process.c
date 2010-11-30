#include <process.h>
#include <kmalloc.h>
#include <segmentation.h>
#include <paging.h>
#include <msr.h>
#include <libC.h>

static struct process	kernel_process;
static char    		kernel_stack[0x1000];

void test_process(void) {
  printf("in test_process\n");
  process_schedule(&kernel_process);
  //for (uint32_t i = 0; i < 200000000; i++);
  //  while (1)
  //    syscall();
}

void process_init(void) {
  kernel_process.pid = 0;
  kernel_process.eip = sysenter_entry;
  kernel_process.esp = (void*)(kernel_stack + 0x1000);
  
  setmsr(MSR_SYSENTER_CS, segment_kernel_code, 0);
  setmsr(MSR_SYSENTER_EIP, kernel_process.eip, 0);
  setmsr(MSR_SYSENTER_EIP, kernel_process.esp, 0);
}

void* new_stack(void) {
  return (void*)((uintptr_t)kmalloc(0x1000) + 0x1000);
}

struct process* process_new(void) {
  struct process* p = kmalloc(sizeof(struct process));
  p->pid = 1;
  p->eip = test_process;
  p->esp = new_stack();
  uintptr_t* esp = p->esp;
  *esp = p->eip;
  esp--;
  p->esp = esp;
  return p;
}

void sysenter_entry_C(void* esp, void* eip) {
  printf("In sysenter_entry_C\n");
}

void process_schedule(struct process* p) {
  printf("Going to switch process, new eip = %p, new esp = %p\n", p->eip, p->esp);
  process_schedule_asm(p->esp);
}
