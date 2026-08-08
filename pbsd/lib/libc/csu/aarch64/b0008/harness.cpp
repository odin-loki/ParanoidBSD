/*
 * harness.cpp -- differential test for PBSD batch b0008.
 *
 * Every ported entry point is driven against the ref_ oracle in oracle.c
 * with hand-written edge cases and a fixed-seed randomised sweep.  For
 * ifunc_init the global ifunc_arg state is compared field-for-field.
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

import pbsd.lib.libc.csu.aarch64.b0008;

namespace port = pbsd::lib_libc_csu_aarch64::b0008;

extern "C" {
typedef struct __ifunc_arg {
	unsigned long _size;
	unsigned long _hwcap;
	unsigned long _hwcap2;
	unsigned long _hwcap3;
	unsigned long _hwcap4;
} __ifunc_arg_t;

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

extern __ifunc_arg_t ifunc_arg;

void ref_ifunc_init(const Elf_Auxinfo *);
void ref_crt1_handle_rela(const Elf_Rela *);
}

namespace {

constexpr int MAX_REPORT = 10;
constexpr long RANDOM_ITERATIONS = 200000;

constexpr std::uint64_t AT_NULL = 0;
constexpr std::uint64_t AT_HWCAP = 16;
constexpr std::uint64_t AT_HWCAP2 = 26;
constexpr std::uint64_t AT_HWCAP3 = 35;
constexpr std::uint64_t AT_HWCAP4 = 38;
constexpr unsigned long IFUNC_ARG_HWCAP = (1ULL << 62);
constexpr std::uint64_t R_AARCH64_IRELATIVE = 1032;

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

	template <typename T>
	T load(std::size_t off) const
	{
		T v;
		std::memcpy(&v, b + off, sizeof(T));
		return v;
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

std::uint64_t prng_u64()
{
	return prng_next();
}

std::uint64_t prng_byte_val()
{
	std::uint64_t r = prng_next();
	switch (r & 7) {
	case 0:
		return 0;
	case 1:
		return 1;
	case 2:
		return 0xFFFFFFFFFFFFFFFFULL;
	case 3:
		return 0x80ULL | (r & 0x7F);
	case 4:
		return 0xFFULL;
	case 5:
		return (r & 0xFF) * 0x0101010101010101ULL;
	case 6:
		return r;
	default:
		return r ^ (r << 17) ^ (r >> 11);
	}
}

bool want_report(Stat &s) { return s.reported++ < MAX_REPORT; }

void note_fail(Stat &s) { s.fails++; }

bool arg_same(const port::__ifunc_arg_t &a, const port::__ifunc_arg_t &b)
{
	return a._size == b._size && a._hwcap == b._hwcap &&
	    a._hwcap2 == b._hwcap2 && a._hwcap3 == b._hwcap3 &&
	    a._hwcap4 == b._hwcap4;
}

void zero_ref_arg()
{
	std::memset(&ifunc_arg, 0, sizeof(ifunc_arg));
}

void zero_port_arg() { std::memset(&port::ifunc_arg, 0, sizeof(port::ifunc_arg)); }

/* ------------------------------------------------------------------ */
/* mock ifunc resolvers for crt1_handle_rela                          */
/* ------------------------------------------------------------------ */

std::uint64_t resolver_xor_hwcap2(std::uint64_t hwcap, const __ifunc_arg_t *arg,
    std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t,
    std::uint64_t)
{
	return hwcap ^ static_cast<std::uint64_t>(arg->_hwcap2) ^
	    0xA5A5A5A5A5A5A5A5ULL;
}

std::uint64_t resolver_sum_fields(std::uint64_t hwcap, const __ifunc_arg_t *arg,
    std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t,
    std::uint64_t)
{
	return static_cast<std::uint64_t>(arg->_size) +
	    static_cast<std::uint64_t>(arg->_hwcap) +
	    static_cast<std::uint64_t>(arg->_hwcap2) +
	    static_cast<std::uint64_t>(arg->_hwcap3) +
	    static_cast<std::uint64_t>(arg->_hwcap4) + hwcap;
}

std::uint64_t resolver_return_zero(std::uint64_t, const __ifunc_arg_t *,
    std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t,
    std::uint64_t)
{
	return 0;
}

std::uint64_t resolver_return_max(std::uint64_t, const __ifunc_arg_t *,
    std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t,
    std::uint64_t)
{
	return 0xFFFFFFFFFFFFFFFFULL;
}

std::uint64_t resolver_identity_hwcap(std::uint64_t hwcap, const __ifunc_arg_t *,
    std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t,
    std::uint64_t)
{
	return hwcap;
}

using resolver_fn = std::uint64_t (*)(std::uint64_t, const __ifunc_arg_t *,
    std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t,
    std::uint64_t);

resolver_fn resolver_table[] = {
    resolver_xor_hwcap2,
    resolver_sum_fields,
    resolver_return_zero,
    resolver_return_max,
    resolver_identity_hwcap,
};

std::uint64_t ELF_R_TYPE(std::uint64_t info)
{
	return info & 0xffffffffULL;
}

/* ------------------------------------------------------------------ */
/* ifunc_init                                                         */
/* ------------------------------------------------------------------ */

void check_ifunc_init(const std::vector<Elf_Auxinfo> &spec, bool guarded)
{
	Stat &s = st_ifunc_init;
	s.cases++;

	std::vector<Elf_Auxinfo> aux_ref = spec;
	std::vector<Elf_Auxinfo> aux_port = spec;

	GuardBuf ga, gb;
	if (guarded) {
		ga.fill();
		gb.fill();
		std::size_t bytes = aux_ref.size() * sizeof(Elf_Auxinfo);
		if (bytes > GUARD_SIZE - 8)
			return;
		std::memcpy(ga.b, aux_ref.data(), bytes);
		std::memcpy(gb.b, aux_port.data(), bytes);
	}

	zero_ref_arg();
	zero_port_arg();

	const Elf_Auxinfo *pref = aux_ref.data();
	const Elf_Auxinfo *pport = aux_port.data();
	if (guarded) {
		pref = reinterpret_cast<const Elf_Auxinfo *>(ga.b);
		pport = reinterpret_cast<const Elf_Auxinfo *>(gb.b);
	}

	ref_ifunc_init(pref);
	port::ifunc_init(reinterpret_cast<const port::Elf_Auxinfo *>(pport));

	port::__ifunc_arg_t ref_copy, port_copy;
	std::memcpy(&ref_copy, &ifunc_arg, sizeof(ref_copy));
	port_copy = port::ifunc_arg;

	bool bad = !arg_same(ref_copy, port_copy);
	if (guarded) {
		std::size_t bytes = spec.size() * sizeof(Elf_Auxinfo);
		if (std::memcmp(ga.b, gb.b, bytes) != 0)
			bad = true;
	}

	if (bad) {
		note_fail(s);
		if (want_report(s))
			std::printf("  FAIL ifunc_init: ref={sz=%lu,hw=%#lx,h2=%#lx,"
				    "h3=%#lx,h4=%#lx} port={sz=%lu,hw=%#lx,"
				    "h2=%#lx,h3=%#lx,h4=%#lx}\n",
			    ref_copy._size, ref_copy._hwcap, ref_copy._hwcap2,
			    ref_copy._hwcap3, ref_copy._hwcap4, port_copy._size,
			    port_copy._hwcap, port_copy._hwcap2,
			    port_copy._hwcap3, port_copy._hwcap4);
	}
}

std::vector<Elf_Auxinfo> make_aux(std::initializer_list<std::pair<std::uint64_t,
				      std::uint64_t>> entries)
{
	std::vector<Elf_Auxinfo> v;
	for (auto [t, val] : entries) {
		Elf_Auxinfo a{};
		a.a_type = t;
		a.a_un.a_val = val;
		v.push_back(a);
	}
	Elf_Auxinfo term{};
	term.a_type = AT_NULL;
	v.push_back(term);
	return v;
}

void run_ifunc_init_edges()
{
	check_ifunc_init(make_aux({}), true);
	check_ifunc_init(make_aux({{AT_NULL, 0}}), true);

	check_ifunc_init(make_aux({{AT_HWCAP, 0}}), true);
	check_ifunc_init(make_aux({{AT_HWCAP, 1}}), true);
	check_ifunc_init(make_aux({{AT_HWCAP, 0xFFFFFFFFFFFFFFFFULL}}), true);
	check_ifunc_init(make_aux({{AT_HWCAP, 0x80}}), true);
	check_ifunc_init(make_aux({{AT_HWCAP, 0xFF}}), true);
	check_ifunc_init(make_aux({{AT_HWCAP, 0x7F}}), true);
	check_ifunc_init(make_aux({{AT_HWCAP, 0x1234567890ABCDEFULL}}), true);

	check_ifunc_init(make_aux({{AT_HWCAP2, 0}}), true);
	check_ifunc_init(make_aux({{AT_HWCAP2, 0xDEADBEEFCAFEBABEULL}}), true);
	check_ifunc_init(make_aux({{AT_HWCAP3, 0x8080808080808080ULL}}), true);
	check_ifunc_init(make_aux({{AT_HWCAP4, 0xFFFFFFFFFFFFFFFFULL}}), true);

	check_ifunc_init(make_aux({
	    {AT_HWCAP, 0x42},
	    {AT_HWCAP2, 0x84},
	    {AT_HWCAP3, 0xFF},
	    {AT_HWCAP4, 0x01},
	}),
	    true);

	check_ifunc_init(make_aux({
	    {AT_HWCAP, 1},
	    {AT_HWCAP, 2},
	    {AT_HWCAP, 3},
	}),
	    true);

	check_ifunc_init(make_aux({
	    {1, 0x1111},
	    {2, 0x2222},
	    {3, 0x3333},
	    {AT_HWCAP, 0x55},
	    {99, 0x9999},
	}),
	    true);

	for (unsigned b = 0x80; b <= 0xFF; b++) {
		check_ifunc_init(make_aux({{AT_HWCAP, b}}), true);
		check_ifunc_init(make_aux({{AT_HWCAP2, b * 0x0101010101010101ULL}}),
		    true);
	}

	for (int i = 0; i < 64; i++) {
		check_ifunc_init(make_aux({{AT_HWCAP, 1ULL << i}}), true);
		check_ifunc_init(make_aux({{AT_HWCAP2, 1ULL << i}}), true);
	}
}

void run_ifunc_init_random()
{
	prng_seed(0x0008000100000001ULL);
	for (long n = 0; n < RANDOM_ITERATIONS; n++) {
		std::vector<Elf_Auxinfo> v;
		unsigned count = static_cast<unsigned>(prng_next() % 16);
		for (unsigned i = 0; i < count; i++) {
			Elf_Auxinfo a{};
			std::uint64_t pick = prng_next() % 10;
			switch (pick) {
			case 0:
				a.a_type = AT_HWCAP;
				break;
			case 1:
				a.a_type = AT_HWCAP2;
				break;
			case 2:
				a.a_type = AT_HWCAP3;
				break;
			case 3:
				a.a_type = AT_HWCAP4;
				break;
			default:
				a.a_type = prng_next() & 0xFF;
				break;
			}
			a.a_un.a_val = prng_byte_val();
			v.push_back(a);
		}
		Elf_Auxinfo term{};
		term.a_type = AT_NULL;
		v.push_back(term);
		check_ifunc_init(v, (n & 0xFF) == 0);
	}
}

/* ------------------------------------------------------------------ */
/* crt1_handle_rela                                                   */
/* ------------------------------------------------------------------ */

void seed_both_args(const std::vector<Elf_Auxinfo> &spec)
{
	zero_ref_arg();
	zero_port_arg();
	ref_ifunc_init(spec.data());
	port::ifunc_init(
	    reinterpret_cast<const port::Elf_Auxinfo *>(spec.data()));
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

	if (ELF_R_TYPE(spec.r_info) == R_AARCH64_IRELATIVE) {
		r_ref.r_offset = reinterpret_cast<std::uint64_t>(
		    ba.b + SLOT_OFF);
		r_port.r_offset = reinterpret_cast<std::uint64_t>(
		    bb.b + SLOT_OFF);
	} else {
		r_ref.r_offset = reinterpret_cast<std::uint64_t>(
		    ba.b + SLOT_OFF);
		r_port.r_offset = reinterpret_cast<std::uint64_t>(
		    bb.b + SLOT_OFF);
	}

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
}

void run_crt1_handle_rela_edges()
{
	std::vector<Elf_Auxinfo> seed = make_aux({
	    {AT_HWCAP, 0x42424242},
	    {AT_HWCAP2, 0x84848484},
	    {AT_HWCAP3, 0xFF00FF00},
	    {AT_HWCAP4, 0x01020304},
	});
	seed_both_args(seed);

	Elf_Rela r{};
	r.r_info = 0;
	r.r_addend = 0;
	check_crt1_handle_rela(r, true);

	r.r_info = 1;
	check_crt1_handle_rela(r, true);

	r.r_info = R_AARCH64_IRELATIVE - 1;
	check_crt1_handle_rela(r, true);

	r.r_info = R_AARCH64_IRELATIVE + 1;
	check_crt1_handle_rela(r, true);

	r.r_info = R_AARCH64_IRELATIVE | (0xDEADBEEFULL << 32);
	r.r_addend = static_cast<long>(
	    reinterpret_cast<std::uintptr_t>(resolver_return_zero));
	check_crt1_handle_rela(r, true);

	for (resolver_fn fn : resolver_table) {
		seed_both_args(seed);
		r.r_info = R_AARCH64_IRELATIVE;
		r.r_addend = static_cast<long>(reinterpret_cast<std::uintptr_t>(fn));
		check_crt1_handle_rela(r, true);
	}

	seed_both_args(make_aux({{AT_HWCAP, 0}}));
	r.r_info = R_AARCH64_IRELATIVE;
	r.r_addend = static_cast<long>(
	    reinterpret_cast<std::uintptr_t>(resolver_identity_hwcap));
	check_crt1_handle_rela(r, true);

	seed_both_args(make_aux({{AT_HWCAP, 0xFFFFFFFFFFFFFFFFULL}}));
	check_crt1_handle_rela(r, true);

	seed_both_args(make_aux({{AT_HWCAP2, 0x8080808080808080ULL}}));
	r.r_addend = static_cast<long>(
	    reinterpret_cast<std::uintptr_t>(resolver_xor_hwcap2));
	check_crt1_handle_rela(r, true);
}

void run_crt1_handle_rela_random()
{
	prng_seed(0x0008000200000002ULL);
	for (long n = 0; n < RANDOM_ITERATIONS; n++) {
		std::vector<Elf_Auxinfo> aux;
		unsigned ac = static_cast<unsigned>(prng_next() % 8);
		for (unsigned i = 0; i < ac; i++) {
			Elf_Auxinfo a{};
			static const std::uint64_t at_types[] = {
			    AT_HWCAP, AT_HWCAP2, AT_HWCAP3, AT_HWCAP4};
			if (prng_next() % 3 == 0)
				a.a_type = at_types[prng_next() % 4];
			else
				a.a_type = prng_next() & 0xFF;
			a.a_un.a_val = prng_byte_val();
			aux.push_back(a);
		}
		Elf_Auxinfo term{};
		term.a_type = AT_NULL;
		aux.push_back(term);
		seed_both_args(aux);

		Elf_Rela r{};
		if (prng_next() & 1) {
			r.r_info = R_AARCH64_IRELATIVE |
			    ((prng_next() & 0xFFFF) << 32);
			r.r_addend = static_cast<long>(reinterpret_cast<std::uintptr_t>(
			    resolver_table[prng_next() %
				(sizeof(resolver_table) /
				    sizeof(resolver_table[0]))]));
		} else {
			r.r_info = prng_next();
			if (ELF_R_TYPE(r.r_info) == R_AARCH64_IRELATIVE)
				r.r_info ^= 1;
			r.r_addend = static_cast<long>(prng_next());
		}
		check_crt1_handle_rela(r, (n & 0xFF) == 0);
	}
}

} /* namespace */

int main()
{
	std::printf("PBSD batch b0008 differential test\n");
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
