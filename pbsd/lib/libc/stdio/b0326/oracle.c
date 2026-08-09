/*
 * PBSD batch b0326 -- reference oracle.
 *
 * The original HardenedBSD sources for
 *     lib/libc/stdio/makebuf.c
 *     lib/libc/stdio/xprintf_time.c
 *     lib/libc/stdio/fclose.c
 *     lib/libc/stdio/ftell.c
 * concatenated, with every function renamed with a "ref_" prefix.  Function
 * bodies are byte-for-byte unmodified.  Only the declarations/defines that the
 * original private libc headers would have supplied are added below.
 */

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE 1

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

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

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

/* ------------------------------------------------------------------ */
/* Minimal libc internals modelled for this batch.                     */
/* ------------------------------------------------------------------ */

typedef long long b0326_fpos_t;
#define fpos_t b0326_fpos_t

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
	unsigned char _mbstate_pad[8];
	int _flags2;
};

typedef struct __sFILE b0326_FILE;
#define FILE b0326_FILE

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
		free((char *)(fp)->_ub._base); \
	(fp)->_ub._base = NULL; \
}

#define HASLB(fp)	((fp)->_lb._base != NULL)
#define FREELB(fp) { \
	free((char *)(fp)->_lb._base); \
	(fp)->_lb._base = NULL; \
}

struct printf_info {
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

struct __printf_io {
	int _dummy;
};

#define PA_POINTER	5

#define MINUTE 60
#define HOUR	(60 * MINUTE)
#define DAY	(24 * HOUR)
#define YEAR	(365 * DAY)

int mock_flock_calls;
int mock_funlock_calls;
int mock_stdio_lock_calls;
int mock_stdio_unlock_calls;

int mock_fstat_ret;
struct stat mock_fstat_st;
int mock_fstat_fd;

int mock_malloc_fail;
int mock_malloc_calls;
size_t mock_malloc_last_n;

int mock_isatty_ret;
int mock_isatty_fd;

int mock_sflush_ret;
int mock_sflush_calls;

int mock_close_ret;
int mock_close_calls;
void *mock_close_cookie;

int mock_sseek_ret;
int mock_sseek_fail;
int mock_sseek_calls;
FILE *mock_sseek_fp;
fpos_t mock_sseek_off;
int mock_sseek_whence;

fpos_t mock___sseek_ret;
int mock___sseek_fail;
int mock___sseek_calls;
void *mock___sseek_cookie;

int mock_printf_out_ret;
int mock_printf_out_calls;
const char *mock_printf_out_last_buf;
int mock_printf_out_last_len;

int mock_printf_flush_calls;

void (*__cleanup)(void);
int mock_cleanup_set;

unsigned char mock_malloc_arena[262144];
size_t mock_malloc_arena_off;

int mock_fstat_calls;

void b0326_reset(void);
void *__real_malloc(size_t);

void
b0326_reset(void)
{

	mock_flock_calls = 0;
	mock_funlock_calls = 0;
	mock_stdio_lock_calls = 0;
	mock_stdio_unlock_calls = 0;
	mock_fstat_ret = 0;
	memset(&mock_fstat_st, 0, sizeof(mock_fstat_st));
	mock_fstat_fd = -1;
	mock_malloc_fail = 0;
	mock_malloc_calls = 0;
	mock_malloc_last_n = 0;
	mock_malloc_arena_off = 0;
	mock_isatty_ret = 0;
	mock_isatty_fd = -1;
	mock_sflush_ret = 0;
	mock_sflush_calls = 0;
	mock_close_ret = 0;
	mock_close_calls = 0;
	mock_close_cookie = NULL;
	mock_sseek_ret = 0;
	mock_sseek_fail = 0;
	mock_sseek_calls = 0;
	mock_sseek_fp = NULL;
	mock_sseek_off = 0;
	mock_sseek_whence = 0;
	mock___sseek_ret = 0;
	mock___sseek_fail = 0;
	mock___sseek_calls = 0;
	mock___sseek_cookie = NULL;
	mock_printf_out_ret = 0;
	mock_printf_out_calls = 0;
	mock_printf_out_last_buf = NULL;
	mock_printf_out_last_len = 0;
	mock_printf_flush_calls = 0;
	mock_fstat_calls = 0;
	__cleanup = NULL;
	mock_cleanup_set = 0;
}

void *
__wrap_malloc(size_t nbytes)
{

	mock_malloc_calls++;
	mock_malloc_last_n = nbytes;
	if (mock_malloc_fail)
		return (NULL);
	if (mock_malloc_arena_off + nbytes > sizeof(mock_malloc_arena))
		return (NULL);
	{
		void *p = &mock_malloc_arena[mock_malloc_arena_off];

		mock_malloc_arena_off = (mock_malloc_arena_off + nbytes + 15U) &
		    ~(size_t)15U;
		return (p);
	}
}

int
_fstat(int fd, struct stat *st)
{

	mock_fstat_calls++;
	mock_fstat_fd = fd;
	if (mock_fstat_ret < 0) {
		errno = EINVAL;
		return (-1);
	}
	memset(st, 0, sizeof(*st));
	st->st_mode = mock_fstat_st.st_mode;
	st->st_blksize = mock_fstat_st.st_blksize;
	return (0);
}

int
isatty(int fd)
{

	mock_isatty_fd = fd;
	return (mock_isatty_ret);
}

void
_cleanup(void)
{

	mock_cleanup_set = 1;
}

int
__sflush(FILE *fp)
{

	mock_sflush_calls++;
	(void)fp;
	return (mock_sflush_ret);
}

int
__sclose(void *cookie)
{

	mock_close_calls++;
	mock_close_cookie = cookie;
	return (mock_close_ret);
}

fpos_t
__sseek(void *cookie, fpos_t pos, int whence)
{

	mock___sseek_calls++;
	mock___sseek_cookie = cookie;
	mock_sseek_whence = whence;
	(void)pos;
	if (mock___sseek_fail)
		return ((fpos_t)-1);
	return (mock___sseek_ret);
}

fpos_t
_sseek(FILE *fp, fpos_t pos, int whence)
{

	mock_sseek_calls++;
	mock_sseek_fp = fp;
	mock_sseek_off = pos;
	mock_sseek_whence = whence;
	if (mock_sseek_fail)
		return ((fpos_t)-1);
	return (mock_sseek_ret);
}

int
__printf_out(struct __printf_io *io, const struct printf_info *pi,
    const char *buf, int len)
{

	(void)io;
	(void)pi;
	mock_printf_out_calls++;
	mock_printf_out_last_buf = buf;
	mock_printf_out_last_len = len;
	return (mock_printf_out_ret);
}

void
__printf_flush(struct __printf_io *io)
{

	(void)io;
	mock_printf_flush_calls++;
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

int ref___swhatbuf(FILE *, size_t *, int *);
off_t ref_ftello(FILE *);
int ref__ftello(FILE *, fpos_t *);

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
ref___smakebuf(FILE *fp)
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
	flags = ref___swhatbuf(fp, &size, &couldbetty);
	if ((p = malloc(size)) == NULL) {
		fp->_flags |= __SNBF;
		fp->_bf._base = fp->_p = fp->_nbuf;
		fp->_bf._size = 1;
		return;
	}
	__cleanup = _cleanup;
	flags |= __SMBF;
	fp->_bf._base = fp->_p = p;
	fp->_bf._size = size;
	if (couldbetty && isatty(fp->_file))
		flags |= __SLBF;
	fp->_flags |= flags;
}

/*
 * Internal routine to determine `proper' buffering for a file.
 */
int
ref___swhatbuf(FILE *fp, size_t *bufsize, int *couldbetty)
{
	struct stat st;

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
ref___printf_arginfo_time(const struct printf_info *pi, size_t n, int *argt)
{

	assert(n >= 1);
	argt[0] = PA_POINTER;
	return (1);
}

int
ref___printf_render_time(struct __printf_io *io, const struct printf_info *pi, const void *const *arg)
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
	ret = __printf_out(io, pi, buf, p - buf);
	__printf_flush(io);
	return (ret);
}

/* ====================================================================== */
/* lib/libc/stdio/fclose.c                                                */
/* ====================================================================== */

#define cleanfile ref_cleanfile

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
		free((char *)fp->_bf._base);
	if (HASUB(fp))
		FREEUB(fp);
	if (HASLB(fp))
		FREELB(fp);
	fp->_file = -1;
	fp->_r = fp->_w = 0;	/* Mess up if reaccessed. */

	/*
	 * Lock the spinlock used to protect __sglue list walk in
	 * __sfp().  The __sfp() uses fp->_flags == 0 test as an
	 * indication of the unused FILE.
	 *
	 * Taking the lock prevents possible compiler or processor
	 * reordering of the writes performed before the final _flags
	 * cleanup, making sure that we are done with the FILE before
	 * it is considered available.
	 */
	STDIO_THREAD_LOCK();
	fp->_flags = 0;		/* Release this FILE for reuse. */
	STDIO_THREAD_UNLOCK();

	return (r);
}

int
ref_fdclose(FILE *fp, int *fdp)
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
ref_fclose(FILE *fp)
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

#undef cleanfile

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
ref_ftell(FILE *fp)
{
	off_t rv;

	rv = ref_ftello(fp);
	if (rv > LONG_MAX) {
		errno = EOVERFLOW;
		return (-1);
	}
	return (rv);
}

/*
 * ftello: return current offset.
 */
off_t
ref_ftello(FILE *fp)
{
	fpos_t rv;
	int ret;

	FLOCKFILE(fp);
	ret = ref__ftello(fp, &rv);
	FUNLOCKFILE(fp);
	if (ret)
		return (-1);
	if (rv < 0) {   /* Unspecified value because of ungetc() at 0 */
		errno = ESPIPE;
		return (-1);
	}
	return (rv);
}

int
ref__ftello(FILE *fp, fpos_t *offset)
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
	    (n = fp->_p - fp->_bf._base) > 0) {
		/*
		 * Writing.  Any buffered characters cause the
		 * position to be greater than that in the
		 * underlying object.
		 */
		if (pos > OFF_MAX - n) {
			errno = EOVERFLOW;
			return (1);
		}
		pos += n;
	}
	*offset = pos;
	return (0);
}
