/*-
 * PBSD batch b0153 -- C++23 module port of
 *
 *	hbsd/src/lib/libc/locale/localeconv.c
 *	hbsd/src/lib/libc/locale/wcsftime.c
 *	hbsd/src/lib/libc/locale/ldpart.c
 *	hbsd/src/lib/libc/locale/gb2312.c
 */

module;

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cerrno>
#include <cstdlib>
#include <cstring>

export module pbsd.lib.libc.locale.b0153;

extern "C" {
#define __mbstate_t_defined 1
typedef union {
	char		__mbstate8[128];
	long long	_mbstateL;
} __mbstate_t;
typedef __mbstate_t mbstate_t;

#include <fcntl.h>
#include <limits.h>
#include <locale.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>

#ifndef PATH_MAX
#define PATH_MAX	4096
#endif

#ifndef O_CLOEXEC
#define O_CLOEXEC	0
#endif

#ifndef EFTYPE
#define EFTYPE		79
#endif

#ifndef u_char
typedef unsigned char u_char;
#endif

#ifndef MB_LEN_MAX
#define MB_LEN_MAX	4
#endif

#ifndef MB_CUR_MAX
#define MB_CUR_MAX	4
#endif

#ifndef MIN
#define MIN(a, b)	((a) < (b) ? (a) : (b))
#endif

#define _LDP_LOADED	0
#define _LDP_ERROR	(-1)
#define _LDP_CACHE	1

#define FAKE_FD		100

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

struct lc_monetary_T {
	const char	*int_curr_symbol;
	const char	*currency_symbol;
	const char	*mon_decimal_point;
	const char	*mon_thousands_sep;
	const char	*mon_grouping;
	const char	*positive_sign;
	const char	*negative_sign;
	const char	*int_frac_digits;
	const char	*frac_digits;
	const char	*p_cs_precedes;
	const char	*p_sep_by_space;
	const char	*n_cs_precedes;
	const char	*n_sep_by_space;
	const char	*p_sign_posn;
	const char	*n_sign_posn;
	const char	*int_p_cs_precedes;
	const char	*int_n_cs_precedes;
	const char	*int_p_sep_by_space;
	const char	*int_n_sep_by_space;
	const char	*int_p_sign_posn;
	const char	*int_n_sign_posn;
};

struct lc_numeric_T {
	const char	*decimal_point;
	const char	*thousands_sep;
	const char	*grouping;
};

struct port_xlocale_component {
	long		retain_count;
	void		(*destructor)(void *);
	char		locale[32];
	char		version[12];
};

struct port_xlocale {
	long		retain_count;
	void		(*destructor)(void *);
	struct port_xlocale_component *components[6];
	int		monetary_locale_changed;
	int		using_monetary_locale;
	int		numeric_locale_changed;
	int		using_numeric_locale;
	int		using_time_locale;
	int		using_messages_locale;
	struct lconv	lconv;
	char		*csym;
};

typedef struct port_xlocale *port_locale_t;

struct port_xlocale_ctype {
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

typedef struct {
	struct lc_monetary_T	monetary;
	struct lc_numeric_T	numeric;
} pbsd_localeconv_data_t;

typedef struct {
	int			monetary_changed;
	int			numeric_changed;
	pbsd_localeconv_data_t	data;
} pbsd_localeconv_hook_t;

typedef struct {
	int			open_fail;
	int			open_errno;
	int			fstat_fail;
	int			fstat_errno;
	off_t			file_size;
	const char		*file_content;
	int			read_fail;
	int			read_errno;
	int			malloc_fail;
	int			close_count;
} pbsd_ldpart_hook_t;

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

extern pbsd_localeconv_hook_t	pbsd_localeconv_hook;
extern pbsd_ldpart_hook_t	pbsd_ldpart_hook;
extern pbsd_wcsftime_hook_t	pbsd_wcsftime_hook;
extern struct port_xlocale	port_test_locale;
extern char			*_PathLocale;

void	pbsd_reset_hooks(void);

static inline int
atomic_load_acq_int(volatile int *p)
{
	return (*p);
}

static inline void
atomic_store_rel_int(volatile int *p, int v)
{
	*p = v;
}

#define FIX_LOCALE(loc)		if ((loc) == NULL) (loc) = port_get_locale()
#define __get_locale()		port_get_locale()

static inline port_locale_t
port_get_locale()
{
	return (&port_test_locale);
}

static struct lc_monetary_T *
port_get_current_monetary_locale(port_locale_t loc)
{
	(void)loc;
	return ((struct lc_monetary_T *)&pbsd_localeconv_hook.data.monetary);
}

static struct lc_numeric_T *
port_get_current_numeric_locale(port_locale_t loc)
{
	(void)loc;
	return ((struct lc_numeric_T *)&pbsd_localeconv_hook.data.numeric);
}

int
port_open(const char *path, int flags, ...)
{
	(void)path;
	(void)flags;
	if (pbsd_ldpart_hook.open_fail != 0) {
		errno = pbsd_ldpart_hook.open_errno;
		return (-1);
	}
	return (FAKE_FD);
}

int
port_fstat(int fd, struct stat *st)
{
	(void)fd;
	if (pbsd_ldpart_hook.fstat_fail != 0) {
		errno = pbsd_ldpart_hook.fstat_errno;
		return (-1);
	}
	std::memset(st, 0, sizeof(*st));
	st->st_size = pbsd_ldpart_hook.file_size;
	return (0);
}

ssize_t
port_read(int fd, void *buf, size_t nbytes)
{
	size_t n;

	(void)fd;
	if (pbsd_ldpart_hook.read_fail != 0) {
		errno = pbsd_ldpart_hook.read_errno;
		return (-1);
	}
	n = (size_t)pbsd_ldpart_hook.file_size;
	if (nbytes < n)
		n = nbytes;
	if (pbsd_ldpart_hook.file_content != NULL && n > 0)
		std::memcpy(buf, pbsd_ldpart_hook.file_content, n);
	return ((ssize_t)n);
}

int
port_close(int fd)
{
	(void)fd;
	pbsd_ldpart_hook.close_count++;
	return (0);
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

static size_t
port_wcsrtombs_l(char * __restrict dst, const wchar_t ** __restrict src,
    size_t len, mbstate_t * __restrict ps, port_locale_t locale)
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

static size_t
port_pbsd_strftime_l(char * __restrict s, size_t max, const char * __restrict fmt,
    const void * __restrict tm, port_locale_t locale)
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
		std::memset(s, 'X', n);
		s[n] = '\0';
	}
	return (n);
}

static size_t
port_mbsrtowcs_l(wchar_t * __restrict dst, const char ** __restrict src,
    size_t len, mbstate_t * __restrict ps, port_locale_t locale)
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
}

export namespace pbsd::lib_libc_locale::b0153 {

using mbstate_t = ::mbstate_t;
using locale_t = ::port_locale_t;

inline locale_t
test_locale()
{
	return (&::port_test_locale);
}

inline void
set_localeconv_flags(int mon, int num)
{
	::port_test_locale.monetary_locale_changed = mon;
	::port_test_locale.numeric_locale_changed = num;
}

struct tm {
	int	tm_sec;
	int	tm_min;
	int	tm_hour;
	int	tm_mday;
	int	tm_mon;
	int	tm_year;
	int	tm_wday;
	int	tm_yday;
	int	tm_isdst;
};

struct lc_monetary_T {
	const char	*int_curr_symbol;
	const char	*currency_symbol;
	const char	*mon_decimal_point;
	const char	*mon_thousands_sep;
	const char	*mon_grouping;
	const char	*positive_sign;
	const char	*negative_sign;
	const char	*int_frac_digits;
	const char	*frac_digits;
	const char	*p_cs_precedes;
	const char	*p_sep_by_space;
	const char	*n_cs_precedes;
	const char	*n_sep_by_space;
	const char	*p_sign_posn;
	const char	*n_sign_posn;
	const char	*int_p_cs_precedes;
	const char	*int_n_cs_precedes;
	const char	*int_p_sep_by_space;
	const char	*int_n_sep_by_space;
	const char	*int_p_sign_posn;
	const char	*int_n_sign_posn;
};

struct lc_numeric_T {
	const char	*decimal_point;
	const char	*thousands_sep;
	const char	*grouping;
};

struct _RuneLocale {
	char		__magic[8];
	char		__encoding[32];
	int		__invalid_rune;
	unsigned long	__runetype[256];
	int		__maplower[256];
	int		__mapupper[256];
	void		*__variable;
	int		__variable_len;
};

struct xlocale_ctype {
	std::size_t	(*__mbrtowc)(wchar_t * __restrict, const char * __restrict,
		    std::size_t, mbstate_t * __restrict);
	int		(*__mbsinit)(const mbstate_t *);
	std::size_t	(*__mbsnrtowcs)(wchar_t * __restrict, const char ** __restrict,
		    std::size_t, std::size_t, mbstate_t * __restrict);
	std::size_t	(*__wcrtomb)(char * __restrict, wchar_t, mbstate_t * __restrict);
	std::size_t	(*__wcsnrtombs)(char * __restrict, const wchar_t ** __restrict,
		    std::size_t, std::size_t, mbstate_t * __restrict);
	_RuneLocale	*runes;
	int		__mb_cur_max;
	int		__mb_sb_limit;
};

struct xlocale {
	long		retain_count;
	void		(*destructor)(void *);
	void		*components[6];
	int		monetary_locale_changed;
	int		using_monetary_locale;
	int		numeric_locale_changed;
	int		using_numeric_locale;
	int		using_time_locale;
	int		using_messages_locale;
	struct ::lconv	lconv;
	char		*csym;
};

using mbrtowc_pfn_t = ::mbrtowc_pfn_t;
using wcrtomb_pfn_t = ::wcrtomb_pfn_t;

std::size_t
__mbsnrtowcs_std(wchar_t * __restrict dst, const char ** __restrict src,
    std::size_t nms, std::size_t len, mbstate_t * __restrict ps,
    mbrtowc_pfn_t pmbrtowc)
{
	const char *s;
	std::size_t nchr;
	wchar_t wc;
	std::size_t nb;

	s = *src;
	nchr = 0;

	if (dst == NULL) {
		for (;;) {
			if ((nb = pmbrtowc(&wc, s, nms, ps)) == (std::size_t)-1)
				return ((std::size_t)-1);
			else if (nb == 0 || nb == (std::size_t)-2)
				return (nchr);
			s += nb;
			nms -= nb;
			nchr++;
		}
	}

	while (len-- > 0) {
		if ((nb = pmbrtowc(dst, s, nms, ps)) == (std::size_t)-1) {
			*src = s;
			return ((std::size_t)-1);
		} else if (nb == (std::size_t)-2) {
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

std::size_t
__wcsnrtombs_std(char * __restrict dst, const wchar_t ** __restrict src,
    std::size_t nwc, std::size_t len, mbstate_t * __restrict ps,
    wcrtomb_pfn_t pwcrtomb)
{
	mbstate_t mbsbak;
	char buf[MB_LEN_MAX];
	const wchar_t *s;
	std::size_t nbytes;
	std::size_t nb;

	s = *src;
	nbytes = 0;

	if (dst == NULL) {
		while (nwc-- > 0) {
			if ((nb = pwcrtomb(buf, *s, ps)) == (std::size_t)-1)
				return ((std::size_t)-1);
			else if (*s == L'\0')
				return (nbytes + nb - 1);
			s++;
			nbytes += nb;
		}
		return (nbytes);
	}

	while (len > 0 && nwc-- > 0) {
		if (len > (std::size_t)MB_CUR_MAX) {
			if ((nb = pwcrtomb(dst, *s, ps)) == (std::size_t)-1) {
				*src = s;
				return ((std::size_t)-1);
			}
		} else {
			mbsbak = *ps;
			if ((nb = pwcrtomb(buf, *s, ps)) == (std::size_t)-1) {
				*src = s;
				return ((std::size_t)-1);
			}
			if (nb > (int)len) {
				*ps = mbsbak;
				break;
			}
			std::memcpy(dst, buf, nb);
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

struct lconv *
localeconv_l(locale_t loc)
{
	FIX_LOCALE(loc);
	struct lconv *ret = &loc->lconv;

	if (atomic_load_acq_int(&loc->monetary_locale_changed) != 0) {
		struct ::lc_monetary_T *mptr;

#define M_ASSIGN_STR(NAME) (ret->NAME = (char*)mptr->NAME)
#define M_ASSIGN_CHAR(NAME) (ret->NAME = mptr->NAME[0])

		mptr = port_get_current_monetary_locale(loc);
		M_ASSIGN_STR(int_curr_symbol);
		M_ASSIGN_STR(currency_symbol);
		M_ASSIGN_STR(mon_decimal_point);
		M_ASSIGN_STR(mon_thousands_sep);
		M_ASSIGN_STR(mon_grouping);
		M_ASSIGN_STR(positive_sign);
		M_ASSIGN_STR(negative_sign);
		M_ASSIGN_CHAR(int_frac_digits);
		M_ASSIGN_CHAR(frac_digits);
		M_ASSIGN_CHAR(p_cs_precedes);
		M_ASSIGN_CHAR(p_sep_by_space);
		M_ASSIGN_CHAR(n_cs_precedes);
		M_ASSIGN_CHAR(n_sep_by_space);
		M_ASSIGN_CHAR(p_sign_posn);
		M_ASSIGN_CHAR(n_sign_posn);
		M_ASSIGN_CHAR(int_p_cs_precedes);
		M_ASSIGN_CHAR(int_n_cs_precedes);
		M_ASSIGN_CHAR(int_p_sep_by_space);
		M_ASSIGN_CHAR(int_n_sep_by_space);
		M_ASSIGN_CHAR(int_p_sign_posn);
		M_ASSIGN_CHAR(int_n_sign_posn);
		atomic_store_rel_int(&loc->monetary_locale_changed, 0);
	}

	if (atomic_load_acq_int(&loc->numeric_locale_changed) != 0) {
		struct ::lc_numeric_T *nptr;

#define N_ASSIGN_STR(NAME) (ret->NAME = (char*)nptr->NAME)

		nptr = port_get_current_numeric_locale(loc);
		N_ASSIGN_STR(decimal_point);
		N_ASSIGN_STR(thousands_sep);
		N_ASSIGN_STR(grouping);
		atomic_store_rel_int(&loc->numeric_locale_changed, 0);
	}

	return (ret);
}

struct lconv *
localeconv()
{
	return (localeconv_l(__get_locale()));
}

std::size_t
wcsftime_l(wchar_t * __restrict wcs, std::size_t maxsize,
	const wchar_t * __restrict format, const tm * __restrict timeptr,
	locale_t locale)
{
	static const mbstate_t initial{};
	mbstate_t mbs;
	char *dst, *sformat;
	const char *dstp;
	const wchar_t *formatp;
	std::size_t n, sflen;
	int sverrno;
	FIX_LOCALE(locale);

	sformat = dst = NULL;

	mbs = initial;
	formatp = format;
	sflen = port_wcsrtombs_l(NULL, &formatp, 0, &mbs, locale);
	if (sflen == (std::size_t)-1)
		goto error;
	if ((sformat = (char *)malloc(sflen + 1)) == NULL)
		goto error;
	mbs = initial;
	port_wcsrtombs_l(sformat, &formatp, sflen + 1, &mbs, locale);

	if (SIZE_MAX / MB_CUR_MAX <= maxsize) {
		errno = EINVAL;
		goto error;
	}
	if ((dst = (char *)malloc(maxsize * MB_CUR_MAX)) == NULL)
		goto error;
	if (port_pbsd_strftime_l(dst, maxsize, sformat, timeptr, locale) == 0)
		goto error;
	dstp = dst;
	mbs = initial;
	n = port_mbsrtowcs_l(wcs, &dstp, maxsize, &mbs, locale);
	if (n == (std::size_t)-2 || n == (std::size_t)-1 || dstp != NULL)
		goto error;

	std::free(sformat);
	std::free(dst);
	return (n);

error:
	sverrno = errno;
	free(sformat);
	free(dst);
	errno = sverrno;
	return (0);
}

std::size_t
wcsftime(wchar_t * __restrict wcs, std::size_t maxsize,
	const wchar_t * __restrict format, const tm * __restrict timeptr)
{
	return (wcsftime_l(wcs, maxsize, format, timeptr, __get_locale()));
}

static int split_lines(char *, const char *);

int
__part_load_locale(const char *name,
		int *using_locale,
		char **locale_buf,
		const char *category_filename,
		int locale_buf_size_max,
		int locale_buf_size_min,
		const char **dst_localebuf)
{
	int		saverr, fd, i, num_lines;
	char		*lbuf, *p;
	const char	*plim;
	char		filename[PATH_MAX];
	struct stat	st;
	std::size_t	namesize, bufsize;

	if (std::strcmp(name, "C") == 0 || std::strcmp(name, "POSIX") == 0 ||
	    std::strncmp(name, "C.", 2) == 0) {
		*using_locale = 0;
		return (_LDP_CACHE);
	}

	if (*locale_buf != NULL && std::strcmp(name, *locale_buf) == 0) {
		*using_locale = 1;
		return (_LDP_CACHE);
	}

	namesize = std::strlen(name) + 1;

	std::strcpy(filename, _PathLocale);
	std::strcat(filename, "/");
	std::strcat(filename, name);
	std::strcat(filename, "/");
	std::strcat(filename, category_filename);
	if ((fd = port_open(filename, O_RDONLY | O_CLOEXEC)) < 0)
		return (_LDP_ERROR);
	if (port_fstat(fd, &st) != 0)
		goto bad_locale;
	if (st.st_size <= 0) {
		errno = EFTYPE;
		goto bad_locale;
	}
	bufsize = namesize + st.st_size;
	if ((lbuf = (char *)malloc(bufsize)) == NULL) {
		errno = ENOMEM;
		goto bad_locale;
	}
	(void)std::strcpy(lbuf, name);
	p = lbuf + namesize;
	plim = p + st.st_size;
	if (port_read(fd, p, (size_t) st.st_size) != st.st_size)
		goto bad_lbuf;
	if (plim[-1] != '\n') {
		errno = EFTYPE;
		goto bad_lbuf;
	}
	num_lines = split_lines(p, plim);
	if (num_lines >= locale_buf_size_max)
		num_lines = locale_buf_size_max;
	else if (num_lines >= locale_buf_size_min)
		num_lines = locale_buf_size_min;
	else {
		errno = EFTYPE;
		goto bad_lbuf;
	}
	(void)port_close(fd);
	if (*locale_buf != NULL)
		free(*locale_buf);
	*locale_buf = lbuf;
	for (p = *locale_buf, i = 0; i < num_lines; i++)
		dst_localebuf[i] = (p += std::strlen(p) + 1);
	for (i = num_lines; i < locale_buf_size_max; i++)
		dst_localebuf[i] = NULL;
	*using_locale = 1;

	return (_LDP_LOADED);

bad_lbuf:
	saverr = errno;
	free(lbuf);
	errno = saverr;
bad_locale:
	saverr = errno;
	(void)port_close(fd);
	errno = saverr;

	return (_LDP_ERROR);
}

static int
split_lines(char *p, const char *plim)
{
	int i;

	i = 0;
	while (p < plim) {
		if (*p == '\n') {
			*p = '\0';
			i++;
		}
		p++;
	}
	return (i);
}

typedef struct {
	int	count;
	u_char	bytes[2];
} _GB2312State;

static std::size_t	_GB2312_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    std::size_t, mbstate_t * __restrict);
static int	_GB2312_mbsinit(const mbstate_t *);
static std::size_t	_GB2312_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict);
static std::size_t	_GB2312_mbsnrtowcs(wchar_t * __restrict,
		    const char ** __restrict, std::size_t, std::size_t,
		    mbstate_t * __restrict);
static std::size_t	_GB2312_wcsnrtombs(char * __restrict,
		    const wchar_t ** __restrict, std::size_t, std::size_t,
		    mbstate_t * __restrict);

static int
_GB2312_check(const char *str, std::size_t n)
{
	const u_char *s = (const u_char *)str;

	if (n == 0)
		return (-2);
	if (s[0] >= 0xa1 && s[0] <= 0xfe) {
		if (n < 2)
			return (-2);
		if (s[1] < 0xa1 || s[1] > 0xfe)
			return (-1);
		return (2);
	} else if (s[0] & 0x80) {
		return (-1);
	}
	return (1);
}

int
GB2312_check(const char *str, std::size_t n)
{
	return (_GB2312_check(str, n));
}

int
GB2312_init(xlocale_ctype *l, _RuneLocale *rl)
{

	l->runes = rl;
	l->__mbrtowc = _GB2312_mbrtowc;
	l->__wcrtomb = _GB2312_wcrtomb;
	l->__mbsinit = _GB2312_mbsinit;
	l->__mbsnrtowcs = _GB2312_mbsnrtowcs;
	l->__wcsnrtombs = _GB2312_wcsnrtombs;
	l->__mb_cur_max = 2;
	l->__mb_sb_limit = 128;
	return (0);
}

static int
_GB2312_mbsinit(const mbstate_t *ps)
{

	return (ps == NULL || ((const _GB2312State *)ps)->count == 0);
}

int
GB2312_mbsinit(const mbstate_t *ps)
{
	return (_GB2312_mbsinit(ps));
}

static std::size_t
_GB2312_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, std::size_t n,
    mbstate_t * __restrict ps)
{
	_GB2312State *gs;
	wchar_t wc;
	int i, len, ocount;
	std::size_t ncopy;

	gs = (_GB2312State *)ps;

	if (gs->count < 0 || gs->count > (int)sizeof(gs->bytes)) {
		errno = EINVAL;
		return ((std::size_t)-1);
	}

	if (s == NULL) {
		s = "";
		n = 1;
		pwc = NULL;
	}

	ncopy = MIN(MIN(n, (std::size_t)MB_CUR_MAX),
	    sizeof(gs->bytes) - gs->count);
	std::memcpy(gs->bytes + gs->count, s, ncopy);
	ocount = gs->count;
	gs->count += (int)ncopy;
	s = (char *)gs->bytes;
	n = gs->count;

	if ((len = _GB2312_check(s, n)) < 0)
		return ((std::size_t)len);
	wc = 0;
	i = len;
	while (i-- > 0)
		wc = (wc << 8) | (unsigned char)*s++;
	if (pwc != NULL)
		*pwc = wc;
	gs->count = 0;
	return (wc == L'\0' ? 0 : len - ocount);
}

std::size_t
GB2312_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, std::size_t n,
    mbstate_t * __restrict ps)
{
	return (_GB2312_mbrtowc(pwc, s, n, ps));
}

static std::size_t
_GB2312_wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps)
{
	_GB2312State *gs;

	gs = (_GB2312State *)ps;

	if (gs->count != 0) {
		errno = EINVAL;
		return ((std::size_t)-1);
	}

	if (s == NULL)
		return (1);
	if (wc & 0x8000) {
		*s++ = (wc >> 8) & 0xff;
		*s = wc & 0xff;
		return (2);
	}
	*s = wc & 0xff;
	return (1);
}

std::size_t
GB2312_wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps)
{
	return (_GB2312_wcrtomb(s, wc, ps));
}

static std::size_t
_GB2312_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    std::size_t nms, std::size_t len, mbstate_t * __restrict ps)
{
	return (__mbsnrtowcs_std(dst, src, nms, len, ps, _GB2312_mbrtowc));
}

std::size_t
GB2312_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    std::size_t nms, std::size_t len, mbstate_t * __restrict ps)
{
	return (_GB2312_mbsnrtowcs(dst, src, nms, len, ps));
}

static std::size_t
_GB2312_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
    std::size_t nwc, std::size_t len, mbstate_t * __restrict ps)
{
	return (__wcsnrtombs_std(dst, src, nwc, len, ps, _GB2312_wcrtomb));
}

std::size_t
GB2312_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
    std::size_t nwc, std::size_t len, mbstate_t * __restrict ps)
{
	return (_GB2312_wcsnrtombs(dst, src, nwc, len, ps));
}

}
