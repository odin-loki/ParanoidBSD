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

module;

#include <netinet/in.h>

#include <bit>
#include <cstdint>

/*
 * <netinet/in.h> defines the byte order routines as function-like macros when
 * the host libc feels like it; FreeBSD's ntoh.c suppresses them by defining
 * _BYTEORDER_FUNC_DEFINED before including <arpa/inet.h>.
 */
#undef htonl
#undef htons
#undef ntohl
#undef ntohs

export module pbsd.lib.libc.net.b0076;

namespace pbsd::lib_libc_net::b0076 {

/*
 * sys/endian.h and machine/endian.h equivalents: __bswap16()/__bswap32() and
 * the __htonl()/__htons()/__ntohl()/__ntohs() aliases, which byte swap on a
 * little-endian host and are the identity on a big-endian one.
 */
namespace detail {

constexpr std::uint16_t
bswap16(std::uint16_t x) noexcept
{

	return (static_cast<std::uint16_t>(((x >> 8) & 0x00ffU) |
	    ((x << 8) & 0xff00U)));
}

constexpr std::uint32_t
bswap32(std::uint32_t x) noexcept
{

	return (((x >> 24) & 0x000000ffUL) | ((x >> 8) & 0x0000ff00UL) |
	    ((x << 8) & 0x00ff0000UL) | ((x << 24) & 0xff000000UL));
}

constexpr std::uint32_t
swap32_if_little(std::uint32_t x) noexcept
{

	if constexpr (std::endian::native == std::endian::little)
		return (bswap32(x));
	else
		return (x);
}

constexpr std::uint16_t
swap16_if_little(std::uint16_t x) noexcept
{

	if constexpr (std::endian::native == std::endian::little)
		return (bswap16(x));
	else
		return (x);
}

constexpr std::uint32_t
__htonl(std::uint32_t x) noexcept
{

	return (swap32_if_little(x));
}

constexpr std::uint16_t
__htons(std::uint16_t x) noexcept
{

	return (swap16_if_little(x));
}

constexpr std::uint32_t
__ntohl(std::uint32_t x) noexcept
{

	return (swap32_if_little(x));
}

constexpr std::uint16_t
__ntohs(std::uint16_t x) noexcept
{

	return (swap16_if_little(x));
}

} /* namespace detail */

export std::uint32_t
htonl(std::uint32_t hl)
{

	return (detail::__htonl(hl));
}

export std::uint16_t
htons(std::uint16_t hs)
{

	return (detail::__htons(hs));
}

export std::uint32_t
ntohl(std::uint32_t nl)
{

	return (detail::__ntohl(nl));
}

export std::uint16_t
ntohs(std::uint16_t ns)
{

	return (detail::__ntohs(ns));
}

/*
 * Definitions of some costant IPv6 addresses.
 *
 * The KAME/FreeBSD IN6ADDR_*_INIT macros are spelled out here for hosts whose
 * <netinet/in.h> lacks them.
 */
#ifndef IN6ADDR_ANY_INIT
#define IN6ADDR_ANY_INIT \
	{{{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }}}
#endif
#ifndef IN6ADDR_LOOPBACK_INIT
#define IN6ADDR_LOOPBACK_INIT \
	{{{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }}}
#endif
#ifndef IN6ADDR_NODELOCAL_ALLNODES_INIT
#define IN6ADDR_NODELOCAL_ALLNODES_INIT \
	{{{ 0xff, 0x01, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x01 }}}
#endif
#ifndef IN6ADDR_LINKLOCAL_ALLNODES_INIT
#define IN6ADDR_LINKLOCAL_ALLNODES_INIT \
	{{{ 0xff, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x01 }}}
#endif

export extern const struct in6_addr in6addr_any = IN6ADDR_ANY_INIT;
export extern const struct in6_addr in6addr_loopback = IN6ADDR_LOOPBACK_INIT;
export extern const struct in6_addr in6addr_nodelocal_allnodes =
    IN6ADDR_NODELOCAL_ALLNODES_INIT;
export extern const struct in6_addr in6addr_linklocal_allnodes =
    IN6ADDR_LINKLOCAL_ALLNODES_INIT;

} /* namespace pbsd::lib_libc_net::b0076 */
