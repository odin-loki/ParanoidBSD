/*
 * PBSD batch b0156s3 -- differential test of the C++23 module port against the
 * ref_ oracle built from the original C.
 *
 * Every ported function is exercised with hand-written edge cases (empty
 * input, single byte, NUL-heavy input, the whole 0x80-0xff lead-byte range,
 * both sides of every length/state boundary) and with a fixed-seed randomised
 * sweep.  Every call is made twice -- once into the port, once into the oracle
 * -- against two independently allocated buffer sets that are pre-filled with
 * the guard byte 0x7f, and the comparison covers the return value, errno, the
 * ENTIRE output buffer (well past the nominal write window), the ENTIRE
 * 128-byte mbstate_t and the input buffer.  Stateful (partial sequence)
 * conversion is additionally driven to exhaustion byte-by-byte with the
 * comparison repeated after every single step.
 *
 * main() returns 0 only if every case matched.
 */

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libc.locale.b0156s3;

namespace P = pbsd::lib_libc_locale::b0156s3;

using PST = P::pbsd_mbstate_t;

extern "C" {

/*
 * Layout-identical stand-in for oracle.c's pbsd_mbstate_t.  The oracle has C
 * linkage, so only the layout matters.
 */
union ref_mbstate_t {
	char		__mbstate8[128];
	long long	_mbstateL;
};

size_t	ref__EUC_mbrtowc_impl(wchar_t * __restrict, const char * __restrict,
	    size_t, ref_mbstate_t * __restrict, uint8_t, uint8_t, uint8_t,
	    uint8_t);
size_t	ref__EUC_wcrtomb_impl(char * __restrict, wchar_t,
	    ref_mbstate_t * __restrict, uint8_t, uint8_t, uint8_t, uint8_t);

size_t	ref__EUC_CN_mbrtowc(wchar_t * __restrict, const char * __restrict,
	    size_t, ref_mbstate_t * __restrict);
size_t	ref__EUC_JP_mbrtowc(wchar_t * __restrict, const char * __restrict,
	    size_t, ref_mbstate_t * __restrict);
size_t	ref__EUC_KR_mbrtowc(wchar_t * __restrict, const char * __restrict,
	    size_t, ref_mbstate_t * __restrict);
size_t	ref__EUC_TW_mbrtowc(wchar_t * __restrict, const char * __restrict,
	    size_t, ref_mbstate_t * __restrict);

size_t	ref__EUC_CN_wcrtomb(char * __restrict, wchar_t,
	    ref_mbstate_t * __restrict);
size_t	ref__EUC_JP_wcrtomb(char * __restrict, wchar_t,
	    ref_mbstate_t * __restrict);
size_t	ref__EUC_KR_wcrtomb(char * __restrict, wchar_t,
	    ref_mbstate_t * __restrict);
size_t	ref__EUC_TW_wcrtomb(char * __restrict, wchar_t,
	    ref_mbstate_t * __restrict);

int	ref__EUC_mbsinit(const ref_mbstate_t *);

}  /* extern "C" */

/* ------------------------------------------------------------------ */
/* Bookkeeping                                                        */
/* ------------------------------------------------------------------ */

enum {
	F_MBSINIT,
	F_MBRTOWC_IMPL,
	F_WCRTOMB_IMPL,
	F_CN_MBRTOWC,
	F_JP_MBRTOWC,
	F_KR_MBRTOWC,
	F_TW_MBRTOWC,
	F_CN_WCRTOMB,
	F_JP_WCRTOMB,
	F_KR_WCRTOMB,
	F_TW_WCRTOMB,
	NFUNC
};

static const char *const FNAME[NFUNC] = {
	"_EUC_mbsinit",
	"_EUC_mbrtowc_impl",
	"_EUC_wcrtomb_impl",
	"_EUC_CN_mbrtowc",
	"_EUC_JP_mbrtowc",
	"_EUC_KR_mbrtowc",
	"_EUC_TW_mbrtowc",
	"_EUC_CN_wcrtomb",
	"_EUC_JP_wcrtomb",
	"_EUC_KR_wcrtomb",
	"_EUC_TW_wcrtomb",
};

static unsigned long g_cases[NFUNC];
static unsigned long g_fails[NFUNC];

static const unsigned long MAXREPORT = 5;

static void
note_fail(int id, const char *ctx, const char *what)
{

	g_fails[id]++;
	if (g_fails[id] <= MAXREPORT)
		printf("  FAIL %-18s %-72s : %s\n", FNAME[id], ctx, what);
}

static long long
sret(size_t v)
{

	return ((long long)(ptrdiff_t)v);
}

static void
hexstr(char *out, size_t outsz, const void *p, size_t n)
{
	const unsigned char *b = (const unsigned char *)p;
	size_t o = 0;
	size_t i;

	if (outsz == 0)
		return;
	out[0] = '\0';
	for (i = 0; i < n; i++) {
		int k = snprintf(out + o, outsz - o, "%s%02x",
		    i == 0 ? "" : " ", b[i]);
		if (k < 0 || (size_t)k >= outsz - o)
			break;
		o += (size_t)k;
	}
}

static void
report_bufdiff(int id, const char *ctx, const char *which, const void *a,
    const void *b, size_t n)
{
	char msg[256];
	char ha[96], hb[96];
	size_t i, lo, hi;

	for (i = 0; i < n; i++)
		if (((const unsigned char *)a)[i] != ((const unsigned char *)b)[i])
			break;
	lo = i;
	hi = lo + 8 > n ? n : lo + 8;
	hexstr(ha, sizeof ha, (const unsigned char *)a + lo, hi - lo);
	hexstr(hb, sizeof hb, (const unsigned char *)b + lo, hi - lo);
	snprintf(msg, sizeof msg, "%s differs at byte %zu: port[%s] ref[%s]",
	    which, lo, ha, hb);
	note_fail(id, ctx, msg);
}

/* ------------------------------------------------------------------ */
/* Shared scaffolding                                                 */
/* ------------------------------------------------------------------ */

/* Must match the _EucState of euc.c exactly. */
struct EucFields {
	wchar_t	ch;
	int	set;
	int	want;
};

union StBuf {
	long long	align[16];
	char		b[128];
};

union WBuf {
	wchar_t		w[8];
	unsigned char	b[32];
};

static constexpr size_t STSZ = 128;
static constexpr size_t SBUFSZ = 32;	/* mbrtowc input buffer */
static constexpr size_t SOFF = 8;	/* input sits here, guards each side */
static constexpr size_t DBUFSZ = 24;	/* wcrtomb output buffer */
static constexpr size_t DOFF = 8;	/* s = buf + DOFF */

struct CsTuple {
	uint8_t	cs2, cs2w, cs3, cs3w;
};

/*
 * The four tuples euc.c actually instantiates, plus synthetic ones that reach
 * corners of the shared implementations the real four cannot: a zero width
 * (want == 0, so the `i < want' test is taken at 0 == 0), widths above
 * MB_CUR_MAX, cs2 == cs3, lead bytes on either side of 0xa1, and a width that
 * cannot be satisfied by any n.
 */
static const CsTuple TUPLES[] = {
	{ 0x8e, 4, 0x00, 0 },		/* EUC-CN, EUC-TW */
	{ 0x8e, 2, 0x8f, 3 },		/* EUC-JP */
	{ 0x00, 0, 0x00, 0 },		/* EUC-KR */
	{ 0x8e, 0, 0x8f, 3 },		/* synthetic: cs2width == 0 */
	{ 0x8e, 3, 0x8f, 0 },		/* synthetic: cs3width == 0 */
	{ 0x80, 1, 0x81, 2 },		/* synthetic: width 1, low leads */
	{ 0xa0, 3, 0x9f, 1 },		/* synthetic: lead just below CS1 */
	{ 0x8e, 3, 0x8e, 2 },		/* synthetic: cs2 == cs3 */
	{ 0xa1, 4, 0xff, 3 },		/* synthetic: leads masked by CS1 */
	{ 0x8e, 5, 0x8f, 9 },		/* synthetic: widths > MB_CUR_MAX */
};
static const size_t NTUPLES = sizeof(TUPLES) / sizeof(TUPLES[0]);

/* ------------------------------------------------------------------ */
/* _EUC_mbsinit                                                       */
/* ------------------------------------------------------------------ */

static void
mbsinit_case(int ch, int set, int want, int null_ps)
{
	StBuf sp, sr;
	EucFields es;
	char ctx[128];
	int pr, rr, pe, re;

	memset(sp.b, 0x7f, STSZ);
	memset(sr.b, 0x7f, STSZ);
	es.ch = (wchar_t)ch;
	es.set = set;
	es.want = want;
	memcpy(sp.b, &es, sizeof es);
	memcpy(sr.b, &es, sizeof es);

	errno = 0;
	pr = P::_EUC_mbsinit(null_ps ? (const PST *)0 : (const PST *)sp.b);
	pe = errno;
	errno = 0;
	rr = ref__EUC_mbsinit(null_ps ? (const ref_mbstate_t *)0 :
	    (const ref_mbstate_t *)sr.b);
	re = errno;

	g_cases[F_MBSINIT]++;
	snprintf(ctx, sizeof ctx, "ps=%s st={ch=%#x,set=%d,want=%d}",
	    null_ps ? "NULL" : "buf", (unsigned)ch, set, want);
	if (pr != rr) {
		char m[64];
		snprintf(m, sizeof m, "ret port=%d ref=%d", pr, rr);
		note_fail(F_MBSINIT, ctx, m);
		return;
	}
	if (pe != re) {
		char m[64];
		snprintf(m, sizeof m, "errno port=%d ref=%d", pe, re);
		note_fail(F_MBSINIT, ctx, m);
		return;
	}
	if (memcmp(sp.b, sr.b, STSZ) != 0) {
		report_bufdiff(F_MBSINIT, ctx, "mbstate", sp.b, sr.b, STSZ);
		return;
	}
}

/* ------------------------------------------------------------------ */
/* mbrtowc family                                                     */
/* ------------------------------------------------------------------ */

struct MbrIn {
	unsigned char	in[16];
	unsigned	inlen;
	size_t		n;
	int		st_ch;
	int		st_set;
	int		st_want;
	int		pwc_null;
	int		s_null;
};

struct MbrOut {
	size_t		ret;
	int		err;
	char		sb[SBUFSZ];
	unsigned char	wb[32];
	char		st[STSZ];
};

template <class St, class Inv>
static void
mbr_run(MbrOut &o, Inv inv, const MbrIn &a)
{
	StBuf st;
	WBuf wb;
	char sb[SBUFSZ];
	EucFields es;
	wchar_t *pwc;
	const char *s;

	memset(st.b, 0x7f, STSZ);
	memset(wb.b, 0x7f, sizeof wb.b);
	memset(sb, 0x7f, sizeof sb);
	es.ch = (wchar_t)a.st_ch;
	es.set = a.st_set;
	es.want = a.st_want;
	memcpy(st.b, &es, sizeof es);
	memcpy(sb + SOFF, a.in, a.inlen);

	pwc = a.pwc_null ? (wchar_t *)0 : &wb.w[2];
	s = a.s_null ? (const char *)0 : sb + SOFF;

	errno = 0;
	o.ret = inv(pwc, s, a.n, (St *)st.b);
	o.err = errno;
	memcpy(o.sb, sb, sizeof sb);
	memcpy(o.wb, wb.b, sizeof wb.b);
	memcpy(o.st, st.b, STSZ);
}

static void
mbr_check(int id, const MbrIn &a, const MbrOut &p, const MbrOut &r,
    const char *tag)
{
	char ctx[224];
	char hx[64];

	g_cases[id]++;
	hexstr(hx, sizeof hx, a.in, a.inlen);
	snprintf(ctx, sizeof ctx,
	    "%s in=[%s] n=%zu st={ch=%#x,set=%d,want=%d} pwc=%s s=%s",
	    tag, hx, a.n, (unsigned)a.st_ch, a.st_set, a.st_want,
	    a.pwc_null ? "NULL" : "buf", a.s_null ? "NULL" : "buf");

	if (p.ret != r.ret) {
		char m[96];
		snprintf(m, sizeof m, "ret port=%lld ref=%lld", sret(p.ret),
		    sret(r.ret));
		note_fail(id, ctx, m);
		return;
	}
	if (p.err != r.err) {
		char m[96];
		snprintf(m, sizeof m, "errno port=%d ref=%d", p.err, r.err);
		note_fail(id, ctx, m);
		return;
	}
	if (memcmp(p.wb, r.wb, sizeof p.wb) != 0) {
		report_bufdiff(id, ctx, "*pwc buffer", p.wb, r.wb,
		    sizeof p.wb);
		return;
	}
	if (memcmp(p.st, r.st, STSZ) != 0) {
		report_bufdiff(id, ctx, "mbstate", p.st, r.st, STSZ);
		return;
	}
	if (memcmp(p.sb, r.sb, SBUFSZ) != 0) {
		report_bufdiff(id, ctx, "input buffer", p.sb, r.sb, SBUFSZ);
		return;
	}
}

static void
mbr_case_impl(const CsTuple &t, const MbrIn &a)
{
	MbrOut p, r;
	char tag[48];

	mbr_run<PST>(p, [&](wchar_t *pw, const char *s, size_t n, PST *st) {
		return P::_EUC_mbrtowc_impl(pw, s, n, st, t.cs2, t.cs2w,
		    t.cs3, t.cs3w);
	}, a);
	mbr_run<ref_mbstate_t>(r, [&](wchar_t *pw, const char *s, size_t n,
	    ref_mbstate_t *st) {
		return ref__EUC_mbrtowc_impl(pw, s, n, st, t.cs2, t.cs2w,
		    t.cs3, t.cs3w);
	}, a);

	snprintf(tag, sizeof tag, "impl(%02x,%u,%02x,%u)", t.cs2,
	    (unsigned)t.cs2w, t.cs3, (unsigned)t.cs3w);
	mbr_check(F_MBRTOWC_IMPL, a, p, r, tag);
}

typedef size_t (*pmbr_t)(wchar_t * __restrict, const char * __restrict, size_t,
    PST * __restrict);
typedef size_t (*rmbr_t)(wchar_t * __restrict, const char * __restrict, size_t,
    ref_mbstate_t * __restrict);

struct MbrWrap {
	int	id;
	pmbr_t	pf;
	rmbr_t	rf;
};

static const MbrWrap MBRWRAP[] = {
	{ F_CN_MBRTOWC, P::_EUC_CN_mbrtowc, ref__EUC_CN_mbrtowc },
	{ F_JP_MBRTOWC, P::_EUC_JP_mbrtowc, ref__EUC_JP_mbrtowc },
	{ F_KR_MBRTOWC, P::_EUC_KR_mbrtowc, ref__EUC_KR_mbrtowc },
	{ F_TW_MBRTOWC, P::_EUC_TW_mbrtowc, ref__EUC_TW_mbrtowc },
};
static const size_t NMBRWRAP = sizeof(MBRWRAP) / sizeof(MBRWRAP[0]);

static void
mbr_case_wrap(const MbrWrap &w, const MbrIn &a)
{
	MbrOut p, r;

	mbr_run<PST>(p, [&](wchar_t *pw, const char *s, size_t n, PST *st) {
		return w.pf(pw, s, n, st);
	}, a);
	mbr_run<ref_mbstate_t>(r, [&](wchar_t *pw, const char *s, size_t n,
	    ref_mbstate_t *st) {
		return w.rf(pw, s, n, st);
	}, a);
	mbr_check(w.id, a, p, r, "wrapper");
}

static void
mbr_case_all(const MbrIn &a)
{
	size_t i;

	for (i = 0; i < NTUPLES; i++)
		mbr_case_impl(TUPLES[i], a);
	for (i = 0; i < NMBRWRAP; i++)
		mbr_case_wrap(MBRWRAP[i], a);
}

/* ------------------------------------------------------------------ */
/* wcrtomb family                                                     */
/* ------------------------------------------------------------------ */

struct WcrIn {
	int	wc;
	int	st_ch;
	int	st_set;
	int	st_want;
	int	s_null;
};

struct WcrOut {
	size_t	ret;
	int	err;
	char	b[DBUFSZ];
	char	st[STSZ];
};

template <class St, class Inv>
static void
wcr_run(WcrOut &o, Inv inv, const WcrIn &a)
{
	StBuf st;
	char b[DBUFSZ];
	EucFields es;
	char *s;

	memset(st.b, 0x7f, STSZ);
	memset(b, 0x7f, sizeof b);
	es.ch = (wchar_t)a.st_ch;
	es.set = a.st_set;
	es.want = a.st_want;
	memcpy(st.b, &es, sizeof es);

	s = a.s_null ? (char *)0 : b + DOFF;

	errno = 0;
	o.ret = inv(s, (wchar_t)a.wc, (St *)st.b);
	o.err = errno;
	memcpy(o.b, b, sizeof b);
	memcpy(o.st, st.b, STSZ);
}

static void
wcr_check(int id, const WcrIn &a, const WcrOut &p, const WcrOut &r,
    const char *tag)
{
	char ctx[224];

	g_cases[id]++;
	snprintf(ctx, sizeof ctx,
	    "%s wc=%#x st={ch=%#x,set=%d,want=%d} s=%s", tag,
	    (unsigned)a.wc, (unsigned)a.st_ch, a.st_set, a.st_want,
	    a.s_null ? "NULL" : "buf");

	if (p.ret != r.ret) {
		char m[96];
		snprintf(m, sizeof m, "ret port=%lld ref=%lld", sret(p.ret),
		    sret(r.ret));
		note_fail(id, ctx, m);
		return;
	}
	if (p.err != r.err) {
		char m[96];
		snprintf(m, sizeof m, "errno port=%d ref=%d", p.err, r.err);
		note_fail(id, ctx, m);
		return;
	}
	if (memcmp(p.b, r.b, DBUFSZ) != 0) {
		report_bufdiff(id, ctx, "output buffer", p.b, r.b, DBUFSZ);
		return;
	}
	if (memcmp(p.st, r.st, STSZ) != 0) {
		report_bufdiff(id, ctx, "mbstate", p.st, r.st, STSZ);
		return;
	}
}

static void
wcr_case_impl(const CsTuple &t, const WcrIn &a)
{
	WcrOut p, r;
	char tag[48];

	wcr_run<PST>(p, [&](char *s, wchar_t wc, PST *st) {
		return P::_EUC_wcrtomb_impl(s, wc, st, t.cs2, t.cs2w, t.cs3,
		    t.cs3w);
	}, a);
	wcr_run<ref_mbstate_t>(r, [&](char *s, wchar_t wc, ref_mbstate_t *st) {
		return ref__EUC_wcrtomb_impl(s, wc, st, t.cs2, t.cs2w, t.cs3,
		    t.cs3w);
	}, a);

	snprintf(tag, sizeof tag, "impl(%02x,%u,%02x,%u)", t.cs2,
	    (unsigned)t.cs2w, t.cs3, (unsigned)t.cs3w);
	wcr_check(F_WCRTOMB_IMPL, a, p, r, tag);
}

typedef size_t (*pwcr_t)(char * __restrict, wchar_t, PST * __restrict);
typedef size_t (*rwcr_t)(char * __restrict, wchar_t, ref_mbstate_t *
    __restrict);

struct WcrWrap {
	int	id;
	pwcr_t	pf;
	rwcr_t	rf;
};

static const WcrWrap WCRWRAP[] = {
	{ F_CN_WCRTOMB, P::_EUC_CN_wcrtomb, ref__EUC_CN_wcrtomb },
	{ F_JP_WCRTOMB, P::_EUC_JP_wcrtomb, ref__EUC_JP_wcrtomb },
	{ F_KR_WCRTOMB, P::_EUC_KR_wcrtomb, ref__EUC_KR_wcrtomb },
	{ F_TW_WCRTOMB, P::_EUC_TW_wcrtomb, ref__EUC_TW_wcrtomb },
};
static const size_t NWCRWRAP = sizeof(WCRWRAP) / sizeof(WCRWRAP[0]);

static void
wcr_case_wrap(const WcrWrap &w, const WcrIn &a)
{
	WcrOut p, r;

	wcr_run<PST>(p, [&](char *s, wchar_t wc, PST *st) {
		return w.pf(s, wc, st);
	}, a);
	wcr_run<ref_mbstate_t>(r, [&](char *s, wchar_t wc, ref_mbstate_t *st) {
		return w.rf(s, wc, st);
	}, a);
	wcr_check(w.id, a, p, r, "wrapper");
}

static void
wcr_case_all(const WcrIn &a)
{
	size_t i;

	for (i = 0; i < NTUPLES; i++)
		wcr_case_impl(TUPLES[i], a);
	for (i = 0; i < NWCRWRAP; i++)
		wcr_case_wrap(WCRWRAP[i], a);
}

/* ------------------------------------------------------------------ */
/* Stateful driving: feed a byte stream in chunks, comparing after     */
/* every single call, until the stream is exhausted.                  */
/* ------------------------------------------------------------------ */

template <class InvP, class InvR>
static void
stream_drive(int id, InvP pinv, InvR rinv, const unsigned char *data,
    size_t len, size_t chunk, const char *tag)
{
	StBuf stp, str;
	WBuf wbp, wbr;
	char sbp[SBUFSZ * 2], sbr[SBUFSZ * 2];
	EucFields es;
	size_t pos, step;

	memset(stp.b, 0x7f, STSZ);
	memset(str.b, 0x7f, STSZ);
	es.ch = 0;
	es.set = 0;
	es.want = 0;
	memcpy(stp.b, &es, sizeof es);
	memcpy(str.b, &es, sizeof es);
	memset(sbp, 0x7f, sizeof sbp);
	memset(sbr, 0x7f, sizeof sbr);
	memcpy(sbp + SOFF, data, len);
	memcpy(sbr + SOFF, data, len);

	pos = 0;
	for (step = 0; pos < len && step < 64; step++) {
		size_t n = chunk > len - pos ? len - pos : chunk;
		size_t pr, rr, adv;
		int pe, re;
		char ctx[224];
		char hx[64];

		memset(wbp.b, 0x7f, sizeof wbp.b);
		memset(wbr.b, 0x7f, sizeof wbr.b);

		errno = 0;
		pr = pinv(&wbp.w[2], sbp + SOFF + pos, n, &stp);
		pe = errno;
		errno = 0;
		rr = rinv(&wbr.w[2], sbr + SOFF + pos, n, &str);
		re = errno;

		g_cases[id]++;
		hexstr(hx, sizeof hx, data, len);
		snprintf(ctx, sizeof ctx,
		    "%s stream=[%s] chunk=%zu step=%zu off=%zu n=%zu",
		    tag, hx, chunk, step, pos, n);

		if (pr != rr) {
			char m[96];
			snprintf(m, sizeof m, "ret port=%lld ref=%lld",
			    sret(pr), sret(rr));
			note_fail(id, ctx, m);
			return;
		}
		if (pe != re) {
			char m[96];
			snprintf(m, sizeof m, "errno port=%d ref=%d", pe, re);
			note_fail(id, ctx, m);
			return;
		}
		if (memcmp(wbp.b, wbr.b, sizeof wbp.b) != 0) {
			report_bufdiff(id, ctx, "*pwc buffer", wbp.b, wbr.b,
			    sizeof wbp.b);
			return;
		}
		if (memcmp(stp.b, str.b, STSZ) != 0) {
			report_bufdiff(id, ctx, "mbstate", stp.b, str.b, STSZ);
			return;
		}
		if (memcmp(sbp, sbr, sizeof sbp) != 0) {
			report_bufdiff(id, ctx, "input buffer", sbp, sbr,
			    sizeof sbp);
			return;
		}

		if (rr == (size_t)-1)
			break;
		if (rr == (size_t)-2)
			adv = n;
		else
			adv = rr == 0 ? 1 : rr;
		if (adv == 0)
			adv = 1;
		if (adv > len - pos)
			adv = len - pos;
		pos += adv;
	}
}

static void
stream_case_impl(const CsTuple &t, const unsigned char *data, size_t len,
    size_t chunk)
{
	char tag[48];

	snprintf(tag, sizeof tag, "impl(%02x,%u,%02x,%u)", t.cs2,
	    (unsigned)t.cs2w, t.cs3, (unsigned)t.cs3w);
	stream_drive(F_MBRTOWC_IMPL,
	    [&](wchar_t *pw, const char *s, size_t n, StBuf *st) {
		return P::_EUC_mbrtowc_impl(pw, s, n, (PST *)st->b, t.cs2,
		    t.cs2w, t.cs3, t.cs3w);
	    },
	    [&](wchar_t *pw, const char *s, size_t n, StBuf *st) {
		return ref__EUC_mbrtowc_impl(pw, s, n, (ref_mbstate_t *)st->b,
		    t.cs2, t.cs2w, t.cs3, t.cs3w);
	    }, data, len, chunk, tag);
}

static void
stream_case_wrap(const MbrWrap &w, const unsigned char *data, size_t len,
    size_t chunk)
{

	stream_drive(w.id,
	    [&](wchar_t *pw, const char *s, size_t n, StBuf *st) {
		return w.pf(pw, s, n, (PST *)st->b);
	    },
	    [&](wchar_t *pw, const char *s, size_t n, StBuf *st) {
		return w.rf(pw, s, n, (ref_mbstate_t *)st->b);
	    }, data, len, chunk, "wrapper");
}

/* ------------------------------------------------------------------ */
/* Fixed-seed PRNG                                                    */
/* ------------------------------------------------------------------ */

static uint64_t g_rs = 0x123456789abcdefULL;

static uint32_t
nrand(void)
{

	g_rs ^= g_rs << 13;
	g_rs ^= g_rs >> 7;
	g_rs ^= g_rs << 17;
	return ((uint32_t)(g_rs >> 32));
}

static unsigned
nrange(unsigned lim)
{

	return (nrand() % lim);
}

/* Bytes biased to the values that decide euc.c's branches. */
static unsigned char
rbyte(void)
{
	static const unsigned char pool[] = {
		0x00, 0x01, 0x7f, 0x80, 0x81, 0x8d, 0x8e, 0x8f, 0x90, 0x9f,
		0xa0, 0xa1, 0xa2, 0xfe, 0xff
	};

	if (nrange(4) != 0)
		return (pool[nrange(sizeof pool)]);
	return ((unsigned char)nrand());
}

/* wchar_t values biased to every length/range boundary of wcrtomb. */
static int
rwc(void)
{
	static const int pool[] = {
		0x0, 0x1, 0x41, 0x7e, 0x7f, 0x80, 0x81, 0xa0, 0xa1, 0xfe,
		0xff, 0x100, 0x101, 0x8e, 0x8f, 0x8e00, 0x8e41, 0x8eff,
		0x8f00, 0x8f41, 0x8fff, 0x9fff, 0xa0ff, 0xa100, 0xa101,
		0xa1a1, 0xfffe, 0xffff, 0x10000, 0x10001, 0x8e0000,
		0x8e4142, 0x8f0000, 0x8f4142, 0xa10000, 0xfffffe, 0xffffff,
		0x1000000, 0x1000001, (int)0x8e000000, (int)0x8e414243, (int)0x8f414243,
		0x7fffffff, (int)0x80000000, -1, -2, (int)0xff000000
	};

	switch (nrange(4)) {
	case 0:
		return ((int)nrand());
	case 1:
		return ((int)(nrand() & 0xffff));
	default:
		return (pool[nrange(sizeof pool / sizeof pool[0])]);
	}
}

static int
rwant(void)
{

	/* -2 .. 9: straddles 0 and MB_CUR_MAX (4) on both sides. */
	return ((int)nrange(12) - 2);
}

static int
rch(void)
{

	switch (nrange(4)) {
	case 0:
		return (0);
	case 1:
		return ((int)(nrand() & 0xff));
	case 2:
		return ((int)(nrand() & 0xffffff));
	default:
		return ((int)nrand());
	}
}

static int
rset(void)
{

	switch (nrange(3)) {
	case 0:
		return (0);
	case 1:
		return (0x5a5a5a5a);
	default:
		return ((int)nrand());
	}
}

/* ------------------------------------------------------------------ */
/* Hand-written case tables                                           */
/* ------------------------------------------------------------------ */

static const MbrIn MBR_CASES[] = {
	/* in bytes,                         inlen, n, ch, set, want, pn, sn */

	/* empty / n == 0 boundary */
	{ { 0 },				0, 0, 0, 0, 0, 0, 0 },
	{ { 0x41 },				1, 0, 0, 0, 0, 0, 0 },
	{ { 0x41 },				1, 1, 0, 0, 0, 0, 0 },
	{ { 0xa1 },				1, 0, 0, 0, 0, 0, 0 },
	{ { 0x41 },				1, 0, 0, 0, 2, 0, 0 },

	/* ASCII (CS0) fast path, including NUL */
	{ { 0x00 },				1, 1, 0, 0, 0, 0, 0 },
	{ { 0x00 },				1, 1, 0, 0, 0, 1, 0 },
	{ { 0x00, 0x00 },			2, 2, 0, 0, 0, 0, 0 },
	{ { 0x01 },				1, 1, 0, 0, 0, 0, 0 },
	{ { 0x41 },				1, 4, 0, 0, 0, 0, 0 },
	{ { 0x7e },				1, 1, 0, 0, 0, 0, 0 },
	{ { 0x7f },				1, 1, 0, 0, 0, 0, 0 },
	{ { 0x7f },				1, 8, 0, 0, 0, 1, 0 },

	/* every interesting lead byte with the high bit set */
	{ { 0x80 },				1, 1, 0, 0, 0, 0, 0 },
	{ { 0x81 },				1, 1, 0, 0, 0, 0, 0 },
	{ { 0x8d },				1, 1, 0, 0, 0, 0, 0 },
	{ { 0x8e },				1, 1, 0, 0, 0, 0, 0 },
	{ { 0x8f },				1, 1, 0, 0, 0, 0, 0 },
	{ { 0x90 },				1, 1, 0, 0, 0, 0, 0 },
	{ { 0x9f },				1, 1, 0, 0, 0, 0, 0 },
	{ { 0xa0 },				1, 1, 0, 0, 0, 0, 0 },
	{ { 0xa1 },				1, 1, 0, 0, 0, 0, 0 },
	{ { 0xa2 },				1, 1, 0, 0, 0, 0, 0 },
	{ { 0xfe },				1, 1, 0, 0, 0, 0, 0 },
	{ { 0xff },				1, 1, 0, 0, 0, 0, 0 },

	/* CS1 (two bytes), n on both sides of the requirement */
	{ { 0xa1, 0xa1 },			2, 1, 0, 0, 0, 0, 0 },
	{ { 0xa1, 0xa1 },			2, 2, 0, 0, 0, 0, 0 },
	{ { 0xa1, 0xa1 },			2, 3, 0, 0, 0, 0, 0 },
	{ { 0xa1, 0xa1 },			2, 8, 0, 0, 0, 0, 0 },
	{ { 0xa1, 0xa1 },			2, 2, 0, 0, 0, 1, 0 },
	{ { 0xa1, 0x00 },			2, 2, 0, 0, 0, 0, 0 },
	{ { 0xa1, 0x7f },			2, 2, 0, 0, 0, 0, 0 },
	{ { 0xa1, 0x80 },			2, 2, 0, 0, 0, 0, 0 },
	{ { 0xff, 0xff },			2, 2, 0, 0, 0, 0, 0 },
	{ { 0xa1, 0xa1, 0xa1 },			3, 3, 0, 0, 0, 0, 0 },

	/* SS2 introducer, every length and every truncation of it */
	{ { 0x8e },				1, 1, 0, 0, 0, 0, 0 },
	{ { 0x8e, 0x41 },			2, 1, 0, 0, 0, 0, 0 },
	{ { 0x8e, 0x41 },			2, 2, 0, 0, 0, 0, 0 },
	{ { 0x8e, 0x41, 0x42 },			3, 2, 0, 0, 0, 0, 0 },
	{ { 0x8e, 0x41, 0x42 },			3, 3, 0, 0, 0, 0, 0 },
	{ { 0x8e, 0x41, 0x42, 0x43 },		4, 3, 0, 0, 0, 0, 0 },
	{ { 0x8e, 0x41, 0x42, 0x43 },		4, 4, 0, 0, 0, 0, 0 },
	{ { 0x8e, 0x41, 0x42, 0x43 },		4, 4, 0, 0, 0, 1, 0 },
	{ { 0x8e, 0x41, 0x42, 0x43 },		4, 8, 0, 0, 0, 0, 0 },
	{ { 0x8e, 0x41, 0x42, 0x43, 0x44 },	5, 5, 0, 0, 0, 0, 0 },
	{ { 0x8e, 0x00, 0x00, 0x00 },		4, 4, 0, 0, 0, 0, 0 },
	{ { 0x8e, 0xff, 0xff, 0xff },		4, 4, 0, 0, 0, 0, 0 },
	{ { 0x8e, 0x80, 0x00, 0x7f },		4, 4, 0, 0, 0, 0, 0 },

	/* SS3 introducer */
	{ { 0x8f },				1, 1, 0, 0, 0, 0, 0 },
	{ { 0x8f, 0x41 },			2, 2, 0, 0, 0, 0, 0 },
	{ { 0x8f, 0x41, 0x42 },			3, 2, 0, 0, 0, 0, 0 },
	{ { 0x8f, 0x41, 0x42 },			3, 3, 0, 0, 0, 0, 0 },
	{ { 0x8f, 0x00, 0x00 },			3, 3, 0, 0, 0, 0, 0 },
	{ { 0x8f, 0x41, 0x42, 0x43 },		4, 4, 0, 0, 0, 0, 0 },

	/* s == NULL (the "" / n = 1 / pwc = NULL rewrite) */
	{ { 0 },				0, 0, 0, 0, 0, 0, 1 },
	{ { 0x41 },				1, 4, 0, 0, 0, 0, 1 },
	{ { 0x41 },				1, 4, 0, 0, 2, 0, 1 },
	{ { 0x41 },				1, 4, 0, 0, 5, 0, 1 },

	/* continuation states: want on both sides of 0 and of MB_CUR_MAX */
	{ { 0x41 },				1, 1, 0, 0, 1, 0, 0 },
	{ { 0x00 },				1, 1, 0, 0, 1, 0, 0 },
	{ { 0x00, 0x00 },			2, 2, 0, 0, 2, 0, 0 },
	{ { 0x00, 0x00, 0x00 },			3, 3, 0, 0, 3, 0, 0 },
	{ { 0x00, 0x00, 0x00, 0x00 },		4, 4, 0, 0, 4, 0, 0 },
	{ { 0xa1 },				1, 1, 0xa1, 0, 1, 0, 0 },
	{ { 0xa1, 0xa1 },			2, 2, 0x8e, 0, 2, 0, 0 },
	{ { 0x41, 0x42, 0x43 },			3, 3, 0x8e, 0, 3, 0, 0 },
	{ { 0x41, 0x42, 0x43, 0x44 },		4, 4, 0x8e41, 0, 4, 0, 0 },
	{ { 0x41, 0x42, 0x43, 0x44 },		4, 3, 0x8e41, 0, 4, 0, 0 },
	{ { 0x41, 0x42, 0x43, 0x44 },		4, 1, 0x8e41, 0, 4, 0, 0 },
	{ { 0x41, 0x42, 0x43, 0x44 },		4, 4, 0, 0, 5, 0, 0 },
	{ { 0x41 },				1, 1, 0, 0, 6, 0, 0 },
	{ { 0x41 },				1, 1, 0, 0, 9, 0, 0 },
	{ { 0x41 },				1, 1, 0, 0, -1, 0, 0 },
	{ { 0x41 },				1, 1, 0, 0, -2, 0, 0 },
	{ { 0x41 },				1, 0, 0, 0, -1, 0, 0 },
	{ { 0xff, 0xff, 0xff, 0xff },		4, 4, -1, 0, 4, 0, 0 },
	{ { 0x41 },				1, 1, 0x7fffffff, 0, 2, 0, 0 },
	{ { 0x41, 0x42 },			2, 2, 0x00ffffff, 0, 2, 0, 0 },

	/* the `set' field must be carried through untouched */
	{ { 0xa1, 0xa1 },			2, 2, 0, 0x5a5a5a5a, 0, 0, 0 },
	{ { 0x41 },				1, 1, 0, -1, 0, 0, 0 },
	{ { 0x8e, 0x41, 0x42, 0x43 },		4, 4, 0, 0x01020304, 0, 0, 0 },
	{ { 0x41 },				1, 1, 0x11223344, 0x55667788, 2, 0, 0 },
};
static const size_t NMBR_CASES = sizeof(MBR_CASES) / sizeof(MBR_CASES[0]);

static const WcrIn WCR_CASES[] = {
	/* wc,		ch, set, want, s_null */

	/* ASCII fast path and its boundary */
	{ 0x0,		0, 0, 0, 0 },
	{ 0x1,		0, 0, 0, 0 },
	{ 0x41,		0, 0, 0, 0 },
	{ 0x7e,		0, 0, 0, 0 },
	{ 0x7f,		0, 0, 0, 0 },
	{ 0x80,		0, 0, 0, 0 },

	/* len == 1 (0x80..0xff) */
	{ 0x81,		0, 0, 0, 0 },
	{ 0x8e,		0, 0, 0, 0 },
	{ 0x8f,		0, 0, 0, 0 },
	{ 0xa0,		0, 0, 0, 0 },
	{ 0xa1,		0, 0, 0, 0 },
	{ 0xfe,		0, 0, 0, 0 },
	{ 0xff,		0, 0, 0, 0 },

	/* len == 2, both sides of 0xff/0x100 and of the CS1 window */
	{ 0x100,	0, 0, 0, 0 },
	{ 0x101,	0, 0, 0, 0 },
	{ 0x8e00,	0, 0, 0, 0 },
	{ 0x8e41,	0, 0, 0, 0 },
	{ 0x8eff,	0, 0, 0, 0 },
	{ 0x8f00,	0, 0, 0, 0 },
	{ 0x8f41,	0, 0, 0, 0 },
	{ 0x8fff,	0, 0, 0, 0 },
	{ 0x9fff,	0, 0, 0, 0 },
	{ 0xa000,	0, 0, 0, 0 },
	{ 0xa0ff,	0, 0, 0, 0 },
	{ 0xa100,	0, 0, 0, 0 },
	{ 0xa101,	0, 0, 0, 0 },
	{ 0xa1a1,	0, 0, 0, 0 },
	{ 0xfffe,	0, 0, 0, 0 },
	{ 0xffff,	0, 0, 0, 0 },

	/* len == 3, both sides of 0xffff/0x10000 */
	{ 0x10000,	0, 0, 0, 0 },
	{ 0x10001,	0, 0, 0, 0 },
	{ 0x8e0000,	0, 0, 0, 0 },
	{ 0x8e4142,	0, 0, 0, 0 },
	{ 0x8f0000,	0, 0, 0, 0 },
	{ 0x8f4142,	0, 0, 0, 0 },
	{ 0x8fffff,	0, 0, 0, 0 },
	{ 0xa10000,	0, 0, 0, 0 },
	{ 0xfffffe,	0, 0, 0, 0 },
	{ 0xffffff,	0, 0, 0, 0 },

	/* len == 4, both sides of 0xffffff/0x1000000 */
	{ 0x1000000,	0, 0, 0, 0 },
	{ 0x1000001,	0, 0, 0, 0 },
	{ (int)0x8e000000,	0, 0, 0, 0 },
	{ (int)0x8e414243,	0, 0, 0, 0 },
	{ (int)0x8effffff,	0, 0, 0, 0 },
	{ (int)0x8f000000,	0, 0, 0, 0 },
	{ (int)0x8f414243,	0, 0, 0, 0 },
	{ (int)0xa1000000,	0, 0, 0, 0 },
	{ 0x7fffffff,	0, 0, 0, 0 },
	{ (int)0x80000000, 0, 0, 0, 0 },
	{ (int)0xff000000, 0, 0, 0, 0 },
	{ -1,		0, 0, 0, 0 },
	{ -2,		0, 0, 0, 0 },

	/* s == NULL, in and out of the initial state */
	{ 0x41,		0, 0, 0, 1 },
	{ 0xa1a1,	0, 0, 0, 1 },
	{ 0x41,		0, 0, 1, 1 },
	{ 0x41,		0, 0, -1, 1 },

	/* non-initial state: EINVAL, both signs and both sides of 0 */
	{ 0x41,		0, 0, 1, 0 },
	{ 0x41,		0, 0, 2, 0 },
	{ 0x41,		0, 0, 4, 0 },
	{ 0x41,		0, 0, 5, 0 },
	{ 0x41,		0, 0, -1, 0 },
	{ 0xa1a1,	0x8e, 0, 3, 0 },

	/* the ch/set fields must be carried through untouched */
	{ 0xa1a1,	0x11223344, 0x5a5a5a5a, 0, 0 },
	{ 0x41,		-1, -1, 0, 0 },
	{ (int)0x8e414243,	0x7fffffff, 0x01020304, 0, 0 },
};
static const size_t NWCR_CASES = sizeof(WCR_CASES) / sizeof(WCR_CASES[0]);

static const unsigned char STREAM_A[] = { 0x8e, 0x41, 0x42, 0x43 };
static const unsigned char STREAM_B[] = { 0xa1, 0xa1, 0x41, 0x00, 0xa1 };
static const unsigned char STREAM_C[] = { 0x8f, 0x00, 0x00, 0x00, 0x7f };
static const unsigned char STREAM_D[] = { 0x00, 0x00, 0x00, 0x00 };
static const unsigned char STREAM_E[] = { 0xff, 0xfe, 0x8e, 0x8f, 0x80, 0xa0 };
static const unsigned char STREAM_F[] = { 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e,
	0x8e, 0x8e };
static const unsigned char STREAM_G[] = { 0x41 };
static const unsigned char STREAM_H[] = { 0x80 };

struct StreamCase {
	const unsigned char	*data;
	size_t			 len;
};

static const StreamCase STREAMS[] = {
	{ STREAM_A, sizeof STREAM_A },
	{ STREAM_B, sizeof STREAM_B },
	{ STREAM_C, sizeof STREAM_C },
	{ STREAM_D, sizeof STREAM_D },
	{ STREAM_E, sizeof STREAM_E },
	{ STREAM_F, sizeof STREAM_F },
	{ STREAM_G, sizeof STREAM_G },
	{ STREAM_H, sizeof STREAM_H },
};
static const size_t NSTREAMS = sizeof(STREAMS) / sizeof(STREAMS[0]);

/* ------------------------------------------------------------------ */
/* main                                                              */
/* ------------------------------------------------------------------ */

static const long SWEEP_MBR = 200000;
static const long SWEEP_WCR = 200000;
static const long SWEEP_MBSINIT = 200000;
static const long SWEEP_STREAM = 20000;

int
main(void)
{
	size_t i, j, k;
	long it;
	unsigned long tc = 0, tf = 0;

	printf("b0156s3: euc.c -- port vs. oracle differential test\n\n");

	/* ---- hand-written cases ---- */

	for (i = 0; i < NMBR_CASES; i++)
		mbr_case_all(MBR_CASES[i]);

	for (i = 0; i < NWCR_CASES; i++)
		wcr_case_all(WCR_CASES[i]);

	{
		static const int wants[] = { 0, 1, 2, 3, 4, 5, -1, -2,
		    0x7fffffff, (int)0x80000000 };
		static const int chs[] = { 0, 1, -1, 0x8e41 };
		static const int sets[] = { 0, 0x5a5a5a5a, -1 };

		for (i = 0; i < sizeof wants / sizeof wants[0]; i++)
			for (j = 0; j < sizeof chs / sizeof chs[0]; j++)
				for (k = 0; k < sizeof sets / sizeof sets[0];
				    k++) {
					mbsinit_case(chs[j], sets[k], wants[i],
					    0);
					mbsinit_case(chs[j], sets[k], wants[i],
					    1);
				}
	}

	for (i = 0; i < NSTREAMS; i++)
		for (j = 1; j <= 4; j++) {
			for (k = 0; k < NTUPLES; k++)
				stream_case_impl(TUPLES[k], STREAMS[i].data,
				    STREAMS[i].len, j);
			for (k = 0; k < NMBRWRAP; k++)
				stream_case_wrap(MBRWRAP[k], STREAMS[i].data,
				    STREAMS[i].len, j);
		}

	/* ---- randomised sweeps (fixed seed) ---- */

	for (it = 0; it < SWEEP_MBR; it++) {
		MbrIn a;
		unsigned m;

		memset(&a, 0, sizeof a);
		a.inlen = nrange(9);
		for (m = 0; m < a.inlen; m++)
			a.in[m] = rbyte();
		a.n = nrange(10);
		a.st_want = nrange(6) == 0 ? rwant() : (int)nrange(5);
		a.st_ch = rch();
		a.st_set = rset();
		a.pwc_null = nrange(8) == 0;
		a.s_null = nrange(16) == 0;

		mbr_case_impl(TUPLES[nrange((unsigned)NTUPLES)], a);
		for (m = 0; m < NMBRWRAP; m++)
			mbr_case_wrap(MBRWRAP[m], a);
	}

	for (it = 0; it < SWEEP_WCR; it++) {
		WcrIn a;
		unsigned m;

		a.wc = rwc();
		a.st_want = nrange(4) == 0 ? rwant() : 0;
		a.st_ch = rch();
		a.st_set = rset();
		a.s_null = nrange(16) == 0;

		wcr_case_impl(TUPLES[nrange((unsigned)NTUPLES)], a);
		for (m = 0; m < NWCRWRAP; m++)
			wcr_case_wrap(WCRWRAP[m], a);
	}

	for (it = 0; it < SWEEP_MBSINIT; it++)
		mbsinit_case(rch(), rset(), rwant(), nrange(4) == 0);

	for (it = 0; it < SWEEP_STREAM; it++) {
		unsigned char data[12];
		size_t len = nrange(12) + 1;
		size_t chunk = nrange(4) + 1;
		unsigned m;

		for (m = 0; m < len; m++)
			data[m] = rbyte();

		if (nrange(2) == 0)
			stream_case_impl(TUPLES[nrange((unsigned)NTUPLES)],
			    data, len, chunk);
		else
			stream_case_wrap(MBRWRAP[nrange((unsigned)NMBRWRAP)],
			    data, len, chunk);
	}

	/* ---- report ---- */

	printf("\n%-22s %12s %10s  %s\n", "function", "cases", "failures",
	    "result");
	printf("%-22s %12s %10s  %s\n", "----------------------",
	    "------------", "----------", "------");
	for (i = 0; i < NFUNC; i++) {
		tc += g_cases[i];
		tf += g_fails[i];
		printf("%-22s %12lu %10lu  %s\n", FNAME[i], g_cases[i],
		    g_fails[i], g_fails[i] == 0 ? "ok" : "FAILED");
	}
	printf("%-22s %12s %10s  %s\n", "----------------------",
	    "------------", "----------", "------");
	printf("%-22s %12lu %10lu  %s\n", "TOTAL", tc, tf,
	    tf == 0 ? "ok" : "FAILED");

	return (tf == 0 ? 0 : 1);
}
