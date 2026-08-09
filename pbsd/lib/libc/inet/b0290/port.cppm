module;

#define _DEFAULT_SOURCE

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>

#include <cassert>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>

export module pbsd.lib.libc.inet.b0290;

export namespace pbsd::lib_libc_inet::b0290 {

typedef unsigned char u_char;
typedef unsigned int u_int;

#ifdef SPRINTF_CHAR
# define SPRINTF(x) strlen(sprintf/**/x)
#else
# define SPRINTF(x) ((size_t)sprintf x)
#endif

/*-
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1996-1999 by Internet Software Consortium.
 */

static const char *inet_ntop4(const u_char *src, char *dst, socklen_t size);
static const char *inet_ntop6(const u_char *src, char *dst, socklen_t size);

const char *
inet_ntop(int af, const void * __restrict src, char * __restrict dst,
    socklen_t size)
{
	switch (af) {
	case AF_INET:
		return (inet_ntop4((const u_char *)src, dst, size));
	case AF_INET6:
		return (inet_ntop6((const u_char *)src, dst, size));
	default:
		errno = EAFNOSUPPORT;
		return (NULL);
	}
}

static const char *
inet_ntop4(const u_char *src, char *dst, socklen_t size)
{
	static const char fmt[] = "%u.%u.%u.%u";
	char tmp[sizeof "255.255.255.255"];
	int l;

	l = snprintf(tmp, sizeof(tmp), fmt, src[0], src[1], src[2], src[3]);
	if (l <= 0 || (socklen_t) l >= size) {
		errno = ENOSPC;
		return (NULL);
	}
	strlcpy(dst, tmp, size);
	return (dst);
}

static const char *
inet_ntop6(const u_char *src, char *dst, socklen_t size)
{
	char tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"], *tp;
	struct { int base, len; } best, cur;
	u_int words[NS_IN6ADDRSZ / NS_INT16SZ];
	int i;

	memset(words, '\0', sizeof words);
	for (i = 0; i < NS_IN6ADDRSZ; i++)
		words[i / 2] |= (src[i] << ((1 - (i % 2)) << 3));
	best.base = -1;
	best.len = 0;
	cur.base = -1;
	cur.len = 0;
	for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
		if (words[i] == 0) {
			if (cur.base == -1)
				cur.base = i, cur.len = 1;
			else
				cur.len++;
		} else {
			if (cur.base != -1) {
				if (best.base == -1 || cur.len > best.len)
					best = cur;
				cur.base = -1;
			}
		}
	}
	if (cur.base != -1) {
		if (best.base == -1 || cur.len > best.len)
			best = cur;
	}
	if (best.base != -1 && best.len < 2)
		best.base = -1;

	tp = tmp;
	for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
		if (best.base != -1 && i >= best.base &&
		    i < (best.base + best.len)) {
			if (i == best.base)
				*tp++ = ':';
			continue;
		}
		if (i != 0)
			*tp++ = ':';
		if (i == 6 && best.base == 0 && (best.len == 6 ||
		    (best.len == 7 && words[7] != 0x0001) ||
		    (best.len == 5 && words[5] == 0xffff))) {
			if (!inet_ntop4(src+12, tp, sizeof tmp - (tp - tmp))) {
				errno = ENOSPC;
				return (NULL);
			}
			tp += strlen(tp);
			break;
		}
		tp += sprintf(tp, "%x", words[i]);
	}
	if (best.base != -1 && (best.base + best.len) ==
	    (NS_IN6ADDRSZ / NS_INT16SZ))
		*tp++ = ':';
	*tp++ = '\0';

	if ((socklen_t)(tp - tmp) > size) {
		errno = ENOSPC;
		return (NULL);
	}
	strcpy(dst, tmp);
	return (dst);
}

/*-
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1996,1999 by Internet Software Consortium.
 */

static int	inet_pton4(const char *src, u_char *dst);
static int	inet_pton6(const char *src, u_char *dst);

int
inet_pton(int af, const char * __restrict src, void * __restrict dst)
{
	switch (af) {
	case AF_INET:
		return (inet_pton4(src, (u_char *)dst));
	case AF_INET6:
		return (inet_pton6(src, (u_char *)dst));
	default:
		errno = EAFNOSUPPORT;
		return (-1);
	}
}

static int
inet_pton4(const char *src, u_char *dst)
{
	static const char digits[] = "0123456789";
	int saw_digit, octets, ch;
	u_char tmp[NS_INADDRSZ], *tp;

	saw_digit = 0;
	octets = 0;
	*(tp = tmp) = 0;
	while ((ch = *src++) != '\0') {
		const char *pch;

		if ((pch = strchr(digits, ch)) != NULL) {
			u_int new_val = *tp * 10 + (pch - digits);

			if (saw_digit && *tp == 0)
				return (0);
			if (new_val > 255)
				return (0);
			*tp = new_val;
			if (!saw_digit) {
				if (++octets > 4)
					return (0);
				saw_digit = 1;
			}
		} else if (ch == '.' && saw_digit) {
			if (octets == 4)
				return (0);
			*++tp = 0;
			saw_digit = 0;
		} else
			return (0);
	}
	if (octets < 4)
		return (0);
	memcpy(dst, tmp, NS_INADDRSZ);
	return (1);
}

static int
inet_pton6(const char *src, u_char *dst)
{
	static const char xdigits_l[] = "0123456789abcdef",
			  xdigits_u[] = "0123456789ABCDEF";
	u_char tmp[NS_IN6ADDRSZ], *tp, *endp, *colonp;
	const char *xdigits, *curtok;
	int ch, seen_xdigits;
	u_int val;

	memset((tp = tmp), '\0', NS_IN6ADDRSZ);
	endp = tp + NS_IN6ADDRSZ;
	colonp = NULL;
	if (*src == ':')
		if (*++src != ':')
			return (0);
	curtok = src;
	seen_xdigits = 0;
	val = 0;
	while ((ch = *src++) != '\0') {
		const char *pch;

		if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
			pch = strchr((xdigits = xdigits_u), ch);
		if (pch != NULL) {
			val <<= 4;
			val |= (pch - xdigits);
			if (++seen_xdigits > 4)
				return (0);
			continue;
		}
		if (ch == ':') {
			curtok = src;
			if (!seen_xdigits) {
				if (colonp)
					return (0);
				colonp = tp;
				continue;
			} else if (*src == '\0') {
				return (0);
			}
			if (tp + NS_INT16SZ > endp)
				return (0);
			*tp++ = (u_char) (val >> 8) & 0xff;
			*tp++ = (u_char) val & 0xff;
			seen_xdigits = 0;
			val = 0;
			continue;
		}
		if (ch == '.' && ((tp + NS_INADDRSZ) <= endp) &&
		    inet_pton4(curtok, tp) > 0) {
			tp += NS_INADDRSZ;
			seen_xdigits = 0;
			break;
		}
		return (0);
	}
	if (seen_xdigits) {
		if (tp + NS_INT16SZ > endp)
			return (0);
		*tp++ = (u_char) (val >> 8) & 0xff;
		*tp++ = (u_char) val & 0xff;
	}
	if (colonp != NULL) {
		const int n = tp - colonp;
		int i;

		if (tp == endp)
			return (0);
		for (i = 1; i <= n; i++) {
			endp[- i] = colonp[n - i];
			colonp[n - i] = 0;
		}
		tp = endp;
	}
	if (tp != endp)
		return (0);
	memcpy(dst, tmp, NS_IN6ADDRSZ);
	return (1);
}

/*-
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1998,1999 by Internet Software Consortium.
 */

static int	inet_cidr_pton_ipv4 (const char *src, u_char *dst, int *bits,
				     int ipv6);
static int	inet_cidr_pton_ipv6 (const char *src, u_char *dst, int *bits);
static int	getbits(const char *, int ipv6);

int
inet_cidr_pton(int af, const char *src, void *dst, int *bits) {
	switch (af) {
	case AF_INET:
		return (inet_cidr_pton_ipv4(src, (u_char *)dst, bits, 0));
	case AF_INET6:
		return (inet_cidr_pton_ipv6(src, (u_char *)dst, bits));
	default:
		errno = EAFNOSUPPORT;
		return (-1);
	}
}

static const char digits[] = "0123456789";

static int
inet_cidr_pton_ipv4(const char *src, u_char *dst, int *pbits, int ipv6) {
	const u_char *odst = dst;
	int n, ch, tmp, bits;
	size_t size = 4;

	while (ch = *src++, (isascii(ch) && isdigit(ch))) {
		tmp = 0;
		do {
			n = strchr(digits, ch) - digits;
			assert(n >= 0 && n <= 9);
			tmp *= 10;
			tmp += n;
			if (tmp > 255)
				goto enoent;
		} while ((ch = *src++) != '\0' && isascii(ch) && isdigit(ch));
		if (size-- == 0U)
			goto emsgsize;
		*dst++ = (u_char) tmp;
		if (ch == '\0' || ch == '/')
			break;
		if (ch != '.')
			goto enoent;
	}

	bits = -1;
	if (ch == '/' && dst > odst) {
		bits = getbits(src, ipv6);
		if (bits == -2)
			goto enoent;
	} else if (ch != '\0')
		goto enoent;

	if (bits == -1) {
		if (dst - odst == 4)
			bits = ipv6 ? 128 : 32;
		else
			goto enoent;
	}

	if (dst == odst)
		goto enoent;

	if (((bits - (ipv6 ? 96 : 0)) / 8) > (dst - odst))
		goto enoent;

	while (size-- > 0U)
		*dst++ = 0;

	*pbits = bits;
	return (0);

 enoent:
	errno = ENOENT;
	return (-1);

 emsgsize:
	errno = EMSGSIZE;
	return (-1);
}

static int
inet_cidr_pton_ipv6(const char *src, u_char *dst, int *pbits) {
	static const char xdigits_l[] = "0123456789abcdef",
			  xdigits_u[] = "0123456789ABCDEF";
	u_char tmp[NS_IN6ADDRSZ], *tp, *endp, *colonp;
	const char *xdigits, *curtok;
	int ch, saw_xdigit;
	u_int val;
	int bits;

	memset((tp = tmp), '\0', NS_IN6ADDRSZ);
	endp = tp + NS_IN6ADDRSZ;
	colonp = NULL;
	if (*src == ':')
		if (*++src != ':')
			return (0);
	curtok = src;
	saw_xdigit = 0;
	val = 0;
	bits = -1;
	while ((ch = *src++) != '\0') {
		const char *pch;

		if ((pch = strchr((xdigits = xdigits_l), ch)) == NULL)
			pch = strchr((xdigits = xdigits_u), ch);
		if (pch != NULL) {
			val <<= 4;
			val |= (pch - xdigits);
			if (val > 0xffff)
				return (0);
			saw_xdigit = 1;
			continue;
		}
		if (ch == ':') {
			curtok = src;
			if (!saw_xdigit) {
				if (colonp)
					return (0);
				colonp = tp;
				continue;
			} else if (*src == '\0') {
				return (0);
			}
			if (tp + NS_INT16SZ > endp)
				return (0);
			*tp++ = (u_char) (val >> 8) & 0xff;
			*tp++ = (u_char) val & 0xff;
			saw_xdigit = 0;
			val = 0;
			continue;
		}
		if (ch == '.' && ((tp + NS_INADDRSZ) <= endp) &&
		    inet_cidr_pton_ipv4(curtok, tp, &bits, 1) == 0) {
			tp += NS_INADDRSZ;
			saw_xdigit = 0;
			break;
		}
		if (ch == '/') {
			bits = getbits(src, 1);
			if (bits == -2)
				goto enoent;
			break;
		}
		goto enoent;
	}
	if (saw_xdigit) {
		if (tp + NS_INT16SZ > endp)
			goto emsgsize;
		*tp++ = (u_char) (val >> 8) & 0xff;
		*tp++ = (u_char) val & 0xff;
	}
	if (colonp != NULL) {
		const int n = tp - colonp;
		int i;

		if (tp == endp)
			goto enoent;
		for (i = 1; i <= n; i++) {
			endp[- i] = colonp[n - i];
			colonp[n - i] = 0;
		}
		tp = endp;
	}

	memcpy(dst, tmp, NS_IN6ADDRSZ);

	*pbits = bits;
	return (0);

 enoent:
	errno = ENOENT;
	return (-1);

 emsgsize:
	errno = EMSGSIZE;
	return (-1);
}

static int
getbits(const char *src, int ipv6) {
	int bits = 0;
	char *cp, ch;

	if (*src == '\0')
		return (-2);
	do {
		ch = *src++;
		cp = (char *)strchr(digits, ch);
		if (cp == NULL)
			return (-2);
		bits *= 10;
		bits += cp - digits;
		if (bits == 0 && *src != '\0')
			return (-2);
		if (bits > (ipv6 ? 128 : 32))
			return (-2);
	} while (*src != '\0');

	return (bits);
}

/*-
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1998,1999 by Internet Software Consortium.
 */

static char *
inet_cidr_ntop_ipv4(const u_char *src, int bits, char *dst, size_t size);
static char *
inet_cidr_ntop_ipv6(const u_char *src, int bits, char *dst, size_t size);
static int decoct(const u_char *src, int bytes, char *dst, size_t size);

char *
inet_cidr_ntop(int af, const void *src, int bits, char *dst, size_t size) {
	switch (af) {
	case AF_INET:
		return (inet_cidr_ntop_ipv4((const u_char *)src, bits, dst, size));
	case AF_INET6:
		return (inet_cidr_ntop_ipv6((const u_char *)src, bits, dst, size));
	default:
		errno = EAFNOSUPPORT;
		return (NULL);
	}
}

static int
decoct(const u_char *src, int bytes, char *dst, size_t size) {
	char *odst = dst;
	char *t;
	int b;

	for (b = 1; b <= bytes; b++) {
		if (size < sizeof "255.")
			return (0);
		t = dst;
		dst += SPRINTF((dst, "%u", *src++));
		if (b != bytes) {
			*dst++ = '.';
			*dst = '\0';
		}
		size -= (size_t)(dst - t);
	}
	return (dst - odst);
}

static char *
inet_cidr_ntop_ipv4(const u_char *src, int bits, char *dst, size_t size) {
	char *odst = dst;
	size_t len = 4;
	size_t b;
	size_t bytes;

	if ((bits < -1) || (bits > 32)) {
		errno = EINVAL;
		return (NULL);
	}

	if (bits == -1)
		len = 4;
	else
		for (len = 1, b = 1 ; b < 4U; b++)
			if (*(src + b))
				len = b + 1;

	bytes = (((bits <= 0) ? 1 : bits) + 7) / 8;
	if (len > bytes)
		bytes = len;
	b = decoct(src, bytes, dst, size);
	if (b == 0U)
		goto emsgsize;
	dst += b;
	size -= b;

	if (bits != -1) {
		if (size < sizeof "/32")
			goto emsgsize;
		dst += SPRINTF((dst, "/%u", bits));
	}

	return (odst);

 emsgsize:
	errno = EMSGSIZE;
	return (NULL);
}

static char *
inet_cidr_ntop_ipv6(const u_char *src, int bits, char *dst, size_t size) {
	char tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255/128"];
	char *tp;
	struct { int base, len; } best, cur;
	u_int words[NS_IN6ADDRSZ / NS_INT16SZ];
	int i;

	if ((bits < -1) || (bits > 128)) {
		errno = EINVAL;
		return (NULL);
	}

	memset(words, '\0', sizeof words);
	for (i = 0; i < NS_IN6ADDRSZ; i++)
		words[i / 2] |= (src[i] << ((1 - (i % 2)) << 3));
	best.base = -1;
	best.len = 0;
	cur.base = -1;
	cur.len = 0;
	for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
		if (words[i] == 0) {
			if (cur.base == -1)
				cur.base = i, cur.len = 1;
			else
				cur.len++;
		} else {
			if (cur.base != -1) {
				if (best.base == -1 || cur.len > best.len)
					best = cur;
				cur.base = -1;
			}
		}
	}
	if (cur.base != -1) {
		if (best.base == -1 || cur.len > best.len)
			best = cur;
	}
	if (best.base != -1 && best.len < 2)
		best.base = -1;

	tp = tmp;
	for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
		if (best.base != -1 && i >= best.base &&
		    i < (best.base + best.len)) {
			if (i == best.base)
				*tp++ = ':';
			continue;
		}
		if (i != 0)
			*tp++ = ':';
		if (i == 6 && best.base == 0 && (best.len == 6 ||
		    (best.len == 7 && words[7] != 0x0001) ||
		    (best.len == 5 && words[5] == 0xffff))) {
			int n;

			if (src[15] || bits == -1 || bits > 120)
				n = 4;
			else if (src[14] || bits > 112)
				n = 3;
			else
				n = 2;
			n = decoct(src+12, n, tp, sizeof tmp - (tp - tmp));
			if (n == 0) {
				errno = EMSGSIZE;
				return (NULL);
			}
			tp += strlen(tp);
			break;
		}
		tp += SPRINTF((tp, "%x", words[i]));
	}

	if (best.base != -1 && (best.base + best.len) ==
	    (NS_IN6ADDRSZ / NS_INT16SZ))
		*tp++ = ':';
	*tp = '\0';

	if (bits != -1)
		tp += SPRINTF((tp, "/%u", bits));

	if ((size_t)(tp - tmp) > size) {
		errno = EMSGSIZE;
		return (NULL);
	}
	strcpy(dst, tmp);
	return (dst);
}

} // namespace
