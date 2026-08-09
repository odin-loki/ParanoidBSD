/*
 * Differential test harness for PBSD batch b0330.
 *
 * lib/msun/bsdsrc/b_log.c   -- __log__D()
 * lib/msun/bsdsrc/b_exp.c   -- __exp__D()
 * lib/msun/bsdsrc/b_tgamma.c -- large_gam(), ratfun_gam(), small_gam(),
 *                                smaller_gam(), neg_gam(), tgamma()
 */

#include <cfloat>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.bsdsrc.b0330;

namespace port = pbsd::lib_msun_bsdsrc::b0330;

extern "C" {
struct Double {
	double a;
	double b;
};

struct Double ref___log__D(double x);
double ref___exp__D(double x, double c);
struct Double ref_large_gam(double x);
struct Double ref_ratfun_gam(double z, double c);
double ref_small_gam(double x);
double ref_smaller_gam(double x);
double ref_neg_gam(double x);
double ref_tgamma(double x);
}

static const unsigned MAX_REPORT = 10;
static const long long RANDOM_ITERS = 30000;

static const double LNHUGE = 0x1.6602b15b7ecf2p9;
static const double LNTINY = -0x1.77af8ebeae354p9;
static const double XMAX = 171.624376956302725;
static const double IOTA = 0x1p-56;
static const double LEFT = -0.3955078125;
static const double X0 = 4.6163214496836236e-1;
static const double XLEFT = 1. + LEFT + X0;

struct Stat {
	const char *name;
	long long cases;
	long long fails;
	long long printed;
};

static Stat st_log = { "__log__D", 0, 0, 0 };
static Stat st_exp = { "__exp__D", 0, 0, 0 };
static Stat st_large = { "large_gam", 0, 0, 0 };
static Stat st_ratfun = { "ratfun_gam", 0, 0, 0 };
static Stat st_small = { "small_gam", 0, 0, 0 };
static Stat st_smaller = { "smaller_gam", 0, 0, 0 };
static Stat st_neg = { "neg_gam", 0, 0, 0 };
static Stat st_tgamma = { "tgamma", 0, 0, 0 };

static bool should_print(Stat &st)
{
	return st.printed++ < MAX_REPORT;
}

static std::uint64_t dbits(double d)
{
	std::uint64_t u;

	std::memcpy(&u, &d, sizeof(u));
	return u;
}

static double fromdbits(std::uint64_t u)
{
	double d;

	std::memcpy(&d, &u, sizeof(d));
	return d;
}

static bool dequal(double a, double b)
{
	return dbits(a) == dbits(b);
}

static bool dequal_struct(const port::Double &p, double oa, double ob)
{
	return dequal(p.a, oa) && dequal(p.b, ob);
}

static void dhex(double d)
{
	std::uint64_t u = dbits(d);

	std::printf("%016llx", (unsigned long long)u);
}

struct Rng {
	std::uint64_t s;

	explicit Rng(std::uint64_t seed) : s(seed ? seed : 1) {}

	std::uint64_t next()
	{
		std::uint64_t x = s;

		x ^= x >> 12;
		x ^= x << 25;
		x ^= x >> 27;
		s = x;
		return x * 0x2545f4914f6cdd1dull;
	}

	double next_double()
	{
		std::uint64_t u = next();

		u &= 0x000fffffffffffffull;
		u |= 0x3ff0000000000000ull;
		return fromdbits(u);
	}

	double next_full()
	{
		return fromdbits(next());
	}

	double next_range(double lo, double hi)
	{
		double t = next_double();

		return lo + (hi - lo) * (t - 1.0);
	}
};

static void check_log(double x, const char *tag)
{
	port::Double p;
	struct Double o;

	st_log.cases++;
	p = port::__log__D(x);
	o = ref___log__D(x);
	if (dequal_struct(p, o.a, o.b))
		return;

	st_log.fails++;
	if (should_print(st_log)) {
		std::printf("  __log__D FAIL [%s] x=", tag);
		dhex(x);
		std::printf(" port.a=");
		dhex(p.a);
		std::printf(" port.b=");
		dhex(p.b);
		std::printf(" ref.a=");
		dhex(o.a);
		std::printf(" ref.b=");
		dhex(o.b);
		std::printf("\n");
	}
}

static void check_exp(double x, double c, const char *tag)
{
	double p, o;

	st_exp.cases++;
	p = port::__exp__D(x, c);
	o = ref___exp__D(x, c);
	if (dequal(p, o))
		return;

	st_exp.fails++;
	if (should_print(st_exp)) {
		std::printf("  __exp__D FAIL [%s] x=", tag);
		dhex(x);
		std::printf(" c=");
		dhex(c);
		std::printf(" port=");
		dhex(p);
		std::printf(" ref=");
		dhex(o);
		std::printf("\n");
	}
}

static void check_large(double x, const char *tag)
{
	port::Double p;
	struct Double o;

	st_large.cases++;
	p = port::large_gam(x);
	o = ref_large_gam(x);
	if (dequal_struct(p, o.a, o.b))
		return;

	st_large.fails++;
	if (should_print(st_large)) {
		std::printf("  large_gam FAIL [%s] x=", tag);
		dhex(x);
		std::printf(" port.a=");
		dhex(p.a);
		std::printf(" port.b=");
		dhex(p.b);
		std::printf(" ref.a=");
		dhex(o.a);
		std::printf(" ref.b=");
		dhex(o.b);
		std::printf("\n");
	}
}

static void check_ratfun(double z, double c, const char *tag)
{
	port::Double p;
	struct Double o;

	st_ratfun.cases++;
	p = port::ratfun_gam(z, c);
	o = ref_ratfun_gam(z, c);
	if (dequal_struct(p, o.a, o.b))
		return;

	st_ratfun.fails++;
	if (should_print(st_ratfun)) {
		std::printf("  ratfun_gam FAIL [%s] z=", tag);
		dhex(z);
		std::printf(" c=");
		dhex(c);
		std::printf(" port.a=");
		dhex(p.a);
		std::printf(" port.b=");
		dhex(p.b);
		std::printf(" ref.a=");
		dhex(o.a);
		std::printf(" ref.b=");
		dhex(o.b);
		std::printf("\n");
	}
}

static void check_small(double x, const char *tag)
{
	double p, o;

	st_small.cases++;
	p = port::small_gam(x);
	o = ref_small_gam(x);
	if (dequal(p, o))
		return;

	st_small.fails++;
	if (should_print(st_small)) {
		std::printf("  small_gam FAIL [%s] x=", tag);
		dhex(x);
		std::printf(" port=");
		dhex(p);
		std::printf(" ref=");
		dhex(o);
		std::printf("\n");
	}
}

static void check_smaller(double x, const char *tag)
{
	double p, o;

	st_smaller.cases++;
	p = port::smaller_gam(x);
	o = ref_smaller_gam(x);
	if (dequal(p, o))
		return;

	st_smaller.fails++;
	if (should_print(st_smaller)) {
		std::printf("  smaller_gam FAIL [%s] x=", tag);
		dhex(x);
		std::printf(" port=");
		dhex(p);
		std::printf(" ref=");
		dhex(o);
		std::printf("\n");
	}
}

static void check_neg(double x, const char *tag)
{
	double p, o;

	st_neg.cases++;
	p = port::neg_gam(x);
	o = ref_neg_gam(x);
	if (dequal(p, o))
		return;

	st_neg.fails++;
	if (should_print(st_neg)) {
		std::printf("  neg_gam FAIL [%s] x=", tag);
		dhex(x);
		std::printf(" port=");
		dhex(p);
		std::printf(" ref=");
		dhex(o);
		std::printf("\n");
	}
}

static void check_tgamma(double x, const char *tag)
{
	double p, o;

	st_tgamma.cases++;
	p = port::tgamma(x);
	o = ref_tgamma(x);
	if (dequal(p, o))
		return;

	st_tgamma.fails++;
	if (should_print(st_tgamma)) {
		std::printf("  tgamma FAIL [%s] x=", tag);
		dhex(x);
		std::printf(" port=");
		dhex(p);
		std::printf(" ref=");
		dhex(o);
		std::printf("\n");
	}
}

static const std::uint64_t dvec[] = {
	0x0000000000000000ull,
	0x8000000000000000ull,
	0x7ff0000000000000ull,
	0xfff0000000000000ull,
	0x7ff8000000000000ull,
	0xfff8000000000000ull,
	0x7ff0000000000001ull,
	0x0000000000000001ull,
	0x8000000000000001ull,
	0x000fffffffffffffull,
	0x0010000000000000ull,
	0x7fefffffffffffffull,
	0xbfefffffffffffffull,
	0x3ff0000000000000ull,
	0xbff0000000000000ull,
	0x3ff0000000000001ull,
	0x3fefffffffffffffull,
	0x3fd5555555555555ull,
	0x40562e42fefa39efull,
	0xc0562e42fefa39efull,
	0x40862e42fefa39efull,
	0xc0862e42fefa39efull,
	0x4040000000000000ull,
	0x408f400000000000ull,
	0x3ff8000000000000ull,
	0x3fe0000000000000ull,
	0x3eb0c6f7a0b5ed8dull,
	0x3ff1111111111111ull,
	0x3ff2222222222222ull,
	0x3fe1111111111111ull,
	0x4057c00000000000ull,
	0x4057d00000000000ull,
	0x4057b00000000000ull,
	0x3ff10c6f7a0b5ed8dULL,
	0x3ff10c6f7a0b5ed8fULL,
	0x3ff10c6f7a0b5ed8bULL,
	0x3fb1111111111111ull,
	0x3f81111111111111ull,
	0xbfd1111111111111ull,
	0xc02a000000000000ull,
	0xc02b000000000000ull,
	0xc02c000000000000ull,
	0xc05d000000000000ull,
	0xc05e800000000000ull,
	0xc05d400000000000ull,
	0x3ff5555555555555ull,
	0x4018000000000000ull,
	0x4017ffffffffffffull,
	0x4018000000000001ull,
	0x4010000000000000ull,
	0x400921fb54442d18ull,
	0x3fd921fb54442d18ull,
};

static const std::size_t NDVEC = sizeof(dvec) / sizeof(dvec[0]);

static void edge_cases(void)
{
	std::size_t i, j;

	std::printf("edge:log/tgamma/neg dvec\n"); std::fflush(stdout);
	for (i = 0; i < NDVEC; i++) {
		double x = fromdbits(dvec[i]);

		check_log(x, "dvec");
		check_tgamma(x, "dvec");
		check_neg(x, "dvec");
	}

	std::printf("edge:exp cross\n"); std::fflush(stdout);
	for (i = 0; i < NDVEC; i++) {
			double x = fromdbits(dvec[i]);
			double c = fromdbits(dvec[j]);

			check_exp(x, c, "cross");
		}
	}

	std::printf("edge:ratfun dvec\n"); std::fflush(stdout);
	for (i = 0; i < NDVEC; i++) {
		double c = fromdbits(dvec[(i * 7 + 3) % NDVEC]);

		check_ratfun(z, c, "dvec");
	}

	std::printf("edge:exp bounds\n"); std::fflush(stdout);
	check_exp(LNHUGE, 0., "lnhuge0");
	check_exp(LNHUGE, 1e-20, "lnhuge1");
	check_exp(LNHUGE + 1e-10, 0., "lnhuge+");
	check_exp(LNHUGE - 1e-10, 0., "lnhuge-");
	check_exp(LNTINY, 0., "lntiny0");
	check_exp(LNTINY - 1e-10, 0., "lntiny-");
	check_exp(LNTINY + 1e-10, 0., "lntiny+");
	check_exp(-1000., 0., "under");
	check_exp(1000., 0., "over");
	check_exp(0., 0., "zero");
	check_exp(-0., 0., "negzero");
	check_exp(1., -0.25, "one");
	check_exp(-1., 0.1, "negone");
	check_exp(0.6931471805599453, -1e-18, "ln2");

	std::printf("edge:log bounds\n"); std::fflush(stdout);
	check_log(1., "one");
	check_log(2., "two");
	check_log(0.5, "half");
	check_log(DBL_MIN, "min");
	check_log(DBL_MAX, "max");
	check_log(1. + 1. / 256., "f256");
	check_log(1. - 1. / 512., "near1");
	check_log(0x1p-1022, "subnorm");
	check_log(0x1.1p-1022, "subnorm2");

	std::printf("edge:tgamma bounds\n"); std::fflush(stdout);
	check_tgamma(6., "six");
	check_tgamma(6. - 1e-15, "six-");
	check_tgamma(6. + 1e-15, "six+");
	check_tgamma(XLEFT, "xleft");
	check_tgamma(XLEFT - 1e-15, "xleft-");
	check_tgamma(XLEFT + 1e-15, "xleft+");
	check_tgamma(IOTA, "iota");
	check_tgamma(IOTA * 0.5, "iota/2");
	check_tgamma(-IOTA, "negiota");
	check_tgamma(-IOTA * 0.5, "negiota/2");
	check_tgamma(XMAX, "xmax");
	check_tgamma(XMAX + 1e-9, "xmax+");
	check_tgamma(XMAX - 1e-9, "xmax-");
	check_tgamma(171.63, "171.63");
	check_tgamma(177.79, "177.79");

	for (i = 1; i <= 20; i++) {
		check_tgamma((double)-i, "negint");
		check_tgamma((double)-i + 0.1, "negfrac");
		check_tgamma((double)-i + 0.5, "neghalf");
		check_tgamma((double)-i + 0.9, "negnear");
	}

	check_tgamma(-170.5, "neg170");
	check_tgamma(-180.5, "neg180");
	check_tgamma(-200., "neg200");
	check_tgamma(-0.25, "negq");
	check_tgamma(-0.75, "neg3q");

	std::printf("edge:helpers dvec\n"); std::fflush(stdout);
	for (i = 0; i < NDVEC; i++) {
			check_large(x, "dvec");
		if (x >= XLEFT && x < 6.)
			check_small(x, "dvec");
		if (x > IOTA && x <= XLEFT)
			check_smaller(x, "dvec");
		if (x < -IOTA && std::isfinite(x))
			check_neg(x, "dvec");
	}

	std::printf("edge:helpers fixed\n"); std::fflush(stdout);
	check_large(6., "six");
	check_large(10., "ten");
	check_large(100., "hundred");
	check_large(XMAX - 1e-6, "xmax-");

	check_small(XLEFT, "xleft");
	check_small(3., "three");
	check_small(5.999999, "just6");

	check_smaller(IOTA * 2., "2iota");
	check_smaller(X0 + LEFT, "x0left");
	check_smaller(0.5, "half");
	check_smaller(1., "one");

	check_neg(-0.5, "half");
	check_neg(-2.5, "twohalf");
	check_neg(-171., "neg171");
	check_neg(-185., "neg185");

	check_ratfun(X0, 0., "x0");
	check_ratfun(X0 + LEFT, 0., "x0left");
	check_ratfun(1., 0., "one");
	check_ratfun(0.5, 1e-17, "halfc");
	check_ratfun(0.1, -1e-17, "tenc");
}

static void random_sweep(void)
{
	Rng rng(0xb0330cafed00d15eull);
	long long n;

	for (n = 0; n < RANDOM_ITERS; n++)
		check_log(rng.next_full(), "rand");

	rng = Rng(0xb0330beefc0de001ull);
	for (n = 0; n < RANDOM_ITERS; n++)
		check_exp(rng.next_full(), rng.next_full(), "rand");

	rng = Rng(0xb0330beefc0de002ull);
	for (n = 0; n < RANDOM_ITERS; n++)
		check_large(rng.next_range(6., XMAX), "rand");

	rng = Rng(0xb0330beefc0de003ull);
	for (n = 0; n < RANDOM_ITERS; n++)
		check_ratfun(rng.next_range(-0.5, 1.5),
		    rng.next_range(-1e-15, 1e-15), "rand");

	rng = Rng(0xb0330beefc0de004ull);
	for (n = 0; n < RANDOM_ITERS; n++)
		check_small(rng.next_range(XLEFT, 6.), "rand");

	rng = Rng(0xb0330beefc0de005ull);
	for (n = 0; n < RANDOM_ITERS; n++)
		check_smaller(rng.next_range(IOTA * 2., XLEFT), "rand");

	rng = Rng(0xb0330beefc0de006ull);
	for (n = 0; n < RANDOM_ITERS; n++) {
		double x;

		do {
			x = rng.next_full();
		} while (!(x < -IOTA && std::isfinite(x)));
		check_neg(x, "rand");
	}

	rng = Rng(0xb0330beefc0de007ull);
	for (n = 0; n < RANDOM_ITERS; n++)
		check_tgamma(rng.next_full(), "rand");
}

static void print_table(void)
{
	Stat *all[] = {
		&st_log, &st_exp, &st_large, &st_ratfun, &st_small,
		&st_smaller, &st_neg, &st_tgamma,
	};
	std::size_t i;
	long long total_cases = 0;
	long long total_fails = 0;

	std::printf("\n%-14s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-14s %12s %12s\n", "--------", "-----", "--------");
	for (i = 0; i < sizeof(all) / sizeof(all[0]); i++) {
		std::printf("%-14s %12lld %12lld\n",
		    all[i]->name, all[i]->cases, all[i]->fails);
		total_cases += all[i]->cases;
		total_fails += all[i]->fails;
	}
	std::printf("%-14s %12lld %12lld\n", "TOTAL", total_cases, total_fails);
}

int main()
{
	edge_cases();
	random_sweep();
	print_table();

	return (st_log.fails + st_exp.fails + st_large.fails + st_ratfun.fails +
	    st_small.fails + st_smaller.fails + st_neg.fails + st_tgamma.fails) ?
	    1 : 0;
}
