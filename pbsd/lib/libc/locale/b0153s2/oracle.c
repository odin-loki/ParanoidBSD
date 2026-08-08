/*
 * PBSD batch b0153s2 -- reference oracle.
 *
 * Original HardenedBSD source concatenated, every function renamed with
 * a ref_ prefix.  Function bodies are UNMODIFIED except for internal static
 * call targets renamed to match.
 *
 * Source:
 *   hbsd/src/lib/libc/locale/wcsftime.c
 */

#ifndef SIZE_T_MAX
#define SIZE_T_MAX	((size_t)-1)
#endif

#ifndef LONG_BIT
#define LONG_BIT	(sizeof(long) * CHAR_BIT)
#endif

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>

#ifndef MB_CUR_MAX
#define MB_CUR_MAX	4
#endif

struct _xlocale {
	long	dummy;
};

typedef struct _xlocale *locale_t;

typedef struct {
	int			wcsrtombs_fail;
	int			fail_malloc_at;
	int			malloc_calls;
	int			strftime_zero;
	size_t			strftime_len;
	int			mbsrtowcs_fail;
	int			mbsrtowcs_incomplete;
	int			mbsrtowcs_dstp_left;
	size_t			mbsrtowcs_count;
} pbsd_wcsftime_hook_t;

pbsd_wcsftime_hook_t	pbsd_wcsftime_hook;

struct _xlocale	ref_test_locale;

void
pbsd_reset_hooks(void)
{
	memset(&pbsd_wcsftime_hook, 0, sizeof(pbsd_wcsftime_hook));
	memset(&ref_test_locale, 0, sizeof(ref_test_locale));
}

pbsd_wcsftime_hook_t *
pbsd_get_wcsftime_hook(void)
{
	return (&pbsd_wcsftime_hook);
}

static void __attribute__((constructor))
ref_oracle_init(void)
{
	pbsd_reset_hooks();
}

#define FIX_LOCALE(loc)		if ((loc) == NULL) (loc) = ref___get_locale()
#define __get_locale()		ref___get_locale()

locale_t
ref___get_locale(void)
{
	return (&ref_test_locale);
}

struct _xlocale *
ref_get_test_locale(void)
{
	return (&ref_test_locale);
}

void *__real_malloc(size_t);

void *
__wrap_malloc(size_t n)
{
	if (pbsd_wcsftime_hook.fail_malloc_at > 0) {
		pbsd_wcsftime_hook.malloc_calls++;
		if (pbsd_wcsftime_hook.malloc_calls ==
		    pbsd_wcsftime_hook.fail_malloc_at)
			return (NULL);
	}
	return (__real_malloc(n));
}

static size_t
mock_wcs_to_mbs_len(const wchar_t *ws)
{
	size_t n = 0;

	while (ws[n] != L'\0') {
		if (ws[n] > 0xff)
			return ((size_t)-1);
		n++;
	}
	return (n);
}

static size_t
mock_wcs_to_mbs(char *dst, const wchar_t *ws, size_t cap)
{
	size_t i;

	for (i = 0; ws[i] != L'\0'; i++) {
		if (ws[i] > 0xff)
			return ((size_t)-1);
		if (dst != NULL) {
			if (i + 1 >= cap)
				break;
			dst[i] = (char)ws[i];
		}
	}
	if (dst != NULL && cap > 0)
		dst[i] = '\0';
	return (i);
}

size_t
wcsrtombs_l(char * __restrict dst, const wchar_t ** __restrict src,
    size_t len, mbstate_t * __restrict ps, locale_t locale)
{
	const wchar_t *s;
	size_t n;

	(void)ps;
	(void)locale;
	if (pbsd_wcsftime_hook.wcsrtombs_fail != 0)
		return ((size_t)-1);
	if (src == NULL)
		return ((size_t)-1);
	s = *src;
	if (s == NULL)
		return ((size_t)-1);
	n = mock_wcs_to_mbs_len(s);
	if (n == (size_t)-1)
		return ((size_t)-1);
	if (dst != NULL) {
		if (n + 1 > len)
			return ((size_t)-1);
		mock_wcs_to_mbs(dst, s, len);
		*src = s + n;
	}
	return (n);
}

size_t
strftime_l(char * __restrict s, size_t max, const char * __restrict fmt,
    const struct tm * __restrict tm, locale_t locale)
{
	size_t n;

	(void)fmt;
	(void)tm;
	(void)locale;
	if (pbsd_wcsftime_hook.strftime_zero != 0)
		return (0);
	n = pbsd_wcsftime_hook.strftime_len;
	if (n == 0)
		n = 4;
	if (s != NULL && max > 0) {
		if (n >= max)
			n = max - 1;
		memset(s, 'X', n);
		s[n] = '\0';
	}
	return (n);
}

size_t
mbsrtowcs_l(wchar_t * __restrict dst, const char ** __restrict src,
    size_t len, mbstate_t * __restrict ps, locale_t locale)
{
	const char *s;
	size_t i, n;

	(void)ps;
	(void)locale;
	if (pbsd_wcsftime_hook.mbsrtowcs_fail != 0)
		return ((size_t)-1);
	if (pbsd_wcsftime_hook.mbsrtowcs_incomplete != 0)
		return ((size_t)-2);
	if (src == NULL)
		return ((size_t)-1);
	s = *src;
	if (s == NULL)
		return ((size_t)-1);
	n = pbsd_wcsftime_hook.mbsrtowcs_count;
	if (n == 0) {
		while (s[n] != '\0')
			n++;
	}
	if (dst != NULL) {
		for (i = 0; i < n && i < len; i++)
			dst[i] = (unsigned char)s[i];
		if (pbsd_wcsftime_hook.mbsrtowcs_dstp_left != 0)
			*src = s + 1;
		else
			*src = NULL;
	}
	return (n);
}

/* wcsftime.c */
size_t
ref_wcsftime_l(wchar_t * __restrict wcs, size_t maxsize,
	const wchar_t * __restrict format, const struct tm * __restrict timeptr,
	locale_t locale)
{
	static const mbstate_t initial;
	mbstate_t mbs;
	char *dst, *sformat;
	const char *dstp;
	const wchar_t *formatp;
	size_t n, sflen;
	int sverrno;
	FIX_LOCALE(locale);

	sformat = dst = NULL;

	/*
	 * Convert the supplied format string to a multibyte representation
	 * for strftime(), which only handles single-byte characters.
	 */
	mbs = initial;
	formatp = format;
	sflen = wcsrtombs_l(NULL, &formatp, 0, &mbs, locale);
	if (sflen == (size_t)-1)
		goto error;
	if ((sformat = malloc(sflen + 1)) == NULL)
		goto error;
	mbs = initial;
	wcsrtombs_l(sformat, &formatp, sflen + 1, &mbs, locale);

	/*
	 * Allocate memory for longest multibyte sequence that will fit
	 * into the caller's buffer and call strftime() to fill it.
	 * Then, copy and convert the result back into wide characters in
	 * the caller's buffer.
	 */
	if (SIZE_T_MAX / MB_CUR_MAX <= maxsize) {
		/* maxsize is prepostorously large - avoid int. overflow. */
		errno = EINVAL;
		goto error;
	}
	if ((dst = malloc(maxsize * MB_CUR_MAX)) == NULL)
		goto error;
	if (strftime_l(dst, maxsize, sformat, timeptr, locale) == 0)
		goto error;
	dstp = dst;
	mbs = initial;
	n = mbsrtowcs_l(wcs, &dstp, maxsize, &mbs, locale);
	if (n == (size_t)-2 || n == (size_t)-1 || dstp != NULL)
		goto error;

	free(sformat);
	free(dst);
	return (n);

error:
	sverrno = errno;
	free(sformat);
	free(dst);
	errno = sverrno;
	return (0);
}
size_t
ref_wcsftime(wchar_t * __restrict wcs, size_t maxsize,
	const wchar_t * __restrict format, const struct tm * __restrict timeptr)
{
	return ref_wcsftime_l(wcs, maxsize, format, timeptr, __get_locale());
}
