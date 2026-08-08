/*
 * Reference oracle for batch b0041.
 *
 * Original HardenedBSD sources concatenated, with every function renamed with
 * a ref_ prefix.  Function bodies are UNMODIFIED.
 *
 * Sources:
 *   hbsd/src/lib/libc/locale/wcstombs.c
 *   hbsd/src/lib/libc/locale/wctomb.c
 *   hbsd/src/lib/libc/locale/btowc.c
 *   hbsd/src/lib/libc/locale/mblen.c
 *
 * Private libc headers (mblocal.h, xlocale_private.h) and the UTF-8 ctype
 * backend from utf8.c are not part of this batch.  The declarations and mock
 * implementations below stand in for them so the unmodified function bodies
 * compile and link.  FreeBSD uses a 128-byte mbstate_t; glibc's is smaller, so
 * __mbstate_t is defined before <wchar.h> to match FreeBSD.
 */

#ifndef SIZE_T_MAX
#define SIZE_T_MAX	((size_t)-1)
#endif

#ifndef LONG_BIT
#define LONG_BIT	(sizeof(long) * CHAR_BIT)
#endif

#define __mbstate_t_defined 1
typedef union {
	char		__mbstate8[128];
	long long	_mbstateL;
} __mbstate_t;
typedef __mbstate_t mbstate_t;

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#ifndef MB_LEN_MAX
#define MB_LEN_MAX	4
#endif

#ifndef MB_CUR_MAX
#define MB_CUR_MAX	4
#endif

#define MIN(a, b)	((a) < (b) ? (a) : (b))

typedef struct xlocale *locale_t;

typedef struct {
	wchar_t	ch;
	int	want;
	wchar_t	lbound;
} _UTF8State;

enum {
	XLC_CTYPE = 1,
};

struct xlocale_ctype {
	size_t (*__mbrtowc)(wchar_t * __restrict, const char * __restrict,
	    size_t, mbstate_t * __restrict);
	size_t (*__wcrtomb)(char * __restrict, wchar_t, mbstate_t * __restrict);
	size_t (*__wcsnrtombs)(char * __restrict, const wchar_t ** __restrict,
	    size_t, size_t, mbstate_t * __restrict);
	mbstate_t	mblen;
	mbstate_t	wctomb;
};

struct xlocale {
	void		*components[6];
};

#define XLOCALE_CTYPE(x)	((struct xlocale_ctype *)(x)->components[XLC_CTYPE])

struct xlocale_ctype	ref_global_ctype;
struct xlocale		ref_global_locale;

static size_t	mock_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);
static size_t	mock_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict);
static size_t	mock_wcsnrtombs(char * __restrict,
		    const wchar_t ** __restrict, size_t, size_t,
		    mbstate_t * __restrict);

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
#define __get_locale()	ref_get_locale()

static void __attribute__((constructor))
ref_locale_init(void)
{

	ref_global_ctype.__mbrtowc = mock_mbrtowc;
	ref_global_ctype.__wcrtomb = mock_wcrtomb;
	ref_global_ctype.__wcsnrtombs = mock_wcsnrtombs;
	memset(&ref_global_ctype.mblen, 0, sizeof(ref_global_ctype.mblen));
	memset(&ref_global_ctype.wctomb, 0, sizeof(ref_global_ctype.wctomb));
	ref_global_locale.components[XLC_CTYPE] = &ref_global_ctype;
}

static size_t
mock_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps)
{
	_UTF8State *us;
	int ch, i, mask, want;
	wchar_t lbound, wch;

	us = (_UTF8State *)ps;

	if (us->want < 0 || us->want > 6) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL) {
		s = "";
		n = 1;
		pwc = NULL;
	}

	if (n == 0)
		return ((size_t)-2);

	if (us->want == 0) {
		ch = (unsigned char)*s;
		if ((ch & 0x80) == 0) {
			if (pwc != NULL)
				*pwc = ch;
			return (ch != '\0' ? 1 : 0);
		}
		if ((ch & 0xe0) == 0xc0) {
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
	} else {
		want = us->want;
		lbound = us->lbound;
	}

	if (us->want == 0)
		wch = (unsigned char)*s++ & mask;
	else
		wch = us->ch;

	for (i = (us->want == 0) ? 1 : 0; i < MIN(want, n); i++) {
		if ((*s & 0xc0) != 0x80) {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		wch <<= 6;
		wch |= *s++ & 0x3f;
	}
	if (i < want) {
		us->want = want - i;
		us->lbound = lbound;
		us->ch = wch;
		return ((size_t)-2);
	}
	if (wch < lbound) {
		errno = EILSEQ;
		return ((size_t)-1);
	}
	if ((wch >= 0xd800 && wch <= 0xdfff) || wch > 0x10ffff) {
		errno = EILSEQ;
		return ((size_t)-1);
	}
	if (pwc != NULL)
		*pwc = wch;
	us->want = 0;
	return (wch == L'\0' ? 0 : want);
}

static size_t
mock_wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps)
{
	_UTF8State *us;
	unsigned char lead;
	int i, len;

	us = (_UTF8State *)ps;

	if (us->want != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL)
		return (1);

	if ((wc & ~0x7f) == 0) {
		*s = (char)wc;
		return (1);
	} else if ((wc & ~0x7ff) == 0) {
		lead = 0xc0;
		len = 2;
	} else if ((wc & ~0xffff) == 0) {
		if (wc >= 0xd800 && wc <= 0xdfff) {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		lead = 0xe0;
		len = 3;
	} else if (wc >= 0 && wc <= 0x10ffff) {
		lead = 0xf0;
		len = 4;
	} else {
		errno = EILSEQ;
		return ((size_t)-1);
	}

	for (i = len - 1; i > 0; i--) {
		s[i] = (wc & 0x3f) | 0x80;
		wc >>= 6;
	}
	*s = (wc & 0xff) | lead;

	return (len);
}

static size_t
mock_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
    size_t nwc, size_t len, mbstate_t * __restrict ps)
{
	_UTF8State *us;
	char buf[MB_LEN_MAX];
	const wchar_t *s;
	size_t nbytes;
	size_t nb;

	us = (_UTF8State *)ps;

	if (us->want != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	s = *src;
	nbytes = 0;

	if (dst == NULL) {
		while (nwc-- > 0) {
			if (0 <= *s && *s < 0x80)
				nb = 1;
			else if ((nb = mock_wcrtomb(buf, *s, ps)) ==
			    (size_t)-1)
				return ((size_t)-1);
			if (*s == L'\0')
				return (nbytes + nb - 1);
			s++;
			nbytes += nb;
		}
		return (nbytes);
	}

	while (len > 0 && nwc-- > 0) {
		if (0 <= *s && *s < 0x80) {
			nb = 1;
			*dst = *s;
		} else if (len > (size_t)MB_CUR_MAX) {
			if ((nb = mock_wcrtomb(dst, *s, ps)) == (size_t)-1) {
				*src = s;
				return ((size_t)-1);
			}
		} else {
			if ((nb = mock_wcrtomb(buf, *s, ps)) == (size_t)-1) {
				*src = s;
				return ((size_t)-1);
			}
			if (nb > (int)len)
				break;
			memcpy(dst, buf, nb);
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

/* wcstombs.c */

size_t
ref_wcstombs_l(char * __restrict s, const wchar_t * __restrict pwcs, size_t n,
		locale_t locale)
{
	static const mbstate_t initial;
	mbstate_t mbs;
	const wchar_t *pwcsp;
	FIX_LOCALE(locale);

	mbs = initial;
	pwcsp = pwcs;
	return (XLOCALE_CTYPE(locale)->__wcsnrtombs(s, &pwcsp, SIZE_T_MAX, n, &mbs));
}
size_t
ref_wcstombs(char * __restrict s, const wchar_t * __restrict pwcs, size_t n)
{
	return ref_wcstombs_l(s, pwcs, n, __get_locale());
}

/* wctomb.c */

int
ref_wctomb_l(char *s, wchar_t wchar, locale_t locale)
{
	static const mbstate_t initial;
	size_t rval;
	FIX_LOCALE(locale);

	if (s == NULL) {
		/* No support for state dependent encodings. */
		XLOCALE_CTYPE(locale)->wctomb = initial;
		return (0);
	}
	if ((rval = XLOCALE_CTYPE(locale)->__wcrtomb(s, wchar,
	    &(XLOCALE_CTYPE(locale)->wctomb))) == (size_t)-1)
		return (-1);
	return ((int)rval);
}
int
ref_wctomb(char *s, wchar_t wchar)
{
	return ref_wctomb_l(s, wchar, __get_locale());
}

/* btowc.c */

wint_t
ref_btowc_l(int c, locale_t l)
{
	static const mbstate_t initial;
	mbstate_t mbs = initial;
	char cc;
	wchar_t wc;
	FIX_LOCALE(l);

	if (c == EOF)
		return (WEOF);
	/*
	 * We expect mbrtowc() to return 0 or 1, hence the check for n > 1
	 * which detects error return values as well as "impossible" byte
	 * counts.
	 */
	cc = (char)c;
	if (XLOCALE_CTYPE(l)->__mbrtowc(&wc, &cc, 1, &mbs) > 1)
		return (WEOF);
	return (wc);
}
wint_t
ref_btowc(int c)
{
	return ref_btowc_l(c, __get_locale());
}

/* mblen.c */

int
ref_mblen_l(const char *s, size_t n, locale_t locale)
{
	static const mbstate_t initial;
	size_t rval;
	FIX_LOCALE(locale);

	if (s == NULL) {
		/* No support for state dependent encodings. */
		XLOCALE_CTYPE(locale)->mblen = initial;
		return (0);
	}
	rval = XLOCALE_CTYPE(locale)->__mbrtowc(NULL, s, n,
	    &(XLOCALE_CTYPE(locale)->mblen));
	if (rval == (size_t)-1 || rval == (size_t)-2)
		return (-1);
	return ((int)rval);
}

int
ref_mblen(const char *s, size_t n)
{
	return ref_mblen_l(s, n, __get_locale());
}
