/*
 * Batch b0007 -- C++23 port of hbsd/src/sys/security/mac_ntpd/mac_ntpd.c
 */

export module pbsd.sys.security.mac.ntpd.b0007;

namespace pbsd::sys_security_mac_ntpd::b0007 {

using uid_t = unsigned int;

inline constexpr int PRIV_ADJTIME = 15;
inline constexpr int PRIV_CLOCK_SETTIME = 17;
inline constexpr int PRIV_NTP_ADJTIME = 16;
inline constexpr int PRIV_NETINET_RESERVEDPORT = 490;
inline constexpr int PRIV_NETINET_REUSEPORT = 504;
inline constexpr int EPERM = 1;

static int ntpd_enabled = 1;
static int ntpd_uid = 123;

} /* namespace pbsd::sys_security_mac_ntpd::b0007 */

export namespace pbsd::sys_security_mac_ntpd::b0007 {

struct ucred {
	unsigned int	cr_uid;			/* effective user id */
};

void
set_ntpd_enabled(int v)
{
	ntpd_enabled = v;
}

void
set_ntpd_uid(int v)
{
	ntpd_uid = v;
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
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

int
ntpd_priv_grant(struct ucred *cred, int priv)
{

	if (ntpd_enabled && cred->cr_uid == ntpd_uid) {
		switch (priv) {
		case PRIV_ADJTIME:
		case PRIV_CLOCK_SETTIME:
		case PRIV_NTP_ADJTIME:
		case PRIV_NETINET_RESERVEDPORT:
		case PRIV_NETINET_REUSEPORT:
			return (0);
		default:
			break;
		}
	}
	return (EPERM);
}

} /* namespace pbsd::sys_security_mac_ntpd::b0007 */
