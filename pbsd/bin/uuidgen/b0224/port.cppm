/*
 * Copyright (c) 2002 Marcel Moolenaar
 * Copyright (c) 2022 Tobias C. Berner
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
 *
 */

module;

#define _GNU_SOURCE
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

export module pbsd.bin.uuidgen.b0224;

export namespace pbsd::bin_uuidgen::b0224 {

struct uuid {
	std::uint32_t	time_low;
	std::uint16_t	time_mid;
	std::uint16_t	time_hi_and_version;
	std::uint8_t	clock_seq_hi_and_reserved;
	std::uint8_t	clock_seq_low;
	std::uint8_t	node[6];
};

typedef struct uuid uuid_t;

constexpr std::uint32_t uuid_s_ok = 0;
constexpr std::uint32_t uuid_s_no_memory = 3;

extern "C" void arc4random_buf(void *buf, size_t len);

static void
uuid_create_nil(uuid_t *u, std::uint32_t *status)
{

	if (status)
		*status = uuid_s_ok;

	std::memset(u, 0, sizeof(*u));
}

void
uuid_to_compact_string(const uuid_t *u, char **s, std::uint32_t *status)
{
	uuid_t nil;

	if (status != NULL)
		*status = uuid_s_ok;

	if (s == NULL)
		return;

	if (u == NULL) {
		u = &nil;
		uuid_create_nil(&nil, NULL);
	}

	asprintf(s, "%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",
	    u->time_low, u->time_mid, u->time_hi_and_version,
	    u->clock_seq_hi_and_reserved, u->clock_seq_low, u->node[0],
	    u->node[1], u->node[2], u->node[3], u->node[4], u->node[5]);

	if (*s == NULL && status != NULL)
		*status = uuid_s_no_memory;
}

int
uuidgen_v4(struct uuid *store, int count)
{
	int size;
	struct uuid *item;

	if (count < 1) {
		errno = EINVAL;
		return (-1);
	}
	size = sizeof(struct uuid) * count;
	arc4random_buf(store, size);
	item = store;
	for (int i = 0; i < count; ++i) {
		/*
		 * Set the two most significant bits (bits 6 and 7) of the
		 * clock_seq_hi_and_reserved to zero and one, respectively.
		 */
		item->clock_seq_hi_and_reserved &= ~(3 << 6);
		item->clock_seq_hi_and_reserved |= (2 << 6);
		/*
		 * Set the four most significant bits (bits 12 through 15) of
		 * the time_hi_and_version field to the 4-bit version number
		 * from  Section 4.1.3.
		 */
		item->time_hi_and_version &= ~(15 << 12);
		item->time_hi_and_version |= (4 << 12);
		item++;
	};
	return (0);
}

} // namespace pbsd::bin_uuidgen::b0224
