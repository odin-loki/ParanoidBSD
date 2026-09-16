/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
 *
 * libexec/tftpd/tftp-options.c:parse_options() lowercases the option
 * name a client sent with isupper(*c) / tolower(*c), where c is a
 * plain char *.
 *
 *   cbmc -DOLD tools/verify/probes/ctype_signed_char.c
 *       -> the rune table index is in range: FAILED
 *   cbmc       tools/verify/probes/ctype_signed_char.c
 *       -> 0 of N, SUCCESSFUL
 *
 *	char	*c, *cp, *option, *value;
 *	...
 *	for (c = option; *c; c++)
 *		if (isupper(*c))
 *			*c = tolower(*c);
 *
 * option is a field get_field() carved out of the received packet, so
 * *c is any byte a client cares to send.  is*() and to*() are defined
 * only for a value representable as unsigned char, or EOF; plain char
 * is signed on amd64 and i386, so a byte >= 0x80 arrives as a negative
 * int and indexes _CurrentRuneLocale->__runetype[] below zero.
 *
 * FreeBSD's <ctype.h> is the table form, not a function call:
 *
 *	static __inline int __maskrune(__ct_rune_t _c, unsigned long _f)
 *	{
 *		return ((_c < 0 || _c >= _CACHED_RUNES) ?
 *		    ___maskrune(_c, _f) :
 *		    _CurrentRuneLocale->__runetype[_c]) & _f;
 *	}
 *
 * so the inline path is guarded and ___maskrune() is reached instead --
 * but CBMC models the table access and the standard does not promise
 * the guard.  tftpd.c:484, four files away, already writes the cast.
 *
 * The assertion is that the value handed to the table is a subscript
 * the table has.
 */

#define	_CACHED_RUNES	256

int nondet_int(void);

static unsigned long runetype[_CACHED_RUNES];

static int
maskrune(int c)
{

	__CPROVER_assert(c >= 0 && c < _CACHED_RUNES,
	    "the rune table index is in range");
	return ((int)runetype[c]);
}

int
main(void)
{
	/* One byte off the wire, in the option name. */
	char ch = (char)nondet_int();

#ifdef OLD
	return (maskrune(ch));
#else
	return (maskrune((unsigned char)ch));
#endif
}
