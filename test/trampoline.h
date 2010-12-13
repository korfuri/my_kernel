#ifndef TRAMPOLINE_H_
#define TRAMPOLINE_H_

// This function calls `fct', passing `data' as an argument, and
// returns the call's return value. The only difference with a normal
// call is that is uses the provided stack. The old stack is restored
// before returning. It is the caller's responsibility to ensure the
// stack is large enough to call the function and everything it may
// call.
// This function doesn't disable interrupts. Be careful.
unsigned long trampoline(void (*fct)(void* data), void* data, void* stack);

#endif
