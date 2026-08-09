// b0154 — C++23 port of the HardenedBSD libc nameser helpers.
//
// Sources ported in this module:
//	lib/libc/nameser/ns_netint.c
//	lib/libc/nameser/ns_ttl.c
//	lib/libc/nameser/ns_samedomain.c
//	lib/libc/nameser/ns_parse.c	(ns_parserr2 omitted, see skipped.txt)
//
// The port is behaviour-preserving down to integer signedness, evaluation
// order, truncating assignments and pointer arithmetic.  Nothing is fixed,
// tidied or hardened.

module;

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1
#endif

#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/nameser.h>

#include <ctype.h>
#include <errno.h>
#include <resolv.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

export module pbsd.lib.libc.nameser.b0154;

export namespace pbsd::lib_libc_nameser::b0154 {

/* ---------------------------------------------------------------------- */
/* lib/libc/nameser/ns_netint.c						  */
/* ---------------------------------------------------------------------- */

/*-
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1996,1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* Public. */

u_int
ns_get16(const u_char *src) {
	u_int dst;

	{
		const u_char *t_cp = (const u_char *)(src);
		dst = ((uint16_t)t_cp[0] << 8)
		    | ((uint16_t)t_cp[1])
		    ;
		src += NS_INT16SZ;
	}
	return (dst);
}

u_long
ns_get32(const u_char *src) {
	u_long dst;

	{
		const u_char *t_cp = (const u_char *)(src);
		dst = ((uint32_t)t_cp[0] << 24)
		    | ((uint32_t)t_cp[1] << 16)
		    | ((uint32_t)t_cp[2] << 8)
		    | ((uint32_t)t_cp[3])
		    ;
		src += NS_INT32SZ;
	}
	return (dst);
}

void
ns_put16(u_int src, u_char *dst) {
	uint16_t t_s = (uint16_t)(src);
	u_char *t_cp = (u_char *)(dst);
	*t_cp++ = t_s >> 8;
	*t_cp   = t_s;
	dst += NS_INT16SZ;
}

void
ns_put32(u_long src, u_char *dst) {
	uint32_t t_l = (uint32_t)(src);
	u_char *t_cp = (u_char *)(dst);
	*t_cp++ = t_l >> 24;
	*t_cp++ = t_l >> 16;
	*t_cp++ = t_l >> 8;
	*t_cp   = t_l;
	dst += NS_INT32SZ;
}

/*! \file */

/* ---------------------------------------------------------------------- */
/* lib/libc/nameser/ns_ttl.c						  */
/* ---------------------------------------------------------------------- */

/*-
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1996,1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* Forward. */

int	fmt1(int t, char s, char **buf, size_t *buflen);

/* Public. */

int
ns_format_ttl(u_long src, char *dst, size_t dstlen) {
	char *odst = dst;
	int secs, mins, hours, days, weeks, x;
	char *p;

	secs = src % 60;   src /= 60;
	mins = src % 60;   src /= 60;
	hours = src % 24;  src /= 24;
	days = src % 7;    src /= 7;
	weeks = src;       src = 0;

	x = 0;
	if (weeks) {
		if ((fmt1(weeks, 'W', &dst, &dstlen)) < 0)
			return (-1);
		x++;
	}
	if (days) {
		if ((fmt1(days, 'D', &dst, &dstlen)) < 0)
			return (-1);
		x++;
	}
	if (hours) {
		if ((fmt1(hours, 'H', &dst, &dstlen)) < 0)
			return (-1);
		x++;
	}
	if (mins) {
		if ((fmt1(mins, 'M', &dst, &dstlen)) < 0)
			return (-1);
		x++;
	}
	if (secs || !(weeks || days || hours || mins)) {
		if ((fmt1(secs, 'S', &dst, &dstlen)) < 0)
			return (-1);
		x++;
	}

	if (x > 1) {
		int ch;

		for (p = odst; (ch = *p) != '\0'; p++)
			if (isascii(ch) && isupper(ch))
				*p = tolower(ch);
	}

	return (dst - odst);
}

int
ns_parse_ttl(const char *src, u_long *dst) {
	u_long ttl, tmp;
	int ch, digits, dirty;

	ttl = 0;
	tmp = 0;
	digits = 0;
	dirty = 0;
	while ((ch = *src++) != '\0') {
		if (!isascii(ch) || !isprint(ch))
			goto einval;
		if (isdigit(ch)) {
			tmp *= 10;
			tmp += (ch - '0');
			digits++;
			continue;
		}
		if (digits == 0)
			goto einval;
		if (islower(ch))
			ch = toupper(ch);
		switch (ch) {
		case 'W':  tmp *= 7;
		case 'D':  tmp *= 24;
		case 'H':  tmp *= 60;
		case 'M':  tmp *= 60;
		case 'S':  break;
		default:   goto einval;
		}
		ttl += tmp;
		tmp = 0;
		digits = 0;
		dirty = 1;
	}
	if (digits > 0) {
		if (dirty)
			goto einval;
		else
			ttl += tmp;
	} else if (!dirty)
		goto einval;
	*dst = ttl;
	return (0);

 einval:
	errno = EINVAL;
	return (-1);
}

/* Private. */

int
fmt1(int t, char s, char **buf, size_t *buflen) {
	char tmp[50];
	size_t len;

	len = ((size_t)sprintf(tmp, "%d%c", t, s));
	if (len + 1 > *buflen)
		return (-1);
	strcpy(*buf, tmp);
	*buf += len;
	*buflen -= len;
	return (0);
}

/*! \file */

/* ---------------------------------------------------------------------- */
/* lib/libc/nameser/ns_samedomain.c					  */
/* ---------------------------------------------------------------------- */

/*-
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1995,1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* Forward. */

int	ns_samename(const char *a, const char *b);

/*%
 *	Check whether a name belongs to a domain.
 *
 * Inputs:
 *\li	a - the domain whose ancestry is being verified
 *\li	b - the potential ancestor we're checking against
 *
 * Return:
 *\li	boolean - is a at or below b?
 *
 * Notes:
 *\li	Trailing dots are first removed from name and domain.
 *	Always compare complete subdomains, not only whether the
 *	domain name is the trailing string of the given name.
 *
 *\li	"host.foobar.top" lies in "foobar.top" and in "top" and in ""
 *	but NOT in "bar.top"
 */

int
ns_samedomain(const char *a, const char *b) {
	size_t la, lb;
	int diff, i, escaped;
	const char *cp;

	la = strlen(a);
	lb = strlen(b);

	/* Ignore a trailing label separator (i.e. an unescaped dot) in 'a'. */
	if (la != 0U && a[la - 1] == '.') {
		escaped = 0;
		/* Note this loop doesn't get executed if la==1. */
		for (i = la - 2; i >= 0; i--)
			if (a[i] == '\\') {
				if (escaped)
					escaped = 0;
				else
					escaped = 1;
			} else
				break;
		if (!escaped)
			la--;
	}

	/* Ignore a trailing label separator (i.e. an unescaped dot) in 'b'. */
	if (lb != 0U && b[lb - 1] == '.') {
		escaped = 0;
		/* note this loop doesn't get executed if lb==1 */
		for (i = lb - 2; i >= 0; i--)
			if (b[i] == '\\') {
				if (escaped)
					escaped = 0;
				else
					escaped = 1;
			} else
				break;
		if (!escaped)
			lb--;
	}

	/* lb == 0 means 'b' is the root domain, so 'a' must be in 'b'. */
	if (lb == 0U)
		return (1);

	/* 'b' longer than 'a' means 'a' can't be in 'b'. */
	if (lb > la)
		return (0);

	/* 'a' and 'b' being equal at this point indicates sameness. */
	if (lb == la)
		return (strncasecmp(a, b, lb) == 0);

	/* Ok, we know la > lb. */

	diff = la - lb;

	/*
	 * If 'a' is only 1 character longer than 'b', then it can't be
	 * a subdomain of 'b' (because of the need for the '.' label
	 * separator).
	 */
	if (diff < 2)
		return (0);

	/*
	 * If the character before the last 'lb' characters of 'b'
	 * isn't '.', then it can't be a match (this lets us avoid
	 * having "foobar.com" match "bar.com").
	 */
	if (a[diff - 1] != '.')
		return (0);

	/*
	 * We're not sure about that '.', however.  It could be escaped
         * and thus not a really a label separator.
	 */
	escaped = 0;
	for (i = diff - 2; i >= 0; i--)
		if (a[i] == '\\') {
			if (escaped)
				escaped = 0;
			else
				escaped = 1;
		} else
			break;
	if (escaped)
		return (0);
	  
	/* Now compare aligned trailing substring. */
	cp = a + diff;
	return (strncasecmp(cp, b, lb) == 0);
}

/*%
 *	is "a" a subdomain of "b"?
 *
 * Note: guarded by #ifndef _LIBC upstream; kept here so the batch covers
 * every function in the source file.
 */
int
ns_subdomain(const char *a, const char *b) {
	return (ns_samename(a, b) != 1 && ns_samedomain(a, b));
}

/*%
 *	make a canonical copy of domain name "src"
 *
 * notes:
 * \code
 *	foo -> foo.
 *	foo. -> foo.
 *	foo.. -> foo.
 *	foo\. -> foo\..
 *	foo\\. -> foo\\.
 * \endcode
 */

int
ns_makecanon(const char *src, char *dst, size_t dstsize) {
	size_t n = strlen(src);

	if (n + sizeof "." > dstsize) {			/*%< Note: sizeof == 2 */
		errno = EMSGSIZE;
		return (-1);
	}
	strcpy(dst, src);
	while (n >= 1U && dst[n - 1] == '.')		/*%< Ends in "." */
		if (n >= 2U && dst[n - 2] == '\\' &&	/*%< Ends in "\." */
		    (n < 3U || dst[n - 3] != '\\'))	/*%< But not "\\." */
			break;
		else
			dst[--n] = '\0';
	dst[n++] = '.';
	dst[n] = '\0';
	return (0);
}

/*%
 *	determine whether domain name "a" is the same as domain name "b"
 *
 * return:
 *\li	-1 on error
 *\li	0 if names differ
 *\li	1 if names are the same
 */

int
ns_samename(const char *a, const char *b) {
	char ta[NS_MAXDNAME], tb[NS_MAXDNAME];

	if (ns_makecanon(a, ta, sizeof ta) < 0 ||
	    ns_makecanon(b, tb, sizeof tb) < 0)
		return (-1);
	if (strcasecmp(ta, tb) == 0)
		return (1);
	else
		return (0);
}

/*! \file */

/* ---------------------------------------------------------------------- */
/* lib/libc/nameser/ns_parse.c						  */
/* ---------------------------------------------------------------------- */

/*-
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1996,1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* Forward. */

void	setsection(ns_msg *msg, ns_sect sect);

/* Public. */

using ns_flagdata = struct ::_ns_flagdata;

/* These need to be in the same order as the nres.h:ns_flag enum. */
ns_flagdata _ns_flagdata[16] = {
	{ 0x8000, 15 },		/*%< qr. */
	{ 0x7800, 11 },		/*%< opcode. */
	{ 0x0400, 10 },		/*%< aa. */
	{ 0x0200, 9 },		/*%< tc. */
	{ 0x0100, 8 },		/*%< rd. */
	{ 0x0080, 7 },		/*%< ra. */
	{ 0x0040, 6 },		/*%< z. */
	{ 0x0020, 5 },		/*%< ad. */
	{ 0x0010, 4 },		/*%< cd. */
	{ 0x000f, 0 },		/*%< rcode. */
	{ 0x0000, 0 },		/*%< expansion (1/6). */
	{ 0x0000, 0 },		/*%< expansion (2/6). */
	{ 0x0000, 0 },		/*%< expansion (3/6). */
	{ 0x0000, 0 },		/*%< expansion (4/6). */
	{ 0x0000, 0 },		/*%< expansion (5/6). */
	{ 0x0000, 0 },		/*%< expansion (6/6). */
};

int ns_msg_getflag(ns_msg handle, int flag) {
	return(((handle)._flags & _ns_flagdata[flag].mask) >> _ns_flagdata[flag].shift);
}

int
ns_skiprr(const u_char *ptr, const u_char *eom, ns_sect section, int count) {
	const u_char *optr = ptr;

	for ((void)NULL; count > 0; count--) {
		int b, rdlength;

		b = dn_skipname(ptr, eom);
		if (b < 0) {
			errno = EMSGSIZE;
			return (-1);
		}
		ptr += b/*Name*/ + NS_INT16SZ/*Type*/ + NS_INT16SZ/*Class*/;
		if (section != ns_s_qd) {
			if (ptr + NS_INT32SZ + NS_INT16SZ > eom) {
				errno = EMSGSIZE;
				return (-1);
			}
			ptr += NS_INT32SZ/*TTL*/;
			{
				const u_char *t_cp = (const u_char *)(ptr);
				rdlength = ((uint16_t)t_cp[0] << 8)
					 | ((uint16_t)t_cp[1])
					 ;
				ptr += NS_INT16SZ;
			}
			ptr += rdlength/*RData*/;
		}
	}
	if (ptr > eom) {
		errno = EMSGSIZE;
		return (-1);
	}
	return (ptr - optr);
}

int
ns_initparse(const u_char *msg, int msglen, ns_msg *handle) {
	const u_char *eom = msg + msglen;
	int i;

	handle->_msg = msg;
	handle->_eom = eom;
	if (msg + NS_INT16SZ > eom) {
		errno = EMSGSIZE;
		return (-1);
	}
	{
		const u_char *t_cp = (const u_char *)(msg);
		handle->_id = ((uint16_t)t_cp[0] << 8)
			    | ((uint16_t)t_cp[1])
			    ;
		msg += NS_INT16SZ;
	}
	if (msg + NS_INT16SZ > eom) {
		errno = EMSGSIZE;
		return (-1);
	}
	{
		const u_char *t_cp = (const u_char *)(msg);
		handle->_flags = ((uint16_t)t_cp[0] << 8)
			       | ((uint16_t)t_cp[1])
			       ;
		msg += NS_INT16SZ;
	}
	for (i = 0; i < ns_s_max; i++) {
		if (msg + NS_INT16SZ > eom) {
			errno = EMSGSIZE;
			return (-1);
		}
		{
			const u_char *t_cp = (const u_char *)(msg);
			handle->_counts[i] = ((uint16_t)t_cp[0] << 8)
					   | ((uint16_t)t_cp[1])
					   ;
			msg += NS_INT16SZ;
		}
	}
	for (i = 0; i < ns_s_max; i++)
		if (handle->_counts[i] == 0)
			handle->_sections[i] = NULL;
		else {
			int b = b0154::ns_skiprr(msg, eom, (ns_sect)i,
					  handle->_counts[i]);

			if (b < 0)
				return (-1);
			handle->_sections[i] = msg;
			msg += b;
		}
	if (msg != eom) {
		errno = EMSGSIZE;
		return (-1);
	}
	setsection(handle, ns_s_max);
	return (0);
}

int
ns_parserr(ns_msg *handle, ns_sect section, int rrnum, ns_rr *rr) {
	int b;
	int tmp;

	/* Make section right. */
	tmp = section;
	if (tmp < 0 || section >= ns_s_max) {
		errno = ENODEV;
		return (-1);
	}
	if (section != handle->_sect)
		setsection(handle, section);

	/* Make rrnum right. */
	if (rrnum == -1)
		rrnum = handle->_rrnum;
	if (rrnum < 0 || rrnum >= handle->_counts[(int)section]) {
		errno = ENODEV;
		return (-1);
	}
	if (rrnum < handle->_rrnum)
		setsection(handle, section);
	if (rrnum > handle->_rrnum) {
		b = b0154::ns_skiprr(handle->_msg_ptr, handle->_eom, section,
			      rrnum - handle->_rrnum);

		if (b < 0)
			return (-1);
		handle->_msg_ptr += b;
		handle->_rrnum = rrnum;
	}

	/* Do the parse. */
	b = dn_expand(handle->_msg, handle->_eom,
		      handle->_msg_ptr, rr->name, NS_MAXDNAME);
	if (b < 0)
		return (-1);
	handle->_msg_ptr += b;
	if (handle->_msg_ptr + NS_INT16SZ + NS_INT16SZ > handle->_eom) {
		errno = EMSGSIZE;
		return (-1);
	}
	{
		const u_char *t_cp = (const u_char *)(handle->_msg_ptr);
		rr->type = ((uint16_t)t_cp[0] << 8)
			 | ((uint16_t)t_cp[1])
			 ;
		handle->_msg_ptr += NS_INT16SZ;
	}
	{
		const u_char *t_cp = (const u_char *)(handle->_msg_ptr);
		rr->rr_class = ((uint16_t)t_cp[0] << 8)
			     | ((uint16_t)t_cp[1])
			     ;
		handle->_msg_ptr += NS_INT16SZ;
	}
	if (section == ns_s_qd) {
		rr->ttl = 0;
		rr->rdlength = 0;
		rr->rdata = NULL;
	} else {
		if (handle->_msg_ptr + NS_INT32SZ + NS_INT16SZ > handle->_eom) {
			errno = EMSGSIZE;
			return (-1);
		}
		{
			const u_char *t_cp = (const u_char *)(handle->_msg_ptr);
			rr->ttl = ((uint32_t)t_cp[0] << 24)
				| ((uint32_t)t_cp[1] << 16)
				| ((uint32_t)t_cp[2] << 8)
				| ((uint32_t)t_cp[3])
				;
			handle->_msg_ptr += NS_INT32SZ;
		}
		{
			const u_char *t_cp = (const u_char *)(handle->_msg_ptr);
			rr->rdlength = ((uint16_t)t_cp[0] << 8)
				     | ((uint16_t)t_cp[1])
				     ;
			handle->_msg_ptr += NS_INT16SZ;
		}
		if (handle->_msg_ptr + rr->rdlength > handle->_eom) {
			errno = EMSGSIZE;
			return (-1);
		}
		rr->rdata = handle->_msg_ptr;
		handle->_msg_ptr += rr->rdlength;
	}
	if (++handle->_rrnum > handle->_counts[(int)section])
		setsection(handle, (ns_sect)((int)section + 1));

	/* All done. */
	return (0);
}

/* Private. */

void
setsection(ns_msg *msg, ns_sect sect) {
	msg->_sect = sect;
	if (sect == ns_s_max) {
		msg->_rrnum = -1;
		msg->_msg_ptr = NULL;
	} else {
		msg->_rrnum = 0;
		msg->_msg_ptr = msg->_sections[(int)sect];
	}
}

/*! \file */

} // namespace pbsd::lib_libc_nameser::b0154
