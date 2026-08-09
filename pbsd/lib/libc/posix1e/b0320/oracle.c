/*
 * oracle.c -- reference implementation for batch b0320.
 *
 * Original C sources, concatenated, with every function renamed with a
 * "ref_" prefix.  Function bodies are UNMODIFIED.
 *
 * Sources:
 *   hbsd/src/lib/libc/posix1e/mac_get.c
 *   hbsd/src/lib/libc/posix1e/acl_init.c
 */

#include <sys/types.h>

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#ifndef SOL_SOCKET
#define SOL_SOCKET	1
#endif
#ifndef SO_PEERLABEL
#define SO_PEERLABEL	0x1010
#endif

struct mac {
	size_t	 m_buflen;
	char	*m_string;
};

typedef uint32_t	acl_tag_t;
typedef uint32_t	acl_perm_t;
typedef uint16_t	acl_entry_type_t;
typedef uint16_t	acl_flag_t;

struct acl_entry {
	acl_tag_t		ae_tag;
	uid_t			ae_id;
	acl_perm_t		ae_perm;
	acl_entry_type_t	ae_entry_type;
	acl_flag_t		ae_flags;
};
typedef struct acl_entry *acl_entry_t;

#define	ACL_MAX_ENTRIES				254
#define	_ACL_T_ALIGNMENT_BITS			13
#define	ACL_BRAND_UNKNOWN			0

struct acl {
	unsigned int		acl_maxcnt;
	unsigned int		acl_cnt;
	int			acl_spare[4];
	struct acl_entry	acl_entry[ACL_MAX_ENTRIES];
};

struct acl_t_struct {
	struct acl		ats_acl;
	int			ats_cur_entry;
	int			ats_brand;
};
typedef struct acl_t_struct *acl_t;

#ifndef CTASSERT
#define CTASSERT(x)		_CTASSERT(x, __LINE__)
#define _CTASSERT(x, y)		__CTASSERT(x, y)
#define __CTASSERT(x, y)	typedef char __assert_ ## y [(x) ? 1 : -1]
#endif

extern int __mac_get_fd(int fd, struct mac *mac_p);
extern int __mac_get_file(const char *path_p, struct mac *mac_p);
extern int __mac_get_link(const char *path_p, struct mac *mac_p);
extern int __mac_get_pid(pid_t pid, struct mac *mac_p);
extern int __mac_get_proc(struct mac *mac_p);
extern int getsockopt(int fd, int level, int optname, void *optval,
    socklen_t *optlen);

/* ------------------------------------------------------------------ */
/* mac_get.c                                                          */
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1999, 2000, 2001, 2002 Robert N. M. Watson
 * All rights reserved.
 *
 * This software was developed by Robert Watson for the TrustedBSD Project.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the authors may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
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
ref_mac_get_fd(int fd, struct mac *label)
{

	return (__mac_get_fd(fd, label));
}

int
ref_mac_get_file(const char *path, struct mac *label)
{

	return (__mac_get_file(path, label));
}

int
ref_mac_get_link(const char *path, struct mac *label)
{

	return (__mac_get_link(path, label));
}

int
ref_mac_get_peer(int fd, struct mac *label)
{
	socklen_t len;

	len = sizeof(*label);
	return (getsockopt(fd, SOL_SOCKET, SO_PEERLABEL, label, &len));
}

int
ref_mac_get_pid(pid_t pid, struct mac *label)
{

	return (__mac_get_pid(pid, label));
}

int
ref_mac_get_proc(struct mac *label)
{

	return (__mac_get_proc(label));
}

/* ------------------------------------------------------------------ */
/* acl_init.c                                                         */
/* ------------------------------------------------------------------ */

#define acl_init	ref_acl_init
#define acl_dup		ref_acl_dup

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
 * acl_init -- return a fresh acl structure
 * acl_dup -- duplicate an acl and return the new copy
 */

CTASSERT(1 << _ACL_T_ALIGNMENT_BITS > sizeof(struct acl_t_struct));

acl_t
ref_acl_init(int count)
{
	int error;
	acl_t acl;

	if (count > ACL_MAX_ENTRIES) {
		errno = ENOMEM;
		return (NULL);
	}
	if (count < 0) {
		errno = EINVAL;
		return (NULL);
	}

	error = posix_memalign((void *)&acl, 1 << _ACL_T_ALIGNMENT_BITS,
	    sizeof(struct acl_t_struct));
	if (error) {
		errno = error;
		return (NULL);
	}

	bzero(acl, sizeof(struct acl_t_struct));
	acl->ats_brand = ACL_BRAND_UNKNOWN;
	acl->ats_acl.acl_maxcnt = ACL_MAX_ENTRIES;

	return (acl);
}

acl_t
ref_acl_dup(acl_t acl)
{
	acl_t	acl_new;

	acl_new = acl_init(ACL_MAX_ENTRIES);
	if (acl_new != NULL) {
		*acl_new = *acl;
		acl->ats_cur_entry = 0;
		acl_new->ats_cur_entry = 0;
	}

	return (acl_new);
}

#undef acl_init
#undef acl_dup
