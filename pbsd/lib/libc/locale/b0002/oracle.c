/*
 * Reference oracle for batch b0002.
 *
 * Original HardenedBSD sources (macro wrappers plus included template
 * headers), concatenated, with every function renamed with a ref_ prefix.
 * Function bodies are UNMODIFIED.
 *
 * Sources:
 *   hbsd/src/lib/libc/locale/c16rtomb_iconv.c  -> cXXrtomb_iconv.h
 *   hbsd/src/lib/libc/locale/c32rtomb_iconv.c  -> cXXrtomb_iconv.h
 *   hbsd/src/lib/libc/locale/mbrtoc16_iconv.c  -> mbrtocXX_iconv.h
 *   hbsd/src/lib/libc/locale/mbrtoc32_iconv.c  -> mbrtocXX_iconv.h
 *
 * Private libc headers (mblocal.h, citrus_iconv.h, xlocale_private.h) and
 * the citrus iconv engine are not part of this batch.  The declarations and
 * mock implementations below stand in for them so the unmodified function
 * bodies compile and link.  FreeBSD uses a 128-byte mbstate_t; glibc's is
 * smaller, so __mbstate_t is defined before <uchar.h> to match FreeBSD.
 */

#define __mbstate_t_defined 1
typedef union {
	char		__mbstate8[128];
	long long	_mbstateL;
} __mbstate_t;
typedef __mbstate_t mbstate_t;

#include <assert.h>
#include <errno.h>
#include <langinfo.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>

typedef struct xlocale *locale_t;

#define _CITRUS_ICONV_F_HIDE_INVALID	0x0001

struct _citrus_iconv_ops;
struct _citrus_iconv_shared;
struct _citrus_iconv;

typedef int (*_citrus_iconv_convert_t)(struct _citrus_iconv * __restrict,
    char * __restrict * __restrict, size_t * __restrict,
    char * __restrict * __restrict, size_t * __restrict, uint32_t,
    size_t * __restrict);

struct _citrus_iconv_ops {
	_citrus_iconv_convert_t	io_convert;
};

struct _citrus_iconv_shared {
	struct _citrus_iconv_ops	*ci_ops;
};

struct _citrus_iconv {
	struct _citrus_iconv_shared	*cv_shared;
	void				*cv_closure;
};

struct iconv_mock_session {
	char	from[64];
	char	to[64];
};

static struct _citrus_iconv_ops	iconv_mock_ops;
static struct _citrus_iconv_shared iconv_mock_shared;

int	ref_iconv_open_fail;

static size_t
pbsd_strlcpy(char *dst, const char *src, size_t dsize)
{
	size_t len = strlen(src);

	if (dsize != 0) {
		if (len >= dsize) {
			memcpy(dst, src, dsize - 1);
			dst[dsize - 1] = '\0';
		} else
			memcpy(dst, src, len + 1);
	}
	return (len);
}

static int	utf8_decode(const unsigned char *, size_t, uint32_t *,
		    size_t *, size_t *);
static int	utf8_encode(uint32_t, unsigned char *, size_t, size_t *);
static int	is_high_surrogate(char16_t);
static int	is_low_surrogate(char16_t);
static uint32_t	surrogate_pair_to_cp(char16_t, char16_t);
static void	cp_to_surrogate_pair(uint32_t, char16_t *, char16_t *);

static int
mock_iconv_convert(struct _citrus_iconv * __restrict cv,
    char * __restrict * __restrict in, size_t * __restrict inleft,
    char * __restrict * __restrict out, size_t * __restrict outleft,
    uint32_t flags, size_t * __restrict invalid)
{
	struct iconv_mock_session *s;
	int from16, from32, to16, to32;
	const unsigned char *src;
	unsigned char *dst;
	size_t srcl, dstl;
	uint32_t cp;
	size_t consumed, produced, inv;
	char16_t w1, w2;
	int err;

	(void)flags;
	s = cv->cv_closure;
	*invalid = 0;
	src = (const unsigned char *)*in;
	srcl = *inleft;
	dst = (unsigned char *)*out;
	dstl = *outleft;

	from16 = strcmp(s->from, "UTF-16-INTERNAL") == 0;
	from32 = strcmp(s->from, "UTF-32-INTERNAL") == 0;
	to16 = strcmp(s->to, "UTF-16-INTERNAL") == 0;
	to32 = strcmp(s->to, "UTF-32-INTERNAL") == 0;

	if (from16) {
		if (srcl < sizeof(char16_t))
			return (0);
		memcpy(&w1, src, sizeof(w1));
		if (is_high_surrogate(w1)) {
			if (srcl < 2 * sizeof(char16_t))
				return (EINVAL);
			memcpy(&w2, src + sizeof(char16_t), sizeof(w2));
			if (!is_low_surrogate(w2)) {
				*invalid = 2 * sizeof(char16_t);
				*in += 2 * sizeof(char16_t);
				*inleft -= 2 * sizeof(char16_t);
				return (0);
			}
			cp = surrogate_pair_to_cp(w1, w2);
			err = utf8_encode(cp, dst, dstl, &produced);
			if (err == E2BIG)
				return (E2BIG);
			if (err != 0) {
				*invalid = 2 * sizeof(char16_t);
				*in += 2 * sizeof(char16_t);
				*inleft -= 2 * sizeof(char16_t);
				return (0);
			}
			*in += 2 * sizeof(char16_t);
			*inleft -= 2 * sizeof(char16_t);
			*out += produced;
			*outleft -= produced;
			return (0);
		}
		if (is_low_surrogate(w1)) {
			*invalid = sizeof(char16_t);
			*in += sizeof(char16_t);
			*inleft -= sizeof(char16_t);
			return (0);
		}
		err = utf8_encode(w1, dst, dstl, &produced);
		if (err == E2BIG)
			return (E2BIG);
		*in += sizeof(char16_t);
		*inleft -= sizeof(char16_t);
		if (produced > 0) {
			*out += produced;
			*outleft -= produced;
		}
		return (0);
	}

	if (from32) {
		char32_t w;

		if (srcl < sizeof(char32_t))
			return (0);
		memcpy(&w, src, sizeof(w));
		if (w > 0x10FFFFu || (w >= 0xD800u && w <= 0xDFFFu)) {
			*invalid = sizeof(char32_t);
			*in += sizeof(char32_t);
			*inleft -= sizeof(char32_t);
			return (0);
		}
		err = utf8_encode(w, dst, dstl, &produced);
		if (err == E2BIG)
			return (E2BIG);
		*in += sizeof(char32_t);
		*inleft -= sizeof(char32_t);
		if (produced > 0) {
			*out += produced;
			*outleft -= produced;
		}
		return (0);
	}

	if (to16 || to32) {
		size_t need;

		err = utf8_decode(src, srcl, &cp, &consumed, &inv);
		if (err == -1)
			return (0);
		if (err == -2) {
			*invalid = inv;
			*in += inv;
			*inleft -= inv;
			return (0);
		}
		if (to16) {
			if (cp > 0xFFFFu) {
				cp_to_surrogate_pair(cp, &w1, &w2);
				need = 2 * sizeof(char16_t);
				if (dstl < need)
					return (E2BIG);
				memcpy(dst, &w1, sizeof(w1));
				memcpy(dst + sizeof(w1), &w2, sizeof(w2));
				produced = need;
			} else {
				w1 = (char16_t)cp;
				need = sizeof(char16_t);
				if (dstl < need)
					return (E2BIG);
				memcpy(dst, &w1, sizeof(w1));
				produced = need;
			}
		} else {
			need = sizeof(char32_t);
			if (dstl < need)
				return (E2BIG);
			memcpy(dst, &cp, sizeof(cp));
			produced = need;
		}
		*in += consumed;
		*inleft -= consumed;
		*out += produced;
		*outleft -= produced;
		return (0);
	}

	return (EINVAL);
}

int
_citrus_iconv_convert(struct _citrus_iconv * __restrict cv,
    char * __restrict * __restrict in, size_t * __restrict inbytes,
    char * __restrict * __restrict out, size_t * __restrict outbytes,
    uint32_t flags, size_t * __restrict nresults)
{

	return ((*cv->cv_shared->ci_ops->io_convert)(cv, in, inbytes, out,
	    outbytes, flags, nresults));
}

int
_citrus_iconv_open(struct _citrus_iconv * __restrict * __restrict rcv,
    const char * __restrict src, const char * __restrict dst)
{
	struct _citrus_iconv *cv;
	struct iconv_mock_session *s;

	if (ref_iconv_open_fail)
		return (-1);
	if (*rcv == NULL) {
		cv = malloc(sizeof(*cv));
		if (cv == NULL)
			return (-1);
	} else
		cv = *rcv;
	s = malloc(sizeof(*s));
	if (s == NULL)
		return (-1);
	pbsd_strlcpy(s->from, src, sizeof(s->from));
	pbsd_strlcpy(s->to, dst, sizeof(s->to));
	cv->cv_closure = s;
	cv->cv_shared = &iconv_mock_shared;
	*rcv = cv;
	return (0);
}

static int
utf8_decode(const unsigned char *p, size_t n, uint32_t *cp, size_t *consumed,
    size_t *invalid)
{
	uint32_t c;

	*invalid = 0;
	if (n == 0)
		return (-1);
	if (p[0] < 0x80) {
		*cp = p[0];
		*consumed = 1;
		return (0);
	}
	if ((p[0] & 0xE0) == 0xC0) {
		if (n < 2)
			return (-1);
		if ((p[1] & 0xC0) != 0x80) {
			*invalid = 1;
			*consumed = 1;
			return (-2);
		}
		c = ((p[0] & 0x1F) << 6) | (p[1] & 0x3F);
		if (c < 0x80) {
			*invalid = 2;
			*consumed = 2;
			return (-2);
		}
		*cp = c;
		*consumed = 2;
		return (0);
	}
	if ((p[0] & 0xF0) == 0xE0) {
		if (n < 3)
			return (-1);
		if ((p[1] & 0xC0) != 0x80 || (p[2] & 0xC0) != 0x80) {
			*invalid = 1;
			*consumed = 1;
			return (-2);
		}
		c = ((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
		if (c < 0x800) {
			*invalid = 3;
			*consumed = 3;
			return (-2);
		}
		*cp = c;
		*consumed = 3;
		return (0);
	}
	if ((p[0] & 0xF8) == 0xF0) {
		if (n < 4)
			return (-1);
		if ((p[1] & 0xC0) != 0x80 || (p[2] & 0xC0) != 0x80 ||
		    (p[3] & 0xC0) != 0x80) {
			*invalid = 1;
			*consumed = 1;
			return (-2);
		}
		c = ((p[0] & 0x07) << 18) | ((p[1] & 0x3F) << 12) |
		    ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
		if (c < 0x10000 || c > 0x10FFFF) {
			*invalid = 4;
			*consumed = 4;
			return (-2);
		}
		*cp = c;
		*consumed = 4;
		return (0);
	}
	*invalid = 1;
	*consumed = 1;
	return (-2);
}

static int
utf8_encode(uint32_t cp, unsigned char *dst, size_t dstl, size_t *produced)
{
	*produced = 0;
	if (cp < 0x80) {
		if (dstl < 1)
			return (E2BIG);
		dst[0] = (unsigned char)cp;
		*produced = 1;
		return (0);
	}
	if (cp < 0x800) {
		if (dstl < 2)
			return (E2BIG);
		dst[0] = (unsigned char)(0xC0 | (cp >> 6));
		dst[1] = (unsigned char)(0x80 | (cp & 0x3F));
		*produced = 2;
		return (0);
	}
	if (cp < 0x10000) {
		if (dstl < 3)
			return (E2BIG);
		dst[0] = (unsigned char)(0xE0 | (cp >> 12));
		dst[1] = (unsigned char)(0x80 | ((cp >> 6) & 0x3F));
		dst[2] = (unsigned char)(0x80 | (cp & 0x3F));
		*produced = 3;
		return (0);
	}
	if (cp <= 0x10FFFF) {
		if (dstl < 4)
			return (E2BIG);
		dst[0] = (unsigned char)(0xF0 | (cp >> 18));
		dst[1] = (unsigned char)(0x80 | ((cp >> 12) & 0x3F));
		dst[2] = (unsigned char)(0x80 | ((cp >> 6) & 0x3F));
		dst[3] = (unsigned char)(0x80 | (cp & 0x3F));
		*produced = 4;
		return (0);
	}
	return (EINVAL);
}

static int
is_high_surrogate(char16_t w)
{

	return (w >= 0xD800 && w <= 0xDBFF);
}

static int
is_low_surrogate(char16_t w)
{

	return (w >= 0xDC00 && w <= 0xDFFF);
}

static uint32_t
surrogate_pair_to_cp(char16_t hi, char16_t lo)
{

	return (0x10000 + (((hi - 0xD800) << 10) | (lo - 0xDC00)));
}

static void
cp_to_surrogate_pair(uint32_t cp, char16_t *hi, char16_t *lo)
{
	uint32_t t;

	t = cp - 0x10000;
	*hi = (char16_t)(0xD800 + (t >> 10));
	*lo = (char16_t)(0xDC00 + (t & 0x3FF));
}

enum {
	XLC_CTYPE = 1,
};

struct xlocale_ctype {
	mbstate_t	mbrtoc16;
	mbstate_t	mbrtoc32;
	mbstate_t	c16rtomb;
	mbstate_t	c32rtomb;
};

struct xlocale {
	void		*components[6];
};

#define XLOCALE_CTYPE(x)	((struct xlocale_ctype *)(x)->components[XLC_CTYPE])

struct xlocale_ctype	ref_global_ctype;
struct xlocale		ref_global_locale;

locale_t
ref_get_locale(void)
{

	return (&ref_global_locale);
}

static inline locale_t
ref_fix_locale(locale_t l)
{

	if (l == NULL)
		return (&ref_global_locale);
	return (l);
}

#define FIX_LOCALE(l)	((l) = ref_fix_locale(l))

#define MB_CUR_MAX_L(x)	((size_t)4)

char *
ref_nl_langinfo_l(nl_item item, locale_t locale)
{
	(void)locale;
	if (item == CODESET)
		return ("UTF-8");
	return ("");
}

static void __attribute__((constructor))
ref_locale_init(void)
{

	ref_global_locale.components[XLC_CTYPE] = &ref_global_ctype;
	iconv_mock_ops.io_convert = mock_iconv_convert;
	iconv_mock_shared.ci_ops = &iconv_mock_ops;
}

/* c16rtomb_iconv.c + cXXrtomb_iconv.h */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2013 Ed Schouten <ed@FreeBSD.org>
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

typedef struct {
	bool			initialized;
	struct _citrus_iconv	iconv;
	union {
		char16_t	widechar[2];
		char		bytes[sizeof(char16_t) * 2];
	} srcbuf;
	size_t			srcbuf_len;
} _ConversionState_c16rtomb;
_Static_assert(sizeof(_ConversionState_c16rtomb) <= sizeof(mbstate_t),
    "Size of _ConversionState must not exceed mbstate_t's size.");

size_t
ref_c16rtomb_l(char * __restrict s, char16_t c, mbstate_t * __restrict ps,
    locale_t locale)
{
	_ConversionState_c16rtomb *cs;
	struct _citrus_iconv *handle;
	char *src, *dst;
	size_t srcleft, dstleft, invlen;
	int err;

	FIX_LOCALE(locale);
	if (ps == NULL)
		ps = &(XLOCALE_CTYPE(locale)->c16rtomb);
	cs = (_ConversionState_c16rtomb *)ps;
	handle = &cs->iconv;

	if (s == NULL || !cs->initialized) {
		if (_citrus_iconv_open(&handle, "UTF-16-INTERNAL",
		    ref_nl_langinfo_l(CODESET, locale)) != 0) {
			cs->initialized = false;
			errno = EINVAL;
			return ((size_t)-1);
		}
		cs->srcbuf_len = 0;
		cs->initialized = true;
		if (s == NULL)
			return (1);
	}

	assert(cs->srcbuf_len < sizeof(cs->srcbuf.widechar) / sizeof(char16_t));
	cs->srcbuf.widechar[cs->srcbuf_len++] = c;

	src = cs->srcbuf.bytes;
	srcleft = cs->srcbuf_len * sizeof(char16_t);
	dst = s;
	dstleft = MB_CUR_MAX_L(locale);
	err = _citrus_iconv_convert(handle, &src, &srcleft, &dst, &dstleft,
	    _CITRUS_ICONV_F_HIDE_INVALID, &invlen);

	if (err == EINVAL)
		return (0);
	cs->srcbuf_len = 0;
	
	if (dst == s) {
		errno = EILSEQ;
		return ((size_t)-1);
	}
	return (dst - s);
}

size_t
ref_c16rtomb(char * __restrict s, char16_t c, mbstate_t * __restrict ps)
{

	return (ref_c16rtomb_l(s, c, ps, ref_get_locale()));
}

/* c32rtomb_iconv.c + cXXrtomb_iconv.h - same copyright as above */

#undef _ConversionState
typedef struct {
	bool			initialized;
	struct _citrus_iconv	iconv;
	union {
		char32_t	widechar[1];
		char		bytes[sizeof(char32_t) * 1];
	} srcbuf;
	size_t			srcbuf_len;
} _ConversionState_c32rtomb;
_Static_assert(sizeof(_ConversionState_c32rtomb) <= sizeof(mbstate_t),
    "Size of _ConversionState must not exceed mbstate_t's size.");

size_t
ref_c32rtomb_l(char * __restrict s, char32_t c, mbstate_t * __restrict ps,
    locale_t locale)
{
	_ConversionState_c32rtomb *cs;
	struct _citrus_iconv *handle;
	char *src, *dst;
	size_t srcleft, dstleft, invlen;
	int err;

	FIX_LOCALE(locale);
	if (ps == NULL)
		ps = &(XLOCALE_CTYPE(locale)->c32rtomb);
	cs = (_ConversionState_c32rtomb *)ps;
	handle = &cs->iconv;

	if (s == NULL || !cs->initialized) {
		if (_citrus_iconv_open(&handle, "UTF-32-INTERNAL",
		    ref_nl_langinfo_l(CODESET, locale)) != 0) {
			cs->initialized = false;
			errno = EINVAL;
			return ((size_t)-1);
		}
		cs->srcbuf_len = 0;
		cs->initialized = true;
		if (s == NULL)
			return (1);
	}

	assert(cs->srcbuf_len < sizeof(cs->srcbuf.widechar) / sizeof(char32_t));
	cs->srcbuf.widechar[cs->srcbuf_len++] = c;

	src = cs->srcbuf.bytes;
	srcleft = cs->srcbuf_len * sizeof(char32_t);
	dst = s;
	dstleft = MB_CUR_MAX_L(locale);
	err = _citrus_iconv_convert(handle, &src, &srcleft, &dst, &dstleft,
	    _CITRUS_ICONV_F_HIDE_INVALID, &invlen);

	if (err == EINVAL)
		return (0);
	cs->srcbuf_len = 0;
	
	if (dst == s) {
		errno = EILSEQ;
		return ((size_t)-1);
	}
	return (dst - s);
}

size_t
ref_c32rtomb(char * __restrict s, char32_t c, mbstate_t * __restrict ps)
{

	return (ref_c32rtomb_l(s, c, ps, ref_get_locale()));
}

/* mbrtoc16_iconv.c + mbrtocXX_iconv.h - same copyright as above */

#undef _ConversionState
typedef struct {
	bool			initialized;
	struct _citrus_iconv	iconv;
	char			srcbuf[MB_LEN_MAX];
	size_t			srcbuf_len;
	union {
		char16_t	widechar[2];
		char		bytes[sizeof(char16_t) * 2];
	} dstbuf;
	size_t			dstbuf_len;
} _ConversionState_mbrtoc16;
_Static_assert(sizeof(_ConversionState_mbrtoc16) <= sizeof(mbstate_t),
    "Size of _ConversionState must not exceed mbstate_t's size.");

size_t
ref_mbrtoc16_l(char16_t * __restrict pc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps, locale_t locale)
{
	_ConversionState_mbrtoc16 *cs;
	struct _citrus_iconv *handle;
	size_t i, retval;
	char16_t retchar;

	FIX_LOCALE(locale);
	if (ps == NULL)
		ps = &(XLOCALE_CTYPE(locale)->mbrtoc16);
	cs = (_ConversionState_mbrtoc16 *)ps;
	handle = &cs->iconv;

	if (s == NULL || !cs->initialized) {
		if (_citrus_iconv_open(&handle,
		    ref_nl_langinfo_l(CODESET, locale), "UTF-16-INTERNAL") != 0) {
			cs->initialized = false;
			errno = EINVAL;
			return ((size_t)-1);
		}
		cs->srcbuf_len = cs->dstbuf_len = 0;
		cs->initialized = true;
		if (s == NULL)
			return (0);
	}

	if (cs->dstbuf_len > 0) {
		retval = (size_t)-3;
		goto return_char;
	}

	if (n > sizeof(cs->srcbuf) - cs->srcbuf_len)
		n = sizeof(cs->srcbuf) - cs->srcbuf_len;
	memcpy(cs->srcbuf + cs->srcbuf_len, s, n);

	for (i = 0; ; i++) {
		char *src, *dst;
		size_t srcleft, dstleft, invlen;
		int err;

		src = cs->srcbuf;
		srcleft = cs->srcbuf_len + n;
		dst = cs->dstbuf.bytes;
		dstleft = i * sizeof(char16_t);
		assert(srcleft <= sizeof(cs->srcbuf) &&
		    dstleft <= sizeof(cs->dstbuf.bytes));
		err = _citrus_iconv_convert(handle, &src, &srcleft,
		    &dst, &dstleft, _CITRUS_ICONV_F_HIDE_INVALID, &invlen);
		cs->dstbuf_len = (dst - cs->dstbuf.bytes) / sizeof(char16_t);

		if (cs->dstbuf_len > 0) {
			assert(src - cs->srcbuf > cs->srcbuf_len);
			retval = src - cs->srcbuf - cs->srcbuf_len;
			cs->srcbuf_len = 0;
			goto return_char;
		}

		if (err == E2BIG)
			continue;

		if (invlen > 0) {
			cs->srcbuf_len = 0;
			errno = EILSEQ;
			return ((size_t)-1);
		}

		memmove(cs->srcbuf, src, srcleft);
		cs->srcbuf_len = srcleft;
		return ((size_t)-2);
	}

return_char:
	retchar = cs->dstbuf.widechar[0];
	memmove(&cs->dstbuf.widechar[0], &cs->dstbuf.widechar[1],
	    --cs->dstbuf_len * sizeof(char16_t));
	if (pc != NULL)
		*pc = retchar;
	if (retchar == 0)
		return (0);
	return (retval);
}

size_t
ref_mbrtoc16(char16_t * __restrict pc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps)
{

	return (ref_mbrtoc16_l(pc, s, n, ps, ref_get_locale()));
}

/* mbrtoc32_iconv.c + mbrtocXX_iconv.h - same copyright as above */

#undef _ConversionState
typedef struct {
	bool			initialized;
	struct _citrus_iconv	iconv;
	char			srcbuf[MB_LEN_MAX];
	size_t			srcbuf_len;
	union {
		char32_t	widechar[1];
		char		bytes[sizeof(char32_t) * 1];
	} dstbuf;
	size_t			dstbuf_len;
} _ConversionState_mbrtoc32;
_Static_assert(sizeof(_ConversionState_mbrtoc32) <= sizeof(mbstate_t),
    "Size of _ConversionState must not exceed mbstate_t's size.");

size_t
ref_mbrtoc32_l(char32_t * __restrict pc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps, locale_t locale)
{
	_ConversionState_mbrtoc32 *cs;
	struct _citrus_iconv *handle;
	size_t i, retval;
	char32_t retchar;

	FIX_LOCALE(locale);
	if (ps == NULL)
		ps = &(XLOCALE_CTYPE(locale)->mbrtoc32);
	cs = (_ConversionState_mbrtoc32 *)ps;
	handle = &cs->iconv;

	if (s == NULL || !cs->initialized) {
		if (_citrus_iconv_open(&handle,
		    ref_nl_langinfo_l(CODESET, locale), "UTF-32-INTERNAL") != 0) {
			cs->initialized = false;
			errno = EINVAL;
			return ((size_t)-1);
		}
		cs->srcbuf_len = cs->dstbuf_len = 0;
		cs->initialized = true;
		if (s == NULL)
			return (0);
	}

	if (cs->dstbuf_len > 0) {
		retval = (size_t)-3;
		goto return_char;
	}

	if (n > sizeof(cs->srcbuf) - cs->srcbuf_len)
		n = sizeof(cs->srcbuf) - cs->srcbuf_len;
	memcpy(cs->srcbuf + cs->srcbuf_len, s, n);

	for (i = 0; ; i++) {
		char *src, *dst;
		size_t srcleft, dstleft, invlen;
		int err;

		src = cs->srcbuf;
		srcleft = cs->srcbuf_len + n;
		dst = cs->dstbuf.bytes;
		dstleft = i * sizeof(char32_t);
		assert(srcleft <= sizeof(cs->srcbuf) &&
		    dstleft <= sizeof(cs->dstbuf.bytes));
		err = _citrus_iconv_convert(handle, &src, &srcleft,
		    &dst, &dstleft, _CITRUS_ICONV_F_HIDE_INVALID, &invlen);
		cs->dstbuf_len = (dst - cs->dstbuf.bytes) / sizeof(char32_t);

		if (cs->dstbuf_len > 0) {
			assert(src - cs->srcbuf > cs->srcbuf_len);
			retval = src - cs->srcbuf - cs->srcbuf_len;
			cs->srcbuf_len = 0;
			goto return_char;
		}

		if (err == E2BIG)
			continue;

		if (invlen > 0) {
			cs->srcbuf_len = 0;
			errno = EILSEQ;
			return ((size_t)-1);
		}

		memmove(cs->srcbuf, src, srcleft);
		cs->srcbuf_len = srcleft;
		return ((size_t)-2);
	}

return_char:
	retchar = cs->dstbuf.widechar[0];
	memmove(&cs->dstbuf.widechar[0], &cs->dstbuf.widechar[1],
	    --cs->dstbuf_len * sizeof(char32_t));
	if (pc != NULL)
		*pc = retchar;
	if (retchar == 0)
		return (0);
	return (retval);
}

size_t
ref_mbrtoc32(char32_t * __restrict pc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps)
{

	return (ref_mbrtoc32_l(pc, s, n, ps, ref_get_locale()));
}
