/*
 * PBSD batch b0039 -- differential test: port vs. ref_ oracle.
 *
 * Every case runs both implementations against byte-identical inputs held in
 * two separate buffers and compares the return value, errno, the whole
 * destination buffer (including the guard bytes past the nominal write
 * window), the updated *src offset, the caller's mbstate_t and every
 * mbstate_t held inside both locale objects.
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <cstdint>

import pbsd.lib.libc.locale.b0039;

namespace P = pbsd::lib_libc_locale::b0039;

using P::pbsd_locale;
using P::pbsd_locale_t;
using P::pbsd_mbstate_t;

extern "C" {
size_t	ref_mbsrtowcs_l(wchar_t *, const char **, size_t, pbsd_mbstate_t *,
	    pbsd_locale_t);
size_t	ref_mbsrtowcs(wchar_t *, const char **, size_t, pbsd_mbstate_t *);
size_t	ref_wcsrtombs_l(char *, const wchar_t **, size_t, pbsd_mbstate_t *,
	    pbsd_locale_t);
size_t	ref_wcsrtombs(char *, const wchar_t **, size_t, pbsd_mbstate_t *);
int	ref_mbtowc_l(wchar_t *, const char *, size_t, pbsd_locale_t);
int	ref_mbtowc(wchar_t *, const char *, size_t);
int	ref_wcwidth(wchar_t);
int	ref_wcwidth_l(wchar_t, pbsd_locale_t);
}

/* ------------------------------------------------------------------ */
/* bookkeeping								*/
/* ------------------------------------------------------------------ */

enum {
	F_MBSRTOWCS_L, F_MBSRTOWCS,
	F_WCSRTOMBS_L, F_WCSRTOMBS,
	F_MBTOWC_L, F_MBTOWC,
	F_WCWIDTH, F_WCWIDTH_L,
	F_COUNT
};

static const char *const fname[F_COUNT] = {
	"mbsrtowcs_l", "mbsrtowcs",
	"wcsrtombs_l", "wcsrtombs",
	"mbtowc_l", "mbtowc",
	"wcwidth", "wcwidth_l"
};

static long long ncase[F_COUNT];
static long long nfail[F_COUNT];
static int nprinted[F_COUNT];

static void
report(int f, const char *why)
{

	nfail[f]++;
	if (nprinted[f]++ < 6)
		std::printf("  FAIL %-12s : %s\n", fname[f], why);
}

/* ------------------------------------------------------------------ */
/* deterministic PRNG							*/
/* ------------------------------------------------------------------ */

static uint64_t rstate = 0x9e3779b97f4a7c15ULL;

static inline uint64_t
nxt()
{

	rstate ^= rstate << 13;
	rstate ^= rstate >> 7;
	rstate ^= rstate << 17;
	return (rstate);
}

static inline uint32_t
u32(uint32_t m)
{

	return ((uint32_t)(nxt() % m));
}

/* ------------------------------------------------------------------ */
/* state / locale helpers						*/
/* ------------------------------------------------------------------ */

static bool
st_eq(const pbsd_mbstate_t &a, const pbsd_mbstate_t &b)
{

	return (a.want == b.want && a.have == b.have && a.wch == b.wch &&
	    a.lbound == b.lbound);
}

static bool
loc_eq(const pbsd_locale &a, const pbsd_locale &b)
{

	return (st_eq(a.ctype.mbsrtowcs, b.ctype.mbsrtowcs) &&
	    st_eq(a.ctype.wcsrtombs, b.ctype.wcsrtombs) &&
	    st_eq(a.ctype.mbtowc, b.ctype.mbtowc) &&
	    a.ctype.wcwidth_mode == b.ctype.wcwidth_mode &&
	    a.ctype.__mbsnrtowcs == b.ctype.__mbsnrtowcs &&
	    a.ctype.__wcsnrtombs == b.ctype.__wcsnrtombs &&
	    a.ctype.__mbrtowc == b.ctype.__mbrtowc);
}

/* Only states the decoder itself can produce: a partial sequence, or clean. */
static pbsd_mbstate_t
rand_state()
{
	pbsd_mbstate_t st;
	unsigned want, have, bits;

	st.want = 0;
	st.have = 0;
	st.wch = 0;
	st.lbound = 0;
	if (u32(5) < 2)
		return (st);
	want = 2 + u32(3);
	have = 1 + u32(want - 1);
	bits = (want == 2 ? 5u : (want == 3 ? 4u : 3u)) + 6u * (have - 1);
	st.want = want;
	st.have = have;
	st.wch = (unsigned)(nxt() & ((1u << bits) - 1u));
	st.lbound = (want == 2 ? 0x80u : (want == 3 ? 0x800u : 0x10000u));
	return (st);
}

struct Env {
	pbsd_locale	act_p, act_r;	/* the "current" locale */
	pbsd_locale	exp_p, exp_r;	/* the explicitly passed locale */
};

static void
env_build(Env &e)
{
	pbsd_mbstate_t sa[3], se[3];
	int ma, me;
	int i;

	for (i = 0; i < 3; i++) {
		sa[i] = rand_state();
		se[i] = rand_state();
	}
	ma = (int)u32(2);
	me = (int)u32(2);

	P::pbsd_locale_init(&e.act_p, ma);
	P::pbsd_locale_init(&e.act_r, ma);
	P::pbsd_locale_init(&e.exp_p, me);
	P::pbsd_locale_init(&e.exp_r, me);

	e.act_p.ctype.mbsrtowcs = e.act_r.ctype.mbsrtowcs = sa[0];
	e.act_p.ctype.wcsrtombs = e.act_r.ctype.wcsrtombs = sa[1];
	e.act_p.ctype.mbtowc = e.act_r.ctype.mbtowc = sa[2];
	e.exp_p.ctype.mbsrtowcs = e.exp_r.ctype.mbsrtowcs = se[0];
	e.exp_p.ctype.wcsrtombs = e.exp_r.ctype.wcsrtombs = se[1];
	e.exp_p.ctype.mbtowc = e.exp_r.ctype.mbtowc = se[2];
}

static inline long
off_of(const void *p, const void *base)
{

	if (p == NULL)
		return (-1);
	return ((long)((intptr_t)(uintptr_t)p - (intptr_t)(uintptr_t)base));
}

/* ------------------------------------------------------------------ */
/* input generators							*/
/* ------------------------------------------------------------------ */

static wchar_t
rand_cp_valid()
{
	unsigned v;

	for (;;) {
		switch (u32(4)) {
		case 0:  v = u32(0x80); break;
		case 1:  v = 0x80 + u32(0x780); break;
		case 2:  v = 0x800 + u32(0xf800); break;
		default: v = 0x10000 + u32(0x100000); break;
		}
		if (v >= 0xd800 && v <= 0xdfff)
			continue;
		return ((wchar_t)v);
	}
}

static wchar_t
rand_wc_any()
{

	switch (u32(12)) {
	case 0:  return ((wchar_t)0);
	case 1:  return ((wchar_t)(0xd800 + u32(0x800)));
	case 2:  return ((wchar_t)(0x110000u + u32(4096)));
	case 3:  return ((wchar_t)(-(int)(1 + u32(4096))));
	case 4:  return ((wchar_t)0x10ffff);
	case 5:  return ((wchar_t)0x110000);
	default: return (rand_cp_valid());
	}
}

static unsigned char
rand_byte()
{

	switch (u32(16)) {
	case 0: case 1:
		return (0x00);
	case 2: case 3: case 4:
		return ((unsigned char)(0x20 + u32(0x5f)));
	case 5: case 6: case 7:
		return ((unsigned char)(0x80 + u32(0x40)));
	case 8: case 9:
		return ((unsigned char)(0xc0 + u32(0x20)));
	case 10: case 11:
		return ((unsigned char)(0xe0 + u32(0x10)));
	case 12:
		return ((unsigned char)(0xf0 + u32(0x08)));
	case 13:
		return ((unsigned char)(0xf8 + u32(0x08)));
	default:
		return ((unsigned char)u32(256));
	}
}

/* Builds a NUL-terminated byte string; returns its length. */
static size_t
gen_bytes(char *buf, size_t maxlen)
{
	size_t n, target;
	char tmp[8];
	size_t k, j;

	n = 0;
	target = u32((uint32_t)maxlen + 1);
	while (n < target) {
		if (u32(2) == 0) {
			k = P::pbsd_wcrtomb(tmp, rand_cp_valid(), NULL);
			if (k == (size_t)-1)
				k = 0;
			for (j = 0; j < k && n < target; j++)
				buf[n++] = tmp[j];
		} else {
			buf[n++] = (char)rand_byte();
		}
	}
	buf[n] = '\0';
	return (n);
}

/* Builds a L'\0'-terminated wide string; returns its length. */
static size_t
gen_wides(wchar_t *buf, size_t maxlen)
{
	size_t n, target;

	target = u32((uint32_t)maxlen + 1);
	for (n = 0; n < target; n++)
		buf[n] = rand_wc_any();
	buf[n] = L'\0';
	return (n);
}

/* ------------------------------------------------------------------ */
/* mbsrtowcs / mbsrtowcs_l						*/
/* ------------------------------------------------------------------ */

#define	MBS_SRC		40
#define	MBS_DCAP	48
#define	MBS_MAXLEN	40

static void
do_mbsrtowcs(const char *in, size_t inlen, size_t len, bool dst_null,
    bool ps_null, int lmode, pbsd_mbstate_t seed)
{
	Env e;
	char sp[MBS_SRC + 8], sr[MBS_SRC + 8];
	wchar_t dp[MBS_DCAP], dr[MBS_DCAP];
	pbsd_mbstate_t psp, psr;
	const char *cp, *cr;
	size_t rp, rr;
	int ep, er;
	int f;

	f = (lmode == 2) ? F_MBSRTOWCS : F_MBSRTOWCS_L;
	ncase[f]++;

	env_build(e);

	std::memset(sp, 0, sizeof(sp));
	std::memset(sr, 0, sizeof(sr));
	std::memcpy(sp, in, inlen);
	std::memcpy(sr, in, inlen);

	std::memset(dp, 0x7f, sizeof(dp));
	std::memset(dr, 0x7f, sizeof(dr));

	psp = seed;
	psr = seed;
	cp = sp;
	cr = sr;

	errno = 0;
	P::pbsd_set_active_locale(&e.act_p);
	if (lmode == 2)
		rp = P::mbsrtowcs(dst_null ? NULL : dp, &cp, len,
		    ps_null ? NULL : &psp);
	else
		rp = P::mbsrtowcs_l(dst_null ? NULL : dp, &cp, len,
		    ps_null ? NULL : &psp, lmode == 0 ? &e.exp_p : NULL);
	ep = errno;

	errno = 0;
	P::pbsd_set_active_locale(&e.act_r);
	if (lmode == 2)
		rr = ref_mbsrtowcs(dst_null ? NULL : dr, &cr, len,
		    ps_null ? NULL : &psr);
	else
		rr = ref_mbsrtowcs_l(dst_null ? NULL : dr, &cr, len,
		    ps_null ? NULL : &psr, lmode == 0 ? &e.exp_r : NULL);
	er = errno;

	if (rp != rr)
		report(f, "return value");
	else if (ep != er)
		report(f, "errno");
	else if (off_of(cp, sp) != off_of(cr, sr))
		report(f, "*src offset");
	else if (std::memcmp(dp, dr, sizeof(dp)) != 0)
		report(f, "destination buffer (incl. guard bytes)");
	else if (std::memcmp(sp, sr, sizeof(sp)) != 0)
		report(f, "source buffer clobbered");
	else if (!st_eq(psp, psr))
		report(f, "caller mbstate_t");
	else if (!loc_eq(e.act_p, e.act_r))
		report(f, "current locale state");
	else if (!loc_eq(e.exp_p, e.exp_r))
		report(f, "explicit locale state");
}

/* ------------------------------------------------------------------ */
/* wcsrtombs / wcsrtombs_l						*/
/* ------------------------------------------------------------------ */

#define	WCS_SRC		24
#define	WCS_DCAP	72
#define	WCS_MAXLEN	64

static void
do_wcsrtombs(const wchar_t *in, size_t inlen, size_t len, bool dst_null,
    bool ps_null, int lmode, pbsd_mbstate_t seed)
{
	Env e;
	wchar_t sp[WCS_SRC + 8], sr[WCS_SRC + 8];
	char dp[WCS_DCAP], dr[WCS_DCAP];
	pbsd_mbstate_t psp, psr;
	const wchar_t *cp, *cr;
	size_t rp, rr;
	int ep, er;
	int f;

	f = (lmode == 2) ? F_WCSRTOMBS : F_WCSRTOMBS_L;
	ncase[f]++;

	env_build(e);

	std::memset(sp, 0, sizeof(sp));
	std::memset(sr, 0, sizeof(sr));
	std::memcpy(sp, in, inlen * sizeof(wchar_t));
	std::memcpy(sr, in, inlen * sizeof(wchar_t));

	std::memset(dp, 0x7f, sizeof(dp));
	std::memset(dr, 0x7f, sizeof(dr));

	psp = seed;
	psr = seed;
	cp = sp;
	cr = sr;

	errno = 0;
	P::pbsd_set_active_locale(&e.act_p);
	if (lmode == 2)
		rp = P::wcsrtombs(dst_null ? NULL : dp, &cp, len,
		    ps_null ? NULL : &psp);
	else
		rp = P::wcsrtombs_l(dst_null ? NULL : dp, &cp, len,
		    ps_null ? NULL : &psp, lmode == 0 ? &e.exp_p : NULL);
	ep = errno;

	errno = 0;
	P::pbsd_set_active_locale(&e.act_r);
	if (lmode == 2)
		rr = ref_wcsrtombs(dst_null ? NULL : dr, &cr, len,
		    ps_null ? NULL : &psr);
	else
		rr = ref_wcsrtombs_l(dst_null ? NULL : dr, &cr, len,
		    ps_null ? NULL : &psr, lmode == 0 ? &e.exp_r : NULL);
	er = errno;

	if (rp != rr)
		report(f, "return value");
	else if (ep != er)
		report(f, "errno");
	else if (off_of(cp, sp) != off_of(cr, sr))
		report(f, "*src offset");
	else if (std::memcmp(dp, dr, sizeof(dp)) != 0)
		report(f, "destination buffer (incl. guard bytes)");
	else if (std::memcmp(sp, sr, sizeof(sp)) != 0)
		report(f, "source buffer clobbered");
	else if (!st_eq(psp, psr))
		report(f, "caller mbstate_t");
	else if (!loc_eq(e.act_p, e.act_r))
		report(f, "current locale state");
	else if (!loc_eq(e.exp_p, e.exp_r))
		report(f, "explicit locale state");
}

/* ------------------------------------------------------------------ */
/* mbtowc / mbtowc_l							*/
/* ------------------------------------------------------------------ */

#define	MBT_SRC		24

static void
do_mbtowc(const char *in, size_t inlen, size_t n, bool s_null, bool pwc_null,
    int lmode)
{
	Env e;
	char sp[MBT_SRC + 8], sr[MBT_SRC + 8];
	wchar_t wp[4], wr[4];
	int rp, rr, ep, er;
	int f;

	f = (lmode == 2) ? F_MBTOWC : F_MBTOWC_L;
	ncase[f]++;

	env_build(e);

	std::memset(sp, 0, sizeof(sp));
	std::memset(sr, 0, sizeof(sr));
	std::memcpy(sp, in, inlen);
	std::memcpy(sr, in, inlen);

	std::memset(wp, 0x7f, sizeof(wp));
	std::memset(wr, 0x7f, sizeof(wr));

	errno = 0;
	P::pbsd_set_active_locale(&e.act_p);
	if (lmode == 2)
		rp = P::mbtowc(pwc_null ? NULL : wp, s_null ? NULL : sp, n);
	else
		rp = P::mbtowc_l(pwc_null ? NULL : wp, s_null ? NULL : sp, n,
		    lmode == 0 ? &e.exp_p : NULL);
	ep = errno;

	errno = 0;
	P::pbsd_set_active_locale(&e.act_r);
	if (lmode == 2)
		rr = ref_mbtowc(pwc_null ? NULL : wr, s_null ? NULL : sr, n);
	else
		rr = ref_mbtowc_l(pwc_null ? NULL : wr, s_null ? NULL : sr, n,
		    lmode == 0 ? &e.exp_r : NULL);
	er = errno;

	if (rp != rr)
		report(f, "return value");
	else if (ep != er)
		report(f, "errno");
	else if (std::memcmp(wp, wr, sizeof(wp)) != 0)
		report(f, "pwc buffer (incl. guard bytes)");
	else if (std::memcmp(sp, sr, sizeof(sp)) != 0)
		report(f, "source buffer clobbered");
	else if (!loc_eq(e.act_p, e.act_r))
		report(f, "current locale state");
	else if (!loc_eq(e.exp_p, e.exp_r))
		report(f, "explicit locale state");
}

/*
 * mbtowc_l keeps conversion state inside the locale, so also drive it as a
 * stateful iterator: feed one byte at a time and compare after every step.
 */
static void
do_mbtowc_stream(const char *in, size_t inlen, int lmode, bool reset_midway)
{
	Env e;
	char sp[MBT_SRC + 8], sr[MBT_SRC + 8];
	wchar_t wp[4], wr[4];
	size_t i;
	int rp, rr, ep, er;
	int f;

	f = (lmode == 2) ? F_MBTOWC : F_MBTOWC_L;

	env_build(e);
	std::memset(sp, 0, sizeof(sp));
	std::memset(sr, 0, sizeof(sr));
	std::memcpy(sp, in, inlen);
	std::memcpy(sr, in, inlen);

	for (i = 0; i <= inlen; i++) {
		ncase[f]++;
		std::memset(wp, 0x7f, sizeof(wp));
		std::memset(wr, 0x7f, sizeof(wr));

		bool snull = (reset_midway && i == inlen / 2);

		errno = 0;
		P::pbsd_set_active_locale(&e.act_p);
		if (lmode == 2)
			rp = P::mbtowc(wp, snull ? NULL : sp + i, 1);
		else
			rp = P::mbtowc_l(wp, snull ? NULL : sp + i, 1,
			    lmode == 0 ? &e.exp_p : NULL);
		ep = errno;

		errno = 0;
		P::pbsd_set_active_locale(&e.act_r);
		if (lmode == 2)
			rr = ref_mbtowc(wr, snull ? NULL : sr + i, 1);
		else
			rr = ref_mbtowc_l(wr, snull ? NULL : sr + i, 1,
			    lmode == 0 ? &e.exp_r : NULL);
		er = errno;

		if (rp != rr)
			report(f, "stream: return value");
		else if (ep != er)
			report(f, "stream: errno");
		else if (std::memcmp(wp, wr, sizeof(wp)) != 0)
			report(f, "stream: pwc buffer (incl. guard bytes)");
		else if (!loc_eq(e.act_p, e.act_r))
			report(f, "stream: current locale state");
		else if (!loc_eq(e.exp_p, e.exp_r))
			report(f, "stream: explicit locale state");
	}
}

/* ------------------------------------------------------------------ */
/* wcwidth / wcwidth_l							*/
/* ------------------------------------------------------------------ */

static void
do_wcwidth(wchar_t wc, int lmode)
{
	Env e;
	int rp, rr;
	int f;

	f = (lmode == 2) ? F_WCWIDTH : F_WCWIDTH_L;
	ncase[f]++;

	env_build(e);

	P::pbsd_set_active_locale(&e.act_p);
	rp = (lmode == 2) ? P::wcwidth(wc)
			  : P::wcwidth_l(wc, lmode == 0 ? &e.exp_p : NULL);

	P::pbsd_set_active_locale(&e.act_r);
	rr = (lmode == 2) ? ref_wcwidth(wc)
			  : ref_wcwidth_l(wc, lmode == 0 ? &e.exp_r : NULL);

	if (rp != rr)
		report(f, "return value");
	else if (!loc_eq(e.act_p, e.act_r))
		report(f, "current locale state");
	else if (!loc_eq(e.exp_p, e.exp_r))
		report(f, "explicit locale state");
}

/* ------------------------------------------------------------------ */
/* hand-written edge inputs						*/
/* ------------------------------------------------------------------ */

struct Bytes { const char *d; size_t n; };
#define	B(s)	{ s, sizeof(s) - 1 }

static const Bytes edge_bytes[] = {
	B(""),
	B("A"),
	B("\x7f"),
	B("\x01"),
	B("\x00"),
	B("\x00\x00"),
	B("A\x00" "B"),
	B("AB"),
	B("ABCDEFGH"),
	B("\x80"),
	B("\xbf"),
	B("\x80\x80\x80"),
	B("\xc0\x80"),			/* overlong */
	B("\xc1\xbf"),			/* overlong */
	B("\xc2\x80"),			/* U+0080, lowest 2-byte */
	B("\xc2"),			/* truncated 2-byte */
	B("\xdf\xbf"),			/* U+07FF, highest 2-byte */
	B("\xe0\x80\x80"),		/* overlong */
	B("\xe0\xa0\x80"),		/* U+0800, lowest 3-byte */
	B("\xe0\xa0"),			/* truncated 3-byte */
	B("\xe0"),
	B("\xed\x9f\xbf"),		/* U+D7FF */
	B("\xed\xa0\x80"),		/* surrogate U+D800 */
	B("\xed\xbf\xbf"),		/* surrogate U+DFFF */
	B("\xee\x80\x80"),		/* U+E000 */
	B("\xef\xbf\xbf"),		/* U+FFFF */
	B("\xf0\x80\x80\x80"),		/* overlong */
	B("\xf0\x90\x80\x80"),		/* U+10000, lowest 4-byte */
	B("\xf0\x90\x80"),		/* truncated 4-byte */
	B("\xf0\x90"),
	B("\xf0"),
	B("\xf4\x8f\xbf\xbf"),		/* U+10FFFF */
	B("\xf4\x90\x80\x80"),		/* U+110000, out of range */
	B("\xf5\x80\x80\x80"),
	B("\xf7\xbf\xbf\xbf"),
	B("\xf8\x88\x80\x80\x80"),	/* 5-byte, invalid lead */
	B("\xfe"),
	B("\xff"),
	B("\xff\xfe"),
	B("A\xc2\xa9" "B"),
	B("\xe4\xb8\xad\xe6\x96\x87"),	/* two CJK ideographs */
	B("\xe4\xb8\xad\x00\xe6\x96\x87"),
	B("ab\xc2"),			/* trailing truncation */
	B("ab\x80" "cd"),
	B("\xc2\xa9\xc2"),
	B("\x41\xf0\x9f\x98\x80\x42"),	/* ASCII, emoji, ASCII */
	B("\xcc\x80"),			/* combining grave, width 0 */
	B("\xef\xb8\x8f"),		/* U+FE0F, width 0 */
	B("\xef\xbc\xa1"),		/* U+FF21, width 2 */
};
static const size_t n_edge_bytes = sizeof(edge_bytes) / sizeof(edge_bytes[0]);

static const wchar_t we0[] = { 0 };
static const wchar_t we1[] = { L'A', 0 };
static const wchar_t we2[] = { L'A', L'B', L'C', 0 };
static const wchar_t we3[] = { 0x7f, 0x80, 0 };
static const wchar_t we4[] = { 0x7ff, 0x800, 0 };
static const wchar_t we5[] = { 0xffff, 0x10000, 0 };
static const wchar_t we6[] = { 0x10ffff, 0 };
static const wchar_t we7[] = { 0x110000, 0 };
static const wchar_t we8[] = { L'A', 0xd800, L'B', 0 };
static const wchar_t we9[] = { L'A', 0xdfff, 0 };
static const wchar_t we10[] = { (wchar_t)-1, 0 };
static const wchar_t we11[] = { L'A', (wchar_t)-2, 0 };
static const wchar_t we12[] = { 0x4e2d, 0x6587, 0 };
static const wchar_t we13[] = { 0x1f600, 0x1f601, 0x1f602, 0 };
static const wchar_t we14[] = { L'a', 0x80, 0x800, 0x10000, 0 };
static const wchar_t we15[] = { 0x10000, 0x10000, 0x10000, 0x10000, 0 };
static const wchar_t we16[] = { L'x', L'y', L'z', L'w', L'v', L'u', L't', 0 };
static const wchar_t we17[] = { 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0 };
static const wchar_t we18[] = { 0x800, 0x800, 0x800, 0 };
static const wchar_t we19[] = { L'A', 0x110000, 0xd800, 0 };

struct Wides { const wchar_t *d; size_t n; };
#define	W(a)	{ a, sizeof(a) / sizeof(wchar_t) - 1 }

static const Wides edge_wides[] = {
	W(we0), W(we1), W(we2), W(we3), W(we4), W(we5), W(we6), W(we7),
	W(we8), W(we9), W(we10), W(we11), W(we12), W(we13), W(we14),
	W(we15), W(we16), W(we17), W(we18), W(we19)
};
static const size_t n_edge_wides = sizeof(edge_wides) / sizeof(edge_wides[0]);

static const size_t edge_lens[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 16, 40 };
static const size_t n_edge_lens = sizeof(edge_lens) / sizeof(edge_lens[0]);

static pbsd_mbstate_t
fixed_state(int k)
{
	pbsd_mbstate_t st;

	st.want = 0;
	st.have = 0;
	st.wch = 0;
	st.lbound = 0;
	switch (k) {
	case 1:	st.want = 2; st.have = 1; st.wch = 0x02; st.lbound = 0x80; break;
	case 2:	st.want = 3; st.have = 1; st.wch = 0x04; st.lbound = 0x800; break;
	case 3:	st.want = 3; st.have = 2; st.wch = 0x120; st.lbound = 0x800; break;
	case 4:	st.want = 4; st.have = 1; st.wch = 0x00; st.lbound = 0x10000; break;
	case 5:	st.want = 4; st.have = 3; st.wch = 0x0410; st.lbound = 0x10000; break;
	default: break;
	}
	return (st);
}

/* ------------------------------------------------------------------ */

static void
edge_pass()
{
	size_t i, j;
	int lm, dn, pn, sk;

	for (i = 0; i < n_edge_bytes; i++)
		for (j = 0; j < n_edge_lens; j++)
			for (lm = 0; lm < 3; lm++)
				for (dn = 0; dn < 2; dn++)
					for (pn = 0; pn < 2; pn++)
						for (sk = 0; sk < 6; sk++)
							do_mbsrtowcs(
							    edge_bytes[i].d,
							    edge_bytes[i].n,
							    edge_lens[j],
							    dn != 0, pn != 0,
							    lm,
							    fixed_state(sk));

	for (i = 0; i < n_edge_wides; i++)
		for (j = 0; j < n_edge_lens; j++)
			for (lm = 0; lm < 3; lm++)
				for (dn = 0; dn < 2; dn++)
					for (pn = 0; pn < 2; pn++)
						for (sk = 0; sk < 6; sk++)
							do_wcsrtombs(
							    edge_wides[i].d,
							    edge_wides[i].n,
							    edge_lens[j],
							    dn != 0, pn != 0,
							    lm,
							    fixed_state(sk));

	/* wcsrtombs with lengths right around MB_CUR_MAX and around the
	 * exact byte count of the conversion. */
	for (i = 0; i < n_edge_wides; i++)
		for (j = 0; j <= 24; j++)
			for (lm = 0; lm < 3; lm++)
				do_wcsrtombs(edge_wides[i].d, edge_wides[i].n,
				    j, false, false, lm, fixed_state(0));

	for (i = 0; i < n_edge_bytes; i++)
		for (j = 0; j <= 8; j++)
			for (lm = 0; lm < 3; lm++)
				for (dn = 0; dn < 2; dn++)
					do_mbtowc(edge_bytes[i].d,
					    edge_bytes[i].n, j, false, dn != 0,
					    lm);

	/* s == NULL resets the locale's mbtowc state. */
	for (j = 0; j <= 4; j++)
		for (lm = 0; lm < 3; lm++)
			for (dn = 0; dn < 2; dn++)
				do_mbtowc("", 0, j, true, dn != 0, lm);

	for (i = 0; i < n_edge_bytes; i++)
		for (lm = 0; lm < 3; lm++) {
			do_mbtowc_stream(edge_bytes[i].d, edge_bytes[i].n, lm,
			    false);
			do_mbtowc_stream(edge_bytes[i].d, edge_bytes[i].n, lm,
			    true);
		}

	{
		static const long widths[] = {
			-3, -2, -1, 0, 1, 0x1f, 0x20, 0x21, 0x7e, 0x7f, 0x80,
			0x9f, 0xa0, 0xa1, 0x2ff, 0x300, 0x36f, 0x370,
			0x10ff, 0x1100, 0x115f, 0x1160, 0x200a, 0x200b,
			0x200f, 0x2010, 0x2e7f, 0x2e80, 0xa4cf, 0xa4d0,
			0xabff, 0xac00, 0xd7a3, 0xd7a4, 0xf8ff, 0xf900,
			0xfaff, 0xfb00, 0xfdff, 0xfe00, 0xfe0f, 0xfe10,
			0xfeff, 0xff00, 0xff60, 0xff61, 0xffdf, 0xffe0,
			0xffe6, 0xffe7, 0x1ffff, 0x20000, 0x3fffd, 0x3fffe,
			0x10fffe, 0x10ffff, 0x110000, 0x110001, 0x7ffffff
		};
		size_t k;

		for (k = 0; k < sizeof(widths) / sizeof(widths[0]); k++)
			for (lm = 0; lm < 3; lm++)
				do_wcwidth((wchar_t)widths[k], lm);
		for (long v = 0; v < 0x1200; v++)
			for (lm = 0; lm < 3; lm++)
				do_wcwidth((wchar_t)v, lm);
	}
}

/* ------------------------------------------------------------------ */

#define	SWEEP	200000

static void
random_pass()
{
	char bbuf[MBS_SRC + 8];
	wchar_t wbuf[WCS_SRC + 8];
	size_t n, len;
	long i;
	int lm;

	for (i = 0; i < SWEEP; i++) {
		n = gen_bytes(bbuf, MBS_SRC);
		len = u32(MBS_MAXLEN + 1);
		lm = (int)u32(3);
		do_mbsrtowcs(bbuf, n, len, u32(4) == 0, u32(2) == 0, lm,
		    rand_state());
	}

	for (i = 0; i < SWEEP; i++) {
		n = gen_wides(wbuf, WCS_SRC);
		len = (u32(2) == 0) ? u32(10) : u32(WCS_MAXLEN + 1);
		lm = (int)u32(3);
		do_wcsrtombs(wbuf, n, len, u32(4) == 0, u32(2) == 0, lm,
		    rand_state());
	}

	for (i = 0; i < SWEEP; i++) {
		n = gen_bytes(bbuf, MBT_SRC);
		lm = (int)u32(3);
		do_mbtowc(bbuf, n, u32(10), u32(16) == 0, u32(4) == 0, lm);
	}

	for (i = 0; i < SWEEP / 4; i++) {
		n = gen_bytes(bbuf, 10);
		lm = (int)u32(3);
		do_mbtowc_stream(bbuf, n, lm, u32(3) == 0);
	}

	for (i = 0; i < SWEEP; i++) {
		lm = (int)u32(3);
		do_wcwidth(rand_wc_any(), lm);
	}
}

int
main()
{
	int f;
	long long tc, tf;

	edge_pass();
	random_pass();

	std::printf("\n%-14s %12s %12s  %s\n", "function", "cases", "failures",
	    "result");
	std::printf("-------------------------------------------------"
	    "----------\n");
	tc = 0;
	tf = 0;
	for (f = 0; f < F_COUNT; f++) {
		std::printf("%-14s %12lld %12lld  %s\n", fname[f], ncase[f],
		    nfail[f], nfail[f] == 0 ? "PASS" : "FAIL");
		tc += ncase[f];
		tf += nfail[f];
	}
	std::printf("-------------------------------------------------"
	    "----------\n");
	std::printf("%-14s %12lld %12lld  %s\n", "TOTAL", tc, tf,
	    tf == 0 ? "PASS" : "FAIL");

	return (tf == 0 ? 0 : 1);
}
