module;

#include <cerrno>
#include <cstdio>
#include <cstdlib>

#include <fcntl.h>
#include <unistd.h>

#ifndef O_CLOFORK
#define O_CLOFORK 0x08000000
#endif
#ifndef F_DUP3FD
#define F_DUP3FD 24
#endif
#ifndef F_DUP3FD_SHIFT
#define F_DUP3FD_SHIFT 16
#endif
#ifndef FD_CLOFORK
#define FD_CLOFORK 4
#endif

export module pbsd.lib.libc.gen.b0258;

export namespace pbsd::lib_libc_gen::b0258 {

extern "C" int _fcntl(int, int, int);

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2012 Jukka A. Ukkonen
 * All rights reserved.
 *
 * This software was developed by Jukka Ukkonen for FreeBSD.
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

int
__dup3(int oldfd, int newfd, int flags)
{
	int fdflags;

	if (oldfd == newfd) {
		errno = EINVAL;
		return (-1);
	}

	if ((flags & ~(O_CLOEXEC | O_CLOFORK)) != 0) {
		errno = EINVAL;
		return (-1);
	}

	fdflags = ((flags & O_CLOEXEC) != 0 ? FD_CLOEXEC : 0) |
	    ((flags & O_CLOFORK) != 0 ? FD_CLOFORK : 0);

	return (_fcntl(oldfd, F_DUP3FD | (fdflags << F_DUP3FD_SHIFT), newfd));
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
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

void
__assert(const char *func, const char *file, int line, const char *failedexpr)
{
	if (func == NULL)
		(void)fprintf(stderr,
		     "Assertion failed: (%s), file %s, line %d.\n", failedexpr,
		     file, line);
	else
		(void)fprintf(stderr,
		     "Assertion failed: (%s), function %s, file %s, line %d.\n",
		     failedexpr, func, file, line);
	abort();
	/* NOTREACHED */
}

} /* namespace pbsd::lib_libc_gen::b0258 */
