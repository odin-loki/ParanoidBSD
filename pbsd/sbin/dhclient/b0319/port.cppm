module;

#define _DEFAULT_SOURCE
#include <sys/types.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <arpa/inet.h>
#include <netinet/in.h>

export module pbsd.sbin.dhclient.b0319;

export namespace pbsd::sbin_dhclient::b0319 {

using caddr_t = char *;

struct pair_node {
	caddr_t car;
	pair_node *cdr;
};

using pair = pair_node *;

#define DEFAULT_HASH_SIZE 97

struct string_list {
	string_list *next;
	char *string;
};

struct hash_bucket {
	hash_bucket *next;
	const unsigned char *name;
	int len;
	unsigned char *value;
};

struct hash_table {
	int hash_count;
	hash_bucket *buckets[DEFAULT_HASH_SIZE];
};

struct iaddr {
	size_t len;
	unsigned char iabuf[16];
};

[[noreturn]] static void
error(const char *fmt, ...)
{
	(void)fmt;
	std::abort();
}

/*	$OpenBSD: tree.c,v 1.13 2004/05/06 22:29:15 deraadt Exp $	*/

/* Routines for manipulating parse trees... */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1995, 1996, 1997 The Internet Software Consortium.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of The Internet Software Consortium nor the names
 *    of its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INTERNET SOFTWARE CONSORTIUM AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE INTERNET SOFTWARE CONSORTIUM OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This software has been written for the Internet Software Consortium
 * by Ted Lemon <mellon@fugue.com> in cooperation with Vixie
 * Enterprises.  To learn more about the Internet Software Consortium,
 * see ``http://www.vix.com/isc''.  To learn more about Vixie
 * Enterprises, see ``http://www.vix.com''.
 */

pair
cons(caddr_t car, pair cdr)
{
	pair foo = (pair)calloc(1, sizeof(*foo));
	if (!foo)
		error("no memory for cons.");
	foo->car = car;
	foo->cdr = cdr;
	return (foo);
}

/*	$OpenBSD: alloc.c,v 1.9 2004/05/04 20:28:40 deraadt Exp $	*/

/* Memory allocation... */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1995, 1996, 1998 The Internet Software Consortium.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of The Internet Software Consortium nor the names
 *    of its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INTERNET SOFTWARE CONSORTIUM AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE INTERNET SOFTWARE CONSORTIUM OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This software has been written for the Internet Software Consortium
 * by Ted Lemon <mellon@fugue.com> in cooperation with Vixie
 * Enterprises.  To learn more about the Internet Software Consortium,
 * see ``http://www.vix.com/isc''.  To learn more about Vixie
 * Enterprises, see ``http://www.vix.com''.
 */

string_list *
new_string_list(size_t size)
{
	string_list *rval;

	rval = (string_list *)calloc(1, sizeof(string_list) + size);
	if (rval != NULL)
		rval->string = ((char *)rval) + sizeof(string_list);
	return (rval);
}

hash_table *
new_hash_table(int count)
{
	hash_table *rval;

	rval = (hash_table *)calloc(1, sizeof(hash_table) -
	    (DEFAULT_HASH_SIZE * sizeof(hash_bucket *)) +
	    (count * sizeof(hash_bucket *)));
	if (rval == NULL)
		return (NULL);
	rval->hash_count = count;
	return (rval);
}

hash_bucket *
new_hash_bucket(void)
{
	hash_bucket *rval = (hash_bucket *)calloc(1, sizeof(hash_bucket));

	return (rval);
}

/*	$OpenBSD: convert.c,v 1.5 2004/02/07 11:35:59 henning Exp $	*/

/*
 * Safe copying of option values into and out of the option buffer,
 * which can't be assumed to be aligned.
 */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1995, 1996 The Internet Software Consortium.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of The Internet Software Consortium nor the names
 *    of its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INTERNET SOFTWARE CONSORTIUM AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE INTERNET SOFTWARE CONSORTIUM OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This software has been written for the Internet Software Consortium
 * by Ted Lemon <mellon@fugue.com> in cooperation with Vixie
 * Enterprises.  To learn more about the Internet Software Consortium,
 * see ``http://www.vix.com/isc''.  To learn more about Vixie
 * Enterprises, see ``http://www.vix.com''.
 */

uint32_t
getULong(unsigned char *buf)
{
	uint32_t ibuf;

	memcpy(&ibuf, buf, sizeof(ibuf));
	return (ntohl(ibuf));
}

int32_t
getLong(unsigned char *(buf))
{
	int32_t ibuf;

	memcpy(&ibuf, buf, sizeof(ibuf));
	return (ntohl(ibuf));
}

uint16_t
getUShort(unsigned char *buf)
{
	uint16_t ibuf;

	memcpy(&ibuf, buf, sizeof(ibuf));
	return (ntohs(ibuf));
}

int16_t
getShort(unsigned char *buf)
{
	int16_t ibuf;

	memcpy(&ibuf, buf, sizeof(ibuf));
	return (ntohs(ibuf));
}

void
putULong(unsigned char *obuf, uint32_t val)
{
	uint32_t tmp = htonl(val);

	memcpy(obuf, &tmp, sizeof(tmp));
}

void
putLong(unsigned char *obuf, int32_t val)
{
	int32_t tmp = htonl(val);

	memcpy(obuf, &tmp, sizeof(tmp));
}

void
putUShort(unsigned char *obuf, unsigned int val)
{
	uint16_t tmp = htons(val);

	memcpy(obuf, &tmp, sizeof(tmp));
}

void
putShort(unsigned char *obuf, int val)
{
	int16_t tmp = htons(val);

	memcpy(obuf, &tmp, sizeof(tmp));
}

/*	$OpenBSD: inet.c,v 1.7 2004/05/04 21:48:16 deraadt Exp $	*/

/*
 * Subroutines to manipulate internet addresses in a safely portable
 * way...
 */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1996 The Internet Software Consortium.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of The Internet Software Consortium nor the names
 *    of its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INTERNET SOFTWARE CONSORTIUM AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE INTERNET SOFTWARE CONSORTIUM OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This software has been written for the Internet Software Consortium
 * by Ted Lemon <mellon@fugue.com> in cooperation with Vixie
 * Enterprises.  To learn more about the Internet Software Consortium,
 * see ``http://www.vix.com/isc''.  To learn more about Vixie
 * Enterprises, see ``http://www.vix.com''.
 */

/*
 * Return just the network number of an internet address...
 */
iaddr
subnet_number(iaddr addr, iaddr mask)
{
	iaddr rv;
	unsigned i;

	rv.len = 0;

	/* Both addresses must have the same length... */
	if (addr.len != mask.len)
		return (rv);

	rv.len = addr.len;
	for (i = 0; i < rv.len; i++)
		rv.iabuf[i] = addr.iabuf[i] & mask.iabuf[i];
	return (rv);
}

/*
 * Given a subnet number and netmask, return the address on that subnet
 * for which the host portion of the address is all ones (the standard
 * broadcast address).
 */
iaddr
broadcast_addr(iaddr subnet, iaddr mask)
{
	iaddr rv;
	unsigned i;

	if (subnet.len != mask.len) {
		rv.len = 0;
		return (rv);
	}

	for (i = 0; i < subnet.len; i++)
		rv.iabuf[i] = subnet.iabuf[i] | (~mask.iabuf[i] & 255);
	rv.len = subnet.len;

	return (rv);
}

int
addr_eq(iaddr addr1, iaddr addr2)
{
	if (addr1.len != addr2.len)
		return (0);
	return (memcmp(addr1.iabuf, addr2.iabuf, addr1.len) == 0);
}

char *
piaddr(iaddr addr)
{
	static char pbuf[32];
	struct in_addr a;
	char *s;

	memcpy(&a, &(addr.iabuf), sizeof(struct in_addr));

	if (addr.len == 0)
		strlcpy(pbuf, "<null address>", sizeof(pbuf));
	else {
		s = inet_ntoa(a);
		if (s != NULL)
			strlcpy(pbuf, s, sizeof(pbuf));
		else
			strlcpy(pbuf, "<invalid address>", sizeof(pbuf));
	}
	return (pbuf);
}

} /* namespace pbsd::sbin_dhclient::b0319 */
