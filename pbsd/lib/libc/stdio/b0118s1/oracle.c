/*
 * oracle.c -- reference implementation for batch b0118s1 (fgets.c).
 *
 * The source file of this batch is included below with the function renamed
 * with a "ref_" prefix.  The function body is otherwise unmodified.
 *
 * A self-contained mock of the private FreeBSD stdio FILE internals used by
 * fgets is provided first.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define	__SEOF	0x0020
#define	__SERR	0x0040

#define	PB_BUFSZ	256

typedef struct pb_file {
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
} pb_file_t;

void
pb_file_init(pb_file_t *fp)
{
	memset(fp, 0, sizeof(*fp));
	memset(fp->_buf, 0xaa, sizeof(fp->_buf));
	fp->_file = -1;
}

int
pb_srefill(pb_file_t *fp)
{
	size_t n;

	fp->refill_calls++;
	if (fp->fail_refill_at != 0 && fp->refill_calls == fp->fail_refill_at) {
		fp->_flags |= __SERR;
		fp->_p = fp->_buf;
		fp->_r = 0;
		return (-1);
	}
	if (fp->in_pos >= fp->in_len) {
		fp->_flags |= __SEOF;
		fp->_p = fp->_buf;
		fp->_r = 0;
		return (-1);
	}
	n = fp->in_len - fp->in_pos;
	if (fp->chunk != 0 && n > fp->chunk)
		n = fp->chunk;
	if (n > PB_BUFSZ)
		n = PB_BUFSZ;
	memcpy(fp->_buf, fp->in_data + fp->in_pos, n);
	fp->in_pos += n;
	fp->_p = fp->_buf;
	fp->_r = (int)n;
	return (0);
}

void
pb_file_input(pb_file_t *fp, const unsigned char *data, size_t len,
    size_t chunk, int fail_at, int prefill)
{
	fp->in_data = data;
	fp->in_len = len;
	fp->in_pos = 0;
	fp->chunk = chunk;
	fp->fail_refill_at = fail_at;
	fp->refill_calls = 0;
	fp->_p = NULL;
	fp->_r = 0;
	if (prefill) {
		(void)pb_srefill(fp);
		fp->refill_calls = 0;
		fp->_flags = (short)(fp->_flags & ~(__SEOF | __SERR));
	}
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
ref_fgets(char * __restrict buf, int n, FILE * __restrict fp)
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
	while (n != 0) {
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
		t = memchr((void *)p, '\n', len);
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
