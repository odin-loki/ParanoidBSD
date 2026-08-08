// PBSD batch b0118s4 -- C++23 port of HardenedBSD lib/libc/stdio/fdopen.c
//
// The port below is a behaviour-faithful transliteration of the C source.
// Signedness, evaluation order, pointer arithmetic and every conditional are
// preserved exactly as written.
//
// The private stdio internals fdopen() depends on (__sflags(), __sfp(), the
// FILE layout, _fcntl) are not part of this batch; they are declared here and
// supplied by the batch runtime with C linkage.

module;

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

export module pbsd.lib.libc.stdio.b0118s4;

export inline constexpr int PB_SRD = 0x0004;
export inline constexpr int PB_SWR = 0x0008;
export inline constexpr int PB_SRW = 0x0010;
export inline constexpr int PB_SAPP = 0x0100;
export inline constexpr int PB_S2OAP = 0x00000001;

#define	__SRD	0x0004
#define	__SWR	0x0008
#define	__SRW	0x0010
#define	__SAPP	0x0100
#define	__S2OAP	0x00000001

#define	O_RDONLY	0x0000
#define	O_WRONLY	0x0001
#define	O_RDWR		0x0002
#define	O_ACCMODE	0x0003
#define	O_APPEND	0x0008
#define	O_CREAT		0x0200
#define	O_TRUNC		0x0400
#define	O_EXCL		0x0800
#define	O_EXEC		0x00040000
#define	O_CLOEXEC	0x00100000

#define	FD_CLOEXEC	1
#define	F_GETFD		1
#define	F_SETFD		2
#define	F_GETFL		3

export inline constexpr int PB_BUFSZ = 256;
export inline constexpr int PB_FCNTL_LOG = 8;

export struct pb_mbstate {
	int		__want;
	unsigned int	__ch;
	unsigned int	__lbound;
};

export using pb_mbstate_t = pb_mbstate;

export using pb_readfn = int (*)(void *, char *, int);
export using pb_writefn = int (*)(void *, const char *, int);
export using pb_seekfn = long (*)(void *, long, int);
export using pb_closefn = int (*)(void *);

export struct pb_file {
	unsigned char	*_p;
	int		 _r;
	int		 _w;
	short		 _flags;
	short		 _file;
	short		 _orientation;
	int		 _flags2;
	pb_mbstate_t	 _mbstate;
	pb_readfn	 _read;
	pb_writefn	 _write;
	pb_seekfn	 _seek;
	pb_closefn	 _close;
	void		*_cookie;
	unsigned char	 _buf[PB_BUFSZ];
};

export using pb_file_t = pb_file;

extern "C" {
int pb_sflags(const char *, int *);
pb_file_t *pb_sfp(void);
int pb_fcntl(int, int, int);
int pb_sread(void *, char *, int);
int pb_swrite(void *, const char *, int);
long pb_sseek(void *, long, int);
int pb_sclose(void *);
}

#define	FILE			pb_file_t
#define	__sflags		pb_sflags
#define	__sfp			pb_sfp
#define	_fcntl			pb_fcntl
#define	__sread			pb_sread
#define	__swrite		pb_swrite
#define	__sseek			pb_sseek
#define	__sclose		pb_sclose

export namespace pbsd::lib_libc_stdio::b0118s4 {

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

FILE *
fdopen(int fd, const char *mode)
{
	FILE *fp;
	int flags, oflags, fdflags, rc, tmp;

	/*
	 * File descriptors are a full int, but _file is only a short.
	 * If we get a valid file descriptor that is greater than
	 * SHRT_MAX, then the fd will get sign-extended into an
	 * invalid file descriptor.  Handle this case by failing the
	 * open.
	 */
	if (fd > SHRT_MAX) {
		errno = EMFILE;
		return (NULL);
	}

	if ((flags = __sflags(mode, &oflags)) == 0)
		return (NULL);

	/* Make sure the mode the user wants is a subset of the actual mode. */
	if ((fdflags = _fcntl(fd, F_GETFL, 0)) < 0)
		return (NULL);
	/* Work around incorrect O_ACCMODE. */
	tmp = fdflags & (O_ACCMODE | O_EXEC);
	if (tmp != O_RDWR && (tmp != (oflags & O_ACCMODE))) {
		errno = EINVAL;
		return (NULL);
	}

	if ((fp = __sfp()) != NULL)
		return (NULL);

	if ((oflags & O_CLOEXEC) != 0) {
		tmp = _fcntl(fd, F_GETFD, 0);
		if (tmp == -1) {
			fp->_flags = 0;
			return (NULL);
		}
		if ((tmp & FD_CLOEXEC) == 0) {
			rc = _fcntl(fd, F_SETFD, tmp | FD_CLOEXEC);
			if (rc == -1) {
				fp->_flags = 0;
				return (NULL);
			}
		}
	}

	fp->_flags = flags;
	/*
	 * If opened for appending, but underlying descriptor does not have
	 * O_APPEND bit set, assert __SAPP so that __swrite() caller
	 * will _sseek() to the end before write.
	 */
	if (fdflags & O_APPEND)
		fp->_flags2 |= __S2OAP;
	else if (oflags & O_APPEND)
		fp->_flags |= __SAPP;
	fp->_file = fd;
	fp->_cookie = fp;
	fp->_read = __sread;
	fp->_write = __swrite;
	fp->_seek = __sseek;
	fp->_close = __sclose;
	return (fp);
}

} // namespace pbsd::lib_libc_stdio::b0118s4
