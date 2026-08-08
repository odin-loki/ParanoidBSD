/*
 * Differential test for PBSD batch b0043 (lib/libcalendar).
 *
 * Every case is run through both the C++23 port and the unmodified C
 * oracle.  Date buffers and int out-parameters are embedded in 0x7f-filled
 * guard frames; the entire frame is compared after each call.
 */

#include <cstdio>
#include <cstdint>
#include <cstring>

import pbsd.lib.libcalendar.b0043;

extern "C" {
struct date {
	int y;
	int m;
	int d;
};

date *ref_jdate(int ndays, date *dt);
date *ref_gdate(int ndays, date *dt);
int ref_ndaysj(date *dt);
int ref_ndaysg(date *dt);
int ref_ndaysji(date *idt);
int ref_ndaysgi(date *idt);
date *ref_date2idt(date *idt, date *dt);
date *ref_idt2date(date *dt, date *idt);
int ref_weekday(int nd);
int ref_firstweek(int y);
int ref_week(int nd, int *y);
date *ref_easterg(int y, date *dt);
date *ref_easterog(int y, date *dt);
date *ref_easteroj(int y, date *dt);
int ref_easterodn(int y);
}

namespace P = pbsd::lib_libcalendar::b0043;

namespace {

constexpr int kMaxReport = 8;
constexpr int kRandomPerFn = 200000;
constexpr unsigned char kGuard = 0x7f;

struct Stat {
	const char *name;
	std::uint64_t cases;
	std::uint64_t fails;
	int reported;
};

Stat stats[15];
int nstats;

Stat &
reg(const char *name)
{
	stats[nstats++] = Stat{name, 0, 0, 0};
	return stats[nstats - 1];
}

Stat &st_jdate = reg("jdate");
Stat &st_gdate = reg("gdate");
Stat &st_ndaysj = reg("ndaysj");
Stat &st_ndaysg = reg("ndaysg");
Stat &st_ndaysji = reg("ndaysji");
Stat &st_ndaysgi = reg("ndaysgi");
Stat &st_date2idt = reg("date2idt");
Stat &st_idt2date = reg("idt2date");
Stat &st_weekday = reg("weekday");
Stat &st_firstweek = reg("firstweek");
Stat &st_week = reg("week");
Stat &st_easterg = reg("easterg");
Stat &st_easterog = reg("easterog");
Stat &st_easteroj = reg("easteroj");
Stat &st_easterodn = reg("easterodn");

std::uint64_t rng_state = 0xb0043deadbeefULL;

std::uint64_t
nextrand(void)
{
	std::uint64_t x = rng_state;

	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	rng_state = x;
	return x * 0x2545f4914f6cdd1dULL;
}

int
randrange(int lo, int hi)
{
	if (hi <= lo)
		return lo;
	return lo + (int)(nextrand() % (std::uint64_t)(hi - lo + 1));
}

struct DateFrame {
	unsigned char lead[32];
	P::date dt;
	unsigned char trail[32];
};

struct CDateFrame {
	unsigned char lead[32];
	date dt;
	unsigned char trail[32];
};

struct IntFrame {
	unsigned char lead[32];
	int val;
	unsigned char trail[32];
};

void
date_init(DateFrame &f, int y, int m, int d)
{
	std::memset(&f, kGuard, sizeof f);
	f.dt.y = y;
	f.dt.m = m;
	f.dt.d = d;
}

void
cdate_init(CDateFrame &f, int y, int m, int d)
{
	std::memset(&f, kGuard, sizeof f);
	f.dt.y = y;
	f.dt.m = m;
	f.dt.d = d;
}

void
int_init(IntFrame &f, int v)
{
	std::memset(&f, kGuard, sizeof f);
	f.val = v;
}

bool
frames_match(const DateFrame &a, const DateFrame &b)
{
	return std::memcmp(&a, &b, sizeof a) == 0;
}

bool
cframes_match(const CDateFrame &a, const CDateFrame &b)
{
	return std::memcmp(&a, &b, sizeof a) == 0;
}

bool
intframes_match(const IntFrame &a, const IntFrame &b)
{
	return std::memcmp(&a, &b, sizeof a) == 0;
}

void
report_fail(Stat &s, const char *detail)
{
	s.fails++;
	if (s.reported < kMaxReport) {
		s.reported++;
		std::printf("  %s: MISMATCH %s\n", s.name, detail);
	}
}

void
chk_jdate(int ndays)
{
	DateFrame pf, pf2;
	CDateFrame cf, cf2;
	date_init(pf, 0x11111111, 0x22222222, 0x33333333);
	date_init(pf2, pf.dt.y, pf.dt.m, pf.dt.d);
	cdate_init(cf, pf.dt.y, pf.dt.m, pf.dt.d);
	cdate_init(cf2, cf.dt.y, cf.dt.m, cf.dt.d);

	P::date *pr = P::jdate(ndays, &pf.dt);
	date *cr = ref_jdate(ndays, &cf.dt);

	st_jdate.cases++;
	if ((pr - &pf.dt) != (cr - &cf.dt) ||
	    !frames_match(pf, pf2) || !cframes_match(cf, cf2)) {
		char buf[80];
		std::snprintf(buf, sizeof buf, "ndays=%d", ndays);
		report_fail(st_jdate, buf);
	}
}

void
chk_gdate(int ndays)
{
	DateFrame pf, pf2;
	CDateFrame cf, cf2;
	date_init(pf, 0x11111111, 0x22222222, 0x33333333);
	date_init(pf2, pf.dt.y, pf.dt.m, pf.dt.d);
	cdate_init(cf, pf.dt.y, pf.dt.m, pf.dt.d);
	cdate_init(cf2, cf.dt.y, cf.dt.m, cf.dt.d);

	P::date *pr = P::gdate(ndays, &pf.dt);
	date *cr = ref_gdate(ndays, &cf.dt);

	st_gdate.cases++;
	if ((pr - &pf.dt) != (cr - &cf.dt) ||
	    !frames_match(pf, pf2) || !cframes_match(cf, cf2)) {
		char buf[80];
		std::snprintf(buf, sizeof buf, "ndays=%d", ndays);
		report_fail(st_gdate, buf);
	}
}

void
chk_ndaysj(int y, int m, int d)
{
	DateFrame pf, pf2;
	CDateFrame cf, cf2;
	date_init(pf, y, m, d);
	date_init(pf2, y, m, d);
	cdate_init(cf, y, m, d);
	cdate_init(cf2, y, m, d);

	int pr = P::ndaysj(&pf.dt);
	int cr = ref_ndaysj(&cf.dt);

	st_ndaysj.cases++;
	if (pr != cr || !frames_match(pf, pf2) || !cframes_match(cf, cf2)) {
		char buf[80];
		std::snprintf(buf, sizeof buf, "%d-%02d-%02d", y, m, d);
		report_fail(st_ndaysj, buf);
	}
}

void
chk_ndaysg(int y, int m, int d)
{
	DateFrame pf, pf2;
	CDateFrame cf, cf2;
	date_init(pf, y, m, d);
	date_init(pf2, y, m, d);
	cdate_init(cf, y, m, d);
	cdate_init(cf2, y, m, d);

	int pr = P::ndaysg(&pf.dt);
	int cr = ref_ndaysg(&cf.dt);

	st_ndaysg.cases++;
	if (pr != cr || !frames_match(pf, pf2) || !cframes_match(cf, cf2)) {
		char buf[80];
		std::snprintf(buf, sizeof buf, "%d-%02d-%02d", y, m, d);
		report_fail(st_ndaysg, buf);
	}
}

void
chk_ndaysji(int y, int m, int d)
{
	DateFrame pf, pf2;
	CDateFrame cf, cf2;
	date_init(pf, y, m, d);
	date_init(pf2, y, m, d);
	cdate_init(cf, y, m, d);
	cdate_init(cf2, y, m, d);

	int pr = P::ndaysji(&pf.dt);
	int cr = ref_ndaysji(&cf.dt);

	st_ndaysji.cases++;
	if (pr != cr || !frames_match(pf, pf2) || !cframes_match(cf, cf2)) {
		char buf[80];
		std::snprintf(buf, sizeof buf, "idt %d-%d-%d", y, m, d);
		report_fail(st_ndaysji, buf);
	}
}

void
chk_ndaysgi(int y, int m, int d)
{
	DateFrame pf, pf2;
	CDateFrame cf, cf2;
	date_init(pf, y, m, d);
	date_init(pf2, y, m, d);
	cdate_init(cf, y, m, d);
	cdate_init(cf2, y, m, d);

	int pr = P::ndaysgi(&pf.dt);
	int cr = ref_ndaysgi(&cf.dt);

	st_ndaysgi.cases++;
	if (pr != cr || !frames_match(pf, pf2) || !cframes_match(cf, cf2)) {
		char buf[80];
		std::snprintf(buf, sizeof buf, "idt %d-%d-%d", y, m, d);
		report_fail(st_ndaysgi, buf);
	}
}

void
chk_date2idt(int y, int m, int d)
{
	DateFrame out_p, out_p2, in_p;
	CDateFrame out_c, out_c2, in_c;
	date_init(in_p, y, m, d);
	date_init(in_p2, y, m, d);
	date_init(out_p, 0, 0, 0);
	date_init(out_p2, 0, 0, 0);
	cdate_init(in_c, y, m, d);
	cdate_init(in_c2, y, m, d);
	cdate_init(out_c, 0, 0, 0);
	cdate_init(out_c2, 0, 0, 0);

	P::date *pr = P::date2idt(&out_p.dt, &in_p.dt);
	date *cr = ref_date2idt(&out_c.dt, &in_c.dt);

	st_date2idt.cases++;
	if ((pr == 0) != (cr == 0) ||
	    (pr != 0 && (pr - &out_p.dt) != (cr - &out_c.dt)) ||
	    !frames_match(in_p, in_p2) || !cframes_match(in_c, in_c2) ||
	    !frames_match(out_p, out_p2) || !cframes_match(out_c, out_c2)) {
		char buf[80];
		std::snprintf(buf, sizeof buf, "%d-%02d-%02d", y, m, d);
		report_fail(st_date2idt, buf);
	}
}

void
chk_idt2date(int y, int m, int d)
{
	DateFrame out_p, out_p2, in_p;
	CDateFrame out_c, out_c2, in_c;
	date_init(in_p, y, m, d);
	date_init(in_p2, y, m, d);
	date_init(out_p, 0, 0, 0);
	date_init(out_p2, 0, 0, 0);
	cdate_init(in_c, y, m, d);
	cdate_init(in_c2, y, m, d);
	cdate_init(out_c, 0, 0, 0);
	cdate_init(out_c2, 0, 0, 0);

	P::date *pr = P::idt2date(&out_p.dt, &in_p.dt);
	date *cr = ref_idt2date(&out_c.dt, &in_c.dt);

	st_idt2date.cases++;
	if ((pr == 0) != (cr == 0) ||
	    (pr != 0 && (pr - &out_p.dt) != (cr - &out_c.dt)) ||
	    !frames_match(in_p, in_p2) || !cframes_match(in_c, in_c2) ||
	    !frames_match(out_p, out_p2) || !cframes_match(out_c, out_c2)) {
		char buf[80];
		std::snprintf(buf, sizeof buf, "idt %d-%d-%d", y, m, d);
		report_fail(st_idt2date, buf);
	}
}

void
chk_weekday(int nd)
{
	int pr = P::weekday(nd);
	int cr = ref_weekday(nd);

	st_weekday.cases++;
	if (pr != cr) {
		char buf[80];
		std::snprintf(buf, sizeof buf, "nd=%d", nd);
		report_fail(st_weekday, buf);
	}
}

void
chk_firstweek(int y)
{
	int pr = P::firstweek(y);
	int cr = ref_firstweek(y);

	st_firstweek.cases++;
	if (pr != cr) {
		char buf[80];
		std::snprintf(buf, sizeof buf, "y=%d", y);
		report_fail(st_firstweek, buf);
	}
}

void
chk_week(int nd)
{
	IntFrame py, py2;
	IntFrame cy, cy2;
	int_init(py, 0x55555555);
	int_init(py2, py.val);
	int_init(cy, py.val);
	int_init(cy2, cy.val);

	int pr = P::week(nd, &py.val);
	int cr = ref_week(nd, &cy.val);

	st_week.cases++;
	if (pr != cr || py.val != cy.val ||
	    !intframes_match(py, py2) || !intframes_match(cy, cy2)) {
		char buf[80];
		std::snprintf(buf, sizeof buf, "nd=%d", nd);
		report_fail(st_week, buf);
	}
}

void
chk_easterg(int y)
{
	DateFrame pf, pf2;
	CDateFrame cf, cf2;
	date_init(pf, 0, 0, 0);
	date_init(pf2, 0, 0, 0);
	cdate_init(cf, 0, 0, 0);
	cdate_init(cf2, 0, 0, 0);

	P::date *pr = P::easterg(y, &pf.dt);
	date *cr = ref_easterg(y, &cf.dt);

	st_easterg.cases++;
	if ((pr - &pf.dt) != (cr - &cf.dt) ||
	    !frames_match(pf, pf2) || !cframes_match(cf, cf2)) {
		char buf[80];
		std::snprintf(buf, sizeof buf, "y=%d", y);
		report_fail(st_easterg, buf);
	}
}

void
chk_easterog(int y)
{
	DateFrame pf, pf2;
	CDateFrame cf, cf2;
	date_init(pf, 0, 0, 0);
	date_init(pf2, 0, 0, 0);
	cdate_init(cf, 0, 0, 0);
	cdate_init(cf2, 0, 0, 0);

	P::date *pr = P::easterog(y, &pf.dt);
	date *cr = ref_easterog(y, &cf.dt);

	st_easterog.cases++;
	if ((pr - &pf.dt) != (cr - &cf.dt) ||
	    !frames_match(pf, pf2) || !cframes_match(cf, cf2)) {
		char buf[80];
		std::snprintf(buf, sizeof buf, "y=%d", y);
		report_fail(st_easterog, buf);
	}
}

void
chk_easteroj(int y)
{
	DateFrame pf, pf2;
	CDateFrame cf, cf2;
	date_init(pf, 0, 0, 0);
	date_init(pf2, 0, 0, 0);
	cdate_init(cf, 0, 0, 0);
	cdate_init(cf2, 0, 0, 0);

	P::date *pr = P::easteroj(y, &pf.dt);
	date *cr = ref_easteroj(y, &cf.dt);

	st_easteroj.cases++;
	if ((pr - &pf.dt) != (cr - &cf.dt) ||
	    !frames_match(pf, pf2) || !cframes_match(cf, cf2)) {
		char buf[80];
		std::snprintf(buf, sizeof buf, "y=%d", y);
		report_fail(st_easteroj, buf);
	}
}

void
chk_easterodn(int y)
{
	int pr = P::easterodn(y);
	int cr = ref_easterodn(y);

	st_easterodn.cases++;
	if (pr != cr) {
		char buf[80];
		std::snprintf(buf, sizeof buf, "y=%d", y);
		report_fail(st_easterodn, buf);
	}
}

void
edge_cases(void)
{
	static const int ndays_edges[] = {
		-100000, -1, 0, 1, 2, 3, 4, 10, 30, 59, 60, 61, 90, 91,
		120, 121, 151, 181, 212, 243, 273, 304, 334, 365, 366,
		577700, 577737, 577738, 577739, 577740, 577750, 577760,
		730000, 1000000, 2000000, 0x7fffffff
	};
	static const int years[] = {
		-100, -1, 0, 1, 2, 3, 4, 100, 400, 999, 1000, 1582, 1583,
		1600, 1700, 1800, 1900, 1997, 1999, 2000, 2001, 2024, 2100,
		2400, 4000, 10000
	};
	static const int dates[][3] = {
		{0, 1, 1}, {0, 2, 29}, {0, 3, 1}, {1, 1, 1}, {1, 12, 31},
		{4, 2, 29}, {100, 2, 28}, {400, 2, 29}, {1582, 10, 4},
		{1582, 10, 5}, {1582, 10, 14}, {1582, 10, 15}, {1582, 10, 31},
		{1600, 2, 29}, {1600, 3, 1}, {1700, 2, 28}, {1900, 2, 28},
		{2000, 2, 29}, {2024, 2, 29}, {2024, 12, 31},
		{-1, 1, 1}, {0, 0, 1}, {1, 0, 1}, {1, 13, 1}, {1, 1, 0},
		{1, 1, 32}, {100, 13, 5}, {100, 1, 0}
	};
	static const int idts[][3] = {
		{0, 0, 0}, {0, 9, 30}, {0, 10, 0}, {0, 11, 30},
		{1582, 7, 3}, {1582, 7, 4}, {1582, 7, 13}, {1582, 7, 14},
		{1599, 11, 30}, {1600, 0, 0}, {1600, 1, 28}, {1600, 2, 0},
		{-1, 11, 30}, {0, -1, 0}, {0, 12, 0}, {100, 11, 31}
	};

	for (int nd : ndays_edges) {
		chk_jdate(nd);
		chk_gdate(nd);
		chk_weekday(nd);
	}
	for (int y : years) {
		chk_firstweek(y);
		chk_easterg(y);
		chk_easterog(y);
		chk_easteroj(y);
		chk_easterodn(y);
	}
	for (auto &d : dates) {
		chk_ndaysj(d[0], d[1], d[2]);
		chk_ndaysg(d[0], d[1], d[2]);
		chk_date2idt(d[0], d[1], d[2]);
	}
	for (auto &idt : idts) {
		chk_ndaysji(idt[0], idt[1], idt[2]);
		chk_ndaysgi(idt[0], idt[1], idt[2]);
		chk_idt2date(idt[0], idt[1], idt[2]);
	}
	for (int nd : ndays_edges)
		chk_week(nd);

	/* Metonic cycle boundaries for easterodn */
	for (int y = 0; y < 19; y++) {
		chk_easterodn(y);
		chk_easterodn(y + 19);
		chk_easterodn(y + 1900);
	}

	/* weekday negative-modulo branch: values just below nmonday */
	for (int i = 0; i < 20; i++)
		chk_weekday(i - 1000000);

	/* firstweek wd>3 vs wd<=3: years around ISO week boundaries */
	for (int y = 1970; y <= 1980; y++)
		chk_firstweek(y);
	for (int y = 1999; y <= 2005; y++)
		chk_firstweek(y);

	/* ndaysgi branches: y>=1600, nd>nswitch, else */
	chk_ndaysgi(1599, 11, 30);
	chk_ndaysgi(1600, 0, 0);
	chk_ndaysgi(1600, 11, 30);
	chk_ndaysgi(2000, 1, 28);
	chk_ndaysgi(1900, 1, 27);

	/* gdate 1582 switch: month1s table and Oct +10 */
	for (int nd = 577730; nd <= 577745; nd++)
		chk_gdate(nd);

	/* easterg algorithm boundaries */
	for (int y = 1583; y <= 1700; y += 17)
		chk_easterg(y);
	for (int y = 2100; y <= 2200; y += 13)
		chk_easterg(y);
}

void
random_jdate(void)
{
	for (int i = 0; i < kRandomPerFn; i++)
		chk_jdate(randrange(-500000, 3000000));
}

void
random_gdate(void)
{
	for (int i = 0; i < kRandomPerFn; i++)
		chk_gdate(randrange(-500000, 3000000));
}

void
random_ndaysj(void)
{
	for (int i = 0; i < kRandomPerFn; i++) {
		int y = randrange(-5, 5000);
		int m = randrange(-2, 14);
		int d = randrange(-2, 32);
		chk_ndaysj(y, m, d);
	}
}

void
random_ndaysg(void)
{
	for (int i = 0; i < kRandomPerFn; i++) {
		int y = randrange(-5, 5000);
		int m = randrange(-2, 14);
		int d = randrange(-2, 32);
		chk_ndaysg(y, m, d);
	}
}

void
random_ndaysji(void)
{
	for (int i = 0; i < kRandomPerFn; i++) {
		int y = randrange(-5, 5000);
		int m = randrange(-2, 14);
		int d = randrange(-2, 32);
		chk_ndaysji(y, m, d);
	}
}

void
random_ndaysgi(void)
{
	for (int i = 0; i < kRandomPerFn; i++) {
		int y = randrange(-5, 5000);
		int m = randrange(-2, 14);
		int d = randrange(-2, 32);
		chk_ndaysgi(y, m, d);
	}
}

void
random_date2idt(void)
{
	for (int i = 0; i < kRandomPerFn; i++) {
		int y = randrange(-5, 5000);
		int m = randrange(-2, 14);
		int d = randrange(-2, 32);
		chk_date2idt(y, m, d);
	}
}

void
random_idt2date(void)
{
	for (int i = 0; i < kRandomPerFn; i++) {
		int y = randrange(-5, 5000);
		int m = randrange(-2, 14);
		int d = randrange(-2, 32);
		chk_idt2date(y, m, d);
	}
}

void
random_weekday(void)
{
	for (int i = 0; i < kRandomPerFn; i++)
		chk_weekday(randrange(-2000000, 3000000));
}

void
random_firstweek(void)
{
	for (int i = 0; i < kRandomPerFn; i++)
		chk_firstweek(randrange(-100, 10000));
}

void
random_week(void)
{
	for (int i = 0; i < kRandomPerFn; i++)
		chk_week(randrange(-500000, 3000000));
}

void
random_easterg(void)
{
	for (int i = 0; i < kRandomPerFn; i++)
		chk_easterg(randrange(-500, 10000));
}

void
random_easterog(void)
{
	for (int i = 0; i < kRandomPerFn; i++)
		chk_easterog(randrange(-500, 10000));
}

void
random_easteroj(void)
{
	for (int i = 0; i < kRandomPerFn; i++)
		chk_easteroj(randrange(-500, 10000));
}

void
random_easterodn(void)
{
	for (int i = 0; i < kRandomPerFn; i++)
		chk_easterodn(randrange(-500, 10000));
}

} /* namespace */

int
main(void)
{
	edge_cases();
	random_jdate();
	random_gdate();
	random_ndaysj();
	random_ndaysg();
	random_ndaysji();
	random_ndaysgi();
	random_date2idt();
	random_idt2date();
	random_weekday();
	random_firstweek();
	random_week();
	random_easterg();
	random_easterog();
	random_easteroj();
	random_easterodn();

	std::uint64_t total_cases = 0;
	std::uint64_t total_fails = 0;

	std::printf("\n%-12s  %12s  %12s\n", "function", "cases", "failures");
	std::printf("%-12s  %12s  %12s\n", "--------", "-----", "--------");
	for (int i = 0; i < nstats; i++) {
		std::printf("%-12s  %12llu  %12llu\n",
		    stats[i].name,
		    (unsigned long long)stats[i].cases,
		    (unsigned long long)stats[i].fails);
		total_cases += stats[i].cases;
		total_fails += stats[i].fails;
	}
	std::printf("%-12s  %12llu  %12llu\n", "TOTAL",
	    (unsigned long long)total_cases,
	    (unsigned long long)total_fails);

	return total_fails == 0 ? 0 : 1;
}
