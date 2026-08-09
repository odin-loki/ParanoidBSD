/*
 * b0055 differential harness.
 *
 *   port : pbsd::lib_libc_amd64_gen::b0055::{fpgetprec,fpgetround,
 *                                            fpgetmask,fpgetsticky}
 *   ref  : ref_fpgetprec / ref_fpgetround / ref_fpgetmask / ref_fpgetsticky
 *          (oracle.c, bodies verbatim from HardenedBSD)
 *
 * None of the four functions takes an argument or writes to a buffer: their
 * entire observable input is the x87 control word, the x87 status word and
 * MXCSR, and their only nominal output is the return value.  So the harness
 * plays the role a caller-supplied buffer plays elsewhere:
 *
 *   - the "input" is written by loading a fully specified 28-byte x87
 *     environment (control word, status word, tag word, FIP/FCS/FDP/FDS) plus
 *     MXCSR before *every* individual call, so neither implementation can
 *     observe state left behind by the other;
 *   - the "output buffer" is a snapshot struct.  Two of them are allocated,
 *     both pre-filled with the guard byte 0x7f, then partially written by the
 *     capture code, then compared in their ENTIRETY with memcmp: return value,
 *     all seven post-call environment words, post-call MXCSR, struct padding
 *     and 16 trailing guard bytes.  A port that returned the right value but
 *     clobbered the FPU state (fldcw where fnstcw was meant, a stray x87 push,
 *     ldmxcsr instead of stmxcsr) fails here.  Comparing only the return value
 *     would let those through.
 *
 * Coverage: the control word is swept exhaustively over all 65536 values, the
 * x87 status-word sticky byte is swept exhaustively against the MXCSR sticky
 * byte, and a fixed-seed pseudo-random sweep of 250000 iterations covers full
 * 16-bit combinations of all three registers.  Every constant in the port is
 * therefore driven from both sides of its boundary: bits inside and outside
 * each *_FLD mask (so `&' cannot become `|'), odd values in every field (so a
 * 0 shift count cannot become 1), all four rounding and precision codes, all
 * 64 mask-field values including 0x00 and 0x3f (so the `~' cannot vanish), and
 * all 4096 combinations of x87-sticky against SSE-sticky (so `|' cannot become
 * `&' and neither operand can be dropped).
 *
 * An anti-vacuity check at the end asserts that every value each function is
 * capable of returning really was observed, so the table cannot report
 * "all passed" from a harness that silently failed to vary its inputs.
 *
 * FPU-safety note: loading a status word with sticky bits set while the
 * matching control-word mask bit is clear leaves a pending unmasked x87
 * exception.  Only x87 *control* instructions (fnclex/fnstenv/fnstcw/fnstsw,
 * plus fldenv which is always preceded by fnclex) execute while such a state
 * is live, and MXCSR is always loaded with all six SSE exception masks set, so
 * no SIGFPE can be raised.  The default environment is restored before any
 * printf.
 */

#include <cstdio>
#include <cstring>

import pbsd.lib.libc.amd64.gen.b0055;

namespace port = pbsd::lib_libc_amd64_gen::b0055;

extern "C" {
int ref_fpgetprec(void);
int ref_fpgetround(void);
int ref_fpgetmask(void);
int ref_fpgetsticky(void);
}

/* ------------------------------------------------------------------ */
/* raw FPU / SSE state access                                         */
/* ------------------------------------------------------------------ */

struct fenv28 {
	unsigned w[7];	/* cw, sw, tw, fip, fcs+opcode, fdp, fds */
};

static const unsigned CW_DEFAULT = 0x037fu;
static const unsigned MXCSR_MASKS = 0x1f80u;

static inline unsigned
mk_mxcsr(unsigned raw)
{
	/*
	 * Reserved MXCSR bits 16..31 must be zero or ldmxcsr faults, and all
	 * six SSE exception masks are forced on so that no SSE instruction can
	 * trap.  fpgetsticky() reads only MXCSR[5:0], which stays free, and
	 * the forced-on mask field doubles as out-of-field noise that a
	 * widened SSE_STKY_FLD would wrongly pick up.
	 */
	return (raw & 0xffffu) | MXCSR_MASKS;
}

static inline void
set_state(unsigned cw, unsigned sw, unsigned mxcsr)
{
	fenv28 e;
	unsigned m;

	e.w[0] = cw & 0xffffu;
	e.w[1] = sw & 0xffffu;
	e.w[2] = 0xffffu;	/* tag word: every x87 register empty */
	e.w[3] = 0u;
	e.w[4] = 0u;
	e.w[5] = 0u;
	e.w[6] = 0u;

	/*
	 * fnclex first: fldenv is a waiting instruction and would raise #MF if
	 * the previous iteration left a pending unmasked exception behind.
	 */
	__asm__ __volatile__("fnclex" : : : "memory");
	__asm__ __volatile__("fldenv %0" : : "m" (e) : "memory");
	m = mk_mxcsr(mxcsr);
	__asm__ __volatile__("ldmxcsr %0" : : "m" (m) : "memory");
}

static inline void
get_state(fenv28 *e, unsigned *mxcsr)
{
	/*
	 * fnstenv is non-waiting.  It masks all x87 exceptions as a side
	 * effect, which is harmless: set_state() rewrites the whole
	 * environment before the next call.
	 */
	__asm__ __volatile__("fnstenv %0" : "=m" (*e) : : "memory");
	__asm__ __volatile__("stmxcsr %0" : "=m" (*mxcsr) : : "memory");
}

static void
restore_default(void)
{
	set_state(CW_DEFAULT, 0u, MXCSR_MASKS);
}

/* ------------------------------------------------------------------ */
/* snapshot: the "output buffer" of a single call                     */
/* ------------------------------------------------------------------ */

struct snap {
	int		ret;
	fenv28		env;
	unsigned	mxcsr;
	unsigned char	guard[16];
};

enum {
	FN_PREC = 0,
	FN_ROUND = 1,
	FN_MASK = 2,
	FN_STICKY = 3,
	NFN = 4
};

static const char *const fn_name[NFN] = {
	"fpgetprec",
	"fpgetround",
	"fpgetmask",
	"fpgetsticky"
};

static void
call_port(int fn, unsigned cw, unsigned sw, unsigned mxcsr, snap *s)
{
	volatile int r;

	std::memset(s, 0x7f, sizeof(*s));
	set_state(cw, sw, mxcsr);
	switch (fn) {
	case FN_PREC:
		r = static_cast<int>(port::fpgetprec());
		break;
	case FN_ROUND:
		r = static_cast<int>(port::fpgetround());
		break;
	case FN_MASK:
		r = static_cast<int>(port::fpgetmask());
		break;
	default:
		r = static_cast<int>(port::fpgetsticky());
		break;
	}
	get_state(&s->env, &s->mxcsr);
	s->ret = r;
}

static void
call_ref(int fn, unsigned cw, unsigned sw, unsigned mxcsr, snap *s)
{
	volatile int r;

	std::memset(s, 0x7f, sizeof(*s));
	set_state(cw, sw, mxcsr);
	switch (fn) {
	case FN_PREC:
		r = ref_fpgetprec();
		break;
	case FN_ROUND:
		r = ref_fpgetround();
		break;
	case FN_MASK:
		r = ref_fpgetmask();
		break;
	default:
		r = ref_fpgetsticky();
		break;
	}
	get_state(&s->env, &s->mxcsr);
	s->ret = r;
}

/* ------------------------------------------------------------------ */
/* bookkeeping                                                        */
/* ------------------------------------------------------------------ */

static long long cases[NFN];
static long long fails[NFN];
static unsigned long long seen[NFN];	/* bitmap of return values 0..63 */

struct failrec {
	int		fn;
	unsigned	cw, sw, mxcsr;
	int		pret, rret;
	int		state_diff;
	fenv28		penv, renv;
	unsigned	pmx, rmx;
};

static failrec faillog[16];
static int nfaillog;

static void
check(int fn, unsigned cw, unsigned sw, unsigned mxcsr)
{
	snap sp, sr;

	call_port(fn, cw, sw, mxcsr, &sp);
	call_ref(fn, cw, sw, mxcsr, &sr);

	cases[fn]++;
	if (sp.ret >= 0 && sp.ret < 64)
		seen[fn] |= 1ULL << sp.ret;

	if (std::memcmp(&sp, &sr, sizeof(sp)) != 0) {
		fails[fn]++;
		if (nfaillog < 16) {
			failrec *f = &faillog[nfaillog++];
			f->fn = fn;
			f->cw = cw & 0xffffu;
			f->sw = sw & 0xffffu;
			f->mxcsr = mk_mxcsr(mxcsr);
			f->pret = sp.ret;
			f->rret = sr.ret;
			f->state_diff = (std::memcmp(&sp.env, &sr.env,
			    sizeof(sp.env)) != 0 || sp.mxcsr != sr.mxcsr);
			f->penv = sp.env;
			f->renv = sr.env;
			f->pmx = sp.mxcsr;
			f->rmx = sr.mxcsr;
		}
	}
}

static void
check_all(unsigned cw, unsigned sw, unsigned mxcsr)
{
	for (int fn = 0; fn < NFN; fn++)
		check(fn, cw, sw, mxcsr);
}

/* ------------------------------------------------------------------ */
/* fixed-seed PRNG                                                    */
/* ------------------------------------------------------------------ */

static unsigned long long rng = 0x243f6a8885a308d3ULL;

static unsigned long long
rnd64(void)
{
	unsigned long long z;

	z = (rng += 0x9e3779b97f4a7c15ULL);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

/* ------------------------------------------------------------------ */
/* curated edge values                                                */
/* ------------------------------------------------------------------ */

/*
 * Control words: both extremes, the libc default, every rounding-control and
 * precision-control code, each individual exception-mask bit alone, each one
 * punched out of a full mask, and the reserved bits alone -- the last group is
 * what tells `_cw & FP_xxx_FLD' apart from `_cw | FP_xxx_FLD'.
 */
static const unsigned cw_edge[] = {
	0x0000u, 0xffffu, 0x037fu, 0x027fu, 0x127fu, 0x1f3fu,
	0x0001u, 0x0002u, 0x0004u, 0x0008u, 0x0010u, 0x0020u, 0x003fu,
	0x003eu, 0x003du, 0x003bu, 0x0037u, 0x002fu, 0x001fu,
	0x0040u, 0x0080u, 0x00c0u, 0x00ffu,
	0x0100u, 0x0200u, 0x0300u, 0x01ffu, 0x02ffu, 0x03ffu,
	0x0400u, 0x0800u, 0x0c00u, 0x0500u, 0x0a00u, 0x0f00u,
	0x1000u, 0x2000u, 0x4000u, 0x8000u, 0xf000u, 0xf0c0u,
	0xfcffu, 0xf3ffu, 0xff3fu, 0x0cffu, 0x03c0u, 0x0fc0u
};

/*
 * Status words: both extremes, each sticky bit alone, each one punched out of
 * a full sticky field, the stack-fault and error-summary bits, the condition
 * codes and the TOP field -- everything from 0x40 up lies outside
 * FP_STKY_FLD and must not leak into the result.
 */
static const unsigned sw_edge[] = {
	0x0000u, 0xffffu, 0x0001u, 0x0002u, 0x0004u, 0x0008u, 0x0010u,
	0x0020u, 0x003fu, 0x003eu, 0x003du, 0x003bu, 0x0037u, 0x002fu,
	0x001fu, 0x0040u, 0x0080u, 0x00c0u, 0x00ffu, 0x0100u, 0x0200u,
	0x0400u, 0x3800u, 0x4000u, 0x8000u, 0xffc0u, 0xff00u, 0x0f0fu
};

/*
 * MXCSR: mk_mxcsr() forces the six mask bits on, so what varies here is the
 * sticky field that fpgetsticky() reads, plus DAZ, the rounding field and FZ,
 * which must all stay out of the result.
 */
static const unsigned mx_edge[] = {
	0x0000u, 0xffffu, 0x0001u, 0x0002u, 0x0004u, 0x0008u, 0x0010u,
	0x0020u, 0x003fu, 0x003eu, 0x003du, 0x003bu, 0x0037u, 0x002fu,
	0x001fu, 0x0040u, 0x0060u, 0x2000u, 0x4000u, 0x6000u, 0x8000u,
	0xe000u, 0xffc0u, 0x1fc0u
};

#define NELEM(a)	(sizeof(a) / sizeof((a)[0]))

int
main(void)
{
	unsigned i, j, k;
	long long iter;
	long long total_cases = 0;
	long long total_fails = 0;
	int vacuous = 0;

	/*
	 * Phase 1 -- hand-written edge cases.  The cross product of the three
	 * curated lists puts every interesting control-word field against
	 * every interesting status-word and MXCSR field, including the
	 * all-zero and all-ones states and both sides of every bit-field
	 * boundary.
	 */
	for (i = 0; i < NELEM(cw_edge); i++)
		for (j = 0; j < NELEM(sw_edge); j++)
			for (k = 0; k < NELEM(mx_edge); k++)
				check_all(cw_edge[i], sw_edge[j], mx_edge[k]);

	/*
	 * Phase 2 -- exhaustive control word.  fpgetprec(), fpgetround() and
	 * fpgetmask() read nothing but the control word, so this is a complete
	 * enumeration of their observable domain.
	 */
	for (i = 0; i <= 0xffffu; i++) {
		check(FN_PREC, i, 0x0000u, 0x0000u);
		check(FN_ROUND, i, 0x0000u, 0x0000u);
		check(FN_MASK, i, 0x0000u, 0x0000u);
	}

	/*
	 * Phase 3 -- exhaustive x87 status byte against MXCSR sticky byte.
	 * fpgetsticky() ORs the two fields, so for all six exception bits this
	 * covers "set in x87 only", "set in SSE only", "set in both" and "set
	 * in neither".  Run twice: once with the surrounding non-sticky bits
	 * clear, once with them set.
	 */
	for (i = 0; i <= 0xffu; i++) {
		for (j = 0; j <= 0xffu; j++) {
			check(FN_STICKY, 0x037fu, i, j);
			check(FN_STICKY, 0x0000u, i | 0xff00u, j | 0xe000u);
		}
	}

	/*
	 * Phase 4 -- fixed-seed randomised sweep, full 16-bit range on all
	 * three registers, all four functions per iteration.
	 */
	for (iter = 0; iter < 250000; iter++) {
		unsigned long long r = rnd64();
		unsigned cw = (unsigned)(r & 0xffffu);
		unsigned sw = (unsigned)((r >> 16) & 0xffffu);
		unsigned mx = (unsigned)((r >> 32) & 0xffffu);

		/*
		 * Occasionally splice in a curated value so the random sweep
		 * also lands exactly on the bit-field boundaries.
		 */
		switch ((unsigned)((r >> 60) & 7u)) {
		case 0:
			cw = cw_edge[(r >> 48) % NELEM(cw_edge)];
			break;
		case 1:
			sw = sw_edge[(r >> 48) % NELEM(sw_edge)];
			break;
		case 2:
			mx = mx_edge[(r >> 48) % NELEM(mx_edge)];
			break;
		case 3:
			cw = cw_edge[(r >> 48) % NELEM(cw_edge)];
			sw = sw_edge[(r >> 52) % NELEM(sw_edge)];
			mx = mx_edge[(r >> 56) % NELEM(mx_edge)];
			break;
		default:
			break;
		}
		check_all(cw, sw, mx);
	}

	restore_default();

	/*
	 * Anti-vacuity: every value each function can return must actually
	 * have been observed, otherwise the sweep above did not drive the
	 * inputs it claims to drive.
	 */
	if ((seen[FN_PREC] & 0xfULL) != 0xfULL)
		vacuous |= 1 << FN_PREC;
	if ((seen[FN_ROUND] & 0xfULL) != 0xfULL)
		vacuous |= 1 << FN_ROUND;
	if (seen[FN_MASK] != ~0ULL)
		vacuous |= 1 << FN_MASK;
	if (seen[FN_STICKY] != ~0ULL)
		vacuous |= 1 << FN_STICKY;

	for (i = 0; i < (unsigned)nfaillog; i++) {
		const failrec *f = &faillog[i];

		std::printf("FAIL %-12s cw=%04x sw=%04x mxcsr=%04x  "
		    "port=%d(0x%x) ref=%d(0x%x)%s\n",
		    fn_name[f->fn], f->cw, f->sw, f->mxcsr,
		    f->pret, (unsigned)f->pret, f->rret, (unsigned)f->rret,
		    f->state_diff ? "  [FPU STATE DIFFERS]" : "");
		if (f->state_diff) {
			std::printf("     port env %08x %08x %08x %08x %08x "
			    "%08x %08x mxcsr %08x\n",
			    f->penv.w[0], f->penv.w[1], f->penv.w[2],
			    f->penv.w[3], f->penv.w[4], f->penv.w[5],
			    f->penv.w[6], f->pmx);
			std::printf("     ref  env %08x %08x %08x %08x %08x "
			    "%08x %08x mxcsr %08x\n",
			    f->renv.w[0], f->renv.w[1], f->renv.w[2],
			    f->renv.w[3], f->renv.w[4], f->renv.w[5],
			    f->renv.w[6], f->rmx);
		}
	}
	if (nfaillog == 16)
		std::printf("(failure log truncated at 16 entries)\n");

	std::printf("\n%-14s %14s %14s %10s\n",
	    "function", "cases", "failures", "result");
	std::printf("-------------------------------------------------------"
	    "--\n");
	for (i = 0; i < NFN; i++) {
		const char *res;

		if (fails[i] != 0)
			res = "FAIL";
		else if ((vacuous & (1 << i)) != 0)
			res = "VACUOUS";
		else
			res = "ok";
		std::printf("%-14s %14lld %14lld %10s\n",
		    fn_name[i], cases[i], fails[i], res);
		total_cases += cases[i];
		total_fails += fails[i];
	}
	std::printf("-------------------------------------------------------"
	    "--\n");
	std::printf("%-14s %14lld %14lld %10s\n", "TOTAL", total_cases,
	    total_fails, (total_fails == 0 && vacuous == 0) ? "PASS" : "FAIL");

	if (vacuous != 0) {
		std::printf("\nharness rejected: input sweep was vacuous for "
		    "one or more functions (mask 0x%x)\n", (unsigned)vacuous);
		return 1;
	}
	return total_fails == 0 ? 0 : 1;
}
