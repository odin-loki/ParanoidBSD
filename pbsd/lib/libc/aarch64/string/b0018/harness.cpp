/*
 * Differential test harness for PBSD batch b0018.
 *
 * Compares memset_resolver in pbsd.lib.libc.aarch64.string.b0018 against
 * ref_memset_resolver in oracle.c.
 */

import pbsd.lib.libc.aarch64.string.b0018;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

extern "C" {
struct __ifunc_arg_t {
	unsigned long _size;
	unsigned long _hwcap;
	unsigned long _hwcap2;
	unsigned long _hwcap3;
	unsigned long _hwcap4;
};

extern uint64_t test_dczid_el0;
void *ref_memset_resolver(uint64_t, const struct __ifunc_arg_t *, uint64_t,
    uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
void *__memset_aarch64(void *, int, size_t);
void *__memset_aarch64_zva64(void *, int, size_t);
void *__memset_aarch64_mops(void *, int, size_t);
}

namespace port = pbsd::lib_libc_aarch64_string::b0018;

namespace {

constexpr std::uint64_t HWCAP2_MOPS = 1ULL << 43;
constexpr std::uint64_t DCZID_DZP = 1ULL << 4;
constexpr std::uint64_t DCZID_BS_MASK = 0xfULL;

constexpr std::size_t RANDOM_ITERS = 200000;

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
};

Stat st_memset_resolver{"memset_resolver", 0, 0};

unsigned int prng_state = 0xB0018u;

std::uint32_t prng_next()
{
	prng_state ^= prng_state << 13;
	prng_state ^= prng_state >> 17;
	prng_state ^= prng_state << 5;
	return (prng_state);
}

const char *
variant_name(void *fn)
{
	if (fn == reinterpret_cast<void *>(__memset_aarch64_mops))
		return ("mops");
	if (fn == reinterpret_cast<void *>(__memset_aarch64_zva64))
		return ("zva64");
	if (fn == reinterpret_cast<void *>(__memset_aarch64))
		return ("aarch64");
	return ("unknown");
}

port::__ifunc_arg_t make_arg(std::uint64_t hwcap2)
{
	port::__ifunc_arg_t arg{};

	arg._size = sizeof(arg);
	arg._hwcap = 0;
	arg._hwcap2 = hwcap2;
	arg._hwcap3 = 0;
	arg._hwcap4 = 0;
	return (arg);
}

void *
call_ref(std::uint64_t at_hwcap, const port::__ifunc_arg_t *arg,
    std::uint64_t a3, std::uint64_t a4, std::uint64_t a5, std::uint64_t a6,
    std::uint64_t a7, std::uint64_t a8)
{
	return (ref_memset_resolver(at_hwcap,
	    reinterpret_cast<const struct __ifunc_arg_t *>(arg), a3, a4, a5, a6,
	    a7, a8));
}

void *
call_port(std::uint64_t at_hwcap, const port::__ifunc_arg_t *arg,
    std::uint64_t a3, std::uint64_t a4, std::uint64_t a5, std::uint64_t a6,
    std::uint64_t a7, std::uint64_t a8)
{
	return (port::memset_resolver(at_hwcap, arg, a3, a4, a5, a6, a7, a8));
}

int shown = 0;

void fail(const char *tag, std::uint64_t hwcap2, std::uint64_t dczid, void *rp,
    void *ro)
{
	if (shown < 20) {
		shown++;
		std::printf("  FAIL [%s] hwcap2=%#llx dczid=%#llx port=%s (%p)"
		    " oracle=%s (%p)\n",
		    tag, static_cast<unsigned long long>(hwcap2),
		    static_cast<unsigned long long>(dczid), variant_name(rp),
		    rp, variant_name(ro), ro);
	}
}

void check_case(Stat &s, const char *tag, std::uint64_t hwcap2,
    std::uint64_t dczid, std::uint64_t at_hwcap, std::uint64_t a3,
    std::uint64_t a4, std::uint64_t a5, std::uint64_t a6, std::uint64_t a7,
    std::uint64_t a8)
{
	port::__ifunc_arg_t arg = make_arg(hwcap2);

	s.cases++;
	test_dczid_el0 = dczid;

	void *ro = call_ref(at_hwcap, &arg, a3, a4, a5, a6, a7, a8);
	void *rp = call_port(at_hwcap, &arg, a3, a4, a5, a6, a7, a8);

	if (rp != ro) {
		s.fails++;
		fail(tag, hwcap2, dczid, rp, ro);
	}
}

void run_edges()
{
	/* MOPS branch: bit set regardless of dczid */
	check_case(st_memset_resolver, "mops_bit", HWCAP2_MOPS, 0, 0, 0, 0, 0,
	    0, 0, 0);
	check_case(st_memset_resolver, "mops_only", HWCAP2_MOPS, 4, 0, 0, 0, 0,
	    0, 0, 0);
	check_case(st_memset_resolver, "mops_dzp", HWCAP2_MOPS, DCZID_DZP | 4,
	    0, 0, 0, 0, 0, 0, 0);
	check_case(st_memset_resolver, "mops_all", HWCAP2_MOPS,
	    0xFFFFFFFFFFFFFFFFULL, 0, 0, 0, 0, 0, 0, 0);

	/* ZVA64 branch: no MOPS, DZP clear, BS == 4 */
	check_case(st_memset_resolver, "zva_exact", 0, 4, 0, 0, 0, 0, 0, 0,
	    0);
	check_case(st_memset_resolver, "zva_high", 0, 4 | 0xF0, 0, 0, 0, 0, 0,
	    0, 0);
	check_case(st_memset_resolver, "zva_noise", 0x12345678ULL, 4, 0, 0, 0,
	    0, 0, 0, 0);

	/* Default aarch64: DZP set */
	check_case(st_memset_resolver, "dzp_only", 0, DCZID_DZP, 0, 0, 0, 0, 0,
	    0, 0);
	check_case(st_memset_resolver, "dzp_bs4", 0, DCZID_DZP | 4, 0, 0, 0, 0,
	    0, 0, 0);

	/* Default aarch64: BS != 4 */
	check_case(st_memset_resolver, "bs0", 0, 0, 0, 0, 0, 0, 0, 0, 0);
	check_case(st_memset_resolver, "bs1", 0, 1, 0, 0, 0, 0, 0, 0, 0);
	check_case(st_memset_resolver, "bs3", 0, 3, 0, 0, 0, 0, 0, 0, 0);
	check_case(st_memset_resolver, "bs5", 0, 5, 0, 0, 0, 0, 0, 0, 0);
	check_case(st_memset_resolver, "bs15", 0, 15, 0, 0, 0, 0, 0, 0, 0);
	check_case(st_memset_resolver, "bs_high", 0, 0xF, 0, 0, 0, 0, 0, 0, 0);

	/* MOPS boundary: bit clear vs set */
	check_case(st_memset_resolver, "pre_mops", 0, 4, 0, 0, 0, 0, 0, 0, 0);
	check_case(st_memset_resolver, "mops_minus1", HWCAP2_MOPS - 1, 4, 0, 0,
	    0, 0, 0, 0, 0);
	check_case(st_memset_resolver, "mops_plus1", HWCAP2_MOPS + 1, 4, 0, 0,
	    0, 0, 0, 0, 0);

	/* && boundary: only one side of DCZID test true */
	check_case(st_memset_resolver, "dzp_clear_bs3", 0, 3, 0, 0, 0, 0, 0, 0,
	    0);
	check_case(st_memset_resolver, "dzp_set_bs4", 0, DCZID_DZP | 4, 0, 0, 0,
	    0, 0, 0, 0);

	/* NUL-heavy / high-bit hwcap2 bytes without MOPS */
	check_case(st_memset_resolver, "hw80", 0x80, 4, 0, 0, 0, 0, 0, 0, 0);
	check_case(st_memset_resolver, "hwff", 0xFF, 4, 0, 0, 0, 0, 0, 0, 0);
	check_case(st_memset_resolver, "hw8080", 0x80808080ULL, 4, 0, 0, 0, 0,
	    0, 0, 0);
	check_case(st_memset_resolver, "dcz80", 0, 0x80, 0, 0, 0, 0, 0, 0, 0);
	check_case(st_memset_resolver, "dczff", 0, 0xFF, 0, 0, 0, 0, 0, 0, 0);

	/* Unused resolver arguments should not affect selection */
	check_case(st_memset_resolver, "args_noise", 0, 4, 0xDEADBEEFULL,
	    0xCAFEBABEULL, 0x12345678ULL, 0x9ABCDEF0ULL, 0x11111111ULL,
	    0x22222222ULL, 0x33333333ULL);
}

void run_random()
{
	for (std::size_t i = 0; i < RANDOM_ITERS; i++) {
		std::uint64_t hwcap2 = static_cast<std::uint64_t>(prng_next());
		hwcap2 |= (static_cast<std::uint64_t>(prng_next()) << 32);

		std::uint64_t dczid = static_cast<std::uint64_t>(prng_next());
		dczid |= (static_cast<std::uint64_t>(prng_next()) << 32);

		std::uint64_t at_hwcap = static_cast<std::uint64_t>(prng_next());
		at_hwcap |= (static_cast<std::uint64_t>(prng_next()) << 32);
		std::uint64_t a3 = static_cast<std::uint64_t>(prng_next());
		a3 |= (static_cast<std::uint64_t>(prng_next()) << 32);
		std::uint64_t a4 = static_cast<std::uint64_t>(prng_next());
		a4 |= (static_cast<std::uint64_t>(prng_next()) << 32);
		std::uint64_t a5 = static_cast<std::uint64_t>(prng_next());
		a5 |= (static_cast<std::uint64_t>(prng_next()) << 32);
		std::uint64_t a6 = static_cast<std::uint64_t>(prng_next());
		a6 |= (static_cast<std::uint64_t>(prng_next()) << 32);
		std::uint64_t a7 = static_cast<std::uint64_t>(prng_next());
		a7 |= (static_cast<std::uint64_t>(prng_next()) << 32);
		std::uint64_t a8 = static_cast<std::uint64_t>(prng_next());
		a8 |= (static_cast<std::uint64_t>(prng_next()) << 32);

		check_case(st_memset_resolver, "random", hwcap2, dczid, at_hwcap,
		    a3, a4, a5, a6, a7, a8);
	}
}

void print_table()
{
	const Stat *all[] = {&st_memset_resolver};

	std::printf("\n%-22s %12s %12s %s\n", "function", "cases", "failures",
	    "status");
	std::printf("%-22s %12s %12s %s\n", "----------------------",
	    "------------", "------------", "--------");

	int any_fail = 0;
	for (const Stat *s : all) {
		const char *status = (s->fails == 0) ? "PASS" : "FAIL";
		if (s->fails != 0)
			any_fail = 1;
		std::printf("%-22s %12llu %12llu %s\n", s->name, s->cases,
		    s->fails, status);
	}

	if (any_fail)
		std::printf("\nOVERALL: FAIL\n");
	else
		std::printf("\nOVERALL: PASS\n");
}

} /* namespace */

int main()
{
	run_edges();
	run_random();
	print_table();
	return (st_memset_resolver.fails == 0) ? 0 : 1;
}
