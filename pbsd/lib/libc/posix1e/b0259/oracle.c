/*
 * oracle.c -- reference implementation for batch b0259.
 *
 * Original C sources, concatenated, with every function renamed with a
 * "ref_" prefix.  Function bodies are UNMODIFIED.
 *
 * Sources:
 *   hbsd/src/lib/libc/posix1e/acl_free.c
 *   hbsd/src/lib/libc/posix1e/mac_exec.c
 *   hbsd/src/lib/libc/posix1e/acl_compat.c
 */

#include <sys/types.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#define __sym_compat(sym, impl, ver)

struct mac {
	size_t	 m_buflen;
	char	*m_string;
};

typedef uint32_t	acl_perm_t;
typedef uint32_t	*acl_permset_t;
typedef mode_t		oldacl_perm_t;

extern int __mac_execve(char *fname, char **argv, char **envv,
    struct mac *mac_p);
extern int acl_get_perm_np(acl_permset_t permset_d, acl_perm_t perm);
extern int acl_add_perm(acl_permset_t permset_d, acl_perm_t perm);
extern int acl_delete_perm(acl_permset_t permset_d, acl_perm_t perm);

/* ------------------------------------------------------------------ */
/* acl_free.c                                                         */
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 1999, 2000, 2001 Robert N. M. Watson
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
 * acl_free -- free ACL objects from user memory
 */

/*
 * acl_free() (23.4.12): free any releasable memory allocated to the
 * ACL data object identified by obj_p.
 */
int
ref_acl_free(void *obj_p)
{

	if (obj_p) {
		free(obj_p);
		obj_p = NULL;
	}

	return (0);
}

/* ------------------------------------------------------------------ */
/* mac_exec.c                                                         */
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002 Networks Associates Technology, Inc.
 * All rights reserved.
 *
 * This software was developed for the FreeBSD Project in part by Network
 * Associates Laboratories, the Security Research Division of Network
 * Associates, Inc. under DARPA/SPAWAR contract N66001-01-C-8035 ("CBOSS"),
 * as part of the DARPA CHATS research program.
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
ref_mac_execve(char *fname, char **argv, char **envv, struct mac *label)
{

	return (__mac_execve(fname, argv, envv, label));
}

/* ------------------------------------------------------------------ */
/* acl_compat.c                                                       */
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2008 Edward Tomasz Napierała <trasz@FreeBSD.org>
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

int ref___oldacl_get_perm_np(acl_permset_t, oldacl_perm_t);
int ref___oldacl_add_perm(acl_permset_t, oldacl_perm_t);
int ref___oldacl_delete_perm(acl_permset_t, oldacl_perm_t);

/*
 * Compatibility wrappers for applications compiled against libc from before
 * NFSv4 ACLs were added.
 */
int
ref___oldacl_get_perm_np(acl_permset_t permset_d, oldacl_perm_t perm)
{

	return (acl_get_perm_np(permset_d, perm));
}

int
ref___oldacl_add_perm(acl_permset_t permset_d, oldacl_perm_t perm)
{

	return (acl_add_perm(permset_d, perm));
}

int
ref___oldacl_delete_perm(acl_permset_t permset_d, oldacl_perm_t perm)
{

	return (acl_delete_perm(permset_d, perm));
}

__sym_compat(acl_get_perm_np, __oldacl_get_perm_np, FBSD_1.0);
__sym_compat(acl_add_perm, __oldacl_add_perm, FBSD_1.0);
__sym_compat(acl_delete_perm, __oldacl_delete_perm, FBSD_1.0);
