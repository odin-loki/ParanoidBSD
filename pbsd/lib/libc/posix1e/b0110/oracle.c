/*
 * oracle.c -- reference implementation for PBSD batch b0110.
 *
 * The original HardenedBSD C sources, concatenated, with every function
 * renamed with a "ref_" prefix.  Function bodies are UNMODIFIED.  The only
 * additions are the type/macro definitions that the original translation
 * units obtained from <sys/mac.h>, <sys/acl.h>, <sys/extattr.h> and the
 * private acl_support.h header, plus the feature-test macro required to get
 * strdup() declared under -std=c11.
 */

#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef LONG_BIT
#define	LONG_BIT	(sizeof(long) * 8)
#endif

/* <sys/mac.h> */
struct mac {
	size_t	 m_buflen;
	char	*m_string;
};
typedef struct mac *mac_t;

/* <sys/acl.h> */
typedef uint32_t	acl_tag_t;
typedef uint32_t	acl_perm_t;
typedef uint16_t	acl_entry_type_t;
typedef uint16_t	acl_flag_t;
typedef int		acl_type_t;

struct acl_entry {
	acl_tag_t		ae_tag;
	uid_t			ae_id;
	acl_perm_t		ae_perm;
	/* NFSv4 entry fields */
	acl_entry_type_t	ae_entry_type;
	acl_flag_t		ae_flags;
};
typedef struct acl_entry *acl_entry_t;

struct acl_t_struct;
typedef struct acl_t_struct *acl_t;

#define	ACL_TYPE_ACCESS_OLD	0x00000000
#define	ACL_TYPE_DEFAULT_OLD	0x00000001
#define	ACL_TYPE_ACCESS		0x00000002
#define	ACL_TYPE_DEFAULT	0x00000003
#define	ACL_TYPE_NFS4		0x00000004

#define	ACL_BRAND_UNKNOWN	0
#define	ACL_BRAND_POSIX		1
#define	ACL_BRAND_NFS4		2

/* <sys/extattr.h> */
#define	EXTATTR_NAMESPACE_EMPTY		0x00000000
#define	EXTATTR_NAMESPACE_EMPTY_STRING	"empty"
#define	EXTATTR_NAMESPACE_USER		0x00000001
#define	EXTATTR_NAMESPACE_USER_STRING	"user"
#define	EXTATTR_NAMESPACE_SYSTEM	0x00000002
#define	EXTATTR_NAMESPACE_SYSTEM_STRING	"system"

/* syscall stubs and acl_support.h / acl_branding.c helpers */
extern	int	__mac_set_fd(int fd, struct mac *mac_p);
extern	int	__mac_set_file(const char *path_p, struct mac *mac_p);
extern	int	__mac_set_link(const char *path_p, struct mac *mac_p);
extern	int	__mac_set_proc(struct mac *mac_p);

extern	int	__acl_delete_file(const char *path_p, acl_type_t type);
extern	int	__acl_delete_link(const char *path_p, acl_type_t type);
extern	int	___acl_delete_fd(int filedes, acl_type_t type);

extern	acl_type_t	_acl_type_unold(acl_type_t type);

extern	int	_entry_brand(acl_entry_t entry);
extern	int	_entry_brand_may_be(acl_entry_t entry, int brand);
extern	void	_entry_brand_as(acl_entry_t entry, int brand);

/*
 * ==========================================================================
 * lib/libc/posix1e/mac_set.c
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1999, 2000, 2001, 2002 Robert N. M. Watson
 * All rights reserved.
 *
 * This software was developed by Robert Watson for the TrustedBSD Project.
 * ==========================================================================
 */

int
ref_mac_set_fd(int fd, struct mac *label)
{

	return (__mac_set_fd(fd, label));
}

int
ref_mac_set_file(const char *path, struct mac *label)
{

	return (__mac_set_file(path, label));
}

int
ref_mac_set_link(const char *path, struct mac *label)
{

	return (__mac_set_link(path, label));
}

int
ref_mac_set_proc(struct mac *label)
{

	return (__mac_set_proc(label));
}

/*
 * ==========================================================================
 * lib/libc/posix1e/acl_delete.c
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 1999, 2000, 2001, 2002 Robert N. M. Watson
 * All rights reserved.
 *
 * acl_delete_def_file -- remove a default acl from a file
 * ==========================================================================
 */

int
ref_acl_delete_def_file(const char *path_p)
{

	return (__acl_delete_file(path_p, ACL_TYPE_DEFAULT));
}

int
ref_acl_delete_def_link_np(const char *path_p)
{

	return (__acl_delete_link(path_p, ACL_TYPE_DEFAULT));
}

int
ref_acl_delete_file_np(const char *path_p, acl_type_t type)
{

	type = _acl_type_unold(type);
	return (__acl_delete_file(path_p, type));
}

int
ref_acl_delete_link_np(const char *path_p, acl_type_t type)
{

	type = _acl_type_unold(type);
	return (__acl_delete_link(path_p, type));
}

int
ref_acl_delete_fd_np(int filedes, acl_type_t type)
{

	type = _acl_type_unold(type);
	return (___acl_delete_fd(filedes, type));
}

/*
 * ==========================================================================
 * lib/libc/posix1e/extattr.c
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2001 Robert N. M. Watson
 * All rights reserved.
 *
 * TrustedBSD: Utility functions for extended attributes.
 * ==========================================================================
 */

int
ref_extattr_namespace_to_string(int attrnamespace, char **string)
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
ref_extattr_string_to_namespace(const char *string, int *attrnamespace)
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
 * ==========================================================================
 * lib/libc/posix1e/acl_copy.c
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2001-2002 Chris D. Faulhaber
 * All rights reserved.
 * ==========================================================================
 */

/*
 * acl_copy_entry() (23.4.4): copy the contents of ACL entry src_d to
 * ACL entry dest_d
 */
int
ref_acl_copy_entry(acl_entry_t dest_d, acl_entry_t src_d)
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
ref_acl_copy_ext(void *buf_p, acl_t acl, ssize_t size)
{

	errno = ENOSYS;
	return (-1);
}

acl_t
ref_acl_copy_int(const void *buf_p)
{

	errno = ENOSYS;
	return (NULL);
}
