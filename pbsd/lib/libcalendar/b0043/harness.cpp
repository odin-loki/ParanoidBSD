/*
 * Differential harness for batch b0043 (lib/libcalendar).
 *
 * Every case runs the C++23 port and the C oracle on independent guard-filled
 * buffers, then compares return values, pointer offsets from buffer bases,
 * and the entire buffer contents.
 */

#include <climits>
#include <cstddef>
#include <csignal>
#include <csetjmp>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libcalendar.b0043;

namespace P = pbsd::lib_libcalendar::b0043;

struct c_date {
	int y;
	int m;
	int d;
};

extern "C" {
struct date *ref_easterg(int y, struct date *dt);
struct date *ref_easterog(int y, struct date *dt);
struct date *ref_easteroj(int y, struct date *dt);
struct date *ref_gdate(int ndays, struct date *dt);
struct date *ref_jdate(int ndays, struct date *dt);
int ref_ndaysg(struct date *dt);
int ref_ndaysj(struct date *dt);
int ref_week(int nd, int *y);
int ref_weekday(int nd);
}

enum {
	F_EASTERG, F_EASTEROG, F_EASTEROJ,
	F_GDATE, F_JDATE,
	F_NDAYSG, F_NDAYSJ,
	F_WEEK, F_WEEKDAY,
	F_COUNT
};

static const char *const fn_name[F_COUNT] = {
	"easterg", "easterog", "easteroj",
	"gdate", "jdate",
	"ndaysg", "ndaysj",
	"week", "weekday"
};

static unsigned long long n_cases[F_COUNT];
static unsigned long long n_fails[F_COUNT];

static constexpr unsigned char GUARD = 0x7f;
static constexpr unsigned long long RANDOM_ITERS = 200000;

struct PortDateFrame {
	unsigned char lead[32];
	P::date dt;
	unsigned char trail[32];
};

struct RefDateFrame {
	unsigned char lead[32];
	c_date dt;
	unsigned char trail[32];
};

struct IntFrame {
	unsigned char lead[32];
	int val;
	unsigned char trail[32];
};

static uint64_t rng_state = 0xB00043ULL;

static sigjmp_buf g_fpe_jmp;
static volatile sig_atomic_t g_in_fpe_guard;

static void
fpe_handler(int)
{
	if (g_in_fpe_guard)
		siglongjmp(g_fpe_jmp, 1);
}

struct EasterRun {
	bool ran;
	bool fpe;
	ptrdiff_t off;
};

static EasterRun
run_port_easter(int fn, int y, PortDateFrame &pf)
{
	struct sigaction sa {}, old {};
	EasterRun out {};

	sa.sa_handler = fpe_handler;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGFPE, &sa, &old);
	g_in_fpe_guard = 1;
	if (sigsetjmp(g_fpe_jmp, 1) != 0) {
		g_in_fpe_guard = 0;
		sigaction(SIGFPE, &old, nullptr);
		out.fpe = true;
		return out;
	}

	P::date *pgot;
	switch (fn) {
	case F_EASTERG:
		pgot = P::easterg(y, &pf.dt);
		break;
	case F_EASTEROG:
		pgot = P::easterog(y, &pf.dt);
		break;
	default:
		pgot = P::easteroj(y, &pf.dt);
		break;
	}

	g_in_fpe_guard = 0;
	sigaction(SIGFPE, &old, nullptr);
	out.ran = true;
	out.off = pgot == nullptr ? -1 : pgot - &pf.dt;
	return out;
}

static EasterRun
run_ref_easter(int fn, int y, RefDateFrame &rf)
{
	struct sigaction sa {}, old {};
	EasterRun out {};

	sa.sa_handler = fpe_handler;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGFPE, &sa, &old);
	g_in_fpe_guard = 1;
	if (sigsetjmp(g_fpe_jmp, 1) != 0) {
		g_in_fpe_guard = 0;
		sigaction(SIGFPE, &old, nullptr);
		out.fpe = true;
		return out;
	}

	struct date *rgot;
	switch (fn) {
	case F_EASTERG:
		rgot = ref_easterg(y, reinterpret_cast<struct date *>(&rf.dt));
		break;
	case F_EASTEROG:
		rgot = ref_easterog(y, reinterpret_cast<struct date *>(&rf.dt));
		break;
	default:
		rgot = ref_easteroj(y, reinterpret_cast<struct date *>(&rf.dt));
		break;
	}

	g_in_fpe_guard = 0;
	sigaction(SIGFPE, &old, nullptr);
	out.ran = true;
	out.off = rgot == nullptr ? -1 :
	    reinterpret_cast<c_date *>(rgot) - &rf.dt;
	return out;
}

static uint64_t
rnd(void)
{
	uint64_t z;

	rng_state += 0x9e3779b97f4a7c15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static int
rnd_year(void)
{
	uint64_t r = rnd();
	int sign = (r & 1) ? 1 : -1;
	int mag = (int)(r % 100000u);
	return sign * mag;
}

static int
rnd_ndays(void)
{
	uint64_t r = rnd();
	int sign = (r & 3) == 0 ? -1 : 1;
	int mag = (int)(r % 2000000u);
	return sign * mag;
}

static void
port_date_frame_init(PortDateFrame &f, int y, int m, int d)
{
	std::memset(&f, GUARD, sizeof f);
	f.dt.y = y;
	f.dt.m = m;
	f.dt.d = d;
}

static void
ref_date_frame_init(RefDateFrame &f, int y, int m, int d)
{
	std::memset(&f, GUARD, sizeof f);
	f.dt.y = y;
	f.dt.m = m;
	f.dt.d = d;
}

static bool
date_frames_match(const PortDateFrame &pf, const RefDateFrame &rf)
{
	if (std::memcmp(pf.lead, rf.lead, sizeof pf.lead) != 0)
		return false;
	if (pf.dt.y != rf.dt.y || pf.dt.m != rf.dt.m || pf.dt.d != rf.dt.d)
		return false;
	if (std::memcmp(pf.trail, rf.trail, sizeof pf.trail) != 0)
		return false;
	return true;
}

static_assert(sizeof(P::date) == sizeof(c_date));
static_assert(alignof(P::date) == alignof(c_date));

static void
int_frame_init(IntFrame &f, int v)
{
	std::memset(&f, GUARD, sizeof f);
	f.val = v;
}

static bool
int_frames_eq(const IntFrame &a, const IntFrame &b)
{
	return std::memcmp(&a, &b, sizeof a) == 0;
}

static unsigned reported;

static void
report(int fn, const char *what, long long pv, long long rv)
{
	if (reported >= 40u) {
		if (reported == 40u)
			std::printf("  ... further mismatches suppressed\n");
		reported++;
		return;
	}
	reported++;
	std::printf("  MISMATCH %s %s port=%lld ref=%lld\n",
	    fn_name[fn], what, pv, rv);
}

static void
bump_fail(int fn)
{
	n_fails[fn]++;
}

/* ------------------------------------------------------------------ */
/* easterg / easterog / easteroj                                      */
/* ------------------------------------------------------------------ */

static void
check_easter(int fn, int y, const char *tag)
{
	PortDateFrame pf;
	RefDateFrame rf;
	EasterRun pr, rr;

	n_cases[fn]++;
	port_date_frame_init(pf, 0x11111111, 0x22222222, 0x33333333);
	ref_date_frame_init(rf, 0x11111111, 0x22222222, 0x33333333);

	pr = run_port_easter(fn, y, pf);
	rr = run_ref_easter(fn, y, rf);

	if (pr.fpe != rr.fpe) {
		bump_fail(fn);
		report(fn, tag, pr.fpe ? -2 : pr.off, rr.fpe ? -2 : rr.off);
		return;
	}
	if (pr.fpe)
		return;

	if (pr.off != rr.off || !date_frames_match(pf, rf)) {
		bump_fail(fn);
		if (pr.off != rr.off)
			report(fn, tag, pr.off, rr.off);
		else
			report(fn, tag, pf.dt.y, rf.dt.y);
	}
	(void)tag;
}

/* ------------------------------------------------------------------ */
/* gdate / jdate                                                      */
/* ------------------------------------------------------------------ */

static void
check_date_conv(int fn, int ndays, const char *tag)
{
	PortDateFrame pf;
	RefDateFrame rf;

	n_cases[fn]++;
	port_date_frame_init(pf, 0x44444444, 0x55555555, 0x66666666);
	ref_date_frame_init(rf, 0x44444444, 0x55555555, 0x66666666);

	P::date *pgot;
	struct date *rgot;
	long long poff, roff;

	if (fn == F_GDATE) {
		pgot = P::gdate(ndays, &pf.dt);
		rgot = ref_gdate(ndays, reinterpret_cast<struct date *>(&rf.dt));
	} else {
		pgot = P::jdate(ndays, &pf.dt);
		rgot = ref_jdate(ndays, reinterpret_cast<struct date *>(&rf.dt));
	}

	poff = pgot == nullptr ? -1 : (long long)(pgot - &pf.dt);
	roff = rgot == nullptr ? -1 :
	    (long long)(reinterpret_cast<c_date *>(rgot) - &rf.dt);

	if (poff != roff || !date_frames_match(pf, rf)) {
		bump_fail(fn);
		if (poff != roff)
			report(fn, tag, poff, roff);
		else
			report(fn, tag, pf.dt.y, rf.dt.y);
	}
	(void)tag;
}

/* ------------------------------------------------------------------ */
/* ndaysg / ndaysj                                                    */
/* ------------------------------------------------------------------ */

static void
check_ndays(int fn, int y, int m, int d, const char *tag)
{
	PortDateFrame pf;
	RefDateFrame rf;
	int pgot, rgot;

	n_cases[fn]++;
	port_date_frame_init(pf, y, m, d);
	ref_date_frame_init(rf, y, m, d);

	if (fn == F_NDAYSG) {
		pgot = P::ndaysg(&pf.dt);
		rgot = ref_ndaysg(reinterpret_cast<struct date *>(&rf.dt));
	} else {
		pgot = P::ndaysj(&pf.dt);
		rgot = ref_ndaysj(reinterpret_cast<struct date *>(&rf.dt));
	}

	if (pgot != rgot || !date_frames_match(pf, rf)) {
		bump_fail(fn);
		report(fn, tag, pgot, rgot);
	}
	(void)tag;
}

/* ------------------------------------------------------------------ */
/* week                                                               */
/* ------------------------------------------------------------------ */

static void
check_week(int nd, const char *tag)
{
	IntFrame py, ry;
	int pw, rw;

	n_cases[F_WEEK]++;
	int_frame_init(py, 0x77777777);
	int_frame_init(ry, 0x77777777);

	pw = P::week(nd, &py.val);
	rw = ref_week(nd, &ry.val);

	if (pw != rw || py.val != ry.val || !int_frames_eq(py, ry)) {
		bump_fail(F_WEEK);
		if (pw != rw)
			report(F_WEEK, tag, pw, rw);
		else
			report(F_WEEK, tag, py.val, ry.val);
	}
	(void)tag;
}

/* ------------------------------------------------------------------ */
/* weekday                                                            */
/* ------------------------------------------------------------------ */

static void
check_weekday(int nd, const char *tag)
{
	int pgot, rgot;

	n_cases[F_WEEKDAY]++;
	pgot = P::weekday(nd);
	rgot = ref_weekday(nd);

	if (pgot != rgot) {
		bump_fail(F_WEEKDAY);
		report(F_WEEKDAY, tag, pgot, rgot);
	}
	(void)tag;
}

/* ------------------------------------------------------------------ */
/* hand-written edge cases                                            */
/* ------------------------------------------------------------------ */

static void
run_edge_cases(void)
{
	static const int years[] = {
		INT_MIN, -400, -100, -4, -1, 0, 1, 4, 19, 20, 99, 100,
		1581, 1582, 1583, 1599, 1600, 1601, 1899, 1900, 1999,
		2000, 2024, 2025, 400, 9999, INT_MAX
	};

	static const struct { int y, m, d; } dates[] = {
		{ 0, 0, 0 }, { -1, 1, 1 }, { 0, 1, 1 }, { 1, 1, 1 },
		{ 1, 2, 29 }, { 4, 2, 29 }, { 4, 3, 1 }, { 1582, 10, 4 },
		{ 1582, 10, 5 }, { 1582, 10, 14 }, { 1582, 10, 15 },
		{ 1600, 2, 29 }, { 1900, 2, 28 }, { 2000, 2, 29 },
		{ 2024, 12, 31 }, { 9999, 12, 31 }, { 1, 3, 1 },
		{ 2, 1, 1 }, { 2, 2, 28 }, { 2, 2, 29 },
		{ 100, 12, 31 }, { 400, 1, 1 }, { 1582, 3, 1 },
		{ 1582, 9, 30 }, { 1582, 10, 1 }, { 1582, 10, 20 },
		{ 1599, 12, 31 }, { 1600, 1, 1 }, { 1600, 3, 1 },
	};

	static const int ndays_vals[] = {
		INT_MIN, -1000000, -10000, -1000, -100, -10, -1, 0, 1, 2,
		3, 4, 5, 6, 7, 10, 28, 29, 30, 31, 59, 60, 90, 100, 365,
		366, 1000, 10000, 577736, 577737, 577738, 577739,
		730000, 1000000, INT_MAX / 4, INT_MAX / 2, INT_MAX
	};

	char tag[64];

	for (unsigned i = 0; i < sizeof years / sizeof years[0]; i++) {
		std::snprintf(tag, sizeof tag, "year-%d", years[i]);
		check_easter(F_EASTERG, years[i], tag);
		check_easter(F_EASTEROG, years[i], tag);
		check_easter(F_EASTEROJ, years[i], tag);
	}

	for (unsigned i = 0; i < sizeof dates / sizeof dates[0]; i++) {
		std::snprintf(tag, sizeof tag, "date-%d-%d-%d",
		    dates[i].y, dates[i].m, dates[i].d);
		check_ndays(F_NDAYSG, dates[i].y, dates[i].m, dates[i].d, tag);
		check_ndays(F_NDAYSJ, dates[i].y, dates[i].m, dates[i].d, tag);
	}

	for (unsigned i = 0; i < sizeof ndays_vals / sizeof ndays_vals[0]; i++) {
		std::snprintf(tag, sizeof tag, "ndays-%d", ndays_vals[i]);
		if (ndays_vals[i] != INT_MAX) {
			check_date_conv(F_GDATE, ndays_vals[i], tag);
			check_date_conv(F_JDATE, ndays_vals[i], tag);
		}
		/* week(INT_MIN) never returns in the original implementation */
		if (ndays_vals[i] != INT_MIN)
			check_week(ndays_vals[i], tag);
		check_weekday(ndays_vals[i], tag);
	}

	/* Round-trip: ndaysg(gdate(nd)) and ndaysj(jdate(nd)) for sample nd */
	for (int nd = -5000; nd <= 5000; nd += 17) {
		PortDateFrame pf;
		RefDateFrame rf;
		int ng, nj;

		port_date_frame_init(pf, 0, 0, 0);
		ref_date_frame_init(rf, 0, 0, 0);
		(void)P::gdate(nd, &pf.dt);
		(void)ref_gdate(nd, reinterpret_cast<struct date *>(&rf.dt));
		ng = P::ndaysg(&pf.dt);
		nj = ref_ndaysg(reinterpret_cast<struct date *>(&rf.dt));
		std::snprintf(tag, sizeof tag, "rt-g-%d", nd);
		if (ng != nj)
			check_ndays(F_NDAYSG, pf.dt.y, pf.dt.m, pf.dt.d, tag);

		port_date_frame_init(pf, 0, 0, 0);
		ref_date_frame_init(rf, 0, 0, 0);
		(void)P::jdate(nd, &pf.dt);
		(void)ref_jdate(nd, reinterpret_cast<struct date *>(&rf.dt));
		ng = P::ndaysj(&pf.dt);
		nj = ref_ndaysj(reinterpret_cast<struct date *>(&rf.dt));
		std::snprintf(tag, sizeof tag, "rt-j-%d", nd);
		if (ng != nj)
			check_ndays(F_NDAYSJ, pf.dt.y, pf.dt.m, pf.dt.d, tag);
	}

	/* Boundary around 1582 switch internal representation */
	for (int d = 1; d <= 31; d++) {
		std::snprintf(tag, sizeof tag, "oct1582-%d", d);
		check_ndays(F_NDAYSG, 1582, 10, d, tag);
		check_ndays(F_NDAYSJ, 1582, 10, d, tag);
	}

	/* firstweek / week boundaries: years where week-1 logic differs */
	for (int y = 1580; y <= 1610; y++) {
		for (int wk = -14; wk <= 14; wk++) {
			int nd = 577000 + y * 7 + wk;
			std::snprintf(tag, sizeof tag, "wk-y%d-off%d", y, wk);
			check_week(nd, tag);
		}
	}

	/* weekday: both sides of zero and modulo boundary */
	for (int nd = -20; nd <= 20; nd++) {
		std::snprintf(tag, sizeof tag, "wd-%d", nd);
		check_weekday(nd, tag);
	}

	/* easter years on metonic cycle boundaries */
	for (int y = 0; y < 19; y++) {
		std::snprintf(tag, sizeof tag, "metonic-%d", y);
		check_easter(F_EASTERG, y, tag);
		check_easter(F_EASTEROG, y, tag);
		check_easter(F_EASTEROJ, y, tag);
	}

	/* Century boundaries for easterg algorithm */
	for (int c = 15; c <= 22; c++) {
		int y = c * 100;
		std::snprintf(tag, sizeof tag, "century-%d", y);
		check_easter(F_EASTERG, y, tag);
		check_easter(F_EASTERG, y + 1, tag);
		check_easter(F_EASTERG, y - 1, tag);
	}
}

/* ------------------------------------------------------------------ */
/* randomised sweep                                                   */
/* ------------------------------------------------------------------ */

static void
run_random_sweep(void)
{
	char tag[48];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		int y = rnd_year();
		int m = (int)(rnd() % 13u);
		int d = (int)(rnd() % 32u);
		int nd = rnd_ndays();

		std::snprintf(tag, sizeof tag, "rnd-%llu", i);

		switch ((unsigned)(rnd() % 9u)) {
		case 0:
			check_easter(F_EASTERG, y, tag);
			break;
		case 1:
			check_easter(F_EASTEROG, y, tag);
			break;
		case 2:
			check_easter(F_EASTEROJ, y, tag);
			break;
		case 3:
			check_date_conv(F_GDATE, nd, tag);
			break;
		case 4:
			check_date_conv(F_JDATE, nd, tag);
			break;
		case 5:
			check_ndays(F_NDAYSG, y, m, d, tag);
			break;
		case 6:
			check_ndays(F_NDAYSJ, y, m, d, tag);
			break;
		case 7:
			check_week(nd, tag);
			break;
		default:
			check_weekday(nd, tag);
			break;
		}
	}

	/* Dedicated per-function random passes so every function gets 200k+ */
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		int y = rnd_year();
		std::snprintf(tag, sizeof tag, "rnd-eg-%llu", i);
		check_easter(F_EASTERG, y, tag);
	}
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		int y = rnd_year();
		std::snprintf(tag, sizeof tag, "rnd-eog-%llu", i);
		check_easter(F_EASTEROG, y, tag);
	}
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		int y = rnd_year();
		std::snprintf(tag, sizeof tag, "rnd-eoj-%llu", i);
		check_easter(F_EASTEROJ, y, tag);
	}
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		int nd = rnd_ndays();
		std::snprintf(tag, sizeof tag, "rnd-gd-%llu", i);
		check_date_conv(F_GDATE, nd, tag);
	}
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		int nd = rnd_ndays();
		std::snprintf(tag, sizeof tag, "rnd-jd-%llu", i);
		check_date_conv(F_JDATE, nd, tag);
	}
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		int y = rnd_year();
		int m = (int)(rnd() % 13u);
		int d = (int)(rnd() % 32u);
		std::snprintf(tag, sizeof tag, "rnd-ng-%llu", i);
		check_ndays(F_NDAYSG, y, m, d, tag);
	}
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		int y = rnd_year();
		int m = (int)(rnd() % 13u);
		int d = (int)(rnd() % 32u);
		std::snprintf(tag, sizeof tag, "rnd-nj-%llu", i);
		check_ndays(F_NDAYSJ, y, m, d, tag);
	}
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		int nd = rnd_ndays();
		std::snprintf(tag, sizeof tag, "rnd-wk-%llu", i);
		check_week(nd, tag);
	}
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		int nd = rnd_ndays();
		std::snprintf(tag, sizeof tag, "rnd-wd-%llu", i);
		check_weekday(nd, tag);
	}
}

static void
print_table(void)
{
	unsigned long long total_cases = 0;
	unsigned long long total_fails = 0;

	std::printf("\n%-12s %14s %14s\n", "function", "cases", "failures");
	std::printf("----------------------------------------------\n");
	for (int i = 0; i < F_COUNT; i++) {
		std::printf("%-12s %14llu %14llu\n",
		    fn_name[i], n_cases[i], n_fails[i]);
		total_cases += n_cases[i];
		total_fails += n_fails[i];
	}
	std::printf("----------------------------------------------\n");
	std::printf("%-12s %14llu %14llu\n", "TOTAL", total_cases, total_fails);
	std::printf("\n%s\n", total_fails == 0 ? "PASS" : "FAIL");
	std::fflush(stdout);
}

int
main(void)
{
	run_edge_cases();
	run_random_sweep();
	print_table();
	return n_fails[0] + n_fails[1] + n_fails[2] + n_fails[3] +
	    n_fails[4] + n_fails[5] + n_fails[6] + n_fails[7] +
	    n_fails[8] ? 1 : 0;
}
