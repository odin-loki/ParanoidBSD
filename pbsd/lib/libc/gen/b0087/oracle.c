/*
 * oracle.c -- reference implementation for batch b0087.
 *
 * The original C sources, concatenated, with every function renamed with a
 * "ref_" prefix.  Function bodies are UNMODIFIED.  Only defines/declarations
 * that the unavailable FreeBSD/HardenedBSD headers used to supply have been
 * added.
 *
 * Sources included:
 *	lib/libc/gen/inotify.c
 *
 * Sources omitted (see skipped.txt):
 *	lib/libc/gen/uexterr_gettext.c
 *	lib/libc/gen/scandir_b.c
 */

#include <stddef.h>
#include <stdint.h>

/*
 * Missing defines: the original translation unit obtained these from
 * "namespace.h", <sys/fcntl.h>, <sys/inotify.h>, <sys/specialfd.h>,
 * "un-namespace.h" and "libc_private.h".
 */
#define	AT_FDCWD		(-100)		/* sys/fcntl.h */
#define	IN_NONBLOCK		0x0004		/* sys/inotify.h: O_NONBLOCK */
#define	IN_CLOEXEC		0x00100000	/* sys/inotify.h: O_CLOEXEC */
#define	SPECIALFD_INOTIFY	2		/* sys/specialfd.h */

struct specialfd_inotify {
	int	flags;
};

int inotify_add_watch_at(int fd, int dfd, const char *pathname, uint32_t mask);
int __sys___specialfd(int type, const void *arg, size_t argsize);

/* ------------------------------------------------------------------------- */
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 Klara, Inc.
 */

/* #include "namespace.h"		-- unavailable, substituted above */
/* #include <sys/fcntl.h>		-- unavailable, substituted above */
/* #include <sys/inotify.h>		-- unavailable, substituted above */
/* #include <sys/specialfd.h>		-- unavailable, substituted above */
/* #include "un-namespace.h"		-- unavailable, substituted above */
/* #include "libc_private.h"		-- unavailable, substituted above */

/*
 * Provide compatibility with libinotify, which uses different values for these
 * flags.
 */
#define	IN_NONBLOCK_OLD	0x80000
#define	IN_CLOEXEC_OLD	0x00800

int
ref_inotify_add_watch(int fd, const char *pathname, uint32_t mask)
{
	return (inotify_add_watch_at(fd, AT_FDCWD, pathname, mask));
}

int
ref_inotify_init1(int flags)
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
ref_inotify_init(void)
{
	return (ref_inotify_init1(0));
}
