#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include "util.h"
static void
vlog (char const *const fmt, va_list ap)
{
	 vfprintf (stderr, fmt, ap);
	 fputc ('\n', stderr);
}

void fatal (char const *const fmt, ...)
    __attribute__ ((__noreturn__));

void
fatal (char const *const fmt, ...)
{
	 va_list ap;

	 va_start (ap, fmt);
	 fprintf (stderr, "%s: ", pname);
	 vlog (fmt, ap);
	 va_end (ap);
	 exit (1);
}

void
error (char const *const fmt, ...)
{
	 va_list ap;

	 va_start (ap, fmt);
	 fprintf (stderr, "%s: ", pname);
	 vlog (fmt, ap);
	 va_end (ap);
}

int
all_digits (register char const *const s)
{
	 register char const *r;

	 for (r = s; *r; r++)
	 	 if (!isdigit (*r))
		     return 0;
	 return 1;
}

void *
xmalloc (register unsigned long const size)
{
	register void *const result = malloc (size);

	if (!result)
		fatal ("Memory exhausted");
	return result;
}

void *
xrealloc (register void *const ptr, register unsigned long const size)
{
	register void *const result = realloc (ptr, size);

	if (!result)
		fatal ("Memory exhausted");
	return result;
}