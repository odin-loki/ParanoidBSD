/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kevin Fall.
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

module;

#include <cerrno>
#include <climits>
#include <csetjmp>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>

export module pbsd.bin.cat.b0163;

export namespace pbsd::bin_cat::b0163 {

#ifndef MAXPHYS
#define MAXPHYS (128 * 1024)
#endif

#define	PHYSPAGES_THRESHOLD (32 * 1024)
#define	BUFSIZE_MAX (2 * 1024 * 1024)
#define	BUFSIZE_SMALL (MAXPHYS)

#ifndef MIN
#define	MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define	MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

inline int rval;
inline const char *filename;

inline jmp_buf port_err_jmp;
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
err(int eval, const char *fmt, ...)
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

inline void
warn(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	std::vfprintf(stderr, fmt, ap);
	va_end(ap);
	std::fputc('\n', stderr);
}

inline ssize_t
in_kernel_copy(int rfd)
{
	int wfd;
	ssize_t ret;

	wfd = fileno(stdout);
	ret = 1;

	while (ret > 0)
		ret = copy_file_range(rfd, NULL, wfd, NULL, SSIZE_MAX, 0);

	return (ret);
}

inline void
raw_cat(int rfd)
{
	long pagesize;
	int off, wfd;
	ssize_t nr, nw;
	static size_t bsize;
	static char *buf = NULL;
	struct stat sbuf;

	wfd = fileno(stdout);
	if (buf == NULL) {
		if (fstat(wfd, &sbuf))
			err(1, "stdout");
		if (S_ISREG(sbuf.st_mode)) {
			/* If there's plenty of RAM, use a large copy buffer */
			if (sysconf(_SC_PHYS_PAGES) > PHYSPAGES_THRESHOLD)
				bsize = MIN(BUFSIZE_MAX, MAXPHYS * 8);
			else
				bsize = BUFSIZE_SMALL;
		} else {
			bsize = sbuf.st_blksize;
			pagesize = sysconf(_SC_PAGESIZE);
			if (pagesize > 0)
				bsize = MAX(bsize, (size_t)pagesize);
		}
		if ((buf = (char *)malloc(bsize)) == NULL)
			err(1, "malloc() failure of IO buffer");
	}
	while ((nr = read(rfd, buf, bsize)) > 0)
		for (off = 0; nr; nr -= nw, off += nw)
			if ((nw = write(wfd, buf + off, (size_t)nr)) < 0)
				err(1, "stdout");
	if (nr < 0) {
		warn("%s", filename);
		rval = 1;
	}
}

} /* namespace pbsd::bin_cat::b0163 */
