/*
 * harness.cpp -- differential test for PBSD batch b0056.
 *
 * Compares fpsetround, fpsetprec, fpsetmask and the __infinity / __nan
 * globals against the ref_ oracle on hand-written edge cases and a
 * fixed-seed randomised sweep of 200000 iterations per function.
 */

#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>

import pbsd.lib.libc.amd64.gen.b0056;

namespace port_ns = pbsd::lib_libc_amd64_gen::b0056;

static const port_ns::__infinity_un &port_infinity = port_ns::__infinity;
static const port_ns::__nan_un &port_nan = port_ns::__nan;

extern "C" {
typedef enum {
	FP_RN = 0,
	FP_RM,
	FP_RP,
	FP_RZ
} fp_rnd_t;

typedef enum {
	FP_PS = 0,
	FP_PRS,
	FP_PD,
	FP_PE
} fp_prec_t;

typedef int fp_except_t;

#define FP_X_INV	0x01
#define FP_X_DNML	0x02
#define FP_X_DZ		0x04
#define FP_X_OFL	0x08
#define FP_X_UFL	0x10
#define FP_X_IMP	0x20
#define FP_X_STK	0x40

union __infinity_un {
	unsigned char __uc[8];
	double __ud;
};

union __nan_un {
	unsigned char __uc[sizeof(float)];
	float __uf;
};

fp_rnd_t ref_fpsetround(fp_rnd_t m);
fp_prec_t ref_fpsetprec(fp_prec_t m);
fp_except_t ref_fpsetmask(fp_except_t m);
extern const union __infinity_un ref___infinity;
extern const union __nan_un ref___nan;
}

enum {
	FN_ROUND,
	FN_PREC,
	FN_MASK,
	G_INF,
	G_NAN,
	NSTAT
};

struct Stats {
	const char *name;
	long long cases;
	long long fails;
	int printed;
};

static Stats g_stat[NSTAT] = {
	{ "fpsetround", 0, 0, 0 },
	{ "fpsetprec",  0, 0, 0 },
	{ "fpsetmask",  0, 0, 0 },
	{ "__infinity", 0, 0, 0 },
	{ "__nan",      0, 0, 0 },
};

static const int MAXPRINT = 8;
static const long SWEEP = 200000;

static const unsigned char INF_BYTES[8] = {
	0, 0, 0, 0, 0, 0, 0xf0, 0x7f
};
static const unsigned char NAN_BYTES[4] = { 0, 0, 0xc0, 0xff };

static std::uint64_t rng_state = 0x0123456789abcdefULL;

struct FpSnap {
	unsigned short cw;
	unsigned mxcsr;
};

static std::uint64_t
rnd(void)
{
	std::uint64_t z;

	rng_state += 0x9e3779b97f4a7c15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static void
snap_fp(FpSnap *s)
{
	__asm__ __volatile__("fnstcw %0" : "=m"(s->cw));
	__asm__ __volatile__("stmxcsr %0" : "=m"(s->mxcsr));
}

static void
restore_fp(const FpSnap *s)
{
	__asm__ __volatile__("fldcw %0" : : "m"(s->cw));
	__asm__ __volatile__("ldmxcsr %0" : : "m"(s->mxcsr));
}

static void
set_cw(unsigned short cw)
{
	__asm__ __volatile__("fldcw %0" : : "m"(cw));
}

static void
set_mxcsr(unsigned mxcsr)
{
	__asm__ __volatile__("ldmxcsr %0" : : "m"(mxcsr));
}

static void
fail(int which, const char *label, const char *detail)
{
	g_stat[which].fails++;
	if (g_stat[which].printed++ < MAXPRINT)
		std::printf("  FAIL %-12s %-28s %s\n",
		    g_stat[which].name, label, detail);
}

static long long
total_fails(void)
{
	long long t = 0;
	for (int i = 0; i < NSTAT; i++)
		t += g_stat[i].fails;
	return t;
}

static bool
infinity_full_match(void)
{
	for (unsigned i = 0; i < 8; i++)
		if (port_infinity.__uc[i] != ref___infinity.__uc[i])
			return false;
	return port_infinity.__ud == ref___infinity.__ud &&
	    std::memcmp(&port_infinity, &ref___infinity,
	    sizeof(port_infinity)) == 0;
}

static bool
nan_full_match(void)
{
	for (unsigned i = 0; i < 4; i++)
		if (port_nan.__uc[i] != ref___nan.__uc[i])
			return false;
	return std::memcmp(&port_nan, &ref___nan, sizeof(port_nan)) == 0;
}

static bool
chk_infinity(const char *label)
{
	g_stat[G_INF].cases++;
	if (!infinity_full_match()) {
		fail(G_INF, label, "byte/memcmp/double mismatch");
		return false;
	}
	return true;
}

static bool
chk_nan(const char *label)
{
	g_stat[G_NAN].cases++;
	if (!nan_full_match()) {
		fail(G_NAN, label, "byte/memcmp/float mismatch");
		return false;
	}
	return true;
}

static bool
chk_globals(const char *label)
{
	bool ok = chk_infinity(label);
	ok = chk_nan(label) && ok;
	return ok;
}

static bool
fp_snaps_equal(const FpSnap *a, const FpSnap *b)
{
	return a->cw == b->cw && a->mxcsr == b->mxcsr;
}

static bool
chk_fpsetround(fp_rnd_t m, const char *label)
{
	FpSnap saved, after_ref, after_port;
	fp_rnd_t ret_ref, ret_port;

	g_stat[FN_ROUND].cases++;
	snap_fp(&saved);

	restore_fp(&saved);
	ret_ref = ref_fpsetround(m);
	snap_fp(&after_ref);

	restore_fp(&saved);
	ret_port = port_ns::fpsetround(m);
	snap_fp(&after_port);

	restore_fp(&saved);

	if (ret_ref != ret_port || !fp_snaps_equal(&after_ref, &after_port)) {
		char detail[160];
		std::snprintf(detail, sizeof detail,
		    "m=%d ret %d/%d cw %04x/%04x mxcsr %08x/%08x",
		    (int)m, (int)ret_port, (int)ret_ref,
		    after_port.cw, after_ref.cw,
		    after_port.mxcsr, after_ref.mxcsr);
		fail(FN_ROUND, label, detail);
		return false;
	}
	return true;
}

static bool
chk_fpsetprec(fp_prec_t m, const char *label)
{
	FpSnap saved, after_ref, after_port;
	fp_prec_t ret_ref, ret_port;

	g_stat[FN_PREC].cases++;
	snap_fp(&saved);

	restore_fp(&saved);
	ret_ref = ref_fpsetprec(m);
	snap_fp(&after_ref);

	restore_fp(&saved);
	ret_port = port_ns::fpsetprec(m);
	snap_fp(&after_port);

	restore_fp(&saved);

	if (ret_ref != ret_port || !fp_snaps_equal(&after_ref, &after_port)) {
		char detail[160];
		std::snprintf(detail, sizeof detail,
		    "m=%d ret %d/%d cw %04x/%04x mxcsr %08x/%08x",
		    (int)m, (int)ret_port, (int)ret_ref,
		    after_port.cw, after_ref.cw,
		    after_port.mxcsr, after_ref.mxcsr);
		fail(FN_PREC, label, detail);
		return false;
	}
	return true;
}

static bool
chk_fpsetmask(fp_except_t m, const char *label)
{
	FpSnap saved, after_ref, after_port;
	fp_except_t ret_ref, ret_port;

	g_stat[FN_MASK].cases++;
	snap_fp(&saved);

	restore_fp(&saved);
	ret_ref = ref_fpsetmask(m);
	snap_fp(&after_ref);

	restore_fp(&saved);
	ret_port = port_ns::fpsetmask(m);
	snap_fp(&after_port);

	restore_fp(&saved);

	if (ret_ref != ret_port || !fp_snaps_equal(&after_ref, &after_port)) {
		char detail[160];
		std::snprintf(detail, sizeof detail,
		    "m=%#x ret %#x/%#x cw %04x/%04x mxcsr %08x/%08x",
		    m, ret_port, ret_ref,
		    after_port.cw, after_ref.cw,
		    after_port.mxcsr, after_ref.mxcsr);
		fail(FN_MASK, label, detail);
		return false;
	}
	return true;
}

static void
raise_sticky_dz(void)
{
	volatile double zero = 0.0;
	volatile double one = 1.0;
	volatile double junk = one / zero;
	(void)junk;
}

static void
report(void)
{
	long long cases = 0, fails = 0;

	std::printf("\n%-18s %12s %12s\n", "function", "cases", "failures");
	std::printf("--------------------------------------------\n");
	for (int i = 0; i < NSTAT; i++) {
		std::printf("%-18s %12lld %12lld\n", g_stat[i].name,
		    g_stat[i].cases, g_stat[i].fails);
		cases += g_stat[i].cases;
		fails += g_stat[i].fails;
	}
	std::printf("--------------------------------------------\n");
	std::printf("%-18s %12lld %12lld\n", "TOTAL", cases, fails);
	std::printf("\n%s\n", fails == 0 ? "PASS" : "FAIL");
	std::fflush(stdout);
}

int
main(void)
{
	char label[96];
	FpSnap baseline;

	snap_fp(&baseline);

	/* ---- globals: hand-written edge cases ---- */

	chk_globals("edge-full-compare");

	for (unsigned i = 0; i < 8; i++) {
		g_stat[G_INF].cases++;
		std::sprintf(label, "edge-inf-byte-%u", i);
		if (port_infinity.__uc[i] != ref___infinity.__uc[i])
			fail(G_INF, label, "byte mismatch");
	}
	for (unsigned i = 0; i < 4; i++) {
		g_stat[G_NAN].cases++;
		std::sprintf(label, "edge-nan-byte-%u", i);
		if (port_nan.__uc[i] != ref___nan.__uc[i])
			fail(G_NAN, label, "byte mismatch");
	}

	for (unsigned i = 0; i < 8; i++) {
		std::sprintf(label, "edge-inf-exact-%u", i);
		g_stat[G_INF].cases++;
		if (port_infinity.__uc[i] != INF_BYTES[i])
			fail(G_INF, label, "wrong infinity byte");
	}
	for (unsigned i = 0; i < 4; i++) {
		std::sprintf(label, "edge-nan-exact-%u", i);
		g_stat[G_NAN].cases++;
		if (port_nan.__uc[i] != NAN_BYTES[i])
			fail(G_NAN, label, "wrong nan byte");
	}

	g_stat[G_INF].cases++;
	if (!std::isinf(port_infinity.__ud) || port_infinity.__ud <= 0.0)
		fail(G_INF, "edge-inf-isinf", "double is not +Inf");

	g_stat[G_NAN].cases++;
	if (!std::isnan(port_nan.__uf))
		fail(G_NAN, "edge-nan-isnan", "float is not NaN");

	/* ---- fpsetround edge cases ---- */

	static const fp_rnd_t round_vals[] = {
		FP_RN, FP_RM, FP_RP, FP_RZ,
		(fp_rnd_t)4, (fp_rnd_t)7, (fp_rnd_t)15,
		(fp_rnd_t)-1, (fp_rnd_t)0x7fff, (fp_rnd_t)0x8000
	};
	for (unsigned i = 0; i < sizeof round_vals / sizeof round_vals[0]; i++) {
		std::sprintf(label, "edge-round-%d", (int)round_vals[i]);
		chk_fpsetround(round_vals[i], label);
	}

	/* both sides of rounding-field boundaries */
	for (int m = 0; m <= 3; m++) {
		std::sprintf(label, "edge-round-boundary-%d", m);
		chk_fpsetround((fp_rnd_t)m, label);
		std::sprintf(label, "edge-round-boundary-%d-plus1", m);
		chk_fpsetround((fp_rnd_t)(m + 4), label);
	}

	/* ---- fpsetprec edge cases ---- */

	static const fp_prec_t prec_vals[] = {
		FP_PS, FP_PRS, FP_PD, FP_PE,
		(fp_prec_t)4, (fp_prec_t)7, (fp_prec_t)15,
		(fp_prec_t)-1, (fp_prec_t)0x7fff
	};
	for (unsigned i = 0; i < sizeof prec_vals / sizeof prec_vals[0]; i++) {
		std::sprintf(label, "edge-prec-%d", (int)prec_vals[i]);
		chk_fpsetprec(prec_vals[i], label);
	}

	for (int m = 0; m <= 3; m++) {
		std::sprintf(label, "edge-prec-boundary-%d", m);
		chk_fpsetprec((fp_prec_t)m, label);
		std::sprintf(label, "edge-prec-boundary-%d-plus1", m);
		chk_fpsetprec((fp_prec_t)(m + 4), label);
	}

	/* ---- fpsetmask edge cases ---- */

	static const fp_except_t mask_vals[] = {
		0,
		FP_X_INV, FP_X_DNML, FP_X_DZ, FP_X_OFL, FP_X_UFL, FP_X_IMP, FP_X_STK,
		FP_X_INV | FP_X_DZ,
		FP_X_INV | FP_X_DNML | FP_X_DZ | FP_X_OFL | FP_X_UFL | FP_X_IMP,
		0x3f, 0x7f, 0x80, 0xff, -1, 0x7fffffff
	};
	for (unsigned i = 0; i < sizeof mask_vals / sizeof mask_vals[0]; i++) {
		std::sprintf(label, "edge-mask-%#x", mask_vals[i]);
		chk_fpsetmask(mask_vals[i], label);
	}

	for (unsigned b = 0; b < 7; b++) {
		fp_except_t single = (fp_except_t)(1u << b);
		std::sprintf(label, "edge-mask-bit-%u", b);
		chk_fpsetmask(single, label);
		std::sprintf(label, "edge-mask-bit-%u-complement", b);
		chk_fpsetmask((fp_except_t)(0x3f & ~single), label);
	}

	/* ---- __fnldcw slow-path: unmasked exception with sticky bit ---- */

	{
		unsigned short cw_all_masked, cw_dz_unmasked;
		FpSnap saved;

		snap_fp(&saved);
		cw_all_masked = saved.cw | 0x003f;
		cw_dz_unmasked = (unsigned short)(cw_all_masked & ~0x0004u);

		set_cw(cw_dz_unmasked);
		raise_sticky_dz();

		chk_fpsetround(FP_RZ, "edge-fnldcw-sticky-round");
		chk_fpsetprec(FP_PD, "edge-fnldcw-sticky-prec");
		chk_fpsetmask(FP_X_INV, "edge-fnldcw-sticky-mask");

		restore_fp(&saved);
	}

	/* all masked: fast __fldcw path */
	{
		FpSnap saved;
		unsigned short cw;

		snap_fp(&saved);
		cw = (unsigned short)(saved.cw | 0x003f);
		set_cw(cw);
		chk_fpsetround(FP_RM, "edge-fnldcw-fast-round");
		chk_fpsetmask(0, "edge-fnldcw-fast-mask");
		restore_fp(&saved);
	}

	/* mxcsr rounding/mask fields: seed non-zero before set */
	{
		FpSnap saved;

		snap_fp(&saved);
		set_mxcsr(0x00001f80u);
		chk_fpsetround(FP_RP, "edge-mxcsr-seed-round");
		chk_fpsetmask(FP_X_OFL | FP_X_UFL, "edge-mxcsr-seed-mask");
		restore_fp(&saved);
	}

	/* high-bit byte patterns in control-word setup */
	for (unsigned b = 0x80; b <= 0xff; b += 0x11) {
		FpSnap saved;

		snap_fp(&saved);
		set_cw((unsigned short)(0x037f ^ (b & 0x0c00)));
		std::sprintf(label, "edge-cw-high-%02x-round", b);
		chk_fpsetround((fp_rnd_t)(b & 3), label);
		std::sprintf(label, "edge-cw-high-%02x-mask", b);
		chk_fpsetmask((fp_except_t)(b & 0x3f), label);
		restore_fp(&saved);
	}

	/* ---- randomised sweeps (200000 per function) ---- */

	for (long i = 0; i < SWEEP; i++) {
		fp_rnd_t m_rnd = (fp_rnd_t)(rnd() & 0xffff);
		fp_prec_t m_prc = (fp_prec_t)(rnd() & 0xffff);
		fp_except_t m_msk = (fp_except_t)(rnd() & 0xffffffffu);

		std::sprintf(label, "sweep-round[%ld]", i);
		chk_fpsetround(m_rnd, label);

		std::sprintf(label, "sweep-prec[%ld]", i);
		chk_fpsetprec(m_prc, label);

		std::sprintf(label, "sweep-mask[%ld]", i);
		chk_fpsetmask(m_msk, label);

		if ((i & 0x3fff) == 0) {
			FpSnap saved;
			unsigned short cw;

			snap_fp(&saved);
			cw = (unsigned short)((rnd() & 0xffff) | 0x003f);
			if ((rnd() & 1) != 0)
				cw = (unsigned short)(cw & ~((unsigned short)1u << (rnd() % 6)));
			set_cw(cw);
			if ((cw & 0x003fu) != 0x003fu && (rnd() & 3) == 0)
				raise_sticky_dz();
			std::sprintf(label, "sweep-fnldcw-round[%ld]", i);
			chk_fpsetround((fp_rnd_t)(rnd() & 0xf), label);
			std::sprintf(label, "sweep-fnldcw-mask[%ld]", i);
			chk_fpsetmask((fp_except_t)(rnd() & 0x7f), label);
			restore_fp(&saved);
		}

		std::sprintf(label, "sweep-globals[%ld]", i);
		chk_globals(label);

		unsigned inf_idx = (unsigned)(rnd() % 8);
		unsigned nan_idx = (unsigned)(rnd() % 4);
		std::sprintf(label, "sweep-inf-byte[%ld]-%u", i, inf_idx);
		g_stat[G_INF].cases++;
		if (port_infinity.__uc[inf_idx] != ref___infinity.__uc[inf_idx])
			fail(G_INF, label, "byte mismatch");
		std::sprintf(label, "sweep-nan-byte[%ld]-%u", i, nan_idx);
		g_stat[G_NAN].cases++;
		if (port_nan.__uc[nan_idx] != ref___nan.__uc[nan_idx])
			fail(G_NAN, label, "byte mismatch");
	}

	restore_fp(&baseline);
	report();
	return total_fails() == 0 ? 0 : 1;
}
