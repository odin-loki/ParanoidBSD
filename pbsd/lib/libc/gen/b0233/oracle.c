/*
 * oracle.c -- reference implementation for batch b0233.
 *
 * Original C sources, concatenated, with every function renamed with a
 * "ref_" prefix.  Function bodies are UNMODIFIED.
 *
 * Sources:
 *   hbsd/src/lib/libc/gen/ldexp.c  (+ hbsd/src/lib/msun/src/s_scalbn.c)
 *   hbsd/src/lib/libc/gen/getprogname.c
 *   hbsd/src/lib/libc/gen/setprogname.c
 *   hbsd/src/lib/libc/gen/freadlink.c
 */

#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

/* ------------------------------------------------------------------ */
/* ldexp.c                                                            */
/* ------------------------------------------------------------------ */

/*
 * ldexp() and scalbn() are defined to be identical, but ldexp() lives in libc
 * for backwards compatibility.
 */
#define scalbn ref_ldexp
/*
 * Copyright (c) 2005-2020 Rich Felker, et al.
 *
 * SPDX-License-Identifier: MIT
 *
 * Please see https://git.musl-libc.org/cgit/musl/tree/COPYRIGHT
 * for all contributors to musl.
 */
#include <float.h>
#include <math.h>
#include <stdint.h>

double scalbn(double x, int n)
{
	union {double f; uint64_t i;} u;
	double_t y = x;

	if (n > 1023) {
		y *= 0x1p1023;
		n -= 1023;
		if (n > 1023) {
			y *= 0x1p1023;
			n -= 1023;
			if (n > 1023)
				n = 1023;
		}
	} else if (n < -1022) {
		/* make sure final n < -53 to avoid double
		   rounding in the subnormal range */
		y *= 0x1p-1022 * 0x1p53;
		n += 1022 - 53;
		if (n < -1022) {
			y *= 0x1p-1022 * 0x1p53;
			n += 1022 - 53;
			if (n < -1022)
				n = -1022;
		}
	}
	u.i = (uint64_t)(0x3ff+n)<<52;
	x = y * u.f;
	return x;
}

#if (LDBL_MANT_DIG == 53) && !defined(scalbn)
__weak_reference(scalbn, ldexpl);
__weak_reference(scalbn, scalbnl);
#endif
#undef scalbn

/* ------------------------------------------------------------------ */
/* getprogname.c                                                      */
/* ------------------------------------------------------------------ */

const char *__progname;

const char *
ref__getprogname(void)
{

	return (__progname);
}

/* ------------------------------------------------------------------ */
/* setprogname.c                                                      */
/* ------------------------------------------------------------------ */

void
ref_setprogname(const char *progname)
{
	const char *p;

	p = strrchr(progname, '/');
	if (p != NULL)
		__progname = p + 1;
	else
		__progname = progname;
}

/* ------------------------------------------------------------------ */
/* freadlink.c                                                        */
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright 2026 The FreeBSD Foundation
 *
 * This software were developed by Konstantin Belousov <kib@FreeBSD.org>
 * under sponsorship from the FreeBSD Foundation.
 */

ssize_t
ref_freadlink(int fd, char *buf, size_t bufsize)
{
	return (readlinkat(fd, "", buf, bufsize));
}
