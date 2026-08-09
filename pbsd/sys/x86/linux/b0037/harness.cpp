/*
 * Batch b0037 differential harness.
 *
 * Every ported routine is driven against the ref_ oracle in oracle.c with
 * hand-written edge cases plus a fixed-seed randomised sweep.  No routine in
 * this batch writes through a caller-supplied buffer or returns a pointer, so
 * the buffer-guard / pointer-offset comparison modes have nothing to apply to;
 * what is observable is the return value and, for linux_x86_elf_hwcap2(), the
 * cached static state, which is compared across whole call sequences.
 */

#include <cstdio>
#include <cstdint>
#include <climits>
#include <unistd.h>
#include <sys/wait.h>

import pbsd.sys.x86.linux.b0037;

namespace P = pbsd::sys_x86_linux::b0037;

extern "C" {
extern unsigned int cpu_feature;
extern unsigned int cpu_vendor_id;
extern unsigned int amd_feature;
extern unsigned int cpu_stdext_feature;
extern unsigned int cpu_stdext_feature2;

int ref_linux_vdso_tsc_selector_idx(void);
int ref_linux_vdso_cpu_selector_idx(void);
int ref_linux_translate_traps(int signal, int trap_code);
int ref_bsd_to_linux_trapcode(int code);
unsigned int ref_linux_x86_elf_hwcap2(void);
}

/* Mirrors of the header constants, used only to build test inputs. */
static const unsigned int CPU_VENDOR_AMD = 0x1022;
static const unsigned int CPU_VENDOR_HYGON = 0x1d94;
static const unsigned int CPU_VENDOR_INTEL = 0x8086;
static const unsigned int CPUID_SSE2 = 0x04000000;
static const unsigned int AMDID_RDTSCP = 0x08000000;
static const unsigned int CPUID_STDEXT_FSGSBASE = 0x00000001;
static const unsigned int CPUID_STDEXT2_RDPID = 0x00400000;

static const int LINUX_SIGBUS = 10;

struct Stat {
	const char *name;
	long cases;
	long failures;
	long reported;
};

static Stat st_tsc = { "linux_vdso_tsc_selector_idx", 0, 0, 0 };
static Stat st_cpu = { "linux_vdso_cpu_selector_idx", 0, 0, 0 };
static Stat st_trap = { "linux_translate_traps", 0, 0, 0 };
static Stat st_code = { "bsd_to_linux_trapcode", 0, 0, 0 };
static Stat st_hwcap = { "linux_x86_elf_hwcap2", 0, 0, 0 };

static void
fail(Stat &s, const char *fmt, long a, long b, long c, long d)
{
	s.failures++;
	if (s.reported < 10) {
		s.reported++;
		std::printf("  FAIL %s: ", s.name);
		std::printf(fmt, a, b, c, d);
		std::printf("\n");
	}
}

/* ------------------------------------------------------------------ */
/* fixed-seed PRNG (splitmix64), so the sweep is fully reproducible   */
/* ------------------------------------------------------------------ */

static uint64_t rng_state;

static void
rng_seed(uint64_t s)
{
	rng_state = s;
}

static uint64_t
next64(void)
{
	uint64_t z = (rng_state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return (z ^ (z >> 31));
}

static uint32_t
next32(void)
{
	return ((uint32_t)(next64() >> 32));
}

static uint32_t
below(uint32_t n)
{
	return (next32() % n);
}

/* ------------------------------------------------------------------ */
/* state plumbing: both sides always see identical machine words      */
/* ------------------------------------------------------------------ */

static void
set_state(unsigned int feat, unsigned int vend, unsigned int amd,
    unsigned int sx, unsigned int sx2)
{
	cpu_feature = feat;
	cpu_vendor_id = vend;
	amd_feature = amd;
	cpu_stdext_feature = sx;
	cpu_stdext_feature2 = sx2;

	P::cpu_feature = feat;
	P::cpu_vendor_id = vend;
	P::amd_feature = amd;
	P::cpu_stdext_feature = sx;
	P::cpu_stdext_feature2 = sx2;
}

static void
check_tsc(void)
{
	st_tsc.cases++;
	int got = P::linux_vdso_tsc_selector_idx();
	int want = ref_linux_vdso_tsc_selector_idx();
	if (got != want)
		fail(st_tsc, "cpu_feature=%#lx vendor=%#lx amd=%#lx -> "
		    "port %ld != ref", (long)cpu_feature, (long)cpu_vendor_id,
		    (long)amd_feature, (long)got);
}

static void
check_cpu(void)
{
	st_cpu.cases++;
	int got = P::linux_vdso_cpu_selector_idx();
	int want = ref_linux_vdso_cpu_selector_idx();
	if (got != want)
		fail(st_cpu, "stdext2=%#lx amd=%#lx -> port %ld != ref %ld",
		    (long)cpu_stdext_feature2, (long)amd_feature, (long)got,
		    (long)want);
}

static void
check_trap(int signal, int trap_code)
{
	st_trap.cases++;
	int got = P::linux_translate_traps(signal, trap_code);
	int want = ref_linux_translate_traps(signal, trap_code);
	if (got != want)
		fail(st_trap, "signal=%ld trap=%ld -> port %ld != ref %ld",
		    (long)signal, (long)trap_code, (long)got, (long)want);
}

static void
check_code(int code)
{
	st_code.cases++;
	int got = P::bsd_to_linux_trapcode(code);
	int want = ref_bsd_to_linux_trapcode(code);
	if (got != want)
		fail(st_code, "code=%ld -> port %ld != ref %ld", (long)code,
		    (long)got, (long)want, 0);
}

/* ------------------------------------------------------------------ */
/* linux_x86_elf_hwcap2: latch-once state, so each call sequence runs */
/* in a fresh forked child to get a fresh pair of statics.            */
/* ------------------------------------------------------------------ */

static int
hwcap2_sequence(const unsigned int *seq, int n)
{
	int fails = 0;

	for (int i = 0; i < n; i++) {
		cpu_stdext_feature = seq[i];
		P::cpu_stdext_feature = seq[i];
		unsigned int got = P::linux_x86_elf_hwcap2();
		unsigned int want = ref_linux_x86_elf_hwcap2();
		if (got != want) {
			fails++;
			std::printf("  FAIL linux_x86_elf_hwcap2: step %d of "
			    "%d, cpu_stdext_feature=%#x -> port %#x != ref "
			    "%#x\n", i, n, seq[i], got, want);
		}
	}
	return (fails);
}

static void
run_hwcap2(const unsigned int *seq, int n)
{
	std::fflush(stdout);
	pid_t pid = fork();
	if (pid < 0) {
		st_hwcap.cases += n;
		st_hwcap.failures += n;
		std::printf("  FAIL linux_x86_elf_hwcap2: fork failed\n");
		return;
	}
	if (pid == 0) {
		int f = hwcap2_sequence(seq, n);
		std::fflush(stdout);
		_exit(f > 100 ? 100 : f);
	}
	int status = 0;
	if (waitpid(pid, &status, 0) < 0 || !WIFEXITED(status)) {
		st_hwcap.cases += n;
		st_hwcap.failures += n;
		std::printf("  FAIL linux_x86_elf_hwcap2: child died\n");
		return;
	}
	st_hwcap.cases += n;
	st_hwcap.failures += WEXITSTATUS(status);
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	/*
	 * Hand-written edge cases for the two vDSO selectors: the full cross
	 * product of the interesting cpu_feature / amd_feature / vendor
	 * values, so both sides of every comparison in both routines are
	 * exercised (feature word zero and non-zero, RDTSCP set and clear,
	 * SSE2 set and clear, vendor equal to AMD, equal to HYGON, adjacent
	 * to both and unrelated).
	 */
	static const unsigned int feats[] = {
		0, 1, CPUID_SSE2, CPUID_SSE2 | 1, ~CPUID_SSE2, 0xFFFFFFFFu,
	};
	static const unsigned int amds[] = {
		0, AMDID_RDTSCP, AMDID_RDTSCP | 1, ~AMDID_RDTSCP, 0xFFFFFFFFu,
	};
	static const unsigned int vendors[] = {
		0, CPU_VENDOR_AMD, CPU_VENDOR_AMD - 1, CPU_VENDOR_AMD + 1,
		CPU_VENDOR_HYGON, CPU_VENDOR_HYGON - 1, CPU_VENDOR_HYGON + 1,
		CPU_VENDOR_INTEL, 0xFFFFFFFFu,
	};
	static const unsigned int sx2s[] = {
		0, CPUID_STDEXT2_RDPID, CPUID_STDEXT2_RDPID | 1,
		~CPUID_STDEXT2_RDPID, 0xFFFFFFFFu,
	};

	for (unsigned int f : feats) {
		for (unsigned int a : amds) {
			for (unsigned int v : vendors) {
				for (unsigned int s2 : sx2s) {
					set_state(f, v, a, 0, s2);
					check_tsc();
					check_cpu();
				}
			}
		}
	}

	/*
	 * linux_translate_traps: exhaustive over every signal near the SIGBUS
	 * boundary and every trap code in and around the switch labels,
	 * including negative codes.
	 */
	for (int sig = -4; sig <= 72; sig++)
		for (int tc = -8; tc <= 48; tc++)
			check_trap(sig, tc);
	check_trap(LINUX_SIGBUS, INT_MIN);
	check_trap(LINUX_SIGBUS, INT_MAX);
	check_trap(INT_MIN, 9);
	check_trap(INT_MAX, 12);
	check_trap(LINUX_SIGBUS - 1, 9);
	check_trap(LINUX_SIGBUS + 1, 9);

	/*
	 * bsd_to_linux_trapcode: exhaustive across the whole table, both
	 * sides of the nitems() boundary (30 -> in range, 31 -> out of
	 * range) and negative codes, which the unsigned comparison in the
	 * original folds into the out-of-range arm.
	 */
	for (int code = -80; code <= 120; code++)
		check_code(code);
	check_code(INT_MIN);
	check_code(INT_MAX);
	check_code(INT_MIN + 1);
	check_code(INT_MAX - 1);
	check_code(-1);
	check_code(30);
	check_code(31);
	check_code(32);

	/* linux_x86_elf_hwcap2 hand-written call sequences. */
	static const unsigned int hw_vals[] = {
		0, CPUID_STDEXT_FSGSBASE, 2, 3, ~CPUID_STDEXT_FSGSBASE,
		0xFFFFFFFFu,
	};
	for (unsigned int a : hw_vals) {
		for (unsigned int b : hw_vals) {
			for (unsigned int c : hw_vals) {
				unsigned int seq[3] = { a, b, c };
				run_hwcap2(seq, 3);
			}
		}
	}

	/* ---------------- fixed-seed randomised sweep ---------------- */
	rng_seed(0xB0037ULL);

	const long ITERS = 250000;
	for (long i = 0; i < ITERS; i++) {
		unsigned int f = next32();
		switch (below(5)) {
		case 0:
			f = 0;
			break;
		case 1:
			f |= CPUID_SSE2;
			break;
		case 2:
			f &= ~CPUID_SSE2;
			break;
		case 3:
			f = CPUID_SSE2;
			break;
		default:
			break;
		}

		unsigned int a = next32();
		switch (below(4)) {
		case 0:
			a |= AMDID_RDTSCP;
			break;
		case 1:
			a &= ~AMDID_RDTSCP;
			break;
		case 2:
			a = AMDID_RDTSCP;
			break;
		default:
			break;
		}

		unsigned int v;
		if (below(2) == 0)
			v = vendors[below(sizeof(vendors) / sizeof(vendors[0]))];
		else
			v = next32();

		unsigned int s2 = next32();
		switch (below(4)) {
		case 0:
			s2 |= CPUID_STDEXT2_RDPID;
			break;
		case 1:
			s2 &= ~CPUID_STDEXT2_RDPID;
			break;
		case 2:
			s2 = 0;
			break;
		default:
			break;
		}

		set_state(f, v, a, 0, s2);
		check_tsc();
		check_cpu();

		int sig;
		if (below(2) == 0)
			sig = LINUX_SIGBUS;
		else
			sig = (int)below(96) - 12;
		int tc;
		switch (below(3)) {
		case 0:
			tc = (int)below(32) + 1;
			break;
		case 1:
			tc = (int)below(96) - 12;
			break;
		default:
			tc = (int)next32();
			break;
		}
		check_trap(sig, tc);

		int code;
		switch (below(5)) {
		case 0:
			code = (int)below(96) - 12;
			break;
		case 1:
			code = (int)below(24) + 20;
			break;
		case 2:
			code = (int)next32();
			break;
		case 3:
			code = -(int)below(4096) - 1;
			break;
		default:
			code = (int)below(31);
			break;
		}
		check_code(code);
	}

	/* Randomised hwcap2 call sequences, one fresh child per sequence. */
	for (int i = 0; i < 256; i++) {
		unsigned int seq[8];
		int n = (int)below(8) + 1;
		for (int j = 0; j < n; j++) {
			unsigned int w = next32();
			switch (below(4)) {
			case 0:
				w |= CPUID_STDEXT_FSGSBASE;
				break;
			case 1:
				w &= ~CPUID_STDEXT_FSGSBASE;
				break;
			case 2:
				w = 0;
				break;
			default:
				break;
			}
			seq[j] = w;
		}
		run_hwcap2(seq, n);
	}

	/* ---------------------------- report ---------------------------- */
	Stat *all[] = { &st_tsc, &st_cpu, &st_trap, &st_code, &st_hwcap };
	long total_cases = 0, total_fails = 0;

	std::printf("\n%-32s %12s %10s %s\n", "function", "cases", "failures",
	    "result");
	std::printf("---------------------------------------------------"
	    "-----------------\n");
	for (Stat *s : all) {
		total_cases += s->cases;
		total_fails += s->failures;
		std::printf("%-32s %12ld %10ld %s\n", s->name, s->cases,
		    s->failures, s->failures == 0 ? "PASS" : "FAIL");
	}
	std::printf("---------------------------------------------------"
	    "-----------------\n");
	std::printf("%-32s %12ld %10ld %s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0 ? "PASS" : "FAIL");

	return (total_fails == 0 ? 0 : 1);
}
