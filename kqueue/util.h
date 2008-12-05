#ifndef UTIL_H
#define UTIL_H
void fatal (char const *const fmt, ...);
void error (char const *const fmt, ...);
void *xmalloc (register unsigned long const size);
void *xrealloc (register void *const ptr, register unsigned long const size);
int	all_digits (register char const *const s);
extern char const *pname;
#endif