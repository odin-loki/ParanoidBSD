/*
 * PBSD batch b0112 -- C++23 port of:
 *   hbsd/src/lib/libc/aarch64/string/bzero.c
 *   hbsd/src/lib/libc/aarch64/string/bcopy.c
 *   hbsd/src/lib/libc/aarch64/string/strcat.c
 *   hbsd/src/lib/libc/aarch64/string/strncat.c
 */

module;

/*-
 * Public domain.
 */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2024 Getz Mikalsen <getz@FreeBSD.org>
*/

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Robert Clausecker
 */

#include <cstddef>
#include <cstring>

#undef bzero	/* _FORTIFY_SOURCE */
#undef bcopy
#undef strcat
#undef strncat

export module pbsd.lib.libc.aarch64.string.b0112;

export namespace pbsd::lib_libc_aarch64_string::b0112 {

/*
 * The routines the originals call but do not define in these translation units.
 */
namespace ext {

char *
stpcpy(char *dst, const char *src)
{
	while ((*dst++ = *src++) != '\0')
		;
	return (dst - 1);
}

void *
memccpy(void *__restrict dst, const void *__restrict src, int c, std::size_t n)
{
	unsigned char *d = static_cast<unsigned char *>(dst);
	const unsigned char *s = static_cast<const unsigned char *>(src);
	unsigned char ch = static_cast<unsigned char>(c);

	while (n-- > 0) {
		if ((*d++ = *s++) == ch)
			return (d);
	}
	return (nullptr);
}

} /* namespace ext */

void
bzero(void *b, std::size_t len)
{

	std::memset(b, 0, len);
}

void
bcopy(const void *src, void *dst, std::size_t len)
{

	std::memmove(dst, src, len);
}

char *
strcat(char * __restrict s, const char * __restrict append)
{
	char *save = s;

	/* call into SIMD optimized functions */
	ext::stpcpy(s + std::strlen(s), append);

	return(save);
}

char *
strncat(char *dest, const char *src, std::size_t n)
{
	std::size_t len;
	char *endptr;

	len = std::strlen(dest);
	endptr = static_cast<char *>(ext::memccpy(dest + len, src, '\0', n));

	/* avoid an extra branch */
	if (endptr == nullptr)
		endptr = dest + len + n + 1;

	endptr[-1] = '\0';

	return (dest);
}

} /* namespace pbsd::lib_libc_aarch64_string::b0112 */
