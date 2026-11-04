/*
 * oracle.c -- reference implementation for PBSD batch b0328.
 *
 * The original HardenedBSD C sources of the batch, concatenated, with every
 * function renamed with a "ref_" prefix.  Function bodies are UNMODIFIED.
 *
 * Sources:
 *   hbsd/src/lib/libc/stdlib/strtoull.c
 *   hbsd/src/lib/libc/stdlib/getopt.c
 *   hbsd/src/lib/libc/stdlib/hsearch_r.c
 */

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

typedef struct {
	int dummy;
} ref_xlocale;

typedef ref_xlocale *locale_t;

static ref_xlocale ref___xlocale_C_locale;
static ref_xlocale ref___xlocale_global_locale;

static locale_t
ref_get_real_locale(locale_t locale)
{
	switch ((intptr_t)locale) {
	case 0:
		return (&ref___xlocale_C_locale);
	case -1:
		return (&ref___xlocale_global_locale);
	default:
		return (locale);
	}
}

#define FIX_LOCALE(l) (l = ref_get_real_locale(l))

static int
ref_isspace_l(int c, locale_t locale)
{
	(void)locale;

	return (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' ||
	    c == '\v');
}

#define isspace_l ref_isspace_l

static locale_t
ref___get_locale(void)
{

	return (&ref___xlocale_global_locale);
}

#define strtoull_l ref_strtoull_l
#define strtoull ref_strtoull
#define __get_locale ref___get_locale

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Copyright (c) 2011 The FreeBSD Foundation
 *
 * Portions of this software were developed by David Chisnall
 * under sponsorship from the FreeBSD Foundation.
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
 */

unsigned long long
ref_strtoull_l(const char * __restrict nptr, char ** __restrict endptr, int base,
		locale_t locale)
{
	const char *s;
	unsigned long long acc;
	char c;
	unsigned long long cutoff;
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

	cutoff = ULLONG_MAX / base;
	cutlim = ULLONG_MAX % base;
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
		acc = ULLONG_MAX;
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
unsigned long long
ref_strtoull(const char * __restrict nptr, char ** __restrict endptr, int base)
{
	return strtoull_l(nptr, endptr, base, __get_locale());
}

const char *
ref__getprogname(void)
{

	return ("b0328");
}

#define _getprogname ref__getprogname

int	ref_opterr = 1;
int	ref_optind = 1;
int	ref_optopt;
int	ref_optreset;
char	*ref_optarg;

#define opterr		ref_opterr
#define optind		ref_optind
#define optopt		ref_optopt
#define optreset	ref_optreset
#define optarg		ref_optarg
#define getopt		ref_getopt

/*	$NetBSD: getopt.c,v 1.29 2014/06/05 22:00:22 christos Exp $	*/

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1987, 1993, 1994
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
 */

#define	BADCH	(int)'?'
#define	BADARG	(int)':'
static char EMSG[] = "";

int
ref_getopt(int nargc, char * const nargv[], const char *ostr)
{
	static char *place = EMSG;
	char *oli;

	if (optreset || *place == 0) {
		optreset = 0;
		place = nargv[optind];
		if (optind >= nargc || *place++ != '-') {
			place = EMSG;
			return (-1);
		}
		optopt = *place++;
		if (optopt == '-' && *place == 0) {
			++optind;
			place = EMSG;
			return (-1);
		}
		if (optopt == 0) {
			place = EMSG;
			if (strchr(ostr, '-') == NULL)
				return (-1);
			optopt = '-';
		}
	} else
		optopt = *place++;

	if (optopt == ':' || (oli = strchr(ostr, optopt)) == NULL) {
		if (*place == 0)
			++optind;
		if (opterr && *ostr != ':')
			(void)fprintf(stderr,
			    "%s: illegal option -- %c\n", _getprogname(),
			    optopt);
		return (BADCH);
	}

	if (oli[1] != ':') {
		optarg = NULL;
		if (*place == 0)
			++optind;
	} else {
		if (*place)
			optarg = place;
		else if (oli[2] == ':')
			optarg = NULL;
		else if (nargc > ++optind)
			optarg = nargv[optind];
		else {
			place = EMSG;
			if (*ostr == ':')
				return (BADARG);
			if (opterr)
				(void)fprintf(stderr,
				    "%s: option requires an argument -- %c\n",
				    _getprogname(), optopt);
			return (BADCH);
		}
		place = EMSG;
		++optind;
	}
	return (optopt);
}

void
ref_getopt_reset_state(void)
{

	ref_optreset = 1;
}

typedef struct entry {
	char	*key;
	void	*data;
} ENTRY;

typedef enum {
	FIND, ENTER
} ACTION;

struct __hsearch {
	size_t offset_basis;
	size_t index_mask;
	size_t entries_used;
	ENTRY *entries;
};

struct hsearch_data {
	struct __hsearch *__hsearch;
};

#define hsearch_lookup_free ref_hsearch_lookup_free
#define hsearch_hash ref_hsearch_hash
#define hsearch_r ref_hsearch_r

/*-
 * Copyright (c) 2015 Nuxi, https://nuxi.nl/
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

static ENTRY *
ref_hsearch_lookup_free(struct __hsearch *hsearch, size_t hash)
{
	size_t index, i;

	for (index = hash, i = 0;; index += ++i) {
		ENTRY *entry = &hsearch->entries[index & hsearch->index_mask];
		if (entry->key == NULL)
			return (entry);
	}
}

static size_t
ref_hsearch_hash(size_t offset_basis, const char *str)
{
	size_t hash;

	hash = offset_basis;
	while (*str != '\0') {
		hash ^= (uint8_t)*str++;
		if (sizeof(size_t) * CHAR_BIT <= 32)
			hash *= UINT32_C(16777619);
		else
			hash *= UINT64_C(1099511628211);
	}
	return (hash);
}

int
ref_hsearch_r(ENTRY item, ACTION action, ENTRY **retval, struct hsearch_data *htab)
{
	struct __hsearch *hsearch;
	ENTRY *entry, *old_entries, *new_entries;
	size_t hash, index, i, old_hash, old_count, new_count;

	hsearch = htab->__hsearch;
	hash = hsearch_hash(hsearch->offset_basis, item.key);

	for (index = hash, i = 0;; index += ++i) {
		entry = &hsearch->entries[index & hsearch->index_mask];
		if (entry->key == NULL)
			break;
		if (strcmp(entry->key, item.key) == 0) {
			*retval = entry;
			return (1);
		}
	}

	if (action == FIND) {
		errno = ESRCH;
		return (0);
	}

	if (hsearch->entries_used * 2 >= hsearch->index_mask) {
		old_count = hsearch->index_mask + 1;
		old_entries = hsearch->entries;

		new_count = (hsearch->index_mask + 1) * 2;
		new_entries = calloc(new_count, sizeof(ENTRY));
		if (new_entries == NULL)
			return (0);
		hsearch->entries = new_entries;
		hsearch->index_mask = new_count - 1;

		for (i = 0; i < old_count; ++i) {
			entry = &old_entries[i];
			if (entry->key != NULL) {
				old_hash = hsearch_hash(hsearch->offset_basis,
				    entry->key);
				*hsearch_lookup_free(hsearch, old_hash) =
				    *entry;
			}
		}

		free(old_entries);

		hsearch = htab->__hsearch;
		entry = hsearch_lookup_free(hsearch, hash);
	}

	*entry = item;
	++hsearch->entries_used;
	*retval = entry;
	return (1);
}
