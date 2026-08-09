/*
 * PBSD batch b0318 -- C++23 port of
 *     lib/libc/stdio/fputws.c
 *     lib/libc/stdio/tmpfile.c
 *     lib/libc/stdio/xprintf_hexdump.c
 *     lib/libc/stdio/fgetwln.c
 *
 * Faithful translation: behaviour, signedness, evaluation order and pointer
 * arithmetic are preserved exactly.  Original copyright headers are retained
 * with each function.
 */

module;

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

#include <cassert>
#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <signal.h>
#include <unistd.h>

export module pbsd.lib.libc.stdio.b0318;

export inline constexpr int B0318_SERR = 0x0040;

#define	__SERR	0x0040

export struct __sbuf {
	unsigned char	*_base;
	int		 _size;
};

export struct pb_mbstate {
	int		 __want;
	unsigned int	 __ch;
	unsigned int	 __lbound;
};

export using pb_mbstate_t = pb_mbstate;

export struct __siov {
	void	*iov_base;
	size_t	 iov_len;
};

export struct __suio {
	__siov	*uio_iov;
	int	 uio_iovcnt;
	int	 uio_resid;
};

export struct b0318_FILE {
	short		 _flags;
	short		 _orientation;
	pb_mbstate_t	 _mbstate;
	__sbuf		 _lb;
	unsigned char	 _lb_inline[64];
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
};

export using FILE = b0318_FILE;

export struct xlocale_ctype {
	size_t	(*__wcsnrtombs)(char * __restrict, const wchar_t ** __restrict,
	    size_t, size_t, pb_mbstate_t * __restrict);
};

export using locale_t = xlocale_ctype *;

export struct printf_info {
	int	width;
	int	showsign;
	int	alt;
};

export struct __printf_io {
	int	_dummy;
};

extern "C" {
extern locale_t b0318_default_locale;
extern locale_t b0318_global_locale;
extern int b0318_prepwrite_ret;
extern int b0318_sfvwrite_ret;
extern int b0318_sfvwrite_calls;
extern unsigned char *b0318_sfvwrite_sink;
extern size_t b0318_sfvwrite_sink_cap;
extern size_t b0318_sfvwrite_sink_len;
extern int b0318_wcsnrtombs_fail_at;
extern int b0318_wcsnrtombs_call;
extern int b0318_fgetwc_global_pos;
extern int b0318_fgetwc_global_len;
extern wint_t b0318_fgetwc_global_vals[4096];
extern int b0318_slbexpand_fail;
extern int b0318_slbexpand_calls;
extern size_t b0318_slbexpand_last;
extern struct xlocale_ctype b0318_ctype;
extern const char *b0318_secure_getenv_val;
extern int b0318_asprintf_fail;
extern int b0318_asprintf_calls;
extern char b0318_asprintf_last_path[512];
extern int b0318_mkstemp_ret;
extern int b0318_mkstemp_calls;
extern char b0318_mkstemp_last[512];
extern int b0318_unlink_calls;
extern char b0318_unlink_last[512];
extern int b0318_sigprocmask_calls;
extern int b0318_fdopen_fail;
extern int b0318_fdopen_calls;
extern int b0318_fdopen_last_fd;
extern int b0318_close_calls;
extern int b0318_close_last_fd;
extern int b0318_tmpfile_errno;
extern FILE *b0318_fdopen_result;
extern int b0318_printf_puts_ret;
extern int b0318_printf_puts_calls;
extern const char *b0318_printf_puts_last_buf;
extern int b0318_printf_puts_last_len;
extern unsigned char b0318_printf_puts_capture[8192];
extern size_t b0318_printf_puts_capture_len;
extern int b0318_printf_flush_calls;
extern struct __printf_io *b0318_printf_flush_last_io;
extern locale_t b0318_get_locale(void);
extern int b0318_prepwrite(FILE *);
extern int b0318_sfvwrite(FILE *, __suio *);
extern wint_t b0318_fgetwc(FILE *, locale_t);
extern int b0318_slbexpand(FILE *, size_t);
extern int b0318_sferror(FILE *);
extern void b0318_file_init(FILE *);
extern const char *b0318_secure_getenv(const char *);
extern int b0318_asprintf(char **, const char *, ...);
extern int b0318_mkstemp(char *);
extern int b0318_unlink(const char *);
extern int b0318___libc_sigprocmask(int, const sigset_t *, sigset_t *);
extern FILE *b0318_fdopen(int, const char *);
extern int b0318__close(int);
extern int b0318_printf_puts(__printf_io *, const char *, int);
extern void b0318_printf_flush(__printf_io *);
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

#define	prepwrite		b0318_prepwrite
#define	__sfvwrite		b0318_sfvwrite
#define	__fgetwc		b0318_fgetwc
#define	__slbexpand		b0318_slbexpand
#define	__sferror		b0318_sferror
#define	secure_getenv		b0318_secure_getenv
#define	asprintf		b0318_asprintf
#define	__libc_sigprocmask	b0318___libc_sigprocmask
#define	mkstemp			b0318_mkstemp
#define	unlink			b0318_unlink
#define	fdopen			b0318_fdopen
#define	_close			b0318__close
#define	__printf_puts		b0318_printf_puts
#define	__printf_flush		b0318_printf_flush

export namespace pbsd::lib_libc_stdio::b0318 {

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
fputws_l(const wchar_t * __restrict ws, FILE * __restrict fp, locale_t locale)
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
fputws(const wchar_t * __restrict ws, FILE * __restrict fp)
{
	return fputws_l(ws, fp, __get_locale());
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
tmpfile(void)
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
__printf_arginfo_hexdump(const struct printf_info *pi, size_t n, int *argt)
{

	assert(n >= 2);
	argt[0] = PA_POINTER;
	argt[1] = PA_INT;
	return (2);
}

int
__printf_render_hexdump(struct __printf_io *io, const struct printf_info *pi, const void *const *arg)
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

wchar_t *fgetwln_l(FILE * __restrict, size_t *, locale_t);

wchar_t *
fgetwln_l(FILE * __restrict fp, size_t *lenp, locale_t locale)
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
fgetwln(FILE * __restrict fp, size_t *lenp)
{
	return fgetwln_l(fp, lenp, __get_locale());
}

} // namespace pbsd::lib_libc_stdio::b0318
