/*
 * Reference oracle for batch b0106s2 (fputs.c).
 *
 * The original HardenedBSD fputs.c is emitted below with every function renamed
 * with a "ref_" prefix.  Function bodies are UNMODIFIED; renaming is done with
 * #define.  FreeBSD-private FILE internals are supplied by the test substrate
 * below so fputs_unlocked(3) and fputs(3) compile and run without the real
 * libc.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

/*
 * FILE flag bits, from FreeBSD <stdio.h>.
 */
#define __SLBF	0x0001
#define __SNBF	0x0002
#define __SRD	0x0004
#define __SWR	0x0008
#define __SRW	0x0010
#define __SEOF	0x0020
#define __SERR	0x0040
#define __SMBF	0x0080
#define __SAPP	0x0100
#define __SSTR	0x0200

struct pbsd_shim_bufdesc {
	unsigned char *_base;
	int _size;
};

struct pbsd_shim_file {
	unsigned char *_p;
	int _r;
	int _w;
	short _flags;
	short _file;
	struct pbsd_shim_bufdesc _bf;
	int _lbfsize;
	int _orientation;
	int _lockdepth;
	int _locktotal;
};

/* struct __suio / struct __siov, from FreeBSD's fvwrite.h. */
struct __siov {
	void *iov_base;
	size_t iov_len;
};

struct __suio {
	struct __siov *uio_iov;
	int uio_iovcnt;
	int uio_resid;
};

int pbsd_shim_orient_calls;
int pbsd_shim_orient_last_dir;
int pbsd_shim_sfvwrite_calls;
int pbsd_shim_sfvwrite_entry_iovcnt;
int pbsd_shim_sfvwrite_entry_resid;
size_t pbsd_shim_sfvwrite_entry_iovlen;
void *pbsd_shim_sfvwrite_entry_iovbase;
int pbsd_shim_flockfile_calls;
int pbsd_shim_funlockfile_calls;

void
pbsd_shim_orient(struct pbsd_shim_file *fp, int dir)
{
	pbsd_shim_orient_calls++;
	pbsd_shim_orient_last_dir = dir;
	if (fp->_orientation == 0)
		fp->_orientation = dir;
}

void
pbsd_shim_flockfile(struct pbsd_shim_file *fp)
{
	pbsd_shim_flockfile_calls++;
	if (fp != NULL) {
		fp->_lockdepth++;
		fp->_locktotal++;
	}
}

void
pbsd_shim_funlockfile(struct pbsd_shim_file *fp)
{
	pbsd_shim_funlockfile_calls++;
	if (fp != NULL) {
		fp->_lockdepth--;
		fp->_locktotal++;
	}
}

/*
 * __sfvwrite(): copy uio_resid bytes from the iovecs into the stream.
 */
int
pbsd_shim_sfvwrite(struct pbsd_shim_file *fp, struct __suio *uio)
{
	struct __siov *iov;
	const unsigned char *p;
	size_t n;
	int resid, i;

	pbsd_shim_sfvwrite_calls++;
	pbsd_shim_sfvwrite_entry_iovcnt = uio->uio_iovcnt;
	pbsd_shim_sfvwrite_entry_resid = uio->uio_resid;
	pbsd_shim_sfvwrite_entry_iovlen = uio->uio_iov[0].iov_len;
	pbsd_shim_sfvwrite_entry_iovbase = uio->uio_iov[0].iov_base;
	if ((resid = uio->uio_resid) == 0)
		return (0);
	if ((fp->_flags & __SWR) == 0) {
		fp->_flags |= __SERR;
		return (EOF);
	}
	for (i = 0; i < uio->uio_iovcnt && resid > 0; i++) {
		iov = uio->uio_iov + i;
		p = (const unsigned char *)iov->iov_base;
		for (n = 0; n < iov->iov_len && resid > 0; n++) {
			if (fp->_w <= 0) {
				fp->_flags |= __SERR;
				return (EOF);
			}
			*fp->_p = p[n];
			fp->_p++;
			fp->_w--;
			resid--;
		}
	}
	if (resid != 0) {
		fp->_flags |= __SERR;
		return (EOF);
	}
	return (0);
}

size_t
pbsd_shim_file_layout(int which)
{
	switch (which) {
	case 0:
		return (sizeof(struct pbsd_shim_file));
	case 1:
		return (offsetof(struct pbsd_shim_file, _p));
	case 2:
		return (offsetof(struct pbsd_shim_file, _w));
	case 3:
		return (offsetof(struct pbsd_shim_file, _flags));
	case 4:
		return (offsetof(struct pbsd_shim_file, _orientation));
	case 5:
		return (offsetof(struct pbsd_shim_file, _lockdepth));
	case 6:
		return (offsetof(struct pbsd_shim_file, _locktotal));
	case 7:
		return (sizeof(struct __suio));
	case 8:
		return (sizeof(struct __siov));
	default:
		return ((size_t)-1);
	}
}

#undef FLOCKFILE_CANCELSAFE
#undef FUNLOCKFILE_CANCELSAFE

#define FILE struct pbsd_shim_file

#define FLOCKFILE_CANCELSAFE(fp)					\
	{								\
		FILE *_shim_lockfp = (fp);				\
		pbsd_shim_flockfile(_shim_lockfp);
#define FUNLOCKFILE_CANCELSAFE()					\
		pbsd_shim_funlockfile(_shim_lockfp);			\
	}

#define ORIENT(fp, dir) pbsd_shim_orient((fp), (dir))
#define __sfvwrite(fp, uio) pbsd_shim_sfvwrite((fp), (uio))

#undef fputs_unlocked
#undef fputs
#define fputs_unlocked ref_fputs_unlocked
#define fputs ref_fputs

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
 * Write the given string to the given file.
 */
int
fputs_unlocked(const char * __restrict s, FILE * __restrict fp)
{
	int retval;
	struct __suio uio;
	struct __siov iov;

	iov.iov_base = (void *)s;
	uio.uio_resid = iov.iov_len = strlen(s);
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	ORIENT(fp, -1);
	retval = __sfvwrite(fp, &uio);
	if (retval == 0)
		return (iov.iov_len > INT_MAX ? INT_MAX : iov.iov_len);
	return (retval);
}

int
fputs(const char * __restrict s, FILE * __restrict fp)
{
	int retval;

	FLOCKFILE_CANCELSAFE(fp);
	retval = fputs_unlocked(s, fp);
	FUNLOCKFILE_CANCELSAFE();
	return (retval);
}
