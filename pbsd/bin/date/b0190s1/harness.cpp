/*
 * PBSD batch b0190s1 differential harness: bin/date/vary.c.
 *
 * Every exported function of the port is driven against the ref_ oracle in
 * oracle.c.  The struct tm each side works on lives inside a byte buffer that
 * is pre-filled with the guard byte 0x7f and compared in its entirety after
 * the call -- padding, tm_gmtoff, tm_zone and the 32 guard bytes past the end
 * of the struct included -- so a port that writes the right answer into the
 * wrong place still fails.  Pointer results are compared as list indices
 * rather than as addresses.
 */

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <malloc.h>

import pbsd.bin.date.b0190s1;

namespace P = pbsd::bin_date::b0190s1;

extern "C" {

struct vary {
	char *arg;
	struct vary *next;
};

struct ref_trans {
	std::int64_t val;
	const char *str;
};

extern struct ref_trans ref_trans_mon[];
extern struct ref_trans ref_trans_wday[];
extern char ref_digits[];
extern int ref_mdays[12];

int ref_domktime(struct tm *, char);
int ref_trans(const struct ref_trans *, const char *);
struct vary *ref_vary_append(struct vary *, char *);
int ref_daysinmonth(const struct tm *);
int ref_adjyear(struct tm *, char, std::int64_t, int);
int ref_adjmon(struct tm *, char, std::int64_t, int, int);
int ref_adjday(struct tm *, char, std::int64_t, int);
int ref_adjwday(struct tm *, char, std::int64_t, int, int);
int ref_adjhour(struct tm *, char, std::int64_t, int);
int ref_adjmin(struct tm *, char, std::int64_t, int);
int ref_adjsec(struct tm *, char, std::int64_t, int);
const struct vary *ref_vary_apply(const struct vary *, struct tm *);
void ref_vary_destroy(struct vary *);

}

/* ------------------------------------------------------------------ */
/* Bookkeeping                                                         */
/* ------------------------------------------------------------------ */

enum {
	S_DAYSINMONTH, S_DOMKTIME, S_TRANS, S_ADJYEAR, S_ADJMON, S_ADJDAY,
	S_ADJWDAY, S_ADJHOUR, S_ADJMIN, S_ADJSEC, S_VARY_APPEND, S_VARY_APPLY,
	S_VARY_DESTROY, S_NSTATS
};

struct Stat {
	const char *name;
	long long cases;
	long long fails;
	int shown;
};

static Stat stats[S_NSTATS] = {
	{ "daysinmonth",  0, 0, 0 },
	{ "domktime",     0, 0, 0 },
	{ "trans",        0, 0, 0 },
	{ "adjyear",      0, 0, 0 },
	{ "adjmon",       0, 0, 0 },
	{ "adjday",       0, 0, 0 },
	{ "adjwday",      0, 0, 0 },
	{ "adjhour",      0, 0, 0 },
	{ "adjmin",       0, 0, 0 },
	{ "adjsec",       0, 0, 0 },
	{ "vary_append",  0, 0, 0 },
	{ "vary_apply",   0, 0, 0 },
	{ "vary_destroy", 0, 0, 0 },
};

/* Reporting is deliberately off the hot path: callers bump `cases'
 * themselves and only build a message when the case actually diverged. */
static void
report(int which, const char *fmt, ...)
{
	Stat &s = stats[which];

	s.fails++;
	if (s.shown < 10) {
		va_list ap;

		s.shown++;
		std::fprintf(stderr, "FAIL %s: ", s.name);
		va_start(ap, fmt);
		std::vfprintf(stderr, fmt, ap);
		va_end(ap);
		std::fputc('\n', stderr);
	}
}

/* ------------------------------------------------------------------ */
/* struct tm buffers                                                   */
/* ------------------------------------------------------------------ */

static const std::size_t TM_GUARD = 32;
static const std::size_t TM_RAW = sizeof(struct tm) + TM_GUARD;

struct TmBuf {
	alignas(alignof(struct tm)) unsigned char raw[TM_RAW];

	struct tm *t() { return reinterpret_cast<struct tm *>(raw); }
	const struct tm *t() const
	{
		return reinterpret_cast<const struct tm *>(raw);
	}
};

struct TmSpec {
	int sec, min, hour, mday, mon, year, wday, yday, isdst;
};

static void
tm_init(TmBuf &b, const TmSpec &s)
{
	std::memset(b.raw, 0x7f, sizeof b.raw);

	struct tm *t = b.t();
	t->tm_sec = s.sec;
	t->tm_min = s.min;
	t->tm_hour = s.hour;
	t->tm_mday = s.mday;
	t->tm_mon = s.mon;
	t->tm_year = s.year;
	t->tm_wday = s.wday;
	t->tm_yday = s.yday;
	t->tm_isdst = s.isdst;
#ifdef __GLIBC__
	t->tm_gmtoff = 0;
	t->tm_zone = nullptr;
#endif
}

static bool
tm_same(const TmBuf &a, const TmBuf &b)
{
	return std::memcmp(a.raw, b.raw, TM_RAW) == 0;
}

static const char *
tm_show(const TmBuf &b, char *out, std::size_t n)
{
	const struct tm *t = b.t();

	std::snprintf(out, n, "%d-%d-%d %d:%d:%d wd=%d yd=%d dst=%d",
	    t->tm_year, t->tm_mon, t->tm_mday, t->tm_hour, t->tm_min,
	    t->tm_sec, t->tm_wday, t->tm_yday, t->tm_isdst);
	return out;
}

static const char *
spec_show(const TmSpec &s, char *out, std::size_t n)
{
	std::snprintf(out, n, "{%d,%d,%d,%d,%d,%d,%d,%d,%d}", s.sec, s.min,
	    s.hour, s.mday, s.mon, s.year, s.wday, s.yday, s.isdst);
	return out;
}

/* ------------------------------------------------------------------ */
/* Fixed-seed RNG                                                      */
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state;

static void
rng_seed(std::uint64_t s)
{
	rng_state = s ? s : 0x9e3779b97f4a7c15ULL;
}

static std::uint64_t
rnd()
{
	std::uint64_t x = rng_state;

	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rng_state = x;
	return x;
}

static std::int64_t
rnd_range(std::int64_t lo, std::int64_t hi)
{
	return lo + (std::int64_t)(rnd() % (std::uint64_t)(hi - lo + 1));
}

/* ------------------------------------------------------------------ */
/* Shared input alphabets                                              */
/* ------------------------------------------------------------------ */

/*
 * '+' and '-' take the two named switch arms; everything else falls to
 * default.  '\0', 0x80 and 0xff are here because `type' is a plain char and
 * the sign of the high-bit values is load-bearing at the switch.
 */
static const char types[] = {
	'+', '-', '\0', 'x', 'm', 'S', ' ', (char)0x80, (char)0xff, '0'
};
static const int ntypes = (int)(sizeof types / sizeof types[0]);

/*
 * Bases 1..3 sit on and around 1969-12-31 18:59:59 EST, which is time_t -1:
 * the only input for which mktime() returns -1 with a plausible tm_year, and
 * therefore the only way into domktime()'s retry loop.
 */
static const TmSpec base_specs[] = {
	{  0,  0,  0,  1,  0,  70, 4,   0,  0 },
	{ 59, 59, 18, 31, 11,  69, 3, 364,  0 },
	{ 58, 59, 18, 31, 11,  69, 3, 364,  0 },
	{ 59, 59, 17, 31, 11,  69, 3, 364, -1 },
	{ 30, 30, 12, 31,  2, 124, 0,  90, -1 },
	{  0,  0,  0, 29,  1, 100, 2,  59,  1 },
	{ 59, 59, 23, 28,  1,   0, 0,  58, -1 },
	{ -3, -5, -2, -1, -1,-101,-1,  -2,  0 },
	{ 61, 70, 25, 40, 14, 300, 8, 400,  1 },
	{  0,  0,  2, 10,  2, 124, 0,  69, -1 },
	{  0,  0,  1,  3, 10, 124, 0, 307, -1 },
	{ 12, 34,  6, 15,  6, 137, 3, 195,  0 },
};
static const int nbase = (int)(sizeof base_specs / sizeof base_specs[0]);

static TmSpec
rnd_spec()
{
	TmSpec s;
	unsigned mode = (unsigned)(rnd() % 5);

	if (mode == 0) {
		/* straddling the epoch, where mktime() can return -1 */
		s.year = (int)rnd_range(68, 71);
		s.mon = (int)rnd_range(-1, 12);
		s.mday = (int)rnd_range(27, 33);
		s.hour = (int)rnd_range(15, 24);
		s.min = (int)rnd_range(54, 60);
		s.sec = (int)rnd_range(54, 60);
	} else if (mode == 1) {
		/* pinned to the exact -1 second and its neighbours */
		s.year = 69;
		s.mon = 11;
		s.mday = 31;
		s.hour = 18;
		s.min = 59;
		s.sec = (int)rnd_range(50, 59);
	} else if (mode == 2) {
		/* the EST5EDT spring-forward gap and fall-back overlap */
		s.year = (int)rnd_range(70, 137);
		s.mon = (int)rnd_range(2, 10);
		s.mday = (int)rnd_range(1, 31);
		s.hour = (int)rnd_range(0, 3);
		s.min = (int)rnd_range(0, 59);
		s.sec = (int)rnd_range(0, 59);
	} else {
		s.year = (int)rnd_range(-120, 300);
		s.mon = (int)rnd_range(-3, 14);
		s.mday = (int)rnd_range(-3, 40);
		s.hour = (int)rnd_range(-3, 27);
		s.min = (int)rnd_range(-5, 65);
		s.sec = (int)rnd_range(-5, 65);
	}
	s.wday = (int)rnd_range(-2, 9);
	s.yday = (int)rnd_range(-2, 370);
	s.isdst = (int)rnd_range(-2, 2);
	return s;
}

static char
rnd_type()
{
	if (rnd() % 4 == 0)
		return (char)(rnd() & 0xff);
	return types[rnd() % (unsigned)ntypes];
}

/*
 * Half the draws come from a small window so the `val == N' boundaries inside
 * each arm are hit densely; the rest span the full range.
 */
static std::int64_t
rnd_val(std::int64_t big)
{
	if (rnd() % 2 == 0)
		return rnd_range(-3, 75);
	return rnd_range(-5, big);
}

static const int RITER = 200000;

/* ------------------------------------------------------------------ */
/* daysinmonth                                                         */
/* ------------------------------------------------------------------ */

static void
one_daysinmonth(const TmSpec &s)
{
	TmBuf a, b;

	tm_init(a, s);
	tm_init(b, s);

	int ra = P::daysinmonth(a.t());
	int rb = ref_daysinmonth(b.t());

	stats[S_DAYSINMONTH].cases++;
	if (ra != rb || !tm_same(a, b)) {
		char sa[128], da[128], db[128];

		report(S_DAYSINMONTH, "%s -> %d/%d  tm %s | %s",
		    spec_show(s, sa, sizeof sa), ra, rb,
		    tm_show(a, da, sizeof da), tm_show(b, db, sizeof db));
	}
}

static void
test_daysinmonth()
{
	static const int years[] = {
		-1900, -1899, -1897, -1896, -400, -100, 0, 1, 3, 4, 96, 99,
		100, 101, 103, 104, 123, 124, 199, 200, 203, 204, 299, 300
	};
	static const int nyears = (int)(sizeof years / sizeof years[0]);

	for (int mon = -4; mon <= 15; mon++)
		for (int y = 0; y < nyears; y++) {
			TmSpec s = { 3, 4, 5, 17, mon, years[y], 2, 40, 0 };

			one_daysinmonth(s);
		}

	rng_seed(0x0da415m0d1eULL);
	for (int i = 0; i < RITER; i++)
		one_daysinmonth(rnd_spec());
}

/* ------------------------------------------------------------------ */
/* domktime                                                            */
/* ------------------------------------------------------------------ */

static void
one_domktime(const TmSpec &s, char type)
{
	TmBuf a, b;

	tm_init(a, s);
	tm_init(b, s);

	int ra = P::domktime(a.t(), type);
	int rb = ref_domktime(b.t(), type);

	stats[S_DOMKTIME].cases++;
	if (ra != rb || !tm_same(a, b)) {
		char sa[128], da[128], db[128];

		report(S_DOMKTIME, "%s type=%d -> %d/%d  tm %s | %s",
		    spec_show(s, sa, sizeof sa), (int)type, ra, rb,
		    tm_show(a, da, sizeof da), tm_show(b, db, sizeof db));
	}
}

static void
test_domktime()
{
	for (int i = 0; i < nbase; i++)
		for (int ty = 0; ty < ntypes; ty++)
			one_domktime(base_specs[i], types[ty]);

	/* Walk the whole minute containing time_t -1, all three isdst modes. */
	for (int sec = 50; sec <= 61; sec++)
		for (int ty = 0; ty < ntypes; ty++)
			for (int dst = -1; dst <= 1; dst++) {
				TmSpec s = { sec, 59, 18, 31, 11, 69, 3, 364,
				    dst };

				one_domktime(s, types[ty]);
			}

	rng_seed(0xd0113a1e5eedULL);
	for (int i = 0; i < RITER; i++)
		one_domktime(rnd_spec(), rnd_type());
}

/* ------------------------------------------------------------------ */
/* trans                                                              */
/* ------------------------------------------------------------------ */

static const std::size_t ARGBUF = 64;

static void
one_trans(const char *src, std::size_t nbytes)
{
	char a[ARGBUF], b[ARGBUF];

	std::memset(a, 0x7f, sizeof a);
	std::memset(b, 0x7f, sizeof b);
	std::memcpy(a, src, nbytes);
	std::memcpy(b, src, nbytes);
	a[ARGBUF - 1] = '\0';
	b[ARGBUF - 1] = '\0';

	int pw = P::trans(P::trans_wday, a);
	int rw = ref_trans(ref_trans_wday, b);
	bool ok = pw == rw && std::memcmp(a, b, ARGBUF) == 0;

	int pm = P::trans(P::trans_mon, a);
	int rm = ref_trans(ref_trans_mon, b);

	ok = ok && pm == rm && std::memcmp(a, b, ARGBUF) == 0;

	stats[S_TRANS].cases++;
	if (!ok)
		report(S_TRANS, "arg=\"%s\" wday %d/%d mon %d/%d", a, pw, rw,
		    pm, rm);
}

static void
one_trans_str(const char *s)
{
	one_trans(s, std::strlen(s) + 1);
}

static void
test_trans()
{
	static const char *hand[] = {
		"", "s", "su", "sun", "sund", "sunday", "sundays", "SUNDAY",
		"SuNdAy", "Sun", "sUn", "monday", "mon", "mo", "m", "tue",
		"tuesday", "tues", "wed", "wednesday", "thu", "thursday",
		"fri", "friday", "sat", "saturday", "satur", "january", "jan",
		"ja", "j", "JAN", "Jan", "february", "feb", "fe", "march",
		"mar", "may", "ma", "mayonnaise", "june", "jun", "july",
		"jul", "august", "aug", "september", "sep", "sept",
		"september1", "october", "oct", "november", "nov", "december",
		"dec", "dece", "0", "00", "1jan", "jan1", " jan", "jan ",
		"\x80", "\x80\x80\x80", "\xff\xfe\xfd", "j\xffn",
		"jan\xff", "ja\x80", "sunda\xff", "SUNDA\xff",
		"\xca\xca\xca", "zzz", "zz", "z", "aaa", "~~~", "\x7f\x7f",
		"decembe", "decemberx", "novembe", "sunda", "saturda",
		"septembe", "wednesda", "thursda", "abc", "APR", "apr",
		"april", "aprilx", "ap", "a",
	};
	static const int nhand = (int)(sizeof hand / sizeof hand[0]);

	for (int i = 0; i < nhand; i++)
		one_trans_str(hand[i]);

	/* Embedded NULs and raw high-bit runs. */
	{
		static const char nulheavy[][8] = {
			{ 0, 0, 0, 0, 0, 0, 0, 0 },
			{ 's', 0, 'u', 'n', 0, 0, 0, 0 },
			{ 's', 'u', 'n', 0, 'd', 'a', 'y', 0 },
			{ 'j', 'a', 0, 'n', 0, 0, 0, 0 },
			{ (char)0x80, 0, (char)0xff, 0, 0, 0, 0, 0 },
		};

		for (int i = 0; i < 5; i++)
			one_trans(nulheavy[i], 8);
	}

	static const char *stems[] = {
		"sunday", "monday", "tuesday", "wednesday", "thursday",
		"friday", "saturday", "january", "february", "march", "april",
		"may", "june", "july", "august", "september", "october",
		"november", "december"
	};
	static const int nstems = (int)(sizeof stems / sizeof stems[0]);

	rng_seed(0x7ba115eedca5eULL);
	for (int i = 0; i < RITER; i++) {
		char buf[ARGBUF];
		int n = 0;
		unsigned mode = (unsigned)(rnd() % 4);

		std::memset(buf, 0, sizeof buf);
		if (mode == 0) {
			/* a prefix of a real name, randomly re-cased */
			const char *w = stems[rnd() % (unsigned)nstems];
			int len = (int)std::strlen(w);
			int keep = (int)rnd_range(0, len);

			for (int k = 0; k < keep; k++) {
				char c = w[k];

				if (rnd() % 2)
					c = (char)(c - 'a' + 'A');
				buf[n++] = c;
			}
			if (n > 0 && rnd() % 3 == 0)
				buf[rnd() % (unsigned)n] ^= (char)0x20;
			if (rnd() % 3 == 0)
				buf[n++] = (char)rnd_range(1, 255);
		} else if (mode == 1) {
			int len = (int)rnd_range(0, 8);

			for (int k = 0; k < len; k++)
				buf[n++] = (char)rnd_range(1, 255);
		} else if (mode == 2) {
			int len = (int)rnd_range(0, 5);

			for (int k = 0; k < len; k++)
				buf[n++] = (char)rnd_range(0x80, 0xff);
		} else {
			int len = (int)rnd_range(0, 8);

			for (int k = 0; k < len; k++)
				buf[n++] =
				    "adejmnorstuy0123 \x01\xff"[rnd() % 20];
		}
		buf[n] = '\0';
		one_trans(buf, (std::size_t)n + 1);
	}
}

/* ------------------------------------------------------------------ */
/* adj* drivers                                                        */
/* ------------------------------------------------------------------ */

typedef int (*adj4_p)(struct tm *, char, std::int64_t, int);
typedef int (*adj5_p)(struct tm *, char, std::int64_t, int, int);

static void
one_adj4(int which, adj4_p pf, adj4_p rf, const TmSpec &s, char type,
    std::int64_t val, int mk)
{
	TmBuf a, b;

	tm_init(a, s);
	tm_init(b, s);

	int ra = pf(a.t(), type, val, mk);
	int rb = rf(b.t(), type, val, mk);

	stats[which].cases++;
	if (ra != rb || !tm_same(a, b)) {
		char sa[128], da[128], db[128];

		report(which,
		    "%s type=%d val=%lld mk=%d -> %d/%d  tm %s | %s",
		    spec_show(s, sa, sizeof sa), (int)type, (long long)val,
		    mk, ra, rb, tm_show(a, da, sizeof da),
		    tm_show(b, db, sizeof db));
	}
}

static void
one_adj5(int which, adj5_p pf, adj5_p rf, const TmSpec &s, char type,
    std::int64_t val, int istext, int mk)
{
	TmBuf a, b;

	tm_init(a, s);
	tm_init(b, s);

	int ra = pf(a.t(), type, val, istext, mk);
	int rb = rf(b.t(), type, val, istext, mk);

	stats[which].cases++;
	if (ra != rb || !tm_same(a, b)) {
		char sa[128], da[128], db[128];

		report(which,
		    "%s type=%d val=%lld istext=%d mk=%d -> %d/%d  tm %s | %s",
		    spec_show(s, sa, sizeof sa), (int)type, (long long)val,
		    istext, mk, ra, rb, tm_show(a, da, sizeof da),
		    tm_show(b, db, sizeof db));
	}
}

/* ------------------------------------------------------------------ */
/* adjyear                                                             */
/* ------------------------------------------------------------------ */

static void
test_adjyear()
{
	static const std::int64_t vals[] = {
		-2, -1, 0, 1, 2, 67, 68, 69, 70, 99, 100, 101, 136, 137, 138,
		139, 1898, 1899, 1900, 1901, 1902, 1968, 1969, 1970, 2000,
		2024, 2037, 2038, 3000
	};
	static const int nvals = (int)(sizeof vals / sizeof vals[0]);

	for (int i = 0; i < nbase; i++)
		for (int ty = 0; ty < ntypes; ty++)
			for (int v = 0; v < nvals; v++)
				for (int mk = 0; mk < 2; mk++)
					one_adj4(S_ADJYEAR, P::adjyear,
					    ref_adjyear, base_specs[i],
					    types[ty], vals[v], mk);

	/* tm_year sweep across the 69 hinge of the default arm */
	for (int y = 60; y <= 145; y++) {
		TmSpec s = { 1, 2, 3, 15, 5, y, 1, 100, 0 };

		for (int ty = 0; ty < ntypes; ty++)
			for (int v = 0; v < nvals; v++)
				one_adj4(S_ADJYEAR, P::adjyear, ref_adjyear,
				    s, types[ty], vals[v], 1);
	}

	rng_seed(0xadd7ea5e0f17ULL);
	for (int i = 0; i < RITER; i++)
		one_adj4(S_ADJYEAR, P::adjyear, ref_adjyear, rnd_spec(),
		    rnd_type(), rnd_val(4000), (int)(rnd() & 1));
}

/* ------------------------------------------------------------------ */
/* adjmon                                                              */
/* ------------------------------------------------------------------ */

static void
test_adjmon()
{
	static const std::int64_t vals[] = {
		-2, -1, 0, 1, 2, 3, 10, 11, 12, 13, 14, 23, 24, 25, 36, 37
	};
	static const int nvals = (int)(sizeof vals / sizeof vals[0]);
	static const int mons[] = { -2, -1, 0, 1, 2, 3, 10, 11, 12, 13 };
	static const int nmons = (int)(sizeof mons / sizeof mons[0]);
	static const int days[] = { 1, 27, 28, 29, 30, 31 };
	static const int ndays = (int)(sizeof days / sizeof days[0]);

	for (int m = 0; m < nmons; m++)
		for (int d = 0; d < ndays; d++) {
			TmSpec s = { 5, 6, 7, days[d], mons[m], 124, 3, 60,
			    0 };

			for (int ty = 0; ty < ntypes; ty++)
				for (int v = 0; v < nvals; v++)
					for (int it = 0; it < 2; it++)
						for (int mk = 0; mk < 2; mk++)
							one_adj5(S_ADJMON,
							    P::adjmon,
							    ref_adjmon, s,
							    types[ty],
							    vals[v], it, mk);
		}

	for (int i = 0; i < nbase; i++)
		for (int ty = 0; ty < ntypes; ty++)
			for (int v = 0; v < nvals; v++)
				for (int it = 0; it < 2; it++)
					for (int mk = 0; mk < 2; mk++)
						one_adj5(S_ADJMON, P::adjmon,
						    ref_adjmon,
						    base_specs[i], types[ty],
						    vals[v], it, mk);

	rng_seed(0xadd110ff5e70ULL);
	for (int i = 0; i < RITER; i++)
		one_adj5(S_ADJMON, P::adjmon, ref_adjmon, rnd_spec(),
		    rnd_type(), rnd_val(4000), (int)(rnd() & 1),
		    (int)(rnd() & 1));
}

/* ------------------------------------------------------------------ */
/* adjday                                                              */
/* ------------------------------------------------------------------ */

static void
test_adjday()
{
	static const std::int64_t vals[] = {
		-30, -2, -1, 0, 1, 2, 3, 26, 27, 28, 29, 30, 31, 32, 33, 58,
		59, 60, 61, 364, 365, 366
	};
	static const int nvals = (int)(sizeof vals / sizeof vals[0]);
	static const int days[] = { -2, -1, 0, 1, 2, 27, 28, 29, 30, 31, 32,
	    40 };
	static const int ndays = (int)(sizeof days / sizeof days[0]);
	static const int mons[] = { 0, 1, 3, 11, 12, -1 };
	static const int nmons = (int)(sizeof mons / sizeof mons[0]);

	for (int m = 0; m < nmons; m++)
		for (int d = 0; d < ndays; d++) {
			TmSpec s = { 8, 9, 10, days[d], mons[m], 124, 2, 70,
			    0 };

			for (int ty = 0; ty < ntypes; ty++)
				for (int v = 0; v < nvals; v++)
					for (int mk = 0; mk < 2; mk++)
						one_adj4(S_ADJDAY, P::adjday,
						    ref_adjday, s, types[ty],
						    vals[v], mk);
		}

	for (int i = 0; i < nbase; i++)
		for (int ty = 0; ty < ntypes; ty++)
			for (int v = 0; v < nvals; v++)
				for (int mk = 0; mk < 2; mk++)
					one_adj4(S_ADJDAY, P::adjday,
					    ref_adjday, base_specs[i],
					    types[ty], vals[v], mk);

	rng_seed(0xadd1da7e0b0eULL);
	for (int i = 0; i < RITER; i++)
		one_adj4(S_ADJDAY, P::adjday, ref_adjday, rnd_spec(),
		    rnd_type(), rnd_val(2000), (int)(rnd() & 1));
}

/* ------------------------------------------------------------------ */
/* adjwday                                                             */
/* ------------------------------------------------------------------ */

static void
test_adjwday()
{
	static const std::int64_t vals[] = {
		-2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 13, 14, 15, 52, 53
	};
	static const int nvals = (int)(sizeof vals / sizeof vals[0]);
	static const int wdays[] = { -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8 };
	static const int nwdays = (int)(sizeof wdays / sizeof wdays[0]);

	for (int w = 0; w < nwdays; w++) {
		TmSpec s = { 11, 12, 13, 15, 5, 124, wdays[w], 166, 0 };

		for (int ty = 0; ty < ntypes; ty++)
			for (int v = 0; v < nvals; v++)
				for (int it = 0; it < 2; it++)
					for (int mk = 0; mk < 2; mk++)
						one_adj5(S_ADJWDAY,
						    P::adjwday, ref_adjwday,
						    s, types[ty], vals[v],
						    it, mk);
	}

	for (int i = 0; i < nbase; i++)
		for (int ty = 0; ty < ntypes; ty++)
			for (int v = 0; v < nvals; v++)
				for (int it = 0; it < 2; it++)
					for (int mk = 0; mk < 2; mk++)
						one_adj5(S_ADJWDAY,
						    P::adjwday, ref_adjwday,
						    base_specs[i], types[ty],
						    vals[v], it, mk);

	rng_seed(0xadd102dayfeedULL);
	for (int i = 0; i < RITER; i++)
		one_adj5(S_ADJWDAY, P::adjwday, ref_adjwday, rnd_spec(),
		    rnd_type(), rnd_val(280), (int)(rnd() & 1),
		    (int)(rnd() & 1));
}

/* ------------------------------------------------------------------ */
/* adjhour                                                             */
/* ------------------------------------------------------------------ */

static void
test_adjhour()
{
	static const std::int64_t vals[] = {
		-2, -1, 0, 1, 2, 11, 12, 22, 23, 24, 25, 26, 47, 48, 49, 71,
		72, 73, 744, 745
	};
	static const int nvals = (int)(sizeof vals / sizeof vals[0]);
	static const int hours[] = { -2, -1, 0, 1, 2, 11, 12, 22, 23, 24,
	    25 };
	static const int nhours = (int)(sizeof hours / sizeof hours[0]);

	for (int h = 0; h < nhours; h++) {
		TmSpec s = { 14, 15, hours[h], 28, 1, 124, 3, 58, 0 };

		for (int ty = 0; ty < ntypes; ty++)
			for (int v = 0; v < nvals; v++)
				for (int mk = 0; mk < 2; mk++)
					one_adj4(S_ADJHOUR, P::adjhour,
					    ref_adjhour, s, types[ty],
					    vals[v], mk);
	}

	for (int i = 0; i < nbase; i++)
		for (int ty = 0; ty < ntypes; ty++)
			for (int v = 0; v < nvals; v++)
				for (int mk = 0; mk < 2; mk++)
					one_adj4(S_ADJHOUR, P::adjhour,
					    ref_adjhour, base_specs[i],
					    types[ty], vals[v], mk);

	rng_seed(0xadd150c10c4bULL);
	for (int i = 0; i < RITER; i++)
		one_adj4(S_ADJHOUR, P::adjhour, ref_adjhour, rnd_spec(),
		    rnd_type(), rnd_val(3000), (int)(rnd() & 1));
}

/* ------------------------------------------------------------------ */
/* adjmin / adjsec                                                     */
/* ------------------------------------------------------------------ */

static const std::int64_t sixty_vals[] = {
	-2, -1, 0, 1, 2, 29, 30, 58, 59, 60, 61, 62, 119, 120, 121, 1439,
	1440, 1441, 3599, 3600
};
static const int nsixty = (int)(sizeof sixty_vals / sizeof sixty_vals[0]);
static const int sixty_fields[] = { -2, -1, 0, 1, 2, 29, 30, 57, 58, 59, 60,
    61 };
static const int nsixty_fields =
    (int)(sizeof sixty_fields / sizeof sixty_fields[0]);

static void
test_adjmin()
{
	for (int f = 0; f < nsixty_fields; f++) {
		TmSpec s = { 33, sixty_fields[f], 5, 28, 1, 124, 3, 58, 0 };

		for (int ty = 0; ty < ntypes; ty++)
			for (int v = 0; v < nsixty; v++)
				for (int mk = 0; mk < 2; mk++)
					one_adj4(S_ADJMIN, P::adjmin,
					    ref_adjmin, s, types[ty],
					    sixty_vals[v], mk);
	}

	for (int i = 0; i < nbase; i++)
		for (int ty = 0; ty < ntypes; ty++)
			for (int v = 0; v < nsixty; v++)
				for (int mk = 0; mk < 2; mk++)
					one_adj4(S_ADJMIN, P::adjmin,
					    ref_adjmin, base_specs[i],
					    types[ty], sixty_vals[v], mk);

	rng_seed(0xadd117e5m1nULL & 0xffffffffffffULL);
	for (int i = 0; i < RITER; i++)
		one_adj4(S_ADJMIN, P::adjmin, ref_adjmin, rnd_spec(),
		    rnd_type(), rnd_val(180000), (int)(rnd() & 1));
}

static void
test_adjsec()
{
	for (int f = 0; f < nsixty_fields; f++) {
		TmSpec s = { sixty_fields[f], 44, 5, 28, 1, 124, 3, 58, 0 };

		for (int ty = 0; ty < ntypes; ty++)
			for (int v = 0; v < nsixty; v++)
				for (int mk = 0; mk < 2; mk++)
					one_adj4(S_ADJSEC, P::adjsec,
					    ref_adjsec, s, types[ty],
					    sixty_vals[v], mk);
	}

	for (int i = 0; i < nbase; i++)
		for (int ty = 0; ty < ntypes; ty++)
			for (int v = 0; v < nsixty; v++)
				for (int mk = 0; mk < 2; mk++)
					one_adj4(S_ADJSEC, P::adjsec,
					    ref_adjsec, base_specs[i],
					    types[ty], sixty_vals[v], mk);

	rng_seed(0xadd1155ecab1ULL);
	for (int i = 0; i < RITER; i++)
		one_adj4(S_ADJSEC, P::adjsec, ref_adjsec, rnd_spec(),
		    rnd_type(), rnd_val(5000000), (int)(rnd() & 1));
}

/* ------------------------------------------------------------------ */
/* vary_append / vary_apply / vary_destroy                             */
/* ------------------------------------------------------------------ */

static const int MAXARGS = 6;

/* Index of `node' within the list headed by `head': -1 for NULL, -2 if the
 * pointer is not a member of the list at all.  Raw addresses are never
 * compared between the two implementations. */
static int
p_index(const P::vary *head, const P::vary *node)
{
	int i = 0;

	if (node == nullptr)
		return -1;
	for (const P::vary *p = head; p; p = p->next, i++)
		if (p == node)
			return i;
	return -2;
}

static int
r_index(const struct vary *head, const struct vary *node)
{
	int i = 0;

	if (node == nullptr)
		return -1;
	for (const struct vary *p = head; p; p = p->next, i++)
		if (p == node)
			return i;
	return -2;
}

static int
arg_index(char (*bufs)[ARGBUF], int n, const char *p)
{
	for (int i = 0; i < n; i++)
		if (bufs[i] == p)
			return i;
	return -1;
}

static void
p_free(P::vary *v)
{
	while (v) {
		P::vary *n = v->next;

		std::free(v);
		v = n;
	}
}

static void
r_free(struct vary *v)
{
	while (v) {
		struct vary *n = v->next;

		std::free(v);
		v = n;
	}
}

static void
test_vary_append()
{
	for (int n = 1; n <= MAXARGS; n++) {
		char pargs[MAXARGS][ARGBUF], rargs[MAXARGS][ARGBUF];
		P::vary *ph = nullptr;
		struct vary *rh = nullptr;
		bool ok = true;

		for (int i = 0; i < n; i++) {
			std::memset(pargs[i], 0x7f, ARGBUF);
			std::memset(rargs[i], 0x7f, ARGBUF);
			std::snprintf(pargs[i], 8, "a%d", i);
			std::snprintf(rargs[i], 8, "a%d", i);
		}

		for (int i = 0; i < n; i++) {
			P::vary *pr = P::vary_append(ph, pargs[i]);
			struct vary *rr = ref_vary_append(rh, rargs[i]);

			if (i == 0) {
				ok = ok && pr != nullptr && rr != nullptr;
				ph = pr;
				rh = rr;
			} else {
				/* every later call must hand back the head
				 * it was given, not the new tail */
				ok = ok && pr == ph && rr == rh;
				ph = pr;
				rh = rr;
			}
			if (p_index(ph, pr) != r_index(rh, rr))
				ok = false;
		}

		int pn = 0, rn = 0;

		for (P::vary *p = ph; p; p = p->next)
			pn++;
		for (struct vary *r = rh; r; r = r->next)
			rn++;
		ok = ok && pn == n && rn == n;

		P::vary *p = ph;
		struct vary *r = rh;

		for (int i = 0; i < n && p && r; i++, p = p->next,
		    r = r->next) {
			int pi = arg_index(pargs, n, p->arg);
			int ri = arg_index(rargs, n, r->arg);

			ok = ok && pi == ri && pi == i;
			ok = ok && (p->next == nullptr) ==
			    (r->next == nullptr);
		}

		for (int i = 0; i < n; i++)
			ok = ok &&
			    std::memcmp(pargs[i], rargs[i], ARGBUF) == 0;

		stats[S_VARY_APPEND].cases++;
		if (!ok)
			report(S_VARY_APPEND, "n=%d plen=%d rlen=%d", n, pn,
			    rn);

		p_free(ph);
		r_free(rh);
	}

	/* Deep list so the while (v->next) walk runs to a real depth. */
	{
		static char args[64][ARGBUF];
		P::vary *ph = nullptr;
		struct vary *rh = nullptr;
		bool ok = true;
		int idx = 0;

		for (int i = 0; i < 64; i++) {
			std::memset(args[i], 0x7f, ARGBUF);
			std::snprintf(args[i], 8, "z%d", i);

			P::vary *pr = P::vary_append(ph, args[i]);
			struct vary *rr = ref_vary_append(rh, args[i]);

			if (i == 0) {
				ph = pr;
				rh = rr;
			} else
				ok = ok && pr == ph && rr == rh;
		}

		P::vary *p = ph;
		struct vary *r = rh;

		while (p && r) {
			ok = ok && p->arg == args[idx] && r->arg == args[idx];
			p = p->next;
			r = r->next;
			idx++;
		}
		ok = ok && p == nullptr && r == nullptr && idx == 64;

		stats[S_VARY_APPEND].cases++;
		if (!ok)
			report(S_VARY_APPEND, "deep append idx=%d", idx);

		p_free(ph);
		r_free(rh);
	}
}

static void
test_vary_destroy()
{
	static char args[64][ARGBUF];

	/* Warm the allocator so the first measurement is not skewed. */
	(void)mallinfo2();

	for (int n = 1; n <= 64; n *= 2) {
		for (int i = 0; i < n; i++)
			std::snprintf(args[i], 8, "q%d", i);

		std::size_t p0 = mallinfo2().uordblks;
		P::vary *ph = nullptr;

		for (int i = 0; i < n; i++)
			ph = P::vary_append(ph, args[i]);

		std::size_t p1 = mallinfo2().uordblks;

		P::vary_destroy(ph);

		std::size_t p2 = mallinfo2().uordblks;
		std::size_t r0 = mallinfo2().uordblks;
		struct vary *rh = nullptr;

		for (int i = 0; i < n; i++)
			rh = ref_vary_append(rh, args[i]);

		std::size_t r1 = mallinfo2().uordblks;

		ref_vary_destroy(rh);

		std::size_t r2 = mallinfo2().uordblks;

		/* A destroy that stops early leaks; one that runs long
		 * double-frees and aborts. */
		bool ok = p1 > p0 && r1 > r0 && p1 - p0 == r1 - r0 &&
		    p2 == p0 && r2 == r0;

		stats[S_VARY_DESTROY].cases++;
		if (!ok)
			report(S_VARY_DESTROY,
			    "n=%d port %zu/%zu/%zu ref %zu/%zu/%zu", n, p0,
			    p1, p2, r0, r1, r2);
	}

	{
		std::size_t p0 = mallinfo2().uordblks;

		P::vary_destroy(nullptr);
		ref_vary_destroy(nullptr);

		std::size_t p1 = mallinfo2().uordblks;

		stats[S_VARY_DESTROY].cases++;
		if (p0 != p1)
			report(S_VARY_DESTROY, "empty %zu/%zu", p0, p1);
	}
}

/*
 * Both copies of every argument buffer are byte-identical and padded past the
 * terminator with 0x5a, so an off-by-one read of arg[len - 1] lands on a byte
 * that is neither NUL nor a recognised suffix.
 */
static void
gen_arg(char *buf)
{
	static const char suffix[] = { 'S', 'M', 'H', 'd', 'w', 'm', 'y', 'x',
	    'D', 'W', 's', 'Y', '\0', '0', '9', (char)0x80, (char)0xff, ' ' };
	static const int nsuffix = (int)(sizeof suffix / sizeof suffix[0]);
	static const char *words[] = {
		"sunday", "sun", "monday", "mon", "tuesday", "tue",
		"wednesday", "wed", "thursday", "thu", "friday", "fri",
		"saturday", "sat", "january", "jan", "february", "feb",
		"march", "mar", "april", "apr", "may", "june", "jun", "july",
		"jul", "august", "aug", "september", "sep", "october", "oct",
		"november", "nov", "december", "dec", "SUNDAY", "Jan", "ma",
		"s", "d", "0", "", "junk", "zz"
	};
	static const int nwords = (int)(sizeof words / sizeof words[0]);

	int n = 0;
	unsigned sgn = (unsigned)(rnd() % 4);
	unsigned mode = (unsigned)(rnd() % 5);

	std::memset(buf, 0x5a, ARGBUF);
	if (sgn == 0)
		buf[n++] = '+';
	else if (sgn == 1)
		buf[n++] = '-';
	else if (sgn == 2 && rnd() % 4 == 0)
		buf[n++] = (char)rnd_range(1, 255);

	if (mode == 0 || mode == 1) {
		int nd = (int)rnd_range(0, 4);

		for (int k = 0; k < nd; k++)
			buf[n++] = (char)('0' + (int)(rnd() % 10));
		buf[n++] = suffix[rnd() % (unsigned)nsuffix];
	} else if (mode == 2) {
		const char *w = words[rnd() % (unsigned)nwords];

		while (*w)
			buf[n++] = *w++;
		if (rnd() % 4 == 0)
			buf[n++] = (char)('0' + (int)(rnd() % 10));
	} else if (mode == 3) {
		int len = (int)rnd_range(0, 7);

		for (int k = 0; k < len; k++)
			buf[n++] =
			    "0123456789dwmySMHx-+ \xff\x80"[rnd() % 24];
	} else {
		int len = (int)rnd_range(0, 6);

		for (int k = 0; k < len; k++)
			buf[n++] = (char)rnd_range(0, 255);
	}
	buf[n] = '\0';
}

static void
one_vary_apply(char args[][ARGBUF], int n, const TmSpec &s)
{
	char pargs[MAXARGS][ARGBUF], rargs[MAXARGS][ARGBUF];
	P::vary *ph = nullptr;
	struct vary *rh = nullptr;
	TmBuf a, b;

	for (int i = 0; i < n; i++) {
		std::memcpy(pargs[i], args[i], ARGBUF);
		std::memcpy(rargs[i], args[i], ARGBUF);
		ph = P::vary_append(ph, pargs[i]);
		rh = ref_vary_append(rh, rargs[i]);
	}

	tm_init(a, s);
	tm_init(b, s);

	const P::vary *pr = P::vary_apply(ph, a.t());
	const struct vary *rr = ref_vary_apply(rh, b.t());

	int pi = p_index(ph, pr);
	int ri = r_index(rh, rr);
	bool ok = pi == ri && tm_same(a, b);

	for (int i = 0; i < n; i++)
		if (std::memcmp(pargs[i], rargs[i], ARGBUF) != 0 ||
		    std::memcmp(pargs[i], args[i], ARGBUF) != 0)
			ok = false;

	{
		int k = 0;
		const P::vary *p = ph;
		const struct vary *r = rh;

		while (p && r) {
			if (p->arg != pargs[k] || r->arg != rargs[k])
				ok = false;
			p = p->next;
			r = r->next;
			k++;
		}
		if (p != nullptr || r != nullptr || k != n)
			ok = false;
	}

	stats[S_VARY_APPLY].cases++;
	if (!ok) {
		char sa[128], da[160], db[160];

		report(S_VARY_APPLY,
		    "n=%d arg0=\"%s\" %s -> %d/%d  tm %s | %s", n, args[0],
		    spec_show(s, sa, sizeof sa), pi, ri,
		    tm_show(a, da, sizeof da), tm_show(b, db, sizeof db));
	}

	p_free(ph);
	r_free(rh);
}

static void
set_arg(char *buf, const char *s)
{
	std::size_t l = std::strlen(s);

	std::memset(buf, 0x5a, ARGBUF);
	std::memcpy(buf, s, l);
	buf[l] = '\0';
}

static void
test_vary_apply()
{
	static const char *hand[] = {
		"", "+", "-", "d", "1", "1d", "+1d", "-1d", "0d", "+0d",
		"-0d", "31d", "+31d", "-31d", "365d", "+365d", "-365d",
		"1w", "+1w", "-1w", "5w", "+5w", "-5w", "0w", "6w", "7w",
		"+7w", "1m", "+1m", "-1m", "12m", "13m", "0m", "+12m",
		"-12m", "1y", "+1y", "-1y", "68y", "69y", "70y", "1900y",
		"1901y", "2024y", "+100y", "-100y", "1H", "+1H", "-1H",
		"23H", "24H", "+24H", "-24H", "0H", "1M", "+1M", "-1M",
		"59M", "60M", "+60M", "-60M", "1S", "+1S", "-1S", "59S",
		"60S", "+60S", "-60S", "0S", "+0S",
		"sunday", "+sunday", "-sunday", "monday", "+monday",
		"-monday", "saturday", "+saturday", "-saturday", "sun",
		"+sun", "-sun", "january", "+january", "-january",
		"december", "+december", "-december", "jan", "+jan", "-jan",
		"may", "+may", "-may", "feb", "mar",
		"12", "123", "1234567", "+1234567d", "9999999S", "999999M",
		"1x", "+1x", "1D", "1W", "1Y", "1s", "1h", "1\x80",
		"\x80\x80", "\xff\xff\xff", "+\xff\xff", "-\x80\x80",
		"++1d", "--1d", "+-1d", "-+1d", "+ 1d", " 1d", "1 d",
		"1d ", "0000000d", "0000001d", "00d", "000000000012m",
		"+30d", "+29d", "+28d", "-28d", "-29d", "-30d",
	};
	static const int nhand = (int)(sizeof hand / sizeof hand[0]);

	char args[MAXARGS][ARGBUF];

	for (int h = 0; h < nhand; h++) {
		set_arg(args[0], hand[h]);
		for (int i = 0; i < nbase; i++)
			one_vary_apply(args, 1, base_specs[i]);
	}

	/* Multi-element lists, including ones that stop part way. */
	static const char *chains[][3] = {
		{ "+1d", "+1M", "+1S" },
		{ "+1d", "bogus", "+1S" },
		{ "1x", "+1d", "+1d" },
		{ "sunday", "+1m", "-1y" },
		{ "-1m", "-1d", "-1H" },
		{ "31d", "2m", "1H" },
		{ "99999d", "+1d", "+1d" },
		{ "+7w", "-7w", "monday" },
		{ "", "+1d", "+1d" },
		{ "+1d", "", "+1d" },
	};
	static const int nchains = (int)(sizeof chains / sizeof chains[0]);

	for (int c = 0; c < nchains; c++) {
		for (int k = 0; k < 3; k++)
			set_arg(args[k], chains[c][k]);
		for (int i = 0; i < nbase; i++) {
			one_vary_apply(args, 2, base_specs[i]);
			one_vary_apply(args, 3, base_specs[i]);
		}
	}

	/* A string that is empty once its sign is consumed, and an empty
	 * string with junk still living past the terminator. */
	{
		std::memset(args[0], 0x5a, ARGBUF);
		args[0][0] = '+';
		args[0][1] = '\0';
		for (int i = 0; i < nbase; i++)
			one_vary_apply(args, 1, base_specs[i]);

		std::memset(args[0], 0x5a, ARGBUF);
		args[0][0] = '\0';
		for (int i = 0; i < nbase; i++)
			one_vary_apply(args, 1, base_specs[i]);
	}

	rng_seed(0x7a2ya7710ULL & 0xffffffffULL);
	for (int i = 0; i < RITER; i++) {
		int n = (int)rnd_range(1, 3);

		for (int k = 0; k < n; k++)
			gen_arg(args[k]);
		one_vary_apply(args, n, rnd_spec());
	}
}

/* ------------------------------------------------------------------ */

int
main()
{
	/* A POSIX TZ string keeps the result independent of the host's
	 * zoneinfo database while still providing DST transitions. */
	setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0/2", 1);
	tzset();

	test_daysinmonth();
	test_domktime();
	test_trans();
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

	long long tot = 0, bad = 0;

	std::printf("\n%-16s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-16s %12s %12s\n", "----------------",
	    "------------", "------------");
	for (int i = 0; i < S_NSTATS; i++) {
		std::printf("%-16s %12lld %12lld\n", stats[i].name,
		    stats[i].cases, stats[i].fails);
		tot += stats[i].cases;
		bad += stats[i].fails;
	}
	std::printf("%-16s %12s %12s\n", "----------------",
	    "------------", "------------");
	std::printf("%-16s %12lld %12lld\n", "TOTAL", tot, bad);

	return bad == 0 ? 0 : 1;
}
