// PBSD port of HardenedBSD sys/fs/mntfs batch b0211.

module;

#define _KERNEL
#define _POSIX_C_SOURCE 200809L

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

export module pbsd.sys.fs.mntfs.b0211;

export namespace pbsd::sys_fs_mntfs::b0211 {

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
	cdev *v_rdev;
	mount *v_mount;
	vop_vector *v_op;
	int v_state;
	int v_unlock_count;
	int v_lock_flags;
	int v_gone;
	int v_put;
};

struct vop_reclaim_args {
	vnode *a_vp;
};

struct vop_vector {
	vop_vector *vop_default;
	void *vop_fsync;
	void *vop_strategy;
	int (*vop_reclaim)(vop_reclaim_args *);
};

constexpr int VCHR = 2;
constexpr int LK_EXCLUSIVE = 0x00040000;
constexpr int LK_RETRY = 0x00000040;
constexpr int VSTATE_CONSTRUCTED = 4;

} // namespace pbsd::sys_fs_mntfs::b0211

namespace pbsd::sys_fs_mntfs::b0211::detail {

using namespace pbsd::sys_fs_mntfs::b0211;

#define ASSERT_VOP_ELOCKED(vp, msg)	((void)0)

#define VOP_UNLOCK(vp)			do { (vp)->v_unlock_count++; } while (0)

#define VFS_VOP_VECTOR_REGISTER(v)					\
	static const vop_vector *ref_vfs_vop_registered_##v		\
	    __attribute__((unused)) = &(v)

inline vop_vector default_vnodeops{};
inline int vop_stdfsync_stub{};
inline int vop_panic_stub{};

#define vop_stdfsync	(&vop_stdfsync_stub)
#define VOP_PANIC	(&vop_panic_stub)

inline int g_dev_ref_calls = 0;
inline int g_dev_rel_calls = 0;
inline int g_getnewvnode_calls = 0;
inline char g_getnewvnode_name[16]{};
inline mount *g_getnewvnode_mp = nullptr;
inline vop_vector *g_getnewvnode_ops = nullptr;
inline int g_vn_lock_calls = 0;
inline int g_vn_lock_flags = 0;
inline int g_vn_set_state_calls = 0;
inline int g_vn_set_state_value = 0;
inline int g_vgone_calls = 0;
inline int g_vput_calls = 0;

inline void stub_reset() noexcept
{
	g_dev_ref_calls = 0;
	g_dev_rel_calls = 0;
	g_getnewvnode_calls = 0;
	g_getnewvnode_name[0] = '\0';
	g_getnewvnode_mp = nullptr;
	g_getnewvnode_ops = nullptr;
	g_vn_lock_calls = 0;
	g_vn_lock_flags = 0;
	g_vn_set_state_calls = 0;
	g_vn_set_state_value = 0;
	g_vgone_calls = 0;
	g_vput_calls = 0;
}

inline void
dev_ref(cdev *dev)
{

	g_dev_ref_calls++;
	if (dev != nullptr)
		dev->si_refs++;
}

inline void
dev_rel(cdev *dev)
{

	g_dev_rel_calls++;
	if (dev != nullptr)
		dev->si_refs--;
}

inline void
getnewvnode(const char *fstypename, mount *mp, vop_vector *vops, vnode **vpp)
{

	g_getnewvnode_calls++;
	if (fstypename != nullptr) {
		std::strncpy(g_getnewvnode_name, fstypename,
		    sizeof(g_getnewvnode_name) - 1);
		g_getnewvnode_name[sizeof(g_getnewvnode_name) - 1] = '\0';
	} else {
		g_getnewvnode_name[0] = '\0';
	}
	g_getnewvnode_mp = mp;
	g_getnewvnode_ops = vops;
	*vpp = static_cast<vnode *>(std::calloc(1, sizeof(**vpp)));
	if (*vpp != nullptr) {
		(*vpp)->v_mount = mp;
		(*vpp)->v_op = vops;
	}
}

inline void
vn_lock(vnode *vp, int flags)
{

	g_vn_lock_calls++;
	g_vn_lock_flags = flags;
	if (vp != nullptr)
		vp->v_lock_flags = flags;
}

inline void
vn_set_state(vnode *vp, int state)
{

	g_vn_set_state_calls++;
	g_vn_set_state_value = state;
	if (vp != nullptr)
		vp->v_state = state;
}

inline void
vgone(vnode *vp)
{

	g_vgone_calls++;
	if (vp != nullptr)
		vp->v_gone = 1;
}

inline void
vput(vnode *vp)
{

	g_vput_calls++;
	if (vp != nullptr)
		vp->v_put = 1;
}

} // namespace pbsd::sys_fs_mntfs::b0211::detail

export namespace pbsd::sys_fs_mntfs::b0211 {

inline void stub_reset() noexcept
{
	detail::stub_reset();
}

inline int dev_ref_calls() noexcept { return (detail::g_dev_ref_calls); }
inline int dev_rel_calls() noexcept { return (detail::g_dev_rel_calls); }
inline int getnewvnode_calls() noexcept { return (detail::g_getnewvnode_calls); }
inline const char *getnewvnode_name() noexcept
{
	return (detail::g_getnewvnode_name);
}
inline mount *getnewvnode_mp() noexcept { return (detail::g_getnewvnode_mp); }
inline vop_vector *getnewvnode_ops() noexcept
{
	return (detail::g_getnewvnode_ops);
}
inline int vn_lock_calls() noexcept { return (detail::g_vn_lock_calls); }
inline int vn_lock_flags() noexcept { return (detail::g_vn_lock_flags); }
inline int vn_set_state_calls() noexcept
{
	return (detail::g_vn_set_state_calls);
}
inline int vn_set_state_value() noexcept
{
	return (detail::g_vn_set_state_value);
}
inline int vgone_calls() noexcept { return (detail::g_vgone_calls); }
inline int vput_calls() noexcept { return (detail::g_vput_calls); }

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
mntfs_reclaim(vop_reclaim_args *ap)
{
	vnode *vp = ap->a_vp;

	detail::dev_rel(vp->v_rdev);
	return (0);
}

inline vop_vector mntfs_vnodeops = {
	.vop_default =		&detail::default_vnodeops,

	.vop_fsync =		&detail::vop_stdfsync_stub,
	.vop_strategy = 	&detail::vop_panic_stub,
	.vop_reclaim =		mntfs_reclaim,
};
VFS_VOP_VECTOR_REGISTER(mntfs_vnodeops);

vnode *
mntfs_allocvp(mount *mp, vnode *ovp)
{
	vnode *vp;
	cdev *dev;

	ASSERT_VOP_ELOCKED(ovp, __func__);

	dev = ovp->v_rdev;

	detail::getnewvnode("mntfs", mp, &mntfs_vnodeops, &vp);
	vp->v_type = VCHR;
	vp->v_data = nullptr;
	detail::dev_ref(dev);
	vp->v_rdev = dev;

	VOP_UNLOCK(ovp);
	detail::vn_lock(vp, LK_EXCLUSIVE | LK_RETRY);
	detail::vn_set_state(vp, VSTATE_CONSTRUCTED);
	return (vp);
}

void
mntfs_freevp(vnode *vp)
{
	ASSERT_VOP_ELOCKED(vp, "mntfs_freevp");
	detail::vgone(vp);
	detail::vput(vp);
}

} // namespace pbsd::sys_fs_mntfs::b0211
