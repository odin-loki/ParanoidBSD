/*
 * oracle.c -- reference (specification) build for PBSD batch b0071.
 *
 * The original HardenedBSD sources for this batch are concatenated below with
 * every ported function renamed with a "ref_" prefix.  Function bodies are
 * byte-for-byte unmodified.
 *
 * The aarch64 sources in this batch call out to hand written assembly helpers
 * (__memchr_aarch64, __strlcpy, __strcspn) that are merely declared, never
 * defined, in the original translation units.  Definitions with exactly the
 * documented semantics of those helpers are supplied here so that the
 * unmodified reference bodies can be linked and executed; they are support
 * code, not part of the ported specification bodies.
 */

/* ------------------------------------------------------------------ */
/* from hbsd/src/lib/libc/aarch64/string/strlcat.c                     */
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Robert Clausecker
 */

#include <sys/cdefs.h>

#include <string.h>

#undef strlcat	/* _FORTIFY_SOURCE */

void *__memchr_aarch64(const void *, int, size_t);
size_t __strlcpy(char *restrict, const char *restrict, size_t);

/* ------------------------------------------------------------------ */
/* from hbsd/src/lib/libc/aarch64/string/strpbrk.c                     */
/* ------------------------------------------------------------------ */

/*-
 * Copyright (c) 2023 The FreeBSD Foundation
 *
 * This software was developed by Robert Clausecker <fuz@FreeBSD.org>
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ''AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE
 */

size_t __strcspn(const char *, const char *);

/* ------------------------------------------------------------------ */
/* support: definitions for the assembly helpers declared above.       */
/* ------------------------------------------------------------------ */

/*
 * __memchr_aarch64 is the aarch64 assembly memchr(3); same semantics.
 */
void *
__memchr_aarch64(const void *s, int c, size_t n)
{
	return (memchr(s, c, n));
}

/*
 * __strcspn is the aarch64 assembly strcspn(3); same semantics.
 */
size_t
__strcspn(const char *s, const char *charset)
{
	return (strcspn(s, charset));
}

/*
 * __strlcpy is the libc strlcpy(3) (lib/libc/string/strlcpy.c), reproduced
 * verbatim from FreeBSD.
 */
size_t
__strlcpy(char *restrict dst, const char *restrict src, size_t dsize)
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

/* ------------------------------------------------------------------ */
/* reference bodies                                                    */
/* ------------------------------------------------------------------ */

size_t
ref_strlcat(char *restrict dst, const char *restrict src, size_t dstsize)
{
	char *loc = __memchr_aarch64(dst, '\0', dstsize);

	if (loc != NULL) {
		size_t dstlen = (size_t)(loc - dst);

		return (dstlen + __strlcpy(loc, src, dstsize - dstlen));
	} else
		return (dstsize + strlen(src));
}

char *
ref_strpbrk(const char *s, const char *charset)
{
	size_t loc;

	loc = __strcspn(s, charset);

	return (s[loc] == '\0' ? NULL : (char *)&s[loc]);
}
