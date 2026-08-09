/*
 * Oracle for PBSD batch b0021: the original HardenedBSD C sources,
 * concatenated, with every function renamed with a "ref_" prefix.
 * Function bodies are UNMODIFIED.  Only declarations/defines that the
 * host environment does not supply have been added.
 *
 * This file is the specification.  Do not "fix" anything in it.
 */

/*
 * ===== hbsd/src/lib/libc/stdlib/_Exit.c =====
 *
 * This file is in the public domain.  Written by Garrett A. Wollman,
 * 2002-09-07.
 */

#include <stdlib.h>
#include <unistd.h>

/*
 * ISO C99 added this function to provide for Standard C applications
 * which needed something like POSIX _exit().  A new interface was created
 * in case it turned out that _exit() was insufficient to meet the
 * requirements of ISO C.  (That's probably not the case, but here
 * is where you would put the extra code if it were.)
 */
void
ref__Exit(int code)
{
	_exit(code);
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
#define	_SEARCH_PRIVATE
#include <search.h>
#include <stdlib.h>	/* for NULL */

/*
 * Added define: FreeBSD's <search.h> exposes this under _SEARCH_PRIVATE;
 * the host <search.h> does not declare it.
 */
struct que_elem {
	struct que_elem *next;
	struct que_elem *prev;
};

void
ref_remque(void *element)
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

#include <sys/cdefs.h>
#include <stdint.h>
#include <stdlib.h>

/*
 * Added define: the body of l64a() below calls l64a_r(), which must resolve
 * to the reference implementation in this file rather than to any libc one.
 */
int ref_l64a_r(long value, char *buffer, int buflen);
#define	l64a_r	ref_l64a_r

char *
ref_l64a(long value)
{
	static char buf[7];

	(void)l64a_r(value, buf, sizeof(buf));
	return (buf);
}

int
ref_l64a_r(long value, char *buffer, int buflen)
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
