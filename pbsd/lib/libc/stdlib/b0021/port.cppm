/*
 * PBSD -- C++23 port of HardenedBSD lib/libc/stdlib batch b0021.
 *
 * Sources ported here, verbatim in behaviour:
 *	hbsd/src/lib/libc/stdlib/_Exit.c
 *	hbsd/src/lib/libc/stdlib/remque.c
 *	hbsd/src/lib/libc/stdlib/l64a.c
 *
 * Original copyright headers are reproduced above each ported unit.
 */
module;

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

export module pbsd.lib.libc.stdlib.b0021;

export namespace pbsd::lib_libc_stdlib::b0021 {

/*
 * ===== hbsd/src/lib/libc/stdlib/_Exit.c =====
 *
 * This file is in the public domain.  Written by Garrett A. Wollman,
 * 2002-09-07.
 */

/*
 * ISO C99 added this function to provide for Standard C applications
 * which needed something like POSIX _exit().  A new interface was created
 * in case it turned out that _exit() was insufficient to meet the
 * requirements of ISO C.  (That's probably not the case, but here
 * is where you would put the extra code if it were.)
 */
void
_Exit(int code)
{
	::_exit(code);
}

/*
 * ===== hbsd/src/lib/libc/stdlib/remque.c =====
 *
 * Initial implementation:
 * Copyright (c) 2002 Robert Drehmel
 * All rights reserved.
 *
 * As long as the above copyright statement and this notice remain
 * unchanged, you can do what ever you want with this file. 
 */

/* <search.h> under _SEARCH_PRIVATE. */
struct que_elem {
	struct que_elem *next;
	struct que_elem *prev;
};

void
remque(void *element)
{
	struct que_elem *prev, *next, *elem;

	elem = (struct que_elem *)element;

	prev = elem->prev;
	next = elem->next;

	if (prev != NULL)
		prev->next = next;
	if (next != NULL)
		next->prev = prev;
}

/*
 * ===== hbsd/src/lib/libc/stdlib/l64a.c =====
 *
 * Written by J.T. Conklin <jtc@NetBSD.org>.
 * Public domain.
 */

int l64a_r(long value, char *buffer, int buflen);

char *
l64a(long value)
{
	static char buf[7];

	(void)l64a_r(value, buf, sizeof(buf));
	return (buf);
}

int
l64a_r(long value, char *buffer, int buflen)
{
	static const char chars[] =
	    "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	uint32_t v;

	v = value;
	while (buflen-- > 0) {
		if (v == 0) {
			*buffer = '\0';
			return (0);
		}
		*buffer++ = chars[v & 0x3f];
		v >>= 6;
	}
	return (-1);
}

} /* namespace pbsd::lib_libc_stdlib::b0021 */
