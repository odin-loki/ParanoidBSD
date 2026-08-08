/*
 * PBSD batch b0130 -- reference oracle.
 *
 * Original HardenedBSD sources concatenated, with every function renamed with
 * a ref_ prefix.  Function bodies are UNMODIFIED.
 *
 * Sources:
 *   hbsd/src/lib/libc/locale/wcsnrtombs.c
 *   hbsd/src/lib/libc/locale/wcstod.c
 *   hbsd/src/lib/libc/locale/lmessages.c
 *
 * Private libc headers and runtime symbols are not part of this batch.
 * Declarations and deterministic mock backends below stand in so the
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

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#undef MB_LEN_MAX
#undef MB_CUR_MAX
#define MB_LEN_MAX	4
#define MB_CUR_MAX	4

typedef size_t (*wcrtomb_pfn_t)(char * __restrict, wchar_t, mbstate_t * __restrict);

enum {
	XLC_CTYPE = 1,
	XLC_MESSAGES = 5,
};

#define _LDP_LOADED	1
#define _LDP_ERROR	(-1)

struct lc_messages_T {
	const char	*yesexpr;
	const char	*noexpr;
	const char	*yesstr;
	const char	*nostr;
};

struct xlocale_component_header {
	void		(*destructor)(void *);
};

struct xlocale_component {
	struct xlocale_component_header header;
};

struct xlocale_messages {
	struct xlocale_component header;
	char		*buffer;
	struct lc_messages_T locale;
};

struct xlocale_ctype {
	size_t		(*__wcsnrtombs)(char * __restrict,
		    const wchar_t ** __restrict, size_t, size_t,
		    mbstate_t * __restrict);
	mbstate_t	wcsnrtombs;
};

struct xlocale {
	int		using_messages_locale;
	void		*components[8];
};

typedef struct xlocale *locale_t;

#define FIX_LOCALE(loc)		if ((loc) == NULL) (loc) = __get_locale()
#define XLOCALE_CTYPE(l)	((struct xlocale_ctype *)(l)->components[XLC_CTYPE])
#define __get_locale()		ref___get_locale()

struct xlocale_ctype	ref_global_ctype;
struct xlocale		__xlocale_global_locale;
struct xlocale_messages	__xlocale_global_messages;

#define LCMESSAGES_SIZE_FULL (sizeof(struct lc_messages_T) / sizeof(char *))
#define LCMESSAGES_SIZE_MIN \
		(offsetof(struct lc_messages_T, yesstr) / sizeof(char *))

/* ------------------------------------------------------------------ */
/* Harness-controllable mock backends (shared with port.cppm).         */
/* ------------------------------------------------------------------ */

typedef struct {
	unsigned int	call_count;
	unsigned int	fail_at;
	unsigned int	touch_state;
	size_t		forced_nb;
	wchar_t		forced_wc;
} pbsd_wcrtomb_hook_t;

typedef struct {
	int		ret;
	int		null_yesstr;
	int		null_nostr;
	const char	*yesexpr;
	const char	*noexpr;
	const char	*yesstr;
	const char	*nostr;
	unsigned int	call_count;
} pbsd_part_load_hook_t;

pbsd_wcrtomb_hook_t	pbsd_wcrtomb_hook;
pbsd_part_load_hook_t	pbsd_part_load_hook;

void
pbsd_reset_hooks(void)
{
	memset(&pbsd_wcrtomb_hook, 0, sizeof(pbsd_wcrtomb_hook));
	memset(&pbsd_part_load_hook, 0, sizeof(pbsd_part_load_hook));
	pbsd_part_load_hook.ret = _LDP_LOADED;
	pbsd_part_load_hook.yesexpr = "^[yY]";
	pbsd_part_load_hook.noexpr = "^[nN]";
	pbsd_part_load_hook.yesstr = "yes";
	pbsd_part_load_hook.nostr = "no";
}

locale_t
ref___get_locale(void)
{
	return ((locale_t)&__xlocale_global_locale);
}

static void
xlocale_release(void *v)
{
	struct xlocale_component *c = v;

	if (c != NULL && c->header.destructor != NULL)
		c->header.destructor(v);
	free(v);
}

static size_t
pbsd_utf8_encode(wchar_t wc, unsigned char *dst, size_t dstl)
{
	uint32_t cp;

	cp = (uint32_t)wc;
	if (cp < 0x80) {
		if (dstl < 1)
			return ((size_t)-1);
		dst[0] = (unsigned char)cp;
		return (1);
	}
	if (cp < 0x800) {
		if (dstl < 2)
			return ((size_t)-1);
		dst[0] = (unsigned char)(0xC0 | (cp >> 6));
		dst[1] = (unsigned char)(0x80 | (cp & 0x3F));
		return (2);
	}
	if (cp < 0x10000) {
		if (dstl < 3)
			return ((size_t)-1);
		dst[0] = (unsigned char)(0xE0 | (cp >> 12));
		dst[1] = (unsigned char)(0x80 | ((cp >> 6) & 0x3F));
		dst[2] = (unsigned char)(0x80 | (cp & 0x3F));
		return (3);
	}
	if (cp <= 0x10FFFF) {
		if (dstl < 4)
			return ((size_t)-1);
		dst[0] = (unsigned char)(0xF0 | (cp >> 18));
		dst[1] = (unsigned char)(0x80 | ((cp >> 12) & 0x3F));
		dst[2] = (unsigned char)(0x80 | ((cp >> 6) & 0x3F));
		dst[3] = (unsigned char)(0x80 | (cp & 0x3F));
		return (4);
	}
	return ((size_t)-1);
}

size_t
pbsd_wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps)
{
	unsigned char buf[MB_LEN_MAX];
	size_t nb;

	(void)ps;
	pbsd_wcrtomb_hook.call_count++;
	if (pbsd_wcrtomb_hook.fail_at != 0 &&
	    pbsd_wcrtomb_hook.call_count == pbsd_wcrtomb_hook.fail_at)
		return ((size_t)-1);
	if (pbsd_wcrtomb_hook.forced_wc == wc && pbsd_wcrtomb_hook.forced_nb != 0)
		nb = pbsd_wcrtomb_hook.forced_nb;
	else
		nb = pbsd_utf8_encode(wc, buf, sizeof(buf));
	if (nb == (size_t)-1)
		return ((size_t)-1);
	if (pbsd_wcrtomb_hook.touch_state != 0 && ps != NULL)
		ps->__mbstate8[0] = (char)0x80;
	if (s == NULL)
		return (1);
	memcpy(s, buf, nb);
	return (nb);
}

void
pbsd_free(void *p)
{
	free(p);
}

void *
pbsd_malloc(size_t n)
{
	return (malloc(n));
}

void *
pbsd_calloc(size_t n, size_t sz)
{
	return (calloc(n, sz));
}

size_t
pbsd_harness_wcrtomb(char *s, wchar_t wc, void *ps)
{
	return (pbsd_wcrtomb(s, wc, (mbstate_t *)ps));
}

size_t
wcsrtombs_l(char * __restrict dst, const wchar_t ** __restrict src,
    size_t len, mbstate_t * __restrict ps, locale_t locale)
{
	const wchar_t *s;
	unsigned char buf[MB_LEN_MAX];
	size_t nb, total;

	(void)locale;
	(void)ps;
	if (src == NULL)
		return ((size_t)-1);
	s = *src;
	if (s == NULL)
		return ((size_t)-1);
	total = 0;
	while (*s != L'\0') {
		nb = pbsd_utf8_encode(*s, buf, sizeof(buf));
		if (nb == (size_t)-1)
			return ((size_t)-1);
		if (dst != NULL) {
			if (total + nb + 1 > len)
				break;
			memcpy(dst + total, buf, nb);
		}
		total += nb;
		s++;
	}
	if (dst != NULL && total < len)
		dst[total] = '\0';
	total++;
	return (total);
}

int
iswspace_l(wint_t wc, locale_t locale)
{
	(void)locale;
	return (iswspace((wint_t)wc));
}

double
strtod_l(const char * __restrict nptr, char ** __restrict endptr,
    locale_t locale)
{
	(void)locale;
	return (strtod(nptr, endptr));
}

size_t
pbsd_locale_wcsrtombs(char * __restrict dst, const wchar_t ** __restrict src,
    size_t len, mbstate_t * __restrict ps, locale_t locale)
{
	return (wcsrtombs_l(dst, src, len, ps, locale));
}

int
pbsd_locale_iswspace(wint_t wc, locale_t locale)
{
	return (iswspace_l(wc, locale));
}

double
pbsd_locale_strtod(const char * __restrict nptr, char ** __restrict endptr,
    locale_t locale)
{
	return (strtod_l(nptr, endptr, locale));
}

int
__part_load_locale(const char *name, int *using_locale, char **buffer,
    const char *category, int size_full, int size_min, const char **dst)
{
	const char **fields;
	int i;

	(void)name;
	(void)category;
	(void)size_full;
	(void)size_min;
	pbsd_part_load_hook.call_count++;
	fields = (const char **)dst;
	if (pbsd_part_load_hook.ret != _LDP_LOADED)
		return (pbsd_part_load_hook.ret);
	for (i = 0; i < LCMESSAGES_SIZE_FULL; i++)
		fields[i] = NULL;
	fields[0] = pbsd_part_load_hook.yesexpr;
	fields[1] = pbsd_part_load_hook.noexpr;
	fields[2] = pbsd_part_load_hook.null_yesstr ? NULL :
	    pbsd_part_load_hook.yesstr;
	fields[3] = pbsd_part_load_hook.null_nostr ? NULL :
	    pbsd_part_load_hook.nostr;
	if (using_locale != NULL)
		*using_locale = 1;
	if (buffer != NULL)
		*buffer = NULL;
	return (_LDP_LOADED);
}

size_t
ref___wcsnrtombs_std(char * __restrict dst, const wchar_t ** __restrict src,
    size_t nwc, size_t len, mbstate_t * __restrict ps,
    wcrtomb_pfn_t pwcrtomb);

static size_t
ref_dispatch_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
    size_t nwc, size_t len, mbstate_t * __restrict ps)
{
	return (ref___wcsnrtombs_std(dst, src, nwc, len, ps, pbsd_wcrtomb));
}

static void __attribute__((constructor))
ref_locale_init(void)
{
	memset(&ref_global_ctype, 0, sizeof(ref_global_ctype));
	ref_global_ctype.__wcsnrtombs = ref_dispatch_wcsnrtombs;
	__xlocale_global_locale.components[XLC_CTYPE] = &ref_global_ctype;
	pbsd_reset_hooks();
}

/* wcsnrtombs.c */

size_t
ref_wcsnrtombs_l(char * __restrict dst, const wchar_t ** __restrict src, size_t nwc,
    size_t len, mbstate_t * __restrict ps, locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == NULL)
		ps = &(XLOCALE_CTYPE(locale)->wcsnrtombs);
	return (XLOCALE_CTYPE(locale)->__wcsnrtombs(dst, src, nwc, len, ps));
}
size_t
ref_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src, size_t nwc,
    size_t len, mbstate_t * __restrict ps)
{
	return ref_wcsnrtombs_l(dst, src, nwc, len, ps, ref___get_locale());
}


size_t
ref___wcsnrtombs_std(char * __restrict dst, const wchar_t ** __restrict src,
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

/* wcstod.c */

double
ref_wcstod_l(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr,
		locale_t locale)
{
	static const mbstate_t initial;
	mbstate_t mbs;
	double val;
	char *buf, *end;
	const wchar_t *wcp;
	size_t len;
	size_t spaces;
	FIX_LOCALE(locale);

	wcp = nptr;
	spaces = 0;
	while (iswspace_l(*wcp, locale)) {
		wcp++;
		spaces++;
	}

	/*
	 * Convert the supplied numeric wide char. string to multibyte.
	 *
	 * We could attempt to find the end of the numeric portion of the
	 * wide char. string to avoid converting unneeded characters but
	 * choose not to bother; optimising the uncommon case where
	 * the input string contains a lot of text after the number
	 * duplicates a lot of strtod()'s functionality and slows down the
	 * most common cases.
	 */
	mbs = initial;
	if ((len = wcsrtombs_l(NULL, &wcp, 0, &mbs, locale)) == (size_t)-1) {
		if (endptr != NULL)
			*endptr = (wchar_t *)nptr;
		return (0.0);
	}
	if ((buf = malloc(len + 1)) == NULL) {
		if (endptr != NULL)
			*endptr = (wchar_t *)nptr;
		return (0.0);
	}
	mbs = initial;
	wcsrtombs_l(buf, &wcp, len + 1, &mbs, locale);

	/* Let strtod() do most of the work for us. */
	val = strtod_l(buf, &end, locale);

	/*
	 * We only know where the number ended in the _multibyte_
	 * representation of the string. If the caller wants to know
	 * where it ended, count multibyte characters to find the
	 * corresponding position in the wide char string.
	 */
	if (endptr != NULL) {
		*endptr = (wchar_t *)nptr + (end - buf);
		if (buf != end)
			*endptr += spaces;
	}

	free(buf);

	return (val);
}
double
ref_wcstod(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr)
{
	return ref_wcstod_l(nptr, endptr, ref___get_locale());
}

/* lmessages.c */

static char empty[] = "";

static const struct lc_messages_T _C_messages_locale = {
	"^[yY]" ,	/* yesexpr */
	"^[nN]" ,	/* noexpr */
	"yes" , 	/* yesstr */
	"no"		/* nostr */
};

static void
ref_destruct_messages(void *v)
{
	struct xlocale_messages *l = v;
	if (l->buffer)
		free(l->buffer);
	free(l);
}

#define destruct_messages	ref_destruct_messages

static int
ref_messages_load_locale(struct xlocale_messages *loc, int *using_locale,
    const char *name)
{
	int ret;
	struct lc_messages_T *l = &loc->locale;

	ret = __part_load_locale(name, using_locale,
		  &loc->buffer, "LC_MESSAGES",
		  LCMESSAGES_SIZE_FULL, LCMESSAGES_SIZE_MIN,
		  (const char **)l);
	if (ret == _LDP_LOADED) {
		if (l->yesstr == NULL)
			l->yesstr = empty;
		if (l->nostr == NULL)
			l->nostr = empty;
	}
	return (ret);
}

#define messages_load_locale	ref_messages_load_locale

int
ref___messages_load_locale(const char *name)
{
	return (messages_load_locale(&__xlocale_global_messages,
	    &__xlocale_global_locale.using_messages_locale, name));
}

void *
ref___messages_load(const char *name, locale_t l)
{
	struct xlocale_messages *new = calloc(sizeof(struct xlocale_messages),
	    1);
	if (new == NULL)
		return (NULL);
	new->header.header.destructor = destruct_messages;
	if (messages_load_locale(new, &l->using_messages_locale, name) ==
	    _LDP_ERROR) {
		xlocale_release(new);
		return (NULL);
	}
	return (new);
}

struct lc_messages_T *
ref___get_current_messages_locale(locale_t loc)
{
	return (loc->using_messages_locale ? &((struct xlocale_messages *)
	    loc->components[XLC_MESSAGES])->locale :
	    (struct lc_messages_T *)&_C_messages_locale);
}
