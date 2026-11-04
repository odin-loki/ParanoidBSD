/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Copyright (c) 2011 The FreeBSD Foundation
 *
 * Portions of this software were developed by David Chisnall
 * under sponsorship from the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * The copyright notice above is that of all four batch b0069 sources, which
 * carry byte-identical headers:
 *	lib/libc/stdlib/atoi.c
 *	lib/libc/stdlib/atol.c
 *	lib/libc/stdlib/atof.c
 *	lib/libc/stdlib/atoll.c
 *
 * FreeBSD declares the locale_t variants in <xlocale.h>; on this platform the
 * same declarations live in <locale.h>.
 */

module;

#include <stdlib.h>
#include <locale.h>

export module pbsd.lib.libc.stdlib.b0069;

export namespace pbsd::lib_libc_stdlib::b0069 {

/* lib/libc/stdlib/atoi.c */

int
atoi(const char *str)
{
	return (int)::strtol(str, NULL, 10);
}

int
atoi_l(const char *str, ::locale_t locale)
{
	return (int)::strtol_l(str, NULL, 10, locale);
}

/* lib/libc/stdlib/atol.c */

long
atol(const char *str)
{
	return ::strtol(str, (char **)NULL, 10);
}

long
atol_l(const char *str, ::locale_t locale)
{
	return ::strtol_l(str, (char **)NULL, 10, locale);
}

/* lib/libc/stdlib/atof.c */

double
atof(const char *ascii)
{
	return ::strtod(ascii, (char **)NULL);
}

double
atof_l(const char *ascii, ::locale_t locale)
{
	return ::strtod_l(ascii, (char **)NULL, locale);
}

/* lib/libc/stdlib/atoll.c */

long long
atoll(const char *str)
{
	return ::strtoll(str, (char **)NULL, 10);
}

long long
atoll_l(const char *str, ::locale_t locale)
{
	return ::strtoll_l(str, (char **)NULL, 10, locale);
}

} /* namespace pbsd::lib_libc_stdlib::b0069 */
