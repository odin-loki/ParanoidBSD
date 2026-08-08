/*
 * harness.cpp -- differential test for PBSD batch b0142.
 *
 * Compares __archlevel_resolve in pbsd.lib.libc.amd64.string.b0142 against
 * the ref_ oracle in oracle.c with hand-written edge cases and a fixed-seed
 * randomised sweep.  Return offsets from each funcs[] base are compared, never
 * raw pointer addresses.
 */

import pbsd.lib.libc.amd64.string.b0142;

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

namespace port = pbsd::lib_libc_amd64_string::b0142;

extern "C" {
typedef unsigned int u_int;

void ref_reset_cpuid_mocks(void);
void ref_add_cpuid_mock(u_int level, u_int leaf, u_int p0, u_int p1, u_int p2,
    u_int p3);
void ref_set_test_environ(char **env);
void ref_reset_archlevel_state(void);
void *ref___archlevel_resolve(u_int feat_edx, u_int feat_ecx, u_int ext_ebx,
    u_int ext_ecx, int32_t funcs[5]);
}

namespace {

constexpr int X86_64_MAX = 4;
constexpr int MAX_REPORT = 10;
constexpr long RANDOM_ITERS = 200000;

constexpr u_int FEAT_EDX_BASELINE = 0x07808101U;
constexpr u_int FEAT_ECX_V2 = 0x00982201U;
constexpr u_int FEAT_ECX_V3 = 0x38983201U;
constexpr u_int EXT_EBX_V3 = 0x00000128U;
constexpr u_int EXT_EBX_V4 = 0xd0030128U;
constexpr u_int AMD_ECX_V2 = 0x00000001U;
constexpr u_int AMD_ECX_V3 = 0x00000021U;
constexpr u_int CPUID_SSE2 = 0x04000000U;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

Stat st_resolve{"__archlevel_resolve", 0, 0, 0};

std::uint64_t rng_state;

std::uint64_t
rnd()
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

u_int
rnd_u32()
{
	return static_cast<u_int>(rnd());
}

struct FuncBlock {
	int32_t funcs[X86_64_MAX + 1];
	unsigned char guard_before[32];
	unsigned char targets[5][16];
	unsigned char guard_after[32];
};

void
fill_guards(FuncBlock &b)
{
	std::memset(b.guard_before, 0x7f, sizeof(b.guard_before));
	std::memset(b.guard_after, 0x7f, sizeof(b.guard_after));
	for (int i = 0; i <= X86_64_MAX; i++)
		std::memset(b.targets[i], static_cast<unsigned char>(0x80 + i), 16);
}

void
setup_funcs(FuncBlock &b, unsigned mask)
{
	std::memset(b.funcs, 0, sizeof(b.funcs));
	for (int i = 0; i <= X86_64_MAX; i++) {
		if ((mask & (1U << i)) == 0)
			continue;
		b.funcs[i] = static_cast<int32_t>(
		    reinterpret_cast<char *>(&b.targets[i]) -
		    reinterpret_cast<char *>(b.funcs));
	}
}

void
reset_both()
{
	port::reset_cpuid_mocks();
	ref_reset_cpuid_mocks();
	port::reset_archlevel_state();
	ref_reset_archlevel_state();
	port::set_test_environ(nullptr);
	ref_set_test_environ(nullptr);
}

void
add_both_cpuid(u_int level, u_int leaf, u_int p0, u_int p1, u_int p2, u_int p3)
{
	port::add_cpuid_mock(level, leaf, p0, p1, p2, p3);
	ref_add_cpuid_mock(level, leaf, p0, p1, p2, p3);
}

void
set_both_environ(char **env)
{
	port::set_test_environ(env);
	ref_set_test_environ(env);
}

void
mock_amd_noext()
{
	add_both_cpuid(0x80000000U, 0, 0x80000000U, 0, 0, 0);
}

void
mock_amd_v2()
{
	add_both_cpuid(0x80000000U, 0, 0x80000001U, 0, 0, 0);
	add_both_cpuid(0x80000001U, 0, 0, 0, AMD_ECX_V2, 0);
}

void
mock_amd_v3()
{
	add_both_cpuid(0x80000000U, 0, 0x80000001U, 0, 0, 0);
	add_both_cpuid(0x80000001U, 0, 0, 0, AMD_ECX_V3, 0);
}

bool
guards_intact(const FuncBlock &b)
{
	for (std::size_t i = 0; i < sizeof(b.guard_before); i++)
		if (b.guard_before[i] != 0x7f)
			return false;
	for (std::size_t i = 0; i < sizeof(b.guard_after); i++)
		if (b.guard_after[i] != 0x7f)
			return false;
	return true;
}

bool
blocks_match(const FuncBlock &a, const FuncBlock &b)
{
	return std::memcmp(a.funcs, b.funcs, sizeof(a.funcs)) == 0 &&
	    std::memcmp(a.guard_before, b.guard_before, sizeof(a.guard_before)) == 0 &&
	    std::memcmp(a.guard_after, b.guard_after, sizeof(a.guard_after)) == 0 &&
	    std::memcmp(a.targets, b.targets, sizeof(a.targets)) == 0;
}

void
report_fail(const char *tag, u_int feat_edx, u_int feat_ecx, u_int ext_ebx,
    int32_t port_off, int32_t ref_off)
{
	if (st_resolve.reported >= MAX_REPORT)
		return;
	st_resolve.reported++;
	std::printf("  FAIL __archlevel_resolve [%s]\n", tag);
	std::printf("      feat_edx=%08" PRIx32 " feat_ecx=%08" PRIx32
	    " ext_ebx=%08" PRIx32 "\n",
	    static_cast<std::uint32_t>(feat_edx),
	    static_cast<std::uint32_t>(feat_ecx),
	    static_cast<std::uint32_t>(ext_ebx));
	std::printf("      port_off=%" PRId32 " ref_off=%" PRId32 "\n",
	    port_off, ref_off);
}

void
run_case(u_int feat_edx, u_int feat_ecx, u_int ext_ebx, u_int ext_ecx,
    char **env, unsigned func_mask, const char *tag)
{
	FuncBlock pb, rb;

	reset_both();
	mock_amd_v3();
	set_both_environ(env);
	fill_guards(pb);
	fill_guards(rb);
	setup_funcs(pb, func_mask);
	setup_funcs(rb, func_mask);

	void *port_fn = port::__archlevel_resolve(feat_edx, feat_ecx, ext_ebx,
	    ext_ecx, pb.funcs);
	void *ref_fn = ref___archlevel_resolve(feat_edx, feat_ecx, ext_ebx,
	    ext_ecx, rb.funcs);

	int32_t port_off = static_cast<int32_t>(
	    reinterpret_cast<char *>(port_fn) -
	    reinterpret_cast<char *>(pb.funcs));
	int32_t ref_off = static_cast<int32_t>(
	    reinterpret_cast<char *>(ref_fn) -
	    reinterpret_cast<char *>(rb.funcs));

	st_resolve.cases++;
	bool bad = false;

	if (port_off != ref_off)
		bad = true;
	if (!guards_intact(pb) || !guards_intact(rb))
		bad = true;
	if (!blocks_match(pb, rb))
		bad = true;

	if (bad) {
		st_resolve.fails++;
		report_fail(tag, feat_edx, feat_ecx, ext_ebx, port_off, ref_off);
	}
}

void
run_case_cpuid(u_int feat_edx, u_int feat_ecx, u_int ext_ebx, u_int ext_ecx,
    char **env, unsigned func_mask, void (*cpuid_setup)(void),
    const char *tag)
{
	reset_both();
	cpuid_setup();
	set_both_environ(env);
	FuncBlock pb, rb;
	fill_guards(pb);
	fill_guards(rb);
	setup_funcs(pb, func_mask);
	setup_funcs(rb, func_mask);

	void *port_fn = port::__archlevel_resolve(feat_edx, feat_ecx, ext_ebx,
	    ext_ecx, pb.funcs);
	void *ref_fn = ref___archlevel_resolve(feat_edx, feat_ecx, ext_ebx,
	    ext_ecx, rb.funcs);

	int32_t port_off = static_cast<int32_t>(
	    reinterpret_cast<char *>(port_fn) -
	    reinterpret_cast<char *>(pb.funcs));
	int32_t ref_off = static_cast<int32_t>(
	    reinterpret_cast<char *>(ref_fn) -
	    reinterpret_cast<char *>(rb.funcs));

	st_resolve.cases++;
	bool bad = (port_off != ref_off) || !guards_intact(pb) ||
	    !guards_intact(rb) || !blocks_match(pb, rb);
	if (bad) {
		st_resolve.fails++;
		report_fail(tag, feat_edx, feat_ecx, ext_ebx, port_off, ref_off);
	}
}

/* ------------------------------------------------------------------ */
/* edge cases                                                          */
/* ------------------------------------------------------------------ */

void
edge_cases()
{
	char env_scalar[] = "ARCHLEVEL=scalar";
	char env_baseline[] = "ARCHLEVEL=baseline";
	char env_v2[] = "ARCHLEVEL=x86-64-v2";
	char env_v3[] = "ARCHLEVEL=x86-64-v3";
	char env_v4[] = "ARCHLEVEL=x86-64-v4";
	char env_force_v4[] = "ARCHLEVEL=!x86-64-v4";
	char env_force_scalar[] = "ARCHLEVEL=!scalar";
	char env_suffix_colon[] = "ARCHLEVEL=baseline:extra";
	char env_suffix_plus[] = "ARCHLEVEL=x86-64-v2+simd";
	char env_bogus[] = "ARCHLEVEL=notalevel";
	char env_empty[] = "ARCHLEVEL=";
	char env_partial[] = "ARCHLEVEL=base";
	char env_other[] = "OTHERVAR=foo";
	char env_highbit[] = "ARCHLEVEL=\x80scalar";
	char *env_one[] = {env_scalar, nullptr};
	char *env_baseline_p[] = {env_baseline, nullptr};
	char *env_v2_p[] = {env_v2, nullptr};
	char *env_v3_p[] = {env_v3, nullptr};
	char *env_v4_p[] = {env_v4, nullptr};
	char *env_force_v4_p[] = {env_force_v4, nullptr};
	char *env_force_scalar_p[] = {env_force_scalar, nullptr};
	char *env_suffix_colon_p[] = {env_suffix_colon, nullptr};
	char *env_suffix_plus_p[] = {env_suffix_plus, nullptr};
	char *env_bogus_p[] = {env_bogus, nullptr};
	char *env_empty_p[] = {env_empty, nullptr};
	char *env_partial_p[] = {env_partial, nullptr};
	char *env_multi[] = {env_other, env_v3, nullptr};
	char *env_prefix[] = {env_highbit, nullptr};

	const unsigned all_funcs = 0x1f;

	/* no env, missing baseline -> hw level 0 */
	run_case_cpuid(0, 0, 0, 0, nullptr, all_funcs, mock_amd_noext,
	    "no-baseline");

	/* baseline only */
	run_case_cpuid(FEAT_EDX_BASELINE, 0, 0, 0, nullptr, all_funcs,
	    mock_amd_noext, "baseline-only");

	/* x86-64-v2 with AMD LAHF */
	run_case_cpuid(FEAT_EDX_BASELINE, FEAT_ECX_V2, 0, 0, nullptr,
	    all_funcs, mock_amd_v2, "hw-v2");

	/* x86-64-v3 */
	run_case_cpuid(FEAT_EDX_BASELINE, FEAT_ECX_V3, EXT_EBX_V3, 0,
	    nullptr, all_funcs, mock_amd_v3, "hw-v3");

	/* x86-64-v4 */
	run_case_cpuid(FEAT_EDX_BASELINE, FEAT_ECX_V3, EXT_EBX_V4, 0,
	    nullptr, all_funcs, mock_amd_v3, "hw-v4");

	/* missing one baseline bit -> level 0 */
	run_case_cpuid(FEAT_EDX_BASELINE & ~CPUID_SSE2, 0, 0, 0, nullptr,
	    all_funcs, mock_amd_noext, "missing-sse2");

	/* v2 features but no AMD LAHF */
	run_case_cpuid(FEAT_EDX_BASELINE, FEAT_ECX_V2, 0, 0, nullptr,
	    all_funcs, mock_amd_noext, "v2-no-amd");

	/* ARCHLEVEL=scalar on capable hw */
	run_case(FEAT_EDX_BASELINE, FEAT_ECX_V3, EXT_EBX_V4, 0, env_one,
	    all_funcs, "env-scalar");

	/* ARCHLEVEL=baseline */
	run_case(FEAT_EDX_BASELINE, FEAT_ECX_V3, EXT_EBX_V4, 0,
	    env_baseline_p, all_funcs, "env-baseline");

	/* ARCHLEVEL=x86-64-v2 */
	run_case(FEAT_EDX_BASELINE, FEAT_ECX_V3, EXT_EBX_V4, 0, env_v2_p,
	    all_funcs, "env-v2");

	/* ARCHLEVEL=x86-64-v3 */
	run_case(FEAT_EDX_BASELINE, FEAT_ECX_V3, EXT_EBX_V4, 0, env_v3_p,
	    all_funcs, "env-v3");

	/* ARCHLEVEL=x86-64-v4 on v3 hw -> clamp */
	run_case(FEAT_EDX_BASELINE, FEAT_ECX_V3, EXT_EBX_V3, 0, env_v4_p,
	    all_funcs, "env-v4-clamp");

	/* ARCHLEVEL=!x86-64-v4 forces v4 on weak hw */
	run_case_cpuid(FEAT_EDX_BASELINE, 0, 0, 0, env_force_v4_p, all_funcs,
	    mock_amd_noext, "force-v4");

	/* ARCHLEVEL=!scalar */
	run_case(FEAT_EDX_BASELINE, FEAT_ECX_V3, EXT_EBX_V4, 0,
	    env_force_scalar_p, all_funcs, "force-scalar");

	/* suffix : and + */
	run_case(FEAT_EDX_BASELINE, FEAT_ECX_V3, EXT_EBX_V4, 0,
	    env_suffix_colon_p, all_funcs, "suffix-colon");
	run_case(FEAT_EDX_BASELINE, FEAT_ECX_V3, EXT_EBX_V4, 0,
	    env_suffix_plus_p, all_funcs, "suffix-plus");

	/* invalid / empty / partial */
	run_case(FEAT_EDX_BASELINE, FEAT_ECX_V3, EXT_EBX_V3, 0, env_bogus_p,
	    all_funcs, "env-bogus");
	run_case(FEAT_EDX_BASELINE, FEAT_ECX_V3, EXT_EBX_V3, 0, env_empty_p,
	    all_funcs, "env-empty");
	run_case(FEAT_EDX_BASELINE, FEAT_ECX_V3, EXT_EBX_V3, 0, env_partial_p,
	    all_funcs, "env-partial");

	/* environ == NULL */
	run_case_cpuid(FEAT_EDX_BASELINE, FEAT_ECX_V3, EXT_EBX_V3, 0, nullptr,
	    all_funcs, mock_amd_v3, "environ-null");

	/* later ARCHLEVEL in list */
	run_case(FEAT_EDX_BASELINE, FEAT_ECX_V3, EXT_EBX_V3, 0, env_multi,
	    all_funcs, "env-multi");

	/* non-matching prefix before ARCHLEVEL= */
	run_case(FEAT_EDX_BASELINE, FEAT_ECX_V3, EXT_EBX_V3, 0, env_prefix,
	    all_funcs, "env-bad-prefix");

	/* func selection: only level 0 */
	run_case(FEAT_EDX_BASELINE, FEAT_ECX_V3, EXT_EBX_V4, 0, env_one,
	    1U << 0, "func-level0-only");

	/* skip zero higher entries */
	run_case(FEAT_EDX_BASELINE, FEAT_ECX_V3, EXT_EBX_V4, 0, nullptr,
	    (1U << 2), "func-v2-only");

	/* skip level 4, use level 3 */
	run_case(FEAT_EDX_BASELINE, FEAT_ECX_V3, EXT_EBX_V3, 0, nullptr,
	    (1U << 3), "func-v3-at-hw3");

	/* caching: second resolve must match first even if feats change */
	{
		reset_both();
		mock_amd_v3();
		set_both_environ(nullptr);
		FuncBlock pb, rb;
		fill_guards(pb);
		fill_guards(rb);
		setup_funcs(pb, all_funcs);
		setup_funcs(rb, all_funcs);

		void *p1 = port::__archlevel_resolve(FEAT_EDX_BASELINE,
		    FEAT_ECX_V3, EXT_EBX_V3, 0, pb.funcs);
		void *r1 = ref___archlevel_resolve(FEAT_EDX_BASELINE,
		    FEAT_ECX_V3, EXT_EBX_V3, 0, rb.funcs);
		void *p2 = port::__archlevel_resolve(0, 0, 0, 0, pb.funcs);
		void *r2 = ref___archlevel_resolve(0, 0, 0, 0, rb.funcs);

		int32_t p1o = static_cast<int32_t>(reinterpret_cast<char *>(p1) -
		    reinterpret_cast<char *>(pb.funcs));
		int32_t r1o = static_cast<int32_t>(reinterpret_cast<char *>(r1) -
		    reinterpret_cast<char *>(rb.funcs));
		int32_t p2o = static_cast<int32_t>(reinterpret_cast<char *>(p2) -
		    reinterpret_cast<char *>(pb.funcs));
		int32_t r2o = static_cast<int32_t>(reinterpret_cast<char *>(r2) -
		    reinterpret_cast<char *>(rb.funcs));

		st_resolve.cases++;
		if (p1o != r1o || p2o != r2o || p1o != p2o)
			st_resolve.fails++;
	}

	/* boundary: v3 hw, env wants v2 */
	run_case(FEAT_EDX_BASELINE, FEAT_ECX_V3, EXT_EBX_V3, 0, env_v2_p,
	    all_funcs, "env-v2-on-v3hw");

	/* boundary: v2 hw exactly, env wants v3 -> clamp */
	run_case_cpuid(FEAT_EDX_BASELINE, FEAT_ECX_V2, 0, 0, env_v3_p,
	    all_funcs, mock_amd_v2, "env-v3-clamp-v2");
}

void
random_sweep()
{
	const char *level_names[] = {
	    "scalar", "baseline", "x86-64-v2", "x86-64-v3", "x86-64-v4"
	};
	char envbufs[4][64];
	char env_force[64];

	for (long it = 0; it < RANDOM_ITERS; it++) {
		u_int feat_edx = rnd_u32();
		u_int feat_ecx = rnd_u32();
		u_int ext_ebx = rnd_u32();
		u_int ext_ecx = rnd_u32();
		unsigned func_mask = static_cast<unsigned>(rnd() & 0x1fU);

		reset_both();
		if ((rnd() & 3U) == 0)
			mock_amd_noext();
		else if ((rnd() & 3U) == 1)
			mock_amd_v2();
		else
			mock_amd_v3();

		char *envp[4];
		int envc = 0;
		unsigned env_kind = static_cast<unsigned>(rnd() % 8U);

		switch (env_kind) {
		case 0:
			port::set_test_environ(nullptr);
			ref_set_test_environ(nullptr);
			break;
		case 1: {
			int lvl = static_cast<int>(rnd() % 5U);
			std::snprintf(envbufs[0], sizeof(envbufs[0]),
			    "ARCHLEVEL=%s", level_names[lvl]);
			envp[envc++] = envbufs[0];
			envp[envc] = nullptr;
			set_both_environ(envp);
			break;
		}
		case 2: {
			int lvl = static_cast<int>(rnd() % 5U);
			std::snprintf(env_force, sizeof(env_force),
			    "ARCHLEVEL=!%s", level_names[lvl]);
			envp[envc++] = env_force;
			envp[envc] = nullptr;
			set_both_environ(envp);
			break;
		}
		case 3: {
			std::snprintf(envbufs[0], sizeof(envbufs[0]),
			    "ARCHLEVEL=baseline%c%02x",
			    (rnd() & 1) ? ':' : '+',
			    static_cast<unsigned>(rnd() & 0xffU));
			envp[envc++] = envbufs[0];
			envp[envc] = nullptr;
			set_both_environ(envp);
			break;
		}
		case 4:
			envp[envc++] = const_cast<char *>("ARCHLEVEL=");
			envp[envc] = nullptr;
			set_both_environ(envp);
			break;
		case 5:
			envp[envc++] = const_cast<char *>("ARCHLEVEL=xyzzy");
			envp[envc] = nullptr;
			set_both_environ(envp);
			break;
		default:
			set_both_environ(nullptr);
			break;
		}

		FuncBlock pb, rb;
		fill_guards(pb);
		fill_guards(rb);
		setup_funcs(pb, func_mask);
		setup_funcs(rb, func_mask);

		/* skip all-zero func tables (would trap) */
		if (func_mask == 0)
			continue;

		void *port_fn = port::__archlevel_resolve(feat_edx, feat_ecx,
		    ext_ebx, ext_ecx, pb.funcs);
		void *ref_fn = ref___archlevel_resolve(feat_edx, feat_ecx,
		    ext_ebx, ext_ecx, rb.funcs);

		int32_t port_off = static_cast<int32_t>(
		    reinterpret_cast<char *>(port_fn) -
		    reinterpret_cast<char *>(pb.funcs));
		int32_t ref_off = static_cast<int32_t>(
		    reinterpret_cast<char *>(ref_fn) -
		    reinterpret_cast<char *>(rb.funcs));

		st_resolve.cases++;
		if (port_off != ref_off || !guards_intact(pb) ||
		    !guards_intact(rb) || !blocks_match(pb, rb))
			st_resolve.fails++;
	}
}

void
trap_case()
{
	reset_both();
	mock_amd_v3();
	set_both_environ(nullptr);

	pid_t pid = fork();
	if (pid < 0) {
		std::perror("fork");
		std::exit(2);
	}
	if (pid == 0) {
		FuncBlock pb;
		std::memset(pb.funcs, 0, sizeof(pb.funcs));
		port::__archlevel_resolve(FEAT_EDX_BASELINE, FEAT_ECX_V3,
		    EXT_EBX_V3, 0, pb.funcs);
		_exit(99);
	}

	int status = 0;

	waitpid(pid, &status, 0);
	st_resolve.cases++;
	if (!WIFSIGNALED(status))
		st_resolve.fails++;

	pid = fork();
	if (pid < 0) {
		std::perror("fork");
		std::exit(2);
	}
	if (pid == 0) {
		FuncBlock rb;
		std::memset(rb.funcs, 0, sizeof(rb.funcs));
		ref___archlevel_resolve(FEAT_EDX_BASELINE, FEAT_ECX_V3,
		    EXT_EBX_V3, 0, rb.funcs);
		_exit(99);
	}

	status = 0;
	waitpid(pid, &status, 0);
	st_resolve.cases++;
	if (!WIFSIGNALED(status))
		st_resolve.fails++;
}

} /* namespace */

int
main()
{
	rng_state = 0xc0ffee1234567890ULL;

	edge_cases();
	trap_case();
	random_sweep();

	std::printf("PBSD batch b0142 differential test\n");
	std::printf("  %-24s %8ld cases  %8ld failures\n",
	    st_resolve.name, st_resolve.cases, st_resolve.fails);
	std::printf("----------------------------------------\n");
	std::printf("  %-24s %8ld cases  %8ld failures\n",
	    "TOTAL", st_resolve.cases, st_resolve.fails);

	return (st_resolve.fails == 0 ? 0 : 1);
}
