#ifndef STDARG_H_
#define STDARG_H_

// stdarg the GCC way
// This is obviously not portable on another compiler
typedef __builtin_va_list va_list;
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg

#endif
