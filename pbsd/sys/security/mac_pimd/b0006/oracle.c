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

/*
 * oracle.c -- reference implementation for PBSD batch b0006.
 *
 * hbsd/src/sys/security/mac_pimd/mac_pimd.c concatenated, with each
 * ported function renamed with a ref_ prefix.  Function bodies are
 * UNMODIFIED.  This file is the specification.
 *
 * SYSCTL_NODE, SYSCTL_INT and MAC_POLICY_SET are kernel module
 * registration macros, not portable functions; they are omitted.
 */

#include <stdint.h>

#ifndef LONG_BIT
#define LONG_BIT	(sizeof(long) * 8)
#endif

#ifndef EPERM
#define EPERM	1
#endif

#ifndef PRIV_NETINET_MROUTE
#define PRIV_NETINET_MROUTE	496
#endif

typedef uint32_t uid_t;

struct ucred {
	uid_t	cr_uid;
};

int ref_pimd_enabled = 0;
int ref_pimd_uid = 0;

#define pimd_enabled	ref_pimd_enabled
#define pimd_uid	ref_pimd_uid

static int
ref_pimd_priv_grant(struct ucred *cred, int priv)
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
ref_pimd_priv_grant_export(struct ucred *cred, int priv)
{

	return (ref_pimd_priv_grant(cred, priv));
}
