/*
 * Differential test for batch b0153s1 (lib/libc/locale/localeconv.c).
 *
 * Every case runs twice: once through the C++23 port and once through the
 * unmodified C oracle.  Both sides share one string pool and one set of
 * lc_monetary_T / lc_numeric_T source objects, so every pointer the two
 * implementations store into their struct lconv must compare bit-identical.
 * Each locale lives inside a guarded slot pre-filled with 0x7f, and the
 * ENTIRE slot -- struct, padding and 64 trailing guard bytes -- is compared
 * after every call, not just the returned value.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <climits>

import pbsd.lib.libc.locale.b0153s1;

namespace P = pbsd::lib_libc_locale::b0153s1;

/*
 * The oracle's view of the same declarations.  These reach the oracle through
 * C linkage, so only the layout has to agree; keeping them in a namespace
 * avoids colliding with the host <locale.h>.
 */
namespace O {

struct lconv {
	char	*decimal_point;
	char	*thousands_sep;
	char	*grouping;
	char	*int_curr_symbol;
	char	*currency_symbol;
	char	*mon_decimal_point;
	char	*mon_thousands_sep;
	char	*mon_grouping;
	char	*positive_sign;
	char	*negative_sign;
	char	int_frac_digits;
	char	frac_digits;
	char	p_cs_precedes;
	char	p_sep_by_space;
	char	n_cs_precedes;
	char	n_sep_by_space;
	char	p_sign_posn;
	char	n_sign_posn;
	char	int_p_cs_precedes;
	char	int_n_cs_precedes;
	char	int_p_sep_by_space;
	char	int_n_sep_by_space;
	char	int_p_sign_posn;
	char	int_n_sign_posn;
};

struct lc_monetary_T {
	const char	*int_curr_symbol;
	const char	*currency_symbol;
	const char	*mon_decimal_point;
	const char	*mon_thousands_sep;
	const char	*mon_grouping;
	const char	*positive_sign;
	const char	*negative_sign;
	const char	*int_frac_digits;
	const char	*frac_digits;
	const char	*p_cs_precedes;
	const char	*p_sep_by_space;
	const char	*n_cs_precedes;
	const char	*n_sep_by_space;
	const char	*p_sign_posn;
	const char	*n_sign_posn;
	const char	*int_p_cs_precedes;
	const char	*int_p_sep_by_space;
	const char	*int_n_cs_precedes;
	const char	*int_n_sep_by_space;
	const char	*int_p_sign_posn;
	const char	*int_n_sign_posn;
};

struct lc_numeric_T {
	const char	*decimal_point;
	const char	*thousands_sep;
	const char	*grouping;
};

struct _xlocale {
	struct lconv		 lconv;
	int			 monetary_locale_changed;
	int			 numeric_locale_changed;
	struct lc_monetary_T	*__mon;
	struct lc_numeric_T	*__num;
};

typedef struct _xlocale *locale_t;

} /* namespace O */

extern "C" {
O::lconv *ref_localeconv_l(O::locale_t loc);
O::lconv *ref_localeconv(void);
extern O::_xlocale *__ref_current_locale;
}

static_assert(sizeof(P::lconv) == sizeof(O::lconv));
static_assert(sizeof(P::_xlocale) == sizeof(O::_xlocale));
static_assert(sizeof(P::lc_monetary_T) == sizeof(O::lc_monetary_T));
static_assert(sizeof(P::lc_numeric_T) == sizeof(O::lc_numeric_T));

/* ------------------------------------------------------------------ */

#define LCONV_PTR_FIELDS(X)						\
	X(decimal_point) X(thousands_sep) X(grouping)			\
	X(int_curr_symbol) X(currency_symbol) X(mon_decimal_point)	\
	X(mon_thousands_sep) X(mon_grouping) X(positive_sign)		\
	X(negative_sign)

#define LCONV_CHAR_FIELDS(X)						\
	X(int_frac_digits) X(frac_digits) X(p_cs_precedes)		\
	X(p_sep_by_space) X(n_cs_precedes) X(n_sep_by_space)		\
	X(p_sign_posn) X(n_sign_posn) X(int_p_cs_precedes)		\
	X(int_n_cs_precedes) X(int_p_sep_by_space)			\
	X(int_n_sep_by_space) X(int_p_sign_posn) X(int_n_sign_posn)

#define MON_FIELDS(X)							\
	X(int_curr_symbol) X(currency_symbol) X(mon_decimal_point)	\
	X(mon_thousands_sep) X(mon_grouping) X(positive_sign)		\
	X(negative_sign) X(int_frac_digits) X(frac_digits)		\
	X(p_cs_precedes) X(p_sep_by_space) X(n_cs_precedes)		\
	X(n_sep_by_space) X(p_sign_posn) X(n_sign_posn)			\
	X(int_p_cs_precedes) X(int_p_sep_by_space)			\
	X(int_n_cs_precedes) X(int_n_sep_by_space)			\
	X(int_p_sign_posn) X(int_n_sign_posn)

#define NUM_FIELDS(X)	X(decimal_point) X(thousands_sep) X(grouping)

enum {
	POOLSZ	= 8,
	POOLN	= 68,	/* 2 slots * (21 monetary + 3 numeric + 10 lconv) */
	GUARD	= 64,
	NCHAR	= 14,
	NMON	= 21,
	NNUM	= 3,
	MAXREPORT = 20
};

/* Shared by both sides, so identical pointers must come out of both. */
static char pool[POOLN][POOLSZ];

struct PSlot { P::_xlocale loc; unsigned char guard[GUARD]; };
struct OSlot { O::_xlocale loc; unsigned char guard[GUARD]; };

static_assert(sizeof(PSlot) == sizeof(OSlot));

static PSlot pslot[2];
static OSlot oslot[2];

/* One physical source object per slot, aliased by both implementations. */
struct MonStore { P::lc_monetary_T m; unsigned char guard[GUARD]; };
struct NumStore { P::lc_numeric_T n; unsigned char guard[GUARD]; };

static MonStore monst[2], monsave[2];
static NumStore numst[2], numsave[2];

static unsigned char initch[2][NCHAR];

static inline uint64_t
nx(uint64_t &s)
{
	s ^= s << 13;
	s ^= s >> 7;
	s ^= s << 17;
	return s * 0x2545F4914F6CDD1DULL;
}

/*
 * Pool contents.  Most patterns deliberately make pool[i][0] differ from
 * pool[i][1] so a mutated M_ASSIGN_CHAR index is observable, and several push
 * high-bit bytes (0x80-0xff) through the char assignments.
 */
static void
fill_pool(int pattern, uint64_t &st)
{
	for (int i = 0; i < POOLN; i++) {
		for (int j = 0; j < POOLSZ; j++)
			pool[i][j] = (char)(nx(st) & 0xff);
		switch (pattern) {
		case 1:
			memset(pool[i], 0x00, POOLSZ);
			break;
		case 2:
			memset(pool[i], (char)0xff, POOLSZ);
			break;
		case 3:
			memset(pool[i], (char)0x80, POOLSZ);
			break;
		case 4:
			/* same byte as the guard fill */
			memset(pool[i], 0x7f, POOLSZ);
			break;
		case 5:
			/* empty string, distinct byte just past the NUL */
			pool[i][0] = 0x00;
			pool[i][1] = (char)(0x80 + (i & 0x7f));
			break;
		case 6:
			pool[i][0] = (char)(0x80 | (i & 0x7f));
			pool[i][1] = (char)(i & 0x7f);
			break;
		case 7:
			/* adjacent values: catches an off-by-one index */
			pool[i][0] = (char)(i + 1);
			pool[i][1] = (char)i;
			break;
		case 8:
			for (int j = 0; j < POOLSZ; j++)
				pool[i][j] = (char)(i * POOLSZ + j);
			break;
		case 9:
			pool[i][0] = 0x01;
			pool[i][1] = 0x00;
			break;
		default:
			break;
		}
		if (pattern == 0 && pool[i][0] == pool[i][1])
			pool[i][1] = (char)(pool[i][0] ^ 0x5a);
		pool[i][POOLSZ - 1] = 0x00;
	}
}

/*
 * Every field of every structure gets its own pool entry, so a port that
 * writes the right value into the wrong lconv member is caught.
 */
static void
setup_sources(int slot)
{
	memset((void *)&monst[slot], 0x7f, sizeof(monst[slot]));
	memset((void *)&numst[slot], 0x7f, sizeof(numst[slot]));

	int k = slot ? 34 : 0;
#define SETM(NAME) monst[slot].m.NAME = pool[k++];
	MON_FIELDS(SETM)
#undef SETM
#define SETN(NAME) numst[slot].n.NAME = pool[k++];
	NUM_FIELDS(SETN)
#undef SETN

	memcpy((void *)&monsave[slot], (void *)&monst[slot],
	    sizeof(monst[slot]));
	memcpy((void *)&numsave[slot], (void *)&numst[slot],
	    sizeof(numst[slot]));
}

template <class SLOT>
static void
setup_slot(SLOT &s, int slot, int monflag, int numflag)
{
	memset((void *)&s, 0x7f, sizeof(s));

	int k = (slot ? 34 : 0) + NMON + NNUM;
#define SETP(NAME) s.loc.lconv.NAME = pool[k++];
	LCONV_PTR_FIELDS(SETP)
#undef SETP
	int j = 0;
#define SETC(NAME) s.loc.lconv.NAME = (char)initch[slot][j++];
	LCONV_CHAR_FIELDS(SETC)
#undef SETC
	s.loc.monetary_locale_changed = monflag;
	s.loc.numeric_locale_changed = numflag;
	s.loc.__mon = (decltype(s.loc.__mon))(void *)&monst[slot].m;
	s.loc.__num = (decltype(s.loc.__num))(void *)&numst[slot].n;
}

/*
 * Map a pointer to a (slot, byte offset) identity, so returned pointers are
 * never compared as raw addresses.
 */
static long
ident(const void *p, const void *b0, const void *b1, size_t sz)
{
	uintptr_t v = (uintptr_t)p;
	uintptr_t a0 = (uintptr_t)b0;
	uintptr_t a1 = (uintptr_t)b1;

	if (p == nullptr)
		return -1;
	if (v >= a0 && v < a0 + sz)
		return (long)(v - a0);
	if (v >= a1 && v < a1 + sz)
		return 1000000L + (long)(v - a1);
	return -2;
}

struct FnStat {
	const char	*name;
	long		 cases;
	long		 fails;
};

static FnStat stat_tab[2] = {
	{ "localeconv_l", 0, 0 },
	{ "localeconv",   0, 0 },
};

static long reported = 0;

static long
print_table(void)
{
	long total = 0, fails = 0;

	printf("\n%-16s %12s %12s\n", "function", "cases", "failures");
	printf("%-16s %12s %12s\n", "----------------", "------------",
	    "------------");
	for (int i = 0; i < 2; i++) {
		printf("%-16s %12ld %12ld\n", stat_tab[i].name,
		    stat_tab[i].cases, stat_tab[i].fails);
		total += stat_tab[i].cases;
		fails += stat_tab[i].fails;
	}
	printf("%-16s %12s %12s\n", "----------------", "------------",
	    "------------");
	printf("%-16s %12ld %12ld\n", "TOTAL", total, fails);
	fflush(stdout);
	return fails;
}

struct Case {
	int		pattern;
	int		flag[2][2];
	int		gsel;
	int		ncalls;
	int		mode[3];
	unsigned char	ich[2][NCHAR];
};

struct Seq { int n; int m[3]; };

/*
 * mode 0: localeconv_l(&slot0)   mode 1: localeconv_l(NULL)
 * mode 2: localeconv()           mode 3: localeconv_l(&slot1)
 */
static void
run_case(const Case &c, const char *origin, long idx, uint64_t poolseed)
{
	uint64_t st = poolseed | 1;

	fill_pool(c.pattern, st);
	memcpy(initch, c.ich, sizeof(initch));

	for (int s = 0; s < 2; s++) {
		setup_sources(s);
		setup_slot(pslot[s], s, c.flag[s][0], c.flag[s][1]);
		setup_slot(oslot[s], s, c.flag[s][0], c.flag[s][1]);
	}

	P::__current_locale = &pslot[c.gsel].loc;
	__ref_current_locale = &oslot[c.gsel].loc;

	for (int call = 0; call < c.ncalls; call++) {
		int m = c.mode[call];
		int fn = (m == 2) ? 1 : 0;

		P::lconv *rp;
		O::lconv *ro;

		switch (m) {
		case 0:
			rp = P::localeconv_l(&pslot[0].loc);
			ro = ref_localeconv_l(&oslot[0].loc);
			break;
		case 1:
			rp = P::localeconv_l(nullptr);
			ro = ref_localeconv_l(nullptr);
			break;
		case 2:
			rp = P::localeconv();
			ro = ref_localeconv();
			break;
		default:
			rp = P::localeconv_l(&pslot[1].loc);
			ro = ref_localeconv_l(&oslot[1].loc);
			break;
		}

		stat_tab[fn].cases++;

		long ip = ident(rp, &pslot[0], &pslot[1], sizeof(PSlot));
		long io = ident(ro, &oslot[0], &oslot[1], sizeof(OSlot));

		const char *why = nullptr;
		if (ip != io)
			why = "return offset";
		else if (memcmp(&pslot[0], &oslot[0], sizeof(PSlot)) != 0)
			why = "slot 0 buffer";
		else if (memcmp(&pslot[1], &oslot[1], sizeof(PSlot)) != 0)
			why = "slot 1 buffer";
		else if (memcmp((void *)&monst[0], (void *)&monsave[0],
		    sizeof(monst[0])) != 0 ||
		    memcmp((void *)&monst[1], (void *)&monsave[1],
		    sizeof(monst[1])) != 0)
			why = "monetary source clobbered";
		else if (memcmp((void *)&numst[0], (void *)&numsave[0],
		    sizeof(numst[0])) != 0 ||
		    memcmp((void *)&numst[1], (void *)&numsave[1],
		    sizeof(numst[1])) != 0)
			why = "numeric source clobbered";
		else if (ident(P::__current_locale, &pslot[0], &pslot[1],
		    sizeof(PSlot)) != ident(__ref_current_locale, &oslot[0],
		    &oslot[1], sizeof(OSlot)))
			why = "current locale pointer";

		if (why == nullptr)
			continue;

		stat_tab[fn].fails++;
		reported++;
		printf("FAIL %s %ld call %d mode %d pattern %d gsel %d "
		    "flags {%d,%d}/{%d,%d}: %s (port ret %ld, ref ret %ld)\n",
		    origin, idx, call, m, c.pattern, c.gsel, c.flag[0][0],
		    c.flag[0][1], c.flag[1][0], c.flag[1][1], why, ip, io);
		fflush(stdout);

		/*
		 * A port this broken may well fault before the sweep ends, so
		 * report what has been found and leave with a failing status
		 * while the evidence is still printable.
		 */
		if (reported >= MAXREPORT) {
			printf("\n%ld failures reported; stopping early.\n",
			    reported);
			(void)print_table();
			exit(1);
		}
	}
}

static void
fill_ich_ramp(Case &c)
{
	for (int s = 0; s < 2; s++)
		for (int t = 0; t < NCHAR; t++)
			c.ich[s][t] = (unsigned char)(0x80 + s * 0x20 + t);
}

int
main(void)
{
	static const int flagpairs[][2] = {
		{ 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 },
		{ 2, 0 }, { 0, 2 }, { 2, 2 }, { -1, 0 },
		{ 0, -1 }, { -1, -1 }, { 1, -1 }, { -1, 1 },
		{ INT_MIN, INT_MAX }, { INT_MAX, INT_MIN },
		{ 0x10000, 0x100 },
	};
	const int NFP = (int)(sizeof(flagpairs) / sizeof(flagpairs[0]));

	static const Seq seqs[] = {
		{ 1, { 0, 0, 0 } },
		{ 2, { 0, 0, 0 } },
		{ 1, { 1, 0, 0 } },
		{ 2, { 1, 1, 0 } },
		{ 1, { 2, 0, 0 } },
		{ 3, { 2, 2, 2 } },
		{ 3, { 0, 1, 2 } },
		{ 2, { 3, 0, 0 } },
		{ 3, { 3, 3, 0 } },
		{ 3, { 1, 0, 3 } },
	};
	const int NSEQ = (int)(sizeof(seqs) / sizeof(seqs[0]));

	long idx = 0;
	const uint64_t handseed = 0x9E3779B97F4A7C15ULL;

	/*
	 * Explicit-locale calls made while the current locale points at the
	 * other slot come first.  A port that consults the current locale when
	 * it was handed one (or the reverse) is then reported as a mismatch
	 * rather than faulting later on a NULL argument.
	 */
	static const Seq nonnull_seqs[] = {
		{ 1, { 0, 0, 0 } },
		{ 2, { 0, 0, 0 } },
		{ 1, { 3, 0, 0 } },
		{ 2, { 3, 3, 0 } },
		{ 3, { 0, 3, 0 } },
		{ 3, { 3, 0, 3 } },
	};
	const int NNS = (int)(sizeof(nonnull_seqs) / sizeof(nonnull_seqs[0]));

	for (int pattern = 0; pattern < 10; pattern++) {
		for (int a = 0; a < NFP; a++) {
			for (int b = 0; b < NFP; b++) {
				for (int q = 0; q < NNS; q++) {
					Case c;
					c.pattern = pattern;
					c.flag[0][0] = flagpairs[a][0];
					c.flag[0][1] = flagpairs[a][1];
					c.flag[1][0] = flagpairs[b][0];
					c.flag[1][1] = flagpairs[b][1];
					c.gsel = 1 - (q & 1);
					c.ncalls = nonnull_seqs[q].n;
					for (int t = 0; t < 3; t++)
						c.mode[t] = nonnull_seqs[q].m[t];
					fill_ich_ramp(c);
					uint64_t sd = handseed +
					    (uint64_t)idx * 0x01000193ULL;
					run_case(c, "explicit", idx, sd);
					idx++;
				}
			}
		}
	}

	/* ---------------- hand written edge cases ---------------- */
	for (int pattern = 0; pattern < 10; pattern++) {
		for (int a = 0; a < NFP; a++) {
			for (int b = 0; b < NFP; b++) {
				for (int g = 0; g < 2; g++) {
					for (int q = 0; q < NSEQ; q++) {
						Case c;
						c.pattern = pattern;
						c.flag[0][0] = flagpairs[a][0];
						c.flag[0][1] = flagpairs[a][1];
						c.flag[1][0] = flagpairs[b][0];
						c.flag[1][1] = flagpairs[b][1];
						c.gsel = g;
						c.ncalls = seqs[q].n;
						for (int t = 0; t < 3; t++)
							c.mode[t] = seqs[q].m[t];
						fill_ich_ramp(c);
						uint64_t sd = handseed +
						    (uint64_t)idx * 0x01000193ULL;
						run_case(c, "edge", idx, sd);
						idx++;
					}
				}
			}
		}
	}

	/*
	 * Degenerate pre-existing lconv characters (0x00, 0xff, 0x7f) so that
	 * a write of the wrong field stays visible even at the extremes.
	 */
	static const unsigned char fillv[3] = { 0x00, 0xff, 0x7f };
	for (int pattern = 0; pattern < 10; pattern++) {
		for (int v = 0; v < 3; v++) {
			for (int q = 0; q < NSEQ; q++) {
				Case c;
				c.pattern = pattern;
				c.flag[0][0] = q & 1;
				c.flag[0][1] = (q >> 1) & 1;
				c.flag[1][0] = (q >> 2) & 1;
				c.flag[1][1] = 1 - (q & 1);
				c.gsel = q & 1;
				c.ncalls = seqs[q].n;
				for (int t = 0; t < 3; t++)
					c.mode[t] = seqs[q].m[t];
				memset(c.ich, fillv[v], sizeof(c.ich));
				uint64_t sd = handseed ^
				    ((uint64_t)idx * 0x100000001B3ULL);
				run_case(c, "edge2", idx, sd);
				idx++;
			}
		}
	}

	/* ---------------- randomised sweep ---------------- */
	uint64_t st = 0xDEADBEEFCAFEBABEULL;
	const long ITER = 250000;
	for (long i = 0; i < ITER; i++) {
		Case c;
		c.pattern = (int)(nx(st) % 10);
		for (int s = 0; s < 2; s++) {
			for (int f = 0; f < 2; f++) {
				uint64_t r = nx(st);
				switch (r & 7) {
				case 0: case 1: case 2:
					c.flag[s][f] = 0;
					break;
				case 3: case 4: case 5:
					c.flag[s][f] = 1;
					break;
				case 6:
					c.flag[s][f] = (int)(nx(st) & 0xff);
					break;
				default:
					c.flag[s][f] = (int)(uint32_t)nx(st);
					break;
				}
			}
			for (int t = 0; t < NCHAR; t++)
				c.ich[s][t] = (unsigned char)(nx(st) & 0xff);
		}
		c.gsel = (int)(nx(st) & 1);
		c.ncalls = 1 + (int)(nx(st) % 3);
		for (int t = 0; t < 3; t++)
			c.mode[t] = (int)(nx(st) & 3);
		run_case(c, "rand", i, nx(st));
	}

	return print_table() == 0 ? 0 : 1;
}
