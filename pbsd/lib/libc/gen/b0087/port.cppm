/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 Klara, Inc.
 */

module;

#include <stddef.h>
#include <stdint.h>

/*
 * The FreeBSD/HardenedBSD headers that the original translation unit pulled in
 * ("namespace.h", <sys/fcntl.h>, <sys/inotify.h>, <sys/specialfd.h>,
 * "un-namespace.h", "libc_private.h") are not available here, so the pieces
 * that inotify.c actually consumes are reproduced verbatim below.
 */
#define	PBSD_AT_FDCWD		(-100)		/* sys/fcntl.h */
#define	PBSD_IN_NONBLOCK	0x0004		/* sys/inotify.h: O_NONBLOCK */
#define	PBSD_IN_CLOEXEC		0x00100000	/* sys/inotify.h: O_CLOEXEC */
#define	PBSD_SPECIALFD_INOTIFY	2		/* sys/specialfd.h */

#undef	AT_FDCWD
#undef	IN_NONBLOCK
#undef	IN_CLOEXEC
#undef	SPECIALFD_INOTIFY
#define	AT_FDCWD		PBSD_AT_FDCWD
#define	IN_NONBLOCK		PBSD_IN_NONBLOCK
#define	IN_CLOEXEC		PBSD_IN_CLOEXEC
#define	SPECIALFD_INOTIFY	PBSD_SPECIALFD_INOTIFY

struct specialfd_inotify {
	int	flags;
};

extern "C" {
int inotify_add_watch_at(int fd, int dfd, const char *pathname, uint32_t mask);
int __sys___specialfd(int type, const void *arg, size_t argsize);
}

/*
 * Provide compatibility with libinotify, which uses different values for these
 * flags.
 */
#define	IN_NONBLOCK_OLD	0x80000
#define	IN_CLOEXEC_OLD	0x00800

export module pbsd.lib.libc.gen.b0087;

export namespace pbsd::lib_libc_gen::b0087 {

int
inotify_add_watch(int fd, const char *pathname, uint32_t mask)
{
	return (inotify_add_watch_at(fd, AT_FDCWD, pathname, mask));
}

int
inotify_init1(int flags)
{
	struct specialfd_inotify args;

	if ((flags & IN_NONBLOCK_OLD) != 0) {
		flags &= ~IN_NONBLOCK_OLD;
		flags |= IN_NONBLOCK;
	}
	if ((flags & IN_CLOEXEC_OLD) != 0) {
		flags &= ~IN_CLOEXEC_OLD;
		flags |= IN_CLOEXEC;
	}
	args.flags = flags;
	return (__sys___specialfd(SPECIALFD_INOTIFY, &args, sizeof(args)));
}

int
inotify_init(void)
{
	return (inotify_init1(0));
}

} /* namespace pbsd::lib_libc_gen::b0087 */
