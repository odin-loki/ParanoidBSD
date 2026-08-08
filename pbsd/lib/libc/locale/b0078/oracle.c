/*
 * Reference oracle for batch b0078.
 *
 * Original HardenedBSD sources concatenated, every function renamed with a
 * ref_ prefix.  Function bodies are UNMODIFIED.
 *
 * Sources:
 *   hbsd/src/lib/libc/locale/runetype.c
 *   hbsd/src/lib/libc/locale/nextwctype.c
 *   hbsd/src/lib/libc/locale/mbsnrtowcs.c
 *
 * Private libc headers (runetype.h, mblocal.h, xlocale_private.h) are not
 * part of this batch.  The declarations and mock implementations below stand
 * in for them so the unmodified function bodies compile and link.
 */

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
#include <wchar.h>

#ifndef SIZE_T_MAX
#define SIZE_T_MAX	SIZE_MAX
#endif

#define	_CACHED_RUNES	(1 << 8)

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
	unsigned long	__runetype[_CACHED_RUNES];
	__rune_t	__maplower[_CACHED_RUNES];
	__rune_t	__mapupper[_CACHED_RUNES];
	_RuneRange	__runetype_ext;
	_RuneRange	__maplower_ext;
	_RuneRange	__mapupper_ext;
	void		*__variable;
	int		__variable_len;
} _RuneLocale;

typedef unsigned long wctype_t;

#ifndef EOF
#define EOF	(-1)
#endif

enum {
	XLC_CTYPE = 1,
};

struct ref_xlocale_ctype {
	_RuneLocale	*runes;
	size_t		(*__mbsnrtowcs)(wchar_t * __restrict, const char ** __restrict,
		    size_t, size_t, mbstate_t * __restrict);
	int		__mb_cur_max;
	mbstate_t	mbsnrtowcs;
};

struct ref_xlocale {
	void		*components[6];
};

typedef struct ref_xlocale *ref_locale_t;

struct ref_xlocale_ctype	ref_global_ctype;
struct ref_xlocale		ref_global_locale;
struct ref_xlocale_ctype	ref_alt_ctype;
struct ref_xlocale		ref_alt_locale;

ref_locale_t
ref_get_locale(void)
{

	return (&ref_global_locale);
}

static inline ref_locale_t
ref_fix_locale(ref_locale_t l)
{

	if (l == NULL)
		return (&ref_global_locale);
	return (l);
}

#define FIX_LOCALE(l)	((l) = ref_fix_locale(l))
#define XLOCALE_CTYPE(x)	((struct ref_xlocale_ctype *)(x)->components[XLC_CTYPE])
#define __get_locale()		(ref_get_locale())

typedef size_t (*mbrtowc_pfn_t)(wchar_t * __restrict,
    const char * __restrict, size_t, mbstate_t * __restrict);

static int	utf8_decode(const unsigned char *, size_t, wchar_t *,
		    size_t *);

size_t
test_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps)
{
	wchar_t wc;
	size_t consumed;
	int err;

	(void)ps;
	if (n == 0)
		return ((size_t)-2);
	err = utf8_decode((const unsigned char *)s, n, &wc, &consumed);
	if (err == -1)
		return ((size_t)-1);
	if (err == -2)
		return ((size_t)-2);
	if (pwc != NULL)
		*pwc = wc;
	if (wc == L'\0')
		return (0);
	return (consumed);
}

size_t
ref_backend_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps);

static int
utf8_decode(const unsigned char *p, size_t n, wchar_t *wc, size_t *consumed)
{
	uint32_t c;

	if (n == 0)
		return (-2);
	if (p[0] < 0x80) {
		*wc = (wchar_t)p[0];
		*consumed = 1;
		return (0);
	}
	if ((p[0] & 0xE0) == 0xC0) {
		if (n < 2)
			return (-2);
		if ((p[1] & 0xC0) != 0x80)
			return (-1);
		c = ((p[0] & 0x1F) << 6) | (p[1] & 0x3F);
		if (c < 0x80)
			return (-1);
		*wc = (wchar_t)c;
		*consumed = 2;
		return (0);
	}
	if ((p[0] & 0xF0) == 0xE0) {
		if (n < 3)
			return (-2);
		if ((p[1] & 0xC0) != 0x80 || (p[2] & 0xC0) != 0x80)
			return (-1);
		c = ((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
		if (c < 0x800)
			return (-1);
		*wc = (wchar_t)c;
		*consumed = 3;
		return (0);
	}
	if ((p[0] & 0xF8) == 0xF0) {
		if (n < 4)
			return (-2);
		if ((p[1] & 0xC0) != 0x80 || (p[2] & 0xC0) != 0x80 ||
		    (p[3] & 0xC0) != 0x80)
			return (-1);
		c = ((p[0] & 0x07) << 18) | ((p[1] & 0x3F) << 12) |
		    ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
		if (c < 0x10000 || c > 0x10FFFF)
			return (-1);
		*wc = (wchar_t)c;
		*consumed = 4;
		return (0);
	}
	return (-1);
}

void
ref_locale_init(struct ref_xlocale *loc, struct ref_xlocale_ctype *ctype,
    _RuneLocale *runes, int mb_cur_max)
{

	memset(loc, 0, sizeof(*loc));
	memset(ctype, 0, sizeof(*ctype));
	loc->components[XLC_CTYPE] = ctype;
	ctype->runes = runes;
	ctype->__mb_cur_max = mb_cur_max;
	ctype->__mbsnrtowcs = ref_backend_mbsnrtowcs;
}

/* ------------------------------------------------------------------ */
/* hbsd/src/lib/libc/locale/runetype.c					*/
/* ------------------------------------------------------------------ */

unsigned long
ref____runetype_l(__ct_rune_t c, ref_locale_t locale)
{
	size_t lim;
	FIX_LOCALE(locale);
	_RuneRange *rr = &(XLOCALE_CTYPE(locale)->runes->__runetype_ext);
	_RuneEntry *base, *re;

	if (c < 0 || c == EOF)
		return(0L);

	/* Binary search -- see bsearch.c for explanation. */
	base = rr->__ranges;
	for (lim = rr->__nranges; lim != 0; lim >>= 1) {
		re = base + (lim >> 1);
		if (re->__min <= c && c <= re->__max) {
			if (re->__types)
			    return(re->__types[c - re->__min]);
			else
			    return(re->__map);
		} else if (c > re->__max) {
			base = re + 1;
			lim--;
		}
	}

	return(0L);
}
unsigned long
ref____runetype(__ct_rune_t c)
{
	return ref____runetype_l(c, __get_locale());
}

int ref____mb_cur_max(void)
{
	return XLOCALE_CTYPE(__get_locale())->__mb_cur_max;
}
int ref____mb_cur_max_l(ref_locale_t locale)
{
	FIX_LOCALE(locale);
	return XLOCALE_CTYPE(locale)->__mb_cur_max;
}

/* ------------------------------------------------------------------ */
/* hbsd/src/lib/libc/locale/nextwctype.c					*/
/* ------------------------------------------------------------------ */

wint_t
ref_nextwctype_l(wint_t wc, wctype_t wct, ref_locale_t locale)
{
	size_t lim;
	FIX_LOCALE(locale);
	_RuneLocale *runes = XLOCALE_CTYPE(locale)->runes;
	_RuneRange *rr = &runes->__runetype_ext;
	_RuneEntry *base, *re;
	int noinc;

	noinc = 0;
	if (wc < _CACHED_RUNES) {
		wc++;
		while (wc < _CACHED_RUNES) {
			if (runes->__runetype[wc] & wct)
				return (wc);
			wc++;
		}
		wc--;
	}
	if (rr->__ranges != NULL && wc < rr->__ranges[0].__min) {
		wc = rr->__ranges[0].__min;
		noinc = 1;
	}

	/* Binary search -- see bsearch.c for explanation. */
	base = rr->__ranges;
	for (lim = rr->__nranges; lim != 0; lim >>= 1) {
		re = base + (lim >> 1);
		if (re->__min <= wc && wc <= re->__max)
			goto found;
		else if (wc > re->__max) {
			base = re + 1;
			lim--;
		}
	}
	return (-1);
found:
	if (!noinc)
		wc++;
	if (re->__min <= wc && wc <= re->__max) {
		if (re->__types != NULL) {
			for (; wc <= re->__max; wc++)
				if (re->__types[wc - re->__min] & wct)
					return (wc);
		} else if (re->__map & wct)
			return (wc);
	}
	while (++re < rr->__ranges + rr->__nranges) {
		wc = re->__min;
		if (re->__types != NULL) {
			for (; wc <= re->__max; wc++)
				if (re->__types[wc - re->__min] & wct)
					return (wc);
		} else if (re->__map & wct)
			return (wc);
	}
	return (-1);
}
wint_t
ref_nextwctype(wint_t wc, wctype_t wct)
{
	return ref_nextwctype_l(wc, wct, __get_locale());
}

/* ------------------------------------------------------------------ */
/* hbsd/src/lib/libc/locale/mbsnrtowcs.c					*/
/* ------------------------------------------------------------------ */

size_t
ref_mbsnrtowcs_l(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps, ref_locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == NULL)
		ps = &(XLOCALE_CTYPE(locale)->mbsnrtowcs);
	return (XLOCALE_CTYPE(locale)->__mbsnrtowcs(dst, src, nms, len, ps));
}
size_t
ref_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps)
{
	return ref_mbsnrtowcs_l(dst, src, nms, len, ps, __get_locale());
}

size_t
ref___mbsnrtowcs_std(wchar_t * __restrict dst, const char ** __restrict src,
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
ref_backend_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps)
{

	return (ref___mbsnrtowcs_std(dst, src, nms, len, ps, test_mbrtowc));
}
