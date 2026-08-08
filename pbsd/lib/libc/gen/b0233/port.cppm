/*
 * ldexp.c
 *
 * ldexp() and scalbn() are defined to be identical, but ldexp() lives in libc
 * for backwards compatibility.
 */

module;

#include <cfloat>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>

extern "C" ssize_t readlinkat(int, const char *, char *, size_t);

export module pbsd.lib.libc.gen.b0233;

export namespace pbsd::lib_libc_gen::b0233 {

/*
 * Copyright (c) 2005-2020 Rich Felker, et al.
 *
 * SPDX-License-Identifier: MIT
 *
 * Please see https://git.musl-libc.org/cgit/musl/tree/COPYRIGHT
 * for all contributors to musl.
 */
double
ldexp(double x, int n)
{
	union {double f; std::uint64_t i;} u;
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
	u.i = (std::uint64_t)(0x3ff+n)<<52;
	x = y * u.f;
	return x;
}

const char *__progname;

const char *
_getprogname(void)
{

	return (__progname);
}

void
setprogname(const char *progname)
{
	const char *p;

	p = strrchr(progname, '/');
	if (p != NULL)
		__progname = p + 1;
	else
		__progname = progname;
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright 2026 The FreeBSD Foundation
 *
 * This software were developed by Konstantin Belousov <kib@FreeBSD.org>
 * under sponsorship from the FreeBSD Foundation.
 */

ssize_t
freadlink(int fd, char *buf, size_t bufsize)
{
	return (readlinkat(fd, "", buf, bufsize));
}

} /* namespace pbsd::lib_libc_gen::b0233 */
