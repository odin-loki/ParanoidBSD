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

module;

#include <cstddef>

export module pbsd.sys.fs.deadfs.b0213;

export namespace pbsd::sys_fs_deadfs::b0213 {

struct mount;

struct vnode {
	unsigned int v_vflag;
};

struct vop_getwritemount_args {
	mount **a_mpp;
};

struct vop_lookup_args {
	vnode **a_vpp;
};

struct vop_open_args {
	int dummy;
};

struct vop_close_args {
	int dummy;
};

struct vop_read_args {
	vnode *a_vp;
};

struct vop_write_args {
	int dummy;
};

struct vop_poll_args {
	int a_events;
};

struct vop_unset_text_args {
	int dummy;
};

inline constexpr unsigned int VV_ISTTY = 0x0002u;

inline constexpr int ENOTDIR = 20;
inline constexpr int ENXIO = 6;

inline constexpr int POLLIN = 0x0001;
inline constexpr int POLLPRI = 0x0002;
inline constexpr int POLLOUT = 0x0004;
inline constexpr int POLLRDNORM = 0x0040;
inline constexpr int POLLWRNORM = POLLOUT;
inline constexpr int POLLRDBAND = 0x0080;
inline constexpr int POLLWRBAND = 0x0100;
inline constexpr int POLLERR = 0x0008;
inline constexpr int POLLHUP = 0x0010;
inline constexpr int POLLNVAL = 0x0020;

inline constexpr int POLLSTANDARD =
    (POLLIN | POLLPRI | POLLOUT | POLLRDNORM | POLLRDBAND | POLLWRBAND |
	POLLERR | POLLHUP | POLLNVAL);

int dead_getwritemount(vop_getwritemount_args *ap)
{

	*(ap->a_mpp) = NULL;
	return (0);
}

int dead_lookup(vop_lookup_args *ap)
{

	*ap->a_vpp = NULL;
	return (ENOTDIR);
}

int dead_open(vop_open_args *ap)
{
	return (0);
}

int dead_close(vop_close_args *ap)
{
	return (0);
}

int dead_read(vop_read_args *ap)
{

	/*
	 * Return EOF for tty devices, ENXIO for others
	 */
	if (ap->a_vp->v_vflag & VV_ISTTY)
		return (0);
	return (ENXIO);
}

int dead_write(vop_write_args *ap)
{

	return (ENXIO);
}

int dead_poll(vop_poll_args *ap)
{

	if (ap->a_events & ~POLLSTANDARD)
		return (POLLNVAL);

	/*
	 * Let the user find out that the descriptor is gone.
	 */
	return (POLLHUP | ((POLLIN | POLLRDNORM) & ap->a_events));

}

int dead_unset_text(vop_unset_text_args *ap)
{

	return (0);
}

} // namespace pbsd::sys_fs_deadfs::b0213
