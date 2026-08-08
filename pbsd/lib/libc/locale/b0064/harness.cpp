/*
 * Differential harness for batch b0064 (mbsinit, mbsinit_l).
 *
 * Every case runs the C++23 port and the C oracle with parallel locale
 * objects configured identically, then compares return values.
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.locale.b0064;

namespace P = pbsd::lib_libc_locale::b0064;

extern "C" {
int	ref_mbsinit(const mbstate_t *);
int	ref_mbsinit_l(const mbstate_t *, locale_t);
void	ref_locale_init(void);
void	ref_set_thread_locale(int, locale_t);
locale_t ref_c_locale_ptr(void);
locale_t ref_global_locale_ptr(void);
}

typedef union {
	char		__mbstate8[128];
	long long	_mbstateL;
} pbsd_mbstate_t;
typedef pbsd_mbstate_t mbstate_t;

struct xlocale_ctype {
	int		(*__mbsinit)(const mbstate_t *);
};

struct xlocale {
	void		*components[6];
};

enum { XLC_CTYPE = 1 };

#define XLOCALE_CTYPE(x)	((struct xlocale_ctype *)(x)->components[XLC_CTYPE])

typedef struct {
	wchar_t	ch;
	int	want;
	wchar_t	lbound;
} utf8_state_t;

/* ------------------------------------------------------------------ */
/* bookkeeping								*/
/* ------------------------------------------------------------------ */

enum {
	F_MBSINIT,
	F_MBSINIT_L,
	F_COUNT
};

static const char *const fname[F_COUNT] = {
	"mbsinit",
	"mbsinit_l"
};

static long long ncase[F_COUNT];
static long long nfail[F_COUNT];
static int nprinted[F_COUNT];

static void
report(int f, const char *why)
{

	nfail[f]++;
	if (nprinted[f]++ < 8)
		std::printf("  FAIL %-10s : %s\n", fname[f], why);
}

/* ------------------------------------------------------------------ */
/* deterministic PRNG							*/
/* ------------------------------------------------------------------ */

static uint64_t rstate = 0xB00064ULL;

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
/* mock __mbsinit backends (shared logic for port and ref locales)	*/
/* ------------------------------------------------------------------ */

static int
mock_utf8_mbsinit(const mbstate_t *ps)
{

	return (ps == NULL || ((const utf8_state_t *)ps)->want == 0);
}

static int
mock_always_one(const mbstate_t *ps __attribute__((unused)))
{

	return (1);
}

static int
mock_always_zero(const mbstate_t *ps __attribute__((unused)))
{

	return (0);
}

static int
mock_parity(const mbstate_t *ps)
{

	if (ps == NULL)
		return (0);
	return (((uintptr_t)ps >> 3) & 1);
}

static int
mock_want_sign(const mbstate_t *ps)
{

	if (ps == NULL)
		return (1);
	return (((const utf8_state_t *)ps)->want > 0);
}

static int (*const backends[])(const mbstate_t *) = {
	mock_utf8_mbsinit,
	mock_always_one,
	mock_always_zero,
	mock_parity,
	mock_want_sign
};

static const int nbackends =
    (int)(sizeof(backends) / sizeof(backends[0]));

/* ------------------------------------------------------------------ */
/* locale helpers							*/
/* ------------------------------------------------------------------ */

struct Env {
	struct xlocale_ctype	act_p, act_r;
	struct xlocale		act_loc_p, act_loc_r;
	struct xlocale_ctype	custom_p[4], custom_r[4];
	struct xlocale		custom_loc_p[4], custom_loc_r[4];
};

static void
env_init(Env &e)
{
	int i;

	P::pbsd_locale_init();
	ref_locale_init();

	for (i = 0; i < nbackends; i++) {
		e.custom_p[i].__mbsinit = backends[i];
		e.custom_r[i].__mbsinit = backends[i];
		e.custom_loc_p[i].components[XLC_CTYPE] = &e.custom_p[i];
		e.custom_loc_r[i].components[XLC_CTYPE] = &e.custom_r[i];
	}
}

static void
set_active_backends(Env &e, int c_back, int g_back)
{

	XLOCALE_CTYPE(P::pbsd_c_locale())->__mbsinit = backends[c_back];
	XLOCALE_CTYPE(P::pbsd_global_locale())->__mbsinit = backends[g_back];
	XLOCALE_CTYPE(ref_c_locale_ptr())->__mbsinit = backends[c_back];
	XLOCALE_CTYPE(ref_global_locale_ptr())->__mbsinit = backends[g_back];
	e.act_p.__mbsinit = backends[g_back];
	e.act_r.__mbsinit = backends[g_back];
	e.act_loc_p.components[XLC_CTYPE] = &e.act_p;
	e.act_loc_r.components[XLC_CTYPE] = &e.act_r;
}

static void
set_thread(int on, struct xlocale *ploc, struct xlocale *rloc)
{

	P::pbsd_set_thread_locale(on, (locale_t)ploc);
	ref_set_thread_locale(on, (locale_t)rloc);
}

static void
fill_random_state(mbstate_t *st)
{
	size_t i;

	if (st == NULL)
		return;
	for (i = 0; i < sizeof(*st); i++)
		st->__mbstate8[i] = (char)(nxt() & 0xff);
}

static void
make_utf8_state(mbstate_t *st, int want, wchar_t ch, wchar_t lbound)
{
	utf8_state_t *us;

	if (st == NULL)
		return;
	std::memset(st, 0, sizeof(*st));
	us = (utf8_state_t *)st;
	us->ch = ch;
	us->want = want;
	us->lbound = lbound;
}

static locale_t
pick_locale_l(const Env &e, int mode)
{

	switch (mode) {
	case 0:
		return (NULL);
	case 1:
		return ((locale_t)(intptr_t)-1);
	case 2:
		return (P::pbsd_c_locale());
	case 3:
		return (P::pbsd_global_locale());
	default:
		return ((locale_t)&e.custom_loc_p[mode - 4]);
	}
}

static locale_t
pick_locale_r(const Env &e, int mode)
{

	switch (mode) {
	case 0:
		return (NULL);
	case 1:
		return ((locale_t)(intptr_t)-1);
	case 2:
		return (ref_c_locale_ptr());
	case 3:
		return (ref_global_locale_ptr());
	default:
		return ((locale_t)&e.custom_loc_r[mode - 4]);
	}
}

/* ------------------------------------------------------------------ */
/* test drivers								*/
/* ------------------------------------------------------------------ */

static void
do_mbsinit_l(const mbstate_t *ps_p, const mbstate_t *ps_r, int c_back,
    int g_back, int loc_mode)
{
	Env e;
	locale_t lp, lr;
	int rp, rr;
	int f = F_MBSINIT_L;

	ncase[f]++;

	env_init(e);
	set_active_backends(e, c_back, g_back);
	lp = pick_locale_l(e, loc_mode);
	lr = pick_locale_r(e, loc_mode);

	rp = P::mbsinit_l(ps_p, lp);
	rr = ref_mbsinit_l(ps_r, lr);

	if (rp != rr)
		report(f, "return value");
}

static void
do_mbsinit(const mbstate_t *ps_p, const mbstate_t *ps_r, int c_back,
    int g_back, int thread_mode, int thread_back)
{
	Env e;
	int rp, rr;
	int f = F_MBSINIT;

	ncase[f]++;

	env_init(e);
	set_active_backends(e, c_back, g_back);

	switch (thread_mode) {
	case 0:
		set_thread(0, NULL, NULL);
		break;
	case 1:
		set_thread(1, NULL, NULL);
		break;
	case 2:
		e.act_p.__mbsinit = backends[thread_back];
		e.act_r.__mbsinit = backends[thread_back];
		set_thread(1, &e.act_loc_p, &e.act_loc_r);
		break;
	case 3:
		set_thread(1, &e.custom_loc_p[thread_back],
		    &e.custom_loc_r[thread_back]);
		break;
	default:
		set_thread(0, NULL, NULL);
		break;
	}

	rp = P::mbsinit(ps_p);
	rr = ref_mbsinit(ps_r);

	if (rp != rr)
		report(f, "return value");
}

/* ------------------------------------------------------------------ */
/* edge cases								*/
/* ------------------------------------------------------------------ */

static void
edge_pass()
{
	Env e;
	mbstate_t zp, zr;
	mbstate_t sp, sr;
	int c, g, w, lm, tm, tb, i;

	env_init(e);

	/* ps == NULL across locale placeholders and backends. */
	for (c = 0; c < nbackends; c++)
		for (g = 0; g < nbackends; g++)
			for (lm = 0; lm < 4 + nbackends; lm++)
				do_mbsinit_l(NULL, NULL, c, g, lm);

	for (c = 0; c < nbackends; c++)
		for (g = 0; g < nbackends; g++)
			for (tm = 0; tm < 4; tm++)
				do_mbsinit(NULL, NULL, c, g, tm, 0);

	/* zeroed mbstate: utf8 backend returns 1; others may differ. */
	std::memset(&zp, 0, sizeof(zp));
	std::memset(&zr, 0, sizeof(zr));
	for (c = 0; c < nbackends; c++)
		for (g = 0; g < nbackends; g++)
			for (lm = 0; lm < 4 + nbackends; lm++)
				do_mbsinit_l(&zp, &zr, c, g, lm);

	for (c = 0; c < nbackends; c++)
		for (g = 0; g < nbackends; g++)
			for (tm = 0; tm < 4; tm++)
				do_mbsinit(&zp, &zr, c, g, tm, 0);

	/* UTF-8 want boundary: 0 vs 1 and negative want. */
	for (w = -2; w <= 4; w++) {
		make_utf8_state(&zp, w, (wchar_t)0x41, (wchar_t)0x80);
		make_utf8_state(&zr, w, (wchar_t)0x41, (wchar_t)0x80);
		for (lm = 0; lm < 4 + nbackends; lm++)
			do_mbsinit_l(&zp, &zr, 0, 1, lm);
		for (tm = 0; tm < 4; tm++)
			do_mbsinit(&zp, &zr, 0, 1, tm, 0);
	}

	/* High-bit bytes throughout the 128-byte state object. */
	for (i = 0; i < 128; i++) {
		std::memset(&sp, 0x7f, sizeof(sp));
		std::memset(&sr, 0x7f, sizeof(sr));
		sp.__mbstate8[i] = (char)0x80;
		sr.__mbstate8[i] = (char)0x80;
		for (lm = 0; lm < 4; lm++)
			do_mbsinit_l(&sp, &sr, 0, 2, lm);
	}

	/* NUL-heavy fill. */
	std::memset(&sp, 0, sizeof(sp));
	std::memset(&sr, 0, sizeof(sr));
	for (lm = 0; lm < 4 + nbackends; lm++)
		do_mbsinit_l(&sp, &sr, 3, 4, lm);

	/* Distinguish C (NULL locale) from global ((locale_t)-1). */
	make_utf8_state(&zp, 1, (wchar_t)0, (wchar_t)0);
	make_utf8_state(&zr, 1, (wchar_t)0, (wchar_t)0);
	do_mbsinit_l(&zp, &zr, 0, 2, 0);
	do_mbsinit_l(&zp, &zr, 0, 2, 1);
	do_mbsinit_l(&zp, &zr, 0, 2, 2);
	do_mbsinit_l(&zp, &zr, 0, 2, 3);

	/* Thread-locale vs global-default paths for mbsinit(). */
	for (tb = 0; tb < nbackends; tb++) {
		for (tm = 0; tm < 4; tm++)
			do_mbsinit(&zp, &zr, 1, 2, tm, tb);
	}
}

/* ------------------------------------------------------------------ */
/* random sweep								*/
/* ------------------------------------------------------------------ */

#define	SWEEP	200000

static void
random_pass()
{
	mbstate_t sp, sr;
	long i;
	int c, g, lm, tm, tb, ps_kind;

	for (i = 0; i < SWEEP; i++) {
		c = (int)u32(nbackends);
		g = (int)u32(nbackends);
		lm = (int)u32(4 + nbackends);
		ps_kind = (int)u32(6);

		if (ps_kind == 0) {
			do_mbsinit_l(NULL, NULL, c, g, lm);
			continue;
		}

		std::memset(&sp, 0, sizeof(sp));
		std::memset(&sr, 0, sizeof(sr));
		switch (ps_kind) {
		case 1:
			break;
		case 2:
			make_utf8_state(&sp, 0, (wchar_t)u32(0x110000),
			    (wchar_t)u32(0x10000));
			make_utf8_state(&sr, 0, (wchar_t)u32(0x110000),
			    (wchar_t)u32(0x10000));
			break;
		case 3:
			make_utf8_state(&sp, 1 + (int)u32(4), (wchar_t)u32(256),
			    (wchar_t)(0x80 + u32(0x10000)));
			make_utf8_state(&sr, 1 + (int)u32(4), (wchar_t)u32(256),
			    (wchar_t)(0x80 + u32(0x10000)));
			break;
		case 4:
			make_utf8_state(&sp, -(int)u32(3), (wchar_t)-1,
			    (wchar_t)-1);
			make_utf8_state(&sr, -(int)u32(3), (wchar_t)-1,
			    (wchar_t)-1);
			break;
		default:
			fill_random_state(&sp);
			fill_random_state(&sr);
			break;
		}
		do_mbsinit_l(&sp, &sr, c, g, lm);
	}

	for (i = 0; i < SWEEP; i++) {
		c = (int)u32(nbackends);
		g = (int)u32(nbackends);
		tm = (int)u32(4);
		tb = (int)u32(nbackends);
		ps_kind = (int)u32(6);

		if (ps_kind == 0) {
			do_mbsinit(NULL, NULL, c, g, tm, tb);
			continue;
		}

		std::memset(&sp, 0, sizeof(sp));
		std::memset(&sr, 0, sizeof(sr));
		if (ps_kind >= 2) {
			make_utf8_state(&sp, (int)u32(5) - 2, (wchar_t)u32(0xffff),
			    (wchar_t)u32(0x7fff));
			make_utf8_state(&sr, (int)u32(5) - 2, (wchar_t)u32(0xffff),
			    (wchar_t)u32(0x7fff));
		} else if (ps_kind == 1) {
			fill_random_state(&sp);
			fill_random_state(&sr);
		}
		do_mbsinit(&sp, &sr, c, g, tm, tb);
	}
}

/* ------------------------------------------------------------------ */

int
main()
{
	int f;
	long long tc, tf;

	edge_pass();
	random_pass();

	std::printf("\n%-12s %12s %12s  %s\n", "function", "cases", "failures",
	    "result");
	std::printf("-------------------------------------------------------\n");
	tc = 0;
	tf = 0;
	for (f = 0; f < F_COUNT; f++) {
		std::printf("%-12s %12lld %12lld  %s\n", fname[f], ncase[f],
		    nfail[f], nfail[f] == 0 ? "PASS" : "FAIL");
		tc += ncase[f];
		tf += nfail[f];
	}
	std::printf("-------------------------------------------------------\n");
	std::printf("%-12s %12lld %12lld  %s\n", "TOTAL", tc, tf,
	    tf == 0 ? "PASS" : "FAIL");

	return (tf == 0 ? 0 : 1);
}
