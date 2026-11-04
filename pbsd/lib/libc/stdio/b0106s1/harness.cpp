/*
 * harness.cpp -- differential test for PBSD batch b0106s1 (fputwc.c).
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <locale.h>

import pbsd.lib.libc.stdio.b0106s1;

namespace port = pbsd::lib_libc_stdio::b0106s1;

using shim_file = port::shim_file;

extern "C" {
struct pbsd_shim_file;

wint_t ref___fputwc(wchar_t, struct pbsd_shim_file *, locale_t);
wint_t ref_fputwc_l(wchar_t, struct pbsd_shim_file *, locale_t);
wint_t ref_fputwc(wchar_t, struct pbsd_shim_file *);

int pbsd_shim_init(void);
void pbsd_shim_file_bind(struct pbsd_shim_file *, unsigned char *, int,
    short, int);
extern locale_t pbsd_shim_locale_c;
extern locale_t pbsd_shim_locale_utf8;
extern unsigned long long pbsd_shim_sputc_calls;
extern unsigned long long pbsd_shim_sputc_trace;
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr std::size_t BUF_PRE = 16;
constexpr std::size_t BUF_USER = 128;
constexpr std::size_t BUF_POST = 16;
constexpr std::size_t BUF_CAP = BUF_PRE + BUF_USER + BUF_POST;
constexpr unsigned RAND_ITERS = 200000u;
constexpr int MAXPRINT = 8;

enum StatId {
	S___FPUTWC,
	S_FPUTWC_L,
	S_FPUTWC,
	NSTAT
};

struct Stats {
	const char *name;
	long long cases;
	long long fails;
	int printed;
};

Stats g_stat[NSTAT] = {
	{ "__fputwc", 0, 0, 0 },
	{ "fputwc_l", 0, 0, 0 },
	{ "fputwc",   0, 0, 0 },
};

std::uint64_t rng_state = 0xb0106a15faceULL;

std::uint64_t
rnd_u64(void)
{
	std::uint64_t z;

	rng_state += 0x9e3779b97f4a7c15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

unsigned
rnd_u32(void)
{
	return (unsigned)(rnd_u64() & 0xffffffffu);
}

void
fail_msg(StatId which, const char *label, const char *detail)
{
	g_stat[which].fails++;
	if (g_stat[which].printed++ < MAXPRINT)
		std::printf("  FAIL %-10s %-24s %s\n", g_stat[which].name, label,
		    detail);
}

void
case_inc(StatId which)
{
	g_stat[which].cases++;
}

struct CaseObs {
	wint_t ret;
	short flags;
	int w;
	int orientation;
	int lockdepth;
	int locktotal;
	unsigned long long sputc_calls;
	unsigned long long sputc_trace;
	unsigned char buf[BUF_CAP];
};

void
fill_guard(unsigned char *buf)
{
	std::memset(buf, GUARD, BUF_CAP);
}

void
bind_file(shim_file *fp, unsigned char *buf, int usercap, short flags,
    int orientation, const mbstate_t *mb)
{
	pbsd_shim_file_bind(reinterpret_cast<struct pbsd_shim_file *>(fp),
	    buf + BUF_PRE, usercap, flags, orientation);
	if (mb != nullptr)
		fp->_mbstate = *mb;
}

void
snapshot(const shim_file *fp, const unsigned char *raw, CaseObs *obs)
{
	obs->flags = fp->_flags;
	obs->w = fp->_w;
	obs->orientation = fp->_orientation;
	obs->lockdepth = fp->_lockdepth;
	obs->locktotal = fp->_locktotal;
	obs->sputc_calls = pbsd_shim_sputc_calls;
	obs->sputc_trace = pbsd_shim_sputc_trace;
	std::memcpy(obs->buf, raw, BUF_CAP);
}

bool
obs_eq(const CaseObs *a, const CaseObs *b, char *why, std::size_t whysz)
{
	if (a->ret != b->ret) {
		std::snprintf(why, whysz, "ret ref=%d port=%d", (int)a->ret,
		    (int)b->ret);
		return false;
	}
	if (a->flags != b->flags) {
		std::snprintf(why, whysz, "flags ref=%d port=%d", a->flags,
		    b->flags);
		return false;
	}
	if (a->w != b->w) {
		std::snprintf(why, whysz, "_w ref=%d port=%d", a->w, b->w);
		return false;
	}
	if (a->orientation != b->orientation) {
		std::snprintf(why, whysz, "orient ref=%d port=%d",
		    a->orientation, b->orientation);
		return false;
	}
	if (a->lockdepth != b->lockdepth) {
		std::snprintf(why, whysz, "lockdepth ref=%lld port=%lld",
		    (long long)a->lockdepth, (long long)b->lockdepth);
		return false;
	}
	if (a->locktotal != b->locktotal) {
		std::snprintf(why, whysz, "locktotal ref=%lld port=%lld",
		    (long long)a->locktotal, (long long)b->locktotal);
		return false;
	}
	if (a->sputc_calls != b->sputc_calls) {
		std::snprintf(why, whysz, "sputc_calls ref=%llu port=%llu",
		    (unsigned long long)a->sputc_calls,
		    (unsigned long long)b->sputc_calls);
		return false;
	}
	if (a->sputc_trace != b->sputc_trace) {
		std::snprintf(why, whysz, "sputc_trace ref=%llu port=%llu",
		    (unsigned long long)a->sputc_trace,
		    (unsigned long long)b->sputc_trace);
		return false;
	}
	if (std::memcmp(a->buf, b->buf, BUF_CAP) != 0) {
		std::snprintf(why, whysz, "buffer/guard mismatch");
		return false;
	}
	return true;
}

bool
test_one(StatId which, const char *label, wchar_t wc, locale_t loc,
    int usercap, short flags, int orientation, const mbstate_t *mb,
    bool null_locale)
{
	shim_file fp_r, fp_p;
	unsigned char raw_r[BUF_CAP], raw_p[BUF_CAP];
	CaseObs obs_r, obs_p;
	char why[160];
	wint_t ret_r, ret_p;
	locale_t call_loc = loc;
	bool ok = true;

	fill_guard(raw_r);
	fill_guard(raw_p);
	bind_file(&fp_r, raw_r, usercap, flags, orientation, mb);
	bind_file(&fp_p, raw_p, usercap, flags, orientation, mb);

	pbsd_shim_sputc_calls = 0;
	pbsd_shim_sputc_trace = 0;

	switch (which) {
	case S___FPUTWC:
		ret_r = ref___fputwc(wc,
		    reinterpret_cast<struct pbsd_shim_file *>(&fp_r), call_loc);
		ret_p = port::__fputwc(wc, &fp_p, call_loc);
		break;
	case S_FPUTWC_L:
		ret_r = ref_fputwc_l(wc,
		    reinterpret_cast<struct pbsd_shim_file *>(&fp_r),
		    null_locale ? nullptr : call_loc);
		ret_p = port::fputwc_l(wc, &fp_p,
		    null_locale ? nullptr : call_loc);
		break;
	default:
		ret_r = ref_fputwc(wc,
		    reinterpret_cast<struct pbsd_shim_file *>(&fp_r));
		ret_p = port::fputwc(wc, &fp_p);
		break;
	}

	obs_r.ret = ret_r;
	obs_p.ret = ret_p;
	snapshot(&fp_r, raw_r, &obs_r);
	snapshot(&fp_p, raw_p, &obs_p);

	if (!obs_eq(&obs_r, &obs_p, why, sizeof(why))) {
		fail_msg(which, label, why);
		ok = false;
	}

	case_inc(which);
	return ok;
}

void
run_edges(StatId which)
{
	mbstate_t mb0 = {};

	test_one(which, "ascii A", L'A', pbsd_shim_locale_c, 8,
	    port::shim_SWR, 0, &mb0, false);
	test_one(which, "nul wchar", L'\0', pbsd_shim_locale_c, 8,
	    port::shim_SWR, 0, &mb0, false);
	test_one(which, "high latin1", (wchar_t)0x00ff, pbsd_shim_locale_c, 8,
	    port::shim_SWR, 0, &mb0, false);
	test_one(which, "euro utf8", (wchar_t)0x20ac, pbsd_shim_locale_utf8, 8,
	    port::shim_SWR, 0, &mb0, false);
	test_one(which, "surrogate fail", (wchar_t)0xd800,
	    pbsd_shim_locale_utf8, 8, port::shim_SWR, 0, &mb0, false);
	test_one(which, "not for write", L'x', pbsd_shim_locale_c, 8, 0, 0,
	    &mb0, false);
	test_one(which, "zero room", L'x', pbsd_shim_locale_c, 0,
	    port::shim_SWR, 0, &mb0, false);
	test_one(which, "one byte room", (wchar_t)0x20ac,
	    pbsd_shim_locale_utf8, 1, port::shim_SWR, 0, &mb0, false);
	test_one(which, "exact fit 2", (wchar_t)0x00a2,
	    pbsd_shim_locale_utf8, 2, port::shim_SWR, 0, &mb0, false);
	test_one(which, "exact fit 3", (wchar_t)0x20ac,
	    pbsd_shim_locale_utf8, 3, port::shim_SWR, 0, &mb0, false);
	test_one(which, "orient preset -1", L'q', pbsd_shim_locale_c, 8,
	    port::shim_SWR, -1, &mb0, false);
	test_one(which, "orient preset 1", L'q', pbsd_shim_locale_c, 8,
	    port::shim_SWR, 1, &mb0, false);
	test_one(which, "max wchar", (wchar_t)0xffff, pbsd_shim_locale_c, 8,
	    port::shim_SWR, 0, &mb0, false);
	test_one(which, "0x80 byte", (wchar_t)0x80, pbsd_shim_locale_c, 8,
	    port::shim_SWR, 0, &mb0, false);
	test_one(which, "0xfe byte", (wchar_t)0xfe, pbsd_shim_locale_c, 8,
	    port::shim_SWR, 0, &mb0, false);

	if (which == S_FPUTWC_L)
		test_one(which, "null locale", L'9', pbsd_shim_locale_c, 8,
		    port::shim_SWR, 0, &mb0, true);
}

void
run_random(StatId which)
{
	mbstate_t mb0 = {};

	for (unsigned i = 0; i < RAND_ITERS; i++) {
		wchar_t wc;
		locale_t loc;
		int usercap;
		short flags;
		int orientation;
		bool null_locale = false;
		char label[48];

		switch (rnd_u32() % 8) {
		case 0:
			wc = (wchar_t)(rnd_u32() & 0x7f);
			break;
		case 1:
			wc = (wchar_t)(0x80 + (rnd_u32() & 0x7f));
			break;
		case 2:
			wc = (wchar_t)(rnd_u32() & 0xffff);
			break;
		case 3:
			wc = (wchar_t)0xd800 + (rnd_u32() % 0x800);
			break;
		case 4:
			wc = (wchar_t)0xdc00 + (rnd_u32() % 0x400);
			break;
		case 5:
			wc = (wchar_t)(rnd_u32() % 0x110000);
			break;
		case 6:
			wc = L'\0';
			break;
		default:
			wc = (wchar_t)(0x10000 + (rnd_u32() % 0xf000));
			break;
		}

		loc = (rnd_u32() & 1u) ? pbsd_shim_locale_utf8 :
		    pbsd_shim_locale_c;
		usercap = (int)(rnd_u32() % (BUF_USER + 1));
		flags = (rnd_u32() % 16u) == 0 ? (short)0 :
		    (short)port::shim_SWR;
		orientation = (int)(rnd_u32() % 5u) - 2;
		if (which == S_FPUTWC_L && (rnd_u32() % 997u) == 0)
			null_locale = true;

		std::snprintf(label, sizeof(label), "rnd%u", i);
		test_one(which, label, wc, loc, usercap, flags, orientation,
		    &mb0, null_locale);
	}
}

void
report(void)
{
	long long cases = 0, fails = 0;

	std::printf("\n%-12s %12s %12s\n", "function", "cases", "failures");
	std::printf("--------------------------------------------\n");
	for (int i = 0; i < NSTAT; i++) {
		std::printf("%-12s %12lld %12lld\n", g_stat[i].name,
		    g_stat[i].cases, g_stat[i].fails);
		cases += g_stat[i].cases;
		fails += g_stat[i].fails;
	}
	std::printf("--------------------------------------------\n");
	std::printf("%-12s %12lld %12lld\n", "TOTAL", cases, fails);
	std::printf("\n%s\n", fails == 0 ? "PASS" : "FAIL");
	std::fflush(stdout);
}

} /* namespace */

int
main(void)
{
	if (pbsd_shim_init() < 0) {
		std::fprintf(stderr, "pbsd_shim_init failed\n");
		return 2;
	}

	run_edges(S___FPUTWC);
	run_random(S___FPUTWC);
	run_edges(S_FPUTWC_L);
	run_random(S_FPUTWC_L);
	run_edges(S_FPUTWC);
	run_random(S_FPUTWC);

	report();

	long long fails = 0;
	for (int i = 0; i < NSTAT; i++)
		fails += g_stat[i].fails;
	return fails == 0 ? 0 : 1;
}
