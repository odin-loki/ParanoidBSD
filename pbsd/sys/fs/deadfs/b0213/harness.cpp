// Differential test harness for PBSD batch b0213.

import pbsd.sys.fs.deadfs.b0213;

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace P = pbsd::sys_fs_deadfs::b0213;

#define GUARD		0x7f
#define SWEEP		200000L
#define MAX_SHOW	8

#define ENOTDIR		20
#define ENXIO		6

#define POLLIN		0x0001
#define POLLPRI		0x0002
#define POLLOUT		0x0004
#define POLLRDNORM	0x0040
#define POLLRDBAND	0x0080
#define POLLWRBAND	0x0100
#define POLLERR		0x0008
#define POLLHUP		0x0010
#define POLLNVAL	0x0020

#define POLLSTANDARD	(POLLIN|POLLPRI|POLLOUT|POLLRDNORM|POLLRDBAND|\
			 POLLWRBAND|POLLERR|POLLHUP|POLLNVAL)

#define POLLINIGNEOF	0x2000
#define POLLRDHUP	0x4000

#define VV_ISTTY	0x0002u

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	int shown;
};

static Stat st_getwritemount = { "dead_getwritemount", 0, 0, 0 };
static Stat st_lookup = { "dead_lookup", 0, 0, 0 };
static Stat st_open = { "dead_open", 0, 0, 0 };
static Stat st_close = { "dead_close", 0, 0, 0 };
static Stat st_read = { "dead_read", 0, 0, 0 };
static Stat st_write = { "dead_write", 0, 0, 0 };
static Stat st_poll = { "dead_poll", 0, 0, 0 };
static Stat st_unset_text = { "dead_unset_text", 0, 0, 0 };

extern "C" {
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

int ref_dead_getwritemount(struct vop_getwritemount_args *ap);
int ref_dead_lookup(struct vop_lookup_args *ap);
int ref_dead_open(struct vop_open_args *ap);
int ref_dead_close(struct vop_close_args *ap);
int ref_dead_read(struct vop_read_args *ap);
int ref_dead_write(struct vop_write_args *ap);
int ref_dead_poll(struct vop_poll_args *ap);
int ref_dead_unset_text(struct vop_unset_text_args *ap);
}

struct Rng {
	std::uint64_t s;

	explicit Rng(std::uint64_t seed) : s(seed) {}

	std::uint64_t next()
	{
		s += 0x9E3779B97F4A7C15ull;
		std::uint64_t z = s;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}

	std::uint32_t u32() { return (std::uint32_t)next(); }
};

static Rng rng(0x00b0213deadULL);

static bool
fail(Stat &st, const char *what)
{
	st.fails++;
	if (st.shown < MAX_SHOW) {
		st.shown++;
		std::printf("  FAIL %s: %s\n", st.name, what);
	}
	return false;
}

static bool
guards_intact(const unsigned char *buf, std::size_t n)
{
	for (std::size_t i = 0; i < n; i++) {
		if (buf[i] != GUARD)
			return false;
	}
	return true;
}

template <typename T>
struct GuardedPtr {
	unsigned char pre;
	T *ptr;
	unsigned char post;

	void init(T *initial)
	{
		pre = GUARD;
		ptr = initial;
		post = GUARD;
	}

	T **slot() { return &ptr; }

	bool ok() const { return pre == GUARD && post == GUARD; }
};

static bool
run_getwritemount_case(Stat &st, void *initial)
{
	st.cases++;

	GuardedPtr<struct mount> gr;
	GuardedPtr<P::mount> gp;
	gr.init((struct mount *)initial);
	gp.init((P::mount *)initial);

	struct vop_getwritemount_args ar = { gr.slot() };
	P::vop_getwritemount_args ap = { gp.slot() };

	int rr = ref_dead_getwritemount(&ar);
	int rp = P::dead_getwritemount(&ap);

	if (rr != rp) {
		std::printf("    ret %d vs %d (initial=%p)\n", rr, rp, initial);
		return fail(st, "return");
	}
	if (gr.ptr != nullptr || gp.ptr != nullptr) {
		std::printf("    ptr %p vs %p (expected NULL)\n",
		    (void *)gr.ptr, (void *)gp.ptr);
		return fail(st, "output ptr");
	}
	if (!gr.ok() || !gp.ok())
		return fail(st, "guard bytes");
	return true;
}

static void
test_dead_getwritemount(void)
{
	struct mount sentinel;
	struct mount *ptrs[] = {
		&sentinel,
		(struct mount *)0xdeadbeefUL,
		(struct mount *)0x80UL,
		(struct mount *)-1,
		nullptr,
	};

	for (auto p : ptrs)
		run_getwritemount_case(st_getwritemount, p);

	for (long i = 0; i < SWEEP; i++) {
		uintptr_t v = (uintptr_t)rng.u32();
		v |= (uintptr_t)rng.u32() << 32;
		run_getwritemount_case(st_getwritemount, (void *)v);
	}
}

static bool
run_lookup_case(Stat &st, void *initial)
{
	st.cases++;

	GuardedPtr<struct vnode> gr;
	GuardedPtr<P::vnode> gp;
	gr.init((struct vnode *)initial);
	gp.init((P::vnode *)initial);

	struct vop_lookup_args ar = { gr.slot() };
	P::vop_lookup_args ap = { gp.slot() };

	int rr = ref_dead_lookup(&ar);
	int rp = P::dead_lookup(&ap);

	if (rr != rp) {
		std::printf("    ret %d vs %d (initial=%p)\n", rr, rp, initial);
		return fail(st, "return");
	}
	if (rr != ENOTDIR) {
		std::printf("    unexpected ret %d\n", rr);
		return fail(st, "ENOTDIR");
	}
	if (gr.ptr != nullptr || gp.ptr != nullptr) {
		std::printf("    ptr %p vs %p (expected NULL)\n",
		    (void *)gr.ptr, (void *)gp.ptr);
		return fail(st, "output ptr");
	}
	if (!gr.ok() || !gp.ok())
		return fail(st, "guard bytes");
	return true;
}

static void
test_dead_lookup(void)
{
	struct vnode sentinel;

	for (void *p : {
		 (void *)&sentinel,
		 (void *)0xfeedfaceUL,
		 (void *)0x80UL,
		 (void *)-1,
		 nullptr,
	     })
		run_lookup_case(st_lookup, p);

	for (long i = 0; i < SWEEP; i++) {
		uintptr_t v = (uintptr_t)rng.u32();
		v |= (uintptr_t)rng.u32() << 32;
		run_lookup_case(st_lookup, (void *)v);
	}
}

static bool
run_ret_case(Stat &st, int (*ref_fn)(void *), int (*port_fn)(void *), void *ap)
{
	st.cases++;
	int rr = ref_fn(ap);
	int rp = port_fn(ap);
	if (rr != rp) {
		std::printf("    ret %d vs %d\n", rr, rp);
		return fail(st, "return");
	}
	return true;
}

static int
wrap_ref_open(void *p)
{
	return ref_dead_open((struct vop_open_args *)p);
}

static int
wrap_port_open(void *p)
{
	return P::dead_open((P::vop_open_args *)p);
}

static int
wrap_ref_close(void *p)
{
	return ref_dead_close((struct vop_close_args *)p);
}

static int
wrap_port_close(void *p)
{
	return P::dead_close((P::vop_close_args *)p);
}

static int
wrap_ref_write(void *p)
{
	return ref_dead_write((struct vop_write_args *)p);
}

static int
wrap_port_write(void *p)
{
	return P::dead_write((P::vop_write_args *)p);
}

static int
wrap_ref_unset(void *p)
{
	return ref_dead_unset_text((struct vop_unset_text_args *)p);
}

static int
wrap_port_unset(void *p)
{
	return P::dead_unset_text((P::vop_unset_text_args *)p);
}

static void
test_dead_open(void)
{
	struct vop_open_args ar = { 0 };
	P::vop_open_args ap = { 0 };

	run_ret_case(st_open, wrap_ref_open, wrap_port_open, &ar);

	for (long i = 0; i < SWEEP; i++) {
		ar.dummy = (int)rng.u32();
		ap.dummy = ar.dummy;
		run_ret_case(st_open, wrap_ref_open, wrap_port_open, &ar);
	}
}

static void
test_dead_close(void)
{
	struct vop_close_args ar = { 0 };
	P::vop_close_args ap = { 0 };

	run_ret_case(st_close, wrap_ref_close, wrap_port_close, &ar);

	for (long i = 0; i < SWEEP; i++) {
		ar.dummy = (int)rng.u32();
		ap.dummy = ar.dummy;
		run_ret_case(st_close, wrap_ref_close, wrap_port_close, &ar);
	}
}

static void
test_dead_write(void)
{
	struct vop_write_args ar = { 0 };
	P::vop_write_args ap = { 0 };

	run_ret_case(st_write, wrap_ref_write, wrap_port_write, &ar);
	if (st_write.fails == 0 && st_write.cases > 0) {
		int rr = ref_dead_write(&ar);
		if (rr != ENXIO)
			fail(st_write, "ENXIO");
	}

	for (long i = 0; i < SWEEP; i++) {
		ar.dummy = (int)rng.u32();
		ap.dummy = ar.dummy;
		run_ret_case(st_write, wrap_ref_write, wrap_port_write, &ar);
	}
}

static void
test_dead_unset_text(void)
{
	struct vop_unset_text_args ar = { 0 };
	P::vop_unset_text_args ap = { 0 };

	run_ret_case(st_unset_text, wrap_ref_unset, wrap_port_unset, &ar);

	for (long i = 0; i < SWEEP; i++) {
		ar.dummy = (int)rng.u32();
		ap.dummy = ar.dummy;
		run_ret_case(st_unset_text, wrap_ref_unset, wrap_port_unset, &ar);
	}
}

static bool
run_read_case(Stat &st, unsigned int vflag)
{
	st.cases++;

	unsigned char buf_r[sizeof(struct vnode) + 2];
	unsigned char buf_p[sizeof(P::vnode) + 2];

	std::memset(buf_r, GUARD, sizeof(buf_r));
	std::memset(buf_p, GUARD, sizeof(buf_p));

	struct vnode *vr = (struct vnode *)(buf_r + 1);
	P::vnode *vp = (P::vnode *)(buf_p + 1);

	vr->v_vflag = vflag;
	vp->v_vflag = vflag;

	struct vop_read_args ar = { vr };
	P::vop_read_args ap = { vp };

	int rr = ref_dead_read(&ar);
	int rp = P::dead_read(&ap);

	if (rr != rp) {
		std::printf("    ret %d vs %d (vflag=0x%x)\n", rr, rp, vflag);
		return fail(st, "return");
	}
	int expect = (vflag & VV_ISTTY) ? 0 : ENXIO;
	if (rr != expect) {
		std::printf("    ret %d expect %d (vflag=0x%x)\n", rr, expect,
		    vflag);
		return fail(st, "expected errno");
	}
	if (!guards_intact(buf_r, 1) || !guards_intact(buf_r + 1 + sizeof(*vr), 1))
		return fail(st, "guard bytes ref");
	if (!guards_intact(buf_p, 1) || !guards_intact(buf_p + 1 + sizeof(*vp), 1))
		return fail(st, "guard bytes port");
	if (vr->v_vflag != vflag || vp->v_vflag != vflag)
		return fail(st, "vnode mutated");
	return true;
}

static void
test_dead_read(void)
{
	unsigned int flags[] = {
		0,
		VV_ISTTY,
		VV_ISTTY - 1,
		VV_ISTTY + 1,
		VV_ISTTY | 0x80000000u,
		0xffffffffu,
		0x80u,
		0xffu,
		0x0001u,
		0x0004u,
	};

	for (unsigned int f : flags)
		run_read_case(st_read, f);

	for (long i = 0; i < SWEEP; i++)
		run_read_case(st_read, rng.u32());
}

static int
expected_poll(int events)
{
	if (events & ~POLLSTANDARD)
		return POLLNVAL;
	return POLLHUP | ((POLLIN | POLLRDNORM) & events);
}

static bool
run_poll_case(Stat &st, int events)
{
	st.cases++;

	struct vop_poll_args ar = { events };
	P::vop_poll_args ap = { events };

	int rr = ref_dead_poll(&ar);
	int rp = P::dead_poll(&ap);
	int expect = expected_poll(events);

	if (rr != rp) {
		std::printf("    ret 0x%x vs 0x%x (events=0x%x)\n", rr, rp,
		    events);
		return fail(st, "return mismatch");
	}
	if (rr != expect) {
		std::printf("    ret 0x%x expect 0x%x (events=0x%x)\n", rr,
		    expect, events);
		return fail(st, "oracle sanity");
	}
	return true;
}

static void
test_dead_poll(void)
{
	int hand[] = {
		0,
		POLLIN,
		POLLRDNORM,
		POLLIN | POLLRDNORM,
		POLLOUT,
		POLLPRI,
		POLLRDBAND,
		POLLWRBAND,
		POLLERR,
		POLLHUP,
		POLLNVAL,
		POLLSTANDARD,
		POLLSTANDARD & ~POLLIN,
		POLLSTANDARD & ~POLLRDNORM,
		POLLSTANDARD & ~(POLLIN | POLLRDNORM),
		POLLSTANDARD | POLLIN,
		0x200,
		0x2000,
		0x4000,
		POLLINIGNEOF,
		POLLRDHUP,
		POLLINIGNEOF | POLLIN,
		POLLRDHUP | POLLRDNORM,
		~0,
		-1,
		0x7fffffff,
		0x80000000,
		(int)0xdeadbeef,
		POLLSTANDARD + 1,
		POLLSTANDARD - 1,
		(POLLIN | POLLRDNORM) ^ POLLIN,
		(POLLIN | POLLRDNORM) ^ POLLRDNORM,
	};

	for (int e : hand)
		run_poll_case(st_poll, e);

	for (int bit = 9; bit <= 31; bit++) {
		int e = 1 << bit;
		if ((e & ~POLLSTANDARD) != 0)
			run_poll_case(st_poll, e);
	}

	for (long i = 0; i < SWEEP; i++) {
		int events;
		switch (rng.u32() % 5) {
		case 0:
			events = (int)rng.u32();
			break;
		case 1:
			events = (int)(rng.u32() & POLLSTANDARD);
			break;
		case 2:
			events = (int)(rng.u32() | POLLINIGNEOF);
			break;
		case 3:
			events = (int)(rng.u32() & (POLLIN | POLLRDNORM));
			break;
		default:
			events = (int)(rng.u32() & 0xffff);
			break;
		}
		run_poll_case(st_poll, events);
	}
}

static void
print_stat(const Stat &st)
{
	std::printf("%-22s %10llu %10llu\n", st.name,
	    (unsigned long long)st.cases, (unsigned long long)st.fails);
}

int
main()
{
	test_dead_getwritemount();
	test_dead_lookup();
	test_dead_open();
	test_dead_close();
	test_dead_read();
	test_dead_write();
	test_dead_poll();
	test_dead_unset_text();

	std::printf("\n%-22s %10s %10s\n", "function", "cases", "failures");
	print_stat(st_getwritemount);
	print_stat(st_lookup);
	print_stat(st_open);
	print_stat(st_close);
	print_stat(st_read);
	print_stat(st_write);
	print_stat(st_poll);
	print_stat(st_unset_text);

	unsigned long long total_fails = st_getwritemount.fails + st_lookup.fails +
	    st_open.fails + st_close.fails + st_read.fails + st_write.fails +
	    st_poll.fails + st_unset_text.fails;

	return total_fails == 0 ? 0 : 1;
}
