/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2011, 2012, 2013, 2015, 2016, Juniper Networks, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

module;

#include <cstddef>
#include <cstdint>

export module pbsd.sys.security.mac.veriexec.b0029;

export namespace pbsd::sys_security_mac_veriexec::b0029 {

typedef char *caddr_t;

struct sha1_ctxt {
	union {
		std::uint8_t	b8[20];
		std::uint32_t	b32[5];
	} h;
	union {
		std::uint8_t	b8[8];
		std::uint64_t	b64[1];
	} c;
	union {
		std::uint8_t	b8[64];
		std::uint32_t	b32[16];
	} m;
	std::uint8_t	count;
};
typedef struct sha1_ctxt SHA1_CTX;

extern "C" void sha1_result(struct sha1_ctxt *, char *);

void
SHA1_Final(unsigned char *buf, void *ctx)
{

	sha1_result((SHA1_CTX *) ctx, (caddr_t) buf);
}

} /* namespace pbsd::sys_security_mac_veriexec::b0029 */
