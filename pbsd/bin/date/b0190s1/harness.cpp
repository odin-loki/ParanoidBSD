/*
 * harness.cpp -- differential test for PBSD batch b0190s1 (vary.c).
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

import pbsd.bin.date.b0190s1;

namespace P = pbsd::bin_date::b0190s1;

extern "C" {
struct trans {
	int64_t val;
	const char *str;
};

struct vary {
	char *arg;
	struct vary *next;
};

extern struct trans trans_mon[];
extern struct trans trans_wday[];
int ref_domktime(struct tm *, char);
int ref_trans(const struct trans[], const char *);
struct vary *ref_vary_append(struct vary *, char *);
int ref_daysinmonth(const struct tm *);
int ref_adjyear(struct tm *, char, int64_t, int);
int ref_adjmon(struct tm *, char, int64_t, int, int);
int ref_adjday(struct tm *, char, int64_t, int);
int ref_adjwday(struct tm *, char, int64_t, int, int);
int ref_adjhour(struct tm *, char, int64_t, int);
int ref_adjmin(struct tm *, char, int64_t, int);
int ref_adjsec(struct tm *, char, int64_t, int);
const struct vary *ref_vary_apply(const struct vary *, struct tm *);
void ref_vary_destroy(struct vary *);
}

namespace {

constexpr long SWEEP = 200000L;
constexpr int MAX_SHOW = 8;
constexpr unsigned char GUARD = 0x7f;
constexpr int BUF_SZ = 256;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int shown;
};

struct Rng {
	std::uint64_t s;

	explicit Rng(std::uint64_t seed) : s(seed) {}

	std::uint64_t next()
	{
		s += 0x9E3779B97F4A7C15ull;
		std::uint64_t z = s;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}

	int bits(int lo, int hi)
	{
		if (hi <= lo)
			return lo;
		return lo + (int)(next() % (std::uint64_t)(hi - lo + 1));
	}

	unsigned char byte()
	{
		return (unsigned char)(next() & 0xffu);
	}

	bool coin()
	{
		return (next() & 1u) != 0;
	}
};

Rng rng(0xB0190D1FACEULL);

Stat stats[16];
int nstats;

Stat &
reg(const char *name)
{
	stats[nstats++] = { name, 0, 0, 0 };
	return stats[nstats - 1];
}

bool
fail(Stat &st, const char *what)
{
	st.fails++;
	if (st.shown < MAX_SHOW) {
		st.shown++;
		std::printf("  FAIL %s: %s\n", st.name, what);
	}
	return false;
}

void
fill_tm(struct tm *t, int sec, int min, int hour, int mday, int mon, int year,
    int wday, int isdst)
{
	std::memset(t, 0, sizeof(*t));
	t->tm_sec = sec;
	t->tm_min = min;
	t->tm_hour = hour;
	t->tm_mday = mday;
	t->tm_mon = mon;
	t->tm_year = year;
	t->tm_wday = wday;
	t->tm_yday = 0;
	t->tm_isdst = isdst;
}

void
random_tm(struct tm *t)
{
	fill_tm(t, rng.bits(0, 61), rng.bits(0, 59), rng.bits(0, 23),
	    rng.bits(1, 28), rng.bits(0, 11), rng.bits(68, 137), rng.bits(0, 6),
	    rng.coin() ? rng.bits(0, 1) : -1);
}

bool
tm_equal(const struct tm *a, const struct tm *b)
{
	return a->tm_sec == b->tm_sec && a->tm_min == b->tm_min &&
	    a->tm_hour == b->tm_hour && a->tm_mday == b->tm_mday &&
	    a->tm_mon == b->tm_mon && a->tm_year == b->tm_year &&
	    a->tm_wday == b->tm_wday && a->tm_yday == b->tm_yday &&
	    a->tm_isdst == b->tm_isdst;
}

char
random_type()
{
	switch (rng.bits(0, 4)) {
	case 0:
		return '+';
	case 1:
		return '-';
	case 2:
		return '\0';
	default: {
		unsigned char c = rng.byte();
		if (c == '+' || c == '-')
			return (char)c;
		return (char)(32 + (c % 90));
	}
	}
}

int
vary_depth(const struct vary *v)
{
	int n = 0;
	for (; v; v = v->next)
		n++;
	return n;
}

long
vary_off(const struct vary *head, const struct vary *p)
{
	long i = 0;

	if (p == nullptr)
		return -1;
	for (; head; head = head->next, i++) {
		if (head == p)
			return i;
	}
	return -2;
}

long
vary_off(const P::vary *head, const P::vary *p)
{
	long i = 0;

	if (p == nullptr)
		return -1;
	for (; head; head = head->next, i++) {
		if (head == p)
			return i;
	}
	return -2;
}

bool
vary_same(const struct vary *a, const P::vary *b)
{
	for (; a && b; a = a->next, b = b->next) {
		if (a->arg != b->arg && std::strcmp(a->arg, b->arg) != 0)
			return false;
	}
	return a == nullptr && b == nullptr;
}

void
test_trans()
{
	Stat &st = reg("trans");

	auto run = [&](const struct trans *tbl, auto ptbl, const char *arg) {
		st.cases++;
		int r = ref_trans(tbl, arg);
		int p = P::trans(ptbl, arg);
		if (r != p)
			fail(st, arg);
	};

	run(trans_mon, P::trans_mon_table(), "january");
	run(trans_mon, P::trans_mon_table(), "jan");
	run(trans_mon, P::trans_mon_table(), "JaN");
	run(trans_mon, P::trans_mon_table(), "february");
	run(trans_mon, P::trans_mon_table(), "feb");
	run(trans_mon, P::trans_mon_table(), "bogus");
	run(trans_mon, P::trans_mon_table(), "");
	run(trans_mon, P::trans_mon_table(), "a");
	run(trans_wday, P::trans_wday_table(), "sunday");
	run(trans_wday, P::trans_wday_table(), "sun");
	run(trans_wday, P::trans_wday_table(), "monday");
	run(trans_wday, P::trans_wday_table(), "MON");
	run(trans_wday, P::trans_wday_table(), "xyz");
	run(trans_wday, P::trans_wday_table(), "\xff\xff\xff");

	const char *mons[] = { "january", "february", "march", "april", "may",
		"june", "july", "august", "september", "october", "november",
		"december" };
	const char *days[] = { "sunday", "monday", "tuesday", "wednesday",
		"thursday", "friday", "saturday" };

	for (long i = 0; i < SWEEP / 13; i++) {
		char buf[32];
		int n = rng.bits(0, 12);
		if (n < 8) {
			const char *m = mons[rng.bits(0, 11)];
			if (rng.coin())
				std::strcpy(buf, m);
			else
				std::snprintf(buf, sizeof(buf), "%.3s", m);
		} else if (n < 12) {
			const char *d = days[rng.bits(0, 6)];
			if (rng.coin())
				std::strcpy(buf, d);
			else
				std::snprintf(buf, sizeof(buf), "%.3s", d);
		} else {
			int len = rng.bits(0, 10);
			for (int j = 0; j < len; j++)
				buf[j] = (char)rng.byte();
			buf[len] = '\0';
		}
		if (rng.coin())
			run(trans_mon, P::trans_mon_table(), buf);
		else
			run(trans_wday, P::trans_wday_table(), buf);
	}
}

void
test_daysinmonth()
{
	Stat &st = reg("daysinmonth");

	auto run = [&](int mon, int year) {
		struct tm rt, pt;
		fill_tm(&rt, 0, 0, 0, 1, mon, year - 1900, 0, -1);
		fill_tm(&pt, 0, 0, 0, 1, mon, year - 1900, 0, -1);
		st.cases++;
		int r = ref_daysinmonth(&rt);
		int p = P::daysinmonth(&pt);
		if (r != p)
			fail(st, "mismatch");
	};

	run(0, 2020);
	run(1, 2000);
	run(1, 1900);
	run(1, 2004);
	run(1, 2001);
	run(2, 2020);
	run(11, 2020);
	run(-1, 2020);
	run(12, 2020);
	run(1, 2100);

	for (long i = 0; i < SWEEP / 13; i++)
		run(rng.bits(-2, 14), rng.bits(1600, 2200));
}

void
test_domktime()
{
	Stat &st = reg("domktime");

	auto run = [&](const struct tm *seed, char type) {
		struct tm rt = *seed, pt = *seed;
		st.cases++;
		int r = ref_domktime(&rt, type);
		int p = P::domktime(&pt, type);
		if (r != p || !tm_equal(&rt, &pt))
			fail(st, "mismatch");
	};

	struct tm base;
	fill_tm(&base, 30, 15, 12, 15, 5, 120, 3, -1);
	run(&base, '+');
	run(&base, '-');
	run(&base, 'y');

	fill_tm(&base, 0, 0, 2, 29, 1, 100, 0, -1);
	run(&base, '+');
	run(&base, '-');

	for (long i = 0; i < SWEEP / 13; i++) {
		random_tm(&base);
		run(&base, random_type());
	}
}

void
test_adjyear()
{
	Stat &st = reg("adjyear");

	auto run = [&](struct tm seed, char type, int64_t val, int mk) {
		struct tm rt = seed, pt = seed;
		st.cases++;
		int r = ref_adjyear(&rt, type, val, mk);
		int p = P::adjyear(&pt, type, val, mk);
		if (r != p || !tm_equal(&rt, &pt))
			fail(st, "mismatch");
	};

	struct tm base;
	fill_tm(&base, 0, 0, 12, 15, 5, 120, 3, -1);
	run(base, '+', 1, 0);
	run(base, '-', 2, 0);
	run(base, 'y', 2020, 0);
	run(base, 'y', 68, 0);
	run(base, 'y', 69, 0);
	run(base, 'y', 2000, 0);
	run(base, '+', 5, 1);
	run(base, '-', 3, 1);

	for (long i = 0; i < SWEEP / 13; i++) {
		random_tm(&base);
		run(base, random_type(), (int64_t)rng.bits(-5, 200), rng.coin());
	}
}

void
test_adjmon()
{
	Stat &st = reg("adjmon");

	auto run = [&](struct tm seed, char type, int64_t val, int istext, int mk) {
		struct tm rt = seed, pt = seed;
		st.cases++;
		int r = ref_adjmon(&rt, type, val, istext, mk);
		int p = P::adjmon(&pt, type, val, istext, mk);
		if (r != p || !tm_equal(&rt, &pt))
			fail(st, "mismatch");
	};

	struct tm base;
	fill_tm(&base, 0, 0, 12, 31, 2, 120, 3, -1);
	run(base, '+', 1, 0, 0);
	run(base, '+', 1, 1, 0);
	run(base, '-', 1, 0, 0);
	run(base, '-', 1, 1, 0);
	run(base, 'm', 3, 0, 0);
	run(base, 'm', 0, 0, 0);
	run(base, 'm', 13, 0, 0);
	run(base, '+', 6, 0, 1);
	run(base, '+', 6, 1, 1);

	for (long i = 0; i < SWEEP / 13; i++) {
		random_tm(&base);
		base.tm_mday = rng.bits(1, 31);
		run(base, random_type(), (int64_t)rng.bits(-3, 24),
		    rng.coin(), rng.coin());
	}
}

void
test_adjday()
{
	Stat &st = reg("adjday");

	auto run = [&](struct tm seed, char type, int64_t val, int mk) {
		struct tm rt = seed, pt = seed;
		st.cases++;
		int r = ref_adjday(&rt, type, val, mk);
		int p = P::adjday(&pt, type, val, mk);
		if (r != p || !tm_equal(&rt, &pt))
			fail(st, "mismatch");
	};

	struct tm base;
	fill_tm(&base, 0, 0, 12, 15, 5, 120, 3, -1);
	run(base, '+', 1, 0);
	run(base, '+', 30, 0);
	run(base, '-', 1, 0);
	run(base, '-', 20, 0);
	run(base, 'd', 1, 0);
	run(base, 'd', 32, 0);
	run(base, '+', 100, 1);

	for (long i = 0; i < SWEEP / 13; i++) {
		random_tm(&base);
		base.tm_mday = rng.bits(1, 28);
		run(base, random_type(), (int64_t)rng.bits(-5, 400), rng.coin());
	}
}

void
test_adjwday()
{
	Stat &st = reg("adjwday");

	auto run = [&](struct tm seed, char type, int64_t val, int istext, int mk) {
		struct tm rt = seed, pt = seed;
		st.cases++;
		int r = ref_adjwday(&rt, type, val, istext, mk);
		int p = P::adjwday(&pt, type, val, istext, mk);
		if (r != p || !tm_equal(&rt, &pt))
			fail(st, "mismatch");
	};

	struct tm base;
	fill_tm(&base, 0, 0, 12, 15, 5, 120, 3, -1);
	run(base, '+', 1, 0, 0);
	run(base, '+', 1, 1, 0);
	run(base, '-', 1, 0, 0);
	run(base, '-', 1, 1, 0);
	run(base, 'w', 3, 0, 0);
	run(base, 'w', 7, 0, 0);
	run(base, '+', 2, 0, 1);
	run(base, '-', 3, 1, 1);

	for (long i = 0; i < SWEEP / 13; i++) {
		random_tm(&base);
		base.tm_wday = rng.bits(0, 6);
		run(base, random_type(), (int64_t)rng.bits(-3, 10), rng.coin(),
		    rng.coin());
	}
}

void
test_adjhour()
{
	Stat &st = reg("adjhour");

	auto run = [&](struct tm seed, char type, int64_t val, int mk) {
		struct tm rt = seed, pt = seed;
		st.cases++;
		int r = ref_adjhour(&rt, type, val, mk);
		int p = P::adjhour(&pt, type, val, mk);
		if (r != p || !tm_equal(&rt, &pt))
			fail(st, "mismatch");
	};

	struct tm base;
	fill_tm(&base, 0, 30, 12, 15, 5, 120, 3, -1);
	run(base, '+', 1, 0);
	run(base, '+', 25, 0);
	run(base, '-', 1, 0);
	run(base, '-', 13, 0);
	run(base, 'H', 12, 0);
	run(base, 'H', 24, 0);
	run(base, '+', 48, 1);

	for (long i = 0; i < SWEEP / 13; i++) {
		random_tm(&base);
		run(base, random_type(), (int64_t)rng.bits(-5, 200), rng.coin());
	}
}

void
test_adjmin()
{
	Stat &st = reg("adjmin");

	auto run = [&](struct tm seed, char type, int64_t val, int mk) {
		struct tm rt = seed, pt = seed;
		st.cases++;
		int r = ref_adjmin(&rt, type, val, mk);
		int p = P::adjmin(&pt, type, val, mk);
		if (r != p || !tm_equal(&rt, &pt))
			fail(st, "mismatch");
	};

	struct tm base;
	fill_tm(&base, 30, 30, 12, 15, 5, 120, 3, -1);
	run(base, '+', 1, 0);
	run(base, '+', 90, 0);
	run(base, '-', 1, 0);
	run(base, '-', 45, 0);
	run(base, 'M', 30, 0);
	run(base, 'M', 60, 0);
	run(base, '+', 120, 1);

	for (long i = 0; i < SWEEP / 13; i++) {
		random_tm(&base);
		run(base, random_type(), (int64_t)rng.bits(-5, 300), rng.coin());
	}
}

void
test_adjsec()
{
	Stat &st = reg("adjsec");

	auto run = [&](struct tm seed, char type, int64_t val, int mk) {
		struct tm rt = seed, pt = seed;
		st.cases++;
		int r = ref_adjsec(&rt, type, val, mk);
		int p = P::adjsec(&pt, type, val, mk);
		if (r != p || !tm_equal(&rt, &pt))
			fail(st, "mismatch");
	};

	struct tm base;
	fill_tm(&base, 30, 30, 12, 15, 5, 120, 3, -1);
	run(base, '+', 1, 0);
	run(base, '+', 90, 0);
	run(base, '-', 1, 0);
	run(base, '-', 45, 0);
	run(base, 'S', 30, 0);
	run(base, 'S', 60, 0);
	run(base, '+', 120, 1);

	for (long i = 0; i < SWEEP / 13; i++) {
		random_tm(&base);
		run(base, random_type(), (int64_t)rng.bits(-5, 300), rng.coin());
	}
}

void
test_vary_append()
{
	Stat &st = reg("vary_append");

	auto run = [&](const char *const *args, int n) {
		struct vary *rv = nullptr;
		P::vary *pv = nullptr;
		st.cases++;
		for (int i = 0; i < n; i++) {
			rv = ref_vary_append(rv, (char *)args[i]);
			pv = P::vary_append(pv, (char *)args[i]);
		}
		if (vary_depth(rv) != vary_depth((const struct vary *)pv) ||
		    !vary_same(rv, pv))
			fail(st, "structure");
		ref_vary_destroy(rv);
		P::vary_destroy(pv);
	};

	const char *a1[] = { "+1d" };
	const char *a2[] = { "+1d", "-2m", "monday" };
	const char *a3[] = { "1y", "+3H", "january", "-1w", "5S" };
	run(a1, 0);
	run(a1, 1);
	run(a2, 2);
	run(a3, 5);

	char bufs[8][32];
	const char *ptrs[8];
	for (long i = 0; i < SWEEP / 13; i++) {
		int n = rng.bits(0, 7);
		for (int j = 0; j < n; j++) {
			static const char *suffix = "yHMSdmw";
			int kind = rng.bits(0, 6);
			if (kind < 3) {
				std::snprintf(bufs[j], sizeof(bufs[j]), "%c%d%c",
				    rng.coin() ? '+' : '-', rng.bits(0, 100),
				    suffix[rng.bits(0, 6)]);
			} else if (kind < 5) {
				const char *mons[] = { "jan", "feb", "mar",
					"apr", "may", "jun", "jul", "aug",
					"sep", "oct", "nov", "dec" };
				std::strcpy(bufs[j], mons[rng.bits(0, 11)]);
			} else {
				const char *days[] = { "sun", "mon", "tue",
					"wed", "thu", "fri", "sat" };
				std::strcpy(bufs[j], days[rng.bits(0, 6)]);
			}
			ptrs[j] = bufs[j];
		}
		run(ptrs, n);
	}
}

void
test_vary_apply()
{
	Stat &st = reg("vary_apply");

	auto run = [&](const char *const *args, int n, struct tm seed) {
		struct vary *rv = nullptr;
		P::vary *pv = nullptr;
		for (int i = 0; i < n; i++) {
			rv = ref_vary_append(rv, (char *)args[i]);
			pv = P::vary_append(pv, (char *)args[i]);
		}
		struct tm rt = seed, pt = seed;
		st.cases++;
		const struct vary *rbad = ref_vary_apply(rv, &rt);
		const P::vary *pbad = P::vary_apply(pv, &pt);
		long roff = vary_off(rv, rbad);
		long poff = vary_off(pv, pbad);
		if (roff != poff || !tm_equal(&rt, &pt))
			fail(st, "mismatch");
		ref_vary_destroy(rv);
		P::vary_destroy(pv);
	};

	struct tm base;
	fill_tm(&base, 30, 15, 12, 15, 5, 120, 3, -1);

	const char *e1[] = { "+1d" };
	const char *e2[] = { "-1m" };
	const char *e3[] = { "monday" };
	const char *e4[] = { "january" };
	const char *e5[] = { "+3H", "+30M", "+5S" };
	const char *e6[] = { "+2w" };
	const char *e7[] = { "1y" };
	const char *e8[] = { "x" };
	const char *e9[] = { "+" };
	const char *e10[] = { "+1x" };
	const char *e11[] = { "+1d", "bad" };
	run(e1, 1, base);
	run(e2, 1, base);
	run(e3, 1, base);
	run(e4, 1, base);
	run(e5, 3, base);
	run(e6, 1, base);
	run(e7, 1, base);
	run(e8, 1, base);
	run(e9, 1, base);
	run(e10, 1, base);
	run(e11, 2, base);
	run(nullptr, 0, base);

	char bufs[6][32];
	const char *ptrs[6];
	for (long i = 0; i < SWEEP / 13; i++) {
		random_tm(&base);
		int n = rng.bits(0, 5);
		for (int j = 0; j < n; j++) {
			int kind = rng.bits(0, 8);
			if (kind == 0)
				std::snprintf(bufs[j], sizeof(bufs[j]), "%c%dd",
				    rng.coin() ? '+' : '-', rng.bits(0, 50));
			else if (kind == 1)
				std::snprintf(bufs[j], sizeof(bufs[j]), "%c%dm",
				    rng.coin() ? '+' : '-', rng.bits(1, 12));
			else if (kind == 2)
				std::snprintf(bufs[j], sizeof(bufs[j]), "%c%dw",
				    rng.coin() ? '+' : '-', rng.bits(0, 10));
			else if (kind == 3)
				std::snprintf(bufs[j], sizeof(bufs[j]), "%c%dH",
				    rng.coin() ? '+' : '-', rng.bits(0, 48));
			else if (kind == 4)
				std::snprintf(bufs[j], sizeof(bufs[j]), "%dS",
				    rng.bits(0, 59));
			else if (kind == 5) {
				const char *mons[] = { "jan", "feb", "mar",
					"apr", "may", "jun" };
				std::strcpy(bufs[j], mons[rng.bits(0, 5)]);
			} else if (kind == 6) {
				const char *days[] = { "sun", "mon", "tue",
					"wed", "thu", "fri", "sat" };
				std::strcpy(bufs[j], days[rng.bits(0, 6)]);
			} else if (kind == 7)
				std::snprintf(bufs[j], sizeof(bufs[j]), "%d",
				    rng.bits(0, 9));
			else
				bufs[j][0] = '\0';
			ptrs[j] = bufs[j];
		}
		run(ptrs, n, base);
	}
}

void
test_vary_destroy()
{
	Stat &st = reg("vary_destroy");

	auto run = [&](int n) {
		struct vary *rv = nullptr;
		P::vary *pv = nullptr;
		char bufs[8][16];
		for (int i = 0; i < n; i++) {
			std::snprintf(bufs[i], sizeof(bufs[i]), "+%dd", i + 1);
			rv = ref_vary_append(rv, bufs[i]);
			pv = P::vary_append(pv, bufs[i]);
		}
		st.cases++;
		int rd = vary_depth(rv);
		int pd = vary_depth((const struct vary *)pv);
		if (rd != n || pd != n || rd != pd)
			fail(st, "pre-destroy depth");
		ref_vary_destroy(rv);
		P::vary_destroy(pv);
	};

	run(0);
	run(1);
	run(3);
	run(8);

	for (long i = 0; i < SWEEP / 13; i++)
		run(rng.bits(0, 8));
}

} // namespace

int
main()
{
	setenv("TZ", "UTC0", 1);
	tzset();

	test_trans();
	test_daysinmonth();
	test_domktime();
	test_adjyear();
	test_adjmon();
	test_adjday();
	test_adjwday();
	test_adjhour();
	test_adjmin();
	test_adjsec();
	test_vary_append();
	test_vary_apply();
	test_vary_destroy();

	long total_cases = 0;
	long total_fails = 0;

	std::printf("\n%-16s %10s %10s\n", "function", "cases", "failures");
	std::printf("%-16s %10s %10s\n", "--------", "-----", "--------");
	for (int i = 0; i < nstats; i++) {
		std::printf("%-16s %10ld %10ld\n", stats[i].name, stats[i].cases,
		    stats[i].fails);
		total_cases += stats[i].cases;
		total_fails += stats[i].fails;
	}
	std::printf("%-16s %10ld %10ld\n", "TOTAL", total_cases, total_fails);

	return total_fails == 0 ? 0 : 1;
}
