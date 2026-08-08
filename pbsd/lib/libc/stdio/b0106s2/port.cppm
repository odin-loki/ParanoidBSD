module;

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <climits>
#include <cstddef>
#include <cstdio>
#include <cstring>

export module pbsd.lib.libc.stdio.b0106s2;

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

struct __siov {
	void *iov_base;
	size_t iov_len;
};

struct __suio {
	struct __siov *uio_iov;
	int uio_iovcnt;
	int uio_resid;
};

extern "C" {
void pbsd_shim_orient(struct pbsd_shim_file *, int);
void pbsd_shim_flockfile(struct pbsd_shim_file *);
void pbsd_shim_funlockfile(struct pbsd_shim_file *);
int pbsd_shim_sfvwrite(struct pbsd_shim_file *, struct __suio *);
}

export namespace pbsd::lib_libc_stdio::b0106s2 {

using shim_file = pbsd_shim_file;

#define	__SWR	0x0008
#define	__SERR	0x0040

#define FLOCKFILE_CANCELSAFE(fp)					\
	{								\
		shim_file *_shim_lockfp = (fp);				\
		pbsd_shim_flockfile(_shim_lockfp);
#define FUNLOCKFILE_CANCELSAFE()					\
		pbsd_shim_funlockfile(_shim_lockfp);			\
	}

#define ORIENT(fp, dir) pbsd_shim_orient((fp), (dir))
#define __sfvwrite(fp, uio) pbsd_shim_sfvwrite((fp), (uio))

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
fputs_unlocked(const char * __restrict s, shim_file * __restrict fp)
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
	if (retval != 0)
		return (iov.iov_len > INT_MAX ? INT_MAX : iov.iov_len);
	return (retval);
}

int
fputs(const char * __restrict s, shim_file * __restrict fp)
{
	int retval;

	FLOCKFILE_CANCELSAFE(fp);
	retval = fputs_unlocked(s, fp);
	FUNLOCKFILE_CANCELSAFE();
	return (retval);
}

} /* namespace pbsd::lib_libc_stdio::b0106s2 */
