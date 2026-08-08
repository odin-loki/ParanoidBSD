/*
 * harness.cpp -- differential test for PBSD batch b0134.
 *
 * Every ported entry point is driven against the ref_ oracle in oracle.c
 * with hand-written edge cases and a fixed-seed randomised sweep.  For
 * ifunc_init the global cpu feature state is compared field-for-field.
 * For crt1_handle_rela the relocation write slot is embedded in a pair of
 * guard buffers pre-filled with 0x7f; the entire buffer is compared after
 * each pair of calls.
 */

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

import pbsd.lib.libc.csu.amd64.b0134;

namespace port = pbsd::lib_libc_csu_amd64::b0134;

extern "C" {
typedef unsigned int u_int;

typedef struct {
	std::uint64_t a_type;
	union {
		std::uint64_t a_val;
	} a_un;
} Elf_Auxinfo;

typedef struct {
	std::uint64_t r_offset;
	std::uint64_t r_info;
	long r_addend;
} Elf_Rela;

extern std::uint32_t cpu_feature, cpu_feature2;
extern std::uint32_t cpu_stdext_feature, cpu_stdext_feature2;

void ref_reset_cpuid_mocks(void);
void ref_add_cpuid_mock(u_int level, u_int leaf, u_int p0, u_int p1, u_int p2,
    u_int p3);
void ref_ifunc_init(const Elf_Auxinfo *);
void ref_crt1_handle_rela(const Elf_Rela *);
}

namespace {

constexpr int MAX_REPORT = 10;
constexpr long RANDOM_ITERATIONS = 200000;

constexpr std::uint64_t R_X86_64_IRELATIVE = 37;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

Stat st_ifunc_init{"ifunc_init", 0, 0, 0};
Stat st_crt1_handle_rela{"crt1_handle_rela", 0, 0, 0};

constexpr std::size_t GUARD_SIZE = 128;
constexpr std::size_t SLOT_OFF = 48;

struct GuardBuf {
	unsigned char b[GUARD_SIZE];

	void fill() { std::memset(b, 0x7f, GUARD_SIZE); }

	template <typename T>
	void store(std::size_t off, const T &v)
	{
		std::memcpy(b + off, &v, sizeof(T));
	}
};

std::uint64_t prng_state;

void prng_seed(std::uint64_t seed) { prng_state = seed; }

std::uint64_t prng_next()
{
	std::uint64_t z = (prng_state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

std::uint32_t prng_u32()
{
	return static_cast<std::uint32_t>(prng_next());
}

bool want_report(Stat &s) { return s.reported++ < MAX_REPORT; }

void note_fail(Stat &s) { s.fails++; }

void zero_ref_cpu()
{
	cpu_feature = 0;
	cpu_feature2 = 0;
	cpu_stdext_feature = 0;
	cpu_stdext_feature2 = 0;
}

void zero_port_cpu()
{
	port::cpu_feature = 0;
	port::cpu_feature2 = 0;
	port::cpu_stdext_feature = 0;
	port::cpu_stdext_feature2 = 0;
}

bool cpu_same()
{
	return cpu_feature == port::cpu_feature &&
	    cpu_feature2 == port::cpu_feature2 &&
	    cpu_stdext_feature == port::cpu_stdext_feature &&
	    cpu_stdext_feature2 == port::cpu_stdext_feature2;
}

void reset_both_mocks()
{
	ref_reset_cpuid_mocks();
	port::reset_cpuid_mocks();
}

void add_both_mocks(u_int level, u_int leaf, u_int p0, u_int p1, u_int p2,
    u_int p3)
{
	ref_add_cpuid_mock(level, leaf, p0, p1, p2, p3);
	port::add_cpuid_mock(level, leaf, p0, p1, p2, p3);
}

/* ------------------------------------------------------------------ */
/* mock ifunc resolvers for crt1_handle_rela                          */
/* ------------------------------------------------------------------ */

std::uint64_t resolver_sum(std::uint32_t a, std::uint32_t b, std::uint32_t c,
    std::uint32_t d)
{
	return static_cast<std::uint64_t>(a) + b + c + d;
}

std::uint64_t resolver_xor(std::uint32_t a, std::uint32_t b, std::uint32_t c,
    std::uint32_t d)
{
	return static_cast<std::uint64_t>(a) ^ b ^ c ^ d ^
	    0xA5A5A5A5A5A5A5A5ULL;
}

std::uint64_t resolver_return_zero(std::uint32_t, std::uint32_t, std::uint32_t,
    std::uint32_t)
{
	return 0;
}

std::uint64_t resolver_return_max(std::uint32_t, std::uint32_t, std::uint32_t,
    std::uint32_t)
{
	return 0xFFFFFFFFFFFFFFFFULL;
}

std::uint64_t resolver_identity_feat(std::uint32_t a, std::uint32_t, std::uint32_t,
    std::uint32_t)
{
	return static_cast<std::uint64_t>(a);
}

std::uint64_t resolver_identity_feat2(std::uint32_t, std::uint32_t b,
    std::uint32_t, std::uint32_t)
{
	return static_cast<std::uint64_t>(b);
}

std::uint64_t resolver_identity_stdext(std::uint32_t, std::uint32_t,
    std::uint32_t c, std::uint32_t)
{
	return static_cast<std::uint64_t>(c);
}

std::uint64_t resolver_identity_stdext2(std::uint32_t, std::uint32_t,
    std::uint32_t, std::uint32_t d)
{
	return static_cast<std::uint64_t>(d);
}

using resolver_fn = std::uint64_t (*)(std::uint32_t, std::uint32_t,
    std::uint32_t, std::uint32_t);

resolver_fn resolver_table[] = {
    resolver_sum,
    resolver_xor,
    resolver_return_zero,
    resolver_return_max,
    resolver_identity_feat,
    resolver_identity_feat2,
    resolver_identity_stdext,
    resolver_identity_stdext2,
};

std::uint64_t ELF_R_TYPE(std::uint64_t info)
{
	return info & 0xffffffffULL;
}

/* ------------------------------------------------------------------ */
/* ifunc_init                                                         */
/* ------------------------------------------------------------------ */

void check_ifunc_init(bool guarded)
{
	Stat &s = st_ifunc_init;
	s.cases++;

	zero_ref_cpu();
	zero_port_cpu();

	ref_ifunc_init(nullptr);
	port::ifunc_init(nullptr);

	bool bad = !cpu_same();
	if (bad) {
		note_fail(s);
		if (want_report(s))
			std::printf("  FAIL ifunc_init: ref={f=%#x,f2=%#x,se=%#x,"
				    "se2=%#x} port={f=%#x,f2=%#x,se=%#x,se2=%#x}\n",
			    cpu_feature, cpu_feature2, cpu_stdext_feature,
			    cpu_stdext_feature2, port::cpu_feature,
			    port::cpu_feature2, port::cpu_stdext_feature,
			    port::cpu_stdext_feature2);
	}
	(void)guarded;
}

void setup_cpuid_mock(u_int max_level, u_int feat_edx, u_int feat_ecx,
    u_int stdext_ebx, u_int stdext_ecx)
{
	reset_both_mocks();
	add_both_mocks(1, 0, 0, 0, feat_ecx, feat_edx);
	add_both_mocks(0, 0, max_level, 0, 0, 0);
	if (max_level >= 7)
		add_both_mocks(7, 0, 0, stdext_ebx, stdext_ecx, 0);
}

void run_ifunc_init_edges()
{
	/* else branch: max cpuid level < 7 */
	setup_cpuid_mock(6, 0x12345678, 0x9ABCDEF0, 0, 0);
	check_ifunc_init(true);

	setup_cpuid_mock(0, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU,
	    0xFFFFFFFFU);
	check_ifunc_init(true);

	setup_cpuid_mock(6, 0, 0, 0x80808080U, 0xFF00FF00U);
	check_ifunc_init(true);

	/* boundary: max level exactly 7 */
	setup_cpuid_mock(7, 0x00000001U, 0x00000002U, 0x00000003U,
	    0x00000004U);
	check_ifunc_init(true);

	/* true branch with high max level */
	setup_cpuid_mock(15, 0xDEADBEEFU, 0xCAFEBABEU, 0x42424242U,
	    0x84848484U);
	check_ifunc_init(true);

	setup_cpuid_mock(27, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU,
	    0xFFFFFFFFU);
	check_ifunc_init(true);

	setup_cpuid_mock(27, 0, 0, 0, 0);
	check_ifunc_init(true);

	/* high-bit bytes in cpuid results */
	for (unsigned b = 0x80; b <= 0xFF; b++) {
		setup_cpuid_mock(10, b, b | 0x80, b * 0x01010101U,
		    (b << 24) | (b << 16) | (b << 8) | b);
		check_ifunc_init(true);
	}

	/* boundary max_level 6 vs 7 vs 8 */
	setup_cpuid_mock(5, 0x55, 0x66, 0x77, 0x88);
	check_ifunc_init(true);
	setup_cpuid_mock(6, 0x55, 0x66, 0x77, 0x88);
	check_ifunc_init(true);
	setup_cpuid_mock(7, 0x55, 0x66, 0x77, 0x88);
	check_ifunc_init(true);
	setup_cpuid_mock(8, 0x55, 0x66, 0x77, 0x88);
	check_ifunc_init(true);

	/* real cpuid without mocks */
	reset_both_mocks();
	check_ifunc_init(true);
}

void run_ifunc_init_random()
{
	prng_seed(0x0013400100000001ULL);
	for (long n = 0; n < RANDOM_ITERATIONS; n++) {
		u_int max_level = prng_u32() & 0x1F;
		u_int feat_edx = prng_u32();
		u_int feat_ecx = prng_u32();
		u_int stdext_ebx = prng_u32();
		u_int stdext_ecx = prng_u32();

		if ((n & 0xFF) == 0) {
			reset_both_mocks();
			check_ifunc_init(true);
			continue;
		}

		setup_cpuid_mock(max_level, feat_edx, feat_ecx, stdext_ebx,
		    stdext_ecx);
		check_ifunc_init((n & 0x1FF) == 0);
	}
}

/* ------------------------------------------------------------------ */
/* crt1_handle_rela                                                   */
/* ------------------------------------------------------------------ */

void seed_both_cpu(u_int max_level, u_int feat_edx, u_int feat_ecx,
    u_int stdext_ebx, u_int stdext_ecx)
{
	setup_cpuid_mock(max_level, feat_edx, feat_ecx, stdext_ebx, stdext_ecx);
	zero_ref_cpu();
	zero_port_cpu();
	ref_ifunc_init(nullptr);
	port::ifunc_init(nullptr);
}

void check_crt1_handle_rela(const Elf_Rela &spec, bool guarded)
{
	Stat &s = st_crt1_handle_rela;
	s.cases++;

	GuardBuf ba, bb;
	ba.fill();
	bb.fill();

	std::uint64_t slot_ref = 0x7F7F7F7F7F7F7F7FULL;
	std::uint64_t slot_port = 0x7F7F7F7F7F7F7F7FULL;
	ba.store(SLOT_OFF, slot_ref);
	bb.store(SLOT_OFF, slot_port);

	Elf_Rela r_ref = spec;
	Elf_Rela r_port = spec;

	r_ref.r_offset = reinterpret_cast<std::uint64_t>(ba.b + SLOT_OFF);
	r_port.r_offset = reinterpret_cast<std::uint64_t>(bb.b + SLOT_OFF);

	ref_crt1_handle_rela(&r_ref);
	port::crt1_handle_rela(reinterpret_cast<const port::Elf_Rela *>(&r_port));

	bool bad = std::memcmp(ba.b, bb.b, GUARD_SIZE) != 0;
	if (bad) {
		note_fail(s);
		if (want_report(s))
			std::printf("  FAIL crt1_handle_rela(r_info=%#" PRIx64
				    ", addend=%#" PRIx64 "): guard buffer diff\n",
			    spec.r_info, static_cast<std::uint64_t>(spec.r_addend));
	}
	(void)guarded;
}

void run_crt1_handle_rela_edges()
{
	seed_both_cpu(10, 0x11111111U, 0x22222222U, 0x33333333U, 0x44444444U);

	Elf_Rela r{};
	r.r_info = 0;
	r.r_addend = 0;
	check_crt1_handle_rela(r, true);

	r.r_info = 1;
	check_crt1_handle_rela(r, true);

	r.r_info = R_X86_64_IRELATIVE - 1;
	check_crt1_handle_rela(r, true);

	r.r_info = R_X86_64_IRELATIVE + 1;
	check_crt1_handle_rela(r, true);

	r.r_info = R_X86_64_IRELATIVE | (0xDEADBEEFULL << 32);
	r.r_addend = static_cast<long>(
	    reinterpret_cast<std::uintptr_t>(resolver_return_zero));
	check_crt1_handle_rela(r, true);

	for (resolver_fn fn : resolver_table) {
		seed_both_cpu(10, 0x01020304U, 0x05060708U, 0x090A0B0CU,
		    0x0D0E0F10U);
		r.r_info = R_X86_64_IRELATIVE;
		r.r_addend = static_cast<long>(reinterpret_cast<std::uintptr_t>(fn));
		check_crt1_handle_rela(r, true);
	}

	seed_both_cpu(6, 0xAAAAAAAAU, 0x55555555U, 0xFFFFFFFFU, 0xFFFFFFFFU);
	r.r_info = R_X86_64_IRELATIVE;
	r.r_addend = static_cast<long>(
	    reinterpret_cast<std::uintptr_t>(resolver_sum));
	check_crt1_handle_rela(r, true);

	seed_both_cpu(7, 0, 0, 0, 0);
	r.r_addend = static_cast<long>(
	    reinterpret_cast<std::uintptr_t>(resolver_identity_feat));
	check_crt1_handle_rela(r, true);

	seed_both_cpu(15, 0xFFFFFFFFU, 0xFFFFFFFFU, 0x80808080U, 0x7F7F7F7FU);
	r.r_addend = static_cast<long>(
	    reinterpret_cast<std::uintptr_t>(resolver_xor));
	check_crt1_handle_rela(r, true);

	for (unsigned b = 0x80; b <= 0xFF; b++) {
		seed_both_cpu(10, b, b | 0x80, b * 0x01010101U,
		    (b << 24) | (b << 16) | (b << 8) | b);
		r.r_info = R_X86_64_IRELATIVE;
		r.r_addend = static_cast<long>(
		    reinterpret_cast<std::uintptr_t>(resolver_sum));
		check_crt1_handle_rela(r, true);
	}
}

void run_crt1_handle_rela_random()
{
	prng_seed(0x0013400200000002ULL);
	for (long n = 0; n < RANDOM_ITERATIONS; n++) {
		u_int max_level = prng_u32() & 0x1F;
		u_int feat_edx = prng_u32();
		u_int feat_ecx = prng_u32();
		u_int stdext_ebx = prng_u32();
		u_int stdext_ecx = prng_u32();
		seed_both_cpu(max_level, feat_edx, feat_ecx, stdext_ebx,
		    stdext_ecx);

		Elf_Rela r{};
		if (prng_next() & 1) {
			r.r_info = R_X86_64_IRELATIVE |
			    ((prng_next() & 0xFFFF) << 32);
			r.r_addend = static_cast<long>(reinterpret_cast<std::uintptr_t>(
			    resolver_table[prng_next() %
				(sizeof(resolver_table) /
				    sizeof(resolver_table[0]))]));
		} else {
			r.r_info = prng_next();
			if (ELF_R_TYPE(r.r_info) == R_X86_64_IRELATIVE)
				r.r_info ^= 1;
			r.r_addend = static_cast<long>(prng_next());
		}
		check_crt1_handle_rela(r, (n & 0xFF) == 0);
	}
}

} /* namespace */

int main()
{
	std::printf("PBSD batch b0134 differential test\n");
	std::printf("(oracle.c is the specification; guard byte 0x7f)\n\n");

	run_ifunc_init_edges();
	run_ifunc_init_random();
	run_crt1_handle_rela_edges();
	run_crt1_handle_rela_random();

	const Stat *all[] = {&st_ifunc_init, &st_crt1_handle_rela};

	std::printf("\n%-20s %12s %12s %10s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-20s %12s %12s %10s\n", "--------------------",
	    "------------", "------------", "----------");

	long total_cases = 0;
	long total_fails = 0;
	for (const Stat *s : all) {
		total_cases += s->cases;
		total_fails += s->fails;
		std::printf("%-20s %12ld %12ld %10s\n", s->name, s->cases,
		    s->fails, s->fails == 0 ? "PASS" : "FAIL");
	}
	std::printf("%-20s %12s %12s %10s\n", "--------------------",
	    "------------", "------------", "----------");
	std::printf("%-20s %12ld %12ld %10s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0 ? "PASS" : "FAIL");

	return total_fails == 0 ? 0 : 1;
}
