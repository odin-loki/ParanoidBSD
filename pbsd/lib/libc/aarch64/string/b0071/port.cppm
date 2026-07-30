/*
 * PBSD batch b0071 -- C++23 port of:
 *   hbsd/src/lib/libc/aarch64/string/strlcat.c
 *   hbsd/src/lib/libc/aarch64/string/strpbrk.c
 *
 * hbsd/src/lib/libc/aarch64/string/memcpy_resolver.c and memmove_resolver.c
 * are not ported; see skipped.txt.
 *
 * The two ported bodies are faithful transliterations: same evaluation order,
 * same signedness, same pointer arithmetic, same behaviour (including the
 * unterminated-dst return of dstsize + strlen(src)).
 *
 * The assembly helpers that the originals only declare (__memchr_aarch64,
 * __strlcpy, __strcspn) are provided in namespace ext with exactly the
 * semantics of the routines they stand for.
 */

module;

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Robert Clausecker
 */

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

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2026 Arm Ltd
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

#include <cstddef>
#include <cstring>

#undef strlcat	/* _FORTIFY_SOURCE */
#undef strlcpy
#undef strpbrk

export module pbsd.lib.libc.aarch64.string.b0071;

export namespace pbsd::lib_libc_aarch64_string::b0071 {

/*
 * The assembly routines the originals declare but do not define.
 */
namespace ext {

/* stands for __memchr_aarch64: the aarch64 assembly memchr(3) */
void *
memchr_aarch64(const void *s, int c, std::size_t n)
{
	return (const_cast<void *>(std::memchr(s, c, n)));
}

/* stands for __strcspn: the aarch64 assembly strcspn(3) */
std::size_t
strcspn(const char *s, const char *charset)
{
	return (std::strcspn(s, charset));
}

/* stands for __strlcpy: libc strlcpy(3), lib/libc/string/strlcpy.c */
std::size_t
strlcpy(char *__restrict dst, const char *__restrict src, std::size_t dsize)
{
	const char *osrc = src;
	std::size_t nleft = dsize;

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

} /* namespace ext */

std::size_t
strlcat(char *__restrict dst, const char *__restrict src, std::size_t dstsize)
{
	char *loc = static_cast<char *>(ext::memchr_aarch64(dst, '\0', dstsize));

	if (loc != nullptr) {
		std::size_t dstlen = static_cast<std::size_t>(loc - dst);

		return (dstlen + ext::strlcpy(loc, src, dstsize - dstlen));
	} else
		return (dstsize + std::strlen(src));
}

char *
strpbrk(const char *s, const char *charset)
{
	std::size_t loc;

	loc = ext::strcspn(s, charset);

	return (s[loc] == '\0' ? nullptr : const_cast<char *>(&s[loc]));
}

} /* namespace pbsd::lib_libc_aarch64_string::b0071 */
