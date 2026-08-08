/*
 * Reference oracle for batch b0118.
 *
 * hbsd/src/lib/libc/stdio/fgets.c, fwrite.c, fgetws.c, and fdopen.c
 * concatenated with every function renamed with a ref_ prefix.  Function
 * bodies are UNMODIFIED.  Declarations from unavailable FreeBSD/HardenedBSD
 * private headers are supplied below.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <uchar.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef O_VERIFY
#define O_VERIFY 0
#endif

#ifndef O_EXEC
#define O_EXEC 0
#endif

#define EOF (-1)
typedef long fpos_t;

extern void *memcpy(void *, const void *, size_t);
extern void *memchr(const void *, int, size_t);
extern void *memset(void *, int, size_t);
extern size_t mbsnrtowcs(wchar_t * __restrict, const char ** __restrict, size_t,
    size_t, mbstate_t * __restrict);
extern int mbsinit(const mbstate_t *);

struct __sbuf {
	unsigned char *_base;
	int _size;
};

struct __sFILE {
	unsigned char *_p;
	int _r;
	int _w;
	short _flags;
	short _file;
	struct __sbuf _bf;
	int _lbfsize;
	void *_cookie;
	int (*_close)(void *);
	int (*_read)(void *, char *, int);
	fpos_t (*_seek)(void *, fpos_t, int);
	int (*_write)(void *, const char *, int);
	struct __sbuf _ub;
	unsigned char *_up;
	int _ur;
	unsigned char _ubuf[3];
	unsigned char _nbuf[1];
	struct __sbuf _lb;
	int _blksize;
	fpos_t _offset;
	void *_fl_mutex;
	void *_fl_owner;
	int _fl_count;
	int _orientation;
	mbstate_t _mbstate;
	int _flags2;
};
typedef struct __sFILE FILE;
typedef void *locale_t;

#define	__SLBF	0x0001
#define	__SNBF	0x0002
#define	__SRD	0x0004
#define	__SWR	0x0008
#define	__SRW	0x0010
#define	__SEOF	0x0020
#define	__SERR	0x0040
#define	__SMBF	0x0080
#define	__SAPP	0x0100
#define	__SSTR	0x0200
#define	__SOPT	0x0400
#define	__SNPT	0x0800
#define	__SOFF	0x1000
#define	__SMOD	0x2000
#define	__SALC	0x4000
#define	__SIGN	0x8000
#define	__S2OAP	0x0001

#define	__sfeof(p)	(((p)->_flags & __SEOF) != 0)
#define	ORIENT(fp, o)	do {					\
	if ((fp)->_orientation == 0)				\
		(fp)->_orientation = (o);			\
} while (0)

#define	FLOCKFILE_CANCELSAFE(fp)	do { (void)(fp); } while (0)
#define	FUNLOCKFILE_CANCELSAFE()	do { } while (0)

struct __siov {
	void *iov_base;
	size_t iov_len;
};

struct __suio {
	struct __siov *uio_iov;
	int uio_iovcnt;
	int uio_resid;
};

struct xlocale_ctype {
	size_t (*__mbsnrtowcs)(wchar_t * __restrict, const char ** __restrict,
	    size_t, size_t, mbstate_t * __restrict);
	int (*__mbsinit)(const mbstate_t *);
};

#define FIX_LOCALE(l)	((void)(l))
#define XLOCALE_CTYPE(x) (&b0118_ctype)

typedef struct b0118_stream {
	const unsigned char *data;
	size_t len;
	size_t pos;
	int err_on_refill;
	unsigned char *buf;
	size_t buf_cap;
} b0118_stream;

typedef struct b0118_write_ctx {
	unsigned char *out;
	size_t out_cap;
	size_t out_len;
	size_t max_write;
	int fail;
} b0118_write_ctx;

static FILE *b0118_sfp_target;
static struct xlocale_ctype b0118_ctype;

static size_t
b0118_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nmc, size_t len, mbstate_t * __restrict ps)
{
	return (mbsnrtowcs(dst, src, nmc, len, ps));
}

static int
b0118_mbsinit(const mbstate_t *ps)
{
	return (mbsinit(ps));
}

static void
b0118_locale_setup(void)
{
	b0118_ctype.__mbsnrtowcs = b0118_mbsnrtowcs;
	b0118_ctype.__mbsinit = b0118_mbsinit;
}

void *
__get_locale(void)
{
	return ((void *)0);
}

void
b0118_set_sfp_target(FILE *fp)
{
	b0118_sfp_target = fp;
}

int
__srefill(FILE *fp)
{
	b0118_stream *st;

	fp->_r = 0;

	if (fp->_flags & __SEOF)
		return (EOF);

	st = (b0118_stream *)fp->_cookie;
	if (st == NULL) {
		fp->_flags |= __SEOF;
		return (EOF);
	}

	if (st->err_on_refill) {
		fp->_flags |= __SERR;
		errno = EIO;
		return (EOF);
	}

	if (st->pos >= st->len) {
		fp->_flags |= __SEOF;
		return (EOF);
	}

	{
		size_t avail = st->len - st->pos;
		unsigned char *dbuf = st->buf != NULL ? st->buf : fp->_nbuf;
		size_t dcap = st->buf != NULL ? st->buf_cap : 1;
		size_t chunk = avail > dcap ? dcap : avail;
		if (chunk > 128)
			chunk = 128;
		if (chunk > dcap)
			chunk = dcap;

		memcpy(dbuf, st->data + st->pos, chunk);
		st->pos += chunk;
		fp->_bf._base = dbuf;
		fp->_bf._size = (int)dcap;
		fp->_p = dbuf;
		fp->_r = (int)chunk;
		fp->_flags |= __SRD;
	}

	return (0);
}

int
__sfvwrite(FILE *fp, struct __suio *uio)
{
	b0118_write_ctx *ctx;
	struct __siov *iov;
	size_t n, written;
	unsigned char *dst;

	if (uio->uio_iovcnt != 1)
		return (EOF);

	ctx = (b0118_write_ctx *)fp->_cookie;
	if (ctx == NULL)
		return (EOF);

	if (ctx->fail) {
		fp->_flags |= __SERR;
		return (EOF);
	}

	iov = uio->uio_iov;
	n = iov->iov_len;
	if (n != (size_t)uio->uio_resid)
		return (EOF);

	if (ctx->max_write < n)
		n = ctx->max_write;

	written = 0;
	dst = (unsigned char *)iov->iov_base;
	while (written < n) {
		if (ctx->out_len >= ctx->out_cap)
			break;
		ctx->out[ctx->out_len++] = dst[written];
		written++;
	}

	uio->uio_resid -= (int)written;
	if (written != iov->iov_len)
		return (EOF);

	return (0);
}

FILE *
__sfp(void)
{
	FILE *fp;

	if (b0118_sfp_target == NULL)
		return (NULL);

	fp = b0118_sfp_target;
	if (fp->_flags != 0)
		return (NULL);

	fp->_flags = 1;
	fp->_p = NULL;
	fp->_w = 0;
	fp->_r = 0;
	fp->_bf._base = NULL;
	fp->_bf._size = 0;
	fp->_lbfsize = 0;
	fp->_file = -1;
	fp->_ub._base = NULL;
	fp->_ub._size = 0;
	fp->_lb._base = NULL;
	fp->_lb._size = 0;
	fp->_orientation = 0;
	memset(&fp->_mbstate, 0, sizeof(mbstate_t));
	fp->_flags2 = 0;
	return (fp);
}

int
__sflags(const char *mode, int *optr)
{
	int ret, m, o, known;

	switch (*mode++) {

	case 'r':
		ret = __SRD;
		m = O_RDONLY;
		o = 0;
		break;

	case 'w':
		ret = __SWR;
		m = O_WRONLY;
		o = O_CREAT | O_TRUNC;
		break;

	case 'a':
		ret = __SWR;
		m = O_WRONLY;
		o = O_CREAT | O_APPEND;
		break;

	default:
		errno = EINVAL;
		return (0);
	}

	do {
		known = 1;
		switch (*mode++) {
		case 'b':
			break;
		case '+':
			ret = __SRW;
			m = O_RDWR;
			break;
		case 'x':
			o |= O_EXCL;
			break;
		case 'e':
			o |= O_CLOEXEC;
			break;
		case 'v':
			o |= O_VERIFY;
			break;
		default:
			known = 0;
			break;
		}
	} while (known);

	if ((o & O_EXCL) != 0 && m == O_RDONLY) {
		errno = EINVAL;
		return (0);
	}

	*optr = m | o;
	return (ret);
}

int
_fcntl(int fd, int cmd, ...)
{
	va_list ap;
	int arg, ret;

	va_start(ap, cmd);
	arg = va_arg(ap, int);
	va_end(ap);

	if (cmd == F_SETFD || cmd == F_SETFL)
		ret = fcntl(fd, cmd, arg);
	else
		ret = fcntl(fd, cmd);
	return (ret);
}

int
__sread(void *cookie, char *buf, int n)
{
	FILE *fp = (FILE *)cookie;

	return ((int)read(fp->_file, buf, (size_t)n));
}

int
__swrite(void *cookie, const char *buf, int n)
{
	FILE *fp = (FILE *)cookie;

	return ((int)write(fp->_file, buf, (size_t)n));
}

fpos_t
__sseek(void *cookie, fpos_t pos, int whence)
{
	FILE *fp = (FILE *)cookie;

	return ((fpos_t)lseek(fp->_file, (off_t)pos, whence));
}

int
__sclose(void *cookie)
{
	FILE *fp = (FILE *)cookie;

	return (close(fp->_file));
}

/* ======================= fgets.c ======================= */

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

/* ======================= fwrite.c ======================= */

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

/* ======================= fgetws.c ======================= */

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

/* ======================= fdopen.c ======================= */

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

void
b0118_oracle_init(void)
{
	b0118_locale_setup();
}

struct xlocale_ctype *
b0118_get_ctype(void)
{
	return (&b0118_ctype);
}
