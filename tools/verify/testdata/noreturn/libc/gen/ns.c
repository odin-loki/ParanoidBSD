/*
 * The err.c shape.  `_err' is what the definition is called; `err' is
 * what <err.h> declares __dead2, and namespace.h beside this turns one
 * into the other for every caller inside libc.  The lint must not
 * report it.
 */
#include "namespace.h"
#include <err.h>
#include "un-namespace.h"

static void
verrci(int eval, const char *fmt, va_list ap)
{
	vfprintf(stderr, fmt, ap);
	exit(eval);
}

void
_err(int eval, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	verrci(eval, fmt, ap);
	va_end(ap);
}

/*
 * The control.  namespace.h maps `quit' to `_quit' exactly as it maps
 * `err' to `_err', but nothing declares quit() noreturn -- so finding
 * an alias must not by itself excuse the definition.  This one is
 * reported.
 */
void
_quit(int code)
{
	exit(code);
}

/*
 * A private name namespace.h does not map at all: the walk finds the
 * header, finds no alias, and reports it.
 */
void
_privately_exits(int code)
{
	exit(code);
}
