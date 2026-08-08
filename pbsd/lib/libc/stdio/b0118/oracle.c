/*
 * oracle.c -- reference implementations for batch b0118.
 *
 * The four source files of this batch are concatenated below with every
 * function renamed with a "ref_" prefix.  The function bodies are otherwise
 * unmodified.
 *
 * The batch operates on the private FreeBSD stdio FILE internals (_p, _r,
 * _flags, __srefill(), __sfvwrite(), __sflags(), __sfp(), the xlocale ctype
 * conversion vector, ...) which do not exist on this host.  A self contained
 * mock of exactly those internals is provided first; it is compiled into this
 * translation unit and shared verbatim by the C++ port, so that the reference
 * and the port execute against bit-identical infrastructure and any observed
 * difference is a difference between the two function bodies.
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
#define	__SEOF	0x0020		/* found EOF */
#define	__SERR	0x0040		/* found error */
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
	unsigned char	*_p;		/* current position in (some) buffer */
	int		 _r;		/* read space left for getc() */
	int		 _w;		/* write space left for putc() */
	short		 _flags;	/* flags, below */
	short		 _file;		/* fileno, if Unix descriptor */
	short		 _orientation;	/* orientation for fwide() */
	int		 _flags2;
	pb_mbstate_t	 _mbstate;	/* multibyte conversion state */
	pb_readfn	 _read;
	pb_writefn	 _write;
	pb_seekfn	 _seek;
	pb_closefn	 _close;
	void		*_cookie;
	/* mock plumbing */
	unsigned char	 _buf[PB_BUFSZ];
	const unsigned char *in_data;
	size_t		 in_len;
	size_t		 in_pos;
	size_t		 chunk;
	int		 fail_refill_at;
	int		 refill_calls;
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

struct xlocale_ctype {
	size_t	(*__mbsnrtowcs)(wchar_t * __restrict, const char ** __restrict,
		    size_t, size_t, pb_mbstate_t * __restrict);
	int	(*__mbsinit)(const pb_mbstate_t *);
};

typedef struct xlocale_ctype *pb_locale_t;

/* ------------------------------------------------------------------ */
/* Mock character conversion (a UTF-8 locale and a single byte locale).*/
/* ------------------------------------------------------------------ */

static size_t
pb_utf8_mbrtowc(wchar_t *pwc, const char *s, size_t n, pb_mbstate_t *ps)
{
	const unsigned char *us;
	unsigned int ch, wch, lbound;
	int want, i, mask;

	if (n == 0)
		return ((size_t)-2);
	us = (const unsigned char *)s;
	if (ps->__want == 0) {
		ch = us[0];
		if ((ch & 0x80) == 0) {
			mask = 0x7f;
			want = 1;
			lbound = 0;
		} else if ((ch & 0xe0) == 0xc0) {
			mask = 0x1f;
			want = 2;
			lbound = 0x80;
		} else if ((ch & 0xf0) == 0xe0) {
			mask = 0x0f;
			want = 3;
			lbound = 0x800;
		} else if ((ch & 0xf8) == 0xf0) {
			mask = 0x07;
			want = 4;
			lbound = 0x10000;
		} else {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		wch = ch & (unsigned int)mask;
		us++;
		i = 1;
	} else {
		want = ps->__want;
		lbound = ps->__lbound;
		wch = ps->__ch;
		i = 0;
	}
	for (; i < want && (size_t)i < n; i++) {
		if ((us[0] & 0xc0) != 0x80) {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		wch = (wch << 6) | (unsigned int)(us[0] & 0x3f);
		us++;
	}
	if (i < want) {
		ps->__want = want - i;
		ps->__lbound = lbound;
		ps->__ch = wch;
		return ((size_t)-2);
	}
	if (wch < lbound) {
		errno = EILSEQ;
		return ((size_t)-1);
	}
	ps->__want = 0;
	ps->__ch = 0;
	ps->__lbound = 0;
	if (pwc != NULL)
		*pwc = (wchar_t)wch;
	return (wch == 0 ? 0 : (size_t)want);
}

static size_t
pb_sb_mbrtowc(wchar_t *pwc, const char *s, size_t n, pb_mbstate_t *ps)
{
	unsigned int ch;

	(void)ps;
	if (n == 0)
		return ((size_t)-2);
	ch = (unsigned char)s[0];
	if (pwc != NULL)
		*pwc = (wchar_t)ch;
	return (ch == 0 ? 0 : 1);
}

static size_t
pb_mbsnrtowcs_std(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, pb_mbstate_t * __restrict ps,
    size_t (*pmbrtowc)(wchar_t *, const char *, size_t, pb_mbstate_t *))
{
	const char *s;
	size_t nchr;
	wchar_t wc;
	size_t nb;

	s = *src;
	nchr = 0;

	if (dst == NULL) {
		for (;;) {
			if ((nb = pmbrtowc(&wc, s, nms, ps)) == (size_t)-1)
				return ((size_t)-1);
			else if (nb == 0 || nb == (size_t)-2)
				return (nchr);
			s += nb;
			nms -= nb;
			nchr++;
		}
	}

	while (len-- > 0) {
		if ((nb = pmbrtowc(dst, s, nms, ps)) == (size_t)-1) {
			*src = s;
			return ((size_t)-1);
		} else if (nb == (size_t)-2) {
			*src = s + nms;
			return (nchr);
		} else if (nb == 0) {
			*src = NULL;
			return (nchr);
		}
		s += nb;
		nms -= nb;
		nchr++;
		dst++;
	}
	*src = s;
	return (nchr);
}

static size_t
pb_utf8_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, pb_mbstate_t * __restrict ps)
{
	return (pb_mbsnrtowcs_std(dst, src, nms, len, ps, pb_utf8_mbrtowc));
}

static size_t
pb_sb_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, pb_mbstate_t * __restrict ps)
{
	return (pb_mbsnrtowcs_std(dst, src, nms, len, ps, pb_sb_mbrtowc));
}

static int
pb_utf8_mbsinit(const pb_mbstate_t *ps)
{
	return (ps->__want == 0);
}

static int
pb_sb_mbsinit(const pb_mbstate_t *ps)
{
	(void)ps;
	return (1);
}

struct xlocale_ctype pb_utf8_locale = { pb_utf8_mbsnrtowcs, pb_utf8_mbsinit };
struct xlocale_ctype pb_sb_locale = { pb_sb_mbsnrtowcs, pb_sb_mbsinit };
pb_locale_t pb_global_locale = &pb_utf8_locale;

pb_locale_t
pb_get_locale(void)
{
	return (pb_global_locale);
}

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
#define	locale_t		pb_locale_t
#define	FLOCKFILE_CANCELSAFE(fp)	{ {
#define	FUNLOCKFILE_CANCELSAFE()	} }
#define	ORIENT(fp, o)		do {					\
					if ((fp)->_orientation == 0)	\
						(fp)->_orientation = (o); \
				} while (0)
#define	__sfeof(fp)		(((fp)->_flags & __SEOF) != 0)
#define	__srefill		pb_srefill
#define	__sfvwrite		pb_sfvwrite
#define	__sflags		pb_sflags
#define	__sfp			pb_sfp
#define	_fcntl			pb_fcntl
#define	__sread			pb_sread
#define	__swrite		pb_swrite
#define	__sseek			pb_sseek
#define	__sclose		pb_sclose
#define	__get_locale()		pb_get_locale()
#define	FIX_LOCALE(l)		do {					\
					if ((l) == NULL)		\
						(l) = __get_locale();	\
				} while (0)
#define	XLOCALE_CTYPE(l)	(l)

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

/* lib/libc/stdio/fgets.c */

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

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * [BSD-3-Clause licence text as above]
 */

/* lib/libc/stdio/fwrite.c */

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

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002-2004 Tim J. Robbins.
 * All rights reserved.
 *
 * Copyright (c) 2011 The FreeBSD Foundation
 *
 * Portions of this software were developed by David Chisnall
 * under sponsorship from the FreeBSD Foundation.
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

/* lib/libc/stdio/fgetws.c */

wchar_t *
ref_fgetws_l(wchar_t * __restrict ws, int n, FILE * __restrict fp, locale_t locale)
{
	int sret;
	wchar_t *wsp, *ret;
	size_t nconv;
	const char *src;
	unsigned char *nl;
	FIX_LOCALE(locale);
	struct xlocale_ctype *l = XLOCALE_CTYPE(locale);

	FLOCKFILE_CANCELSAFE(fp);
	ORIENT(fp, 1);

	if (n <= 0) {
		fp->_flags |= __SERR;
		errno = EINVAL;
		goto error;
	}

	wsp = ws;
	if (n == 1)
		goto ok;

	if (fp->_r <= 0 && __srefill(fp))
		/* EOF or ferror */
		goto error;

	sret = 0;
	do {
		src = fp->_p;
		nl = memchr(fp->_p, '\n', fp->_r);
		nconv = l->__mbsnrtowcs(wsp, &src,
		    nl != NULL ? (nl - fp->_p + 1) : fp->_r,
		    n - 1, &fp->_mbstate);
		if (nconv == (size_t)-1) {
			/* Conversion error */
			fp->_flags |= __SERR;
			goto error;
		}
		if (src == NULL) {
			/*
			 * We hit a null byte. Increment the character count,
			 * since mbsnrtowcs()'s return value doesn't include
			 * the terminating null, then resume conversion
			 * after the null.
			 */
			nconv++;
			src = memchr(fp->_p, '\0', fp->_r);
			src++;
		}
		fp->_r -= (unsigned char *)src - fp->_p;
		fp->_p = (unsigned char *)src;
		n -= nconv;
		wsp += nconv;
	} while ((wsp == ws || wsp[-1] != L'\n') && n > 1 && (fp->_r > 0 ||
	    (sret = __srefill(fp)) == 0));
	if (sret && !__sfeof(fp))
		/* ferror */
		goto error;
	if (!l->__mbsinit(&fp->_mbstate)) {
		/* Incomplete character */
		fp->_flags |= __SERR;
		errno = EILSEQ;
		goto error;
	}
	if (wsp == ws)
		/* EOF */
		goto error;
ok:
	*wsp = L'\0';
	ret = ws;
end:
	FUNLOCKFILE_CANCELSAFE();
	return (ret);

error:
	ret = NULL;
	goto end;
}

wchar_t *
ref_fgetws(wchar_t * __restrict ws, int n, FILE * __restrict fp)
{
	return ref_fgetws_l(ws, n, fp, __get_locale());
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * [BSD-3-Clause licence text as above]
 */

/* lib/libc/stdio/fdopen.c */

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
