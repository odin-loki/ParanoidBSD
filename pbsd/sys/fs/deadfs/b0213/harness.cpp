/*
 * Differential test harness for PBSD batch b0213
 * (hbsd/src/sys/fs/deadfs/dead_vnops.c).
 *
 * Every ported function is driven with identical inputs alongside the ref_
 * oracle compiled from the unmodified C source.  Return values are compared,
 * and the ENTIRE surrounding scratch object -- guard-filled with 0x7f, well
 * past the nominal write window -- is compared byte for byte after every
 * call.  Pointer results are compared as offsets from their own scratch
 * base, never as raw addresses.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.sys.fs.deadfs.b0213;

namespace P = pbsd::sys_fs_deadfs::b0213;

/* ------------------------------------------------------------------ */
/* Oracle declarations: layouts must match oracle.c exactly.           */
/* ------------------------------------------------------------------ */

extern "C" {

struct ref_mount;

struct ref_vnode {
	std::uint32_t	v_vflag;
};

struct ref_vop_getwritemount_args {
	ref_vnode	*a_vp;
	ref_mount	**a_mpp;
};

struct ref_vop_lookup_args {
	ref_vnode	*a_dvp;
	ref_vnode	**a_vpp;
};

struct ref_vop_open_args {
	ref_vnode	*a_vp;
	int		a_mode;
	int		a_fdidx;
};

struct ref_vop_close_args {
	ref_vnode	*a_vp;
	int		a_fflag;
};

struct ref_vop_read_args {
	ref_vnode	*a_vp;
	int		a_ioflag;
};

struct ref_vop_write_args {
	ref_vnode	*a_vp;
	int		a_ioflag;
};

struct ref_vop_poll_args {
	ref_vnode	*a_vp;
	int		a_events;
};

struct ref_vop_unset_text_args {
	ref_vnode	*a_vp;
};

int ref_dead_getwritemount(ref_vop_getwritemount_args *ap);
int ref_dead_lookup(ref_vop_lookup_args *ap);
int ref_dead_open(ref_vop_open_args *ap);
int ref_dead_close(ref_vop_close_args *ap);
int ref_dead_read(ref_vop_read_args *ap);
int ref_dead_write(ref_vop_write_args *ap);
int ref_dead_poll(ref_vop_poll_args *ap);
int ref_dead_unset_text(ref_vop_unset_text_args *ap);

} /* extern "C" */

/* ------------------------------------------------------------------ */
/* Guard-filled scratch objects                                        */
/* ------------------------------------------------------------------ */

static const unsigned char GUARD = 0x7f;

struct ScratchP {
	P::vnode	vp;
	unsigned char	pad0[28];
	P::mount	*mp;
	unsigned char	pad1[24];
	P::vnode	*vpp;
	unsigned char	pad2[24];
};

struct ScratchR {
	ref_vnode	vp;
	unsigned char	pad0[28];
	ref_mount	*mp;
	unsigned char	pad1[24];
	ref_vnode	*vpp;
	unsigned char	pad2[24];
};

static_assert(sizeof(ScratchP) == sizeof(ScratchR), "scratch size mismatch");
static_assert(offsetof(ScratchP, mp) == offsetof(ScratchR, mp), "mp offset");
static_assert(offsetof(ScratchP, vpp) == offsetof(ScratchR, vpp), "vpp offset");
static_assert(offsetof(ScratchP, pad2) == offsetof(ScratchR, pad2), "pad2");

template <class S>
static bool
pads_intact(const S &s)
{
	for (std::size_t i = 0; i < sizeof(s.pad0); i++)
		if (s.pad0[i] != GUARD)
			return false;
	for (std::size_t i = 0; i < sizeof(s.pad1); i++)
		if (s.pad1[i] != GUARD)
			return false;
	for (std::size_t i = 0; i < sizeof(s.pad2); i++)
		if (s.pad2[i] != GUARD)
			return false;
	return true;
}

/*
 * Whole-object byte comparison, including every guard byte outside the
 * nominal write window.
 */
static bool
scratch_equal(const ScratchP &a, const ScratchR &b)
{
	return std::memcmp(&a, &b, sizeof(ScratchP)) == 0;
}

/*
 * Pointers are never compared raw between the two runs: they are normalised
 * to an offset from their own scratch base.  -1 means NULL, -2 means the
 * pointer landed outside the scratch object.
 */
static long
ptr_off(const void *p, const void *base, std::size_t n)
{
	std::uintptr_t v, b;

	if (p == nullptr)
		return -1;
	v = reinterpret_cast<std::uintptr_t>(p);
	b = reinterpret_cast<std::uintptr_t>(base);
	if (v >= b && v < b + n)
		return static_cast<long>(v - b);
	return -2;
}

/* ------------------------------------------------------------------ */
/* Bookkeeping                                                         */
/* ------------------------------------------------------------------ */

enum {
	F_GETWRITEMOUNT = 0,
	F_LOOKUP,
	F_OPEN,
	F_CLOSE,
	F_READ,
	F_WRITE,
	F_POLL,
	F_UNSET_TEXT,
	NFUNC
};

static const char *fname[NFUNC] = {
	"dead_getwritemount",
	"dead_lookup",
	"dead_open",
	"dead_close",
	"dead_read",
	"dead_write",
	"dead_poll",
	"dead_unset_text",
};

static unsigned long fcases[NFUNC];
static unsigned long ffails[NFUNC];
static unsigned long fshown[NFUNC];

static bool
should_show(int i)
{
	ffails[i]++;
	return fshown[i]++ < 8;
}

/* ------------------------------------------------------------------ */
/* Per-function differential cases                                     */
/* ------------------------------------------------------------------ */

static void
t_getwritemount(std::uint32_t vflag)
{
	ScratchP sp;
	ScratchR sr;

	std::memset(&sp, GUARD, sizeof(sp));
	std::memset(&sr, GUARD, sizeof(sr));
	sp.vp.v_vflag = vflag;
	sr.vp.v_vflag = vflag;

	P::vop_getwritemount_args ap;
	ref_vop_getwritemount_args ar;

	ap.a_vp = &sp.vp;
	ap.a_mpp = &sp.mp;
	ar.a_vp = &sr.vp;
	ar.a_mpp = &sr.mp;

	int rp = P::dead_getwritemount(&ap);
	int rr = ref_dead_getwritemount(&ar);

	long op = ptr_off(sp.mp, &sp, sizeof(sp));
	long orr = ptr_off(sr.mp, &sr, sizeof(sr));

	fcases[F_GETWRITEMOUNT]++;
	bool ok = rp == rr && scratch_equal(sp, sr) && op == orr &&
	    pads_intact(sp) && pads_intact(sr) &&
	    sp.vp.v_vflag == vflag && sr.vp.v_vflag == vflag &&
	    ap.a_vp == &sp.vp && ap.a_mpp == &sp.mp &&
	    ar.a_vp == &sr.vp && ar.a_mpp == &sr.mp;
	if (!ok && should_show(F_GETWRITEMOUNT))
		std::printf("  FAIL dead_getwritemount vflag=0x%08lx: "
		    "ret %d/%d mpp-off %ld/%ld buffer=%s\n",
		    (unsigned long)vflag, rp, rr, op, orr,
		    scratch_equal(sp, sr) ? "same" : "DIFFER");
}

static void
t_lookup(std::uint32_t vflag)
{
	ScratchP sp;
	ScratchR sr;

	std::memset(&sp, GUARD, sizeof(sp));
	std::memset(&sr, GUARD, sizeof(sr));
	sp.vp.v_vflag = vflag;
	sr.vp.v_vflag = vflag;

	P::vop_lookup_args ap;
	ref_vop_lookup_args ar;

	ap.a_dvp = &sp.vp;
	ap.a_vpp = &sp.vpp;
	ar.a_dvp = &sr.vp;
	ar.a_vpp = &sr.vpp;

	int rp = P::dead_lookup(&ap);
	int rr = ref_dead_lookup(&ar);

	long op = ptr_off(sp.vpp, &sp, sizeof(sp));
	long orr = ptr_off(sr.vpp, &sr, sizeof(sr));

	fcases[F_LOOKUP]++;
	bool ok = rp == rr && scratch_equal(sp, sr) && op == orr &&
	    pads_intact(sp) && pads_intact(sr) &&
	    sp.vp.v_vflag == vflag && sr.vp.v_vflag == vflag &&
	    ap.a_dvp == &sp.vp && ap.a_vpp == &sp.vpp &&
	    ar.a_dvp == &sr.vp && ar.a_vpp == &sr.vpp;
	if (!ok && should_show(F_LOOKUP))
		std::printf("  FAIL dead_lookup vflag=0x%08lx: "
		    "ret %d/%d vpp-off %ld/%ld buffer=%s\n",
		    (unsigned long)vflag, rp, rr, op, orr,
		    scratch_equal(sp, sr) ? "same" : "DIFFER");
}

static void
t_open(std::uint32_t vflag, int mode, int fdidx)
{
	ScratchP sp;
	ScratchR sr;

	std::memset(&sp, GUARD, sizeof(sp));
	std::memset(&sr, GUARD, sizeof(sr));
	sp.vp.v_vflag = vflag;
	sr.vp.v_vflag = vflag;

	P::vop_open_args ap;
	ref_vop_open_args ar;

	ap.a_vp = &sp.vp;
	ap.a_mode = mode;
	ap.a_fdidx = fdidx;
	ar.a_vp = &sr.vp;
	ar.a_mode = mode;
	ar.a_fdidx = fdidx;

	int rp = P::dead_open(&ap);
	int rr = ref_dead_open(&ar);

	fcases[F_OPEN]++;
	bool ok = rp == rr && scratch_equal(sp, sr) &&
	    pads_intact(sp) && pads_intact(sr) &&
	    sp.vp.v_vflag == vflag && sr.vp.v_vflag == vflag &&
	    ap.a_mode == mode && ar.a_mode == mode &&
	    ap.a_fdidx == fdidx && ar.a_fdidx == fdidx &&
	    ap.a_vp == &sp.vp && ar.a_vp == &sr.vp;
	if (!ok && should_show(F_OPEN))
		std::printf("  FAIL dead_open vflag=0x%08lx mode=%d fdidx=%d: "
		    "ret %d/%d buffer=%s\n", (unsigned long)vflag, mode, fdidx,
		    rp, rr, scratch_equal(sp, sr) ? "same" : "DIFFER");
}

static void
t_close(std::uint32_t vflag, int fflag)
{
	ScratchP sp;
	ScratchR sr;

	std::memset(&sp, GUARD, sizeof(sp));
	std::memset(&sr, GUARD, sizeof(sr));
	sp.vp.v_vflag = vflag;
	sr.vp.v_vflag = vflag;

	P::vop_close_args ap;
	ref_vop_close_args ar;

	ap.a_vp = &sp.vp;
	ap.a_fflag = fflag;
	ar.a_vp = &sr.vp;
	ar.a_fflag = fflag;

	int rp = P::dead_close(&ap);
	int rr = ref_dead_close(&ar);

	fcases[F_CLOSE]++;
	bool ok = rp == rr && scratch_equal(sp, sr) &&
	    pads_intact(sp) && pads_intact(sr) &&
	    sp.vp.v_vflag == vflag && sr.vp.v_vflag == vflag &&
	    ap.a_fflag == fflag && ar.a_fflag == fflag &&
	    ap.a_vp == &sp.vp && ar.a_vp == &sr.vp;
	if (!ok && should_show(F_CLOSE))
		std::printf("  FAIL dead_close vflag=0x%08lx fflag=%d: "
		    "ret %d/%d buffer=%s\n", (unsigned long)vflag, fflag,
		    rp, rr, scratch_equal(sp, sr) ? "same" : "DIFFER");
}

static void
t_read(std::uint32_t vflag, int ioflag)
{
	ScratchP sp;
	ScratchR sr;

	std::memset(&sp, GUARD, sizeof(sp));
	std::memset(&sr, GUARD, sizeof(sr));
	sp.vp.v_vflag = vflag;
	sr.vp.v_vflag = vflag;

	P::vop_read_args ap;
	ref_vop_read_args ar;

	ap.a_vp = &sp.vp;
	ap.a_ioflag = ioflag;
	ar.a_vp = &sr.vp;
	ar.a_ioflag = ioflag;

	int rp = P::dead_read(&ap);
	int rr = ref_dead_read(&ar);

	fcases[F_READ]++;
	bool ok = rp == rr && scratch_equal(sp, sr) &&
	    pads_intact(sp) && pads_intact(sr) &&
	    sp.vp.v_vflag == vflag && sr.vp.v_vflag == vflag &&
	    ap.a_ioflag == ioflag && ar.a_ioflag == ioflag &&
	    ap.a_vp == &sp.vp && ar.a_vp == &sr.vp;
	if (!ok && should_show(F_READ))
		std::printf("  FAIL dead_read vflag=0x%08lx ioflag=%d: "
		    "ret %d/%d buffer=%s\n", (unsigned long)vflag, ioflag,
		    rp, rr, scratch_equal(sp, sr) ? "same" : "DIFFER");
}

static void
t_write(std::uint32_t vflag, int ioflag)
{
	ScratchP sp;
	ScratchR sr;

	std::memset(&sp, GUARD, sizeof(sp));
	std::memset(&sr, GUARD, sizeof(sr));
	sp.vp.v_vflag = vflag;
	sr.vp.v_vflag = vflag;

	P::vop_write_args ap;
	ref_vop_write_args ar;

	ap.a_vp = &sp.vp;
	ap.a_ioflag = ioflag;
	ar.a_vp = &sr.vp;
	ar.a_ioflag = ioflag;

	int rp = P::dead_write(&ap);
	int rr = ref_dead_write(&ar);

	fcases[F_WRITE]++;
	bool ok = rp == rr && scratch_equal(sp, sr) &&
	    pads_intact(sp) && pads_intact(sr) &&
	    sp.vp.v_vflag == vflag && sr.vp.v_vflag == vflag &&
	    ap.a_ioflag == ioflag && ar.a_ioflag == ioflag &&
	    ap.a_vp == &sp.vp && ar.a_vp == &sr.vp;
	if (!ok && should_show(F_WRITE))
		std::printf("  FAIL dead_write vflag=0x%08lx ioflag=%d: "
		    "ret %d/%d buffer=%s\n", (unsigned long)vflag, ioflag,
		    rp, rr, scratch_equal(sp, sr) ? "same" : "DIFFER");
}

static void
t_poll(std::uint32_t vflag, int events)
{
	ScratchP sp;
	ScratchR sr;

	std::memset(&sp, GUARD, sizeof(sp));
	std::memset(&sr, GUARD, sizeof(sr));
	sp.vp.v_vflag = vflag;
	sr.vp.v_vflag = vflag;

	P::vop_poll_args ap;
	ref_vop_poll_args ar;

	ap.a_vp = &sp.vp;
	ap.a_events = events;
	ar.a_vp = &sr.vp;
	ar.a_events = events;

	int rp = P::dead_poll(&ap);
	int rr = ref_dead_poll(&ar);

	fcases[F_POLL]++;
	bool ok = rp == rr && scratch_equal(sp, sr) &&
	    pads_intact(sp) && pads_intact(sr) &&
	    sp.vp.v_vflag == vflag && sr.vp.v_vflag == vflag &&
	    ap.a_events == events && ar.a_events == events &&
	    ap.a_vp == &sp.vp && ar.a_vp == &sr.vp;
	if (!ok && should_show(F_POLL))
		std::printf("  FAIL dead_poll events=0x%08lx (%d): "
		    "ret 0x%x/0x%x buffer=%s\n",
		    (unsigned long)(unsigned)events, events,
		    (unsigned)rp, (unsigned)rr,
		    scratch_equal(sp, sr) ? "same" : "DIFFER");
}

static void
t_unset_text(std::uint32_t vflag)
{
	ScratchP sp;
	ScratchR sr;

	std::memset(&sp, GUARD, sizeof(sp));
	std::memset(&sr, GUARD, sizeof(sr));
	sp.vp.v_vflag = vflag;
	sr.vp.v_vflag = vflag;

	P::vop_unset_text_args ap;
	ref_vop_unset_text_args ar;

	ap.a_vp = &sp.vp;
	ar.a_vp = &sr.vp;

	int rp = P::dead_unset_text(&ap);
	int rr = ref_dead_unset_text(&ar);

	fcases[F_UNSET_TEXT]++;
	bool ok = rp == rr && scratch_equal(sp, sr) &&
	    pads_intact(sp) && pads_intact(sr) &&
	    sp.vp.v_vflag == vflag && sr.vp.v_vflag == vflag &&
	    ap.a_vp == &sp.vp && ar.a_vp == &sr.vp;
	if (!ok && should_show(F_UNSET_TEXT))
		std::printf("  FAIL dead_unset_text vflag=0x%08lx: "
		    "ret %d/%d buffer=%s\n", (unsigned long)vflag, rp, rr,
		    scratch_equal(sp, sr) ? "same" : "DIFFER");
}

/* ------------------------------------------------------------------ */
/* Input generation                                                    */
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state = 0x0123456789abcdefULL;

static std::uint64_t
nextr(void)
{
	std::uint64_t x = rng_state;

	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	rng_state = x;
	return x * 0x2545f4914f6cdd1dULL;
}

static const std::uint32_t ISTTY = static_cast<std::uint32_t>(P::VV_ISTTY);
static const int STD = P::POLLSTANDARD;

static std::uint32_t
gen_vflag(void)
{
	std::uint32_t r = static_cast<std::uint32_t>(nextr());

	switch (r % 10u) {
	case 0:
		return 0u;
	case 1:
		return ISTTY;
	case 2:
		return ~ISTTY;
	case 3:
		return 1u << (nextr() % 32u);
	case 4:
		/* random, never the tty bit */
		return static_cast<std::uint32_t>(nextr()) & ~ISTTY;
	case 5:
		/* random, always the tty bit */
		return static_cast<std::uint32_t>(nextr()) | ISTTY;
	case 6:
		return static_cast<std::uint32_t>(nextr()) & 0x3fffu;
	case 7:
		return 0xffffffffu;
	case 8:
		return ISTTY | (1u << (nextr() % 32u));
	default:
		return static_cast<std::uint32_t>(nextr());
	}
}

static int
gen_events(void)
{
	std::uint32_t r = static_cast<std::uint32_t>(nextr());
	std::uint32_t s = static_cast<std::uint32_t>(STD);

	switch (r % 12u) {
	case 0:
		return 0;
	case 1:
		return STD;
	case 2:
		/* strictly inside POLLSTANDARD */
		return static_cast<int>(static_cast<std::uint32_t>(nextr()) & s);
	case 3:
		return static_cast<int>(1u << (nextr() % 31u));
	case 4:
		return P::POLLIN;
	case 5:
		return P::POLLRDNORM;
	case 6:
		return P::POLLIN | P::POLLRDNORM;
	case 7:
		/* one past the top of POLLSTANDARD */
		return STD + 1;
	case 8:
		return static_cast<int>(static_cast<std::uint32_t>(nextr()));
	case 9:
		/* only bits outside POLLSTANDARD */
		return static_cast<int>(static_cast<std::uint32_t>(nextr()) & ~s);
	case 10:
		/* in-standard bits plus exactly one out-of-standard bit */
		return static_cast<int>(
		    (static_cast<std::uint32_t>(nextr()) & s) |
		    (1u << (9u + nextr() % 22u)));
	default:
		return -static_cast<int>(
		    static_cast<std::uint32_t>(nextr()) & 0x7fffffffu);
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	static const std::uint32_t edge_vflag[] = {
		0u,
		1u,				/* VV_ROOT, not a tty */
		2u,				/* VV_ISTTY alone */
		3u,				/* tty plus another bit */
		0xfffffffdu,			/* everything but the tty bit */
		0xffffffffu,			/* everything */
		0x80000000u,			/* high bit only */
		0x7fffffffu,
		0x7f7f7f7fu,			/* the guard pattern itself */
		0x00000004u,
		0x00002000u,
		0x00001ffeu,
	};
	static const int edge_events[] = {
		0,
		P::POLLIN,
		P::POLLPRI,
		P::POLLOUT,
		P::POLLERR,
		P::POLLHUP,
		P::POLLNVAL,
		P::POLLRDNORM,
		P::POLLRDBAND,
		P::POLLWRBAND,
		P::POLLIN | P::POLLRDNORM,
		P::POLLIN | P::POLLPRI,
		P::POLLRDNORM | P::POLLHUP,
		P::POLLSTANDARD,		/* boundary: all standard bits */
		P::POLLSTANDARD + 1,		/* boundary: first value past */
		0x0200,				/* lowest non-standard bit */
		P::POLLINIGNEOF,
		P::POLLRDHUP,
		P::POLLSTANDARD | P::POLLINIGNEOF,
		P::POLLIN | P::POLLRDHUP,
		P::POLLRDNORM | 0x0200,
		-1,
		0x7fffffff,
		static_cast<int>(0x80000000u),
		static_cast<int>(0xffff0000u),
		0x0400,
		0x0800,
		0x1000,
		0x00010000,
	};
	static const int edge_scalars[] = {
		0, 1, -1, 2, 0x7fffffff, static_cast<int>(0x80000000u), 0x7f,
	};

	const std::size_t nvf = sizeof(edge_vflag) / sizeof(edge_vflag[0]);
	const std::size_t nev = sizeof(edge_events) / sizeof(edge_events[0]);
	const std::size_t nsc = sizeof(edge_scalars) / sizeof(edge_scalars[0]);

	for (std::size_t i = 0; i < nvf; i++) {
		t_getwritemount(edge_vflag[i]);
		t_lookup(edge_vflag[i]);
		t_unset_text(edge_vflag[i]);
		for (std::size_t j = 0; j < nsc; j++) {
			t_open(edge_vflag[i], edge_scalars[j],
			    edge_scalars[(j + 1) % nsc]);
			t_close(edge_vflag[i], edge_scalars[j]);
			t_read(edge_vflag[i], edge_scalars[j]);
			t_write(edge_vflag[i], edge_scalars[j]);
		}
	}

	/* every bit of v_vflag alone, with the tty bit, and inverted */
	for (int b = 0; b < 32; b++) {
		std::uint32_t v = 1u << b;

		t_read(v, 0);
		t_read(v | ISTTY, 0);
		t_read(~v, 0);
		t_write(v, 0);
		t_getwritemount(v);
		t_lookup(v);
		t_open(v, b, -b);
		t_close(v, b);
		t_unset_text(v);
	}

	for (std::size_t i = 0; i < nev; i++) {
		t_poll(0u, edge_events[i]);
		t_poll(ISTTY, edge_events[i]);
		t_poll(0xffffffffu, edge_events[i]);
	}

	/* every single-bit event, and each bit added to / removed from STD */
	for (int b = 0; b < 32; b++) {
		int e = static_cast<int>(1u << b);

		t_poll(0u, e);
		t_poll(0u, e | P::POLLIN);
		t_poll(0u, e | P::POLLRDNORM);
		t_poll(0u, STD & ~e);
		t_poll(0u, STD | e);
	}

	/* exhaustive sweep of every subset of POLLSTANDARD (0x1ff) */
	for (int e = 0; e <= P::POLLSTANDARD; e++)
		t_poll(0u, e);

	/* fixed-seed randomised sweep */
	const unsigned long ITER = 250000UL;

	for (unsigned long n = 0; n < ITER; n++) {
		std::uint32_t vflag = gen_vflag();
		int events = gen_events();
		int s1 = static_cast<int>(static_cast<std::uint32_t>(nextr()));
		int s2 = static_cast<int>(static_cast<std::uint32_t>(nextr()));

		t_getwritemount(vflag);
		t_lookup(vflag);
		t_open(vflag, s1, s2);
		t_close(vflag, s1);
		t_read(vflag, s1);
		t_write(vflag, s2);
		t_poll(vflag, events);
		t_unset_text(vflag);
	}

	unsigned long total_cases = 0;
	unsigned long total_fails = 0;

	std::printf("\n%-24s %12s %12s  %s\n", "function", "cases", "failures",
	    "result");
	std::printf("-----------------------------------------------"
	    "--------------------\n");
	for (int i = 0; i < NFUNC; i++) {
		total_cases += fcases[i];
		total_fails += ffails[i];
		std::printf("%-24s %12lu %12lu  %s\n", fname[i], fcases[i],
		    ffails[i], ffails[i] == 0 ? "PASS" : "FAIL");
	}
	std::printf("-----------------------------------------------"
	    "--------------------\n");
	std::printf("%-24s %12lu %12lu  %s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0 ? "PASS" : "FAIL");

	return total_fails == 0 ? 0 : 1;
}
