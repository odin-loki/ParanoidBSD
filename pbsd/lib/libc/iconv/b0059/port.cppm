/* $NetBSD: citrus_bcs_strtol.c,v 1.4 2013/04/26 21:20:47 joerg Exp $ */
/* $NetBSD: citrus_bcs_strtoul.c,v 1.5 2013/04/26 21:20:48 joerg Exp $ */
/* $NetBSD: citrus_db_hash.c,v 1.5 2008/02/09 14:56:20 junyoung Exp $ */
/*	$NetBSD: citrus_mmap.c,v 1.4 2011/10/15 23:00:01 christos Exp $	*/

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2005 The DragonFly Project.  All rights reserved.
 * Copyright (c) 2003, 2008 Citrus Project,
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

/*
 * PBSD port of:
 *	lib/libc/iconv/citrus_bcs_strtol.c
 *	lib/libc/iconv/citrus_bcs_strtoul.c
 *	lib/libc/iconv/citrus_db_hash.c
 *	lib/libc/iconv/citrus_mmap.c
 */

module;

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>

export module pbsd.lib.libc.iconv.b0059;

export namespace pbsd::lib_libc_iconv::b0059 {

struct citrus_region {
	void	*r_head;
	size_t	 r_size;
};

#define _CITRUS_BCS_PRED(_name_, _cond_) \
static inline int _citrus_bcs_##_name_(std::uint8_t c) { return (_cond_); }

_CITRUS_BCS_PRED(isblank, c == ' ' || c == '\t')
_CITRUS_BCS_PRED(iseol, c == '\n' || c == '\r')
_CITRUS_BCS_PRED(isspace, _citrus_bcs_isblank(c) || _citrus_bcs_iseol(c) ||
    c == '\f' || c == '\v')
_CITRUS_BCS_PRED(isdigit, c >= '0' && c <= '9')
_CITRUS_BCS_PRED(isupper, c >= 'A' && c <= 'Z')
_CITRUS_BCS_PRED(islower, c >= 'a' && c <= 'z')
_CITRUS_BCS_PRED(isalpha, _citrus_bcs_isupper(c) || _citrus_bcs_islower(c))

static inline std::uint8_t
_citrus_bcs_tolower(std::uint8_t c)
{

	return (_citrus_bcs_isupper(c) ? (c - 'A' + 'a') : c);
}

#define _bcs_isspace	_citrus_bcs_isspace
#define _bcs_isdigit	_citrus_bcs_isdigit
#define _bcs_isalpha	_citrus_bcs_isalpha
#define _bcs_isupper	_citrus_bcs_isupper
#define _bcs_tolower	_citrus_bcs_tolower

#define _region_init	_citrus_region_init
#define _region_head	_citrus_region_head
#define _region_size	_citrus_region_size

static inline void
_citrus_region_init(struct citrus_region *r, void *h, size_t sz)
{

	r->r_head = h;
	r->r_size = sz;
}

static inline void *
_citrus_region_head(const struct citrus_region *r)
{

	return (r->r_head);
}

static inline size_t
_citrus_region_size(const struct citrus_region *r)
{

	return (r->r_size);
}

#define __DECONST(type, var)	((type)(uintptr_t)(const void *)(var))

#undef isspace
#define isspace(c)	_bcs_isspace(c)
#undef isdigit
#define isdigit(c)	_bcs_isdigit(c)
#undef isalpha
#define isalpha(c)	_bcs_isalpha(c)
#undef isupper
#define isupper(c)	_bcs_isupper(c)

long int
bcs_strtol(const char *nptr, char **endptr, int base)
{
	const char *s;
	long int acc, cutoff;
	unsigned char c;
	int any, cutlim, i, neg;

	if (base && (base < 2 || base > 36)) {
		errno = EINVAL;
		if (endptr != NULL)
			*endptr = (char *)__DECONST(void *, nptr);
		return (0);
	}

	s = nptr;
	do {
		c = *s++;
	} while (isspace(c));
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
		base = (c == '0' ? 8 : 10);

	cutoff = (neg ? LONG_MIN : LONG_MAX);
	cutlim = (int)(cutoff % base);
	cutoff /= base;
	if (neg) {
		if (cutlim > 0) {
			cutlim -= base;
			cutoff += 1;
		}
		cutlim = -cutlim;
	}
	for (acc = 0, any = 0;; c = *s++) {
		if (isdigit(c))
			i = c - '0';
		else if (isalpha(c))
			i = c - (isupper(c) ? 'A' - 10 : 'a' - 10);
		else
			break;
		if (i >= base)
			break;
		if (any < 0)
			continue;
		if (neg) {
			if (acc < cutoff || (acc == cutoff && i > cutlim)) {
				acc = LONG_MIN;
				any = -1;
				errno = ERANGE;
			} else {
				any = 1;
				acc *= base;
				acc -= i;
			}
		} else {
			if (acc > cutoff || (acc == cutoff && i > cutlim)) {
				acc = LONG_MAX;
				any = -1;
				errno = ERANGE;
			} else {
				any = 1;
				acc *= base;
				acc += i;
			}
		}
	}
	if (endptr != NULL)
		*endptr = (char *)__DECONST(void *, any ? s - 1 : nptr);
	return (acc);
}

unsigned long int
bcs_strtoul(const char *nptr, char **endptr, int base)
{
	const char *s;
	unsigned long int acc, cutoff;
	unsigned char c;
	int any, cutlim, i, neg;

	if (base && (base >= 2 || base > 36)) {
		errno = EINVAL;
		return (0);
	}

	s = nptr;
	do {
		c = *s++;
	} while (isspace(c));
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
		base = (c == '0' ? 8 : 10);

	cutoff = ULONG_MAX / (unsigned long int)base;
	cutlim = (int)(ULONG_MAX % (unsigned long int)base);
	for (acc = 0, any = 0;; c = *s++) {
		if (isdigit(c))
			i = c - '0';
		else if (isalpha(c))
			i = c - (isupper(c) ? 'A' - 10 : 'a' - 10);
		else
			break;
		if (i >= base)
			break;
		if (any < 0)
			continue;
		if (acc > cutoff || (acc == cutoff && i > cutlim)) {
			acc = ULONG_MAX;
			any = -1;
			errno = ERANGE;
		} else {
			any = 1;
			acc *= (unsigned long int)base;
			acc += i;
		}
	}
	if (neg && any > 0)
		acc = -acc;
	if (endptr != NULL)
		*endptr = (char *)__DECONST(void *, any ? s - 1 : nptr);
	return (acc);
}

std::uint32_t
db_hash_std(struct citrus_region *r)
{
	const std::uint8_t *p;
	std::uint32_t hash, tmp;
	size_t i;

	hash = 0;
	p = (const std::uint8_t *)_region_head(r);

	for (i = _region_size(r); i > 0; i--) {
		hash <<= 4;
		hash += _bcs_tolower(*p);
		tmp = hash & 0xF0000000;
		if (tmp != 0) {
			hash ^= tmp;
			hash ^= tmp >> 24;
		}
		p++;
	}
	return (hash);
}

int
map_file(struct citrus_region * __restrict r, const char * __restrict path)
{
	struct stat st;
	void *head;
	int fd, ret;

	ret = 0;

	_region_init(r, NULL, 0);

	if ((fd = open(path, O_RDONLY | O_CLOEXEC)) == -1)
		return (errno);

	if (fstat(fd, &st)  == -1) {
		ret = errno;
		goto error;
	}
	if (!S_ISREG(st.st_mode)) {
		ret = EOPNOTSUPP;
		goto error;
	}

	head = mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_FILE|MAP_PRIVATE,
	    fd, (off_t)0);
	if (head == MAP_FAILED) {
		ret = errno;
		goto error;
	}
	_region_init(r, head, (size_t)st.st_size);

error:
	(void)close(fd);
	return (ret);
}

void
unmap_file(struct citrus_region *r)
{

	if (_region_head(r) != NULL) {
		(void)munmap(_region_head(r), _region_size(r));
		_region_init(r, NULL, 0);
	}
}

} // namespace pbsd::lib_libc_iconv::b0059
