/*
 * oracle.c -- reference implementations for PBSD batch b0318.
 *
 * The original HardenedBSD sources for
 *     lib/libc/stdio/fputws.c
 *     lib/libc/stdio/tmpfile.c
 *     lib/libc/stdio/xprintf_hexdump.c
 *     lib/libc/stdio/fgetwln.c
 * concatenated, with every function renamed with a "ref_" prefix.  Function
 * bodies below the "ORIGINAL SOURCES" banner are UNMODIFIED.
 *
 * Everything above that banner is the environment the bodies compile against.
 */

#define _POSIX_C_SOURCE 200809L

#ifndef BUFSIZ
#define BUFSIZ 1024
#endif

#ifndef SIZE_T_MAX
#define SIZE_T_MAX ((size_t)-1)
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp"
#endif

#ifndef PA_POINTER
#define PA_POINTER 5
#endif

#ifndef PA_INT
#define PA_INT 1
#endif

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <unistd.h>

#undef locale_t

typedef struct xlocale_ctype *b0318_locale_t;
#define locale_t b0318_locale_t

/* ------------------------------------------------------------------ */
/* Mock stdio internals shared by fputws and fgetwln.                  */
/* ------------------------------------------------------------------ */

#define	__SERR	0x0040

struct __sbuf {
	unsigned char	*_base;
	int		 _size;
};

typedef struct pb_mbstate {
	int		 __want;
	unsigned int	 __ch;
	unsigned int	 __lbound;
} pb_mbstate_t;

struct __siov {
	void	*iov_base;
	size_t	 iov_len;
};

struct __suio {
	struct __siov	*uio_iov;
	int		 uio_iovcnt;
	int		 uio_resid;
};

typedef struct b0318_FILE {
	short		 _flags;
	short		 _orientation;
	pb_mbstate_t	 _mbstate;
	struct __sbuf	 _lb;
	unsigned char	 _lb_inline[64];
	/* mock plumbing */
	int		 prepwrite_ret;
	int		 sfvwrite_ret;
	int		 sfvwrite_calls;
	unsigned char	*sfvwrite_sink;
	size_t		 sfvwrite_sink_cap;
	size_t		 sfvwrite_sink_len;
	int		 fgetwc_pos;
	int		 fgetwc_len;
	wint_t		 fgetwc_vals[4096];
	int		 slbexpand_fail;
	int		 slbexpand_calls;
	size_t		 slbexpand_last;
} b0318_FILE;

#define	FILE			b0318_FILE

struct xlocale_ctype {
	size_t	(*__wcsnrtombs)(char * __restrict, const wchar_t ** __restrict,
	    size_t, size_t, pb_mbstate_t * __restrict);
};

static size_t b0318_wcsnrtombs_std(char * __restrict,
    const wchar_t ** __restrict, size_t, size_t, pb_mbstate_t * __restrict);

int b0318_prepwrite_ret = 0;
int b0318_sfvwrite_ret = 0;
int b0318_sfvwrite_calls = 0;
unsigned char *b0318_sfvwrite_sink = NULL;
size_t b0318_sfvwrite_sink_cap = 0;
size_t b0318_sfvwrite_sink_len = 0;

int b0318_wcsnrtombs_fail_at = -1;
int b0318_wcsnrtombs_call = 0;

int b0318_fgetwc_global_pos = 0;
int b0318_fgetwc_global_len = 0;
wint_t b0318_fgetwc_global_vals[4096];

int b0318_slbexpand_fail = 0;
int b0318_slbexpand_calls = 0;
size_t b0318_slbexpand_last = 0;

extern locale_t b0318_global_locale;

locale_t
b0318_get_locale(void)
{
	return (b0318_global_locale);
}

#define	__get_locale()		b0318_get_locale()
#define	FIX_LOCALE(loc)		do {					\
					if ((loc) == NULL)		\
						(loc) = __get_locale();	\
				} while (0)
#define	XLOCALE_CTYPE(loc)	((struct xlocale_ctype *)(loc))

#define	FLOCKFILE_CANCELSAFE(fp)	{ {
#define	FUNLOCKFILE_CANCELSAFE()	} }
#define	ORIENT(fp, direction)	do {					\
					(fp)->_orientation = (direction); \
				} while (0)

static size_t
b0318_simple_wcrtomb(char *s, wchar_t wc, pb_mbstate_t *ps)
{
	unsigned int ch;

	(void)ps;
	ch = (unsigned int)wc;
	if (ch > 0xff) {
		errno = EILSEQ;
		return ((size_t)-1);
	}
	s[0] = (char)ch;
	return (ch == 0 ? 0 : 1);
}

static size_t
b0318_wcsnrtombs_std(char * __restrict dst, const wchar_t ** __restrict src,
    size_t nwc, size_t len, pb_mbstate_t * __restrict ps)
{
	const wchar_t *s;
	size_t nbytes, nb;
	char buf[16];

	b0318_wcsnrtombs_call++;
	if (b0318_wcsnrtombs_fail_at >= 0 &&
	    b0318_wcsnrtombs_call - 1 == b0318_wcsnrtombs_fail_at)
		return ((size_t)-1);

	s = *src;
	nbytes = 0;
	if (dst == NULL) {
		while (nwc-- > 0) {
			if ((nb = b0318_simple_wcrtomb(buf, *s, ps)) ==
			    (size_t)-1)
				return ((size_t)-1);
			else if (*s == L'\0')
				return (nbytes + nb - 1);
			s++;
			nbytes += nb;
		}
		return (nbytes);
	}

	while (len > 0 && nwc-- > 0) {
		if ((nb = b0318_simple_wcrtomb(dst, *s, ps)) == (size_t)-1) {
			*src = s;
			return ((size_t)-1);
		}
		if (*s == L'\0') {
			*src = NULL;
			return (nbytes + nb - 1);
		}
		s++;
		dst += nb;
		len -= nb;
		nbytes += nb;
	}
	*src = s;
	return (nbytes);
}

struct xlocale_ctype b0318_ctype = { b0318_wcsnrtombs_std };

locale_t b0318_default_locale = &b0318_ctype;
locale_t b0318_global_locale = &b0318_ctype;

int
b0318_prepwrite(FILE *fp)
{
	return (fp->prepwrite_ret != 0 ? fp->prepwrite_ret : b0318_prepwrite_ret);
}

int
b0318_sfvwrite(FILE *fp, struct __suio *uio)
{
	struct __siov *iov;
	unsigned char *sink;
	size_t want, avail, put;
	int i;

	fp->sfvwrite_calls++;
	b0318_sfvwrite_calls++;
	if (fp->sfvwrite_ret != 0)
		return (fp->sfvwrite_ret);
	if (b0318_sfvwrite_ret != 0)
		return (b0318_sfvwrite_ret);

	sink = fp->sfvwrite_sink != NULL ? fp->sfvwrite_sink :
	    b0318_sfvwrite_sink;
	avail = fp->sfvwrite_sink != NULL ? fp->sfvwrite_sink_cap :
	    b0318_sfvwrite_sink_cap;
	if (sink == NULL)
		return (0);

	iov = uio->uio_iov;
	for (i = 0; i < uio->uio_iovcnt; i++) {
		want = iov[i].iov_len;
		if (fp->sfvwrite_sink != NULL)
			put = want < (fp->sfvwrite_sink_cap -
			    fp->sfvwrite_sink_len) ? want :
			    (fp->sfvwrite_sink_cap - fp->sfvwrite_sink_len);
		else
			put = want < (b0318_sfvwrite_sink_cap -
			    b0318_sfvwrite_sink_len) ? want :
			    (b0318_sfvwrite_sink_cap - b0318_sfvwrite_sink_len);
		if (put != 0)
			memcpy(sink + (fp->sfvwrite_sink != NULL ?
			    fp->sfvwrite_sink_len : b0318_sfvwrite_sink_len),
			    iov[i].iov_base, put);
		if (fp->sfvwrite_sink != NULL)
			fp->sfvwrite_sink_len += put;
		else
			b0318_sfvwrite_sink_len += put;
		uio->uio_resid -= (int)put;
		if (put < want)
			return (-1);
	}
	return (0);
}

wint_t
b0318_fgetwc(FILE *fp, locale_t locale)
{
	(void)locale;
	if (fp->fgetwc_len > 0) {
		if (fp->fgetwc_pos >= fp->fgetwc_len)
			return (WEOF);
		return (fp->fgetwc_vals[fp->fgetwc_pos++]);
	}
	if (b0318_fgetwc_global_pos >= b0318_fgetwc_global_len)
		return (WEOF);
	return (b0318_fgetwc_global_vals[b0318_fgetwc_global_pos++]);
}

int
b0318_slbexpand(FILE *fp, size_t size)
{
	unsigned char *nb;

	fp->slbexpand_calls++;
	b0318_slbexpand_calls++;
	fp->slbexpand_last = size;
	b0318_slbexpand_last = size;
	if (fp->slbexpand_fail || b0318_slbexpand_fail)
		return (1);
	if ((size_t)fp->_lb._size >= size)
		return (0);
	if (fp->_lb._base == fp->_lb_inline) {
		nb = malloc(size);
		if (nb == NULL)
			return (1);
		memcpy(nb, fp->_lb._base, (size_t)fp->_lb._size);
	} else {
		nb = realloc(fp->_lb._base, size);
		if (nb == NULL)
			return (1);
	}
	fp->_lb._base = nb;
	fp->_lb._size = (int)size;
	return (0);
}

int
b0318_sferror(FILE *fp)
{
	return ((fp->_flags & __SERR) != 0);
}

#define	prepwrite		b0318_prepwrite
#define	__sfvwrite		b0318_sfvwrite
#define	__fgetwc		b0318_fgetwc
#define	__slbexpand		b0318_slbexpand
#define	__sferror		b0318_sferror

void
b0318_file_init(FILE *fp)
{
	memset(fp, 0, sizeof(*fp));
	fp->_lb._base = fp->_lb_inline;
	fp->_lb._size = (int)sizeof(fp->_lb_inline);
}

/* ------------------------------------------------------------------ */
/* Mock plumbing for tmpfile().                                          */
/* ------------------------------------------------------------------ */

const char *b0318_secure_getenv_val = NULL;
int b0318_asprintf_fail = 0;
int b0318_asprintf_calls = 0;
char b0318_asprintf_last_path[512];
int b0318_mkstemp_ret = 3;
int b0318_mkstemp_calls = 0;
char b0318_mkstemp_last[512];
int b0318_unlink_calls = 0;
char b0318_unlink_last[512];
int b0318_sigprocmask_calls = 0;
int b0318_fdopen_fail = 0;
int b0318_fdopen_calls = 0;
int b0318_fdopen_last_fd = -1;
int b0318_close_calls = 0;
int b0318_close_last_fd = -1;
int b0318_tmpfile_errno = 0;
FILE *b0318_fdopen_result = NULL;

const char *
b0318_secure_getenv(const char *name)
{
	(void)name;
	return (b0318_secure_getenv_val);
}

int
b0318_asprintf(char **strp, const char *fmt, ...)
{
	va_list ap;
	int n;

	b0318_asprintf_calls++;
	if (b0318_asprintf_fail) {
		*strp = NULL;
		return (-1);
	}
	va_start(ap, fmt);
	n = vsnprintf(b0318_asprintf_last_path, sizeof(b0318_asprintf_last_path),
	    fmt, ap);
	va_end(ap);
	if (n < 0)
		return (-1);
	*strp = strdup(b0318_asprintf_last_path);
	return (*strp == NULL ? -1 : n);
}

int
b0318_mkstemp(char *tmpl)
{
	b0318_mkstemp_calls++;
	strncpy(b0318_mkstemp_last, tmpl, sizeof(b0318_mkstemp_last) - 1);
	b0318_mkstemp_last[sizeof(b0318_mkstemp_last) - 1] = '\0';
	if (b0318_mkstemp_ret < 0)
		return (-1);
	strcpy(tmpl, "/tmp/tmp.XXXXXX");
	return (b0318_mkstemp_ret);
}

int
b0318_unlink(const char *path)
{
	b0318_unlink_calls++;
	strncpy(b0318_unlink_last, path, sizeof(b0318_unlink_last) - 1);
	b0318_unlink_last[sizeof(b0318_unlink_last) - 1] = '\0';
	return (0);
}

int
b0318___libc_sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
	(void)how;
	(void)set;
	b0318_sigprocmask_calls++;
	if (oset != NULL)
		memset(oset, 0, sizeof(*oset));
	return (0);
}

FILE *
b0318_fdopen(int fd, const char *mode)
{
	(void)mode;
	b0318_fdopen_calls++;
	b0318_fdopen_last_fd = fd;
	if (b0318_fdopen_fail) {
		errno = b0318_tmpfile_errno != 0 ? b0318_tmpfile_errno : ENOMEM;
		return (NULL);
	}
	b0318_fdopen_result = calloc(1, sizeof(b0318_FILE));
	if (b0318_fdopen_result == NULL) {
		errno = ENOMEM;
		return (NULL);
	}
	return (b0318_fdopen_result);
}

int
b0318__close(int fd)
{
	b0318_close_calls++;
	b0318_close_last_fd = fd;
	return (0);
}

#define	secure_getenv		b0318_secure_getenv
#define	asprintf		b0318_asprintf
#define	__libc_sigprocmask	b0318___libc_sigprocmask
#define	mkstemp			b0318_mkstemp
#define	unlink			b0318_unlink
#define	fdopen			b0318_fdopen
#define	_close			b0318__close

/* ------------------------------------------------------------------ */
/* Mock plumbing for xprintf_hexdump.                                  */
/* ------------------------------------------------------------------ */

struct printf_info {
	int	width;
	int	showsign;
	int	alt;
};

struct __printf_io {
	int	_dummy;
};

int b0318_printf_puts_ret = 0;
int b0318_printf_puts_calls = 0;
const char *b0318_printf_puts_last_buf = NULL;
int b0318_printf_puts_last_len = 0;
unsigned char b0318_printf_puts_capture[8192];
size_t b0318_printf_puts_capture_len = 0;

int b0318_printf_flush_calls = 0;
struct __printf_io *b0318_printf_flush_last_io = NULL;

int
b0318_printf_puts(struct __printf_io *io, const char *buf, int len)
{
	size_t n;

	(void)io;
	b0318_printf_puts_calls++;
	b0318_printf_puts_last_buf = buf;
	b0318_printf_puts_last_len = len;
	n = (size_t)len;
	if (n > sizeof(b0318_printf_puts_capture) - b0318_printf_puts_capture_len)
		n = sizeof(b0318_printf_puts_capture) - b0318_printf_puts_capture_len;
	if (n > 0) {
		memcpy(b0318_printf_puts_capture + b0318_printf_puts_capture_len,
		    buf, n);
		b0318_printf_puts_capture_len += n;
	}
	return (b0318_printf_puts_ret);
}

void
b0318_printf_flush(struct __printf_io *io)
{
	b0318_printf_flush_calls++;
	b0318_printf_flush_last_io = io;
}

#define	__printf_puts		b0318_printf_puts
#define	__printf_flush		b0318_printf_flush

/* ====================================================================== */
/* ORIGINAL SOURCES                                                       */
/* ====================================================================== */

/* ====================================================================== */
/* lib/libc/stdio/fputws.c                                                */
/* ====================================================================== */

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

int
ref_fputws_l(const wchar_t * __restrict ws, FILE * __restrict fp, locale_t locale)
{
	size_t nbytes;
	char buf[BUFSIZ];
	struct __suio uio;
	struct __siov iov;
	const wchar_t *wsp;
	FIX_LOCALE(locale);
	struct xlocale_ctype *l = XLOCALE_CTYPE(locale);
	int ret;

	ret = -1;
	FLOCKFILE_CANCELSAFE(fp);
	ORIENT(fp, 1);
	if (prepwrite(fp) != 0)
		goto end;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	iov.iov_base = buf;
	wsp = ws;
	do {
		nbytes = l->__wcsnrtombs(buf, &wsp, SIZE_T_MAX, sizeof(buf),
		    &fp->_mbstate);
		if (nbytes == (size_t)-1)
			goto end;
		uio.uio_resid = iov.iov_len = nbytes;
		if (__sfvwrite(fp, &uio) != 0)
			goto end;
	} while (wsp != NULL);
	ret = 0;
end:
	FUNLOCKFILE_CANCELSAFE();
	return (ret);
}

int
ref_fputws(const wchar_t * __restrict ws, FILE * __restrict fp)
{
	return ref_fputws_l(ws, fp, __get_locale());
}

/* ====================================================================== */
/* lib/libc/stdio/tmpfile.c                                               */
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

FILE *
ref_tmpfile(void)
{
	sigset_t set, oset;
	FILE *fp;
	int fd, sverrno;
#define	TRAILER	"tmp.XXXXXX"
	char *buf;
	const char *tmpdir;

	tmpdir = secure_getenv("TMPDIR");
	if (tmpdir == NULL)
		tmpdir = _PATH_TMP;

	(void)asprintf(&buf, "%s%s%s", tmpdir,
	    (tmpdir[strlen(tmpdir) - 1] == '/') ? "" : "/", TRAILER);
	if (buf == NULL)
		return (NULL);

	sigfillset(&set);
	(void)__libc_sigprocmask(SIG_BLOCK, &set, &oset);

	fd = mkstemp(buf);
	if (fd != -1)
		(void)unlink(buf);

	free(buf);

	(void)__libc_sigprocmask(SIG_SETMASK, &oset, NULL);

	if (fd == -1)
		return (NULL);

	if ((fp = fdopen(fd, "w+")) == NULL) {
		sverrno = errno;
		(void)_close(fd);
		errno = sverrno;
		return (NULL);
	}
	return (fp);
}

/* ====================================================================== */
/* lib/libc/stdio/xprintf_hexdump.c                                       */
/* ====================================================================== */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2005 Poul-Henning Kamp
 * All rights reserved.
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

int
ref___printf_arginfo_hexdump(const struct printf_info *pi, size_t n, int *argt)
{

	assert(n >= 2);
	argt[0] = PA_POINTER;
	argt[1] = PA_INT;
	return (2);
}

int
ref___printf_render_hexdump(struct __printf_io *io, const struct printf_info *pi, const void *const *arg)
{
	unsigned char *p;
	int i;
	unsigned u, l, j, a;
	char buf[100], *q;
	int ret;

	if (pi->width > 0 && pi->width < 16)
		l = pi->width;
	else
		l = 16;
	p = *((unsigned char **)arg[0]);
	i = *((int *)arg[1]);
	if (i < 0)
		i = 0;
	u = i;

	ret = 0;
	a = 0;
	while (u > 0) {
		q = buf;
		if (pi->showsign)
			q += sprintf(q, " %04x", a);
		for (j = 0; j < l && j < u; j++)
			q += sprintf(q, " %02x", p[j]);
		if (pi->alt) {
			for (; j < l; j++)
				q += sprintf(q, "   ");
			q += sprintf(q, "  |");
			for (j = 0; j < l && j < u; j++) {
				if (p[j] < ' ' || p[j] > '~')
					*q++ = '.';
				else
					*q++ = p[j];
			}
			for (; j < l; j++)
				*q++ = ' ';
			*q++ = '|';
		}
		if (l < u)
			j = l;
		else
			j = u;
		p += j;
		u -= j;
		a += j;
		if (u > 0)
			*q++ = '\n';
		ret += __printf_puts(io, buf + 1, q - (buf + 1));
		__printf_flush(io);
	}
	return (ret);
}

/* ====================================================================== */
/* lib/libc/stdio/fgetwln.c                                               */
/* ====================================================================== */

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

wchar_t *ref_fgetwln_l(FILE * __restrict, size_t *, locale_t);

wchar_t *
ref_fgetwln_l(FILE * __restrict fp, size_t *lenp, locale_t locale)
{
	wchar_t *ret;
	wint_t wc;
	size_t len;
	int savserr;

	FIX_LOCALE(locale);

	FLOCKFILE_CANCELSAFE(fp);
	ORIENT(fp, 1);

	savserr = fp->_flags & __SERR;
	fp->_flags &= ~__SERR;

	len = 0;
	while ((wc = __fgetwc(fp, locale)) != WEOF) {
#define	GROW	512
		if (len * sizeof(wchar_t) >= fp->_lb._size &&
		    __slbexpand(fp, (len + GROW) * sizeof(wchar_t))) {
			fp->_flags |= __SERR;
			goto error;
		}
		*((wchar_t *)fp->_lb._base + len++) = wc;
		if (wc == L'\n')
			break;
	}
	/* fgetwc(3) may set both __SEOF and __SERR at once. */
	if (__sferror(fp))
		goto error;

	fp->_flags |= savserr;
	if (len == 0)
		goto error;

	*lenp = len;
	ret = (wchar_t *)fp->_lb._base;
end:
	FUNLOCKFILE_CANCELSAFE();
	return (ret);

error:
	*lenp = 0;
	ret = NULL;
	goto end;
}

wchar_t *
ref_fgetwln(FILE * __restrict fp, size_t *lenp)
{
	return ref_fgetwln_l(fp, lenp, __get_locale());
}
