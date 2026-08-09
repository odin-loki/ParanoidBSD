/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1989, 1993
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

/*
 * PBSD b0171 -- C++23 port of hbsd/src/bin/echo/echo.c.
 *
 * main() becomes echo_main(); the syscall/library surface is routed through
 * the same shim the oracle uses so that the side effects are observable.  The
 * only other departure from the C text is the cast on the malloc() result,
 * which C++ requires and C does not.
 */

module;

#include <sys/types.h>
#include <sys/uio.h>

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#ifndef IOV_MAX
#define	IOV_MAX	1024
#endif

#undef caph_limit_stdio
#undef caph_enter
#define	caph_limit_stdio()	pbsd_shim_caph_limit_stdio()
#define	caph_enter()		pbsd_shim_caph_enter()

#undef malloc
#define	malloc(n)		pbsd_shim_malloc(n)

#undef writev
#define	writev(a, b, c)		pbsd_shim_writev((a), (b), (c))

#undef err
#define	err(...)		pbsd_shim_err(__VA_ARGS__)

export module pbsd.bin.echo.b0171;

/*
 * The observable shim, shared with the oracle.  Declared here rather than in
 * the global module fragment because a GMF may only contain preprocessor
 * inclusions; extern "C" declarations attach to the global module regardless.
 */
extern "C" {
int pbsd_shim_caph_limit_stdio(void);
int pbsd_shim_caph_enter(void);
void *pbsd_shim_malloc(std::size_t);
ssize_t pbsd_shim_writev(int, const struct iovec *, int);
[[noreturn]] void pbsd_shim_err(int, const char *);
}

export namespace pbsd::bin_echo::b0171 {

int
echo_main(int argc, char *argv[])
{
	int nflag;	/* if not set, output a trailing newline. */
	int veclen;	/* number of writev arguments. */
	struct iovec *iov, *vp; /* Elements to write, current element. */
	char space[] = " ";
	char newline[] = "\n";

	if (caph_limit_stdio() < 0 || caph_enter() < 0)
		err(1, "capsicum");

	/* This utility may NOT do getopt(3) option parsing. */
	if (*++argv && !std::strcmp(*argv, "-n")) {
		++argv;
		--argc;
		nflag = 1;
	} else
		nflag = 0;

	veclen = (argc >= 2) ? (argc - 2) * 2 + 1 : 0;

	if ((vp = iov = static_cast<struct iovec *>(
	    malloc((veclen + 1) * sizeof(struct iovec)))) == nullptr)
		err(1, "malloc");

	while (argv[0] != nullptr) {
		std::size_t len;

		len = std::strlen(argv[0]);

		/*
		 * If the next argument is NULL then this is the last argument,
		 * therefore we need to check for a trailing \c.
		 */
		if (argv[1] == nullptr) {
			/* is there room for a '\c' and is there one? */
			if (len >= 2 &&
			    argv[0][len - 2] == '\\' &&
			    argv[0][len - 1] == 'c') {
				/* chop it and set the no-newline flag. */
				len -= 2;
				nflag = 1;
			}
		}
		vp->iov_base = *argv;
		vp++->iov_len = len;
		if (*++argv) {
			vp->iov_base = space;
			vp++->iov_len = 1;
		}
	}
	if (!nflag) {
		veclen++;
		vp->iov_base = newline;
		vp++->iov_len = 1;
	}
	/* assert(veclen == (vp - iov)); */
	while (veclen) {
		int nwrite;

		nwrite = (veclen > IOV_MAX) ? IOV_MAX : veclen;
		if (writev(STDOUT_FILENO, iov, nwrite) == -1)
			err(1, "write");
		iov += nwrite;
		veclen -= nwrite;
	}
	return 0;
}

} // namespace pbsd::bin_echo::b0171
