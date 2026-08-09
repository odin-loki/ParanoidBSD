/*
 * oracle.c -- reference implementation for PBSD batch b0211.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

struct cdev {
	int si_refs;
};

struct mount {
	int dummy;
};

struct vop_vector;

struct vnode {
	int v_type;
	void *v_data;
	struct cdev *v_rdev;
	struct mount *v_mount;
	struct vop_vector *v_op;
	int v_state;
	int v_unlock_count;
	int v_lock_flags;
	int v_gone;
	int v_put;
};

struct vop_reclaim_args {
	struct vnode *a_vp;
};

struct vop_vector {
	struct vop_vector *vop_default;
	void *vop_fsync;
	void *vop_strategy;
	int (*vop_reclaim)(struct vop_reclaim_args *);
};

#define VCHR			2
#define LK_EXCLUSIVE		0x00040000
#define LK_RETRY		0x00000040
#define VSTATE_CONSTRUCTED	4

#define ASSERT_VOP_ELOCKED(vp, msg)	((void)0)

#define VOP_UNLOCK(vp)			do { (vp)->v_unlock_count++; } while (0)

#define VFS_VOP_VECTOR_REGISTER(v)					\
	static const struct vop_vector *ref_vfs_vop_registered_##v	\
	    __attribute__((unused)) = &(v)

static struct vop_vector default_vnodeops;
static int vop_stdfsync_stub;
static int vop_panic_stub;

#define vop_stdfsync	(&vop_stdfsync_stub)
#define VOP_PANIC	(&vop_panic_stub)

int ref_dev_ref_calls;
int ref_dev_rel_calls;
int ref_getnewvnode_calls;
char ref_getnewvnode_name[16];
struct mount *ref_getnewvnode_mp;
struct vop_vector *ref_getnewvnode_ops;
int ref_vn_lock_calls;
int ref_vn_lock_flags;
int ref_vn_set_state_calls;
int ref_vn_set_state_value;
int ref_vgone_calls;
int ref_vput_calls;

void
ref_stub_reset(void)
{
	ref_dev_ref_calls = 0;
	ref_dev_rel_calls = 0;
	ref_getnewvnode_calls = 0;
	ref_getnewvnode_name[0] = '\0';
	ref_getnewvnode_mp = NULL;
	ref_getnewvnode_ops = NULL;
	ref_vn_lock_calls = 0;
	ref_vn_lock_flags = 0;
	ref_vn_set_state_calls = 0;
	ref_vn_set_state_value = 0;
	ref_vgone_calls = 0;
	ref_vput_calls = 0;
}

void
dev_ref(struct cdev *dev)
{

	ref_dev_ref_calls++;
	if (dev != NULL)
		dev->si_refs++;
}

void
dev_rel(struct cdev *dev)
{

	ref_dev_rel_calls++;
	if (dev != NULL)
		dev->si_refs--;
}

void
getnewvnode(const char *fstypename, struct mount *mp, struct vop_vector *vops,
    struct vnode **vpp)
{

	ref_getnewvnode_calls++;
	if (fstypename != NULL) {
		strncpy(ref_getnewvnode_name, fstypename,
		    sizeof(ref_getnewvnode_name) - 1);
		ref_getnewvnode_name[sizeof(ref_getnewvnode_name) - 1] = '\0';
	} else {
		ref_getnewvnode_name[0] = '\0';
	}
	ref_getnewvnode_mp = mp;
	ref_getnewvnode_ops = vops;
	*vpp = calloc(1, sizeof(**vpp));
	if (*vpp != NULL) {
		(*vpp)->v_mount = mp;
		(*vpp)->v_op = vops;
	}
}

void
vn_lock(struct vnode *vp, int flags)
{

	ref_vn_lock_calls++;
	ref_vn_lock_flags = flags;
	if (vp != NULL)
		vp->v_lock_flags = flags;
}

void
vn_set_state(struct vnode *vp, int state)
{

	ref_vn_set_state_calls++;
	ref_vn_set_state_value = state;
	if (vp != NULL)
		vp->v_state = state;
}

void
vgone(struct vnode *vp)
{

	ref_vgone_calls++;
	if (vp != NULL)
		vp->v_gone = 1;
}

void
vput(struct vnode *vp)
{

	ref_vput_calls++;
	if (vp != NULL)
		vp->v_put = 1;
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 Netflix, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

int
ref_mntfs_reclaim(struct vop_reclaim_args *ap)
{
	struct vnode *vp = ap->a_vp;

	dev_rel(vp->v_rdev);
	return (0);
}

struct vop_vector ref_mntfs_vnodeops = {
	.vop_default =		&default_vnodeops,

	.vop_fsync =		vop_stdfsync,
	.vop_strategy = 	VOP_PANIC,
	.vop_reclaim =		ref_mntfs_reclaim,
};
VFS_VOP_VECTOR_REGISTER(ref_mntfs_vnodeops);

struct vnode *
ref_mntfs_allocvp(struct mount *mp, struct vnode *ovp)
{
	struct vnode *vp;
	struct cdev *dev;

	ASSERT_VOP_ELOCKED(ovp, __func__);

	dev = ovp->v_rdev;

	getnewvnode("mntfs", mp, &ref_mntfs_vnodeops, &vp);
	vp->v_type = VCHR;
	vp->v_data = NULL;
	dev_ref(dev);
	vp->v_rdev = dev;

	VOP_UNLOCK(ovp);
	vn_lock(vp, LK_EXCLUSIVE | LK_RETRY);
	vn_set_state(vp, VSTATE_CONSTRUCTED);
	return (vp);
}

void
ref_mntfs_freevp(struct vnode *vp)
{
	ASSERT_VOP_ELOCKED(vp, "mntfs_freevp");
	vgone(vp);
	vput(vp);
}
