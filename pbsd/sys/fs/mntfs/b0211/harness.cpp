/*
 * harness.cpp -- differential test for PBSD batch b0211.
 *
 * Every ported entry point is driven against the ref_ oracle in oracle.c and
 * the two sides are compared exhaustively: return values, every field of every
 * vnode/cdev/mount touched, the whole guard-filled arena the objects live in
 * (so writes outside the nominal window are caught), and every kernel-shim
 * observation (call counts and recorded arguments).
 *
 * The batch has no string buffers, so the usual "empty / NUL-heavy / high-bit"
 * edge cases map onto this batch's actual input domain: NULL vs non-NULL for
 * each pointer argument and field, and integer extremes for every counter that
 * the code increments or decrements.
 */

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>

import pbsd.sys.fs.mntfs.b0211;

namespace pt = pbsd::sys_fs_mntfs::b0211;

/*
 * Mirror of the oracle's layout.  oracle.c is compiled as C11 and exposes its
 * shim state as plain globals; these declarations must match it exactly.
 */
namespace ora {

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

extern "C" {
void ref_stub_reset(void);
int ref_mntfs_reclaim(vop_reclaim_args *ap);
vnode *ref_mntfs_allocvp(mount *mp, vnode *ovp);
void ref_mntfs_freevp(vnode *vp);

extern vop_vector ref_mntfs_vnodeops;
extern int ref_dev_ref_calls;
extern int ref_dev_rel_calls;
extern int ref_getnewvnode_calls;
extern char ref_getnewvnode_name[16];
extern mount *ref_getnewvnode_mp;
extern vop_vector *ref_getnewvnode_ops;
extern int ref_vn_lock_calls;
extern int ref_vn_lock_flags;
extern int ref_vn_set_state_calls;
extern int ref_vn_set_state_value;
extern int ref_vgone_calls;
extern int ref_vput_calls;
}

} // namespace ora

static_assert(sizeof(ora::vnode) == sizeof(pt::vnode));
static_assert(sizeof(ora::cdev) == sizeof(pt::cdev));
static_assert(sizeof(ora::mount) == sizeof(pt::mount));
static_assert(offsetof(ora::vnode, v_type) == offsetof(pt::vnode, v_type));
static_assert(offsetof(ora::vnode, v_data) == offsetof(pt::vnode, v_data));
static_assert(offsetof(ora::vnode, v_rdev) == offsetof(pt::vnode, v_rdev));
static_assert(offsetof(ora::vnode, v_mount) == offsetof(pt::vnode, v_mount));
static_assert(offsetof(ora::vnode, v_op) == offsetof(pt::vnode, v_op));
static_assert(offsetof(ora::vnode, v_state) == offsetof(pt::vnode, v_state));
static_assert(offsetof(ora::vnode, v_unlock_count) ==
    offsetof(pt::vnode, v_unlock_count));
static_assert(offsetof(ora::vnode, v_lock_flags) ==
    offsetof(pt::vnode, v_lock_flags));
static_assert(offsetof(ora::vnode, v_gone) == offsetof(pt::vnode, v_gone));
static_assert(offsetof(ora::vnode, v_put) == offsetof(pt::vnode, v_put));

/* ------------------------------------------------------------------ */
/* Guarded arena                                                       */
/* ------------------------------------------------------------------ */

static constexpr std::size_t VN_SIZE = sizeof(pt::vnode);
static constexpr std::size_t ARENA_SIZE = 256;
static constexpr std::size_t OFF_OVP = 16;
static constexpr std::size_t OFF_DEV = 112;
static constexpr std::size_t OFF_MP = 144;
static constexpr unsigned char GUARD = 0x7f;

static_assert(OFF_OVP + VN_SIZE <= OFF_DEV);
static_assert(OFF_DEV + sizeof(pt::cdev) <= OFF_MP);
static_assert(OFF_MP + sizeof(pt::mount) <= ARENA_SIZE);

/*
 * Pointer fields hold genuinely different addresses on the two sides, so they
 * are excluded from the raw byte comparison and compared separately as offsets
 * from the arena base.  Everything else -- guard bytes, padding, scalars -- is
 * compared byte for byte.
 */
static bool g_skip_vnode[VN_SIZE];
static bool g_skip_arena[ARENA_SIZE];

static void
build_skip_tables(void)
{
	const std::size_t poff[] = {
		offsetof(pt::vnode, v_data),
		offsetof(pt::vnode, v_rdev),
		offsetof(pt::vnode, v_mount),
		offsetof(pt::vnode, v_op),
	};

	for (std::size_t k = 0; k < sizeof(poff) / sizeof(poff[0]); k++)
		for (std::size_t i = 0; i < sizeof(void *); i++)
			g_skip_vnode[poff[k] + i] = true;
	for (std::size_t i = 0; i < VN_SIZE; i++)
		g_skip_arena[OFF_OVP + i] = g_skip_vnode[i];
}

struct Ctx {
	const unsigned char *arena;
	const void *vops;
	const void *heapvp;
};

static long
canon(const void *p, const Ctx &c)
{
	const unsigned char *b;

	if (p == nullptr)
		return (0);
	b = static_cast<const unsigned char *>(p);
	if (b >= c.arena && b < c.arena + ARENA_SIZE)
		return (1000 + static_cast<long>(b - c.arena));
	if (p == c.vops)
		return (2000);
	if (c.heapvp != nullptr && p == c.heapvp)
		return (3000);
	return (-1);
}

struct VSnap {
	int v_type;
	long v_data;
	long v_rdev;
	long v_mount;
	long v_op;
	int v_state;
	int v_unlock_count;
	int v_lock_flags;
	int v_gone;
	int v_put;
};

template <class V>
static VSnap
snap(const V *v, const Ctx &c)
{
	VSnap s;

	s.v_type = v->v_type;
	s.v_data = canon(v->v_data, c);
	s.v_rdev = canon(v->v_rdev, c);
	s.v_mount = canon(v->v_mount, c);
	s.v_op = canon(v->v_op, c);
	s.v_state = v->v_state;
	s.v_unlock_count = v->v_unlock_count;
	s.v_lock_flags = v->v_lock_flags;
	s.v_gone = v->v_gone;
	s.v_put = v->v_put;
	return (s);
}

struct GSnap {
	int dev_ref;
	int dev_rel;
	int gnv;
	long gnv_mp;
	long gnv_ops;
	char name[16];
	int vn_lock;
	int vn_lock_flags;
	int vn_set_state;
	int vn_set_state_value;
	int vgone;
	int vput;
};

struct Result {
	int reclaim_ret;
	int reclaim_valid;
	int ret_ptr_null;
	int ret_ptr_valid;
	int ret_is_ovp;
	VSnap ovp;
	VSnap ret;
	int ret_valid;
	int dev_refs;
	int mp_dummy;
	GSnap g;
	int heap_valid;
	unsigned char heap[VN_SIZE];
	unsigned char arena[ARENA_SIZE];
};

/* ------------------------------------------------------------------ */
/* Scenarios                                                           */
/* ------------------------------------------------------------------ */

enum {
	OP_RECLAIM = 0,
	OP_ALLOCVP = 1,
	OP_FREEVP = 2,
	OP_LIFECYCLE = 3,
	OP_VNODEOPS = 4,
	NOPS = 5
};

struct Scenario {
	int op;
	int v_type;
	int v_state;
	int v_unlock;
	int v_lockflags;
	int v_gone;
	int v_put;
	int data_sel;
	int rdev_sel;
	int mount_sel;
	int vop_sel;
	int dev_refs;
	int mp_dummy;
	int mp_null;
	int extra; /* OP_RECLAIM: dispatch via vop vector; OP_FREEVP: NULL vp */
};

static void
run_port(const Scenario &sc, Result &r)
{
	alignas(16) unsigned char arena[ARENA_SIZE];

	std::memset(arena, GUARD, ARENA_SIZE);
	std::memset(&r, 0, sizeof(r));

	pt::cdev *dev = new (arena + OFF_DEV) pt::cdev{};
	pt::mount *mp = new (arena + OFF_MP) pt::mount{};
	pt::vnode *ovp = new (arena + OFF_OVP) pt::vnode{};

	dev->si_refs = sc.dev_refs;
	mp->dummy = sc.mp_dummy;
	ovp->v_type = sc.v_type;
	ovp->v_data = sc.data_sel ? static_cast<void *>(arena + OFF_MP) :
	    nullptr;
	ovp->v_rdev = sc.rdev_sel ? dev : nullptr;
	ovp->v_mount = sc.mount_sel ? mp : nullptr;
	ovp->v_op = sc.vop_sel ? &pt::mntfs_vnodeops : nullptr;
	ovp->v_state = sc.v_state;
	ovp->v_unlock_count = sc.v_unlock;
	ovp->v_lock_flags = sc.v_lockflags;
	ovp->v_gone = sc.v_gone;
	ovp->v_put = sc.v_put;

	pt::mount *mparg = sc.mp_null ? nullptr : mp;
	pt::vnode *ret = nullptr;

	pt::stub_reset();

	switch (sc.op) {
	case OP_RECLAIM: {
		pt::vop_reclaim_args ap;

		ap.a_vp = ovp;
		r.reclaim_ret = sc.extra ?
		    pt::mntfs_vnodeops.vop_reclaim(&ap) : pt::mntfs_reclaim(&ap);
		r.reclaim_valid = 1;
		break;
	}
	case OP_ALLOCVP:
		ret = pt::mntfs_allocvp(mparg, ovp);
		r.ret_ptr_null = (ret == nullptr);
		r.ret_ptr_valid = 1;
		r.ret_is_ovp = (ret == ovp);
		break;
	case OP_FREEVP:
		pt::mntfs_freevp(sc.extra ? nullptr : ovp);
		break;
	default: {
		ret = pt::mntfs_allocvp(mparg, ovp);
		r.ret_ptr_null = (ret == nullptr);
		r.ret_ptr_valid = 1;
		r.ret_is_ovp = (ret == ovp);
		if (ret != nullptr) {
			pt::vop_reclaim_args ap;

			ap.a_vp = ret;
			r.reclaim_ret = pt::mntfs_vnodeops.vop_reclaim(&ap);
			r.reclaim_valid = 1;
			pt::mntfs_freevp(ret);
		}
		break;
	}
	}

	Ctx c{arena, &pt::mntfs_vnodeops, ret};

	r.ovp = snap(ovp, c);
	if (ret != nullptr) {
		std::memcpy(r.heap, ret, VN_SIZE);
		r.ret = snap(ret, c);
		r.ret_valid = 1;
		r.heap_valid = 1;
	}
	r.dev_refs = dev->si_refs;
	r.mp_dummy = mp->dummy;

	r.g.dev_ref = pt::dev_ref_calls();
	r.g.dev_rel = pt::dev_rel_calls();
	r.g.gnv = pt::getnewvnode_calls();
	r.g.gnv_mp = canon(pt::getnewvnode_mp(), c);
	r.g.gnv_ops = canon(pt::getnewvnode_ops(), c);
	std::memcpy(r.g.name, pt::getnewvnode_name(), sizeof(r.g.name));
	r.g.vn_lock = pt::vn_lock_calls();
	r.g.vn_lock_flags = pt::vn_lock_flags();
	r.g.vn_set_state = pt::vn_set_state_calls();
	r.g.vn_set_state_value = pt::vn_set_state_value();
	r.g.vgone = pt::vgone_calls();
	r.g.vput = pt::vput_calls();

	std::memcpy(r.arena, arena, ARENA_SIZE);
	std::free(ret);
}

static void
run_ref(const Scenario &sc, Result &r)
{
	alignas(16) unsigned char arena[ARENA_SIZE];

	std::memset(arena, GUARD, ARENA_SIZE);
	std::memset(&r, 0, sizeof(r));

	ora::cdev *dev = new (arena + OFF_DEV) ora::cdev{};
	ora::mount *mp = new (arena + OFF_MP) ora::mount{};
	ora::vnode *ovp = new (arena + OFF_OVP) ora::vnode{};

	dev->si_refs = sc.dev_refs;
	mp->dummy = sc.mp_dummy;
	ovp->v_type = sc.v_type;
	ovp->v_data = sc.data_sel ? static_cast<void *>(arena + OFF_MP) :
	    nullptr;
	ovp->v_rdev = sc.rdev_sel ? dev : nullptr;
	ovp->v_mount = sc.mount_sel ? mp : nullptr;
	ovp->v_op = sc.vop_sel ? &ora::ref_mntfs_vnodeops : nullptr;
	ovp->v_state = sc.v_state;
	ovp->v_unlock_count = sc.v_unlock;
	ovp->v_lock_flags = sc.v_lockflags;
	ovp->v_gone = sc.v_gone;
	ovp->v_put = sc.v_put;

	ora::mount *mparg = sc.mp_null ? nullptr : mp;
	ora::vnode *ret = nullptr;

	ora::ref_stub_reset();

	switch (sc.op) {
	case OP_RECLAIM: {
		ora::vop_reclaim_args ap;

		ap.a_vp = ovp;
		r.reclaim_ret = sc.extra ?
		    ora::ref_mntfs_vnodeops.vop_reclaim(&ap) :
		    ora::ref_mntfs_reclaim(&ap);
		r.reclaim_valid = 1;
		break;
	}
	case OP_ALLOCVP:
		ret = ora::ref_mntfs_allocvp(mparg, ovp);
		r.ret_ptr_null = (ret == nullptr);
		r.ret_ptr_valid = 1;
		r.ret_is_ovp = (ret == ovp);
		break;
	case OP_FREEVP:
		ora::ref_mntfs_freevp(sc.extra ? nullptr : ovp);
		break;
	default: {
		ret = ora::ref_mntfs_allocvp(mparg, ovp);
		r.ret_ptr_null = (ret == nullptr);
		r.ret_ptr_valid = 1;
		r.ret_is_ovp = (ret == ovp);
		if (ret != nullptr) {
			ora::vop_reclaim_args ap;

			ap.a_vp = ret;
			r.reclaim_ret =
			    ora::ref_mntfs_vnodeops.vop_reclaim(&ap);
			r.reclaim_valid = 1;
			ora::ref_mntfs_freevp(ret);
		}
		break;
	}
	}

	Ctx c{arena, &ora::ref_mntfs_vnodeops, ret};

	r.ovp = snap(ovp, c);
	if (ret != nullptr) {
		std::memcpy(r.heap, ret, VN_SIZE);
		r.ret = snap(ret, c);
		r.ret_valid = 1;
		r.heap_valid = 1;
	}
	r.dev_refs = dev->si_refs;
	r.mp_dummy = mp->dummy;

	r.g.dev_ref = ora::ref_dev_ref_calls;
	r.g.dev_rel = ora::ref_dev_rel_calls;
	r.g.gnv = ora::ref_getnewvnode_calls;
	r.g.gnv_mp = canon(ora::ref_getnewvnode_mp, c);
	r.g.gnv_ops = canon(ora::ref_getnewvnode_ops, c);
	std::memcpy(r.g.name, ora::ref_getnewvnode_name, sizeof(r.g.name));
	r.g.vn_lock = ora::ref_vn_lock_calls;
	r.g.vn_lock_flags = ora::ref_vn_lock_flags;
	r.g.vn_set_state = ora::ref_vn_set_state_calls;
	r.g.vn_set_state_value = ora::ref_vn_set_state_value;
	r.g.vgone = ora::ref_vgone_calls;
	r.g.vput = ora::ref_vput_calls;

	std::memcpy(r.arena, arena, ARENA_SIZE);
	std::free(ret);
}

/* ------------------------------------------------------------------ */
/* Comparison                                                          */
/* ------------------------------------------------------------------ */

#define CHK(f)								\
	do {								\
		if (a.f != b.f) {					\
			std::snprintf(why, whyn,			\
			    "%s port=%lld ref=%lld", #f,		\
			    (long long)a.f, (long long)b.f);		\
			return (false);					\
		}							\
	} while (0)

static bool
cmp_results(const Result &a, const Result &b, char *why, std::size_t whyn)
{
	CHK(reclaim_valid);
	if (a.reclaim_valid)
		CHK(reclaim_ret);
	CHK(ret_ptr_valid);
	CHK(ret_ptr_null);
	CHK(ret_is_ovp);

	CHK(ovp.v_type);
	CHK(ovp.v_data);
	CHK(ovp.v_rdev);
	CHK(ovp.v_mount);
	CHK(ovp.v_op);
	CHK(ovp.v_state);
	CHK(ovp.v_unlock_count);
	CHK(ovp.v_lock_flags);
	CHK(ovp.v_gone);
	CHK(ovp.v_put);

	CHK(ret_valid);
	if (a.ret_valid) {
		CHK(ret.v_type);
		CHK(ret.v_data);
		CHK(ret.v_rdev);
		CHK(ret.v_mount);
		CHK(ret.v_op);
		CHK(ret.v_state);
		CHK(ret.v_unlock_count);
		CHK(ret.v_lock_flags);
		CHK(ret.v_gone);
		CHK(ret.v_put);
	}

	CHK(dev_refs);
	CHK(mp_dummy);

	CHK(g.dev_ref);
	CHK(g.dev_rel);
	CHK(g.gnv);
	CHK(g.gnv_mp);
	CHK(g.gnv_ops);
	CHK(g.vn_lock);
	CHK(g.vn_lock_flags);
	CHK(g.vn_set_state);
	CHK(g.vn_set_state_value);
	CHK(g.vgone);
	CHK(g.vput);

	for (std::size_t i = 0; i < sizeof(a.g.name); i++) {
		if (a.g.name[i] != b.g.name[i]) {
			std::snprintf(why, whyn,
			    "getnewvnode_name[%zu] port=0x%02x ref=0x%02x", i,
			    (unsigned)(unsigned char)a.g.name[i],
			    (unsigned)(unsigned char)b.g.name[i]);
			return (false);
		}
	}

	CHK(heap_valid);
	if (a.heap_valid) {
		for (std::size_t i = 0; i < VN_SIZE; i++) {
			if (g_skip_vnode[i])
				continue;
			if (a.heap[i] != b.heap[i]) {
				std::snprintf(why, whyn,
				    "returned vnode byte %zu port=0x%02x "
				    "ref=0x%02x", i, a.heap[i], b.heap[i]);
				return (false);
			}
		}
	}

	for (std::size_t i = 0; i < ARENA_SIZE; i++) {
		if (g_skip_arena[i])
			continue;
		if (a.arena[i] != b.arena[i]) {
			std::snprintf(why, whyn,
			    "arena byte %zu port=0x%02x ref=0x%02x", i,
			    a.arena[i], b.arena[i]);
			return (false);
		}
	}

	return (true);
}

/* ------------------------------------------------------------------ */
/* Driver                                                              */
/* ------------------------------------------------------------------ */

static const char *const g_opname[NOPS] = {
	"mntfs_reclaim",
	"mntfs_allocvp",
	"mntfs_freevp",
	"mntfs lifecycle",
	"mntfs_vnodeops",
};

static long g_cases[NOPS];
static long g_fails[NOPS];
static int g_reported;

static void
run_case(const Scenario &sc)
{
	Result rp, rr;
	char why[256];

	why[0] = '\0';
	run_port(sc, rp);
	run_ref(sc, rr);
	g_cases[sc.op]++;
	if (!cmp_results(rp, rr, why, sizeof(why))) {
		g_fails[sc.op]++;
		if (g_reported < 10) {
			g_reported++;
			std::printf("FAIL %s: %s\n", g_opname[sc.op], why);
			std::printf("     type=%d state=%d unlock=%d "
			    "lockflags=%d gone=%d put=%d\n", sc.v_type,
			    sc.v_state, sc.v_unlock, sc.v_lockflags, sc.v_gone,
			    sc.v_put);
			std::printf("     data=%d rdev=%d mount=%d vop=%d "
			    "refs=%d dummy=%d mpnull=%d extra=%d\n",
			    sc.data_sel, sc.rdev_sel, sc.mount_sel, sc.vop_sel,
			    sc.dev_refs, sc.mp_dummy, sc.mp_null, sc.extra);
		}
	}
}

/*
 * The static vop_vector is data, not code, so it is compared structurally:
 * both sides must agree on which slots are filled and on which function the
 * reclaim slot points at.
 */
static void
check_vnodeops(void)
{
	int p[6], o[6];

	p[0] = pt::mntfs_vnodeops.vop_default != nullptr;
	p[1] = pt::mntfs_vnodeops.vop_fsync != nullptr;
	p[2] = pt::mntfs_vnodeops.vop_strategy != nullptr;
	p[3] = pt::mntfs_vnodeops.vop_reclaim == &pt::mntfs_reclaim;
	p[4] = pt::mntfs_vnodeops.vop_fsync == pt::mntfs_vnodeops.vop_strategy;
	p[5] = pt::mntfs_vnodeops.vop_default == &pt::mntfs_vnodeops;

	o[0] = ora::ref_mntfs_vnodeops.vop_default != nullptr;
	o[1] = ora::ref_mntfs_vnodeops.vop_fsync != nullptr;
	o[2] = ora::ref_mntfs_vnodeops.vop_strategy != nullptr;
	o[3] = ora::ref_mntfs_vnodeops.vop_reclaim == &ora::ref_mntfs_reclaim;
	o[4] = ora::ref_mntfs_vnodeops.vop_fsync ==
	    ora::ref_mntfs_vnodeops.vop_strategy;
	o[5] = ora::ref_mntfs_vnodeops.vop_default ==
	    &ora::ref_mntfs_vnodeops;

	for (int i = 0; i < 6; i++) {
		g_cases[OP_VNODEOPS]++;
		if (p[i] != o[i]) {
			g_fails[OP_VNODEOPS]++;
			std::printf("FAIL mntfs_vnodeops: slot check %d "
			    "port=%d ref=%d\n", i, p[i], o[i]);
		}
	}
}

static std::uint64_t g_state = 0x9e3779b97f4a7c15ULL;

static std::uint64_t
rng(void)
{
	std::uint64_t z;

	g_state += 0x9e3779b97f4a7c15ULL;
	z = g_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

/*
 * Keep counters clear of the ends of the range: the code under test both
 * increments and decrements them and signed overflow would be undefined,
 * which would make the two sides incomparable rather than merely different.
 */
static int
clamp_counter(long long v)
{
	if (v > (long long)INT_MAX - 4)
		return (INT_MAX - 4);
	if (v < (long long)INT_MIN + 4)
		return (INT_MIN + 4);
	return ((int)v);
}

static int
rnd_int(void)
{
	return ((int)(std::uint32_t)rng());
}

static void
edge_cases(void)
{
	static const int refs_edge[] = {
		0, 1, -1, 2, -2, 7, 0x7f7f7f7f, -0x7f7f7f7f,
		INT_MAX - 4, INT_MIN + 4
	};
	struct Pack {
		int v_type, v_state, v_unlock, v_lockflags, v_gone, v_put;
	};
	static const Pack packs[] = {
		{ 0, 0, 0, 0, 0, 0 },
		{ 1, 1, 1, 1, 1, 1 },
		{ 2, 4, 0x40040, 0x40040, 1, 1 },
		{ -1, -1, -1, -1, -1, -1 },
		{ INT_MAX - 4, INT_MAX - 4, INT_MAX - 4, INT_MAX - 4,
		  INT_MAX - 4, INT_MAX - 4 },
		{ INT_MIN + 4, INT_MIN + 4, INT_MIN + 4, INT_MIN + 4,
		  INT_MIN + 4, INT_MIN + 4 },
		{ 0x7f7f7f7f, 0x7f7f7f7f, 0x7f7f7f7f, 0x7f7f7f7f, 0x7f7f7f7f,
		  0x7f7f7f7f },
		{ 2, 4, 5, 6, 7, 8 },
	};
	struct Sel {
		int data, rdev, mount, vop;
	};
	static const Sel sels[] = {
		{ 0, 0, 0, 0 },
		{ 1, 1, 1, 1 },
		{ 1, 0, 1, 0 },
	};

	for (int op = OP_RECLAIM; op <= OP_LIFECYCLE; op++)
		for (int rdev = 0; rdev < 2; rdev++)
			for (int mpnull = 0; mpnull < 2; mpnull++)
				for (std::size_t ri = 0;
				    ri < sizeof(refs_edge) / sizeof(refs_edge[0]);
				    ri++)
					for (std::size_t pi = 0;
					    pi < sizeof(packs) / sizeof(packs[0]);
					    pi++)
						for (std::size_t si = 0;
						    si < sizeof(sels) / sizeof(sels[0]);
						    si++)
							for (int extra = 0;
							    extra < 2;
							    extra++) {
								Scenario sc;

								sc.op = op;
								sc.v_type = packs[pi].v_type;
								sc.v_state = packs[pi].v_state;
								sc.v_unlock = packs[pi].v_unlock;
								sc.v_lockflags = packs[pi].v_lockflags;
								sc.v_gone = packs[pi].v_gone;
								sc.v_put = packs[pi].v_put;
								sc.data_sel = sels[si].data;
								sc.rdev_sel = rdev;
								sc.mount_sel = sels[si].mount;
								sc.vop_sel = sels[si].vop;
								sc.dev_refs = refs_edge[ri];
								sc.mp_dummy = (int)ri * 37 - 11;
								sc.mp_null = mpnull;
								sc.extra = extra;
								run_case(sc);
							}
}

static void
random_sweep(long iters)
{
	for (long i = 0; i < iters; i++) {
		Scenario sc;
		std::uint64_t v = rng();

		sc.op = (int)(v % 4);
		sc.v_type = rnd_int();
		sc.v_state = rnd_int();
		sc.v_unlock = clamp_counter(rnd_int());
		sc.v_lockflags = rnd_int();
		sc.v_gone = (int)(rng() % 3) - 1;
		sc.v_put = (int)(rng() % 3) - 1;
		if ((v >> 8) & 1)
			sc.v_gone = rnd_int();
		if ((v >> 9) & 1)
			sc.v_put = rnd_int();
		sc.data_sel = (int)((v >> 10) & 1);
		sc.rdev_sel = (int)((v >> 11) & 1);
		sc.mount_sel = (int)((v >> 12) & 1);
		sc.vop_sel = (int)((v >> 13) & 1);
		sc.dev_refs = ((v >> 14) & 3) == 0 ?
		    (int)((v >> 16) & 7) - 4 : clamp_counter(rnd_int());
		sc.mp_dummy = rnd_int();
		sc.mp_null = (int)((v >> 20) & 1);
		sc.extra = (int)((v >> 21) & 1);
		run_case(sc);
	}
}

int
main(void)
{
	long total_cases = 0;
	long total_fails = 0;

	build_skip_tables();

	check_vnodeops();
	edge_cases();
	random_sweep(200000);

	std::printf("\n%-20s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-20s %12s %12s\n", "--------------------",
	    "------------", "------------");
	for (int i = 0; i < NOPS; i++) {
		std::printf("%-20s %12ld %12ld\n", g_opname[i], g_cases[i],
		    g_fails[i]);
		total_cases += g_cases[i];
		total_fails += g_fails[i];
	}
	std::printf("%-20s %12s %12s\n", "--------------------",
	    "------------", "------------");
	std::printf("%-20s %12ld %12ld\n", "TOTAL", total_cases, total_fails);

	if (total_fails != 0) {
		std::printf("\nRESULT: FAIL\n");
		return (1);
	}
	std::printf("\nRESULT: PASS\n");
	return (0);
}
