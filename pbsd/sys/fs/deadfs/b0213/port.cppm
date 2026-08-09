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
 * PBSD port of hbsd/src/sys/fs/deadfs/dead_vnops.c
 */

module;

#include <cstdint>

export module pbsd.sys.fs.deadfs.b0213;

export namespace pbsd::sys_fs_deadfs::b0213 {

/* <sys/errno.h> */
inline constexpr int ENXIO	= 6;		/* Device not configured */
inline constexpr int EXDEV	= 18;		/* Cross-device link */
inline constexpr int ENOTDIR	= 20;		/* Not a directory */

/* <sys/poll.h> */
inline constexpr int POLLIN		= 0x0001; /* any readable data available */
inline constexpr int POLLPRI		= 0x0002; /* OOB/Urgent readable data */
inline constexpr int POLLOUT		= 0x0004; /* file descriptor is writeable */
inline constexpr int POLLRDNORM		= 0x0040; /* non-OOB/URG data available */
inline constexpr int POLLWRNORM		= POLLOUT; /* no write type differentiation */
inline constexpr int POLLRDBAND		= 0x0080; /* OOB/Urgent readable data */
inline constexpr int POLLWRBAND		= 0x0100; /* OOB/Urgent data can be written */
inline constexpr int POLLINIGNEOF	= 0x2000; /* like POLLIN, except ignore EOF */
inline constexpr int POLLRDHUP		= 0x4000; /* half shut down */
inline constexpr int POLLERR		= 0x0008; /* some poll error occurred */
inline constexpr int POLLHUP		= 0x0010; /* file descriptor was "hung up" */
inline constexpr int POLLNVAL		= 0x0020; /* requested events "invalid" */
inline constexpr int POLLSTANDARD	= (POLLIN|POLLPRI|POLLOUT|POLLRDNORM|
					   POLLRDBAND|POLLWRBAND|POLLERR|
					   POLLHUP|POLLNVAL);

/* <sys/vnode.h>: v_vflag bits */
inline constexpr int VV_ROOT		= 0x0001; /* root of its filesystem */
inline constexpr int VV_ISTTY		= 0x0002; /* vnode represents a tty */
inline constexpr int VV_NOSYNC		= 0x0004; /* unlinked, stop syncing */
inline constexpr int VV_ETERNALDEV	= 0x0008; /* device that is never destroyed */
inline constexpr int VV_CACHEDLABEL	= 0x0010; /* vnode has valid cached MAC label */
inline constexpr int VV_VMSIZEVNLOCK	= 0x0020; /* object size check requires vnode lock */
inline constexpr int VV_COPYONWRITE	= 0x0040; /* vnode is doing copy-on-write */
inline constexpr int VV_SYSTEM		= 0x0080; /* vnode being used by kernel */
inline constexpr int VV_PROCDEP		= 0x0100; /* vnode is process dependent */
inline constexpr int VV_DELETED		= 0x0200; /* should be removed */
inline constexpr int VV_MD		= 0x0400; /* vnode backs the md device */
inline constexpr int VV_FORCEINSMQ	= 0x0800; /* force the insmntque to succeed */
inline constexpr int VV_READLINK	= 0x1000; /* fdescfs linux vnode */
inline constexpr int VV_UNLINKED	= 0x2000; /* unlinked file */

struct mount;

struct vnode {
	std::uint32_t	v_vflag;	/* vnode flags */
};

struct vop_getwritemount_args {
	vnode		*a_vp;
	mount		**a_mpp;
};

struct vop_lookup_args {
	vnode		*a_dvp;
	vnode		**a_vpp;
};

struct vop_open_args {
	vnode		*a_vp;
	int		a_mode;
	int		a_fdidx;
};

struct vop_close_args {
	vnode		*a_vp;
	int		a_fflag;
};

struct vop_read_args {
	vnode		*a_vp;
	int		a_ioflag;
};

struct vop_write_args {
	vnode		*a_vp;
	int		a_ioflag;
};

struct vop_poll_args {
	vnode		*a_vp;
	int		a_events;
};

struct vop_unset_text_args {
	vnode		*a_vp;
};

int
dead_getwritemount(vop_getwritemount_args *ap)
{

	*(ap->a_mpp) = nullptr;
	return (0);
}

/*
 * Trivial lookup routine that always fails.
 */
int
dead_lookup(vop_lookup_args *ap)
{

	*ap->a_vpp = nullptr;
	return (ENOTDIR);
}

/*
 * Silently succeed open and close.
 */
int
dead_open(vop_open_args *ap)
{
	return (0);
}

int
dead_close(vop_close_args *ap)
{
	return (0);
}

int
dead_read(vop_read_args *ap)
{

	/*
	 * Return EOF for tty devices, ENXIO for others
	 */
	if (ap->a_vp->v_vflag & VV_ISTTY)
		return (0);
	return (ENXIO);
}

int
dead_write(vop_write_args *ap)
{

	return (ENXIO);
}

int
dead_poll(vop_poll_args *ap)
{

	if (ap->a_events & ~POLLSTANDARD)
		return (POLLNVAL);

	/*
	 * Let the user find out that the descriptor is gone.
	 */
	return (POLLHUP | ((POLLIN | POLLRDNORM) & ap->a_events));

}

int
dead_unset_text(vop_unset_text_args *ap)
{

	return (0);
}

} // namespace pbsd::sys_fs_deadfs::b0213
