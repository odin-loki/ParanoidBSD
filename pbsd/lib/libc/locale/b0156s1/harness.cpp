/*
 * Differential harness for batch b0156s1.
 */

import pbsd.lib.libc.locale.b0156s1;

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace P = pbsd::lib_libc_locale::b0156s1;

extern "C" {
typedef int ref_rune_t;

typedef struct {
	ref_rune_t	__min;
	ref_rune_t	__max;
	ref_rune_t	__map;
	unsigned long	*__types;
} ref_rune_entry;

typedef struct {
	int		__nranges;
	ref_rune_entry	*__ranges;
} ref_rune_range;

typedef struct {
	char		__magic[8];
	char		__encoding[32];
	ref_rune_t	(*__sgetrune)(const char *, size_t, char const **);
	int		(*__sputrune)(ref_rune_t, char *, size_t, char **);
	ref_rune_t	__invalid_rune;
	unsigned long	__runetype[256];
	ref_rune_t	__maplower[256];
	ref_rune_t	__mapupper[256];
	ref_rune_range	__runetype_ext;
	ref_rune_range	__maplower_ext;
	ref_rune_range	__mapupper_ext;
	void		*__variable;
	int		__variable_len;
} ref_rune_locale;

struct ref_xlocale_ctype {
	long		retain_count;
	void		(*destructor)(void *);
	char		locale_name[32];
	ref_rune_locale	*runes;
	size_t		(*__mbrtowc)(void *, const char *, size_t, void *);
	int		(*__mbsinit)(const void *);
	size_t		(*__mbsnrtowcs)(void *, const char **, size_t, size_t,
		    void *);
	size_t		(*__wcrtomb)(char *, int, void *);
	size_t		(*__wcsnrtombs)(char *, const void **, size_t, size_t,
		    void *);
	int		__mb_cur_max;
	int		__mb_sb_limit;
};

struct ref_xlocale {
	void		*components[8];
};

typedef struct ref_xlocale *ref_locale_t;

extern const ref_rune_locale	_DefaultRuneLocale;
extern const ref_rune_locale	*_CurrentRuneLocale;

void				pbsd_table_init_locales(void);
ref_rune_locale			*ref___runes_for_locale(ref_locale_t, int *);
}

enum {
	F_DEFAULT,
	F_CURRENT,
	F_RUNES,
	F_COUNT
};

static const char *const fname[F_COUNT] = {
	"_DefaultRuneLocale",
	"_CurrentRuneLocale",
	"__runes_for_locale",
};

static long long ncase[F_COUNT];
static long long nfail[F_COUNT];
static int nprinted[F_COUNT];

static constexpr unsigned char GUARD = 0x7f;
static constexpr long long SWEEP = 200000;
static constexpr size_t POOL = 32;

static uint64_t rng = 0xB0156A01ULL;

static uint64_t
rnd(void)
{
	rng ^= rng << 13;
	rng ^= rng >> 7;
	rng ^= rng << 17;
	return (rng);
}

static uint32_t
u32(uint32_t m)
{
	return ((uint32_t)(rnd() % m));
}

static void
report(int f, const char *why)
{
	nfail[f]++;
	if (nprinted[f]++ < 8)
		std::printf("  FAIL %-22s : %s\n", fname[f], why);
}

static bool
chk_default_field(const char *why, unsigned long pv, unsigned long rv)
{
	ncase[F_DEFAULT]++;
	if (pv != rv) {
		report(F_DEFAULT, why);
		return (false);
	}
	return (true);
}

static bool
chk_current_offset(ptrdiff_t po, ptrdiff_t ro)
{
	ncase[F_CURRENT]++;
	if (po != ro) {
		report(F_CURRENT, "offset");
		return (false);
	}
	return (true);
}

static bool
run_default_memcmp(void)
{
	ncase[F_DEFAULT]++;
	if (std::memcmp(&P::_DefaultRuneLocale, &_DefaultRuneLocale,
	    sizeof(ref_rune_locale)) != 0) {
		report(F_DEFAULT, "memcmp");
		return (false);
	}
	return (true);
}

static void
edge_default(void)
{
	static const unsigned idxs[] = {
		0x00, 0x07, 0x08, 0x09, 0x0a, 0x0d, 0x1f, 0x20, 0x27, 0x30,
		0x39, 0x40, 0x47, 0x50, 0x5a, 0x60, 0x67, 0x78, 0x7e, 0x7f,
		0x80, 0x9f, 0xa0, 0xbf, 0xc0, 0xdf, 0xe0, 0xff,
	};

	run_default_memcmp();
	chk_default_field("magic0",
	    (unsigned long)P::_DefaultRuneLocale.__magic[0],
	    (unsigned long)_DefaultRuneLocale.__magic[0]);
	chk_default_field("encoding",
	    (unsigned long)P::_DefaultRuneLocale.__encoding[0],
	    (unsigned long)_DefaultRuneLocale.__encoding[0]);
	chk_default_field("invalid",
	    (unsigned long)P::_DefaultRuneLocale.__invalid_rune,
	    (unsigned long)_DefaultRuneLocale.__invalid_rune);

	for (size_t i = 0; i < sizeof(idxs) / sizeof(idxs[0]); i++) {
		unsigned u = idxs[i];
		chk_default_field("runetype",
		    P::_DefaultRuneLocale.__runetype[u],
		    _DefaultRuneLocale.__runetype[u]);
		chk_default_field("maplower",
		    (unsigned long)P::_DefaultRuneLocale.__maplower[u],
		    (unsigned long)_DefaultRuneLocale.__maplower[u]);
		chk_default_field("mapupper",
		    (unsigned long)P::_DefaultRuneLocale.__mapupper[u],
		    (unsigned long)_DefaultRuneLocale.__mapupper[u]);
	}
}

static void
sweep_default(void)
{
	for (long long i = 0; i < SWEEP; i++) {
		unsigned u = u32(256);
		unsigned kind = u32(4);
		if (kind == 0) {
			chk_default_field("runetype",
			    P::_DefaultRuneLocale.__runetype[u],
			    _DefaultRuneLocale.__runetype[u]);
		} else if (kind == 1) {
			chk_default_field("maplower",
			    (unsigned long)P::_DefaultRuneLocale.__maplower[u],
			    (unsigned long)_DefaultRuneLocale.__maplower[u]);
		} else if (kind == 2) {
			chk_default_field("mapupper",
			    (unsigned long)P::_DefaultRuneLocale.__mapupper[u],
			    (unsigned long)_DefaultRuneLocale.__mapupper[u]);
		} else {
			run_default_memcmp();
		}
	}
}

static bool
run_current_check(void)
{
	const P::_RuneLocale *pd = P::_CurrentRuneLocale;
	const ref_rune_locale *rd = _CurrentRuneLocale;
	ptrdiff_t po = (const char *)pd - (const char *)&P::_DefaultRuneLocale;
	ptrdiff_t ro = (const char *)rd - (const char *)&_DefaultRuneLocale;

	if (!chk_current_offset(po, ro))
		return (false);

	ncase[F_CURRENT]++;
	if (std::memcmp(pd, &P::_DefaultRuneLocale, sizeof(P::_RuneLocale)) != 0) {
		report(F_CURRENT, "target");
		return (false);
	}
	return (true);
}

static void
edge_current(void)
{
	run_current_check();
}

static void
sweep_current(void)
{
	for (long long i = 0; i < SWEEP; i++)
		run_current_check();
}

struct rune_case {
	unsigned char		loc_buf[sizeof(ref_xlocale) + 16];
	unsigned char		ct_buf[sizeof(ref_xlocale_ctype) + 16];
	unsigned char		rune_buf[sizeof(ref_rune_locale) + 16];
	unsigned char		mb_buf[16];
	ref_locale_t		locale;
	ref_xlocale_ctype	*ct;
	ref_rune_locale		*rune;
	int			expect_mb;
};

static void
init_rune_case(rune_case *c, int mb_limit, unsigned char rune_tag)
{
	std::memset(c->loc_buf, GUARD, sizeof(c->loc_buf));
	std::memset(c->ct_buf, GUARD, sizeof(c->ct_buf));
	std::memset(c->rune_buf, GUARD, sizeof(c->rune_buf));
	std::memset(c->mb_buf, GUARD, sizeof(c->mb_buf));

	c->locale = (ref_locale_t)(c->loc_buf + 8);
	c->ct = (ref_xlocale_ctype *)(c->ct_buf + 8);
	c->rune = (ref_rune_locale *)(c->rune_buf + 8);
	c->locale->components[1] = c->ct;
	c->ct->runes = c->rune;
	c->ct->__mb_sb_limit = mb_limit;
	c->rune->__invalid_rune = (ref_rune_t)(0x10000 + rune_tag);
	c->rune->__runetype[rune_tag] = 0xdeadbeefUL;
	c->expect_mb = mb_limit;
}

static unsigned
rune_tag_from_case(rune_case *c)
{
	return ((unsigned)(c->rune->__invalid_rune & 0xff));
}

static bool
run_runes_for_locale(rune_case *c, ref_locale_t loc)
{
	unsigned char mb_p[16], mb_r[16];
	unsigned char rune_snap[sizeof(ref_rune_locale) + 16];
	ref_rune_locale *rp;
	P::_RuneLocale *pp;
	ref_rune_locale *exp_r;
	P::_RuneLocale *exp_p;
	bool builtin;
	int f = F_RUNES;

	builtin = (loc == nullptr || loc == (ref_locale_t)-1);
	exp_r = builtin ? (ref_rune_locale *)&_DefaultRuneLocale : c->rune;
	exp_p = builtin ? (P::_RuneLocale *)&P::_DefaultRuneLocale :
	    (P::_RuneLocale *)c->rune;

	std::memcpy(mb_p, c->mb_buf, sizeof(mb_p));
	std::memcpy(mb_r, c->mb_buf, sizeof(mb_r));
	std::memcpy(rune_snap, c->rune_buf, sizeof(rune_snap));

	rp = ref___runes_for_locale(loc, (int *)(mb_r + 4));
	pp = P::__runes_for_locale((P::locale_t)loc, (int *)(mb_p + 4));

	ncase[f]++;
	if (std::memcmp(mb_p, mb_r, sizeof(mb_p)) != 0) {
		report(f, "mb_guard");
		return (false);
	}

	ptrdiff_t ro = (const char *)rp - (const char *)exp_r;
	ptrdiff_t po = (const char *)pp - (const char *)exp_p;
	if (ro != po) {
		report(f, "rune_offset");
		return (false);
	}

	int mbr = *(int *)(mb_r + 4);
	int mbp = *(int *)(mb_p + 4);
	ncase[f]++;
	if (mbr != mbp) {
		report(f, "mb_limit");
		return (false);
	}
	if (mbr != c->expect_mb) {
		report(f, "mb_expect");
		return (false);
	}

	if (!builtin) {
		ncase[f]++;
		if (std::memcmp(c->rune_buf, rune_snap, sizeof(c->rune_buf)) != 0) {
			report(f, "rune_guard");
			return (false);
		}

		ncase[f]++;
		if (rp->__invalid_rune != c->rune->__invalid_rune) {
			report(f, "rune_field");
			return (false);
		}

		unsigned utag = rune_tag_from_case(c);
		ncase[f]++;
		if (rp->__runetype[utag] != c->rune->__runetype[utag]) {
			report(f, "rune_type");
			return (false);
		}
	}

	return (true);
}

static void
edge_runes(void)
{
	rune_case c;

	init_rune_case(&c, 0, 0x00);
	run_runes_for_locale(&c, c.locale);

	init_rune_case(&c, 1, 0x09);
	c.expect_mb = 1;
	run_runes_for_locale(&c, nullptr);

	init_rune_case(&c, 127, 0x7f);
	c.expect_mb = 127;
	run_runes_for_locale(&c, (ref_locale_t)-1);

	init_rune_case(&c, -1, 0x80);
	run_runes_for_locale(&c, c.locale);

	init_rune_case(&c, INT_MAX, 0xff);
	run_runes_for_locale(&c, c.locale);

	init_rune_case(&c, INT_MIN, 0x20);
	run_runes_for_locale(&c, c.locale);

	init_rune_case(&c, 255, 0x41);
	run_runes_for_locale(&c, c.locale);
}

static rune_case pool[POOL];

static void
sweep_runes(void)
{
	for (long long i = 0; i < SWEEP; i++) {
		unsigned slot = u32(POOL);
		int mb = (int)(u32(0x10000) - 0x8000);
		unsigned tag = u32(256);
		unsigned mode = u32(5);
		ref_locale_t loc;

		init_rune_case(&pool[slot], mb, (unsigned char)tag);

		switch (mode) {
		case 0:
			loc = nullptr;
			pool[slot].expect_mb = 1;
			break;
		case 1:
			loc = (ref_locale_t)-1;
			pool[slot].expect_mb = 127;
			break;
		default:
			loc = pool[slot].locale;
			break;
		}

		run_runes_for_locale(&pool[slot], loc);
	}
}

int
main(void)
{
	int i;
	long long total_fail = 0;

	pbsd_table_init_locales();
	P::table_init_locales();

	edge_default();
	edge_current();
	edge_runes();

	sweep_default();
	sweep_current();
	sweep_runes();

	std::printf("\n%-24s %12s %12s\n", "function", "cases", "failures");
	for (i = 0; i < F_COUNT; i++) {
		std::printf("%-24s %12lld %12lld\n", fname[i], ncase[i],
		    nfail[i]);
		total_fail += nfail[i];
	}

	return (total_fail == 0 ? 0 : 1);
}
