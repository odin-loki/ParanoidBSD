module;

#include <ctype.h>
#include <cerrno>
#include <climits>
#include <cinttypes>
#include <cstddef>
#include <cstdlib>
#include <cstring>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#ifndef SPECNAMELEN
#define SPECNAMELEN	63
#endif

#ifndef _PATH_DEV
#define _PATH_DEV	"/dev/"
#endif

#ifndef TIOCPTMASTER
#define TIOCPTMASTER	0x20007401UL
#endif

export module pbsd.lib.libc.stdlib.b0214;

extern "C" {
int _ioctl(int fildes, unsigned long request);
char *fdevname_r(int fildes, char *buffer, size_t buflen);
}

export namespace pbsd::lib_libc_stdlib::b0214 {

struct xlocale {
	int __unused;
};

typedef struct xlocale *locale_t;

static struct xlocale default_locale;

locale_t
__get_locale(void)
{
	return (&default_locale);
}

#define FIX_LOCALE(loc)							\
	do {								\
		if ((loc) == NULL)					\
			(loc) = __get_locale();				\
	} while (0)

static int
b0214_isspace_l(int c, locale_t locale)
{
	(void)locale;
	return (isspace((unsigned char)c));
}

#ifdef isspace_l
#undef isspace_l
#endif
#define isspace_l(c, loc)	b0214_isspace_l((c), (loc))

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 */

#define	COMPAR(x,y)	compar(x, y)

void *
bsearch(const void *key, const void *base0, size_t nmemb, size_t size,
    int (*compar)(const void *, const void *))
{
	const char *base = (const char *)base0;
	size_t lim;
	int cmp;
	const void *p;

	for (lim = nmemb; lim != 0; lim >>= 1) {
		p = base + (lim >> 1) * size;
		cmp = COMPAR(key, p);
		if (cmp == 0)
			return ((void *)p);
		if (cmp > 0) {
			base = (char *)p + size;
			lim--;
		}
	}
	return (NULL);
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2008 Ed Schouten <ed@FreeBSD.org>
 */

static int
__isptmaster(int fildes)
{

	if (_ioctl(fildes, TIOCPTMASTER) == 0)
		return (0);

	if (errno != EBADF)
		errno = EINVAL;

	return (-1);
}

int
grantpt(int fildes)
{
	return (__isptmaster(fildes));
}

int
unlockpt(int fildes)
{
	return (__isptmaster(fildes));
}

int
__ptsname_r(int fildes, char *buffer, size_t buflen)
{

	if (buflen <= sizeof(_PATH_DEV)) {
		errno = ERANGE;
		return (-1);
	}

	if (__isptmaster(fildes) != 0)
		return (-1);

	memcpy(buffer, _PATH_DEV, sizeof(_PATH_DEV));
	buffer += sizeof(_PATH_DEV) - 1;
	buflen -= sizeof(_PATH_DEV) - 1;

	if (fdevname_r(fildes, buffer, buflen) == NULL) {
		if (errno == EINVAL)
			errno = ERANGE;
		return (-1);
	}

	return (0);
}

int
ptsname_r(int fildes, char *buffer, size_t buflen)
{
	return (__ptsname_r(fildes, buffer, buflen));
}

char *
ptsname(int fildes)
{
	static char pt_slave[sizeof(_PATH_DEV) + SPECNAMELEN];

	if (__ptsname_r(fildes, pt_slave, sizeof(pt_slave)) == 0)
		return (pt_slave);

	return (NULL);
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Copyright (c) 2011 The FreeBSD Foundation
 */

uintmax_t
strtoumax_l(const char * __restrict nptr, char ** __restrict endptr, int base,
		locale_t locale)
{
	const char *s;
	uintmax_t acc;
	char c;
	uintmax_t cutoff;
	int neg, any, cutlim;
	FIX_LOCALE(locale);

	s = nptr;
	do {
		c = *s++;
	} while (isspace_l((unsigned char)c, locale));
	if (c == '-') {
		neg = 1;
		c = *s++;
	} else {
		neg = 0;
		if (c == '+')
			c = *s++;
	}
	if ((base == 0 || base == 16) &&
	    c == '0' && (*s == 'x' || *s == 'X') &&
	    ((s[1] >= '0' && s[1] <= '9') ||
	    (s[1] >= 'A' && s[1] <= 'F') ||
	    (s[1] >= 'a' && s[1] <= 'f'))) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if ((base == 0 || base == 2) &&
	    c == '0' && (*s == 'b' || *s == 'B') &&
	    (s[1] >= '0' && s[1] <= '1')) {
		c = s[1];
		s += 2;
		base = 2;
	}
	if (base == 0)
		base = c == '0' ? 8 : 10;
	acc = any = 0;
	if (base < 2 || base > 36)
		goto noconv;

	cutoff = UINTMAX_MAX / base;
	cutlim = UINTMAX_MAX % base;
	for ( ; ; c = *s++) {
		if (c >= '0' && c <= '9')
			c -= '0';
		else if (c >= 'A' && c <= 'Z')
			c -= 'A' - 10;
		else if (c >= 'a' && c <= 'z')
			c -= 'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
			any = -1;
		else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if (any < 0) {
		acc = UINTMAX_MAX;
		errno = ERANGE;
	} else if (!any) {
noconv:
		errno = EINVAL;
	} else if (neg)
		acc = -acc;
	if (endptr != NULL)
		*endptr = (char *)(any ? s - 1 : nptr);
	return (acc);
}

uintmax_t
strtoumax(const char * __restrict nptr, char ** __restrict endptr, int base)
{
	return strtoumax_l(nptr, endptr, base, __get_locale());
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Copyright (c) 2011 The FreeBSD Foundation
 */

unsigned long
strtoul_l(const char * __restrict nptr, char ** __restrict endptr, int base, locale_t locale)
{
	const char *s;
	unsigned long acc;
	char c;
	unsigned long cutoff;
	int neg, any, cutlim;
	FIX_LOCALE(locale);

	s = nptr;
	do {
		c = *s++;
	} while (isspace_l((unsigned char)c, locale));
	if (c == '-') {
		neg = 1;
		c = *s++;
	} else {
		neg = 0;
		if (c == '+')
			c = *s++;
	}
	if ((base == 0 || base == 16) &&
	    c == '0' && (*s == 'x' || *s == 'X') &&
	    ((s[1] >= '0' && s[1] <= '9') ||
	    (s[1] >= 'A' && s[1] <= 'F') ||
	    (s[1] >= 'a' && s[1] <= 'f'))) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if ((base == 0 || base == 2) &&
	    c == '0' && (*s == 'b' || *s == 'B') &&
	    (s[1] >= '0' && s[1] <= '1')) {
		c = s[1];
		s += 2;
		base = 2;
	}
	if (base == 0)
		base = c == '0' ? 8 : 10;
	acc = any = 0;
	if (base < 2 || base > 36)
		goto noconv;

	cutoff = ULONG_MAX / base;
	cutlim = ULONG_MAX % base;
	for ( ; ; c = *s++) {
		if (c >= '0' && c <= '9')
			c -= '0';
		else if (c >= 'A' && c <= 'Z')
			c -= 'A' - 10;
		else if (c >= 'a' && c <= 'z')
			c -= 'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
			any = -1;
		else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if (any < 0) {
		acc = ULONG_MAX;
		errno = ERANGE;
	} else if (!any) {
noconv:
		errno = EINVAL;
	} else if (neg)
		acc = -acc;
	if (endptr != NULL)
		*endptr = (char *)(any ? s - 1 : nptr);
	return (acc);
}

unsigned long
strtoul(const char * __restrict nptr, char ** __restrict endptr, int base)
{
	return strtoul_l(nptr, endptr, base, __get_locale());
}

} /* namespace pbsd::lib_libc_stdlib::b0214 */
