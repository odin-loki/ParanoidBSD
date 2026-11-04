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
/*-
 * SPDX-License-Identifier: BSD-2-Clause
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
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2001 Robert N. M. Watson
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
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2001-2002 Chris D. Faulhaber
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
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

module;

#include <sys/types.h>

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

export module pbsd.lib.libc.posix1e.b0110;

export namespace pbsd::lib_libc_posix1e::b0110 {

/*
 * Declarations lifted from the headers the original translation units
 * included: <sys/mac.h>, <sys/acl.h> and <sys/extattr.h>.
 */

struct mac {
	size_t	 m_buflen;
	char	*m_string;
};
typedef struct mac *mac_t;

typedef uint32_t	acl_tag_t;
typedef uint32_t	acl_perm_t;
typedef uint16_t	acl_entry_type_t;
typedef uint16_t	acl_flag_t;
typedef int		acl_type_t;
typedef uint32_t	acl_uid_t;	/* uid_t */

struct acl_entry {
	acl_tag_t		ae_tag;
	acl_uid_t		ae_id;
	acl_perm_t		ae_perm;
	/* NFSv4 entry fields */
	acl_entry_type_t	ae_entry_type;
	acl_flag_t		ae_flags;
};
typedef struct acl_entry *acl_entry_t;

struct acl_t_struct;
typedef struct acl_t_struct *acl_t;

typedef ::ssize_t ssize_t;

inline constexpr acl_type_t ACL_TYPE_ACCESS_OLD		= 0x00000000;
inline constexpr acl_type_t ACL_TYPE_DEFAULT_OLD	= 0x00000001;
inline constexpr acl_type_t ACL_TYPE_ACCESS		= 0x00000002;
inline constexpr acl_type_t ACL_TYPE_DEFAULT		= 0x00000003;
inline constexpr acl_type_t ACL_TYPE_NFS4		= 0x00000004;

inline constexpr int ACL_BRAND_UNKNOWN	= 0;
inline constexpr int ACL_BRAND_POSIX	= 1;
inline constexpr int ACL_BRAND_NFS4	= 2;

inline constexpr int	   EXTATTR_NAMESPACE_EMPTY		= 0x00000000;
inline constexpr const char *EXTATTR_NAMESPACE_EMPTY_STRING	= "empty";
inline constexpr int	   EXTATTR_NAMESPACE_USER		= 0x00000001;
inline constexpr const char *EXTATTR_NAMESPACE_USER_STRING	= "user";
inline constexpr int	   EXTATTR_NAMESPACE_SYSTEM		= 0x00000002;
inline constexpr const char *EXTATTR_NAMESPACE_SYSTEM_STRING	= "system";

/*
 * Interfaces that live outside of these translation units: the syscall
 * stubs and the acl_support.h / acl_branding.c helpers.
 */
extern "C" int	__mac_set_fd(int fd, struct mac *mac_p);
extern "C" int	__mac_set_file(const char *path_p, struct mac *mac_p);
extern "C" int	__mac_set_link(const char *path_p, struct mac *mac_p);
extern "C" int	__mac_set_proc(struct mac *mac_p);

extern "C" int	__acl_delete_file(const char *path_p, acl_type_t type);
extern "C" int	__acl_delete_link(const char *path_p, acl_type_t type);
extern "C" int	___acl_delete_fd(int filedes, acl_type_t type);

extern "C" acl_type_t	_acl_type_unold(acl_type_t type);

extern "C" int	_entry_brand(acl_entry_t entry);
extern "C" int	_entry_brand_may_be(acl_entry_t entry, int brand);
extern "C" void	_entry_brand_as(acl_entry_t entry, int brand);

/*
 * lib/libc/posix1e/mac_set.c
 */

int
mac_set_fd(int fd, struct mac *label)
{

	return (__mac_set_fd(fd, label));
}

int
mac_set_file(const char *path, struct mac *label)
{

	return (__mac_set_file(path, label));
}

int
mac_set_link(const char *path, struct mac *label)
{

	return (__mac_set_link(path, label));
}

int
mac_set_proc(struct mac *label)
{

	return (__mac_set_proc(label));
}

/*
 * lib/libc/posix1e/acl_delete.c
 *
 * acl_delete_def_file -- remove a default acl from a file
 */

int
acl_delete_def_file(const char *path_p)
{

	return (__acl_delete_file(path_p, ACL_TYPE_DEFAULT));
}

int
acl_delete_def_link_np(const char *path_p)
{

	return (__acl_delete_link(path_p, ACL_TYPE_DEFAULT));
}

int
acl_delete_file_np(const char *path_p, acl_type_t type)
{

	type = _acl_type_unold(type);
	return (__acl_delete_file(path_p, type));
}

int
acl_delete_link_np(const char *path_p, acl_type_t type)
{

	type = _acl_type_unold(type);
	return (__acl_delete_link(path_p, type));
}

int
acl_delete_fd_np(int filedes, acl_type_t type)
{

	type = _acl_type_unold(type);
	return (___acl_delete_fd(filedes, type));
}

/*
 * lib/libc/posix1e/extattr.c
 *
 * TrustedBSD: Utility functions for extended attributes.
 */

int
extattr_namespace_to_string(int attrnamespace, char **string)
{

	switch(attrnamespace) {
	case EXTATTR_NAMESPACE_USER:
		if (string != NULL)
			*string = strdup(EXTATTR_NAMESPACE_USER_STRING);
		return (0);

	case EXTATTR_NAMESPACE_SYSTEM:
		if (string != NULL)
			*string = strdup(EXTATTR_NAMESPACE_SYSTEM_STRING);
		return (0);

	default:
		errno = EINVAL;
		return (-1);
	}
}

int
extattr_string_to_namespace(const char *string, int *attrnamespace)
{

	if (!strcmp(string, EXTATTR_NAMESPACE_USER_STRING)) {
		if (attrnamespace != NULL)
			*attrnamespace = EXTATTR_NAMESPACE_USER;
		return (0);
	} else if (!strcmp(string, EXTATTR_NAMESPACE_SYSTEM_STRING)) {
		if (attrnamespace != NULL)
			*attrnamespace = EXTATTR_NAMESPACE_SYSTEM;
		return (0);
	} else {
		errno = EINVAL;
		return (-1);
	}
}

/*
 * lib/libc/posix1e/acl_copy.c
 *
 * acl_copy_entry() (23.4.4): copy the contents of ACL entry src_d to
 * ACL entry dest_d
 */
int
acl_copy_entry(acl_entry_t dest_d, acl_entry_t src_d)
{

	if (src_d == NULL || dest_d == NULL || src_d == dest_d) {
		errno = EINVAL;
		return (-1);
	}

	/*
	 * Can we brand the new entry the same as the source entry?
	 */
	if (!_entry_brand_may_be(dest_d, _entry_brand(src_d))) {
		errno = EINVAL;
		return (-1);
	}

	_entry_brand_as(dest_d, _entry_brand(src_d));

	dest_d->ae_tag = src_d->ae_tag;
	dest_d->ae_id = src_d->ae_id;
	dest_d->ae_perm = src_d->ae_perm;
	dest_d->ae_entry_type = src_d->ae_entry_type;
	dest_d->ae_flags = src_d->ae_flags;

	return (0);
}

ssize_t
acl_copy_ext(void *buf_p, acl_t acl, ssize_t size)
{

	errno = ENOSYS;
	return (-1);
}

acl_t
acl_copy_int(const void *buf_p)
{

	errno = ENOSYS;
	return (NULL);
}

} /* namespace pbsd::lib_libc_posix1e::b0110 */
