/*
 * Batch b0007 oracle: hbsd/src/sys/security/mac_ntpd/mac_ntpd.c
 *
 * Every function renamed with a ref_ prefix; bodies otherwise UNMODIFIED.
 * Kernel-header-supplied definitions the bodies rely on are supplied here so
 * the file builds standalone with cc -std=c11.
 */

#include <stdint.h>

/* sys/_types.h */
typedef uint32_t uid_t;

/* sys/ucred.h */
struct ucred {
	uid_t	cr_uid;			/* effective user id */
};

/* sys/priv.h */
#define	PRIV_ADJTIME			15
#define	PRIV_CLOCK_SETTIME		17
#define	PRIV_NTP_ADJTIME		16
#define	PRIV_NETINET_RESERVEDPORT	490
#define	PRIV_NETINET_REUSEPORT		504

/* sys/errno.h */
#define	EPERM				1

static int ntpd_enabled = 1;
static int ntpd_uid = 123;

void
ref_set_ntpd_enabled(int v)
{
	ntpd_enabled = v;
}

void
ref_set_ntpd_uid(int v)
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
ref_ntpd_priv_grant(struct ucred *cred, int priv)
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
