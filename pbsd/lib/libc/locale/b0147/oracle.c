/*
 * Reference oracle for batch b0147.
 *
 * Original HardenedBSD sources concatenated, every function renamed with a
 * ref_ prefix.  Function bodies are UNMODIFIED.
 *
 * Sources:
 *   hbsd/src/lib/libc/locale/big5.c
 *   hbsd/src/lib/libc/locale/gbk.c
 *   hbsd/src/lib/libc/locale/ascii.c
 *   hbsd/src/lib/libc/locale/mskanji.c
 *
 * Private libc headers (runetype.h, mblocal.h) are not part of this batch.
 * The declarations and helper implementations below stand in for them so the
 * unmodified function bodies compile and link.
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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <wchar.h>

#ifndef __unused
#define __unused	__attribute__((__unused__))
#endif

#ifndef u_int
typedef unsigned int u_int;
#endif

#ifndef MB_LEN_MAX
#define MB_LEN_MAX	4
#endif

#ifndef MB_CUR_MAX
#define MB_CUR_MAX	4
#endif

typedef int __ct_rune_t;
typedef __ct_rune_t __rune_t;

typedef struct {
	__rune_t	__min;
	__rune_t	__max;
	__rune_t	__map;
	unsigned long	*__types;
} _RuneEntry;

typedef struct {
	int		__nranges;
	_RuneEntry	*__ranges;
} _RuneRange;

typedef struct {
	char		__magic[8];
	char		__encoding[32];
	__rune_t	(*__sgetrune)(const char *, size_t, char const **);
	int		(*__sputrune)(__rune_t, char *, size_t, char **);
	__rune_t	__invalid_rune;
	unsigned long	__runetype[256];
	__rune_t	__maplower[256];
	__rune_t	__mapupper[256];
	_RuneRange	__runetype_ext;
	_RuneRange	__maplower_ext;
	_RuneRange	__mapupper_ext;
	void		*__variable;
	int		__variable_len;
} _RuneLocale;

struct xlocale_ctype {
	size_t		(*__mbrtowc)(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);
	int		(*__mbsinit)(const mbstate_t *);
	size_t		(*__mbsnrtowcs)(wchar_t * __restrict, const char ** __restrict,
		    size_t, size_t, mbstate_t * __restrict);
	size_t		(*__wcrtomb)(char * __restrict, wchar_t, mbstate_t * __restrict);
	size_t		(*__wcsnrtombs)(char * __restrict, const wchar_t ** __restrict,
		    size_t, size_t, mbstate_t * __restrict);
	_RuneLocale	*runes;
	int		__mb_cur_max;
	int		__mb_sb_limit;
};

typedef size_t (*mbrtowc_pfn_t)(wchar_t * __restrict,
    const char * __restrict, size_t, mbstate_t * __restrict);
typedef size_t (*wcrtomb_pfn_t)(char * __restrict, wchar_t,
    mbstate_t * __restrict);

int __mb_sb_limit;

size_t
__mbsnrtowcs_std(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps,
    mbrtowc_pfn_t pmbrtowc)
{
	const char *s;
	size_t nchr;
	wchar_t wc;
	size_t nb;

	s = *src;
	nchr = 0;

	if (dst == NULL) {
		for (;;) {
			if ((nb = pmbrtowc(&wc, s, nms, ps)) == (size_t)-1)
				/* Invalid sequence - mbrtowc() sets errno. */
				return ((size_t)-1);
			else if (nb == 0 || nb == (size_t)-2)
				return (nchr);
			s += nb;
			nms -= nb;
			nchr++;
		}
		/*NOTREACHED*/
	}

	while (len-- > 0) {
		if ((nb = pmbrtowc(dst, s, nms, ps)) == (size_t)-1) {
			*src = s;
			return ((size_t)-1);
		} else if (nb == (size_t)-2) {
			*src = s + nms;
			return (nchr);
		} else if (nb == 0) {
			*src = NULL;
			return (nchr);
		}
		s += nb;
		nms -= nb;
		nchr++;
		dst++;
	}
	*src = s;
	return (nchr);
}

size_t
__wcsnrtombs_std(char * __restrict dst, const wchar_t ** __restrict src,
    size_t nwc, size_t len, mbstate_t * __restrict ps,
    wcrtomb_pfn_t pwcrtomb)
{
	mbstate_t mbsbak;
	char buf[MB_LEN_MAX];
	const wchar_t *s;
	size_t nbytes;
	size_t nb;

	s = *src;
	nbytes = 0;

	if (dst == NULL) {
		while (nwc-- > 0) {
			if ((nb = pwcrtomb(buf, *s, ps)) == (size_t)-1)
				/* Invalid character - wcrtomb() sets errno. */
				return ((size_t)-1);
			else if (*s == L'\0')
				return (nbytes + nb - 1);
			s++;
			nbytes += nb;
		}
		return (nbytes);
	}

	while (len > 0 && nwc-- > 0) {
		if (len > (size_t)MB_CUR_MAX) {
			/* Enough space to translate in-place. */
			if ((nb = pwcrtomb(dst, *s, ps)) == (size_t)-1) {
				*src = s;
				return ((size_t)-1);
			}
		} else {
			/*
			 * May not be enough space; use temp. buffer.
			 *
			 * We need to save a copy of the conversion state
			 * here so we can restore it if the multibyte
			 * character is too long for the buffer.
			 */
			mbsbak = *ps;
			if ((nb = pwcrtomb(buf, *s, ps)) == (size_t)-1) {
				*src = s;
				return ((size_t)-1);
			}
			if (nb > (int)len) {
				/* MB sequence for character won't fit. */
				*ps = mbsbak;
				break;
			}
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

/* ------------------------------------------------------------------ */
/* hbsd/src/lib/libc/locale/big5.c					*/
/* ------------------------------------------------------------------ */

typedef struct {
	wchar_t	ch;
} _BIG5State;

size_t	ref__BIG5_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);
int	ref__BIG5_mbsinit(const mbstate_t *);
size_t	ref__BIG5_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict);
size_t	ref__BIG5_mbsnrtowcs(wchar_t * __restrict,
		    const char ** __restrict, size_t, size_t,
		    mbstate_t * __restrict);
size_t	ref__BIG5_wcsnrtombs(char * __restrict,
		    const wchar_t ** __restrict, size_t, size_t,
		    mbstate_t * __restrict);

int
ref__BIG5_init(struct xlocale_ctype *l, _RuneLocale *rl)
{

	l->__mbrtowc = ref__BIG5_mbrtowc;
	l->__wcrtomb = ref__BIG5_wcrtomb;
	l->__mbsnrtowcs = ref__BIG5_mbsnrtowcs;
	l->__wcsnrtombs = ref__BIG5_wcsnrtombs;
	l->__mbsinit = ref__BIG5_mbsinit;
	l->runes = rl;
	l->__mb_cur_max = 2;
	l->__mb_sb_limit = 128;
	return (0);
}

int
ref__BIG5_mbsinit(const mbstate_t *ps)
{

	return (ps == NULL || ((const _BIG5State *)ps)->ch == 0);
}

int
ref__big5_check(u_int c)
{

	c &= 0xff;
	return ((c >= 0xa1 && c <= 0xfe) ? 2 : 1);
}

size_t
ref__BIG5_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps)
{
	_BIG5State *bs;
	wchar_t wc;
	size_t len;

	bs = (_BIG5State *)ps;

	if ((bs->ch & ~0xFF) != 0) {
		/* Bad conversion state. */
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL) {
		s = "";
		n = 1;
		pwc = NULL;
	}

	if (n == 0)
		/* Incomplete multibyte sequence */
		return ((size_t)-2);

	if (bs->ch != 0) {
		if (*s == '\0') {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		wc = (bs->ch << 8) | (*s & 0xFF);
		if (pwc != NULL)
			*pwc = wc;
		bs->ch = 0;
		return (1);
	}

	len = (size_t)ref__big5_check(*s);
	wc = *s++ & 0xff;
	if (len == 2) {
		if (n < 2) {
			/* Incomplete multibyte sequence */
			bs->ch = wc;
			return ((size_t)-2);
		}
		if (*s == '\0') {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		wc = (wc << 8) | (*s++ & 0xff);
		if (pwc != NULL)
			*pwc = wc;
		return (2);
	} else {
		if (pwc != NULL)
			*pwc = wc;
		return (wc == L'\0' ? 0 : 1);
	}
}

size_t
ref__BIG5_wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps)
{
	_BIG5State *bs;

	bs = (_BIG5State *)ps;

	if (bs->ch != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL)
		/* Reset to initial shift state (no-op) */
		return (1);
	if (wc & 0x8000) {
		*s++ = (wc >> 8) & 0xff;
		*s = wc & 0xff;
		return (2);
	}
	*s = wc & 0xff;
	return (1);
}

size_t
ref__BIG5_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps)
{
	return (__mbsnrtowcs_std(dst, src, nms, len, ps, ref__BIG5_mbrtowc));
}

size_t
ref__BIG5_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
    size_t nwc, size_t len, mbstate_t * __restrict ps)
{
	return (__wcsnrtombs_std(dst, src, nwc, len, ps, ref__BIG5_wcrtomb));
}

/* ------------------------------------------------------------------ */
/* hbsd/src/lib/libc/locale/gbk.c					*/
/* ------------------------------------------------------------------ */

typedef struct {
	wchar_t	ch;
} _GBKState;

size_t	ref__GBK_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);
int	ref__GBK_mbsinit(const mbstate_t *);
size_t	ref__GBK_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict);
size_t	ref__GBK_mbsnrtowcs(wchar_t * __restrict,
		    const char ** __restrict, size_t, size_t,
		    mbstate_t * __restrict);
size_t	ref__GBK_wcsnrtombs(char * __restrict,
		    const wchar_t ** __restrict, size_t, size_t,
		    mbstate_t * __restrict);

int
ref__GBK_init(struct xlocale_ctype *l, _RuneLocale *rl)
{

	l->__mbrtowc = ref__GBK_mbrtowc;
	l->__wcrtomb = ref__GBK_wcrtomb;
	l->__mbsinit = ref__GBK_mbsinit;
	l->__mbsnrtowcs = ref__GBK_mbsnrtowcs;
	l->__wcsnrtombs = ref__GBK_wcsnrtombs;
	l->runes = rl;
	l->__mb_cur_max = 2;
	l->__mb_sb_limit = 128;
	return (0);
}

int
ref__GBK_mbsinit(const mbstate_t *ps)
{

	return (ps == NULL || ((const _GBKState *)ps)->ch == 0);
}

int
ref__gbk_check(u_int c)
{

	c &= 0xff;
	return ((c >= 0x81 && c <= 0xfe) ? 2 : 1);
}

size_t
ref__GBK_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps)
{
	_GBKState *gs;
	wchar_t wc;
	size_t len;

	gs = (_GBKState *)ps;

	if ((gs->ch & ~0xFF) != 0) {
		/* Bad conversion state. */
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL) {
		s = "";
		n = 1;
		pwc = NULL;
	}

	if (n == 0)
		/* Incomplete multibyte sequence */
		return ((size_t)-2);

	if (gs->ch != 0) {
		if (*s == '\0') {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		wc = (gs->ch << 8) | (*s & 0xFF);
		if (pwc != NULL)
			*pwc = wc;
		gs->ch = 0;
		return (1);
	}

	len = (size_t)ref__gbk_check(*s);
	wc = *s++ & 0xff;
	if (len == 2) {
		if (n < 2) {
			/* Incomplete multibyte sequence */
			gs->ch = wc;
			return ((size_t)-2);
		}
		if (*s == '\0') {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		wc = (wc << 8) | (*s++ & 0xff);
		if (pwc != NULL)
			*pwc = wc;
		return (2);
	} else {
		if (pwc != NULL)
			*pwc = wc;
		return (wc == L'\0' ? 0 : 1);
	}
}

size_t
ref__GBK_wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps)
{
	_GBKState *gs;

	gs = (_GBKState *)ps;

	if (gs->ch != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL)
		/* Reset to initial shift state (no-op) */
		return (1);
	if (wc & 0x8000) {
		*s++ = (wc >> 8) & 0xff;
		*s = wc & 0xff;
		return (2);
	}
	*s = wc & 0xff;
	return (1);
}

size_t
ref__GBK_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps)
{
	return (__mbsnrtowcs_std(dst, src, nms, len, ps, ref__GBK_mbrtowc));
}

size_t
ref__GBK_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
    size_t nwc, size_t len, mbstate_t * __restrict ps)
{
	return (__wcsnrtombs_std(dst, src, nwc, len, ps, ref__GBK_wcrtomb));
}

/* ------------------------------------------------------------------ */
/* hbsd/src/lib/libc/locale/ascii.c					*/
/* ------------------------------------------------------------------ */

size_t	ref__ascii_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);
int	ref__ascii_mbsinit(const mbstate_t *);
size_t	ref__ascii_mbsnrtowcs(wchar_t * __restrict dst,
		    const char ** __restrict src, size_t nms, size_t len,
		    mbstate_t * __restrict ps __unused);
size_t	ref__ascii_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict);
size_t	ref__ascii_wcsnrtombs(char * __restrict, const wchar_t ** __restrict,
		    size_t, size_t, mbstate_t * __restrict);

int
ref__ascii_init(struct xlocale_ctype *l,_RuneLocale *rl)
{

	l->__mbrtowc = ref__ascii_mbrtowc;
	l->__mbsinit = ref__ascii_mbsinit;
	l->__mbsnrtowcs = ref__ascii_mbsnrtowcs;
	l->__wcrtomb = ref__ascii_wcrtomb;
	l->__wcsnrtombs = ref__ascii_wcsnrtombs;
	l->runes = rl;
	l->__mb_cur_max = 1;
	l->__mb_sb_limit = 128;
	return(0);
}

int
ref__ascii_mbsinit(const mbstate_t *ps __unused)
{

	/*
	 * Encoding is not state dependent - we are always in the
	 * initial state.
	 */
	return (1);
}

size_t
ref__ascii_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps __unused)
{

	if (s == NULL)
		/* Reset to initial shift state (no-op) */
		return (0);
	if (n == 0)
		/* Incomplete multibyte sequence */
		return ((size_t)-2);
	if (*s & 0x80) {
		errno = EILSEQ;
		return ((size_t)-1);
	}
	if (pwc != NULL)
		*pwc = (unsigned char)*s;
	return (*s == '\0' ? 0 : 1);
}

size_t
ref__ascii_wcrtomb(char * __restrict s, wchar_t wc,
    mbstate_t * __restrict ps __unused)
{

	if (s == NULL)
		/* Reset to initial shift state (no-op) */
		return (1);
	if (wc < 0 || wc > 127) {
		errno = EILSEQ;
		return ((size_t)-1);
	}
	*s = (unsigned char)wc;
	return (1);
}

size_t
ref__ascii_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps __unused)
{
	const char *s;
	size_t nchr;

	if (dst == NULL) {
		for (s = *src; nms > 0 && *s != '\0'; s++, nms--) {
			if (*s & 0x80) {
				errno = EILSEQ;
				return ((size_t)-1);
			}
		}
		return (s - *src);
	}

	s = *src;
	nchr = 0;
	while (len-- > 0 && nms-- > 0) {
		if (*s & 0x80) {
			*src = s;
			errno = EILSEQ;
			return ((size_t)-1);
		}
		if ((*dst++ = (unsigned char)*s++) == L'\0') {
			*src = NULL;
			return (nchr);
		}
		nchr++;
	}
	*src = s;
	return (nchr);
}

size_t
ref__ascii_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
    size_t nwc, size_t len, mbstate_t * __restrict ps __unused)
{
	const wchar_t *s;
	size_t nchr;

	if (dst == NULL) {
		for (s = *src; nwc > 0 && *s != L'\0'; s++, nwc--) {
			if (*s < 0 || *s > 127) {
				errno = EILSEQ;
				return ((size_t)-1);
			}
		}
		return (s - *src);
	}

	s = *src;
	nchr = 0;
	while (len-- > 0 && nwc-- > 0) {
		if (*s < 0 || *s > 127) {
			*src = s;
			errno = EILSEQ;
			return ((size_t)-1);
		}
		if ((*dst++ = *s++) == '\0') {
			*src = NULL;
			return (nchr);
		}
		nchr++;
	}
	*src = s;
	return (nchr);
}

/* ------------------------------------------------------------------ */
/* hbsd/src/lib/libc/locale/mskanji.c					*/
/* ------------------------------------------------------------------ */

typedef struct {
	wchar_t	ch;
} _MSKanjiState;

size_t	ref__MSKanji_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);
int	ref__MSKanji_mbsinit(const mbstate_t *);
size_t	ref__MSKanji_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict);
size_t	ref__MSKanji_mbsnrtowcs(wchar_t * __restrict,
		    const char ** __restrict, size_t, size_t,
		    mbstate_t * __restrict);
size_t	ref__MSKanji_wcsnrtombs(char * __restrict,
		    const wchar_t ** __restrict, size_t, size_t,
		    mbstate_t * __restrict);

int
ref__MSKanji_init(struct xlocale_ctype *l, _RuneLocale *rl)
{

	l->__mbrtowc = ref__MSKanji_mbrtowc;
	l->__wcrtomb = ref__MSKanji_wcrtomb;
	l->__mbsnrtowcs = ref__MSKanji_mbsnrtowcs;
	l->__wcsnrtombs = ref__MSKanji_wcsnrtombs;
	l->__mbsinit = ref__MSKanji_mbsinit;
	l->runes = rl;
	l->__mb_cur_max = 2;
	l->__mb_sb_limit = 224;
	return (0);
}

int
ref__MSKanji_mbsinit(const mbstate_t *ps)
{

	return (ps == NULL || ((const _MSKanjiState *)ps)->ch == 0);
}

size_t
ref__MSKanji_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps)
{
	_MSKanjiState *ms;
	wchar_t wc;

	ms = (_MSKanjiState *)ps;

	if ((ms->ch & ~0xFF) != 0) {
		/* Bad conversion state. */
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL) {
		s = "";
		n = 1;
		pwc = NULL;
	}

	if (n == 0)
		/* Incomplete multibyte sequence */
		return ((size_t)-2);

	if (ms->ch != 0) {
		if (*s == '\0') {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		wc = (ms->ch << 8) | (*s & 0xFF);
		if (pwc != NULL)
			*pwc = wc;
		ms->ch = 0;
		return (1);
	}
	wc = *s++ & 0xff;
	if ((wc > 0x80 && wc < 0xa0) || (wc >= 0xe0 && wc < 0xfd)) {
		if (n < 2) {
			/* Incomplete multibyte sequence */
			ms->ch = wc;
			return ((size_t)-2);
		}
		if (*s == '\0') {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		wc = (wc << 8) | (*s++ & 0xff);
		if (pwc != NULL)
			*pwc = wc;
		return (2);
	} else {
		if (pwc != NULL)
			*pwc = wc;
		return (wc == L'\0' ? 0 : 1);
	}
}

size_t
ref__MSKanji_wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps)
{
	_MSKanjiState *ms;
	int len, i;

	ms = (_MSKanjiState *)ps;

	if (ms->ch != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL)
		/* Reset to initial shift state (no-op) */
		return (1);
	len = (wc > 0x100) ? 2 : 1;
	for (i = len; i-- > 0; )
		*s++ = wc >> (i << 3);
	return (len);
}

size_t
ref__MSKanji_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps)
{
	return (__mbsnrtowcs_std(dst, src, nms, len, ps, ref__MSKanji_mbrtowc));
}

size_t
ref__MSKanji_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
    size_t nwc, size_t len, mbstate_t * __restrict ps)
{
	return (__wcsnrtombs_std(dst, src, nwc, len, ps, ref__MSKanji_wcrtomb));
}
