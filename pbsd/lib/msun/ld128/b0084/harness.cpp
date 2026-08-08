/*
 * Differential test harness for PBSD batch b0084.
 *
 * lib/msun/ld128/invtrig.c defines no functions; it exports coefficient tables
 * for ld128 asinl()/acosl()/atanl().  Every scalar and table element in the
 * port is compared bit-for-bit against the ref_ oracle.
 *
 * lib/msun/ld128/s_expl.c and lib/msun/ld128/e_lgammal_r.c are omitted from
 * this batch (they depend on k_expl.h / ld128 kernel helpers not present here).
 */

#include <array>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <random>

import pbsd.lib.msun.ld128.b0084;

namespace port = pbsd::lib_msun_ld128::b0084;

extern "C" {
extern const long double ref_pS0, ref_pS1, ref_pS2, ref_pS3, ref_pS4;
extern const long double ref_pS5, ref_pS6, ref_pS7, ref_pS8, ref_pS9;
extern const long double ref_qS1, ref_qS2, ref_qS3, ref_qS4, ref_qS5;
extern const long double ref_qS6, ref_qS7, ref_qS8, ref_qS9;
extern const long double ref_atanhi[];
extern const long double ref_atanlo[];
extern const long double ref_aT[];
extern const long double ref_pi_lo;
extern const std::size_t ref_atanhi_n;
extern const std::size_t ref_atanlo_n;
extern const std::size_t ref_aT_n;
}

static const unsigned long long RANDOM_ITERS = 200000ull;
static const unsigned MAX_REPORT = 8;

struct stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	unsigned reported;
};

static stat st_pS0 = { "pS0", 0, 0, 0 };
static stat st_pS1 = { "pS1", 0, 0, 0 };
static stat st_pS2 = { "pS2", 0, 0, 0 };
static stat st_pS3 = { "pS3", 0, 0, 0 };
static stat st_pS4 = { "pS4", 0, 0, 0 };
static stat st_pS5 = { "pS5", 0, 0, 0 };
static stat st_pS6 = { "pS6", 0, 0, 0 };
static stat st_pS7 = { "pS7", 0, 0, 0 };
static stat st_pS8 = { "pS8", 0, 0, 0 };
static stat st_pS9 = { "pS9", 0, 0, 0 };
static stat st_qS1 = { "qS1", 0, 0, 0 };
static stat st_qS2 = { "qS2", 0, 0, 0 };
static stat st_qS3 = { "qS3", 0, 0, 0 };
static stat st_qS4 = { "qS4", 0, 0, 0 };
static stat st_qS5 = { "qS5", 0, 0, 0 };
static stat st_qS6 = { "qS6", 0, 0, 0 };
static stat st_qS7 = { "qS7", 0, 0, 0 };
static stat st_qS8 = { "qS8", 0, 0, 0 };
static stat st_qS9 = { "qS9", 0, 0, 0 };
static stat st_pi_lo = { "pi_lo", 0, 0, 0 };
static stat st_atanhi = { "atanhi[]", 0, 0, 0 };
static stat st_atanlo = { "atanlo[]", 0, 0, 0 };
static stat st_aT = { "aT[]", 0, 0, 0 };

static void
report_mem_fail(stat &s, const char *tag, const void *got, const void *want)
{
	const auto *pg = static_cast<const unsigned char *>(got);
	const auto *pw = static_cast<const unsigned char *>(want);
	std::size_t i;

	if (s.reported >= MAX_REPORT)
		return;
	s.reported++;
	std::printf("  %s FAIL [%s] port=", s.name, tag);
	for (i = 0; i < sizeof(long double); ++i)
		std::printf("%02x", pg[i]);
	std::printf(" ref=");
	for (i = 0; i < sizeof(long double); ++i)
		std::printf("%02x", pw[i]);
	std::printf("\n");
}

static void
report_size_fail(stat &s, const char *tag, std::size_t got, std::size_t want)
{
	if (s.reported >= MAX_REPORT)
		return;
	s.reported++;
	std::printf("  %s FAIL [%s] port=%zu ref=%zu\n", s.name, tag, got, want);
}

static void
check_ld_mem(stat &s, const char *tag, const void *got, const void *want)
{
	++s.cases;
	if (std::memcmp(got, want, sizeof(long double)) != 0) {
		++s.fails;
		report_mem_fail(s, tag, got, want);
	}
}

static void
check_size(stat &s, const char *tag, std::size_t got, std::size_t want)
{
	++s.cases;
	if (got != want) {
		++s.fails;
		report_size_fail(s, tag, got, want);
	}
}

static void
check_scalar(stat &s, const void *got, const void *want)
{
	check_ld_mem(s, "scalar", got, want);
}

static void
check_table(stat &s, const char *name, const void *ptbl, const void *rtbl,
    std::size_t pn, std::size_t rn)
{
	char tag[64];
	std::size_t i;
	const auto *p = static_cast<const unsigned char *>(ptbl);
	const auto *r = static_cast<const unsigned char *>(rtbl);

	check_size(s, "n_elem", pn, rn);
	for (i = 0; i < pn && i < rn; ++i) {
		std::snprintf(tag, sizeof(tag), "%s[%zu]", name, i);
		check_ld_mem(s, tag, p + i * sizeof(long double),
		    r + i * sizeof(long double));
	}
	if (pn > 0) {
		check_ld_mem(s, "first", p, r);
		check_ld_mem(s, "last", p + (pn - 1) * sizeof(long double),
		    r + (rn - 1) * sizeof(long double));
	}
}

static void
check_all_scalars()
{
	check_scalar(st_pS0, &port::pS0, &ref_pS0);
	check_scalar(st_pS1, &port::pS1, &ref_pS1);
	check_scalar(st_pS2, &port::pS2, &ref_pS2);
	check_scalar(st_pS3, &port::pS3, &ref_pS3);
	check_scalar(st_pS4, &port::pS4, &ref_pS4);
	check_scalar(st_pS5, &port::pS5, &ref_pS5);
	check_scalar(st_pS6, &port::pS6, &ref_pS6);
	check_scalar(st_pS7, &port::pS7, &ref_pS7);
	check_scalar(st_pS8, &port::pS8, &ref_pS8);
	check_scalar(st_pS9, &port::pS9, &ref_pS9);
	check_scalar(st_qS1, &port::qS1, &ref_qS1);
	check_scalar(st_qS2, &port::qS2, &ref_qS2);
	check_scalar(st_qS3, &port::qS3, &ref_qS3);
	check_scalar(st_qS4, &port::qS4, &ref_qS4);
	check_scalar(st_qS5, &port::qS5, &ref_qS5);
	check_scalar(st_qS6, &port::qS6, &ref_qS6);
	check_scalar(st_qS7, &port::qS7, &ref_qS7);
	check_scalar(st_qS8, &port::qS8, &ref_qS8);
	check_scalar(st_qS9, &port::qS9, &ref_qS9);
	check_scalar(st_pi_lo, &port::pi_lo, &ref_pi_lo);
}

static void
check_all_tables()
{
	check_table(st_atanhi, "atanhi", port::atanhi, ref_atanhi,
	    std::size(port::atanhi), ref_atanhi_n);
	check_table(st_atanlo, "atanlo", port::atanlo, ref_atanlo,
	    std::size(port::atanlo), ref_atanlo_n);
	check_table(st_aT, "aT", port::aT, ref_aT, std::size(port::aT), ref_aT_n);
}

struct probe {
	stat *s;
	const void *port_val;
	const void *ref_val;
	const void *port_tbl;
	const void *ref_tbl;
	std::size_t tbl_n;
};

static void
run_random_sweep()
{
	const std::array<probe, 23> probes = { {
	    { &st_pS0, &port::pS0, &ref_pS0, nullptr, nullptr, 0 },
	    { &st_pS1, &port::pS1, &ref_pS1, nullptr, nullptr, 0 },
	    { &st_pS2, &port::pS2, &ref_pS2, nullptr, nullptr, 0 },
	    { &st_pS3, &port::pS3, &ref_pS3, nullptr, nullptr, 0 },
	    { &st_pS4, &port::pS4, &ref_pS4, nullptr, nullptr, 0 },
	    { &st_pS5, &port::pS5, &ref_pS5, nullptr, nullptr, 0 },
	    { &st_pS6, &port::pS6, &ref_pS6, nullptr, nullptr, 0 },
	    { &st_pS7, &port::pS7, &ref_pS7, nullptr, nullptr, 0 },
	    { &st_pS8, &port::pS8, &ref_pS8, nullptr, nullptr, 0 },
	    { &st_pS9, &port::pS9, &ref_pS9, nullptr, nullptr, 0 },
	    { &st_qS1, &port::qS1, &ref_qS1, nullptr, nullptr, 0 },
	    { &st_qS2, &port::qS2, &ref_qS2, nullptr, nullptr, 0 },
	    { &st_qS3, &port::qS3, &ref_qS3, nullptr, nullptr, 0 },
	    { &st_qS4, &port::qS4, &ref_qS4, nullptr, nullptr, 0 },
	    { &st_qS5, &port::qS5, &ref_qS5, nullptr, nullptr, 0 },
	    { &st_qS6, &port::qS6, &ref_qS6, nullptr, nullptr, 0 },
	    { &st_qS7, &port::qS7, &ref_qS7, nullptr, nullptr, 0 },
	    { &st_qS8, &port::qS8, &ref_qS8, nullptr, nullptr, 0 },
	    { &st_qS9, &port::qS9, &ref_qS9, nullptr, nullptr, 0 },
	    { &st_pi_lo, &port::pi_lo, &ref_pi_lo, nullptr, nullptr, 0 },
	    { &st_atanhi, nullptr, nullptr, port::atanhi, ref_atanhi,
	      std::size(port::atanhi) },
	    { &st_atanlo, nullptr, nullptr, port::atanlo, ref_atanlo,
	      std::size(port::atanlo) },
	    { &st_aT, nullptr, nullptr, port::aT, ref_aT, std::size(port::aT) },
	} };

	std::mt19937_64 rng(0x84008400u);
	std::uniform_int_distribution<int> pick(0, (int)probes.size() - 1);
	unsigned long long i;

	for (i = 0; i < RANDOM_ITERS; ++i) {
		const probe &p = probes[(std::size_t)pick(rng)];
		char tag[32];

		if (p.port_val != nullptr) {
			check_scalar(*p.s, p.port_val, p.ref_val);
			continue;
		}

		std::uniform_int_distribution<std::size_t> idx(0, p.tbl_n - 1);
		const std::size_t j = idx(rng);
		const auto *pb = static_cast<const unsigned char *>(p.port_tbl);
		const auto *rb = static_cast<const unsigned char *>(p.ref_tbl);
		std::snprintf(tag, sizeof(tag), "rand[%zu]", j);
		check_ld_mem(*p.s, tag, pb + j * sizeof(long double),
		    rb + j * sizeof(long double));
	}
}

static void
print_row(const stat &s)
{
	std::printf("%-12s %10llu %10llu\n", s.name, s.cases, s.fails);
}

static unsigned long long
total_fails()
{
	return st_pS0.fails + st_pS1.fails + st_pS2.fails + st_pS3.fails +
	    st_pS4.fails + st_pS5.fails + st_pS6.fails + st_pS7.fails +
	    st_pS8.fails + st_pS9.fails + st_qS1.fails + st_qS2.fails +
	    st_qS3.fails + st_qS4.fails + st_qS5.fails + st_qS6.fails +
	    st_qS7.fails + st_qS8.fails + st_qS9.fails + st_pi_lo.fails +
	    st_atanhi.fails + st_atanlo.fails + st_aT.fails;
}

int
main()
{
	check_all_scalars();
	check_all_tables();
	run_random_sweep();

	std::printf("b0084 differential harness (invtrig coefficient tables)\n");
	std::printf("%-12s %10s %10s\n", "symbol", "cases", "fails");
	print_row(st_pS0);
	print_row(st_pS1);
	print_row(st_pS2);
	print_row(st_pS3);
	print_row(st_pS4);
	print_row(st_pS5);
	print_row(st_pS6);
	print_row(st_pS7);
	print_row(st_pS8);
	print_row(st_pS9);
	print_row(st_qS1);
	print_row(st_qS2);
	print_row(st_qS3);
	print_row(st_qS4);
	print_row(st_qS5);
	print_row(st_qS6);
	print_row(st_qS7);
	print_row(st_qS8);
	print_row(st_qS9);
	print_row(st_pi_lo);
	print_row(st_atanhi);
	print_row(st_atanlo);
	print_row(st_aT);

	return total_fails() == 0 ? 0 : 1;
}
