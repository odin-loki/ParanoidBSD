/*-
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#if 0
#if defined(LIBC_SCCS) && !defined(lint)
__RCSID("$NetBSD: a64l.c,v 1.8 2000/01/22 22:19:19 mycroft Exp $");
#endif /* not lint */
#endif

#include <sys/cdefs.h>
#include <stdlib.h>
#include <inttypes.h>

#define	ADOT	46		/* ASCII '.' */
#define	ASLASH	47		/* ASCII '/' */
#define	A0	48		/* ASCII '0' */
#define	AA	65		/* ASCII 'A' */
#define	Aa	97		/* ASCII 'a' */

long
a64l(const char *s)
{
	long shift;
	int digit, i;
	/*
	 * PBSD: value was an int and the accumulation was `value |=
	 * digit << shift' with shift reaching 30, so any sixth character
	 * whose digit is 2 or more is a shift past the width of an int.
	 * That is not a corner: l64a_r() converts through a uint32_t, so
	 * it emits a sixth character for every value with bit 31 set, and
	 * a64l(l64a(0x80000000)) is ".....0" -- 2 << 30.  UBSan traps it.
	 *
	 * And *s is a plain char, so a byte above 0x7f is negative on the
	 * architectures where char is signed: the first arm then makes
	 * digit negative and the shift operand is negative too.  POSIX
	 * leaves a string l64a() did not produce unspecified, which is
	 * not the same as undefined.
	 *
	 * Accumulating in uint32_t settles both -- the conversion is
	 * modular and the shift is defined for every digit -- and the
	 * result is sign-extended on return, which is what returning an
	 * int used to do.
	 */
	uint32_t value;

	value = 0;
	shift = 0;
	for (i = 0; *s != '\0' && i < 6; i++, s++) {
		if (*s <= ASLASH)
			digit = *s - ASLASH + 1;
		else if (*s <= A0 + 9)
			digit = *s - A0 + 2;
		else if (*s <= AA + 25)
			digit = *s - AA + 12;
		else
			digit = *s - Aa + 38;

		value |= (uint32_t)digit << shift;
		shift += 6;
	}
	return ((long)(int32_t)value);
}
