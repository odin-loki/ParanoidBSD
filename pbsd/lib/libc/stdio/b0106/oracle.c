/*
 * Reference oracle for batch b0106 (lib/libc/stdio).
 *
 * The original HardenedBSD sources are concatenated below with every function
 * renamed with a "ref_" prefix.  The function bodies are UNMODIFIED: the
 * renaming is done with #define, and the FreeBSD libc internals the bodies
 * reach for are supplied by the shared test substrate below.  Only #include
 * lines of FreeBSD-private headers were dropped.
 *
 * The substrate is *shared*: port.cppm calls exactly these objects and
 * functions, so a differential test between the port and this oracle compares
 * the logic that actually lives in the four ported files.  Where a real glibc
 * primitive has the same job as the FreeBSD internal, the real primitive is
 * used (wcrtomb, vsnprintf, vfscanf, uselocale, flockfile), so the substrate
 * invents as little behaviour as possible.
 *
 * scanf.c is emitted first because it is the only file that needs the real
 * <stdio.h> FILE.  The other three reach into FILE's private members, so from
 * the "region 2" marker onwards FILE names the substrate's stand-in struct.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

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

const short pbsd_shim_SLBF = __SLBF;
const short pbsd_shim_SNBF = __SNBF;
const short pbsd_shim_SRD = __SRD;
const short pbsd_shim_SWR = __SWR;
const short pbsd_shim_SRW = __SRW;
const short pbsd_shim_SEOF = __SEOF;
const short pbsd_shim_SERR = __SERR;
const short pbsd_shim_SMBF = __SMBF;
const short pbsd_shim_SAPP = __SAPP;
const short pbsd_shim_SSTR = __SSTR;

#define __va_list va_list

/*
 * ---------------------------------------------------------------------------
 * Shared test substrate, part 1: the pieces that work on a real glibc FILE.
 * ---------------------------------------------------------------------------
 */

/* The stream scanf(3) reads from; the harness points this at an fmemopen(). */
FILE *pbsd_shim_stdin;

/* Last locale handed to the scanf back end, so the plumbing is observable. */
locale_t pbsd_shim_svfscanf_locale;

locale_t
pbsd_shim_get_locale(void)
{
	return (uselocale((locale_t)0));
}

int
pbsd_shim_svfscanf(FILE *fp, locale_t locale, const char *fmt, va_list ap)
{
	locale_t oldloc;
	int r;

	pbsd_shim_svfscanf_locale = locale;
	oldloc = uselocale(locale);
	r = vfscanf(fp, fmt, ap);
	uselocale(oldloc);
	return (r);
}

#define __get_locale() pbsd_shim_get_locale()
#define __svfscanf(fp, loc, fmt, ap) pbsd_shim_svfscanf((fp), (loc), (fmt), (ap))

#define FIX_LOCALE(loc)							\
	do {								\
		if ((loc) == NULL)					\
			(loc) = __get_locale();				\
	} while (0)

/*
 * FreeBSD's cancellation-safe locking macros open a block that
 * FUNLOCKFILE_CANCELSAFE() closes; keep that shape so the bodies compile
 * verbatim.
 */
#define FLOCKFILE_CANCELSAFE(fp)					\
	{								\
		FILE *_shim_lockfp = (fp);				\
		flockfile(_shim_lockfp);
#define FUNLOCKFILE_CANCELSAFE()					\
		funlockfile(_shim_lockfp);				\
	}

#undef stdin
#define stdin pbsd_shim_stdin

/*
 * Function renaming.  The bodies below are byte-for-byte the originals; the
 * "ref_" prefix is applied by the preprocessor, which also fixes up the
 * intra-batch call sites (fputwc_l -> __fputwc, fputs -> fputs_unlocked,
 * vsprintf -> vsprintf_l).  The #undefs guard against glibc's
 * _FORTIFY_SOURCE definitions of the same names.
 */
#undef scanf
#undef scanf_l
#define scanf ref_scanf
#define scanf_l ref_scanf_l

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
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

int
scanf(char const * __restrict fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	FLOCKFILE_CANCELSAFE(stdin);
	ret = __svfscanf(stdin, __get_locale(), fmt, ap);
	FUNLOCKFILE_CANCELSAFE();
	va_end(ap);
	return (ret);
}
int
scanf_l(locale_t locale, char const * __restrict fmt, ...)
{
	int ret;
	va_list ap;
	FIX_LOCALE(locale);

	va_start(ap, fmt);
	FLOCKFILE_CANCELSAFE(stdin);
	ret = __svfscanf(stdin, locale, fmt, ap);
	FUNLOCKFILE_CANCELSAFE();
	va_end(ap);
	return (ret);
}

/*
 * ---------------------------------------------------------------------------
 * Shared test substrate, part 2: FreeBSD's private FILE.
 *
 * fputwc.c, fputs.c and vsprintf.c all reach into FILE's private members
 * (_flags, _p, _w, _bf, _mbstate), so they cannot run against a glibc FILE.
 * pbsd_shim_file stands in for FreeBSD's struct __sFILE with the members those
 * three files touch, plus orientation and lock counters so that ORIENT() and
 * the locking macros are observable to the differential test.
 * ---------------------------------------------------------------------------
 */

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
	mbstate_t _mbstate;
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

/*
 * Per-locale multibyte encoder, as xlocale_ctype supplies on FreeBSD.  Each
 * encoder is the real glibc wcrtomb() running under one specific locale, so
 * the encodings and the EILSEQ failures are genuine.
 */
struct xlocale_ctype {
	size_t (*__wcrtomb)(char *, wchar_t, mbstate_t *);
	const char *__name;
};

locale_t pbsd_shim_locale_c;
locale_t pbsd_shim_locale_utf8;

static size_t
pbsd_shim_wcrtomb_at(locale_t loc, char *s, wchar_t wc, mbstate_t *ps)
{
	locale_t oldloc;
	size_t r;

	oldloc = uselocale(loc);
	r = wcrtomb(s, wc, ps);
	uselocale(oldloc);
	return (r);
}

static size_t
pbsd_shim_wcrtomb_c(char *s, wchar_t wc, mbstate_t *ps)
{
	return (pbsd_shim_wcrtomb_at(pbsd_shim_locale_c, s, wc, ps));
}

static size_t
pbsd_shim_wcrtomb_utf8(char *s, wchar_t wc, mbstate_t *ps)
{
	return (pbsd_shim_wcrtomb_at(pbsd_shim_locale_utf8, s, wc, ps));
}

static struct xlocale_ctype pbsd_shim_ctype_c = { pbsd_shim_wcrtomb_c, "C" };
static struct xlocale_ctype pbsd_shim_ctype_utf8 = {
	pbsd_shim_wcrtomb_utf8, "C.UTF-8"
};

struct xlocale_ctype *
pbsd_shim_xlocale_ctype(locale_t loc)
{
	if (loc == pbsd_shim_locale_utf8)
		return (&pbsd_shim_ctype_utf8);
	return (&pbsd_shim_ctype_c);
}

void
pbsd_shim_orient(struct pbsd_shim_file *fp, int dir)
{
	if (fp->_orientation == 0)
		fp->_orientation = dir;
}

void
pbsd_shim_flockfile(struct pbsd_shim_file *fp)
{
	if (fp != NULL) {
		fp->_lockdepth++;
		fp->_locktotal++;
	}
}

void
pbsd_shim_funlockfile(struct pbsd_shim_file *fp)
{
	if (fp != NULL) {
		fp->_lockdepth--;
		fp->_locktotal++;
	}
}

/*
 * __sputc(): store one byte, or fail if the stream has no room or is not open
 * for writing.  Returns the byte as an unsigned char, like FreeBSD's macro.
 */
int
pbsd_shim_sputc(int c, struct pbsd_shim_file *fp)
{
	pbsd_shim_sputc_last_c = c;
	pbsd_shim_sputc_calls++;
	pbsd_shim_sputc_trace = pbsd_shim_sputc_trace * 1000003ULL +
	    (unsigned long long)(unsigned int)c;
	if ((fp->_flags & __SWR) == 0) {
		fp->_flags |= __SERR;
		return (EOF);
	}
	if (fp->_w <= 0) {
		fp->_flags |= __SERR;
		return (EOF);
	}
	*fp->_p = (unsigned char)c;
	fp->_p++;
	fp->_w--;
	return ((int)(unsigned char)c);
}

/*
 * __sfvwrite(): copy uio_resid bytes, taken from the iovecs in order, into the
 * stream.  Short of room, or short of iovec bytes, it fails like FreeBSD's.
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

/*
 * __vfprintf(): the real glibc formatter, with the caller's saved errno in
 * place so that %m behaves as FreeBSD's serrno argument makes it behave, and
 * the caller's locale installed.  The result is then copied into the stream
 * honouring _p/_w exactly as FreeBSD's __sprint() does for an __SSTR file.
 */
#define PBSD_SHIM_PRINTF_MAX 4096

locale_t pbsd_shim_vfprintf_locale;
int pbsd_shim_vfprintf_serrno;

int
pbsd_shim_vfprintf(struct pbsd_shim_file *fp, locale_t locale, int serrno,
    const char *fmt, va_list ap)
{
	char tmp[PBSD_SHIM_PRINTF_MAX];
	locale_t oldloc;
	int saved_errno, n, i;

	pbsd_shim_vfprintf_locale = locale;
	pbsd_shim_vfprintf_serrno = serrno;
	pbsd_shim_vfprintf_calls++;
	pbsd_shim_vfprintf_entry_flags = fp->_flags;
	pbsd_shim_vfprintf_entry_file = fp->_file;
	pbsd_shim_vfprintf_entry_r = fp->_r;
	pbsd_shim_vfprintf_entry_w = fp->_w;
	pbsd_shim_vfprintf_entry_size = fp->_bf._size;
	pbsd_shim_vfprintf_entry_lbfsize = fp->_lbfsize;
	pbsd_shim_vfprintf_entry_orientation = fp->_orientation;
	pbsd_shim_vfprintf_entry_base = fp->_bf._base;
	pbsd_shim_vfprintf_entry_p = fp->_p;
	if ((fp->_flags & __SWR) == 0) {
		fp->_flags |= __SERR;
		return (-1);
	}
	saved_errno = errno;
	errno = serrno;
	oldloc = uselocale(locale);
	n = vsnprintf(tmp, sizeof(tmp), fmt, ap);
	uselocale(oldloc);
	errno = saved_errno;
	if (n < 0) {
		fp->_flags |= __SERR;
		return (-1);
	}
	for (i = 0; i < n && i < (int)sizeof(tmp) - 1; i++) {
		if (fp->_w <= 0) {
			fp->_flags |= __SERR;
			return (-1);
		}
		*fp->_p = (unsigned char)tmp[i];
		fp->_p++;
		fp->_w--;
	}
	return (n);
}

/*
 * Layout interrogation, so the harness and port.cppm can prove at run time
 * that their copy of struct pbsd_shim_file agrees with this one.
 */
size_t
pbsd_shim_file_layout(int which)
{
	switch (which) {
	case 0:
		return (sizeof(struct pbsd_shim_file));
	case 1:
		return (offsetof(struct pbsd_shim_file, _p));
	case 2:
		return (offsetof(struct pbsd_shim_file, _r));
	case 3:
		return (offsetof(struct pbsd_shim_file, _w));
	case 4:
		return (offsetof(struct pbsd_shim_file, _flags));
	case 5:
		return (offsetof(struct pbsd_shim_file, _file));
	case 6:
		return (offsetof(struct pbsd_shim_file, _bf));
	case 7:
		return (offsetof(struct pbsd_shim_file, _lbfsize));
	case 8:
		return (offsetof(struct pbsd_shim_file, _orientation));
	case 9:
		return (offsetof(struct pbsd_shim_file, _lockdepth));
	case 10:
		return (offsetof(struct pbsd_shim_file, _locktotal));
	case 11:
		return (offsetof(struct pbsd_shim_file, _mbstate));
	case 12:
		return (sizeof(struct __suio));
	case 13:
		return (sizeof(struct __siov));
	default:
		return ((size_t)-1);
	}
}

/*
 * Create the locales the encoders run under.  Returns 0 on success, or 1 if no
 * UTF-8 locale could be created (in which case the multibyte cases degenerate
 * to single-byte ones, identically on both sides).
 */
int
pbsd_shim_init(void)
{
	int degraded = 0;

	pbsd_shim_locale_c = newlocale(LC_ALL_MASK, "C", NULL);
	pbsd_shim_locale_utf8 = newlocale(LC_ALL_MASK, "C.UTF-8", NULL);
	if (pbsd_shim_locale_utf8 == NULL) {
		pbsd_shim_locale_utf8 = newlocale(LC_ALL_MASK, "en_US.UTF-8",
		    NULL);
	}
	if (pbsd_shim_locale_utf8 == NULL) {
		pbsd_shim_locale_utf8 = newlocale(LC_ALL_MASK, "C", NULL);
		degraded = 1;
	}
	if (pbsd_shim_locale_c == NULL || pbsd_shim_locale_utf8 == NULL)
		return (-1);
	return (degraded);
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
#define XLOCALE_CTYPE(loc) pbsd_shim_xlocale_ctype(loc)
#define __sputc(c, fp) pbsd_shim_sputc((c), (fp))
#define __sfvwrite(fp, uio) pbsd_shim_sfvwrite((fp), (uio))
#define __vfprintf(fp, loc, serrno, fmt, ap)				\
	pbsd_shim_vfprintf((fp), (loc), (serrno), (fmt), (ap))
#define FAKE_FILE { ._file = -1 }

#undef __fputwc
#undef fputwc_l
#undef fputwc
#undef fputs_unlocked
#undef fputs
#undef vsprintf_l
#define __fputwc ref___fputwc
#define fputwc_l ref_fputwc_l
#define fputwc ref_fputwc
#define fputs_unlocked ref_fputs_unlocked
#define fputs ref_fputs
#define vsprintf_l ref_vsprintf_l
/* vsprintf itself is renamed after the source's own #undef, further down. */

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

/*
 * Non-MT-safe version.
 */
wint_t
__fputwc(wchar_t wc, FILE *fp, locale_t locale)
{
	char buf[MB_LEN_MAX];
	size_t i, len;
	struct xlocale_ctype *l = XLOCALE_CTYPE(locale);

	if ((len = l->__wcrtomb(buf, wc, &fp->_mbstate)) == (size_t)-1) {
		fp->_flags |= __SERR;
		return (WEOF);
	}

	for (i = 0; i < len; i++)
		if (__sputc((unsigned char)buf[i], fp) == EOF)
			return (WEOF);

	return ((wint_t)wc);
}

/*
 * MT-safe version.
 */
wint_t
fputwc_l(wchar_t wc, FILE *fp, locale_t locale)
{
	wint_t r;
	FIX_LOCALE(locale);

	FLOCKFILE_CANCELSAFE(fp);
	ORIENT(fp, 1);
	r = __fputwc(wc, fp, locale);
	FUNLOCKFILE_CANCELSAFE();

	return (r);
}
wint_t
fputwc(wchar_t wc, FILE *fp)
{
	return fputwc_l(wc, fp, __get_locale());
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

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
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

#undef vsprintf	/* _FORTIFY_SOURCE */

#define vsprintf ref_vsprintf

int
vsprintf_l(char * __restrict str, locale_t locale,
    const char * __restrict fmt, __va_list ap)
{
	FILE f = FAKE_FILE;
	int serrno = errno;
	int ret;
	FIX_LOCALE(locale);

	f._flags = __SWR | __SSTR;
	f._bf._base = f._p = (unsigned char *)str;
	f._bf._size = f._w = INT_MAX;
	ret = __vfprintf(&f, locale, serrno, fmt, ap);
	*f._p = 0;
	return (ret);
}
int
vsprintf(char * __restrict str, const char * __restrict fmt, __va_list ap)
{
	return vsprintf_l(str, __get_locale(), fmt, ap);
}
