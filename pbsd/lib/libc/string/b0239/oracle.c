/*
 * Reference oracle for PBSD batch b0239.
 *
 * The original HardenedBSD C sources concatenated verbatim; every function has
 * been renamed with a `ref_` prefix and nothing else has been touched.
 *
 * Sources:
 *   hbsd/src/lib/libc/string/swab.c
 *   hbsd/src/lib/libc/string/strndup.c
 *   hbsd/src/lib/libc/string/timingsafe_bcmp.c
 */

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/endian.h>
#include <unistd.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

/* ------------------------------------------------------------------------- */
/* hbsd/src/lib/libc/string/swab.c                                           */
/* ------------------------------------------------------------------------- */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 * Copyright (c) 2024 rilysh <nightquick@proton.me>
 */

void
ref_swab(const void * __restrict from, void * __restrict to, ssize_t len)
{
	const char *f = from;
	char *t = to;
	uint16_t tmp;

	/*
	 * POSIX says overlapping copy behavior is undefined, however many
	 * applications assume the old FreeBSD and current GNU libc behavior
	 * that will swap the bytes correctly when from == to. Reading both bytes
	 * and swapping them before writing them back accomplishes this.
	 */
	while (len > 1) {
		memcpy(&tmp, f, 2);
		tmp = bswap16(tmp);
		memcpy(t, &tmp, 2);

		f += 2;
		t += 2;
		len -= 2;
	}
}

/* ------------------------------------------------------------------------- */
/* hbsd/src/lib/libc/string/strndup.c                                        */
/* ------------------------------------------------------------------------- */

/*	$OpenBSD: strndup.c,v 1.1 2010/05/18 22:24:55 tedu Exp $	*/

/*
 * Copyright (c) 2010 Todd C. Miller <Todd.Miller@courtesan.com>
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

char *
ref_strndup(const char *str, size_t maxlen)
{
	char *copy;
	size_t len;

	len = strnlen(str, maxlen);
	copy = malloc(len + 1);
	if (copy != NULL) {
		(void)memcpy(copy, str, len);
		copy[len] = '\0';
	}

	return copy;
}

/* ------------------------------------------------------------------------- */
/* hbsd/src/lib/libc/string/timingsafe_bcmp.c                                */
/* ------------------------------------------------------------------------- */

/*	$OpenBSD: timingsafe_bcmp.c,v 1.3 2015/08/31 02:53:57 guenther Exp $	*/
/*
 * Copyright (c) 2010 Damien Miller.  All rights reserved.
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
ref___timingsafe_bcmp(const void *b1, const void *b2, size_t n)
{
	const unsigned char *p1 = b1, *p2 = b2;
	int ret = 0;

	for (; n > 0; n--)
		ret |= *p1++ ^ *p2++;
	return (ret != 0);
}
