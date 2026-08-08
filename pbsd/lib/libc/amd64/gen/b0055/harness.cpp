/*
 * harness.cpp -- differential test for PBSD batch b0055 (amd64 fpget*).
 *
 * Each getter reads the live x87 control/status word and/or MXCSR.  The
 * harness saves the caller's FPU state, drives both the ref_ oracle and the
 * C++ port through identical hardware states, and compares return values.
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>

import pbsd.lib.libc.amd64.gen.b0055;

namespace port = pbsd::lib_libc_amd64_gen::b0055;

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

fp_prec_t ref_fpgetprec(void);
fp_rnd_t ref_fpgetround(void);
fp_except_t ref_fpgetmask(void);
fp_except_t ref_fpgetsticky(void);

}

#define	FP_MSKS_FLD	0x3f
#define	FP_PRC_FLD	0x300
#define	FP_RND_FLD	0xc00
#define	FP_STKY_FLD	0x3f
#define	FP_MSKS_OFF	0
#define	FP_PRC_OFF	8
#define	FP_RND_OFF	10
#define	FP_STKY_OFF	0
#define	SSE_STKY_FLD	0x3f

struct alignas(16) FpEnv28 {
	unsigned	fcw;
	unsigned	fsw;
	unsigned	ftw;
	unsigned	fpu_op;
	unsigned	fpu_sel;
	unsigned	fpu_ip;
	unsigned	fpu_dp;
};

struct SavedFp {
	FpEnv28		x87;
	unsigned	mxcsr;
};

alignas(16) static FpEnv28 g_fp_env;

struct FnStats {
	const char	*name;
	unsigned long	cases;
	unsigned long	failures;
};

static FnStats stats_fpgetprec = { "fpgetprec", 0, 0 };
static FnStats stats_fpgetround = { "fpgetround", 0, 0 };
static FnStats stats_fpgetmask = { "fpgetmask", 0, 0 };
static FnStats stats_fpgetsticky = { "fpgetsticky", 0, 0 };

static unsigned
xorshift32(unsigned *state)
{
	unsigned x = *state;

	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	*state = x;
	return (x);
}

static void
fnstenv(FpEnv28 *env)
{
	__asm__ __volatile__("fnstenv %0" : "=m"(*env));
}

static void
fldenv(const FpEnv28 *env)
{
	__asm__ __volatile__("fldenv %0" : : "m"(*env));
}

static void
stmxcsr(unsigned *mxcsr)
{
	__asm__ __volatile__("stmxcsr %0" : "=m"(*mxcsr));
}

static void
ldmxcsr(unsigned mxcsr)
{
	__asm__ __volatile__("ldmxcsr %0" : : "m"(mxcsr));
}

static void
save_fp(SavedFp *s)
{
	fnstenv(&g_fp_env);
	s->x87 = g_fp_env;
	stmxcsr(&s->mxcsr);
}

static void
restore_fp(const SavedFp *s)
{
	g_fp_env = s->x87;
	fldenv(&g_fp_env);
	ldmxcsr(s->mxcsr);
}

static unsigned short
build_cw(unsigned rnd, unsigned prc, unsigned mask_en)
{
	unsigned short cw;

	cw = 0x037f;
	cw &= (unsigned short)~(FP_RND_FLD | FP_PRC_FLD | FP_MSKS_FLD);
	cw |= (unsigned short)((rnd << FP_RND_OFF) & FP_RND_FLD);
	cw |= (unsigned short)((prc << FP_PRC_OFF) & FP_PRC_FLD);
	cw |= (unsigned short)((~mask_en << FP_MSKS_OFF) & FP_MSKS_FLD);
	return (cw);
}

static unsigned
stick_mxcsr(unsigned base, unsigned sticky)
{
	return ((base & ~SSE_STKY_FLD) | (sticky & SSE_STKY_FLD));
}

static void
apply_fp_state(const SavedFp *base, unsigned short cw, unsigned short sw,
    unsigned mxcsr)
{
	g_fp_env = base->x87;
	g_fp_env.fcw = (g_fp_env.fcw & ~0xffffu) | (cw & 0xffffu);
	g_fp_env.fsw = (g_fp_env.fsw & ~0xffffu) |
	    ((g_fp_env.fsw & ~FP_STKY_FLD) | (sw & FP_STKY_FLD)) & 0xffffu;
	fldenv(&g_fp_env);
	ldmxcsr(mxcsr);
}

static int
compare_int(const char *fn, int refv, int portv, FnStats *st)
{
	st->cases++;
	if (refv != portv) {
		st->failures++;
		if (st->failures <= 3)
			std::fprintf(stderr,
			    "%s mismatch: ref=%d port=%d\n", fn, refv,
			    portv);
		return (1);
	}
	return (0);
}

static void
test_fpgetprec(const SavedFp *base, unsigned rnd, unsigned prc,
    unsigned mask_en)
{
	unsigned short cw;
	int refv, portv;

	cw = build_cw(rnd, prc, mask_en);
	apply_fp_state(base, cw, 0, base->mxcsr);
	refv = ref_fpgetprec();
	portv = port::fpgetprec();
	compare_int("fpgetprec", refv, portv, &stats_fpgetprec);
}

static void
test_fpgetround(const SavedFp *base, unsigned rnd, unsigned prc,
    unsigned mask_en)
{
	unsigned short cw;
	int refv, portv;

	cw = build_cw(rnd, prc, mask_en);
	apply_fp_state(base, cw, 0, base->mxcsr);
	refv = ref_fpgetround();
	portv = port::fpgetround();
	compare_int("fpgetround", refv, portv, &stats_fpgetround);
}

static void
test_fpgetmask(const SavedFp *base, unsigned rnd, unsigned prc,
    unsigned mask_en)
{
	unsigned short cw;
	int refv, portv;

	cw = build_cw(rnd, prc, mask_en);
	apply_fp_state(base, cw, 0, base->mxcsr);
	refv = ref_fpgetmask();
	portv = port::fpgetmask();
	compare_int("fpgetmask", refv, portv, &stats_fpgetmask);
}

static void
test_fpgetsticky(const SavedFp *base, unsigned short cw, unsigned short sw,
    unsigned mxcsr)
{
	int refv, portv;

	apply_fp_state(base, cw, sw, mxcsr);
	refv = ref_fpgetsticky();
	portv = port::fpgetsticky();
	compare_int("fpgetsticky", refv, portv, &stats_fpgetsticky);
}

static void
run_hand_cases(const SavedFp *base)
{
	unsigned rnd, prc, mask, bit;
	unsigned mxcsr_base;

	mxcsr_base = base->mxcsr & ~SSE_STKY_FLD;

	/* default / live state */
	test_fpgetprec(base, ref_fpgetround(), ref_fpgetprec(),
	    ref_fpgetmask());
	test_fpgetround(base, ref_fpgetround(), ref_fpgetprec(),
	    ref_fpgetmask());
	test_fpgetmask(base, ref_fpgetround(), ref_fpgetprec(),
	    ref_fpgetmask());
	test_fpgetsticky(base, build_cw(ref_fpgetround(), ref_fpgetprec(),
	    ref_fpgetmask()), 0, mxcsr_base);

	/* every rounding mode */
	for (rnd = 0; rnd <= 3; rnd++)
		test_fpgetround(base, rnd, FP_PD, 0);

	/* rounding boundary: field max and one past (masked away) */
	test_fpgetround(base, 3, FP_PD, 0);
	test_fpgetround(base, 4, FP_PD, 0);

	/* every precision mode */
	for (prc = 0; prc <= 3; prc++)
		test_fpgetprec(base, FP_RN, prc, 0);

	test_fpgetprec(base, FP_RN, 3, 0);
	test_fpgetprec(base, FP_RN, 4, 0);

	/* mask: none, all, and each bit alone */
	test_fpgetmask(base, FP_RN, FP_PD, 0);
	test_fpgetmask(base, FP_RN, FP_PD, FP_MSKS_FLD);
	for (bit = 0; bit < 6; bit++)
		test_fpgetmask(base, FP_RN, FP_PD, 1u << bit);

	/* inverted-mask boundary: API 0x3f vs 0x00 */
	test_fpgetmask(base, FP_RM, FP_PS, 0x3f);
	test_fpgetmask(base, FP_RP, FP_PE, 0x00);

	/* sticky: x87 only, SSE only, both, neither */
	for (bit = 0; bit < 6; bit++) {
		test_fpgetsticky(base, build_cw(FP_RN, FP_PD, 0),
		    (unsigned short)(1u << bit), mxcsr_base);
		test_fpgetsticky(base, build_cw(FP_RN, FP_PD, 0), 0,
		    mxcsr_base | (1u << bit));
	}
	test_fpgetsticky(base, build_cw(FP_RN, FP_PD, 0), 0, mxcsr_base);
	test_fpgetsticky(base, build_cw(FP_RN, FP_PD, 0), FP_STKY_FLD,
	    mxcsr_base | SSE_STKY_FLD);
	test_fpgetsticky(base, build_cw(FP_RN, FP_PD, 0), 0x15,
	    mxcsr_base | 0x2a);

	/* high-bit control word garbage masked by fields */
	test_fpgetprec(base, FP_RN, FP_PD, 0x15);
	test_fpgetround(base, 2, FP_PD, 0x2a);
	test_fpgetmask(base, 1, 2, 0x15);
	test_fpgetsticky(base, (unsigned short)0x8080,
	    (unsigned short)0x00c0, stick_mxcsr(mxcsr_base, 0x01));

	/* high-bit patterns confined to the sticky fields */
	test_fpgetsticky(base, build_cw(1, 2, 0x3f), (unsigned short)0x00ff,
	    stick_mxcsr(mxcsr_base, 0x3f));
	test_fpgetsticky(base, build_cw(3, 1, 0x00), (unsigned short)0x00ff,
	    stick_mxcsr(mxcsr_base, 0x15));

	/* empty single-bit boundaries on each side of shifts */
	test_fpgetsticky(base, build_cw(0, 0, 0), 0, mxcsr_base);
	test_fpgetsticky(base, build_cw(3, 3, 0x3f), FP_STKY_FLD,
	    mxcsr_base | SSE_STKY_FLD);
}

static void
run_random_sweep(const SavedFp *base)
{
	unsigned state = 0xc0ffee42u;
	unsigned i;
	unsigned rnd, prc, mask;
	unsigned short sw;
	unsigned mxcsr_base;
	unsigned mxcsr;

	mxcsr_base = base->mxcsr & ~SSE_STKY_FLD;

	for (i = 0; i < 200000u; i++) {
		rnd = xorshift32(&state) & 7u;
		prc = (xorshift32(&state) >> 3) & 7u;
		mask = xorshift32(&state) & 0x3fu;
		sw = (unsigned short)(xorshift32(&state) & 0xffu);
		mxcsr = stick_mxcsr(mxcsr_base, xorshift32(&state));

		test_fpgetprec(base, rnd, prc, mask);
		test_fpgetround(base, rnd, prc, mask);
		test_fpgetmask(base, rnd, prc, mask);
		test_fpgetsticky(base, build_cw(rnd, prc, mask), sw, mxcsr);
	}
}

static void
print_table(const FnStats *st)
{
	std::printf("%-14s %10lu %10lu\n", st->name, st->cases, st->failures);
}

int
main()
{
	SavedFp saved;
	unsigned long total_failures;

	save_fp(&saved);
	run_hand_cases(&saved);
	run_random_sweep(&saved);
	restore_fp(&saved);

	std::printf("function            cases   failures\n");
	print_table(&stats_fpgetprec);
	print_table(&stats_fpgetround);
	print_table(&stats_fpgetmask);
	print_table(&stats_fpgetsticky);

	total_failures = stats_fpgetprec.failures + stats_fpgetround.failures +
	    stats_fpgetmask.failures + stats_fpgetsticky.failures;
	return (total_failures == 0 ? 0 : 1);
}
