/*
 * oracle.c -- reference implementations for batch b0118s2 (fwrite.c).
 *
 * The source file of this batch is concatenated below with every function
 * renamed with a "ref_" prefix.  The function bodies are otherwise unmodified.
 *
 * fwrite() depends on private FreeBSD stdio internals (__sfvwrite(), the FILE
 * layout, ORIENT).  A self-contained mock of exactly those internals is
 * provided first; it is compiled into this translation unit and shared verbatim
 * by the C++ port so that any observed difference is a difference between the
 * two function bodies.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/* ------------------------------------------------------------------ */
/* Mock of <stdio.h> private flags (FreeBSD sys/_stdio.h values).      */
/* ------------------------------------------------------------------ */

#define	__SERR	0x0040		/* found error */

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
	unsigned char	*sink;
	size_t		 sink_size;
	size_t		 sink_cap;
	size_t		 sink_len;
	int		 sfvwrite_calls;
} pb_file_t;

struct __siov {
	void	*iov_base;
	size_t	 iov_len;
};

struct __suio {
	struct __siov	*uio_iov;
	int		 uio_iovcnt;
	int		 uio_resid;
};

/* ------------------------------------------------------------------ */
/* Mock FILE plumbing.                                                 */
/* ------------------------------------------------------------------ */

void
pb_file_init(pb_file_t *fp, unsigned char *sink, size_t sink_size,
    size_t sink_cap)
{
	memset(fp, 0, sizeof(*fp));
	memset(fp->_buf, 0xaa, sizeof(fp->_buf));
	fp->_file = -1;
	fp->sink = sink;
	fp->sink_size = sink_size;
	fp->sink_cap = sink_cap;
	if (sink != NULL)
		memset(sink, 0x7f, sink_size);
}

int
pb_sfvwrite(pb_file_t *fp, struct __suio *uio)
{
	struct __siov *iov;
	size_t want, avail, put;
	int i, err;

	fp->sfvwrite_calls++;
	if (uio->uio_resid == 0)
		return (0);
	err = 0;
	iov = uio->uio_iov;
	for (i = 0; i < uio->uio_iovcnt; i++) {
		want = iov[i].iov_len;
		avail = fp->sink_cap - fp->sink_len;
		put = want < avail ? want : avail;
		if (put != 0)
			memcpy(fp->sink + fp->sink_len, iov[i].iov_base, put);
		fp->sink_len += put;
		uio->uio_resid -= (int)put;
		if (put < want) {
			err = 1;
			break;
		}
	}
	if (err) {
		fp->_flags |= __SERR;
		return (-1);
	}
	return (0);
}

/* ------------------------------------------------------------------ */
/* Glue so that the function bodies below can stay verbatim.           */
/* ------------------------------------------------------------------ */

#define	FILE			pb_file_t
#define	FLOCKFILE_CANCELSAFE(fp)	{ {
#define	FUNLOCKFILE_CANCELSAFE()	} }
#define	ORIENT(fp, o)		do {					\
					if ((fp)->_orientation == 0)	\
						(fp)->_orientation = (o); \
				} while (0)
#define	__sfvwrite		pb_sfvwrite

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
 * Write `count' objects (each size `size') from memory to the given file.
 * Return the number of whole objects written.
 */
size_t
ref_fwrite_unlocked(const void * __restrict buf, size_t size, size_t count,
    FILE * __restrict fp)
{
	size_t n;
	struct __suio uio;
	struct __siov iov;

	/*
	 * ANSI and SUSv2 require a return value of 0 if size or count are 0.
	 */
	if ((count == 0) || (size == 0))
		return (0);

	/*
	 * Check for integer overflow.  As an optimization, first check that
	 * at least one of {count, size} is at least 2^16, since if both
	 * values are less than that, their product can't possibly overflow
	 * (size_t is always at least 32 bits on FreeBSD).
	 */
	if (((count | size) > 0xFFFF) &&
	    (count > SIZE_MAX / size)) {
		errno = EINVAL;
		fp->_flags |= __SERR;
		return (0);
	}

	n = count * size;

	iov.iov_base = (void *)buf;
	uio.uio_resid = iov.iov_len = n;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;

	ORIENT(fp, -1);
	/*
	 * The usual case is success (__sfvwrite returns 0);
	 * skip the divide if this happens, since divides are
	 * generally slow and since this occurs whenever size==0.
	 */
	if (__sfvwrite(fp, &uio) != 0)
	    count = (n - uio.uio_resid) / size;
	return (count);
}

size_t
ref_fwrite(const void * __restrict buf, size_t size, size_t count,
    FILE * __restrict fp)
{
	size_t n;

	FLOCKFILE_CANCELSAFE(fp);
	n = ref_fwrite_unlocked(buf, size, count, fp);
	FUNLOCKFILE_CANCELSAFE();
	return (n);
}
