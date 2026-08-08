/*
 * Differential test harness for PBSD batch b0084.
 *
 * invtrig.c coefficient tables, s_expl.c (expl/expm1l), e_lgammal_r.c.
 */

#include <array>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
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
long double ref_expl(long double);
long double ref_expm1l(long double);
long double ref_lgammal_r(long double, int *);
}

static const unsigned long long RANDOM_ITERS = 200000ull;
static const unsigned MAX_REPORT = 8;
static const int SIGNGAM_GUARD = 0x7f7f7f7f;

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
static stat st_expl = { "expl", 0, 0, 0 };
static stat st_expm1l = { "expm1l", 0, 0, 0 };
static stat st_lgammal_r = { "lgammal_r", 0, 0, 0 };

static bool
ld_equal(long double a, long double b)
{
	return std::memcmp(&a, &b, sizeof(long double)) == 0;
}

static void
report_ld_fail(stat &s, const char *tag, long double got, long double want)
{
	const auto *pg = reinterpret_cast<const unsigned char *>(&got);
	const auto *pw = reinterpret_cast<const unsigned char *>(&want);
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
check_ld(stat &s, const char *tag, long double got, long double want)
{
	++s.cases;
	if (!ld_equal(got, want)) {
		++s.fails;
		report_ld_fail(s, tag, got, want);
	}
}

static void
check_scalar(stat &s, long double got, long double want)
{
	check_ld(s, "scalar", got, want);
}

static void
check_table(stat &s, const char *name, const long double *ptbl,
    const long double *rtbl, std::size_t pn, std::size_t rn)
{
	char tag[64];
	std::size_t i;

	++s.cases;
	if (pn != rn) {
		++s.fails;
		if (s.reported < MAX_REPORT) {
			s.reported++;
			std::printf("  %s FAIL [n_elem] port=%zu ref=%zu\n",
			    s.name, pn, rn);
		}
	}
	for (i = 0; i < pn && i < rn; ++i) {
		std::snprintf(tag, sizeof(tag), "%s[%zu]", name, i);
		check_ld(s, tag, ptbl[i], rtbl[i]);
	}
	if (pn > 0) {
		check_ld(s, "first", ptbl[0], rtbl[0]);
		check_ld(s, "last", ptbl[pn - 1], rtbl[pn - 1]);
	}
}

static long double
mkld128(std::uint16_t expsign, std::uint64_t manh, std::uint64_t manl)
{
	union {
		long double e;
		struct {
			std::uint64_t manl;
			std::uint64_t manh;
			std::uint16_t expsign;
		} x;
	} u;

	u.x.manl = manl;
	u.x.manh = manh;
	u.x.expsign = expsign;
	return u.e;
}

static long double
ld128_from_bytes(const unsigned char (&b)[16])
{
	long double x;

	std::memcpy(&x, b, sizeof(x));
	return x;
}

static void
check_expl(long double x, const char *tag)
{
	check_ld(st_expl, tag, port::expl(x), ref_expl(x));
}

static void
check_expm1l(long double x, const char *tag)
{
	check_ld(st_expm1l, tag, port::expm1l(x), ref_expm1l(x));
}

static void
check_lgammal_r(long double x, const char *tag)
{
	int got_sg, want_sg;
	long double got, want;
	unsigned char gbuf[sizeof(int) + 8];
	unsigned char wbuf[sizeof(int) + 8];

	st_lgammal_r.cases++;
	std::memset(gbuf, 0x7f, sizeof(gbuf));
	std::memset(wbuf, 0x7f, sizeof(wbuf));
	got_sg = SIGNGAM_GUARD;
	want_sg = SIGNGAM_GUARD;
	std::memcpy(gbuf, &got_sg, sizeof(got_sg));
	std::memcpy(wbuf, &want_sg, sizeof(want_sg));
	got = port::lgammal_r(x, reinterpret_cast<int *>(gbuf));
	want = ref_lgammal_r(x, reinterpret_cast<int *>(wbuf));
	got_sg = *reinterpret_cast<int *>(gbuf);
	want_sg = *reinterpret_cast<int *>(wbuf);

	if (!ld_equal(got, want) || got_sg != want_sg ||
	    std::memcmp(gbuf, wbuf, sizeof(gbuf)) != 0) {
		st_lgammal_r.fails++;
		if (st_lgammal_r.reported < MAX_REPORT) {
			st_lgammal_r.reported++;
			std::printf("  lgammal_r FAIL [%s] sign port=%d ref=%d\n",
			    tag, got_sg, want_sg);
			report_ld_fail(st_lgammal_r, tag, got, want);
		}
	}
}

static void
check_all_scalars()
{
	check_scalar(st_pS0, port::pS0, ref_pS0);
	check_scalar(st_pS1, port::pS1, ref_pS1);
	check_scalar(st_pS2, port::pS2, ref_pS2);
	check_scalar(st_pS3, port::pS3, ref_pS3);
	check_scalar(st_pS4, port::pS4, ref_pS4);
	check_scalar(st_pS5, port::pS5, ref_pS5);
	check_scalar(st_pS6, port::pS6, ref_pS6);
	check_scalar(st_pS7, port::pS7, ref_pS7);
	check_scalar(st_pS8, port::pS8, ref_pS8);
	check_scalar(st_pS9, port::pS9, ref_pS9);
	check_scalar(st_qS1, port::qS1, ref_qS1);
	check_scalar(st_qS2, port::qS2, ref_qS2);
	check_scalar(st_qS3, port::qS3, ref_qS3);
	check_scalar(st_qS4, port::qS4, ref_qS4);
	check_scalar(st_qS5, port::qS5, ref_qS5);
	check_scalar(st_qS6, port::qS6, ref_qS6);
	check_scalar(st_qS7, port::qS7, ref_qS7);
	check_scalar(st_qS8, port::qS8, ref_qS8);
	check_scalar(st_qS9, port::qS9, ref_qS9);
	check_scalar(st_pi_lo, port::pi_lo, ref_pi_lo);
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

static const long double EXPM1_T1 = -0.1659L;
static const long double EXPM1_T2 = 0.1659L;
static const long double O_THRESH = 11356.523406294143949491931077970763428L;
static const long double U_THRESH = -11433.462743336297878837243843452621503L;

static void
edge_cases_math()
{
	static const struct {
		std::uint64_t manl;
		std::uint64_t manh;
		std::uint16_t expsign;
	} ldvec[] = {
		{ 0, 0, 0x0000 },
		{ 0, 0, 0x8000 },
		{ 1, 0, 0x0000 },
		{ 1, 0, 0x8000 },
		{ 0x80, 0, 0x0000 },
		{ 0, 0x7fffffffffffffffULL, 0x0000 },
		{ 0, 0x7fffffffffffffffULL, 0x8000 },
		{ 0, 0x8000000000000000ULL, 0x0001 },
		{ 0, 0x8000000000000000ULL, 0x8001 },
		{ 0, 0x8000000000000000ULL, 0x3ffe },
		{ 0, 0x8000000000000000ULL, 0xbffe },
		{ 0, 0x8000000000000000ULL, 0x3fff },
		{ 0, 0x8000000000000000ULL, 0xbfff },
		{ 1, 0x8000000000000000ULL, 0x3fff },
		{ 0x8080808080808080ULL, 0x8000000000000000ULL, 0x3fff },
		{ 0, 0x8000000000000000ULL, 0x7fff },
		{ 0, 0x8000000000000000ULL, 0xffff },
		{ 0, 0xc000000000000000ULL, 0x7fff },
		{ 0, 0xc000000000000000ULL, 0xffff },
		{ 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x7fff },
		{ 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffff },
	};
	std::size_t i;

	for (i = 0; i < sizeof(ldvec) / sizeof(ldvec[0]); ++i) {
		long double x = mkld128(ldvec[i].manl, ldvec[i].manh,
		    ldvec[i].expsign);
		check_expl(x, "ldvec");
		check_expm1l(x, "ldvec");
		check_lgammal_r(x, "ldvec");
	}

	static const long double xs[] = {
		0.0L, -0.0L, 1.0L, -1.0L,
		0x1p-114L, -0x1p-114L, 0x1p-113L, -0x1p-113L,
		0x1p-76L, -0x1p-76L, 0x1p-74L, -0x1p-74L,
		64.0L, -64.0L, 65.0L, -65.0L,
		128.0L, -128.0L, 127.0L, -127.0L,
		8192.0L, -8192.0L, 8191.0L, -8191.0L,
		16384.0L, -16384.0L, 16383.0L, -16383.0L,
		O_THRESH, O_THRESH + 1.0L, O_THRESH - 1.0L,
		U_THRESH, U_THRESH + 1.0L, U_THRESH - 1.0L,
		EXPM1_T1, EXPM1_T2,
		EXPM1_T1 - 1e-6L, EXPM1_T2 + 1e-6L,
		EXPM1_T1 + 1e-6L, EXPM1_T2 - 1e-6L,
		0.03125L, -0.03125L, 0.1659L, -0.1659L,
		0.73159980773925781e-01L, 0.23163998126983643e-01L,
		0.89999961853027344e-01L, 1.2316322326660156e+00L,
		1.7316312789916992e+00L, 2.0L, 3.0L, 7.0L, 8.0L,
		-2.5L, -1.5L, -0.5L, 0.5L,
		1e-20L, -1e-20L, 1e20L, -1e20L,
		LDBL_MAX, -LDBL_MAX,
		LDBL_MIN, -LDBL_MIN,
	};
	std::size_t a;

	for (a = 0; a < sizeof(xs) / sizeof(xs[0]); ++a) {
		check_expl(xs[a], "domain");
		check_expm1l(xs[a], "domain");
		check_lgammal_r(xs[a], "domain");
	}
}

struct probe {
	stat *s;
	const long double *port_val;
	const long double *ref_val;
	const long double *port_tbl;
	const long double *ref_tbl;
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
	std::uint64_t rstate = 0x243f6a8885a308d3ull;
	unsigned long long i;

	for (i = 0; i < RANDOM_ITERS; ++i) {
		const probe &p = probes[(std::size_t)pick(rng)];
		char tag[32];

		if (p.port_val != nullptr) {
			check_scalar(*p.s, *p.port_val, *p.ref_val);
		} else {
			std::uniform_int_distribution<std::size_t> idx(0,
			    p.tbl_n - 1);
			const std::size_t j = idx(rng);
			std::snprintf(tag, sizeof(tag), "rand[%zu]", j);
			check_ld(*p.s, tag, p.port_tbl[j], p.ref_tbl[j]);
		}

		rstate += 0x9e3779b97f4a7c15ull;
		std::uint64_t z = rstate;
		z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
		z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
		z = z ^ (z >> 31);
		unsigned char b[16];
		std::size_t j;

		for (j = 0; j < 16; ++j)
			b[j] = (unsigned char)((z >> ((j % 8) * 8)) & 0xffu);
		long double x = ld128_from_bytes(b);

		if ((i % 3) == 0)
			check_expl(x, "random");
		if ((i % 3) == 1)
			check_expm1l(x, "random");
		if ((i % 3) == 2 && (i % 17) == 2)
			check_lgammal_r(x, "random");
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
	    st_atanhi.fails + st_atanlo.fails + st_aT.fails +
	    st_expl.fails + st_expm1l.fails + st_lgammal_r.fails;
}

int
main()
{
	check_all_scalars();
	check_all_tables();
	edge_cases_math();
	run_random_sweep();

	std::printf("b0084 differential harness\n");
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
	print_row(st_expl);
	print_row(st_expm1l);
	print_row(st_lgammal_r);

	return total_fails() == 0 ? 0 : 1;
}
