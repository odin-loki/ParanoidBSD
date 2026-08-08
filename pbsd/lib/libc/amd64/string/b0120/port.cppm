/*
 * PBSD batch b0120 -- C++23 port of:
 *   hbsd/src/lib/libc/amd64/string/bzero.c
 *   hbsd/src/lib/libc/amd64/string/bcopy.c
 *   hbsd/src/lib/libc/amd64/string/strncat.c
 *   hbsd/src/lib/libc/amd64/string/strlcat.c
 *
 * The original copyright notices of each source file are reproduced
 * immediately above the function they belong to.
 */

module;

#include <cstddef>
#include <cstring>
#include <string.h>

#undef bzero	/* _FORTIFY_SOURCE */
#undef bcopy
#undef strncat	/* _FORTIFY_SOURCE */
#undef strlcat	/* FORTIFY_SOURCE */

export module pbsd.lib.libc.amd64.string.b0120;

namespace pbsd::lib_libc_amd64_string::b0120 {

/*
 * The assembly routines the originals declare but do not define in these
 * translation units.
 */
void *
__memccpy(void * __restrict dst, const void * __restrict src, int c, std::size_t n)
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

void *
__memchr(const void *s, int c, std::size_t n)
{
	return (const_cast<void *>(std::memchr(s, c, n)));
}

std::size_t
__strlcpy(char * __restrict dst, const char * __restrict src, std::size_t dsize)
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

} /* namespace pbsd::lib_libc_amd64_string::b0120 */

export namespace pbsd::lib_libc_amd64_string::b0120 {

/*-
 * Public domain.
 */

void
bzero(void *b, std::size_t len)
{

	std::memset(b, 0, len);
}

/*-
 * Public domain.
 */

void
bcopy(const void *src, void *dst, std::size_t len)
{

	std::memmove(dst, src, len);
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Robert Clausecker
 */

char *
strncat(char *dest, const char *src, std::size_t n)
{
	std::size_t len;
	char *endptr;

	len = std::strlen(dest);
	endptr = static_cast<char *>(__memccpy(dest + len, src, '\0', n));

	/* avoid an extra branch */
	if (endptr == nullptr)
		endptr = dest + len + n + 1;

	endptr[-1] = '\0';

	return (dest);
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Robert Clausecker
 */

std::size_t
strlcat(char * __restrict dst, const char * __restrict src, std::size_t dstsize)
{
	char *loc = static_cast<char *>(__memchr(dst, '\0', dstsize));

	if (loc != nullptr) {
		std::size_t dstlen = static_cast<std::size_t>(loc - dst);

		return (dstlen + __strlcpy(loc, src, dstsize - dstlen));
	} else
		return (dstsize + std::strlen(src));
}

} /* namespace pbsd::lib_libc_amd64_string::b0120 */
