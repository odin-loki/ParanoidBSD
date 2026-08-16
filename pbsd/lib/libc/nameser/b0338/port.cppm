// b0338 — C++23 port of lib/libc/nameser/ns_name.c and ns_print.c.
// Behaviour-preserving down to integer signedness, evaluation order,
// truncating assignments and pointer arithmetic.

module;

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1
#endif

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <arpa/nameser_compat.h>
#include <arpa/inet.h>

#include <assert.h>
#define _LIBC 1
#define INSIST(cond) assert(cond)

#include <errno.h>
#include <resolv.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>


#ifndef NS_MAXNNAME
#define NS_MAXNNAME	256
#endif
#ifndef NS_MAXCDNAME
#define NS_MAXCDNAME	255
#endif
#ifndef MAXCDNAME
#define MAXCDNAME	NS_MAXCDNAME
#endif
#ifndef NS_MD5RSA_MIN_BITS
#define	NS_MD5RSA_MIN_BITS	 512
#define	NS_MD5RSA_MAX_BITS	4096
#define	NS_MD5RSA_MAX_BYTES	((NS_MD5RSA_MAX_BITS+7/8)*2+3)
#define	NS_MD5RSA_MAX_BASE64	(((NS_MD5RSA_MAX_BYTES+2)/3)*4)
#define NS_MD5RSA_MIN_SIZE	((NS_MD5RSA_MIN_BITS+7)/8)
#define NS_MD5RSA_MAX_SIZE	((NS_MD5RSA_MAX_BITS+7)/8)
#endif
#ifndef NS_NXT_BITS
#define	NS_NXT_BITS 8
#define	NS_NXT_BIT_SET(  n,p) (p[(n)/NS_NXT_BITS] |=  (0x80>>((n)%NS_NXT_BITS)))
#define	NS_NXT_BIT_CLEAR(n,p) (p[(n)/NS_NXT_BITS] &= ~(0x80>>((n)%NS_NXT_BITS)))
#define	NS_NXT_BIT_ISSET(n,p) (p[(n)/NS_NXT_BITS] &   (0x80>>((n)%NS_NXT_BITS)))
#define NS_NXT_MAX 127
#endif

#ifndef ns_nname_ct
typedef u_char ns_nname[NS_MAXNNAME];
typedef const u_char *ns_nname_ct;
typedef u_char *ns_nname_t;
struct ns_namemap { ns_nname_ct base; int len; };
typedef struct ns_namemap *ns_namemap_t;
typedef const struct ns_namemap *ns_namemap_ct;
#endif

char *p_secstodate(u_long);


#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifdef SPRINTF_CHAR
# define SPRINTF(x) strlen(sprintf/**/x)
#else
# define SPRINTF(x) ((size_t)sprintf x)
#endif

#define NS_TYPE_ELT			0x40
#define DNS_LABELTYPE_BITSTRING		0x41

#define T(x) \
	do { \
		if ((x) < 0) \
			return (-1); \
	} while (0)

export module pbsd.lib.libc.nameser.b0338;

export namespace pbsd::lib_libc_nameser::b0338 {

static const char base32hex[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUV=0123456789abcdefghijklmnopqrstuv";

/* -------- ns_name.c -------- */

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


#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/nameser.h>

#include <errno.h>
#include <resolv.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>




/* Data. */

static const char	digits[] = "0123456789";

static const char digitvalue[256] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/*16*/
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*32*/
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*48*/
	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1, /*64*/
	-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*80*/
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*96*/
	-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*112*/
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*128*/
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*256*/
};

/* Forward. */

static int		special(int);
static int		printable(int);
static int		dn_find(const u_char *, const u_char *,
				const u_char * const *,
				const u_char * const *);
static int		encode_bitsring(const char **, const char *,
					unsigned char **, unsigned char **,
					unsigned const char *);
static int		labellen(const u_char *);
static int		decode_bitstring(const unsigned char **,
					 char *, const char *);

/* Public. */

/*%
 *	Convert an encoded domain name to printable ascii as per RFC1035.

 * return:
 *\li	Number of bytes written to buffer, or -1 (with errno set)
 *
 * notes:
 *\li	The root is returned as "."
 *\li	All other domains are returned in non absolute form
 */
int
ns_name_ntop(const u_char *src, char *dst, size_t dstsiz)
{
	const u_char *cp;
	char *dn, *eom;
	u_char c;
	u_int n;
	int l;

	cp = src;
	dn = dst;
	eom = dst + dstsiz;

	while ((n = *cp++) != 0) {
		if ((n & NS_CMPRSFLGS) == NS_CMPRSFLGS) {
			/* Some kind of compression pointer. */
			errno = EMSGSIZE;
			return (-1);
		}
		if (dn != dst) {
			if (dn >= eom) {
				errno = EMSGSIZE;
				return (-1);
			}
			*dn++ = '.';
		}
		if ((l = labellen(cp - 1)) < 0) {
			errno = EMSGSIZE; /*%< XXX */
			return (-1);
		}
		if (dn + l >= eom) {
			errno = EMSGSIZE;
			return (-1);
		}
		if ((n & NS_CMPRSFLGS) == NS_TYPE_ELT) {
			int m;

			if (n != DNS_LABELTYPE_BITSTRING) {
				/* XXX: labellen should reject this case */
				errno = EINVAL;
				return (-1);
			}
			if ((m = decode_bitstring(&cp, dn, eom)) < 0)
			{
				errno = EMSGSIZE;
				return (-1);
			}
			dn += m; 
			continue;
		}
		for ((void)NULL; l > 0; l--) {
			c = *cp++;
			if (special(c)) {
				if (dn + 1 >= eom) {
					errno = EMSGSIZE;
					return (-1);
				}
				*dn++ = '\\';
				*dn++ = (char)c;
			} else if (!printable(c)) {
				if (dn + 3 >= eom) {
					errno = EMSGSIZE;
					return (-1);
				}
				*dn++ = '\\';
				*dn++ = digits[c / 100];
				*dn++ = digits[(c % 100) / 10];
				*dn++ = digits[c % 10];
			} else {
				if (dn >= eom) {
					errno = EMSGSIZE;
					return (-1);
				}
				*dn++ = (char)c;
			}
		}
	}
	if (dn == dst) {
		if (dn >= eom) {
			errno = EMSGSIZE;
			return (-1);
		}
		*dn++ = '.';
	}
	if (dn >= eom) {
		errno = EMSGSIZE;
		return (-1);
	}
	*dn++ = '\0';
	return (dn - dst);
}

/*%
 *	Convert a ascii string into an encoded domain name as per RFC1035.
 *
 * return:
 *
 *\li	-1 if it fails
 *\li	1 if string was fully qualified
 *\li	0 is string was not fully qualified
 *
 * notes:
 *\li	Enforces label and domain length limits.
 */
int
ns_name_pton(const char *src, u_char *dst, size_t dstsiz) {
	return (ns_name_pton2(src, dst, dstsiz, NULL));
}

/*
 * ns_name_pton2(src, dst, dstsiz, *dstlen)
 *	Convert a ascii string into an encoded domain name as per RFC1035.
 * return:
 *	-1 if it fails
 *	1 if string was fully qualified
 *	0 is string was not fully qualified
 * side effects:
 *	fills in *dstlen (if non-NULL)
 * notes:
 *	Enforces label and domain length limits.
 */
int
ns_name_pton2(const char *src, u_char *dst, size_t dstsiz, size_t *dstlen) {
	u_char *label, *bp, *eom;
	int c, n, escaped, e = 0;
	char *cp;

	escaped = 0;
	bp = dst;
	eom = dst + dstsiz;
	label = bp++;

	while ((c = *src++) != 0) {
		if (escaped) {
			if (c == '[') { /*%< start a bit string label */
				if ((cp = strchr(src, ']')) == NULL) {
					errno = EINVAL; /*%< ??? */
					return (-1);
				}
				if ((e = encode_bitsring(&src, cp + 2,
							 &label, &bp, eom))
				    != 0) {
					errno = e;
					return (-1);
				}
				escaped = 0;
				label = bp++;
				if ((c = *src++) == 0)
					goto done;
				else if (c != '.') {
					errno = EINVAL;
					return	(-1);
				}
				continue;
			}
			else if ((cp = strchr(digits, c)) != NULL) {
				n = (cp - digits) * 100;
				if ((c = *src++) == 0 ||
				    (cp = strchr(digits, c)) == NULL) {
					errno = EMSGSIZE;
					return (-1);
				}
				n += (cp - digits) * 10;
				if ((c = *src++) == 0 ||
				    (cp = strchr(digits, c)) == NULL) {
					errno = EMSGSIZE;
					return (-1);
				}
				n += (cp - digits);
				if (n > 255) {
					errno = EMSGSIZE;
					return (-1);
				}
				c = n;
			}
			escaped = 0;
		} else if (c == '\\') {
			escaped = 1;
			continue;
		} else if (c == '.') {
			c = (bp - label - 1);
			if ((c & NS_CMPRSFLGS) != 0) {	/*%< Label too big. */
				errno = EMSGSIZE;
				return (-1);
			}
			if (label >= eom) {
				errno = EMSGSIZE;
				return (-1);
			}
			*label = c;
			/* Fully qualified ? */
			if (*src == '\0') {
				if (c != 0) {
					if (bp >= eom) {
						errno = EMSGSIZE;
						return (-1);
					}
					*bp++ = '\0';
				}
				if ((bp - dst) > MAXCDNAME) {
					errno = EMSGSIZE;
					return (-1);
				}
				if (dstlen != NULL)
					*dstlen = (bp - dst);
				return (1);
			}
			if (c == 0 || *src == '.') {
				errno = EMSGSIZE;
				return (-1);
			}
			label = bp++;
			continue;
		}
		if (bp >= eom) {
			errno = EMSGSIZE;
			return (-1);
		}
		*bp++ = (u_char)c;
	}
	c = (bp - label - 1);
	if ((c & NS_CMPRSFLGS) != 0) {		/*%< Label too big. */
		errno = EMSGSIZE;
		return (-1);
	}
  done:
	if (label >= eom) {
		errno = EMSGSIZE;
		return (-1);
	}
	*label = c;
	if (c != 0) {
		if (bp >= eom) {
			errno = EMSGSIZE;
			return (-1);
		}
		*bp++ = 0;
	}
	if ((bp - dst) > MAXCDNAME) {	/*%< src too big */
		errno = EMSGSIZE;
		return (-1);
	}
	if (dstlen != NULL)
		*dstlen = (bp - dst);
	return (0);
}

/*%
 *	Convert a network strings labels into all lowercase.
 *
 * return:
 *\li	Number of bytes written to buffer, or -1 (with errno set)
 *
 * notes:
 *\li	Enforces label and domain length limits.
 */

int
ns_name_ntol(const u_char *src, u_char *dst, size_t dstsiz)
{
	const u_char *cp;
	u_char *dn, *eom;
	u_char c;
	u_int n;
	int l;

	cp = src;
	dn = dst;
	eom = dst + dstsiz;

	if (dn >= eom) {
		errno = EMSGSIZE;
		return (-1);
	}
	while ((n = *cp++) != 0) {
		if ((n & NS_CMPRSFLGS) == NS_CMPRSFLGS) {
			/* Some kind of compression pointer. */
			errno = EMSGSIZE;
			return (-1);
		}
		*dn++ = n;
		if ((l = labellen(cp - 1)) < 0) {
			errno = EMSGSIZE;
			return (-1);
		}
		if (dn + l >= eom) {
			errno = EMSGSIZE;
			return (-1);
		}
		for ((void)NULL; l > 0; l--) {
			c = *cp++;
			if (isascii(c) && isupper(c))
				*dn++ = tolower(c);
			else
				*dn++ = c;
		}
	}
	*dn++ = '\0';
	return (dn - dst);
}

/*%
 *	Unpack a domain name from a message, source may be compressed.
 *
 * return:
 *\li	-1 if it fails, or consumed octets if it succeeds.
 */
int
ns_name_unpack(const u_char *msg, const u_char *eom, const u_char *src,
	       u_char *dst, size_t dstsiz)
{
	return (ns_name_unpack2(msg, eom, src, dst, dstsiz, NULL));
}

/*
 * ns_name_unpack2(msg, eom, src, dst, dstsiz, *dstlen)
 *	Unpack a domain name from a message, source may be compressed.
 * return:
 *	-1 if it fails, or consumed octets if it succeeds.
 * side effect:
 *	fills in *dstlen (if non-NULL).
 */
int
ns_name_unpack2(const u_char *msg, const u_char *eom, const u_char *src,
		u_char *dst, size_t dstsiz, size_t *dstlen)
{
	const u_char *srcp, *dstlim;
	u_char *dstp;
	int n, len, checked, l;

	len = -1;
	checked = 0;
	dstp = dst;
	srcp = src;
	dstlim = dst + dstsiz;
	if (srcp < msg || srcp >= eom) {
		errno = EMSGSIZE;
		return (-1);
	}
	/* Fetch next label in domain name. */
	while ((n = *srcp++) != 0) {
		/* Check for indirection. */
		switch (n & NS_CMPRSFLGS) {
		case 0:
		case NS_TYPE_ELT:
			/* Limit checks. */
			if ((l = labellen(srcp - 1)) < 0) {
				errno = EMSGSIZE;
				return (-1);
			}
			if (dstp + l + 1 >= dstlim || srcp + l >= eom) {
				errno = EMSGSIZE;
				return (-1);
			}
			checked += l + 1;
			*dstp++ = n;
			memcpy(dstp, srcp, l);
			dstp += l;
			srcp += l;
			break;

		case NS_CMPRSFLGS:
			if (srcp >= eom) {
				errno = EMSGSIZE;
				return (-1);
			}
			if (len < 0)
				len = srcp - src + 1;
			l = ((n & 0x3f) << 8) | (*srcp & 0xff);
			if (l >= eom - msg) {  /*%< Out of range. */
				errno = EMSGSIZE;
				return (-1);
			}
			srcp = msg + l;
			checked += 2;
			/*
			 * Check for loops in the compressed name;
			 * if we've looked at the whole message,
			 * there must be a loop.
			 */
			if (checked >= eom - msg) {
				errno = EMSGSIZE;
				return (-1);
			}
			break;

		default:
			errno = EMSGSIZE;
			return (-1);			/*%< flag error */
		}
	}
	*dstp++ = 0;
	if (dstlen != NULL)
		*dstlen = dstp - dst;
	if (len < 0)
		len = srcp - src;
	return (len);
}

/*%
 *	Pack domain name 'domain' into 'comp_dn'.
 *
 * return:
 *\li	Size of the compressed name, or -1.
 *
 * notes:
 *\li	'dnptrs' is an array of pointers to previous compressed names.
 *\li	dnptrs[0] is a pointer to the beginning of the message. The array
 *	ends with NULL.
 *\li	'lastdnptr' is a pointer to the end of the array pointed to
 *	by 'dnptrs'.
 *
 * Side effects:
 *\li	The list of pointers in dnptrs is updated for labels inserted into
 *	the message as we compress the name.  If 'dnptr' is NULL, we don't
 *	try to compress names. If 'lastdnptr' is NULL, we don't update the
 *	list.
 */
int
ns_name_pack(const u_char *src, u_char *dst, int dstsiz,
	     const u_char **dnptrs, const u_char **lastdnptr)
{
	u_char *dstp;
	const u_char **cpp, **lpp, *eob, *msg;
	const u_char *srcp;
	int n, l, first = 1;

	srcp = src;
	dstp = dst;
	eob = dstp + dstsiz;
	lpp = cpp = NULL;
	if (dnptrs != NULL) {
		if ((msg = *dnptrs++) != NULL) {
			for (cpp = dnptrs; *cpp != NULL; cpp++)
				(void)NULL;
			lpp = cpp;	/*%< end of list to search */
		}
	} else
		msg = NULL;

	/* make sure the domain we are about to add is legal */
	l = 0;
	do {
		int l0;

		n = *srcp;
		if ((n & NS_CMPRSFLGS) == NS_CMPRSFLGS) {
			errno = EMSGSIZE;
			return (-1);
		}
		if ((l0 = labellen(srcp)) < 0) {
			errno = EINVAL;
			return (-1);
		}
		l += l0 + 1;
		if (l > MAXCDNAME) {
			errno = EMSGSIZE;
			return (-1);
		}
		srcp += l0 + 1;
	} while (n != 0);

	/* from here on we need to reset compression pointer array on error */
	srcp = src;
	do {
		/* Look to see if we can use pointers. */
		n = *srcp;
		if (n != 0 && msg != NULL) {
			l = dn_find(srcp, msg, (const u_char * const *)dnptrs,
				    (const u_char * const *)lpp);
			if (l >= 0) {
				if (dstp + 1 >= eob) {
					goto cleanup;
				}
				*dstp++ = (l >> 8) | NS_CMPRSFLGS;
				*dstp++ = l % 256;
				return (dstp - dst);
			}
			/* Not found, save it. */
			if (lastdnptr != NULL && cpp < lastdnptr - 1 &&
			    (dstp - msg) < 0x4000 && first) {
				*cpp++ = dstp;
				*cpp = NULL;
				first = 0;
			}
		}
		/* copy label to buffer */
		if ((n & NS_CMPRSFLGS) == NS_CMPRSFLGS) {
			/* Should not happen. */
			goto cleanup;
		}
		n = labellen(srcp);
		if (dstp + 1 + n >= eob) {
			goto cleanup;
		}
		memcpy(dstp, srcp, n + 1);
		srcp += n + 1;
		dstp += n + 1;
	} while (n != 0);

	if (dstp > eob) {
cleanup:
		if (msg != NULL)
			*lpp = NULL;
		errno = EMSGSIZE;
		return (-1);
	} 
	return (dstp - dst);
}

/*%
 *	Expand compressed domain name to presentation format.
 *
 * return:
 *\li	Number of bytes read out of `src', or -1 (with errno set).
 *
 * note:
 *\li	Root domain returns as "." not "".
 */
int
ns_name_uncompress(const u_char *msg, const u_char *eom, const u_char *src,
		   char *dst, size_t dstsiz)
{
	u_char tmp[NS_MAXCDNAME];
	int n;
	
	if ((n = ns_name_unpack(msg, eom, src, tmp, sizeof tmp)) == -1)
		return (-1);
	if (ns_name_ntop(tmp, dst, dstsiz) == -1)
		return (-1);
	return (n);
}

/*%
 *	Compress a domain name into wire format, using compression pointers.
 *
 * return:
 *\li	Number of bytes consumed in `dst' or -1 (with errno set).
 *
 * notes:
 *\li	'dnptrs' is an array of pointers to previous compressed names.
 *\li	dnptrs[0] is a pointer to the beginning of the message.
 *\li	The list ends with NULL.  'lastdnptr' is a pointer to the end of the
 *	array pointed to by 'dnptrs'. Side effect is to update the list of
 *	pointers for labels inserted into the message as we compress the name.
 *\li	If 'dnptr' is NULL, we don't try to compress names. If 'lastdnptr'
 *	is NULL, we don't update the list.
 */
int
ns_name_compress(const char *src, u_char *dst, size_t dstsiz,
		 const u_char **dnptrs, const u_char **lastdnptr)
{
	u_char tmp[NS_MAXCDNAME];

	if (ns_name_pton(src, tmp, sizeof tmp) == -1)
		return (-1);
	return (ns_name_pack(tmp, dst, dstsiz, dnptrs, lastdnptr));
}

/*%
 * Reset dnptrs so that there are no active references to pointers at or
 * after src.
 */
void
ns_name_rollback(const u_char *src, const u_char **dnptrs,
		 const u_char **lastdnptr)
{
	while (dnptrs < lastdnptr && *dnptrs != NULL) {
		if (*dnptrs >= src) {
			*dnptrs = NULL;
			break;
		}
		dnptrs++;
	}
}

/*%
 *	Advance *ptrptr to skip over the compressed name it points at.
 *
 * return:
 *\li	0 on success, -1 (with errno set) on failure.
 */
int
ns_name_skip(const u_char **ptrptr, const u_char *eom)
{
	const u_char *cp;
	u_int n;
	int l = 0;

	cp = *ptrptr;
	while (cp < eom && (n = *cp++) != 0) {
		/* Check for indirection. */
		switch (n & NS_CMPRSFLGS) {
		case 0:			/*%< normal case, n == len */
			cp += n;
			continue;
		case NS_TYPE_ELT: /*%< EDNS0 extended label */
			if (cp < eom && (l = labellen(cp - 1)) < 0) {
				errno = EMSGSIZE; /*%< XXX */
				return (-1);
			}
			cp += l;
			continue;
		case NS_CMPRSFLGS:	/*%< indirection */
			cp++;
			break;
		default:		/*%< illegal type */
			errno = EMSGSIZE;
			return (-1);
		}
		break;
	}
	if (cp > eom) {
		errno = EMSGSIZE;
		return (-1);
	}
	*ptrptr = cp;
	return (0);
}

/* Find the number of octets an nname takes up, including the root label.
 * (This is basically ns_name_skip() without compression-pointer support.)
 * ((NOTE: can only return zero if passed-in namesiz argument is zero.))
 */
ssize_t
ns_name_length(ns_nname_ct nname, size_t namesiz) {
	ns_nname_ct orig = nname;
	u_int n;

	while (namesiz-- > 0 && (n = *nname++) != 0) {
		if ((n & NS_CMPRSFLGS) != 0) {
			errno = EISDIR;
			return (-1);
		}
		if (n > namesiz) {
			errno = EMSGSIZE;
			return (-1);
		}
		nname += n;
		namesiz -= n;
	}
	return (nname - orig);
}

/* Compare two nname's for equality.  Return -1 on error (setting errno).
 */
int
ns_name_eq(ns_nname_ct a, size_t as, ns_nname_ct b, size_t bs) {
	ns_nname_ct ae = a + as, be = b + bs;
	int ac, bc;

	while (ac = *a, bc = *b, ac != 0 && bc != 0) {
		if ((ac & NS_CMPRSFLGS) != 0 || (bc & NS_CMPRSFLGS) != 0) {
			errno = EISDIR;
			return (-1);
		}
		if (a + ac >= ae || b + bc >= be) {
			errno = EMSGSIZE;
			return (-1);
		}
		if (ac != bc || strncasecmp((const char *) ++a,
					    (const char *) ++b, ac) != 0)
			return (0);
		a += ac, b += bc;
	}
	return (ac == 0 && bc == 0);
}

/* Is domain "A" owned by (at or below) domain "B"?
 */
int
ns_name_owned(ns_namemap_ct a, int an, ns_namemap_ct b, int bn) {
	/* If A is shorter, it cannot be owned by B. */
	if (an < bn)
		return (0);

	/* If they are unequal before the length of the shorter, A cannot... */
	while (bn > 0) {
		if (a->len != b->len ||
		    strncasecmp((const char *) a->base,
				(const char *) b->base, a->len) != 0)
			return (0);
		a++, an--;
		b++, bn--;
	}

	/* A might be longer or not, but either way, B owns it. */
	return (1);
}

/* Build an array of <base,len> tuples from an nname, top-down order.
 * Return the number of tuples (labels) thus discovered.
 */
int
ns_name_map(ns_nname_ct nname, size_t namelen, ns_namemap_t map, int mapsize) {
	u_int n;
	int l;

	n = *nname++;
	namelen--;

	/* Root zone? */
	if (n == 0) {
		/* Extra data follows name? */
		if (namelen > 0) {
			errno = EMSGSIZE;
			return (-1);
		}
		return (0);
	}

	/* Compression pointer? */
	if ((n & NS_CMPRSFLGS) != 0) {
		errno = EISDIR;
		return (-1);
	}

	/* Label too long? */
	if (n > namelen) {
		errno = EMSGSIZE;
		return (-1);
	}

	/* Recurse to get rest of name done first. */
	l = ns_name_map(nname + n, namelen - n, map, mapsize);
	if (l < 0)
		return (-1);

	/* Too many labels? */
	if (l >= mapsize) {
		errno = ENAMETOOLONG;
		return (-1);
	}

	/* We're on our way back up-stack, store current map data. */
	map[l].base = nname;
	map[l].len = n;
	return (l + 1);
}

/* Count the labels in a domain name.  Root counts, so COM. has two.  This
 * is to make the result comparable to the result of ns_name_map().
 */
int
ns_name_labels(ns_nname_ct nname, size_t namesiz) {
	int ret = 0;
	u_int n;

	while (namesiz-- > 0 && (n = *nname++) != 0) {
		if ((n & NS_CMPRSFLGS) != 0) {
			errno = EISDIR;
			return (-1);
		}
		if (n > namesiz) {
			errno = EMSGSIZE;
			return (-1);
		}
		nname += n;
		namesiz -= n;
		ret++;
	}
	return (ret + 1);
}

/* Private. */

/*%
 *	Thinking in noninternationalized USASCII (per the DNS spec),
 *	is this characted special ("in need of quoting") ?
 *
 * return:
 *\li	boolean.
 */
static int
special(int ch) {
	switch (ch) {
	case 0x22: /*%< '"' */
	case 0x2E: /*%< '.' */
	case 0x3B: /*%< ';' */
	case 0x5C: /*%< '\\' */
	case 0x28: /*%< '(' */
	case 0x29: /*%< ')' */
	/* Special modifiers in zone files. */
	case 0x40: /*%< '@' */
	case 0x24: /*%< '$' */
		return (1);
	default:
		return (0);
	}
}

/*%
 *	Thinking in noninternationalized USASCII (per the DNS spec),
 *	is this character visible and not a space when printed ?
 *
 * return:
 *\li	boolean.
 */
static int
printable(int ch) {
	return (ch > 0x20 && ch < 0x7f);
}

/*%
 *	Thinking in noninternationalized USASCII (per the DNS spec),
 *	convert this character to lower case if it's upper case.
 */
static int
mklower(int ch) {
	if (ch >= 0x41 && ch <= 0x5A)
		return (ch + 0x20);
	return (ch);
}

/*%
 *	Search for the counted-label name in an array of compressed names.
 *
 * return:
 *\li	offset from msg if found, or -1.
 *
 * notes:
 *\li	dnptrs is the pointer to the first name on the list,
 *\li	not the pointer to the start of the message.
 */
static int
dn_find(const u_char *domain, const u_char *msg,
	const u_char * const *dnptrs,
	const u_char * const *lastdnptr)
{
	const u_char *dn, *cp, *sp;
	const u_char * const *cpp;
	u_int n;

	for (cpp = dnptrs; cpp < lastdnptr; cpp++) {
		sp = *cpp;
		/*
		 * terminate search on:
		 * root label
		 * compression pointer
		 * unusable offset
		 */
		while (*sp != 0 && (*sp & NS_CMPRSFLGS) == 0 &&
		       (sp - msg) < 0x4000) {
			dn = domain;
			cp = sp;
			while ((n = *cp++) != 0) {
				/*
				 * check for indirection
				 */
				switch (n & NS_CMPRSFLGS) {
				case 0:		/*%< normal case, n == len */
					n = labellen(cp - 1); /*%< XXX */
					if (n != *dn++)
						goto next;

					for ((void)NULL; n > 0; n--)
						if (mklower(*dn++) !=
						    mklower(*cp++))
							goto next;
					/* Is next root for both ? */
					if (*dn == '\0' && *cp == '\0')
						return (sp - msg);
					if (*dn)
						continue;
					goto next;
				case NS_CMPRSFLGS:	/*%< indirection */
					cp = msg + (((n & 0x3f) << 8) | *cp);
					break;

				default:	/*%< illegal type */
					errno = EMSGSIZE;
					return (-1);
				}
			}
 next: ;
			sp += *sp + 1;
		}
	}
	errno = ENOENT;
	return (-1);
}

static int
decode_bitstring(const unsigned char **cpp, char *dn, const char *eom)
{
	const unsigned char *cp = *cpp;
	char *beg = dn, tc;
	int b, blen, plen, i;

	if ((blen = (*cp & 0xff)) == 0)
		blen = 256;
	plen = (blen + 3) / 4;
	plen += sizeof("\\[x/]") + (blen > 99 ? 3 : (blen > 9) ? 2 : 1);
	if (dn + plen >= eom)
		return (-1);

	cp++;
	i = SPRINTF((dn, "\\[x"));
	if (i < 0)
		return (-1);
	dn += i;
	for (b = blen; b > 7; b -= 8, cp++) {
		i = SPRINTF((dn, "%02x", *cp & 0xff));
		if (i < 0)
			return (-1);
		dn += i;
	}
	if (b > 4) {
		tc = *cp++;
		i = SPRINTF((dn, "%02x", tc & (0xff << (8 - b))));
		if (i < 0)
			return (-1);
		dn += i;
	} else if (b > 0) {
		tc = *cp++;
		i = SPRINTF((dn, "%1x",
			       ((tc >> 4) & 0x0f) & (0x0f << (4 - b)))); 
		if (i < 0)
			return (-1);
		dn += i;
	}
	i = SPRINTF((dn, "/%d]", blen));
	if (i < 0)
		return (-1);
	dn += i;

	*cpp = cp;
	return (dn - beg);
}

static int
encode_bitsring(const char **bp, const char *end, unsigned char **labelp,
		unsigned char ** dst, unsigned const char *eom)
{
	int afterslash = 0;
	const char *cp = *bp;
	unsigned char *tp;
	char c;
	const char *beg_blen;
	char *end_blen = NULL;
	int value = 0, count = 0, tbcount = 0, blen = 0;

	beg_blen = end_blen = NULL;

	/* a bitstring must contain at least 2 characters */
	if (end - cp < 2)
		return (EINVAL);

	/* XXX: currently, only hex strings are supported */
	if (*cp++ != 'x')
		return (EINVAL);
	if (!isxdigit((*cp) & 0xff)) /*%< reject '\[x/BLEN]' */
		return (EINVAL);

	for (tp = *dst + 1; cp < end && tp < eom; cp++) {
		switch((c = *cp)) {
		case ']':	/*%< end of the bitstring */
			if (afterslash) {
				if (beg_blen == NULL)
					return (EINVAL);
				blen = (int)strtol(beg_blen, &end_blen, 10);
				if (*end_blen != ']')
					return (EINVAL);
			}
			if (count)
				*tp++ = ((value << 4) & 0xff);
			cp++;	/*%< skip ']' */
			goto done;
		case '/':
			afterslash = 1;
			break;
		default:
			if (afterslash) {
				if (!isdigit(c&0xff))
					return (EINVAL);
				if (beg_blen == NULL) {
					
					if (c == '0') {
						/* blen never begings with 0 */
						return (EINVAL);
					}
					beg_blen = cp;
				}
			} else {
				if (!isxdigit(c&0xff))
					return (EINVAL);
				value <<= 4;
				value += digitvalue[(int)c];
				count += 4;
				tbcount += 4;
				if (tbcount > 256)
					return (EINVAL);
				if (count == 8) {
					*tp++ = value;
					count = 0;
				}
			}
			break;
		}
	}
  done:
	if (cp >= end || tp >= eom)
		return (EMSGSIZE);

	/*
	 * bit length validation:
	 * If a <length> is present, the number of digits in the <bit-data>
	 * MUST be just sufficient to contain the number of bits specified
	 * by the <length>. If there are insignificant bits in a final
	 * hexadecimal or octal digit, they MUST be zero.
	 * RFC2673, Section 3.2.
	 */
	if (blen > 0) {
		int traillen;

		if (((blen + 3) & ~3) != tbcount)
			return (EINVAL);
		traillen = tbcount - blen; /*%< between 0 and 3 */
		if (((value << (8 - traillen)) & 0xff) != 0)
			return (EINVAL);
	}
	else
		blen = tbcount;
	if (blen == 256)
		blen = 0;

	/* encode the type and the significant bit fields */
	**labelp = DNS_LABELTYPE_BITSTRING;
	**dst = blen;

	*bp = cp;
	*dst = tp;

	return (0);
}

static int
labellen(const u_char *lp)
{
	int bitlen;
	u_char l = *lp;

	if ((l & NS_CMPRSFLGS) == NS_CMPRSFLGS) {
		/* should be avoided by the caller */
		return (-1);
	}

	if ((l & NS_CMPRSFLGS) == NS_TYPE_ELT) {
		if (l == DNS_LABELTYPE_BITSTRING) {
			if ((bitlen = *(lp + 1)) == 0)
				bitlen = 256;
			return ((bitlen + 7 ) / 8 + 1);
		}
		return (-1);	/*%< unknown ELT */
	}
	return (l);
}

/*! \file */

/* -------- ns_print.c -------- */

/*-
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1996-1999 by Internet Software Consortium.
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

/* Import. */


#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>

#include <errno.h>
#include <resolv.h>
#include <string.h>
#include <ctype.h>



/* Forward. */

static size_t	prune_origin(const char *name, const char *origin);
static int	charstr(const u_char *rdata, const u_char *edata,
			char **buf, size_t *buflen);
static int	addname(const u_char *msg, size_t msglen,
			const u_char **p, const char *origin,
			char **buf, size_t *buflen);
static void	addlen(size_t len, char **buf, size_t *buflen);
static int	addstr(const char *src, size_t len,
		       char **buf, size_t *buflen);
static int	addtab(size_t len, size_t target, int spaced,
		       char **buf, size_t *buflen);

/* Macros. */



/* Public. */

/*%
 *	Convert an RR to presentation format.
 *
 * return:
 *\li	Number of characters written to buf, or -1 (check errno).
 */
int
ns_sprintrr(const ns_msg *handle, const ns_rr *rr,
	    const char *name_ctx, const char *origin,
	    char *buf, size_t buflen)
{
	int n;

	n = ns_sprintrrf(ns_msg_base(*handle), ns_msg_size(*handle),
			 ns_rr_name(*rr), ns_rr_class(*rr), ns_rr_type(*rr),
			 ns_rr_ttl(*rr), ns_rr_rdata(*rr), ns_rr_rdlen(*rr),
			 name_ctx, origin, buf, buflen);
	return (n);
}

/*%
 *	Convert the fields of an RR into presentation format.
 *
 * return:
 *\li	Number of characters written to buf, or -1 (check errno).
 */
int
ns_sprintrrf(const u_char *msg, size_t msglen,
	    const char *name, ns_class class, ns_type type,
	    u_long ttl, const u_char *rdata, size_t rdlen,
	    const char *name_ctx, const char *origin,
	    char *buf, size_t buflen)
{
	const char *obuf = buf;
	const u_char *edata = rdata + rdlen;
	int spaced = 0;

	const char *comment;
	char tmp[100];
	int len, x;

	/*
	 * Owner.
	 */
	if (name_ctx != NULL && ns_samename(name_ctx, name) == 1) {
		T(addstr("\t\t\t", 3, &buf, &buflen));
	} else {
		len = prune_origin(name, origin);
		if (*name == '\0') {
			goto root;
		} else if (len == 0) {
			T(addstr("@\t\t\t", 4, &buf, &buflen));
		} else {
			T(addstr(name, len, &buf, &buflen));
			/* Origin not used or not root, and no trailing dot? */
			if (((origin == NULL || origin[0] == '\0') ||
			    (origin[0] != '.' && origin[1] != '\0' &&
			    name[len] == '\0')) && name[len - 1] != '.') {
 root:
				T(addstr(".", 1, &buf, &buflen));
				len++;
			}
			T(spaced = addtab(len, 24, spaced, &buf, &buflen));
		}
	}

	/*
	 * TTL, Class, Type.
	 */
	T(x = ns_format_ttl(ttl, buf, buflen));
	addlen(x, &buf, &buflen);
	len = SPRINTF((tmp, " %s %s", p_class(class), p_type(type)));
	T(addstr(tmp, len, &buf, &buflen));
	T(spaced = addtab(x + len, 16, spaced, &buf, &buflen));

	/*
	 * RData.
	 */
	switch (type) {
	case ns_t_a:
		if (rdlen != (size_t)NS_INADDRSZ)
			goto formerr;
		(void) inet_ntop(AF_INET, rdata, buf, buflen);
		addlen(strlen(buf), &buf, &buflen);
		break;

	case ns_t_cname:
	case ns_t_mb:
	case ns_t_mg:
	case ns_t_mr:
	case ns_t_ns:
	case ns_t_ptr:
	case ns_t_dname:
		T(addname(msg, msglen, &rdata, origin, &buf, &buflen));
		break;

	case ns_t_hinfo:
	case ns_t_isdn:
		/* First word. */
		T(len = charstr(rdata, edata, &buf, &buflen));
		if (len == 0)
			goto formerr;
		rdata += len;
		T(addstr(" ", 1, &buf, &buflen));

		    
		/* Second word, optional in ISDN records. */
		if (type == ns_t_isdn && rdata == edata)
			break;
		    
		T(len = charstr(rdata, edata, &buf, &buflen));
		if (len == 0)
			goto formerr;
		rdata += len;
		break;

	case ns_t_soa: {
		u_long t;

		/* Server name. */
		T(addname(msg, msglen, &rdata, origin, &buf, &buflen));
		T(addstr(" ", 1, &buf, &buflen));

		/* Administrator name. */
		T(addname(msg, msglen, &rdata, origin, &buf, &buflen));
		T(addstr(" (\n", 3, &buf, &buflen));
		spaced = 0;

		if ((edata - rdata) != 5*NS_INT32SZ)
			goto formerr;

		/* Serial number. */
		t = ns_get32(rdata);  rdata += NS_INT32SZ;
		T(addstr("\t\t\t\t\t", 5, &buf, &buflen));
		len = SPRINTF((tmp, "%lu", t));
		T(addstr(tmp, len, &buf, &buflen));
		T(spaced = addtab(len, 16, spaced, &buf, &buflen));
		T(addstr("; serial\n", 9, &buf, &buflen));
		spaced = 0;

		/* Refresh interval. */
		t = ns_get32(rdata);  rdata += NS_INT32SZ;
		T(addstr("\t\t\t\t\t", 5, &buf, &buflen));
		T(len = ns_format_ttl(t, buf, buflen));
		addlen(len, &buf, &buflen);
		T(spaced = addtab(len, 16, spaced, &buf, &buflen));
		T(addstr("; refresh\n", 10, &buf, &buflen));
		spaced = 0;

		/* Retry interval. */
		t = ns_get32(rdata);  rdata += NS_INT32SZ;
		T(addstr("\t\t\t\t\t", 5, &buf, &buflen));
		T(len = ns_format_ttl(t, buf, buflen));
		addlen(len, &buf, &buflen);
		T(spaced = addtab(len, 16, spaced, &buf, &buflen));
		T(addstr("; retry\n", 8, &buf, &buflen));
		spaced = 0;

		/* Expiry. */
		t = ns_get32(rdata);  rdata += NS_INT32SZ;
		T(addstr("\t\t\t\t\t", 5, &buf, &buflen));
		T(len = ns_format_ttl(t, buf, buflen));
		addlen(len, &buf, &buflen);
		T(spaced = addtab(len, 16, spaced, &buf, &buflen));
		T(addstr("; expiry\n", 9, &buf, &buflen));
		spaced = 0;

		/* Minimum TTL. */
		t = ns_get32(rdata);  rdata += NS_INT32SZ;
		T(addstr("\t\t\t\t\t", 5, &buf, &buflen));
		T(len = ns_format_ttl(t, buf, buflen));
		addlen(len, &buf, &buflen);
		T(addstr(" )", 2, &buf, &buflen));
		T(spaced = addtab(len, 16, spaced, &buf, &buflen));
		T(addstr("; minimum\n", 10, &buf, &buflen));

		break;
	    }

	case ns_t_mx:
	case ns_t_afsdb:
	case ns_t_rt:
	case ns_t_kx: {
		u_int t;

		if (rdlen < (size_t)NS_INT16SZ)
			goto formerr;

		/* Priority. */
		t = ns_get16(rdata);
		rdata += NS_INT16SZ;
		len = SPRINTF((tmp, "%u ", t));
		T(addstr(tmp, len, &buf, &buflen));

		/* Target. */
		T(addname(msg, msglen, &rdata, origin, &buf, &buflen));

		break;
	    }

	case ns_t_px: {
		u_int t;

		if (rdlen < (size_t)NS_INT16SZ)
			goto formerr;

		/* Priority. */
		t = ns_get16(rdata);
		rdata += NS_INT16SZ;
		len = SPRINTF((tmp, "%u ", t));
		T(addstr(tmp, len, &buf, &buflen));

		/* Name1. */
		T(addname(msg, msglen, &rdata, origin, &buf, &buflen));
		T(addstr(" ", 1, &buf, &buflen));

		/* Name2. */
		T(addname(msg, msglen, &rdata, origin, &buf, &buflen));

		break;
	    }

	case ns_t_x25:
		T(len = charstr(rdata, edata, &buf, &buflen));
		if (len == 0)
			goto formerr;
		rdata += len;
		break;

	case ns_t_txt:
	case ns_t_spf:
		while (rdata < edata) {
			T(len = charstr(rdata, edata, &buf, &buflen));
			if (len == 0)
				goto formerr;
			rdata += len;
			if (rdata < edata)
				T(addstr(" ", 1, &buf, &buflen));
		}
		break;

	case ns_t_nsap: {
		char t[2+255*3];

		(void) inet_nsap_ntoa(rdlen, rdata, t);
		T(addstr(t, strlen(t), &buf, &buflen));
		break;
	    }

	case ns_t_aaaa:
		if (rdlen != (size_t)NS_IN6ADDRSZ)
			goto formerr;
		(void) inet_ntop(AF_INET6, rdata, buf, buflen);
		addlen(strlen(buf), &buf, &buflen);
		break;

	case ns_t_loc: {
		char t[255];

		/* XXX protocol format checking? */
		(void) loc_ntoa(rdata, t);
		T(addstr(t, strlen(t), &buf, &buflen));
		break;
	    }

	case ns_t_naptr: {
		u_int order, preference;
		char t[50];

		if (rdlen < 2U*NS_INT16SZ)
			goto formerr;

		/* Order, Precedence. */
		order = ns_get16(rdata);	rdata += NS_INT16SZ;
		preference = ns_get16(rdata);	rdata += NS_INT16SZ;
		len = SPRINTF((t, "%u %u ", order, preference));
		T(addstr(t, len, &buf, &buflen));

		/* Flags. */
		T(len = charstr(rdata, edata, &buf, &buflen));
		if (len == 0)
			goto formerr;
		rdata += len;
		T(addstr(" ", 1, &buf, &buflen));

		/* Service. */
		T(len = charstr(rdata, edata, &buf, &buflen));
		if (len == 0)
			goto formerr;
		rdata += len;
		T(addstr(" ", 1, &buf, &buflen));

		/* Regexp. */
		T(len = charstr(rdata, edata, &buf, &buflen));
		if (len < 0)
			return (-1);
		if (len == 0)
			goto formerr;
		rdata += len;
		T(addstr(" ", 1, &buf, &buflen));

		/* Server. */
		T(addname(msg, msglen, &rdata, origin, &buf, &buflen));
		break;
	    }

	case ns_t_srv: {
		u_int priority, weight, port;
		char t[50];

		if (rdlen < 3U*NS_INT16SZ)
			goto formerr;

		/* Priority, Weight, Port. */
		priority = ns_get16(rdata);  rdata += NS_INT16SZ;
		weight   = ns_get16(rdata);  rdata += NS_INT16SZ;
		port     = ns_get16(rdata);  rdata += NS_INT16SZ;
		len = SPRINTF((t, "%u %u %u ", priority, weight, port));
		T(addstr(t, len, &buf, &buflen));

		/* Server. */
		T(addname(msg, msglen, &rdata, origin, &buf, &buflen));
		break;
	    }

	case ns_t_minfo:
	case ns_t_rp:
		/* Name1. */
		T(addname(msg, msglen, &rdata, origin, &buf, &buflen));
		T(addstr(" ", 1, &buf, &buflen));

		/* Name2. */
		T(addname(msg, msglen, &rdata, origin, &buf, &buflen));

		break;

	case ns_t_wks: {
		int n, lcnt;

		if (rdlen < 1U + NS_INT32SZ)
			goto formerr;

		/* Address. */
		(void) inet_ntop(AF_INET, rdata, buf, buflen);
		addlen(strlen(buf), &buf, &buflen);
		rdata += NS_INADDRSZ;

		/* Protocol. */
		len = SPRINTF((tmp, " %u ( ", *rdata));
		T(addstr(tmp, len, &buf, &buflen));
		rdata += NS_INT8SZ;

		/* Bit map. */
		n = 0;
		lcnt = 0;
		while (rdata < edata) {
			u_int c = *rdata++;
			do {
				if (c & 0200) {
					if (lcnt == 0) {
						T(addstr("\n\t\t\t\t", 5,
							 &buf, &buflen));
						lcnt = 10;
						spaced = 0;
					}
					len = SPRINTF((tmp, "%d ", n));
					T(addstr(tmp, len, &buf, &buflen));
					lcnt--;
				}
				c <<= 1;
			} while (++n & 07);
		}
		T(addstr(")", 1, &buf, &buflen));

		break;
	    }

	case ns_t_key:
	case ns_t_dnskey: {
		char base64_key[NS_MD5RSA_MAX_BASE64];
		u_int keyflags, protocol, algorithm, key_id;
		const char *leader;
		int n;

		if (rdlen < 0U + NS_INT16SZ + NS_INT8SZ + NS_INT8SZ)
			goto formerr;

		/* Key flags, Protocol, Algorithm. */
#ifndef _LIBC
		key_id = dst_s_dns_key_id(rdata, edata-rdata);
#else
		key_id = 0;
#endif
		keyflags = ns_get16(rdata);  rdata += NS_INT16SZ;
		protocol = *rdata++;
		algorithm = *rdata++;
		len = SPRINTF((tmp, "0x%04x %u %u",
			       keyflags, protocol, algorithm));
		T(addstr(tmp, len, &buf, &buflen));

		/* Public key data. */
		len = b64_ntop(rdata, edata - rdata,
			       base64_key, sizeof base64_key);
		if (len < 0)
			goto formerr;
		if (len > 15) {
			T(addstr(" (", 2, &buf, &buflen));
			leader = "\n\t\t";
			spaced = 0;
		} else
			leader = " ";
		for (n = 0; n < len; n += 48) {
			T(addstr(leader, strlen(leader), &buf, &buflen));
			T(addstr(base64_key + n, MIN(len - n, 48),
				 &buf, &buflen));
		}
		if (len > 15)
			T(addstr(" )", 2, &buf, &buflen));
		n = SPRINTF((tmp, " ; key_tag= %u", key_id));
		T(addstr(tmp, n, &buf, &buflen));

		break;
	    }

	case ns_t_sig:
	case ns_t_rrsig: {
		char base64_key[NS_MD5RSA_MAX_BASE64];
		u_int type, algorithm, labels, footprint;
		const char *leader;
		u_long t;
		int n;

		if (rdlen < 22U)
			goto formerr;

		/* Type covered, Algorithm, Label count, Original TTL. */
		type = ns_get16(rdata);  rdata += NS_INT16SZ;
		algorithm = *rdata++;
		labels = *rdata++;
		t = ns_get32(rdata);  rdata += NS_INT32SZ;
		len = SPRINTF((tmp, "%s %d %d %lu ",
			       p_type(type), algorithm, labels, t));
		T(addstr(tmp, len, &buf, &buflen));
		if (labels > (u_int)dn_count_labels(name))
			goto formerr;

		/* Signature expiry. */
		t = ns_get32(rdata);  rdata += NS_INT32SZ;
		len = SPRINTF((tmp, "%s ", p_secstodate(t)));
		T(addstr(tmp, len, &buf, &buflen));

		/* Time signed. */
		t = ns_get32(rdata);  rdata += NS_INT32SZ;
		len = SPRINTF((tmp, "%s ", p_secstodate(t)));
		T(addstr(tmp, len, &buf, &buflen));

		/* Signature Footprint. */
		footprint = ns_get16(rdata);  rdata += NS_INT16SZ;
		len = SPRINTF((tmp, "%u ", footprint));
		T(addstr(tmp, len, &buf, &buflen));

		/* Signer's name. */
		T(addname(msg, msglen, &rdata, origin, &buf, &buflen));

		/* Signature. */
		len = b64_ntop(rdata, edata - rdata,
			       base64_key, sizeof base64_key);
		if (len > 15) {
			T(addstr(" (", 2, &buf, &buflen));
			leader = "\n\t\t";
			spaced = 0;
		} else
			leader = " ";
		if (len < 0)
			goto formerr;
		for (n = 0; n < len; n += 48) {
			T(addstr(leader, strlen(leader), &buf, &buflen));
			T(addstr(base64_key + n, MIN(len - n, 48),
				 &buf, &buflen));
		}
		if (len > 15)
			T(addstr(" )", 2, &buf, &buflen));
		break;
	    }

	case ns_t_nxt: {
		int n, c;

		/* Next domain name. */
		T(addname(msg, msglen, &rdata, origin, &buf, &buflen));

		/* Type bit map. */
		n = edata - rdata;
		for (c = 0; c < n*8; c++)
			if (NS_NXT_BIT_ISSET(c, rdata)) {
				len = SPRINTF((tmp, " %s", p_type(c)));
				T(addstr(tmp, len, &buf, &buflen));
			}
		break;
	    }

	case ns_t_cert: {
		u_int c_type, key_tag, alg;
		int n;
		unsigned int siz;
		char base64_cert[8192], tmp[40];
		const char *leader;

		c_type  = ns_get16(rdata); rdata += NS_INT16SZ;
		key_tag = ns_get16(rdata); rdata += NS_INT16SZ;
		alg = (u_int) *rdata++;

		len = SPRINTF((tmp, "%d %d %d ", c_type, key_tag, alg));
		T(addstr(tmp, len, &buf, &buflen));
		siz = (edata-rdata)*4/3 + 4; /* "+4" accounts for trailing \0 */
		if (siz > sizeof(base64_cert) * 3/4) {
			const char *str = "record too long to print";
			T(addstr(str, strlen(str), &buf, &buflen));
		}
		else {
			len = b64_ntop(rdata, edata-rdata, base64_cert, siz);

			if (len < 0)
				goto formerr;
			else if (len > 15) {
				T(addstr(" (", 2, &buf, &buflen));
				leader = "\n\t\t";
				spaced = 0;
			}
			else
				leader = " ";
	
			for (n = 0; n < len; n += 48) {
				T(addstr(leader, strlen(leader),
					 &buf, &buflen));
				T(addstr(base64_cert + n, MIN(len - n, 48),
					 &buf, &buflen));
			}
			if (len > 15)
				T(addstr(" )", 2, &buf, &buflen));
		}
		break;
	    }

	case ns_t_tkey: {
		/* KJD - need to complete this */
		u_long t;
		int mode, err, keysize;

		/* Algorithm name. */
		T(addname(msg, msglen, &rdata, origin, &buf, &buflen));
		T(addstr(" ", 1, &buf, &buflen));

		/* Inception. */
		t = ns_get32(rdata);  rdata += NS_INT32SZ;
		len = SPRINTF((tmp, "%s ", p_secstodate(t)));
		T(addstr(tmp, len, &buf, &buflen));

		/* Experation. */
		t = ns_get32(rdata);  rdata += NS_INT32SZ;
		len = SPRINTF((tmp, "%s ", p_secstodate(t)));
		T(addstr(tmp, len, &buf, &buflen));

		/* Mode , Error, Key Size. */
		/* Priority, Weight, Port. */
		mode = ns_get16(rdata);  rdata += NS_INT16SZ;
		err  = ns_get16(rdata);  rdata += NS_INT16SZ;
		keysize  = ns_get16(rdata);  rdata += NS_INT16SZ;
		len = SPRINTF((tmp, "%u %u %u ", mode, err, keysize));
		T(addstr(tmp, len, &buf, &buflen));

		/* XXX need to dump key, print otherdata length & other data */
		break;
	    }

	case ns_t_tsig: {
		/* BEW - need to complete this */
		int n;

		T(len = addname(msg, msglen, &rdata, origin, &buf, &buflen));
		T(addstr(" ", 1, &buf, &buflen));
		rdata += 8; /*%< time */
		n = ns_get16(rdata); rdata += INT16SZ;
		rdata += n; /*%< sig */
		n = ns_get16(rdata); rdata += INT16SZ; /*%< original id */
		sprintf(buf, "%d", ns_get16(rdata));
		rdata += INT16SZ;
		addlen(strlen(buf), &buf, &buflen);
		break;
	    }

	case ns_t_a6: {
		struct in6_addr a;
		int pbyte, pbit;

		/* prefix length */
		if (rdlen == 0U) goto formerr;
		len = SPRINTF((tmp, "%d ", *rdata));
		T(addstr(tmp, len, &buf, &buflen));
		pbit = *rdata;
		if (pbit > 128) goto formerr;
		pbyte = (pbit & ~7) / 8;
		rdata++;

		/* address suffix: provided only when prefix len != 128 */
		if (pbit < 128) {
			if (rdata + pbyte >= edata) goto formerr;
			memset(&a, 0, sizeof(a));
			memcpy(&a.s6_addr[pbyte], rdata, sizeof(a) - pbyte);
			(void) inet_ntop(AF_INET6, &a, buf, buflen);
			addlen(strlen(buf), &buf, &buflen);
			rdata += sizeof(a) - pbyte;
		}

		/* prefix name: provided only when prefix len > 0 */
		if (pbit == 0)
			break;
		if (rdata >= edata) goto formerr;
		T(addstr(" ", 1, &buf, &buflen));
		T(addname(msg, msglen, &rdata, origin, &buf, &buflen));
		
		break;
	    }

	case ns_t_opt: {
		len = SPRINTF((tmp, "%u bytes", class));
		T(addstr(tmp, len, &buf, &buflen));
		break;
	    }

	case ns_t_ds:
	case ns_t_dlv:
	case ns_t_sshfp: {
		u_int t;

		if (type == ns_t_ds || type == ns_t_dlv) {
			if (rdlen < 4U) goto formerr;
			t = ns_get16(rdata);
			rdata += NS_INT16SZ;
			len = SPRINTF((tmp, "%u ", t));
			T(addstr(tmp, len, &buf, &buflen));
		} else
			if (rdlen < 2U) goto formerr;

		len = SPRINTF((tmp, "%u ", *rdata));
		T(addstr(tmp, len, &buf, &buflen));
		rdata++;

		len = SPRINTF((tmp, "%u ", *rdata));
		T(addstr(tmp, len, &buf, &buflen));
		rdata++;

		while (rdata < edata) {
			len = SPRINTF((tmp, "%02X", *rdata));
			T(addstr(tmp, len, &buf, &buflen));
			rdata++;
		}
		break;
	    }

	case ns_t_nsec3:
	case ns_t_nsec3param: {
		u_int t, w, l, j, k, c;
		
		len = SPRINTF((tmp, "%u ", *rdata));
		T(addstr(tmp, len, &buf, &buflen));
		rdata++;

		len = SPRINTF((tmp, "%u ", *rdata));
		T(addstr(tmp, len, &buf, &buflen));
		rdata++;

		t = ns_get16(rdata);
		rdata += NS_INT16SZ;
		len = SPRINTF((tmp, "%u ", t));
		T(addstr(tmp, len, &buf, &buflen));

		t = *rdata++;
		if (t == 0) {
			T(addstr("-", 1, &buf, &buflen));
		} else {
			while (t-- > 0) {
				len = SPRINTF((tmp, "%02X", *rdata));
				T(addstr(tmp, len, &buf, &buflen));
				rdata++;
			}
		}
		if (type == ns_t_nsec3param)
			break;
		T(addstr(" ", 1, &buf, &buflen));

		t = *rdata++;
		while (t > 0) {
			switch (t) {
			case 1:
				tmp[0] = base32hex[((rdata[0]>>3)&0x1f)];
				tmp[1] = base32hex[((rdata[0]<<2)&0x1c)];
				tmp[2] = tmp[3] = tmp[4] = '=';
				tmp[5] = tmp[6] = tmp[7] = '=';
				break;
			case 2:
				tmp[0] = base32hex[((rdata[0]>>3)&0x1f)];
				tmp[1] = base32hex[((rdata[0]<<2)&0x1c)|
						   ((rdata[1]>>6)&0x03)];
				tmp[2] = base32hex[((rdata[1]>>1)&0x1f)];
				tmp[3] = base32hex[((rdata[1]<<4)&0x10)];
				tmp[4] = tmp[5] = tmp[6] = tmp[7] = '=';
				break;
			case 3:
				tmp[0] = base32hex[((rdata[0]>>3)&0x1f)];
				tmp[1] = base32hex[((rdata[0]<<2)&0x1c)|
						   ((rdata[1]>>6)&0x03)];
				tmp[2] = base32hex[((rdata[1]>>1)&0x1f)];
				tmp[3] = base32hex[((rdata[1]<<4)&0x10)|
						   ((rdata[2]>>4)&0x0f)];
				tmp[4] = base32hex[((rdata[2]<<1)&0x1e)];
				tmp[5] = tmp[6] = tmp[7] = '=';
				break;
			case 4:
				tmp[0] = base32hex[((rdata[0]>>3)&0x1f)];
				tmp[1] = base32hex[((rdata[0]<<2)&0x1c)|
						   ((rdata[1]>>6)&0x03)];
				tmp[2] = base32hex[((rdata[1]>>1)&0x1f)];
				tmp[3] = base32hex[((rdata[1]<<4)&0x10)|
						   ((rdata[2]>>4)&0x0f)];
				tmp[4] = base32hex[((rdata[2]<<1)&0x1e)|
						   ((rdata[3]>>7)&0x01)];
				tmp[5] = base32hex[((rdata[3]>>2)&0x1f)];
				tmp[6] = base32hex[(rdata[3]<<3)&0x18];
				tmp[7] = '=';
				break;
			default:
				tmp[0] = base32hex[((rdata[0]>>3)&0x1f)];
				tmp[1] = base32hex[((rdata[0]<<2)&0x1c)|
						   ((rdata[1]>>6)&0x03)];
				tmp[2] = base32hex[((rdata[1]>>1)&0x1f)];
				tmp[3] = base32hex[((rdata[1]<<4)&0x10)|
						   ((rdata[2]>>4)&0x0f)];
				tmp[4] = base32hex[((rdata[2]<<1)&0x1e)|
						   ((rdata[3]>>7)&0x01)];
				tmp[5] = base32hex[((rdata[3]>>2)&0x1f)];
				tmp[6] = base32hex[((rdata[3]<<3)&0x18)|
						   ((rdata[4]>>5)&0x07)];
				tmp[7] = base32hex[(rdata[4]&0x1f)];
				break;
			}
			T(addstr(tmp, 8, &buf, &buflen));
			if (t >= 5) {
				rdata += 5;
				t -= 5;
			} else {
				rdata += t;
				t -= t;
			}
		}

		while (rdata < edata) {
			w = *rdata++;
			l = *rdata++;
			for (j = 0; j < l; j++) {
				if (rdata[j] == 0)
					continue;
				for (k = 0; k < 8; k++) {
					if ((rdata[j] & (0x80 >> k)) == 0)
						continue;
					c = w * 256 + j * 8 + k;
					len = SPRINTF((tmp, " %s", p_type(c)));
					T(addstr(tmp, len, &buf, &buflen));
				}
			}
			rdata += l;
		}
		break;
	    }

	case ns_t_nsec: {
		u_int w, l, j, k, c;

		T(addname(msg, msglen, &rdata, origin, &buf, &buflen));

		while (rdata < edata) {
			w = *rdata++;
			l = *rdata++;
			for (j = 0; j < l; j++) {
				if (rdata[j] == 0)
					continue;
				for (k = 0; k < 8; k++) {
					if ((rdata[j] & (0x80 >> k)) == 0)
						continue;
					c = w * 256 + j * 8 + k;
					len = SPRINTF((tmp, " %s", p_type(c)));
					T(addstr(tmp, len, &buf, &buflen));
				}
			}
			rdata += l;
		}
		break;
	    }

	case ns_t_dhcid: {
		int n;
		unsigned int siz;
		char base64_dhcid[8192];
		const char *leader;

		siz = (edata-rdata)*4/3 + 4; /* "+4" accounts for trailing \0 */
		if (siz > sizeof(base64_dhcid) * 3/4) {
			const char *str = "record too long to print";
			T(addstr(str, strlen(str), &buf, &buflen));
		} else {
			len = b64_ntop(rdata, edata-rdata, base64_dhcid, siz);
		
			if (len < 0)
				goto formerr;

			else if (len > 15) {
				T(addstr(" (", 2, &buf, &buflen));
				leader = "\n\t\t";
				spaced = 0;
			}
			else
				leader = " ";

			for (n = 0; n < len; n += 48) {
				T(addstr(leader, strlen(leader),
					 &buf, &buflen));
				T(addstr(base64_dhcid + n, MIN(len - n, 48),
					 &buf, &buflen));
			}
			if (len > 15)
				T(addstr(" )", 2, &buf, &buflen));
		}
		break;
	}

	case ns_t_ipseckey: {
		int n;
		unsigned int siz;
		char base64_key[8192];
		const char *leader;
	
		if (rdlen < 2)
			goto formerr;

		switch (rdata[1]) {
		case 0:
		case 3:
			if (rdlen < 3)
				goto formerr;
			break;
		case 1:
			if (rdlen < 7)
				goto formerr;
			break;
		case 2:
			if (rdlen < 19)
				goto formerr;
			break;
		default:
			comment = "unknown IPSECKEY gateway type";
			goto hexify;
		}

		len = SPRINTF((tmp, "%u ", *rdata));
		T(addstr(tmp, len, &buf, &buflen));
		rdata++;

		len = SPRINTF((tmp, "%u ", *rdata));
		T(addstr(tmp, len, &buf, &buflen));
		rdata++;
		
		len = SPRINTF((tmp, "%u ", *rdata));
		T(addstr(tmp, len, &buf, &buflen));
		rdata++;

		switch (rdata[-2]) {
		case 0:
			T(addstr(".", 1, &buf, &buflen));
			break;
		case 1:
			(void) inet_ntop(AF_INET, rdata, buf, buflen);
			addlen(strlen(buf), &buf, &buflen);
			rdata += 4;
			break;
		case 2:
			(void) inet_ntop(AF_INET6, rdata, buf, buflen);
			addlen(strlen(buf), &buf, &buflen);
			rdata += 16;
			break;
		case 3:
			T(addname(msg, msglen, &rdata, origin, &buf, &buflen));
			break;
		}

		if (rdata >= edata)
			break;

		siz = (edata-rdata)*4/3 + 4; /* "+4" accounts for trailing \0 */
		if (siz > sizeof(base64_key) * 3/4) {
			const char *str = "record too long to print";
			T(addstr(str, strlen(str), &buf, &buflen));
		} else {
			len = b64_ntop(rdata, edata-rdata, base64_key, siz);

			if (len < 0)
				goto formerr;

			else if (len > 15) {
				T(addstr(" (", 2, &buf, &buflen));
				leader = "\n\t\t";
				spaced = 0;
			}
			else
				leader = " ";

			for (n = 0; n < len; n += 48) {
				T(addstr(leader, strlen(leader),
					 &buf, &buflen));
				T(addstr(base64_key + n, MIN(len - n, 48),
					 &buf, &buflen));
			}
			if (len > 15)
				T(addstr(" )", 2, &buf, &buflen));
		}
	}

	case ns_t_hip: {
		unsigned int i, hip_len, algorithm, key_len;
		char base64_key[NS_MD5RSA_MAX_BASE64];
		unsigned int siz;
		const char *leader = "\n\t\t\t\t\t";
		
		hip_len = *rdata++;
		algorithm = *rdata++;
		key_len = ns_get16(rdata);
		rdata += NS_INT16SZ;

		siz = key_len*4/3 + 4; /* "+4" accounts for trailing \0 */
		if (siz > sizeof(base64_key) * 3/4) {
			const char *str = "record too long to print";
			T(addstr(str, strlen(str), &buf, &buflen));
		} else {
			len = sprintf(tmp, "( %u ", algorithm);
			T(addstr(tmp, len, &buf, &buflen));

			for (i = 0; i < hip_len; i++) {
				len = sprintf(tmp, "%02X", *rdata);
				T(addstr(tmp, len, &buf, &buflen));
				rdata++;
			}
			T(addstr(leader, strlen(leader), &buf, &buflen));

			len = b64_ntop(rdata, key_len, base64_key, siz);
			if (len < 0)
				goto formerr;

			T(addstr(base64_key, len, &buf, &buflen));
				
			rdata += key_len;
			while (rdata < edata) {
				T(addstr(leader, strlen(leader), &buf, &buflen));
				T(addname(msg, msglen, &rdata, origin,
					  &buf, &buflen));
			}
			T(addstr(" )", 2, &buf, &buflen));
		}
		break;
	}

	default:
		comment = "unknown RR type";
		goto hexify;
	}
	return (buf - obuf);
 formerr:
	comment = "RR format error";
 hexify: {
	int n, m;
	char *p;

	len = SPRINTF((tmp, "\\# %u%s\t; %s", (unsigned)(edata - rdata),
		       rdlen != 0U ? " (" : "", comment));
	T(addstr(tmp, len, &buf, &buflen));
	while (rdata < edata) {
		p = tmp;
		p += SPRINTF((p, "\n\t"));
		spaced = 0;
		n = MIN(16, edata - rdata);
		for (m = 0; m < n; m++)
			p += SPRINTF((p, "%02x ", rdata[m]));
		T(addstr(tmp, p - tmp, &buf, &buflen));
		if (n < 16) {
			T(addstr(")", 1, &buf, &buflen));
			T(addtab(p - tmp + 1, 48, spaced, &buf, &buflen));
		}
		p = tmp;
		p += SPRINTF((p, "; "));
		for (m = 0; m < n; m++)
			*p++ = (isascii(rdata[m]) && isprint(rdata[m]))
				? rdata[m]
				: '.';
		T(addstr(tmp, p - tmp, &buf, &buflen));
		rdata += n;
	}
	return (buf - obuf);
    }
}

/* Private. */

/*%
 * size_t
 * prune_origin(name, origin)
 *	Find out if the name is at or under the current origin.
 * return:
 *	Number of characters in name before start of origin,
 *	or length of name if origin does not match.
 * notes:
 *	This function should share code with samedomain().
 */
static size_t
prune_origin(const char *name, const char *origin) {
	const char *oname = name;

	while (*name != '\0') {
		if (origin != NULL && ns_samename(name, origin) == 1)
			return (name - oname - (name > oname));
		while (*name != '\0') {
			if (*name == '\\') {
				name++;
				/* XXX need to handle \nnn form. */
				if (*name == '\0')
					break;
			} else if (*name == '.') {
				name++;
				break;
			}
			name++;
		}
	}
	return (name - oname);
}

/*%
 * int
 * charstr(rdata, edata, buf, buflen)
 *	Format a <character-string> into the presentation buffer.
 * return:
 *	Number of rdata octets consumed
 *	0 for protocol format error
 *	-1 for output buffer error
 * side effects:
 *	buffer is advanced on success.
 */
static int
charstr(const u_char *rdata, const u_char *edata, char **buf, size_t *buflen) {
	const u_char *odata = rdata;
	size_t save_buflen = *buflen;
	char *save_buf = *buf;

	if (addstr("\"", 1, buf, buflen) < 0)
		goto enospc;
	if (rdata < edata) {
		int n = *rdata;

		if (rdata + 1 + n <= edata) {
			rdata++;
			while (n-- > 0) {
				if (strchr("\n\"\\", *rdata) != NULL)
					if (addstr("\\", 1, buf, buflen) < 0)
						goto enospc;
				if (addstr((const char *)rdata, 1,
					   buf, buflen) < 0)
					goto enospc;
				rdata++;
			}
		}
	}
	if (addstr("\"", 1, buf, buflen) < 0)
		goto enospc;
	return (rdata - odata);
 enospc:
	errno = ENOSPC;
	*buf = save_buf;
	*buflen = save_buflen;
	return (-1);
}

static int
addname(const u_char *msg, size_t msglen,
	const u_char **pp, const char *origin,
	char **buf, size_t *buflen)
{
	size_t newlen, save_buflen = *buflen;
	char *save_buf = *buf;
	int n;

	n = dn_expand(msg, msg + msglen, *pp, *buf, *buflen);
	if (n < 0)
		goto enospc;	/*%< Guess. */
	newlen = prune_origin(*buf, origin);
	if (**buf == '\0') {
		goto root;
	} else if (newlen == 0U) {
		/* Use "@" instead of name. */
		if (newlen + 2 > *buflen)
			goto enospc;        /* No room for "@\0". */
		(*buf)[newlen++] = '@';
		(*buf)[newlen] = '\0';
	} else {
		if (((origin == NULL || origin[0] == '\0') ||
		    (origin[0] != '.' && origin[1] != '\0' &&
		    (*buf)[newlen] == '\0')) && (*buf)[newlen - 1] != '.') {
			/* No trailing dot. */
 root:
			if (newlen + 2 > *buflen)
				goto enospc;	/* No room for ".\0". */
			(*buf)[newlen++] = '.';
			(*buf)[newlen] = '\0';
		}
	}
	*pp += n;
	addlen(newlen, buf, buflen);
	**buf = '\0';
	return (newlen);
 enospc:
	errno = ENOSPC;
	*buf = save_buf;
	*buflen = save_buflen;
	return (-1);
}

static void
addlen(size_t len, char **buf, size_t *buflen) {
	INSIST(len <= *buflen);
	*buf += len;
	*buflen -= len;
}

static int
addstr(const char *src, size_t len, char **buf, size_t *buflen) {
	if (len >= *buflen) {
		errno = ENOSPC;
		return (-1);
	}
	memcpy(*buf, src, len);
	addlen(len, buf, buflen);
	**buf = '\0';
	return (0);
}

static int
addtab(size_t len, size_t target, int spaced, char **buf, size_t *buflen) {
	size_t save_buflen = *buflen;
	char *save_buf = *buf;
	int t;

	if (spaced || len >= target - 1) {
		T(addstr("  ", 2, buf, buflen));
		spaced = 1;
	} else {
		for (t = (target - len - 1) / 8; t >= 0; t--)
			if (addstr("\t", 1, buf, buflen) < 0) {
				*buflen = save_buflen;
				*buf = save_buf;
				return (-1);
			}
		spaced = 0;
	}
	return (spaced);
}

/*! \file */
} // namespace pbsd::lib_libc_nameser::b0338
