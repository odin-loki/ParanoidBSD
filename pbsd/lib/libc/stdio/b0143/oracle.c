/*
 * oracle.c -- reference implementation for PBSD batch b0143.
 *
 * The original C sources of
 *
 *	lib/libc/stdio/vswscanf.c
 *	lib/libc/stdio/fopen.c
 *	lib/libc/stdio/vswprintf.c
 *
 * concatenated, with every function renamed with a `ref_' prefix.  The
 * function bodies below the "ORIGINAL SOURCES" banner are UNMODIFIED.
 *
 * Everything above that banner is the environment the bodies compile
 * against: the FreeBSD `struct __sFILE' layout, the __S* flag values,
 * FAKE_FILE, FIX_LOCALE, and definitions for the libc internals the
 * bodies call (__sflags, __sfp, _open, _close, _sseek, __sread,
 * __swrite, __sseek, __sclose, __vfwscanf, __vfwprintf, wcsrtombs_l,
 * mbsrtowcs_l, __get_locale, __get_locale_r).  glibc provides none of
 * these, so they are defined here once and linked against by BOTH the
 * oracle and the port -- the differential test therefore compares the
 * two ports of the batch functions and nothing else.  They record every
 * call into a log the harness inspects.
 */

#define _GNU_SOURCE 1

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <locale.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <wchar.h>

/*
 * <stdio.h> is deliberately not included: `FILE' has to name the BSD
 * stdio object, not glibc's.  All system headers are above this point,
 * so redirecting the two names here is safe.
 */
typedef long b0143_fpos_t;
#define fpos_t b0143_fpos_t

typedef va_list __va_list;

#ifndef EOF
#define EOF (-1)
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif
#ifndef SIZE_T_MAX
#define SIZE_T_MAX SIZE_MAX
#endif
#ifndef DEFFILEMODE
#define DEFFILEMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

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

typedef struct __sFILE b0143_FILE;
#define FILE b0143_FILE

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

#define	FAKE_FILE {				\
	._file = -1,				\
}

#define	FIX_LOCALE(l)	((l) = __get_locale_r(l))

/* ------------------------------------------------------------------ */
/* Shared environment: call log.                                       */
/* ------------------------------------------------------------------ */

#define B0143_DATA_MAX	288
#define B0143_PATH_MAX	64
#define B0143_MODE_MAX	40

struct b0143_log {
	int n_sflags;
	int n_sfp;
	int n_open;
	int n_close;
	int n_sseek;
	int n_vfwscanf;
	int n_vfwprintf;
	int n_getlocale;
	int n_fixlocale;
	int n_wcsrtombs;
	int n_mbsrtowcs;

	char sflags_mode[B0143_MODE_MAX];
	int sflags_ret;
	int sflags_oflags;

	char open_path[B0143_PATH_MAX];
	int open_oflags;
	int open_mode;
	int open_ret;

	int close_fd;

	long sseek_off;
	int sseek_whence;
	int sseek_fp_is_target;

	/* Snapshot of the FILE handed to __vfwscanf()/__vfwprintf(). */
	int f_flags;
	int f_r;
	int f_w;
	int f_file;
	int f_lbfsize;
	int f_flags2;
	int f_bf_size;
	int f_base_is_null;
	long f_p_off;
	int f_cookie_is_null;
	int f_cookie_is_self;
	int f_read_is_null;
	int f_read_ret;
	int f_write_is_null;
	int f_seek_is_null;
	int f_close_is_null;
	int f_ub_size;
	int f_lb_size;
	int f_ur;
	int f_blksize;
	int f_orientation;
	long f_offset;
	int f_locale_is_null;
	int f_locale_is_global;
	int f_locale_is_c;
	int f_fmt_len;
	unsigned int f_fmt0;
	int f_datalen;
	unsigned char f_data[B0143_DATA_MAX];

	int wp_written;
	int wp_ret;
	int scanf_ret;
};

static struct b0143_log g_log;

static void *g_cfg_sfp;
static int g_cfg_open_ret = 3;
static int g_cfg_open_err = ENOENT;
static char g_cfg_wp_buf[512];
static int g_cfg_wp_len;
static int g_cfg_wp_ret;

void b0143_reset(void);
void b0143_get_log(struct b0143_log *);
void b0143_cfg_sfp(void *);
void b0143_cfg_open(int, int);
void b0143_cfg_vfwprintf(const char *, int, int);
void *b0143_fnptr(int);
void *b0143_locale(void);
size_t b0143_log_size(void);

void
b0143_reset(void)
{

	memset(&g_log, 0, sizeof(g_log));
}

void
b0143_get_log(struct b0143_log *out)
{

	memcpy(out, &g_log, sizeof(g_log));
}

void
b0143_cfg_sfp(void *fp)
{

	g_cfg_sfp = fp;
}

void
b0143_cfg_open(int ret, int err)
{

	g_cfg_open_ret = ret;
	g_cfg_open_err = err;
}

void
b0143_cfg_vfwprintf(const char *bytes, int len, int ret)
{

	if (len < 0)
		len = 0;
	if (len > (int)sizeof(g_cfg_wp_buf))
		len = (int)sizeof(g_cfg_wp_buf);
	memset(g_cfg_wp_buf, 0, sizeof(g_cfg_wp_buf));
	if (bytes != NULL && len > 0)
		memcpy(g_cfg_wp_buf, bytes, (size_t)len);
	g_cfg_wp_len = len;
	g_cfg_wp_ret = ret;
}

size_t
b0143_log_size(void)
{

	return (sizeof(struct b0143_log));
}

/* ------------------------------------------------------------------ */
/* Shared environment: locale.                                         */
/* ------------------------------------------------------------------ */

void *
b0143_locale(void)
{
	static locale_t loc;
	static int tried;

	if (!tried) {
		tried = 1;
		loc = newlocale(LC_ALL_MASK, "C.UTF-8", (locale_t)0);
		if (loc == (locale_t)0)
			loc = newlocale(LC_ALL_MASK, "C.utf8", (locale_t)0);
		if (loc == (locale_t)0)
			loc = newlocale(LC_ALL_MASK, "C", (locale_t)0);
	}
	return (loc);
}

locale_t __get_locale(void);
locale_t __get_locale_r(locale_t);

locale_t
__get_locale(void)
{

	g_log.n_getlocale++;
	return ((locale_t)b0143_locale());
}

locale_t
__get_locale_r(locale_t base)
{

	g_log.n_fixlocale++;
	if (base == (locale_t)0 || base == LC_GLOBAL_LOCALE)
		return (__get_locale());
	return (base);
}

size_t wcsrtombs_l(char * __restrict, const wchar_t ** __restrict, size_t,
    mbstate_t * __restrict, locale_t);
size_t mbsrtowcs_l(wchar_t * __restrict, const char ** __restrict, size_t,
    mbstate_t * __restrict, locale_t);

size_t
wcsrtombs_l(char * __restrict dst, const wchar_t ** __restrict src, size_t len,
    mbstate_t * __restrict ps, locale_t locale)
{
	locale_t old;
	size_t r;

	g_log.n_wcsrtombs++;
	old = uselocale(locale == (locale_t)0 ? LC_GLOBAL_LOCALE : locale);
	r = wcsrtombs(dst, src, len, ps);
	uselocale(old == (locale_t)0 ? LC_GLOBAL_LOCALE : old);
	return (r);
}

size_t
mbsrtowcs_l(wchar_t * __restrict dst, const char ** __restrict src, size_t len,
    mbstate_t * __restrict ps, locale_t locale)
{
	locale_t old;
	size_t r;

	g_log.n_mbsrtowcs++;
	old = uselocale(locale == (locale_t)0 ? LC_GLOBAL_LOCALE : locale);
	r = mbsrtowcs(dst, src, len, ps);
	uselocale(old == (locale_t)0 ? LC_GLOBAL_LOCALE : old);
	return (r);
}

/* ------------------------------------------------------------------ */
/* Shared environment: stdio internals.                                */
/* ------------------------------------------------------------------ */

int __sread(void *, char *, int);
int __swrite(void *, const char *, int);
fpos_t __sseek(void *, fpos_t, int);
int __sclose(void *);
fpos_t _sseek(FILE *, fpos_t, int);
FILE *__sfp(void);
int __sflags(const char *, int *);
int _open(const char *, int, ...);
int _close(int);
int __vfwscanf(FILE * __restrict, locale_t, const wchar_t * __restrict,
    va_list);
int __vfwprintf(FILE *, locale_t, const wchar_t *, __va_list);

int
__sread(void *cookie, char *buf, int n)
{

	(void)cookie;
	(void)buf;
	(void)n;
	return (-1);
}

int
__swrite(void *cookie, const char *buf, int n)
{

	(void)cookie;
	(void)buf;
	(void)n;
	return (-1);
}

fpos_t
__sseek(void *cookie, fpos_t offset, int whence)
{

	(void)cookie;
	(void)offset;
	(void)whence;
	return (-1);
}

int
__sclose(void *cookie)
{

	(void)cookie;
	return (-1);
}

void *
b0143_fnptr(int which)
{

	switch (which) {
	case 0:
		return ((void *)(intptr_t)__sread);
	case 1:
		return ((void *)(intptr_t)__swrite);
	case 2:
		return ((void *)(intptr_t)__sseek);
	case 3:
		return ((void *)(intptr_t)__sclose);
	default:
		return (NULL);
	}
}

fpos_t
_sseek(FILE *fp, fpos_t offset, int whence)
{

	g_log.n_sseek++;
	g_log.sseek_off = (long)offset;
	g_log.sseek_whence = whence;
	g_log.sseek_fp_is_target = (fp == (FILE *)g_cfg_sfp);
	return ((fpos_t)0);
}

FILE *
__sfp(void)
{

	g_log.n_sfp++;
	return ((FILE *)g_cfg_sfp);
}

/*
 * Modelled on lib/libc/stdio/flags.c.
 */
int
__sflags(const char *mode, int *optr)
{
	int ret, m, o;

	strncpy(g_log.sflags_mode, mode, sizeof(g_log.sflags_mode) - 1);
	g_log.sflags_mode[sizeof(g_log.sflags_mode) - 1] = '\0';
	g_log.n_sflags++;

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
		g_log.sflags_ret = 0;
		return (0);
	}

	for (; *mode != '\0'; mode++) {
		if (*mode == 'b')
			continue;
		if (*mode == '+') {
			ret = __SRW;
			m = O_RDWR;
			continue;
		}
		if (*mode == 'x') {
			if (m == O_RDONLY) {
				errno = EINVAL;
				g_log.sflags_ret = 0;
				return (0);
			}
			o |= O_EXCL;
			continue;
		}
		if (*mode == 'e') {
			o |= O_CLOEXEC;
			continue;
		}
		break;
	}

	*optr = m | o;
	g_log.sflags_ret = ret;
	g_log.sflags_oflags = *optr;
	return (ret);
}

int
_open(const char *path, int oflags, ...)
{
	va_list ap;
	int mode;

	va_start(ap, oflags);
	mode = va_arg(ap, int);
	va_end(ap);

	g_log.n_open++;
	if (path == NULL) {
		strcpy(g_log.open_path, "(null)");
	} else {
		strncpy(g_log.open_path, path, sizeof(g_log.open_path) - 1);
		g_log.open_path[sizeof(g_log.open_path) - 1] = '\0';
	}
	g_log.open_oflags = oflags;
	g_log.open_mode = mode;
	g_log.open_ret = g_cfg_open_ret;
	if (g_cfg_open_ret < 0)
		errno = g_cfg_open_err;
	return (g_cfg_open_ret);
}

int
_close(int fd)
{

	g_log.n_close++;
	g_log.close_fd = fd;
	return (0);
}

static void
b0143_snap(FILE *fp, locale_t locale, const wchar_t *fmt, int with_data)
{
	char tmp[8];
	int len;

	g_log.f_flags = (int)fp->_flags;
	g_log.f_r = fp->_r;
	g_log.f_w = fp->_w;
	g_log.f_file = (int)fp->_file;
	g_log.f_lbfsize = fp->_lbfsize;
	g_log.f_flags2 = fp->_flags2;
	g_log.f_bf_size = fp->_bf._size;
	g_log.f_base_is_null = (fp->_bf._base == NULL);
	g_log.f_p_off = (long)(fp->_p - fp->_bf._base);
	g_log.f_cookie_is_null = (fp->_cookie == NULL);
	g_log.f_cookie_is_self = (fp->_cookie == (void *)fp);
	g_log.f_write_is_null = (fp->_write == NULL);
	g_log.f_seek_is_null = (fp->_seek == NULL);
	g_log.f_close_is_null = (fp->_close == NULL);
	g_log.f_ub_size = fp->_ub._size;
	g_log.f_lb_size = fp->_lb._size;
	g_log.f_ur = fp->_ur;
	g_log.f_blksize = fp->_blksize;
	g_log.f_orientation = fp->_orientation;
	g_log.f_offset = (long)fp->_offset;
	g_log.f_locale_is_null = (locale == (locale_t)0);
	g_log.f_locale_is_global = (locale == LC_GLOBAL_LOCALE);
	g_log.f_locale_is_c = (locale == (locale_t)b0143_locale());
	g_log.f_fmt_len = fmt == NULL ? -1 : (int)wcslen(fmt);
	g_log.f_fmt0 = fmt == NULL ? 0u : (unsigned int)fmt[0];

	g_log.f_read_is_null = (fp->_read == NULL);
	if (fp->_read != NULL) {
		memset(tmp, 0, sizeof(tmp));
		g_log.f_read_ret = fp->_read(fp->_cookie, tmp, (int)sizeof(tmp));
	} else {
		g_log.f_read_ret = -12345;
	}

	g_log.f_datalen = 0;
	if (with_data && fp->_bf._base != NULL) {
		len = fp->_r;
		if (len < 0)
			len = 0;
		if (len > B0143_DATA_MAX)
			len = B0143_DATA_MAX;
		memcpy(g_log.f_data, fp->_bf._base, (size_t)len);
		g_log.f_datalen = len;
	}
}

int
__vfwscanf(FILE * __restrict fp, locale_t locale, const wchar_t * __restrict fmt,
    va_list ap)
{
	int r;

	(void)ap;
	g_log.n_vfwscanf++;
	b0143_snap(fp, locale, fmt, 1);

	r = g_log.f_r * 3 + g_log.f_bf_size * 5 + g_log.f_flags * 7 +
	    g_log.f_read_ret * 11 + (int)g_log.f_p_off * 13 +
	    (int)g_log.f_fmt0 + g_log.f_fmt_len * 17 + g_log.f_file * 19 +
	    g_log.f_locale_is_c * 23 + g_log.f_datalen * 29;
	g_log.scanf_ret = r;
	return (r);
}

int
__vfwprintf(FILE *fp, locale_t locale, const wchar_t *fmt, __va_list ap)
{
	int room, n;

	(void)ap;
	g_log.n_vfwprintf++;
	b0143_snap(fp, locale, fmt, 0);

	room = fp->_w;
	if (room < 0)
		room = 0;
	n = g_cfg_wp_len;
	if (n > room)
		n = room;
	if (fp->_bf._base != NULL && n > 0)
		memcpy(fp->_bf._base, g_cfg_wp_buf, (size_t)n);
	if (fp->_bf._base != NULL) {
		fp->_p = fp->_bf._base + n;
		fp->_w = fp->_bf._size - n;
	}
	g_log.wp_written = n;
	g_log.wp_ret = g_cfg_wp_ret;
	return (g_cfg_wp_ret);
}

/* ================================================================== */
/* ORIGINAL SOURCES -- function bodies below are unmodified.           */
/* ================================================================== */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Donn Seeley at UUNET Technologies, Inc.
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

static int	ref_eofread(void *, char *, int);

static int
ref_eofread(void *cookie, char *buf, int len)
{

	return (0);
}

int
ref_vswscanf_l(const wchar_t * __restrict str, locale_t locale,
		const wchar_t * __restrict fmt, va_list ap)
{
	static const mbstate_t initial;
	mbstate_t mbs;
	FILE f = FAKE_FILE;
	char *mbstr;
	size_t mlen;
	int r;
	const wchar_t *strp;
	FIX_LOCALE(locale);

	/*
	 * XXX Convert the wide character string to multibyte, which
	 * __vfwscanf() will convert back to wide characters.
	 */
	if ((mbstr = malloc(wcslen(str) * MB_CUR_MAX + 1)) == NULL)
		return (EOF);
	mbs = initial;
	strp = str;
	if ((mlen = wcsrtombs_l(mbstr, &strp, SIZE_T_MAX, &mbs, locale)) == (size_t)-1) {
		free(mbstr);
		return (EOF);
	}
	f._flags = __SRD;
	f._bf._base = f._p = (unsigned char *)mbstr;
	f._bf._size = f._r = mlen;
	f._read = ref_eofread;
	r = __vfwscanf(&f, locale, fmt, ap);
	free(mbstr);

	return (r);
}
int
ref_vswscanf(const wchar_t * __restrict str, const wchar_t * __restrict fmt,
    va_list ap)
{
	return ref_vswscanf_l(str, __get_locale(), fmt, ap);
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
ref_fopen(const char * __restrict file, const char * __restrict mode)
{
	FILE *fp;
	int f;
	int flags, oflags;

	if ((flags = __sflags(mode, &oflags)) == 0)
		return (NULL);
	if ((fp = __sfp()) == NULL)
		return (NULL);
	if ((f = _open(file, oflags, DEFFILEMODE)) < 0) {
		fp->_flags = 0;			/* release */
		return (NULL);
	}
	/*
	 * File descriptors are a full int, but _file is only a short.
	 * If we get a valid file descriptor that is greater than
	 * SHRT_MAX, then the fd will get sign-extended into an
	 * invalid file descriptor.  Handle this case by failing the
	 * open.
	 */
	if (f > SHRT_MAX) {
		fp->_flags = 0;			/* release */
		_close(f);
		errno = EMFILE;
		return (NULL);
	}
	fp->_file = f;
	fp->_flags = flags;
	fp->_cookie = fp;
	fp->_read = __sread;
	fp->_write = __swrite;
	fp->_seek = __sseek;
	fp->_close = __sclose;
	/*
	 * When opening in append mode, even though we use O_APPEND,
	 * we need to seek to the end so that ftell() gets the right
	 * answer.  If the user then alters the seek pointer, or
	 * the file extends, this will fail, but there is not much
	 * we can do about this.  (We could set __SAPP and check in
	 * fseek and ftell.)
	 */
	if (oflags & O_APPEND) {
		fp->_flags2 |= __S2OAP;
		(void)_sseek(fp, (fpos_t)0, SEEK_END);
	}
	return (fp);
}

/*	$OpenBSD: vasprintf.c,v 1.4 1998/06/21 22:13:47 millert Exp $	*/

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1997 Todd C. Miller <Todd.Miller@courtesan.com>
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
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

int
ref_vswprintf_l(wchar_t * __restrict s, size_t n, locale_t locale,
		const wchar_t * __restrict fmt, __va_list ap)
{
	static const mbstate_t initial;
	mbstate_t mbs;
	FILE f = FAKE_FILE;
	char *mbp;
	int ret, sverrno;
	size_t nwc;
	FIX_LOCALE(locale);

	if (n == 0) {
		errno = EINVAL;
		return (-1);
	}
	if (n - 1 > INT_MAX) {
		errno = EOVERFLOW;
		*s = L'\0';
		return (-1);
	}

	f._flags = __SWR | __SSTR | __SALC;
	f._bf._base = f._p = (unsigned char *)malloc(128);
	if (f._bf._base == NULL) {
		errno = ENOMEM;
		*s = L'\0';
		return (-1);
	}
	f._bf._size = f._w = 127;		/* Leave room for the NUL */
	ret = __vfwprintf(&f, locale, fmt, ap);
	if (ret < 0) {
		sverrno = errno;
		free(f._bf._base);
		errno = sverrno;
		*s = L'\0';
		return (-1);
	}
	*f._p = '\0';
	mbp = f._bf._base;
	/*
	 * XXX Undo the conversion from wide characters to multibyte that
	 * fputwc() did in __vfwprintf().
	 */
	mbs = initial;
	nwc = mbsrtowcs_l(s, (const char **)&mbp, n, &mbs, locale);
	free(f._bf._base);
	if (nwc == (size_t)-1) {
		errno = EILSEQ;
		*s = L'\0';
		return (-1);
	}
	if (nwc == n) {
		s[n - 1] = L'\0';
		errno = EOVERFLOW;
		return (-1);
	}

	return (ret);
}
int
ref_vswprintf(wchar_t * __restrict s, size_t n, const wchar_t * __restrict fmt,
    __va_list ap)
{
	return ref_vswprintf_l(s, n, __get_locale(), fmt, ap);
}
