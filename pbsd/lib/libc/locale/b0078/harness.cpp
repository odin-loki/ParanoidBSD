/*
 * PBSD batch b0078 -- differential test: port vs. ref_ oracle.
 */

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <iterator>

import pbsd.lib.libc.locale.b0078;

namespace P = pbsd::lib_libc_locale::b0078;

extern "C" {
typedef union {
	char		__mbstate8[128];
	long long	_mbstateL;
} ref_mbstate_t;

typedef struct {
	int		__min;
	int		__max;
	int		__map;
	unsigned long	*__types;
} ref_rune_entry;

typedef struct {
	int		__nranges;
	ref_rune_entry	*__ranges;
} ref_rune_range;

typedef struct {
	char		__magic[8];
	char		__encoding[32];
	int		(*__sgetrune)(const char *, size_t, char const **);
	int		(*__sputrune)(int, char *, size_t, char **);
	int		__invalid_rune;
	unsigned long	__runetype[256];
	int		__maplower[256];
	int		__mapupper[256];
	ref_rune_range	__runetype_ext;
	ref_rune_range	__maplower_ext;
	ref_rune_range	__mapupper_ext;
	void		*__variable;
	int		__variable_len;
} ref_rune_locale;

struct ref_xlocale_ctype {
	ref_rune_locale	*runes;
	size_t		(*__mbsnrtowcs)(wchar_t *, const char **, size_t, size_t,
		    ref_mbstate_t *);
	int		__mb_cur_max;
	ref_mbstate_t	mbsnrtowcs;
};

struct ref_xlocale {
	void		*components[6];
};

typedef struct ref_xlocale *ref_locale_t;

extern struct ref_xlocale ref_global_locale;
extern struct ref_xlocale ref_alt_locale;
extern struct ref_xlocale_ctype ref_global_ctype;
extern struct ref_xlocale_ctype ref_alt_ctype;

unsigned long	ref____runetype_l(int, ref_locale_t);
unsigned long	ref____runetype(int);
int		ref____mb_cur_max(void);
int		ref____mb_cur_max_l(ref_locale_t);
wint_t		ref_nextwctype_l(wint_t, unsigned long, ref_locale_t);
wint_t		ref_nextwctype(wint_t, unsigned long);
size_t		ref_mbsnrtowcs_l(wchar_t *, const char **, size_t, size_t,
		    ref_mbstate_t *, ref_locale_t);
size_t		ref_mbsnrtowcs(wchar_t *, const char **, size_t, size_t,
		    ref_mbstate_t *);
size_t		ref___mbsnrtowcs_std(wchar_t *, const char **, size_t, size_t,
		    ref_mbstate_t *, size_t (*)(wchar_t *, const char *, size_t,
		    ref_mbstate_t *));
size_t		test_mbrtowc(wchar_t *, const char *, size_t, ref_mbstate_t *);
void		ref_locale_init(struct ref_xlocale *, struct ref_xlocale_ctype *,
		    ref_rune_locale *, int);
}

enum {
	F___RUNETYPE_L,
	F___RUNETYPE,
	F___MB_CUR_MAX,
	F___MB_CUR_MAX_L,
	F_NEXTWCTYPE_L,
	F_NEXTWCTYPE,
	F_MBSNRTOWCS_L,
	F_MBSNRTOWCS,
	F___MBSNRTOWCS_STD,
	F_COUNT
};

static const char *const fname[F_COUNT] = {
	"___runetype_l", "___runetype",
	"___mb_cur_max", "___mb_cur_max_l",
	"nextwctype_l", "nextwctype",
	"mbsnrtowcs_l", "mbsnrtowcs",
	"__mbsnrtowcs_std"
};

static long long ncase[F_COUNT];
static long long nfail[F_COUNT];
static int nprinted[F_COUNT];

static constexpr int CACHED_RUNES = 256;
static constexpr unsigned char GUARD = 0x7f;
static constexpr wchar_t WGUARD = (wchar_t)0x7f7f;
static constexpr long long SWEEP = 200000;

static constexpr P::wctype_t T_ALPHA = 0x0001;
static constexpr P::wctype_t T_DIGIT = 0x0002;
static constexpr P::wctype_t T_SPACE = 0x0004;
static constexpr P::wctype_t T_UPPER = 0x0008;
static constexpr P::wctype_t T_PUNCT = 0x0010;

struct RuneTables {
	unsigned long		cached[CACHED_RUNES];
	P::_RuneEntry		ranges[8];
	unsigned long		types[8][32];
	int			nranges;
};

struct Env {
	RuneTables		rtab;
	ref_rune_locale		ref_runes;
	P::_RuneLocale		port_runes;
	ref_xlocale_ctype	ref_ctype;
	P::xlocale_ctype	port_ctype;
	ref_xlocale		ref_loc;
	P::xlocale		port_loc;
	int			mb_cur_max;
};

static Env genv;
static Env altenv;

static uint64_t rng = 0xc0ffeeULL;

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
		std::printf("  FAIL %-18s : %s\n", fname[f], why);
}

static void
mb_copy(const P::mbstate_t &s, ref_mbstate_t &d)
{

	std::memset(&d, 0, sizeof(d));
	std::memcpy(&d, &s, sizeof(s));
}

static void
mb_copy(const ref_mbstate_t &s, P::mbstate_t &d)
{

	std::memset(&d, 0, sizeof(d));
	std::memcpy(&d, &s, sizeof(s));
}

static void
apply_rune_tables(Env &e, const RuneTables &t)
{
	int i;

	std::memcpy(e.ref_runes.__runetype, t.cached, sizeof(t.cached));
	std::memcpy(e.port_runes.__runetype, t.cached, sizeof(t.cached));
	e.rtab = t;
	for (i = 0; i < t.nranges; i++) {
		e.ref_runes.__runetype_ext.__ranges[i].__min = t.ranges[i].__min;
		e.ref_runes.__runetype_ext.__ranges[i].__max = t.ranges[i].__max;
		e.ref_runes.__runetype_ext.__ranges[i].__map = t.ranges[i].__map;
		e.ref_runes.__runetype_ext.__ranges[i].__types = t.ranges[i].__types;
		e.port_runes.__runetype_ext.__ranges[i] = t.ranges[i];
	}
	e.ref_runes.__runetype_ext.__nranges = t.nranges;
	e.port_runes.__runetype_ext.__nranges = t.nranges;
	e.ref_runes.__runetype_ext.__ranges =
	    (ref_rune_entry *)e.rtab.ranges;
	e.port_runes.__runetype_ext.__ranges = e.rtab.ranges;
}

static void
env_init(Env &e, int mbmax)
{

	std::memset(&e, 0, sizeof(e));
	e.mb_cur_max = mbmax;
	ref_locale_init(&e.ref_loc, &e.ref_ctype, &e.ref_runes, mbmax);
	P::pbsd_locale_init(&e.port_loc, &e.port_ctype, &e.port_runes, mbmax);
	e.ref_runes.__runetype_ext.__ranges =
	    (ref_rune_entry *)e.rtab.ranges;
	e.port_runes.__runetype_ext.__ranges = e.rtab.ranges;
}

static void
use_env(Env &e)
{

	ref_global_locale = e.ref_loc;
	ref_global_ctype = e.ref_ctype;
	P::pbsd_set_active_locale(&e.port_loc);
}

static void
setup_default_locale(void)
{
	RuneTables t;

	std::memset(&t, 0, sizeof(t));
	for (int i = 0; i < CACHED_RUNES; i++)
		t.cached[i] = (i >= 'A' && i <= 'Z') ? T_UPPER :
		    (i >= 'a' && i <= 'z') ? T_ALPHA :
		    (i >= '0' && i <= '9') ? T_DIGIT :
		    (i == ' ') ? T_SPACE : 0;
	t.cached[0x80] = T_PUNCT;
	t.cached[0xff] = T_PUNCT;

	t.nranges = 3;
	t.ranges[0] = { 300, 305, 0, t.types[0] };
	for (int i = 0; i <= 5; i++)
		t.types[0][i] = (i & 1) ? T_ALPHA : T_DIGIT;
	t.ranges[1] = { 400, 402, T_SPACE, nullptr };
	t.ranges[2] = { 500, 510, T_PUNCT, t.types[2] };
	for (int i = 0; i <= 10; i++)
		t.types[2][i] = (i == 3 || i == 7) ? T_UPPER : 0;

	env_init(genv, 4);
	apply_rune_tables(genv, t);
	env_init(altenv, 1);
	apply_rune_tables(altenv, t);
	altenv.ref_ctype.__mb_cur_max = 1;
	altenv.port_ctype.__mb_cur_max = 1;
	use_env(genv);
}

static bool
chk_runetype(int f, int c, unsigned long pv, unsigned long rv)
{

	ncase[f]++;
	if (pv != rv) {
		report(f, "return mismatch");
		return (false);
	}
	return (true);
}

static void
test_runetype_edge(void)
{
	unsigned long pv, rv;
	Env saved = genv;

	/* EOF and negatives */
	pv = P::___runetype_l(EOF, &genv.port_loc);
	rv = ref____runetype_l(EOF, &genv.ref_loc);
	chk_runetype(F___RUNETYPE_L, EOF, pv, rv);
	pv = P::___runetype_l(-1, &genv.port_loc);
	rv = ref____runetype_l(-1, &genv.ref_loc);
	chk_runetype(F___RUNETYPE_L, -1, pv, rv);
	pv = P::___runetype_l(-100, nullptr);
	rv = ref____runetype_l(-100, nullptr);
	chk_runetype(F___RUNETYPE_L, -100, pv, rv);

	/* cached-range codepoints are not in ext table */
	pv = P::___runetype_l('A', &genv.port_loc);
	rv = ref____runetype_l('A', &genv.ref_loc);
	chk_runetype(F___RUNETYPE_L, 'A', pv, rv);

	/* __types hit */
	pv = P::___runetype_l(302, &genv.port_loc);
	rv = ref____runetype_l(302, &genv.ref_loc);
	chk_runetype(F___RUNETYPE_L, 302, pv, rv);

	/* __map-only hit */
	pv = P::___runetype_l(401, &genv.port_loc);
	rv = ref____runetype_l(401, &genv.ref_loc);
	chk_runetype(F___RUNETYPE_L, 401, pv, rv);

	/* miss below */
	pv = P::___runetype_l(250, &genv.port_loc);
	rv = ref____runetype_l(250, &genv.ref_loc);
	chk_runetype(F___RUNETYPE_L, 250, pv, rv);

	/* miss above */
	pv = P::___runetype_l(600, &genv.port_loc);
	rv = ref____runetype_l(600, &genv.ref_loc);
	chk_runetype(F___RUNETYPE_L, 600, pv, rv);

	/* non-_l via active locale */
	use_env(genv);
	pv = P::___runetype(503);
	rv = ref____runetype(503);
	chk_runetype(F___RUNETYPE, 503, pv, rv);

	/* binary-search boundary: exactly on range edges */
	pv = P::___runetype_l(300, &genv.port_loc);
	rv = ref____runetype_l(300, &genv.ref_loc);
	chk_runetype(F___RUNETYPE_L, 300, pv, rv);
	pv = P::___runetype_l(305, &genv.port_loc);
	rv = ref____runetype_l(305, &genv.ref_loc);
	chk_runetype(F___RUNETYPE_L, 305, pv, rv);

	genv = saved;
	use_env(genv);
}

static void
sweep_runetype(int f, bool use_l)
{
	int c;
	unsigned long pv, rv;

	for (long long i = 0; i < SWEEP; i++) {
		switch (u32(8)) {
		case 0:
			c = (int)u32(CACHED_RUNES);
			break;
		case 1:
			c = 280 + (int)u32(80);
			break;
		case 2:
			c = 390 + (int)u32(30);
			break;
		case 3:
			c = 495 + (int)u32(30);
			break;
		case 4:
			c = -1 - (int)u32(20);
			break;
		case 5:
			c = EOF;
			break;
		default:
			c = (int)(rnd() & 0x7ff);
			break;
		}
		if (use_l) {
			ref_locale_t rl = (u32(3) == 0) ? nullptr :
			    ((u32(2) == 0) ? &altenv.ref_loc : &genv.ref_loc);
			P::locale_t pl = (rl == &altenv.ref_loc) ? &altenv.port_loc :
			    (rl == nullptr ? nullptr : &genv.port_loc);
			pv = P::___runetype_l(c, pl);
			rv = ref____runetype_l(c, rl);
		} else {
			use_env((u32(2) == 0) ? altenv : genv);
			pv = P::___runetype(c);
			rv = ref____runetype(c);
		}
		chk_runetype(f, c, pv, rv);
	}
}

static bool
chk_wint(int f, wint_t pv, wint_t rv)
{

	ncase[f]++;
	if (pv != rv) {
		report(f, "return mismatch");
		return (false);
	}
	return (true);
}

static void
test_nextwctype_edge(void)
{
	wint_t pv, rv;

	/* cached scan from -1 */
	pv = P::nextwctype_l(-1, T_UPPER, &genv.port_loc);
	rv = ref_nextwctype_l(-1, T_UPPER, &genv.ref_loc);
	chk_wint(F_NEXTWCTYPE_L, pv, rv);

	/* cached miss for rare mask */
	pv = P::nextwctype_l(50, (P::wctype_t)0x80000000UL, &genv.port_loc);
	rv = ref_nextwctype_l(50, 0x80000000UL, &genv.ref_loc);
	chk_wint(F_NEXTWCTYPE_L, pv, rv);

	/* below first ext range -> noinc path */
	pv = P::nextwctype_l(200, T_ALPHA, &genv.port_loc);
	rv = ref_nextwctype_l(200, T_ALPHA, &genv.ref_loc);
	chk_wint(F_NEXTWCTYPE_L, pv, rv);

	/* inside ext range with __types */
	pv = P::nextwctype_l(301, T_ALPHA, &genv.port_loc);
	rv = ref_nextwctype_l(301, T_ALPHA, &genv.ref_loc);
	chk_wint(F_NEXTWCTYPE_L, pv, rv);

	/* __map-only range */
	pv = P::nextwctype_l(399, T_SPACE, &genv.port_loc);
	rv = ref_nextwctype_l(399, T_SPACE, &genv.ref_loc);
	chk_wint(F_NEXTWCTYPE_L, pv, rv);

	/* second range via while loop */
	pv = P::nextwctype_l(405, T_UPPER, &genv.port_loc);
	rv = ref_nextwctype_l(405, T_UPPER, &genv.ref_loc);
	chk_wint(F_NEXTWCTYPE_L, pv, rv);

	/* past all */
	pv = P::nextwctype_l(2000, T_ALPHA, &genv.port_loc);
	rv = ref_nextwctype_l(2000, T_ALPHA, &genv.ref_loc);
	chk_wint(F_NEXTWCTYPE_L, pv, rv);

	/* at _CACHED_RUNES-1 boundary */
	pv = P::nextwctype_l(254, T_PUNCT, &genv.port_loc);
	rv = ref_nextwctype_l(254, T_PUNCT, &genv.ref_loc);
	chk_wint(F_NEXTWCTYPE_L, pv, rv);

	use_env(genv);
	pv = P::nextwctype(100, T_DIGIT);
	rv = ref_nextwctype(100, T_DIGIT);
	chk_wint(F_NEXTWCTYPE, pv, rv);
}

static void
sweep_nextwctype(int f, bool use_l)
{
	static const P::wctype_t masks[] = {
		T_ALPHA, T_DIGIT, T_SPACE, T_UPPER, T_PUNCT,
		T_ALPHA | T_DIGIT, 0UL, 0xffffffffUL
	};
	wint_t wc, pv, rv;
	P::wctype_t wct;

	for (long long i = 0; i < SWEEP; i++) {
		wct = masks[u32(8)];
		switch (u32(8)) {
		case 0:
			wc = (wint_t)u32(CACHED_RUNES);
			break;
		case 1:
			wc = 250 + (wint_t)u32(100);
			break;
		case 2:
			wc = 390 + (wint_t)u32(30);
			break;
		case 3:
			wc = 495 + (wint_t)u32(30);
			break;
		case 4:
			wc = (wint_t)(2000 + u32(100));
			break;
		case 5:
			wc = -1;
			break;
		default:
			wc = (wint_t)(rnd() & 0xfff);
			break;
		}
		if (use_l) {
			ref_locale_t rl = (u32(3) == 0) ? nullptr : &genv.ref_loc;
			P::locale_t pl = (rl == nullptr) ? nullptr : &genv.port_loc;
			pv = P::nextwctype_l(wc, wct, pl);
			rv = ref_nextwctype_l(wc, wct, rl);
		} else {
			use_env(genv);
			pv = P::nextwctype(wc, wct);
			rv = ref_nextwctype(wc, wct);
		}
		chk_wint(f, pv, rv);
	}
}

static void
test_mb_cur_max_edge(void)
{
	int pv, rv;

	use_env(genv);
	pv = P::___mb_cur_max();
	rv = ref____mb_cur_max();
	ncase[F___MB_CUR_MAX]++;
	if (pv != rv)
		report(F___MB_CUR_MAX, "return mismatch");

	pv = P::___mb_cur_max_l(&genv.port_loc);
	rv = ref____mb_cur_max_l(&genv.ref_loc);
	ncase[F___MB_CUR_MAX_L]++;
	if (pv != rv)
		report(F___MB_CUR_MAX_L, "return mismatch");

	pv = P::___mb_cur_max_l(nullptr);
	rv = ref____mb_cur_max_l(nullptr);
	ncase[F___MB_CUR_MAX_L]++;
	if (pv != rv)
		report(F___MB_CUR_MAX_L, "null locale mismatch");

	use_env(altenv);
	pv = P::___mb_cur_max_l(&altenv.port_loc);
	rv = ref____mb_cur_max_l(&altenv.ref_loc);
	ncase[F___MB_CUR_MAX_L]++;
	if (pv != rv)
		report(F___MB_CUR_MAX_L, "alt locale mismatch");
	use_env(genv);
}

static void
sweep_mb_cur_max(int f, bool use_l)
{

	for (long long i = 0; i < SWEEP; i++) {
		int want = 1 + (int)u32(8);
		genv.port_ctype.__mb_cur_max = want;
		genv.ref_ctype.__mb_cur_max = want;
		if (use_l) {
			int pv = P::___mb_cur_max_l((u32(2) == 0) ?
			    nullptr : &genv.port_loc);
			int rv = ref____mb_cur_max_l((u32(2) == 0) ?
			    nullptr : &genv.ref_loc);
			ncase[f]++;
			if (pv != rv)
				report(f, "return mismatch");
		} else {
			use_env(genv);
			int pv = P::___mb_cur_max();
			int rv = ref____mb_cur_max();
			ncase[f]++;
			if (pv != rv)
				report(f, "return mismatch");
		}
	}
}

static void
fill_guard(char *b, size_t n)
{

	std::memset(b, GUARD, n);
}

static void
fill_wguard(wchar_t *b, size_t n)
{

	for (size_t i = 0; i < n; i++)
		b[i] = WGUARD;
}

static bool
bufs_eq(const char *a, const char *b, size_t n)
{

	return (std::memcmp(a, b, n) == 0);
}

static bool
wbufs_eq(const wchar_t *a, const wchar_t *b, size_t n)
{

	return (std::memcmp(a, b, n * sizeof(wchar_t)) == 0);
}

struct MbsCase {
	const char	*label;
	const char	*bytes;
	size_t		nms;
	size_t		len;
	bool		dst_null;
	bool		ps_null;
};

static void
run_mbs_std_case(const MbsCase &tc)
{
	char in_p[96], in_r[96];
	wchar_t out_p[48], out_r[48];
	const char *sp, *sr;
	size_t pv, rv;
	P::mbstate_t ps_p, ps_r;
	ref_mbstate_t ps_ref_p, ps_ref_r;
	int f = F___MBSNRTOWCS_STD;

	ncase[f]++;
	fill_guard(in_p, sizeof(in_p));
	fill_guard(in_r, sizeof(in_r));
	std::memcpy(in_p + 8, tc.bytes, std::strlen(tc.bytes) + 1);
	std::memcpy(in_r + 8, tc.bytes, std::strlen(tc.bytes) + 1);
	fill_wguard(out_p, std::size(out_p));
	fill_wguard(out_r, std::size(out_r));
	std::memset(&ps_p, 0x55, sizeof(ps_p));
	std::memset(&ps_r, 0x55, sizeof(ps_r));
	mb_copy(ps_p, ps_ref_p);
	mb_copy(ps_r, ps_ref_r);

	sp = in_p + 8;
	sr = in_r + 8;
	pv = P::__mbsnrtowcs_std(tc.dst_null ? nullptr : out_p, &sp, tc.nms,
	    tc.len, &ps_p, P::pbsd_test_mbrtowc);
	rv = ref___mbsnrtowcs_std(tc.dst_null ? nullptr : out_r, &sr, tc.nms,
	    tc.len, &ps_ref_p, test_mbrtowc);

	if (pv != rv) {
		report(f, tc.label);
		return;
	}
	if (!tc.dst_null && !wbufs_eq(out_p, out_r, std::size(out_p))) {
		report(f, "wchar buffer mismatch");
		return;
	}
	if ((sp == nullptr) != (sr == nullptr) ||
	    (sp != nullptr && sr != nullptr && (sp - in_p) != (sr - in_r))) {
		report(f, "src pointer mismatch");
		return;
	}
	if (!bufs_eq(in_p, in_r, sizeof(in_p))) {
		report(f, "input buffer mismatch");
		return;
	}
}

static void
run_mbs_wrap_case(int f, bool use_l, const MbsCase &tc)
{
	char in_p[96], in_r[96];
	wchar_t out_p[48], out_r[48];
	const char *sp, *sr;
	size_t pv, rv;
	P::mbstate_t ps_p, ps_r, loc_p, loc_r;
	ref_mbstate_t ps_ref_p, ps_ref_r, loc_ref_p, loc_ref_r;

	ncase[f]++;
	fill_guard(in_p, sizeof(in_p));
	fill_guard(in_r, sizeof(in_r));
	std::memcpy(in_p + 8, tc.bytes, std::strlen(tc.bytes) + 1);
	std::memcpy(in_r + 8, tc.bytes, std::strlen(tc.bytes) + 1);
	fill_wguard(out_p, std::size(out_p));
	fill_wguard(out_r, std::size(out_r));
	std::memset(&ps_p, 0x33, sizeof(ps_p));
	std::memset(&ps_r, 0x33, sizeof(ps_r));
	std::memset(&loc_p, 0x44, sizeof(loc_p));
	mb_copy(loc_p, loc_ref_p);
	genv.port_ctype.mbsnrtowcs = loc_p;
	std::memcpy(&genv.ref_ctype.mbsnrtowcs, &loc_ref_p, sizeof(loc_ref_p));
	mb_copy(ps_p, ps_ref_p);

	sp = in_p + 8;
	sr = in_r + 8;
	if (use_l) {
		pv = P::mbsnrtowcs_l(tc.dst_null ? nullptr : out_p, &sp, tc.nms,
		    tc.len, tc.ps_null ? nullptr : &ps_p, &genv.port_loc);
		rv = ref_mbsnrtowcs_l(tc.dst_null ? nullptr : out_r, &sr,
		    tc.nms, tc.len, tc.ps_null ? nullptr : &ps_ref_p,
		    &genv.ref_loc);
	} else {
		use_env(genv);
		pv = P::mbsnrtowcs(tc.dst_null ? nullptr : out_p, &sp, tc.nms,
		    tc.len, tc.ps_null ? nullptr : &ps_p);
		rv = ref_mbsnrtowcs(tc.dst_null ? nullptr : out_r, &sr, tc.nms,
		    tc.len, tc.ps_null ? nullptr : &ps_ref_p);
	}

	if (pv != rv) {
		report(f, tc.label);
		return;
	}
	if (!tc.dst_null && !wbufs_eq(out_p, out_r, std::size(out_p))) {
		report(f, "wchar buffer mismatch");
		return;
	}
	if ((sp == nullptr) != (sr == nullptr) ||
	    (sp != nullptr && sr != nullptr && (sp - in_p) != (sr - in_r))) {
		report(f, "src pointer mismatch");
		return;
	}
	if (!bufs_eq(in_p, in_r, sizeof(in_p))) {
		report(f, "input buffer mismatch");
		return;
	}
	if (tc.ps_null) {
		ref_mbstate_t ref_st;
		P::mbstate_t port_st;

		std::memcpy(&ref_st, &genv.ref_ctype.mbsnrtowcs, sizeof(ref_st));
		port_st = genv.port_ctype.mbsnrtowcs;
		mb_copy(port_st, ref_st);
		if (std::memcmp(&genv.port_ctype.mbsnrtowcs, &port_st,
		    sizeof(port_st)) != 0 ||
		    std::memcmp(&genv.ref_ctype.mbsnrtowcs, &ref_st,
		    sizeof(ref_st)) != 0)
			report(f, "locale mbstate mismatch");
	} else if (std::memcmp(&ps_p, &ps_ref_p, sizeof(ps_p)) != 0) {
		report(f, "ps mismatch");
	}
}

static void
test_mbsnrtowcs_edge(void)
{
	static const MbsCase cases[] = {
		{ "empty", "", 0, 0, false, true },
		{ "nul only", "", 1, 4, false, true },
		{ "ascii", "abc", 3, 10, false, true },
		{ "count", "abc\xff", 4, 0, true, true },
		{ "nul mid", "a\0b", 3, 5, false, true },
		{ "invalid", "\xff", 1, 4, false, true },
		{ "incomplete", "\xc2", 1, 4, false, true },
		{ "utf8", "\xc3\xa9", 2, 4, false, true },
		{ "len0", "abc", 3, 0, false, true },
		{ "nms1", "abc", 1, 8, false, true },
		{ "highbit", "\x80\x81", 2, 4, false, false },
		{ "trunc2", "\xc3\xa9z", 2, 8, false, true },
		{ "dstnull_inv", "\xff", 1, 0, true, true },
		{ "dstnull_ok", "xy", 2, 0, true, true },
	};

	for (const auto &tc : cases) {
		run_mbs_std_case(tc);
		run_mbs_wrap_case(F_MBSNRTOWCS_L, true, tc);
		run_mbs_wrap_case(F_MBSNRTOWCS, false, tc);
	}
}

static void
rand_bytes(char *dst, size_t n)
{
	size_t i;

	for (i = 0; i < n; i++) {
		switch (u32(6)) {
		case 0:
			dst[i] = (char)u32(128);
			break;
		case 1:
			dst[i] = (char)(0x80 + u32(128));
			break;
		case 2:
			dst[i] = (char)(0xc0 + u32(32));
			break;
		default:
			dst[i] = (char)(32 + u32(96));
			break;
		}
	}
	dst[n] = '\0';
}

static void
sweep_mbs_std(void)
{
	char in_p[80], in_r[80];
	wchar_t out_p[40], out_r[40];
	const char *sp, *sr;
	size_t nms, len;
	bool dst_null;
	int f = F___MBSNRTOWCS_STD;

	for (long long i = 0; i < SWEEP; i++) {
		size_t blen = u32(16);
		ncase[f]++;
		fill_guard(in_p, sizeof(in_p));
		fill_guard(in_r, sizeof(in_r));
		rand_bytes(in_p + 4, blen);
		std::memcpy(in_r + 4, in_p + 4, blen + 1);
		if (u32(5) == 0)
			in_p[4 + u32((uint32_t)blen + 1)] = '\0';
		nms = u32((uint32_t)blen + 4);
		len = u32(12);
		dst_null = (u32(4) == 0);
		fill_wguard(out_p, std::size(out_p));
		fill_wguard(out_r, std::size(out_r));
		sp = in_p + 4;
		sr = in_r + 4;
		P::mbstate_t ps_p{}, ps_r{};
		ref_mbstate_t ps_ref_p{}, ps_ref_r{};
		size_t pv = P::__mbsnrtowcs_std(dst_null ? nullptr : out_p, &sp,
		    nms, len, &ps_p, P::pbsd_test_mbrtowc);
		size_t rv = ref___mbsnrtowcs_std(dst_null ? nullptr : out_r, &sr,
		    nms, len, &ps_ref_p, test_mbrtowc);
		if (pv != rv || (!dst_null && !wbufs_eq(out_p, out_r,
		    std::size(out_p))) ||
		    (sp == nullptr) != (sr == nullptr) ||
		    (sp != nullptr && sr != nullptr &&
		    (sp - in_p) != (sr - in_r)) ||
		    !bufs_eq(in_p, in_r, sizeof(in_p)))
			report(f, "random");
	}
}

static void
sweep_mbs_wrap(int f, bool use_l)
{
	char in_p[80], in_r[80];
	wchar_t out_p[40], out_r[40];
	const char *sp, *sr;
	size_t nms, len;
	bool dst_null, ps_null;

	for (long long i = 0; i < SWEEP; i++) {
		size_t blen = u32(16);
		ncase[f]++;
		fill_guard(in_p, sizeof(in_p));
		fill_guard(in_r, sizeof(in_r));
		rand_bytes(in_p + 4, blen);
		std::memcpy(in_r + 4, in_p + 4, blen + 1);
		nms = u32((uint32_t)blen + 4);
		len = u32(12);
		dst_null = (u32(4) == 0);
		ps_null = (u32(3) == 0);
		fill_wguard(out_p, std::size(out_p));
		fill_wguard(out_r, std::size(out_r));
		sp = in_p + 4;
		sr = in_r + 4;
		P::mbstate_t ps_p{}, ps_r{};
		ref_mbstate_t ps_ref_p{}, ps_ref_r{};
		std::memset(&genv.port_ctype.mbsnrtowcs, 0x12,
		    sizeof(genv.port_ctype.mbsnrtowcs));
		std::memset(&genv.ref_ctype.mbsnrtowcs, 0x12,
		    sizeof(genv.ref_ctype.mbsnrtowcs));
		size_t pv, rv;
		if (use_l) {
			pv = P::mbsnrtowcs_l(dst_null ? nullptr : out_p, &sp,
			    nms, len, ps_null ? nullptr : &ps_p,
			    &genv.port_loc);
			rv = ref_mbsnrtowcs_l(dst_null ? nullptr : out_r, &sr,
			    nms, len, ps_null ? nullptr : &ps_ref_p,
			    &genv.ref_loc);
		} else {
			use_env(genv);
			pv = P::mbsnrtowcs(dst_null ? nullptr : out_p, &sp,
			    nms, len, ps_null ? nullptr : &ps_p);
			rv = ref_mbsnrtowcs(dst_null ? nullptr : out_r, &sr,
			    nms, len, ps_null ? nullptr : &ps_ref_p);
		}
		if (pv != rv || (!dst_null && !wbufs_eq(out_p, out_r,
		    std::size(out_p))) ||
		    (sp == nullptr) != (sr == nullptr) ||
		    (sp != nullptr && sr != nullptr &&
		    (sp - in_p) != (sr - in_r)) ||
		    !bufs_eq(in_p, in_r, sizeof(in_p)))
			report(f, "random");
	}
}

int
main()
{
	setup_default_locale();

	test_runetype_edge();
	sweep_runetype(F___RUNETYPE_L, true);
	sweep_runetype(F___RUNETYPE, false);

	test_nextwctype_edge();
	sweep_nextwctype(F_NEXTWCTYPE_L, true);
	sweep_nextwctype(F_NEXTWCTYPE, false);

	test_mb_cur_max_edge();
	sweep_mb_cur_max(F___MB_CUR_MAX, false);
	sweep_mb_cur_max(F___MB_CUR_MAX_L, true);

	test_mbsnrtowcs_edge();
	sweep_mbs_std();
	sweep_mbs_wrap(F_MBSNRTOWCS_L, true);
	sweep_mbs_wrap(F_MBSNRTOWCS, false);

	std::printf("\n%-22s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < F_COUNT; i++)
		std::printf("%-22s %12lld %12lld\n", fname[i], ncase[i],
		    nfail[i]);

	long long total_fail = 0;
	for (int i = 0; i < F_COUNT; i++)
		total_fail += nfail[i];
	return (total_fail == 0 ? 0 : 1);
}
