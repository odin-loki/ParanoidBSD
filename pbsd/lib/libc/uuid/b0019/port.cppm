/*	$NetBSD: uuid_stream.c,v 1.3 2008/04/19 18:21:38 plunky Exp $	*/

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002 Marcel Moolenaar
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

module;

#include <cstdint>

export module pbsd.lib.libc.uuid.b0019;

export namespace pbsd::lib_libc_uuid::b0019 {

/* <sys/uuid.h> */
inline constexpr int _UUID_NODE_LEN = 6;

struct uuid {
	std::uint32_t	time_low;
	std::uint16_t	time_mid;
	std::uint16_t	time_hi_and_version;
	std::uint8_t	clock_seq_hi_and_reserved;
	std::uint8_t	clock_seq_low;
	std::uint8_t	node[_UUID_NODE_LEN];
};

using uuid_t = struct uuid;

/* <sys/endian.h> */
inline std::uint16_t
be16dec(const void *pp)
{
	std::uint8_t const *p = (std::uint8_t const *)pp;

	return ((p[0] << 8) | p[1]);
}

inline std::uint32_t
be32dec(const void *pp)
{
	std::uint8_t const *p = (std::uint8_t const *)pp;

	return (((unsigned)p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}

inline std::uint16_t
le16dec(const void *pp)
{
	std::uint8_t const *p = (std::uint8_t const *)pp;

	return ((p[1] << 8) | p[0]);
}

inline std::uint32_t
le32dec(const void *pp)
{
	std::uint8_t const *p = (std::uint8_t const *)pp;

	return (((unsigned)p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0]);
}

inline void
be16enc(void *pp, std::uint16_t u)
{
	std::uint8_t *p = (std::uint8_t *)pp;

	p[0] = (u >> 8) & 0xff;
	p[1] = u & 0xff;
}

inline void
be32enc(void *pp, std::uint32_t u)
{
	std::uint8_t *p = (std::uint8_t *)pp;

	p[0] = (u >> 24) & 0xff;
	p[1] = (u >> 16) & 0xff;
	p[2] = (u >> 8) & 0xff;
	p[3] = u & 0xff;
}

inline void
le16enc(void *pp, std::uint16_t u)
{
	std::uint8_t *p = (std::uint8_t *)pp;

	p[0] = u & 0xff;
	p[1] = (u >> 8) & 0xff;
}

inline void
le32enc(void *pp, std::uint32_t u)
{
	std::uint8_t *p = (std::uint8_t *)pp;

	p[0] = u & 0xff;
	p[1] = (u >> 8) & 0xff;
	p[2] = (u >> 16) & 0xff;
	p[3] = (u >> 24) & 0xff;
}

/*
 * Encode/Decode UUID into octet-stream.
 *   http://www.opengroup.org/dce/info/draft-leach-uuids-guids-01.txt
 *
 * 0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                          time_low                             |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |       time_mid                |         time_hi_and_version   |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |clk_seq_hi_res |  clk_seq_low  |         node (0-1)            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                         node (2-5)                            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * NOTE: These routines are not part of the DCE RPC API. They are
 * provided for convenience.
 */

void
uuid_enc_le(void *buf, const uuid_t *uuid)
{
	std::uint8_t *p = (std::uint8_t *)buf;
	int i;

	le32enc(p, uuid->time_low);
	le16enc(p + 4, uuid->time_mid);
	le16enc(p + 6, uuid->time_hi_and_version);
	p[8] = uuid->clock_seq_hi_and_reserved;
	p[9] = uuid->clock_seq_low;
	for (i = 0; i < _UUID_NODE_LEN; i++)
		p[10 + i] = uuid->node[i];
}

void
uuid_dec_le(const void *buf, uuid_t *uuid)
{
	const std::uint8_t *p = (const std::uint8_t *)buf;
	int i;

	uuid->time_low = le32dec(p);
	uuid->time_mid = le16dec(p + 4);
	uuid->time_hi_and_version = le16dec(p + 6);
	uuid->clock_seq_hi_and_reserved = p[8];
	uuid->clock_seq_low = p[9];
	for (i = 0; i < _UUID_NODE_LEN; i++)
		uuid->node[i] = p[10 + i];
}

void
uuid_enc_be(void *buf, const uuid_t *uuid)
{
	std::uint8_t *p = (std::uint8_t *)buf;
	int i;

	be32enc(p, uuid->time_low);
	be16enc(p + 4, uuid->time_mid);
	be16enc(p + 6, uuid->time_hi_and_version);
	p[8] = uuid->clock_seq_hi_and_reserved;
	p[9] = uuid->clock_seq_low;
	for (i = 0; i < _UUID_NODE_LEN; i++)
		p[10 + i] = uuid->node[i];
}

void
uuid_dec_be(const void *buf, uuid_t *uuid)
{
	const std::uint8_t *p = (const std::uint8_t *)buf;
	int i;

	uuid->time_low = be32dec(p);
	uuid->time_mid = be16dec(p + 4);
	uuid->time_hi_and_version = be16dec(p + 6);
	uuid->clock_seq_hi_and_reserved = p[8];
	uuid->clock_seq_low = p[9];
	for (i = 0; i < _UUID_NODE_LEN; i++)
		uuid->node[i] = p[10 + i];
}

} /* namespace pbsd::lib_libc_uuid::b0019 */
