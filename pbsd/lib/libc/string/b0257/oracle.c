/*
 * Reference oracle for PBSD batch b0257.
 *
 * The original HardenedBSD C sources concatenated verbatim; every function has
 * been renamed with a `ref_` prefix and nothing else has been touched.
 *
 * Sources:
 *   hbsd/src/lib/libc/string/wcsncasecmp.c
 *   hbsd/src/lib/libc/string/timingsafe_memcmp.c
 */

#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

/* ------------------------------------------------------------------------- */
/* hbsd/src/lib/libc/string/wcsncasecmp.c                                    */
/* ------------------------------------------------------------------------- */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2009 David Schultz <das@FreeBSD.org>
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

int
ref_wcsncasecmp(const wchar_t *s1, const wchar_t *s2, size_t n)
{
	wchar_t c1, c2;

	if (n == 0)
		return (0);
	for (; *s1; s1++, s2++) {
		c1 = towlower(*s1);
		c2 = towlower(*s2);
		if (c1 != c2)
			return ((int)c1 - c2);
		if (--n == 0)
			return (0);
	}
	return (-*s2);
}

/* ------------------------------------------------------------------------- */
/* hbsd/src/lib/libc/string/timingsafe_memcmp.c                              */
/* ------------------------------------------------------------------------- */

/*	$OpenBSD: timingsafe_memcmp.c,v 1.2 2015/08/31 02:53:57 guenther Exp $	*/
/*
 * Copyright (c) 2014 Google Inc.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

int
ref___timingsafe_memcmp(const void *b1, const void *b2, size_t len)
{
        const unsigned char *p1 = b1, *p2 = b2;
        size_t i;
        int res = 0, done = 0;

        for (i = 0; i < len; i++) {
                /* lt is -1 if p1[i] < p2[i]; else 0. */
                int lt = (p1[i] - p2[i]) >> CHAR_BIT;

                /* gt is -1 if p1[i] > p2[i]; else 0. */
                int gt = (p2[i] - p1[i]) >> CHAR_BIT;

                /* cmp is 1 if p1[i] > p2[i]; -1 if p1[i] < p2[i]; else 0. */
                int cmp = lt - gt;

                /* set res = cmp if !done. */
                res |= cmp & ~done;

                /* set done if p1[i] != p2[i]. */
                done |= lt | gt;
        }

        return (res);
}
