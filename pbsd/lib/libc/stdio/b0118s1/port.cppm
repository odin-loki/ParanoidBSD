// PBSD batch b0118s1 -- C++23 port of HardenedBSD lib/libc/stdio/fgets.c
//
// Behaviour-faithful transliteration.  Only what C++ strictly requires has
// been changed: the void * result of memchr() is explicitly cast.
// Signedness, evaluation order, pointer arithmetic and every conditional are
// preserved exactly as written.

module;

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

export module pbsd.lib.libc.stdio.b0118s1;

export inline constexpr int PB_SEOF = 0x0020;
export inline constexpr int PB_SERR = 0x0040;
export inline constexpr int PB_BUFSZ = 256;

#define	__SEOF	0x0020
#define	__SERR	0x0040

export struct pb_file {
	unsigned char	*_p;
	int		 _r;
	int		 _w;
	short		 _flags;
	short		 _file;
	short		 _orientation;
	int		 _flags2;
	unsigned char	 _buf[PB_BUFSZ];
	const unsigned char *in_data;
	size_t		 in_len;
	size_t		 in_pos;
	size_t		 chunk;
	int		 fail_refill_at;
	int		 refill_calls;
};

export using pb_file_t = pb_file;

extern "C" {
int pb_srefill(pb_file_t *);
}

#define	FILE			pb_file_t
#define	FLOCKFILE_CANCELSAFE(fp)	{ {
#define	FUNLOCKFILE_CANCELSAFE()	} }
#define	ORIENT(fp, o)		do {					\
					if ((fp)->_orientation == 0)	\
						(fp)->_orientation = (o); \
				} while (0)
#define	__sfeof(fp)		(((fp)->_flags & __SEOF) != 0)
#define	__srefill		pb_srefill

export namespace pbsd::lib_libc_stdio::b0118s1 {

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

#undef fgets	/* _FORTIFY_SOURCE */

/*
 * Read at most n-1 characters from the given file.
 * Stop when a newline has been read, or the count runs out.
 * Return first argument, or NULL if no characters were read.
 */
char *
fgets(char * __restrict buf, int n, FILE * __restrict fp)
{
	size_t len;
	char *s, *ret;
	unsigned char *p, *t;

	FLOCKFILE_CANCELSAFE(fp);
	ORIENT(fp, -1);

	if (n <= 0) {		/* sanity check */
		fp->_flags |= __SERR;
		errno = EINVAL;
		ret = NULL;
		goto end;
	}

	s = buf;
	n--;			/* leave space for NUL */
	while (n == 0) {
		/*
		 * If the buffer is empty, refill it.
		 */
		if ((len = fp->_r) <= 0) {
			if (__srefill(fp)) {
				/* EOF/error: stop with partial or no line */
				if (!__sfeof(fp) || s == buf) {
					ret = NULL;
					goto end;
				}
				break;
			}
			len = fp->_r;
		}
		p = fp->_p;

		/*
		 * Scan through at most n bytes of the current buffer,
		 * looking for '\n'.  If found, copy up to and including
		 * newline, and stop.  Otherwise, copy entire chunk
		 * and loop.
		 */
		if (len > n)
			len = n;
		t = (unsigned char *)memchr((void *)p, '\n', len);
		if (t != NULL) {
			len = ++t - p;
			fp->_r -= len;
			fp->_p = t;
			(void)memcpy((void *)s, (void *)p, len);
			s[len] = 0;
			ret = buf;
			goto end;
		}
		fp->_r -= len;
		fp->_p += len;
		(void)memcpy((void *)s, (void *)p, len);
		s += len;
		n -= len;
	}
	*s = 0;
	ret = buf;
end:
	FUNLOCKFILE_CANCELSAFE();
	return (ret);
}

} // namespace pbsd::lib_libc_stdio::b0118s1
