/*
 * oracle.c -- reference (specification) build for PBSD batch b0120.
 *
 * The original HardenedBSD sources
 *
 *	hbsd/src/lib/libc/amd64/string/bzero.c
 *	hbsd/src/lib/libc/amd64/string/bcopy.c
 *	hbsd/src/lib/libc/amd64/string/strncat.c
 *	hbsd/src/lib/libc/amd64/string/strlcat.c
 *
 * concatenated verbatim, with each exported function renamed with a "ref_"
 * prefix.  No function body has been modified.
 */

#include <stddef.h>
#include <string.h>
#include <sys/cdefs.h>

/*
 * The wrappers below delegate the actual work to the hand written amd64
 * assembly routines __memccpy(), __memchr() and __strlcpy(), which are not
 * part of this batch.  Bind those names to equivalent libc entry points, or
 * supply definitions with exactly the documented semantics, so that the
 * unmodified bodies below can be linked and executed.
 */
void *
ref_h_memccpy(void *restrict dst, const void *restrict src, int c, size_t n)
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

void *
ref_h_memchr(const void *s, int c, size_t n)
{
	return (memchr(s, c, n));
}

size_t
ref_h_strlcpy(char *restrict dst, const char *restrict src, size_t dsize)
{
	const char *osrc = src;
	size_t nleft = dsize;

	/* Copy as many bytes as will fit. */
	if (nleft != 0) {
		while (--nleft != 0) {
			if ((*dst++ = *src++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src. */
	if (nleft == 0) {
		if (dsize != 0)
			*dst = '\0';		/* NUL-terminate dst */
		while (*src++)
			;
	}

	return (src - osrc - 1);	/* count does not include NUL */
}

#define __memccpy	ref_h_memccpy
#define __memchr	ref_h_memchr
#define __strlcpy	ref_h_strlcpy

/* ------------------------------------------------------------------ */
/* hbsd/src/lib/libc/amd64/string/bzero.c				*/
/* ------------------------------------------------------------------ */

/*-
 * Public domain.
 */

#undef bzero	/* _FORTIFY_SOURCE */

void
ref_bzero(void *b, size_t len)
{

	memset(b, 0, len);
}

/* ------------------------------------------------------------------ */
/* hbsd/src/lib/libc/amd64/string/bcopy.c				*/
/* ------------------------------------------------------------------ */

/*-
 * Public domain.
 */

#undef bcopy	/* _FORTIFY_SOURCE */

void
ref_bcopy(const void *src, void *dst, size_t len)
{

	memmove(dst, src, len);
}

/* ------------------------------------------------------------------ */
/* hbsd/src/lib/libc/amd64/string/strncat.c				*/
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Robert Clausecker
 */

#undef strncat	/* _FORTIFY_SOURCE */

void *__memccpy(void *restrict, const void *restrict, int, size_t);

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

/* ------------------------------------------------------------------ */
/* hbsd/src/lib/libc/amd64/string/strlcat.c				*/
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Robert Clausecker
 */

#undef strlcat	/* FORTIFY_SOURCE */

void *__memchr(const void *, int, size_t);
size_t __strlcpy(char *restrict, const char *restrict, size_t);

size_t
ref_strlcat(char *restrict dst, const char *restrict src, size_t dstsize)
{
	char *loc = __memchr(dst, '\0', dstsize);

	if (loc != NULL) {
		size_t dstlen = (size_t)(loc - dst);

		return (dstlen + __strlcpy(loc, src, dstsize - dstlen));
	} else
		return (dstsize + strlen(src));
}
