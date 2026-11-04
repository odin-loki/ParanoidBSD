/*
 * oracle.c -- reference (specification) build for PBSD batch b0112.
 *
 * The original HardenedBSD sources for this batch are concatenated below with
 * every ported function renamed with a "ref_" prefix.  Function bodies are
 * byte-for-byte unmodified.
 *
 * The aarch64 sources call out to helpers that are merely declared or supplied
 * by the system headers but not defined in these translation units.  Support
 * definitions with exactly the documented semantics are supplied here so that
 * the unmodified reference bodies can be linked and executed.
 */

/* ------------------------------------------------------------------ */
/* from hbsd/src/lib/libc/aarch64/string/bzero.c                      */
/* ------------------------------------------------------------------ */

/*-
 * Public domain.
 */

#include <string.h>

#undef bzero	/* _FORTIFY_SOURCE */

/* ------------------------------------------------------------------ */
/* from hbsd/src/lib/libc/aarch64/string/bcopy.c                      */
/* ------------------------------------------------------------------ */

/*-
 * Public domain.
 */

#undef bcopy	/* _FORTIFY_SOURCE */

/* ------------------------------------------------------------------ */
/* from hbsd/src/lib/libc/aarch64/string/strcat.c                     */
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2024 Getz Mikalsen <getz@FreeBSD.org>
*/

#undef strcat	/* _FORTIFY_SOURCE */

/* ------------------------------------------------------------------ */
/* from hbsd/src/lib/libc/aarch64/string/strncat.c                    */
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Robert Clausecker
 */

#include <sys/cdefs.h>

#undef strncat	/* _FORTIFY_SOURCE */

void *__memccpy(void *restrict, const void *restrict, int, size_t);

/* ------------------------------------------------------------------ */
/* support: definitions for helpers used by the reference bodies.      */
/* ------------------------------------------------------------------ */

/*
 * stpcpy(3) -- write src to dst including NUL, return pointer to the NUL.
 */
char *
stpcpy(char *dst, const char *src)
{
	while ((*dst++ = *src++) != '\0')
		;
	return (dst - 1);
}

/*
 * __memccpy is the libc memccpy(3); same semantics.
 */
void *
__memccpy(void *restrict dst, const void *restrict src, int c, size_t n)
{
	unsigned char *d = dst;
	const unsigned char *s = src;
	unsigned char ch = (unsigned char)c;

	while (n-- > 0) {
		if ((*d++ = *s++) == ch)
			return (d);
	}
	return (NULL);
}

/* ------------------------------------------------------------------ */
/* reference bodies                                                    */
/* ------------------------------------------------------------------ */

void
ref_bzero(void *b, size_t len)
{

	memset(b, 0, len);
}

void
ref_bcopy(const void *src, void *dst, size_t len)
{

	memmove(dst, src, len);
}

char *
ref_strcat(char * __restrict s, const char * __restrict append)
{
	char *save = s;

	/* call into SIMD optimized functions */
	stpcpy(s + strlen(s), append);

	return(save);
}

char *
ref_strncat(char *dest, const char *src, size_t n)
{
	size_t len;
	char *endptr;

	len = strlen(dest);
	endptr = __memccpy(dest + len, src, '\0', n);

	/* avoid an extra branch */
	if (endptr == NULL)
		endptr = dest + len + n + 1;

	endptr[-1] = '\0';

	return (dest);
}
