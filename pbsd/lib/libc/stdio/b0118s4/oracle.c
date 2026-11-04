/*
 * oracle.c -- reference implementation for batch b0118s4 (fdopen.c).
 *
 * The source file of this batch is concatenated below with every function
 * renamed with a "ref_" prefix.  The function bodies are otherwise unmodified.
 *
 * fdopen() depends on private FreeBSD stdio internals (__sflags(), __sfp(),
 * the FILE layout, _fcntl).  A self-contained mock of exactly those internals
 * is provided first; it is compiled into this translation unit and shared
 * verbatim by the C++ port so that any observed difference is a difference
 * between the two function bodies.
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

#define	__SRD	0x0004		/* OK to read */
#define	__SWR	0x0008		/* OK to write */
#define	__SRW	0x0010		/* open for reading & writing */
#define	__SAPP	0x0100		/* fdopen()ed in append mode */

#define	__S2OAP	0x00000001	/* O_APPEND mode is set */

/* Mock of <fcntl.h> (FreeBSD values). */
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

#define	PB_BUFSZ	256
#define	PB_FCNTL_LOG	8

typedef struct pb_mbstate {
	int		__want;
	unsigned int	__ch;
	unsigned int	__lbound;
} pb_mbstate_t;

typedef int (*pb_readfn)(void *, char *, int);
typedef int (*pb_writefn)(void *, const char *, int);
typedef long (*pb_seekfn)(void *, long, int);
typedef int (*pb_closefn)(void *);

typedef struct pb_file {
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
} pb_file_t;

/* ------------------------------------------------------------------ */
/* Mock FILE plumbing.                                                 */
/* ------------------------------------------------------------------ */

int
pb_sflags(const char *mode, int *optr)
{
	int ret, m, o;

	switch (*mode++) {
	case 'r':	/* open for reading */
		ret = __SRD;
		m = O_RDONLY;
		o = 0;
		break;
	case 'w':	/* open for writing */
		ret = __SWR;
		m = O_WRONLY;
		o = O_CREAT | O_TRUNC;
		break;
	case 'a':	/* open for appending */
		ret = __SWR;
		m = O_WRONLY;
		o = O_CREAT | O_APPEND;
		break;
	default:	/* illegal mode */
		errno = EINVAL;
		return (0);
	}

	for (;;) {
		switch (*mode++) {
		case '\0':
			break;
		case '+':
			ret = __SRW;
			m = O_RDWR;
			continue;
		case 'b':
			continue;
		case 'x':
			o |= O_EXCL;
			continue;
		case 'e':
			o |= O_CLOEXEC;
			continue;
		default:
			continue;
		}
		break;
	}

	*optr = m | o;
	return (ret);
}

int pb_fcntl_getfl_ret = 0;
int pb_fcntl_getfd_ret = 0;
int pb_fcntl_setfd_ret = 0;
int pb_fcntl_errno = EBADF;
int pb_fcntl_log_n = 0;
int pb_fcntl_log_fd[PB_FCNTL_LOG];
int pb_fcntl_log_cmd[PB_FCNTL_LOG];
int pb_fcntl_log_arg[PB_FCNTL_LOG];

void
pb_fcntl_reset(void)
{
	int i;

	pb_fcntl_log_n = 0;
	for (i = 0; i < PB_FCNTL_LOG; i++) {
		pb_fcntl_log_fd[i] = 0;
		pb_fcntl_log_cmd[i] = 0;
		pb_fcntl_log_arg[i] = 0;
	}
}

int
pb_fcntl(int fd, int cmd, int arg)
{
	int r;

	if (pb_fcntl_log_n < PB_FCNTL_LOG) {
		pb_fcntl_log_fd[pb_fcntl_log_n] = fd;
		pb_fcntl_log_cmd[pb_fcntl_log_n] = cmd;
		pb_fcntl_log_arg[pb_fcntl_log_n] = arg;
	}
	pb_fcntl_log_n++;
	switch (cmd) {
	case F_GETFL:
		r = pb_fcntl_getfl_ret;
		break;
	case F_GETFD:
		r = pb_fcntl_getfd_ret;
		break;
	case F_SETFD:
		r = pb_fcntl_setfd_ret;
		break;
	default:
		r = -1;
		break;
	}
	if (r < 0)
		errno = pb_fcntl_errno;
	return (r);
}

int pb_sfp_fail = 0;
pb_file_t *pb_sfp_last = NULL;

pb_file_t *
pb_sfp(void)
{
	pb_file_t *fp;

	pb_sfp_last = NULL;
	if (pb_sfp_fail) {
		errno = EMFILE;
		return (NULL);
	}
	fp = (pb_file_t *)calloc(1, sizeof(*fp));
	if (fp == NULL)
		return (NULL);
	memset(fp->_buf, 0xaa, sizeof(fp->_buf));
	fp->_flags = 1;		/* reserve this file */
	fp->_file = -1;
	pb_sfp_last = fp;
	return (fp);
}

int
pb_sread(void *cookie, char *buf, int n)
{
	(void)cookie; (void)buf; (void)n;
	return (0);
}

int
pb_swrite(void *cookie, const char *buf, int n)
{
	(void)cookie; (void)buf;
	return (n);
}

long
pb_sseek(void *cookie, long off, int whence)
{
	(void)cookie; (void)whence;
	return (off);
}

int
pb_sclose(void *cookie)
{
	(void)cookie;
	return (0);
}

/* ------------------------------------------------------------------ */
/* Glue so that the function bodies below can stay verbatim.           */
/* ------------------------------------------------------------------ */

#define	FILE			pb_file_t
#define	__sflags		pb_sflags
#define	__sfp			pb_sfp
#define	_fcntl			pb_fcntl
#define	__sread			pb_sread
#define	__swrite		pb_swrite
#define	__sseek			pb_sseek
#define	__sclose		pb_sclose

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
ref_fdopen(int fd, const char *mode)
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

	if ((fp = __sfp()) == NULL)
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
