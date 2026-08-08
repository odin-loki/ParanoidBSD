/*
 * Reference oracle for batch b0143.
 *
 * hbsd/src/lib/libc/stdio/vswscanf.c, fopen.c, and vswprintf.c concatenated
 * with every function renamed with a ref_ prefix.  Function bodies are
 * UNMODIFIED.  Declarations from unavailable FreeBSD/HardenedBSD private
 * headers are supplied below.
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
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wchar.h>
#include <locale.h>

#ifndef EOF
#define EOF (-1)
#endif

#undef FILE

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef O_VERIFY
#define O_VERIFY 0
#endif

#ifndef O_EXEC
#define O_EXEC 0
#endif

#ifndef DEFFILEMODE
#define DEFFILEMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
#endif

typedef va_list __va_list;
typedef long fpos_t;

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
#define FILE struct __sFILE

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

static FILE *b0143_sfp_target;
static int b0143_open_hook_active;
static int b0143_open_hook_val;

static locale_t
b0143_get_C_locale(void)
{
	static locale_t c_locale = NULL;
	static int inited = 0;

	if (!inited) {
		c_locale = newlocale(LC_ALL_MASK, "C", (locale_t)0);
		inited = 1;
	}
	return (c_locale);
}

static inline locale_t
get_real_locale(locale_t locale)
{
	switch ((intptr_t)locale) {
	case 0:
		return (b0143_get_C_locale());
	case -1:
		return (LC_GLOBAL_LOCALE);
	default:
		return (locale);
	}
}

#define FIX_LOCALE(l) (l = get_real_locale(l))

static inline locale_t
__get_locale(void)
{
	locale_t loc;

	loc = uselocale((locale_t)0);
	if (loc == (locale_t)0)
		return (LC_GLOBAL_LOCALE);
	return (loc);
}

size_t
wcsrtombs_l(char * __restrict dst, const wchar_t ** __restrict src,
    size_t len, mbstate_t * __restrict ps, locale_t locale)
{
	locale_t old = uselocale(locale);
	size_t r = wcsrtombs(dst, src, len, ps);

	uselocale(old);
	return (r);
}

size_t
mbsrtowcs_l(wchar_t * __restrict dst, const char ** __restrict src,
    size_t len, mbstate_t * __restrict ps, locale_t locale)
{
	locale_t old = uselocale(locale);
	size_t r = mbsrtowcs(dst, src, len, ps);

	uselocale(old);
	return (r);
}

void
b0143_set_sfp_target(FILE *fp)
{
	b0143_sfp_target = fp;
}

void
b0143_set_open_hook(int active, int val)
{
	b0143_open_hook_active = active;
	b0143_open_hook_val = val;
}

int
_open(const char *file, int oflags, ...)
{
	mode_t mode = DEFFILEMODE;
	va_list ap;

	if (oflags & O_CREAT) {
		va_start(ap, oflags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
	}
	if (b0143_open_hook_active) {
		b0143_open_hook_active = 0;
		return (b0143_open_hook_val);
	}
	return (open(file, oflags, mode));
}

int
_close(int fd)
{
	return (close(fd));
}

fpos_t
__sseek(void *cookie, fpos_t pos, int whence)
{
	FILE *fp = (FILE *)cookie;

	return (lseek(fp->_file, (off_t)pos, whence));
}

fpos_t
_sseek(FILE *fp, fpos_t offset, int whence)
{
	return (fp->_seek(fp->_cookie, offset, whence));
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

int
__sclose(void *cookie)
{
	FILE *fp = (FILE *)cookie;

	return (close(fp->_file));
}

FILE *
__sfp(void)
{
	FILE *fp;

	if (b0143_sfp_target == NULL)
		return (NULL);

	fp = b0143_sfp_target;
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
__vfwscanf(FILE * __restrict f, locale_t locale,
    const wchar_t * __restrict fmt, __va_list ap)
{
	wchar_t wbuf[4096];
	const char *mb;
	mbstate_t ps;
	size_t wlen;
	locale_t loc;

	FIX_LOCALE(loc);
	if (f->_r <= 0)
		return (EOF);
	mb = (const char *)f->_p;
	memset(&ps, 0, sizeof(ps));
	wlen = mbsnrtowcs(wbuf, &mb, (size_t)f->_r, 4095, &ps);
	if (wlen == (size_t)-1)
		return (EOF);
	wbuf[wlen] = L'\0';
	return (vswscanf(wbuf, fmt, ap));
}

int
__vfwprintf(FILE *f, locale_t locale, const wchar_t *fmt, __va_list ap)
{
	wchar_t wtmp[4096];
	va_list ap2;
	const wchar_t *wp;
	char mbuf[8192];
	mbstate_t ps;
	size_t mlen;
	int nw;
	locale_t loc;

	FIX_LOCALE(loc);
	va_copy(ap2, ap);
	nw = vswprintf(wtmp, 4096, fmt, ap2);
	va_end(ap2);
	if (nw < 0)
		return (-1);

	wp = wtmp;
	memset(&ps, 0, sizeof(ps));
	mlen = wcsrtombs_l(mbuf, &wp, sizeof(mbuf), &ps, loc);
	if (mlen == (size_t)-1)
		return (-1);

	if (mlen > (size_t)f->_bf._size) {
		unsigned char *nbase;

		nbase = realloc(f->_bf._base, mlen + 1);
		if (nbase == NULL)
			return (-1);
		f->_bf._base = nbase;
		f->_bf._size = (int)(mlen + 1);
	}
	memcpy(f->_bf._base, mbuf, mlen);
	f->_p = f->_bf._base + mlen;
	f->_w = f->_bf._size - (int)mlen;
	return (nw);
}

/* ======================= vswscanf.c ======================= */

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
	if ((mlen = wcsrtombs_l(mbstr, &strp, SIZE_MAX, &mbs, locale)) == (size_t)-1) {
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

/* ======================= fopen.c ======================= */

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

/* ======================= vswprintf.c ======================= */

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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS,
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING NEGLIGENCE OR
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
