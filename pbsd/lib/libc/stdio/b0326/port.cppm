/*
 * PBSD batch b0326 -- C++23 port of
 *     lib/libc/stdio/makebuf.c
 *     lib/libc/stdio/xprintf_time.c
 *     lib/libc/stdio/fclose.c
 *     lib/libc/stdio/ftell.c
 *
 * Faithful translation: behaviour, signedness, evaluation order and pointer
 * arithmetic are preserved exactly.  Original copyright headers are retained
 * with each function.
 */

module;

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1
#endif
#ifndef BUFSIZ
#define BUFSIZ 1024
#endif
#undef BUFSIZ
#define BUFSIZ 1024

#include <cassert>
#include <cerrno>
#include <climits>
#include <cinttypes>
#include <cstdbool>
#include <cstddef>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

export module pbsd.lib.libc.stdio.b0326;

extern "C" int sprintf(char *, const char *, ...);

export struct __sbuf {
	unsigned char *_base;
	int _size;
};

export struct __b0326_FILE {
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
	long long (*_seek)(void *, long long, int);
	int (*_write)(void *, const char *, int);
	struct __sbuf _ub;
	unsigned char *_up;
	int _ur;
	unsigned char _ubuf[3];
	unsigned char _nbuf[1];
	struct __sbuf _lb;
	int _blksize;
	long long _offset;
	void *_fl_mutex;
	void *_fl_owner;
	int _fl_count;
	int _orientation;
	unsigned char _mbstate_pad[8];
	int _flags2;
};

export struct printf_info {
	int prec;
	int width;
	char pad;
	char showsign;
	char left;
	char space;
	char alt;
	char is_char;
	char is_short;
	char is_long;
	char is_long_double;
	char is_ptrdiff;
	char is_quad;
	char is_intmax;
	char is_size;
	char is_ptr;
};

export struct __printf_io {
	int _dummy;
};

#ifndef EOF
#define EOF (-1)
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif
#ifndef BUFSIZ
#define BUFSIZ 1024
#endif
#ifndef OFF_MAX
#define OFF_MAX LLONG_MAX
#endif

#define PA_POINTER	5

#define MINUTE 60
#define HOUR	(60 * MINUTE)
#define DAY	(24 * HOUR)
#define YEAR	(365 * DAY)

extern "C" {
extern int mock_flock_calls;
extern int mock_funlock_calls;
extern int mock_stdio_lock_calls;
extern int mock_stdio_unlock_calls;
extern int mock_fstat_ret;
extern struct stat mock_fstat_st;
extern int mock_malloc_fail;
extern int mock_malloc_calls;
extern int mock_isatty_ret;
extern int mock_sflush_ret;
extern int mock_close_ret;
extern long long mock_sseek_ret;
extern int mock_sseek_fail;
extern long long mock___sseek_ret;
extern int mock___sseek_fail;
extern int mock_printf_out_ret;
extern int mock_printf_out_calls;
extern const char *mock_printf_out_last_buf;
extern int mock_printf_out_last_len;
extern int mock_printf_flush_calls;
extern void (*__cleanup)(void);
extern int mock_cleanup_set;
extern unsigned char mock_malloc_arena[262144];
void b0326_reset(void);
int _fstat(int, struct stat *);
int isatty(int);
void _cleanup(void);
int __sflush(struct __b0326_FILE *);
int __sclose(void *);
long long __sseek(void *, long long, int);
long long _sseek(struct __b0326_FILE *, long long, int);
int __printf_out(struct __printf_io *, const struct printf_info *,
    const char *, int);
void __printf_flush(struct __printf_io *);
}

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

#define HASUB(fp)	((fp)->_ub._base != NULL)
#define FREEUB(fp) { \
	if ((fp)->_ub._base != (fp)->_ubuf) \
		::free((char *)(fp)->_ub._base); \
	(fp)->_ub._base = NULL; \
}

#define HASLB(fp)	((fp)->_lb._base != NULL)
#define FREELB(fp) { \
	::free((char *)(fp)->_lb._base); \
	(fp)->_lb._base = NULL; \
}

#define	FLOCKFILE(fp)	do {						\
		mock_flock_calls++;					\
	} while (0)
#define	FUNLOCKFILE(fp)	do {						\
		mock_funlock_calls++;					\
	} while (0)

#define	FLOCKFILE_CANCELSAFE(fp)					\
	{								\
		FILE *_flfp = (fp);					\
		FLOCKFILE(_flfp);
#define	FUNLOCKFILE_CANCELSAFE()					\
		FUNLOCKFILE(_flfp);					\
	}

#define	STDIO_THREAD_LOCK()	do { mock_stdio_lock_calls++; } while (0)
#define	STDIO_THREAD_UNLOCK()	do { mock_stdio_unlock_calls++; } while (0)

export namespace pbsd::lib_libc_stdio::b0326 {

using FILE = __b0326_FILE;
using fpos_t = long long;

static int cleanfile(FILE *fp, bool c);
int __swhatbuf(FILE *fp, size_t *bufsize, int *couldbetty);
off_t ftello(FILE *fp);
int _ftello(FILE *fp, fpos_t *offset);

static int
cleanfile(FILE *fp, bool c)
{
	int r;

	r = fp->_flags & __SWR ? __sflush(fp) : 0;
	if (c) {
		if (fp->_close != NULL && (*fp->_close)(fp->_cookie) < 0)
			r = EOF;
	}

	if (fp->_flags & __SMBF)
		::free((char *)fp->_bf._base);
	if (HASUB(fp))
		FREEUB(fp);
	if (HASLB(fp))
		FREELB(fp);
	fp->_file = -1;
	fp->_r = fp->_w = 0;	/* Mess up if reaccessed. */

	STDIO_THREAD_LOCK();
	fp->_flags = 0;		/* Release this FILE for reuse. */
	STDIO_THREAD_UNLOCK();

	return (r);
}

/* ====================================================================== */
/* lib/libc/stdio/makebuf.c                                               */
/* ====================================================================== */

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

void
__smakebuf(FILE *fp)
{
	void *p;
	int flags;
	size_t size;
	int couldbetty;

	if (fp->_flags & __SNBF) {
		fp->_bf._base = fp->_p = fp->_nbuf;
		fp->_bf._size = 1;
		return;
	}
	flags = __swhatbuf(fp, &size, &couldbetty);
	if ((p = ::malloc(size)) == NULL) {
		fp->_flags |= __SNBF;
		fp->_bf._base = fp->_p = fp->_nbuf;
		fp->_bf._size = 1;
		return;
	}
	__cleanup = _cleanup;
	flags |= __SMBF;
	fp->_bf._base = fp->_p = (unsigned char *)p;
	fp->_bf._size = size;
	if (couldbetty && isatty(fp->_file))
		flags |= __SLBF;
	fp->_flags |= flags;
}

/*
 * Internal routine to determine `proper' buffering for a file.
 */
int
__swhatbuf(FILE *fp, size_t *bufsize, int *couldbetty)
{
	alignas(16) struct stat st;

	if (fp->_file < 0 || _fstat(fp->_file, &st) < 0) {
		*couldbetty = 0;
		*bufsize = BUFSIZ;
		return (__SNPT);
	}

	/* could be a tty iff it is a character device */
	*couldbetty = (st.st_mode & S_IFMT) == S_IFCHR;
	if (st.st_blksize <= 0) {
		*bufsize = BUFSIZ;
		return (__SNPT);
	}

	/*
	 * Optimise fseek() only if it is a regular file.  (The test for
	 * __sseek is mainly paranoia.)  It is safe to set _blksize
	 * unconditionally; it will only be used if __SOPT is also set.
	 */
	*bufsize = st.st_blksize;
	fp->_blksize = st.st_blksize;
	return ((st.st_mode & S_IFMT) == S_IFREG && fp->_seek == __sseek ?
	    __SOPT : __SNPT);
}

/* ====================================================================== */
/* lib/libc/stdio/xprintf_time.c                                          */
/* ====================================================================== */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2005 Poul-Henning Kamp
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

int
__printf_arginfo_time(const struct printf_info *pi, size_t n, int *argt)
{

	assert(n >= 1);
	argt[0] = PA_POINTER;
	return (1);
}

int
__printf_render_time(struct __printf_io *io, const struct printf_info *pi, const void *const *arg)
{
	char buf[100];
	char *p;
	struct timeval *tv;
	struct timespec *ts;
	time_t *tp;
	intmax_t t, tx;
	int i, prec, nsec, ret;

	if (pi->is_long) {
		tv = *((struct timeval **)arg[0]);
		t = tv->tv_sec;
		nsec = tv->tv_usec * 1000;
		prec = 6;
	} else if (pi->is_long_double) {
		ts = *((struct timespec **)arg[0]);
		t = ts->tv_sec;
		nsec = ts->tv_nsec;
		prec = 9;
	} else {
		tp = *((time_t **)arg[0]);
		t = *tp;
		nsec = 0;
		prec = 0;
	}
	if (pi->is_long || pi->is_long_double) {
		if (pi->prec >= 0) 
			prec = pi->prec;
		if (prec == 0)
			nsec = 0;
	}

	p = buf;
	if (pi->alt) {
		tx = t;
		if (t >= YEAR) {
			p += sprintf(p, "%jdy", t / YEAR);
			t %= YEAR;
		}
		if (tx >= DAY && (t != 0 || prec != 0)) {
			p += sprintf(p, "%jdd", t / DAY);
			t %= DAY;
		}
		if (tx >= HOUR && (t != 0 || prec != 0)) {
			p += sprintf(p, "%jdh", t / HOUR);
			t %= HOUR;
		}
		if (tx >= MINUTE && (t != 0 || prec != 0)) {
			p += sprintf(p, "%jdm", t / MINUTE);
			t %= MINUTE;
		}
		if (t != 0 || tx == 0 || prec != 0)
			p += sprintf(p, "%jds", t);
	} else  {
		p += sprintf(p, "%jd", (intmax_t)t);
	}
	if (prec != 0) {
		for (i = prec; i < 9; i++)
			nsec /= 10;
		p += sprintf(p, ".%.*d", prec, nsec);
	}
	ret = __printf_out(io, pi, buf, (int)(p - buf));
	__printf_flush(io);
	return (ret);
}

/* ====================================================================== */
/* lib/libc/stdio/fclose.c                                                */
/* ====================================================================== */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993 The Regents of the University of California.
 * Copyright (c) 2013 Mariusz Zaborski <oshogbo@FreeBSD.org>
 * All rights reserved.
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

int
fdclose(FILE *fp, int *fdp)
{
	int r, err;

	if (fdp != NULL)
		*fdp = -1;

	if (fp->_flags == 0) {	/* not open! */
		errno = EBADF;
		return (EOF);
	}

	FLOCKFILE_CANCELSAFE(fp);
	r = 0;
	if (fp->_close != __sclose) {
		r = EOF;
		errno = EOPNOTSUPP;
	} else if (fp->_file < 0) {
		r = EOF;
		errno = EBADF;
	}
	if (r == EOF) {
		err = errno;
		(void)cleanfile(fp, true);
		errno = err;
	} else {
		if (fdp != NULL)
			*fdp = fp->_file;
		r = cleanfile(fp, false);
	}
	FUNLOCKFILE_CANCELSAFE();

	return (r);
}

int
fclose(FILE *fp)
{
	int r;

	if (fp->_flags == 0) {	/* not open! */
		errno = EBADF;
		return (EOF);
	}

	FLOCKFILE_CANCELSAFE(fp);
	r = cleanfile(fp, true);
	FUNLOCKFILE_CANCELSAFE();

	return (r);
}

/* ====================================================================== */
/* lib/libc/stdio/ftell.c                                                 */
/* ====================================================================== */

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

long
ftell(FILE *fp)
{
	off_t rv;

	rv = ftello(fp);
	if (rv > LONG_MAX) {
		errno = EOVERFLOW;
		return (-1);
	}
	return ((long)rv);
}

/*
 * ftello: return current offset.
 */
off_t
ftello(FILE *fp)
{
	fpos_t rv;
	int ret;

	FLOCKFILE(fp);
	ret = _ftello(fp, &rv);
	FUNLOCKFILE(fp);
	if (ret)
		return (-1);
	if (rv < 0) {   /* Unspecified value because of ungetc() at 0 */
		errno = ESPIPE;
		return (-1);
	}
	return ((off_t)rv);
}

int
_ftello(FILE *fp, fpos_t *offset)
{
	fpos_t pos;
	size_t n;

	if (fp->_seek == NULL) {
		errno = ESPIPE;			/* historic practice */
		return (1);
	}

	/*
	 * Find offset of underlying I/O object, then
	 * adjust for buffered bytes.
	 */
	if (!(fp->_flags & __SRD) && (fp->_flags & __SWR) &&
	    fp->_p != NULL && fp->_p - fp->_bf._base > 0 &&
	    ((fp->_flags & __SAPP) || (fp->_flags2 & __S2OAP))) {
		pos = _sseek(fp, (fpos_t)0, SEEK_END);
		if (pos == -1)
			return (1);
	} else if (fp->_flags & __SOFF)
		pos = fp->_offset;
	else {
		pos = _sseek(fp, (fpos_t)0, SEEK_CUR);
		if (pos == -1)
			return (1);
	}
	if (fp->_flags & __SRD) {
		/*
		 * Reading.  Any unread characters (including
		 * those from ungetc) cause the position to be
		 * smaller than that in the underlying object.
		 */
		if ((pos -= (HASUB(fp) ? fp->_ur : fp->_r)) < 0) {
			fp->_flags |= __SERR;
			errno = EIO;
			return (1);
		}
		if (HASUB(fp))
			pos -= fp->_r;  /* Can be negative at this point. */
	} else if ((fp->_flags & __SWR) && fp->_p != NULL &&
	    (n = (size_t)(fp->_p - fp->_bf._base)) > 0) {
		/*
		 * Writing.  Any buffered characters cause the
		 * position to be greater than that in the
		 * underlying object.
		 */
		if (pos > OFF_MAX - (fpos_t)n) {
			errno = EOVERFLOW;
			return (1);
		}
		pos += (fpos_t)n;
	}
	*offset = pos;
	return (0);
}

} /* namespace pbsd::lib_libc_stdio::b0326 */
