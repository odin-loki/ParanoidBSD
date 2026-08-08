/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Semihalf, Stormshield
 * Copyright (c) 2018 Ian Lepore <ian@FreeBSD.org>
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

module;

#include <cstdint>

export module pbsd.sys.security.mac.pimd.b0006;

export namespace pbsd::sys_security_mac_pimd::b0006 {

inline constexpr int EPERM = 1;
inline constexpr int PRIV_NETINET_MROUTE = 496;

using uid_t = std::uint32_t;

struct ucred {
	uid_t	cr_uid;
};

inline int pimd_enabled = 0;
inline int pimd_uid = 0;

static int
pimd_priv_grant(struct ucred *cred, int priv)
{

	if (pimd_enabled && cred->cr_uid == pimd_uid) {
		switch (priv) {
		case PRIV_NETINET_MROUTE:
			return (0);
		default:
			break;
		}
	}
	return (EPERM);
}

int
pimd_priv_grant_export(struct ucred *cred, int priv)
{

	return (pimd_priv_grant(cred, priv));
}

} // namespace pbsd::sys_security_mac_pimd::b0006
