// port.cppm -- PBSD C++23 port of HardenedBSD lib/libc/stdio batch b0140.
//
// Faithful ports of:
//	lib/libc/stdio/putchar.c	putchar(), putchar_unlocked()
//	lib/libc/stdio/xprintf_errno.c	__printf_arginfo_errno(),
//					__printf_render_errno()
//	lib/libc/stdio/printf.c		printf()
//
// See skipped.txt for what is not here and why.  Behaviour, signedness,
// evaluation order and return values are preserved exactly as written in the
// original C, bugs included.

module;

#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#undef putchar
#undef putchar_unlocked

// <limits.h> on this host defines NL_TEXTMAX as INT_MAX; FreeBSD's value,
// which is the one xprintf_errno.c was written against, is 2048.
#undef NL_TEXTMAX
#define NL_TEXTMAX 2048

// From lib/libc/include/printf.h.
#define PA_INT 0

export module pbsd.lib.libc.stdio.b0140;

// The private FreeBSD headers these come from do not exist off-FreeBSD.  The
// linkage-specification attaches them to the global module, so they are the
// very entities the C oracle and the harness see.
extern "C" {

struct printf_info {
	int	prec;
	int	width;
	wchar_t	spec;
	unsigned is_long_double:1;
	unsigned is_char:1;
	unsigned is_short:1;
	unsigned is_long:1;
	unsigned is_long_long:1;
	unsigned is_intmax:1;
	unsigned is_ptrdiff:1;
	unsigned is_size:1;
	unsigned alt:1;
	unsigned space:1;
	unsigned left:1;
	unsigned showsign:1;
	unsigned group:1;
	unsigned wide:1;
	unsigned vsep:2;
	wchar_t	pad;
	void	*loc;
};

struct __printf_io;
int __printf_out(struct __printf_io *, const struct printf_info *,
    const void *, size_t);
void __printf_flush(struct __printf_io *);

// From lib/libc/stdio/errlst.h.
extern const int __hidden_sys_nerr;

// FreeBSD's strerror_r() is the POSIX one: it always writes the message into
// the caller's buffer.  Name the host's XSI implementation directly so that
// the GNU char *-returning strerror_r() cannot be selected here.
int __xpg_strerror_r(int, char *, size_t);

} // extern "C"

export namespace pbsd::lib_libc_stdio::b0140 {

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
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
 * A subroutine version of the macro putchar
 *
 * __sputc() is FreeBSD's inline unlocked putc(): it stores the low byte of
 * `c' through the stream's unsigned char * put pointer and yields that byte
 * as an int, i.e. exactly putc_unlocked().  FLOCKFILE_CANCELSAFE() locks the
 * stream and pushes a cancellation cleanup that does nothing absent a pending
 * cancel.  oracle.c uses the identical mapping.
 */
int putchar(int c)
{
	int retval;
	FILE *so = stdout;

	::flockfile(so);
	/* Orientation set by __sputc() when buffer is full. */
	/* ORIENT(so, -1); */
	retval = ::putc_unlocked(c, so);
	::funlockfile(so);
	return (retval);
}

int putchar_unlocked(int ch)
{

	return (::putc_unlocked(ch, stdout));
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2005 Poul-Henning Kamp
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

int __printf_arginfo_errno(const struct printf_info *pi [[maybe_unused]],
    size_t n, int *argt)
{

	assert(n >= 1);
	argt[0] = PA_INT;
	return (1);
}

int __printf_render_errno(struct __printf_io *io,
    const struct printf_info *pi [[maybe_unused]], const void *const *arg)
{
	int ret, error;
	char buf[64];
	char errnomsg[NL_TEXTMAX];

	ret = 0;
	error = *((const int *)arg[0]);
	if (error >= 0 && error < ::__hidden_sys_nerr) {
		::__xpg_strerror_r(error, errnomsg, sizeof(errnomsg));
		return (::__printf_out(io, pi, errnomsg, ::strlen(errnomsg)));
	}
	::sprintf(buf, "errno=%d/0x%x", error, error);
	ret += ::__printf_out(io, pi, buf, ::strlen(buf));
	::__printf_flush(io);
	return(ret);
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
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

int printf(char const * __restrict fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = ::vfprintf(stdout, fmt, ap);
	va_end(ap);
	return (ret);
}

} // export namespace pbsd::lib_libc_stdio::b0140
