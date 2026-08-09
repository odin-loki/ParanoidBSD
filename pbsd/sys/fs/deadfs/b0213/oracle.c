/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * hbsd/src/sys/fs/deadfs/dead_vnops.c
 *
 * Reference ("oracle") build.  The kernel headers <sys/param.h>,
 * <sys/systm.h>, <sys/kernel.h>, <sys/lock.h>, <sys/mutex.h>, <sys/poll.h>
 * and <sys/vnode.h> are unavailable in a userspace differential-test build,
 * so the handful of declarations the ported functions actually touch are
 * reproduced below with the values they carry in the kernel.  The function
 * bodies are unmodified; only the names carry the ref_ prefix and the
 * `static` storage class was dropped so the harness can link against them.
 */

#include <stddef.h>
#include <stdint.h>

/* <sys/errno.h> */
#define	ENXIO		6		/* Device not configured */
#define	EXDEV		18		/* Cross-device link */
#define	ENOTDIR		20		/* Not a directory */

/* <sys/poll.h> */
#define	POLLIN		0x0001		/* any readable data available */
#define	POLLPRI		0x0002		/* OOB/Urgent readable data */
#define	POLLOUT		0x0004		/* file descriptor is writeable */
#define	POLLRDNORM	0x0040		/* non-OOB/URG data available */
#define	POLLWRNORM	POLLOUT		/* no write type differentiation */
#define	POLLRDBAND	0x0080		/* OOB/Urgent readable data */
#define	POLLWRBAND	0x0100		/* OOB/Urgent data can be written */
#define	POLLINIGNEOF	0x2000		/* like POLLIN, except ignore EOF */
#define	POLLRDHUP	0x4000		/* half shut down */
#define	POLLERR		0x0008		/* some poll error occurred */
#define	POLLHUP		0x0010		/* file descriptor was "hung up" */
#define	POLLNVAL	0x0020		/* requested events "invalid" */
#define	POLLSTANDARD	(POLLIN|POLLPRI|POLLOUT|POLLRDNORM|POLLRDBAND|\
			 POLLWRBAND|POLLERR|POLLHUP|POLLNVAL)

/* <sys/vnode.h>: v_vflag bits */
#define	VV_ROOT		0x0001		/* root of its filesystem */
#define	VV_ISTTY	0x0002		/* vnode represents a tty */
#define	VV_NOSYNC	0x0004		/* unlinked, stop syncing */
#define	VV_ETERNALDEV	0x0008		/* device that is never destroyed */
#define	VV_CACHEDLABEL	0x0010		/* vnode has valid cached MAC label */
#define	VV_VMSIZEVNLOCK	0x0020		/* object size check requires vnode lock */
#define	VV_COPYONWRITE	0x0040		/* vnode is doing copy-on-write */
#define	VV_SYSTEM	0x0080		/* vnode being used by kernel */
#define	VV_PROCDEP	0x0100		/* vnode is process dependent */
#define	VV_DELETED	0x0200		/* should be removed */
#define	VV_MD		0x0400		/* vnode backs the md device */
#define	VV_FORCEINSMQ	0x0800		/* force the insmntque to succeed */
#define	VV_READLINK	0x1000		/* fdescfs linux vnode */
#define	VV_UNLINKED	0x2000		/* unlinked file */

struct ref_mount;

struct ref_vnode {
	uint32_t	v_vflag;	/* vnode flags */
};

struct ref_vop_getwritemount_args {
	struct ref_vnode *a_vp;
	struct ref_mount **a_mpp;
};

struct ref_vop_lookup_args {
	struct ref_vnode *a_dvp;
	struct ref_vnode **a_vpp;
};

struct ref_vop_open_args {
	struct ref_vnode *a_vp;
	int		a_mode;
	int		a_fdidx;
};

struct ref_vop_close_args {
	struct ref_vnode *a_vp;
	int		a_fflag;
};

struct ref_vop_read_args {
	struct ref_vnode *a_vp;
	int		a_ioflag;
};

struct ref_vop_write_args {
	struct ref_vnode *a_vp;
	int		a_ioflag;
};

struct ref_vop_poll_args {
	struct ref_vnode *a_vp;
	int		a_events;
};

struct ref_vop_unset_text_args {
	struct ref_vnode *a_vp;
};

int ref_dead_getwritemount(struct ref_vop_getwritemount_args *ap);
int ref_dead_lookup(struct ref_vop_lookup_args *ap);
int ref_dead_open(struct ref_vop_open_args *ap);
int ref_dead_close(struct ref_vop_close_args *ap);
int ref_dead_read(struct ref_vop_read_args *ap);
int ref_dead_write(struct ref_vop_write_args *ap);
int ref_dead_poll(struct ref_vop_poll_args *ap);
int ref_dead_unset_text(struct ref_vop_unset_text_args *ap);

int
ref_dead_getwritemount(struct ref_vop_getwritemount_args *ap)
{

	*(ap->a_mpp) = NULL;
	return (0);
}

/*
 * Trivial lookup routine that always fails.
 */
int
ref_dead_lookup(struct ref_vop_lookup_args *ap)
{

	*ap->a_vpp = NULL;
	return (ENOTDIR);
}

/*
 * Silently succeed open and close.
 */
int
ref_dead_open(struct ref_vop_open_args *ap)
{
	return (0);
}

int
ref_dead_close(struct ref_vop_close_args *ap)
{
	return (0);
}

int
ref_dead_read(struct ref_vop_read_args *ap)
{

	/*
	 * Return EOF for tty devices, ENXIO for others
	 */
	if (ap->a_vp->v_vflag & VV_ISTTY)
		return (0);
	return (ENXIO);
}

int
ref_dead_write(struct ref_vop_write_args *ap)
{

	return (ENXIO);
}

int
ref_dead_poll(struct ref_vop_poll_args *ap)
{

	if (ap->a_events & ~POLLSTANDARD)
		return (POLLNVAL);

	/*
	 * Let the user find out that the descriptor is gone.
	 */
	return (POLLHUP | ((POLLIN | POLLRDNORM) & ap->a_events));

}

int
ref_dead_unset_text(struct ref_vop_unset_text_args *ap)
{

	return (0);
}
