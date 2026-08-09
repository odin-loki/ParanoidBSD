module;

#include <csetjmp>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <stdint.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

export module pbsd.bin.chio.b0192;

export namespace pbsd::bin_chio::b0192 {

#ifndef CHET_MT
#define CHET_MT		0
#define CHET_ST		1
#define CHET_IE		2
#define CHET_DT		3
#endif

#ifndef CHET_VT
#define	CHET_VT		10
#endif

typedef enum {
	CES_STATUS_FULL	  = 0x001,
	CES_STATUS_IMPEXP = 0x002,
	CES_STATUS_EXCEPT = 0x004,
	CES_STATUS_ACCESS = 0x008,
	CES_STATUS_EXENAB = 0x010,
	CES_STATUS_INENAB = 0x020,
	CES_INVERT	  = 0x040,
	CES_SOURCE_VALID  = 0x080,
	CES_SCSIID_VALID  = 0x100,
	CES_LUN_VALID	  = 0x200,
	CES_PIV		  = 0x400
} ces_status_flags;

#define	CESTATUS_BITS	\
	"\20\6INENAB\5EXENAB\4ACCESS\3EXCEPT\2IMPEXP\1FULL"

struct element_type {
	const	char *et_name;
	int	et_type;
};

struct special_word {
	const	char *sw_name;
	int	sw_value;
};

#define SW_INVERT	1
#define SW_INVERT1	2
#define SW_INVERT2	3

inline std::jmp_buf port_err_jmp;
inline int port_err_armed;
inline int port_err_called;
inline int port_err_status;

inline void
port_err_arm()
{
	port_err_armed = 1;
}

inline void
port_err_disarm()
{
	port_err_armed = 0;
}

inline void
errx(int eval, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	std::vfprintf(stderr, fmt, ap);
	va_end(ap);
	std::fputc('\n', stderr);
	port_err_called = 1;
	port_err_status = eval;
	if (port_err_armed)
		std::longjmp(port_err_jmp, 1);
	std::exit(eval);
}

static const struct element_type elements[] = {
	{ "drive",		CHET_DT },
	{ "picker",		CHET_MT },
	{ "portal",		CHET_IE },
	{ "slot",		CHET_ST },
	{ "voltag",		CHET_VT },
	{ NULL,			0 },
};

static const struct special_word specials[] = {
	{ "inv",		SW_INVERT },
	{ "inv1",		SW_INVERT1 },
	{ "inv2",		SW_INVERT2 },
	{ NULL,			0 },
};

/*	$NetBSD: chio.c,v 1.6 1998/01/04 23:53:58 thorpej Exp $ */
/*-
 * SPDX-License-Identifier: BSD-4-Clause
 *
 * Copyright (c) 1996 Jason R. Thorpe <thorpej@and.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgements:
 *	This product includes software developed by Jason R. Thorpe
 *	for And Communications, http://www.and.com/
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * Additional Copyright (c) 1997, by Matthew Jacob, for NASA/Ames Research Ctr.
 * Additional Copyright (c) 2000, by C. Stephen Gunn, Waterspout Communications
 */

u_int16_t
parse_element_type(char *cp)
{
	int i;

	for (i = 0; elements[i].et_name != NULL; ++i)
		if (strcmp(elements[i].et_name, cp) == 0)
			return ((u_int16_t)elements[i].et_type);

	errx(1, "invalid element type `%s'", cp);
	/* NOTREACHED */
}

const char *
element_type_name(int et)
{
	int i;

	for (i = 0; elements[i].et_name != NULL; i++)
		if (elements[i].et_type == et)
			return elements[i].et_name;

	return "unknown";
}

u_int16_t
parse_element_unit(char *cp)
{
	int i;
	char *p;

	i = (int)strtol(cp, &p, 10);
	if ((i < 0) || (*p != '\0'))
		errx(1, "invalid unit number `%s'", cp);

	return ((u_int16_t)i);
}

int
is_special(char *cp)
{
	int i;

	for (i = 0; specials[i].sw_name != NULL; ++i)
		if (strcmp(specials[i].sw_name, cp) == 0)
			return (specials[i].sw_value);

	return (0);
}

int
parse_special(char *cp)
{
	int val;

	val = is_special(cp);
	if (val)
		return (val);

	errx(1, "invalid modifier `%s'", cp);
	/* NOTREACHED */
}

const char *
bits_to_string(ces_status_flags v, const char *cp)
{
	const char *np;
	char f, sep, *bp;
	static char buf[128];

	bp = buf;
	(void) memset(buf, 0, sizeof(buf));

	for (sep = '<'; (f = *cp++) != 0; cp = np) {
		for (np = cp; *np >= ' ';)
			np++;
		if (((int)v & (1 << (f - 1))) == 0)
			continue;
		(void) snprintf(bp, sizeof(buf) - (size_t)(bp - &buf[0]),
			"%c%.*s", sep, (int)(long)(np - cp), cp);
		bp += strlen(bp);
		sep = ',';
	}
	if (sep != '<')
		*bp = '>';

	return (buf);
}

} // namespace pbsd::bin_chio::b0192
