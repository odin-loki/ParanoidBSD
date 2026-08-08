/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2013 Peter Wemm
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
 * Batch b0072, ported from:
 *	lib/libc/iconv/iconv_open_into.c
 *	lib/libc/iconv/__iconv.c
 *	lib/libc/iconv/iconv.c
 *	lib/libc/iconv/citrus_hash.c
 */

module;

#include <sys/types.h>

export module pbsd.lib.libc.iconv.b0072;

extern "C" {
size_t	__bsd_iconv(void *, char **, size_t *, char **, size_t *);
size_t	__bsd___iconv(void *, char **, size_t *, char **, size_t *,
	    __uint32_t, size_t *);
int	__bsd_iconv_open_into(const char *, const char *, void *);
}

#include <stdint.h>
#include <string.h>

#ifndef __DECONST
#define	__DECONST(type, var)	((type)(uintptr_t)(const void *)(var))
#endif

export namespace pbsd::lib_libc_iconv::b0072 {

using iconv_t = void *;

struct iconv_allocation_t {
	void *spaceholder[64];
};

struct _citrus_region {
	void	*r_head;
	size_t	 r_size;
};

#define	_region			_citrus_region
#define	_region_init		_citrus_region_init
#define	_region_head		_citrus_region_head
#define	_region_size		_citrus_region_size
#define	_bcs_tolower		_citrus_bcs_tolower
#define	_db_hash_std		_citrus_db_hash_std

#define _CITRUS_BCS_PRED(_name_, _cond_) \
static inline int _citrus_bcs_##_name_(uint8_t c) { return (_cond_); }

_CITRUS_BCS_PRED(isupper, c >= 'A' && c <= 'Z')

static inline uint8_t
_citrus_bcs_tolower(uint8_t c)
{

	return (_citrus_bcs_isupper(c) ? (c - 'A' + 'a') : c);
}

static inline void
_citrus_region_init(struct _citrus_region *r, void *h, size_t sz)
{

	r->r_head = h;
	r->r_size = sz;
}

static inline void *
_citrus_region_head(const struct _citrus_region *r)
{

	return (r->r_head);
}

static inline size_t
_citrus_region_size(const struct _citrus_region *r)
{

	return (r->r_size);
}

static uint32_t
_citrus_db_hash_std(struct _region *r)
{
	const uint8_t *p;
	uint32_t hash, tmp;
	size_t i;

	hash = 0;
	p = (const uint8_t *)_region_head(r);

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
iconv_open_into(const char *a, const char *b, iconv_allocation_t *c)
{
	return __bsd_iconv_open_into(a, b, c);
}

size_t
__iconv(iconv_t a, char **b, size_t *c, char **d,
     size_t *e, __uint32_t f, size_t *g)
{
	return __bsd___iconv(a, b, c, d, e, f, g);
}

size_t
iconv(iconv_t a, char ** __restrict b,
      size_t * __restrict c, char ** __restrict d,
      size_t * __restrict e)
{
	return __bsd_iconv(a, b, c, d, e);
}

int
_citrus_string_hash_func(const char *key, int hashsize)
{
	struct _region r;

	_region_init(&r, __DECONST(void *, key), strlen(key));

	return ((int)(_db_hash_std(&r) % (uint32_t)hashsize));
}

} // namespace pbsd::lib_libc_iconv::b0072
