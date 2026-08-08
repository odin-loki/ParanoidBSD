/*
 * oracle.c -- reference implementation for PBSD batch b0006.
 *
 * Source: hbsd/src/sys/security/mac_pimd/mac_pimd.c
 *
 * The C source is reproduced verbatim below with only these mechanical
 * changes, none of which touch a function body:
 *   - the kernel #include block is replaced by the shim definitions the
 *     bodies need (struct ucred, struct mac_policy_ops, PRIV_NETINET_MROUTE,
 *     EPERM and no-op SYSCTL_* / MAC_POLICY_SET macros);
 *   - functions gain a ref_ prefix and lose "static" so the harness can link
 *     against them;
 *   - the two sysctl-backed variables lose "static" so the harness can drive
 *     them.  Their names and initialisers are untouched.
 */

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

#include <errno.h>
#include <limits.h>
#include <stdint.h>

/* --- shims standing in for the kernel headers ------------------------- */

#ifndef LONG_BIT
#define	LONG_BIT	(sizeof(long) * 8)
#endif

#ifndef PRIV_NETINET_MROUTE
#define	PRIV_NETINET_MROUTE	496	/* Administer multicast routing. */
#endif

#ifndef EPERM
#define	EPERM	1
#endif

typedef unsigned int pbsd_u_int;
typedef uint32_t pbsd_uid_t;
typedef uint32_t pbsd_gid_t;

#define	u_int	pbsd_u_int
#define	uid_t	pbsd_uid_t
#define	gid_t	pbsd_gid_t

struct ucred {
	u_int	cr_ref;
	uid_t	cr_uid;
	uid_t	cr_ruid;
	uid_t	cr_svuid;
	gid_t	cr_rgid;
	gid_t	cr_svgid;
};

struct mac_policy_ops {
	int (*mpo_priv_grant)(struct ucred *cred, int priv);
};

#define	SYSCTL_NODE(parent, nbr, name, access, handler, descr)		\
	int pbsd_sysctl_node_##name##_unused __attribute__((unused))

#define	SYSCTL_INT(parent, nbr, name, access, ptr, val, descr)		\
	static int pbsd_sysctl_int_##name##_unused __attribute__((unused))

#define	MAC_POLICY_SET(ops, mpname, fullname, flags, privdata)		\
	static int pbsd_mac_policy_set_##mpname##_unused __attribute__((unused))

/* --- original translation unit ---------------------------------------- */

static SYSCTL_NODE(_security_mac, OID_AUTO, pimd,
    CTLFLAG_RW | CTLFLAG_MPSAFE, 0,
    "mac_pimd policy controls");

int pimd_enabled = 0;
SYSCTL_INT(_security_mac_pimd, OID_AUTO, enabled, CTLFLAG_RWTUN,
    &pimd_enabled, 0, "Enable mac_pimd policy");

int pimd_uid = 0;
SYSCTL_INT(_security_mac_pimd, OID_AUTO, uid, CTLFLAG_RWTUN,
    &pimd_uid, 0, "User id for pimd user");

int
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

static struct mac_policy_ops pimd_ops __attribute__((unused)) =
{
	.mpo_priv_grant = ref_pimd_priv_grant,
};

MAC_POLICY_SET(&pimd_ops, mac_pimd, "MAC/pimd",
    MPC_LOADTIME_FLAG_UNLOADOK, NULL);
