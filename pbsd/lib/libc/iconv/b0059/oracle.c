#define _DEFAULT_SOURCE

/*
 * PBSD batch b0059 -- reference oracle.
 *
 * Concatenation of the original HardenedBSD C sources, every function renamed
 * with a "ref_" prefix.  Function bodies are UNMODIFIED.  Only declarations,
 * typedefs and macros that the original private headers would have supplied
 * have been added so that the file builds standalone.
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

#ifndef LONG_BIT
#define	LONG_BIT	(sizeof(long) * CHAR_BIT)
#endif

#ifndef __DECONST
#define	__DECONST(type, var)	((type)(uintptr_t)(const void *)(var))
#endif

#ifndef __inline
#define	__inline	inline
#endif

struct _citrus_region {
	void	*r_head;
	size_t	 r_size;
};

static __inline void
_citrus_region_init(struct _citrus_region *r, void *h, size_t sz)
{

	r->r_head = h;
	r->r_size = sz;
}

static __inline void *
_citrus_region_head(const struct _citrus_region *r)
{

	return (r->r_head);
}

static __inline size_t
_citrus_region_size(const struct _citrus_region *r)
{

	return (r->r_size);
}

#define _region			_citrus_region
#define _region_init		_citrus_region_init
#define _region_head		_citrus_region_head
#define _region_size		_citrus_region_size

#define _CITRUS_BCS_PRED(_name_, _cond_) \
static __inline int _citrus_bcs_##_name_(uint8_t c) { return (_cond_); }

_CITRUS_BCS_PRED(isblank, c == ' ' || c == '\t')
_CITRUS_BCS_PRED(iseol, c == '\n' || c == '\r')
_CITRUS_BCS_PRED(isspace, _citrus_bcs_isblank(c) || _citrus_bcs_iseol(c) ||
    c == '\f' || c == '\v')
_CITRUS_BCS_PRED(isdigit, c >= '0' && c <= '9')
_CITRUS_BCS_PRED(isupper, c >= 'A' && c <= 'Z')
_CITRUS_BCS_PRED(islower, c >= 'a' && c <= 'z')
_CITRUS_BCS_PRED(isalpha, _citrus_bcs_isupper(c) || _citrus_bcs_islower(c))

static __inline uint8_t
_citrus_bcs_tolower(uint8_t c)
{

	return (_citrus_bcs_isupper(c) ? (c - 'A' + 'a') : c);
}

#define _bcs_isalnum		_citrus_bcs_isalnum
#define _bcs_isalpha		_citrus_bcs_isalpha
#define _bcs_isblank		_citrus_bcs_isblank
#define _bcs_isdigit		_citrus_bcs_isdigit
#define _bcs_islower		_citrus_bcs_islower
#define _bcs_iseol		_citrus_bcs_iseol
#define _bcs_isspace		_citrus_bcs_isspace
#define _bcs_isupper		_citrus_bcs_isupper
#define _bcs_isxdigit		_citrus_bcs_isxdigit
#define _bcs_tolower		_citrus_bcs_tolower
#define _bcs_toupper		_citrus_bcs_toupper

/* ================================================================== */
/* lib/libc/iconv/citrus_bcs_strtol.c                                */
/* ================================================================== */

/* $NetBSD: citrus_bcs_strtol.c,v 1.4 2013/04/26 21:20:47 joerg Exp $ */

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

#define	_FUNCNAME	ref__bcs_strtol
#define	__INT		long int
#define	__INT_MIN	LONG_MIN
#define	__INT_MAX	LONG_MAX

#undef isspace
#define isspace(c)	_bcs_isspace(c)

#undef isdigit
#define isdigit(c)	_bcs_isdigit(c)

#undef isalpha
#define isalpha(c)	_bcs_isalpha(c)

#undef isupper
#define isupper(c)	_bcs_isupper(c)

/* $NetBSD: _strtol.h,v 1.2 2009/05/20 22:03:29 christos Exp $ */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Original version ID:
 * NetBSD: src/lib/libc/locale/_wcstol.h,v 1.2 2003/08/07 16:43:03 agc Exp
 */

__INT
_FUNCNAME(const char *nptr, char **endptr, int base)
{
	const char *s;
	__INT acc, cutoff;
	unsigned char c;
	int any, cutlim, i, neg;

	/* check base value */
	if (base && (base < 2 || base > 36)) {
#if !defined(_KERNEL) && !defined(_STANDALONE)
		errno = EINVAL;
		if (endptr != NULL)
			/* LINTED interface specification */
			*endptr = __DECONST(void *, nptr);
		return (0);
#else
		panic("%s: invalid base %d", __func__, base);
#endif
	}

	/*
	 * Skip white space and pick up leading +/- sign if any.
	 * If base is 0, allow 0x for hex and 0 for octal, else
	 * assume decimal; if base is already 16, allow 0x.
	 */
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

	/*
	 * Compute the cutoff value between legal numbers and illegal
	 * numbers.  That is the largest legal value, divided by the
	 * base.  An input number that is greater than this value, if
	 * followed by a legal input character, is too big.  One that
	 * is equal to this value may be valid or not; the limit
	 * between valid and invalid numbers is then based on the last
	 * digit.  For instance, if the range for longs is
	 * [-2147483648..2147483647] and the input base is 10,
	 * cutoff will be set to 214748364 and cutlim to either
	 * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
	 * a value > 214748364, or equal but the next digit is > 7 (or 8),
	 * the number is too big, and we will return a range error.
	 *
	 * Set any if any `digits' consumed; make it negative to indicate
	 * overflow.
	 */
	cutoff = (neg ? __INT_MIN : __INT_MAX);
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
				acc = __INT_MIN;
#if !defined(_KERNEL) && !defined(_STANDALONE)
				any = -1;
				errno = ERANGE;
#else
				any = 0;
				break;
#endif
			} else {
				any = 1;
				acc *= base;
				acc -= i;
			}
		} else {
			if (acc > cutoff || (acc == cutoff && i > cutlim)) {
				acc = __INT_MAX;
#if !defined(_KERNEL) && !defined(_STANDALONE)
				any = -1;
				errno = ERANGE;
#else
				any = 0;
				break;
#endif
			} else {
				any = 1;
				acc *= base;
				acc += i;
			}
		}
	}
	if (endptr != NULL)
		/* LINTED interface specification */
		*endptr = __DECONST(void *, any ? s - 1 : nptr);
	return(acc);
}

/* ================================================================== */
/* lib/libc/iconv/citrus_bcs_strtoul.c                               */
/* ================================================================== */

/* $NetBSD: citrus_bcs_strtoul.c,v 1.5 2013/04/26 21:20:48 joerg Exp $ */

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

#define	_FUNCNAME	ref__bcs_strtoul
#define	__UINT		unsigned long int
#define	__UINT_MAX	ULONG_MAX

#undef isspace
#define isspace(c)	_bcs_isspace(c)

#undef isdigit
#define isdigit(c)	_bcs_isdigit(c)

#undef isalpha
#define isalpha(c)	_bcs_isalpha(c)

#undef isupper
#define isupper(c)	_bcs_isupper(c)

/* $NetBSD: _strtoul.h,v 1.1 2008/08/20 12:42:26 joerg Exp $ */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Original version ID:
 * NetBSD: src/lib/libc/locale/_wcstoul.h,v 1.2 2003/08/07 16:43:03 agc Exp
 */

__UINT
_FUNCNAME(const char *nptr, char **endptr, int base)
{
	const char *s;
	__UINT acc, cutoff;
	unsigned char c;
	int any, cutlim, i, neg;

	/* check base value */
	if (base && (base < 2 || base > 36)) {
#if !defined(_KERNEL) && !defined(_STANDALONE)
		errno = EINVAL;
		return (0);
#else
		panic("%s: invalid base %d", __func__, base);
#endif
	}

	/*
	 * Skip white space and pick up leading +/- sign if any.
	 * If base is 0, allow 0x for hex and 0 for octal, else
	 * assume decimal; if base is already 16, allow 0x.
	 */
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

	/*
	 * See strtol for comments as to the logic used.
	 */
	cutoff = __UINT_MAX / (__UINT)base;
	cutlim = (int)(__UINT_MAX % (__UINT)base);
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
			acc = __UINT_MAX;
#if !defined(_KERNEL) && !defined(_STANDALONE)
			any = -1;
			errno = ERANGE;
#else
			any = 0;
			break;
#endif
		} else {
			any = 1;
			acc *= (__UINT)base;
			acc += i;
		}
	}
	if (neg && any > 0)
		acc = -acc;
	if (endptr != NULL)
		/* LINTED interface specification */
		*endptr = __DECONST(void *, any ? s - 1 : nptr);
	return (acc);
}

/* ================================================================== */
/* lib/libc/iconv/citrus_db_hash.c                                     */
/* ================================================================== */

/* $NetBSD: citrus_db_hash.c,v 1.5 2008/02/09 14:56:20 junyoung Exp $ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c)2003 Citrus Project,
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

uint32_t
ref__citrus_db_hash_std(struct _region *r)
{
	const uint8_t *p;
	uint32_t hash, tmp;
	size_t i;

	hash = 0;
	p = _region_head(r);

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

/* ================================================================== */
/* lib/libc/iconv/citrus_mmap.c                                        */
/* ================================================================== */

/*	$NetBSD: citrus_mmap.c,v 1.4 2011/10/15 23:00:01 christos Exp $	*/

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c)2003 Citrus Project,
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

#define _open	open
#define _close	close
#define _fstat	fstat

int
ref__citrus_map_file(struct _citrus_region * __restrict r,
    const char * __restrict path)
{
	struct stat st;
	void *head;
	int fd, ret;

	ret = 0;

	_region_init(r, NULL, 0);

	if ((fd = _open(path, O_RDONLY | O_CLOEXEC)) == -1)
		return (errno);

	if (_fstat(fd, &st)  == -1) {
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
	(void)_close(fd);
	return (ret);
}

void
ref__citrus_unmap_file(struct _citrus_region *r)
{

	if (_region_head(r) != NULL) {
		(void)munmap(_region_head(r), _region_size(r));
		_region_init(r, NULL, 0);
	}
}
