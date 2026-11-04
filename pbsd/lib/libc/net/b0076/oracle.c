/*
 * Reference oracle for batch b0076: the original HardenedBSD sources
 * concatenated, with every function (and, for vars.c, every exported object)
 * renamed with a "ref_" prefix.  Bodies and initialisers are unmodified; only
 * host-portability defines have been added.
 */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2006 Olivier Houchard
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* #include <sys/endian.h> -- absent on non-BSD hosts, see defines below */
#include <stdint.h>
#define _BYTEORDER_FUNC_DEFINED
#include <arpa/inet.h>

/* sys/endian.h + machine/endian.h: __bswap16/__bswap32 and the ntoh aliases. */
#ifndef __bswap16
#define	__bswap16(x)							\
	((uint16_t)((((uint16_t)(x)) >> 8) | (((uint16_t)(x)) << 8)))
#endif
#ifndef __bswap32
#define	__bswap32(x)							\
	((uint32_t)((((uint32_t)(x)) >> 24) |				\
	    ((((uint32_t)(x)) >> 8) & 0x0000ff00UL) |			\
	    ((((uint32_t)(x)) << 8) & 0x00ff0000UL) |			\
	    (((uint32_t)(x)) << 24)))
#endif

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#ifndef __htonl
#define	__htonl(x)	((uint32_t)(x))
#define	__htons(x)	((uint16_t)(x))
#define	__ntohl(x)	((uint32_t)(x))
#define	__ntohs(x)	((uint16_t)(x))
#endif
#else
#ifndef __htonl
#define	__htonl(x)	__bswap32(x)
#define	__htons(x)	__bswap16(x)
#define	__ntohl(x)	__bswap32(x)
#define	__ntohs(x)	__bswap16(x)
#endif
#endif

uint32_t
ref_htonl(uint32_t hl)
{

	return (__htonl(hl));
}

uint16_t
ref_htons(uint16_t hs)
{
	
	return (__htons(hs));
}

uint32_t
ref_ntohl(uint32_t nl)
{

	return (__ntohl(nl));
}

uint16_t
ref_ntohs(uint16_t ns)
{
	
	return (__ntohs(ns));
}

/*	$KAME: vars.c,v 1.2 2001/08/20 02:32:41 itojun Exp $	*/

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/types.h>
#include <netinet/in.h>

/* KAME/FreeBSD IN6ADDR_*_INIT macros, for hosts lacking them. */
#ifndef IN6ADDR_ANY_INIT
#define	IN6ADDR_ANY_INIT						\
	{{{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }}}
#endif
#ifndef IN6ADDR_LOOPBACK_INIT
#define	IN6ADDR_LOOPBACK_INIT						\
	{{{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }}}
#endif
#ifndef IN6ADDR_NODELOCAL_ALLNODES_INIT
#define	IN6ADDR_NODELOCAL_ALLNODES_INIT					\
	{{{ 0xff, 0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x01 }}}
#endif
#ifndef IN6ADDR_LINKLOCAL_ALLNODES_INIT
#define	IN6ADDR_LINKLOCAL_ALLNODES_INIT					\
	{{{ 0xff, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x01 }}}
#endif

/*
 * Definitions of some costant IPv6 addresses.
 */
const struct in6_addr ref_in6addr_any = IN6ADDR_ANY_INIT;
const struct in6_addr ref_in6addr_loopback = IN6ADDR_LOOPBACK_INIT;
const struct in6_addr ref_in6addr_nodelocal_allnodes = IN6ADDR_NODELOCAL_ALLNODES_INIT;
const struct in6_addr ref_in6addr_linklocal_allnodes = IN6ADDR_LINKLOCAL_ALLNODES_INIT;
