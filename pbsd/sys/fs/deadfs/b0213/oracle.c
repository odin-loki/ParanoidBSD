/*
 * oracle.c -- reference implementation for PBSD batch b0213.
 *
 * The original HardenedBSD kernel source is reproduced below with every
 * function renamed with a "ref_" prefix.  Function bodies are UNMODIFIED.
 * Supporting types, macros, and shims are added only where the original file
 * obtained them from kernel headers.
 */

#define _KERNEL
#define _POSIX_C_SOURCE 200809L

#include <stddef.h>
#include <stdint.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

struct mount;

struct vnode {
	unsigned int v_vflag;
};

struct vop_getwritemount_args {
	struct mount **a_mpp;
};

struct vop_lookup_args {
	struct vnode **a_vpp;
};

struct vop_open_args {
	int dummy;
};

struct vop_close_args {
	int dummy;
};

struct vop_read_args {
	struct vnode *a_vp;
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

#define VV_ISTTY	0x0002

#define ENOTDIR		20
#define ENXIO		6

#define POLLIN		0x0001
#define POLLPRI		0x0002
#define POLLOUT		0x0004
#define POLLRDNORM	0x0040
#define POLLWRNORM	POLLOUT
#define POLLRDBAND	0x0080
#define POLLWRBAND	0x0100
#define POLLERR		0x0008
#define POLLHUP		0x0010
#define POLLNVAL	0x0020

#define POLLSTANDARD	(POLLIN|POLLPRI|POLLOUT|POLLRDNORM|POLLRDBAND|\
			 POLLWRBAND|POLLERR|POLLHUP|POLLNVAL)

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

int
ref_dead_getwritemount(struct vop_getwritemount_args *ap)
{

	*(ap->a_mpp) = NULL;
	return (0);
}

int
ref_dead_lookup(struct vop_lookup_args *ap)
{

	*ap->a_vpp = NULL;
	return (ENOTDIR);
}

int
ref_dead_open(struct vop_open_args *ap)
{
	return (0);
}

int
ref_dead_close(struct vop_close_args *ap)
{
	return (0);
}

int
ref_dead_read(struct vop_read_args *ap)
{

	/*
	 * Return EOF for tty devices, ENXIO for others
	 */
	if (ap->a_vp->v_vflag & VV_ISTTY)
		return (0);
	return (ENXIO);
}

int
ref_dead_write(struct vop_write_args *ap)
{

	return (ENXIO);
}

int
ref_dead_poll(struct vop_poll_args *ap)
{

	if (ap->a_events & ~POLLSTANDARD)
		return (POLLNVAL);

	/*
	 * Let the user find out that the descriptor is gone.
	 */
	return (POLLHUP | ((POLLIN | POLLRDNORM) & ap->a_events));

}

int
ref_dead_unset_text(struct vop_unset_text_args *ap)
{

	return (0);
}
