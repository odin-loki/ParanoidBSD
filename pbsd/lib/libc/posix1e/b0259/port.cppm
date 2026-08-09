module;

#include <sys/types.h>

#include <cstddef>
#include <cstdint>
#include <cstdlib>

export module pbsd.lib.libc.posix1e.b0259;

export namespace pbsd::lib_libc_posix1e::b0259 {

struct mac {
	size_t	 m_buflen;
	char	*m_string;
};

typedef std::uint32_t	acl_perm_t;
typedef std::uint32_t	*acl_permset_t;
typedef mode_t		oldacl_perm_t;

extern "C" int __mac_execve(char *fname, char **argv, char **envv,
    struct mac *mac_p);
extern "C" int acl_get_perm_np(acl_permset_t permset_d, acl_perm_t perm);
extern "C" int acl_add_perm(acl_permset_t permset_d, acl_perm_t perm);
extern "C" int acl_delete_perm(acl_permset_t permset_d, acl_perm_t perm);

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
acl_free(void *obj_p)
{

	if (obj_p) {
		free(obj_p);
		obj_p = NULL;
	}

	return (0);
}

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
mac_execve(char *fname, char **argv, char **envv, struct mac *label)
{

	return (__mac_execve(fname, argv, envv, label));
}

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

/*
 * Compatibility wrappers for applications compiled against libc from before
 * NFSv4 ACLs were added.
 */
int
__oldacl_get_perm_np(acl_permset_t permset_d, oldacl_perm_t perm)
{

	return (acl_get_perm_np(permset_d, perm));
}

int
__oldacl_add_perm(acl_permset_t permset_d, oldacl_perm_t perm)
{

	return (acl_add_perm(permset_d, perm));
}

int
__oldacl_delete_perm(acl_permset_t permset_d, oldacl_perm_t perm)
{

	return (acl_delete_perm(permset_d, perm));
}

} /* namespace pbsd::lib_libc_posix1e::b0259 */
