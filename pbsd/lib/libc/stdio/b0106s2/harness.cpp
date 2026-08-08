/*
 * harness.cpp -- differential test for PBSD batch b0106s2 (fputs.c).
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.stdio.b0106s2;

namespace port = pbsd::lib_libc_stdio::b0106s2;

extern "C" {
struct pbsd_shim_bufdesc {
	unsigned char *_base;
	int _size;
};

struct pbsd_shim_file {
	unsigned char *_p;
	int _r;
	int _w;
	short _flags;
	short _file;
	struct pbsd_shim_bufdesc _bf;
	int _lbfsize;
	int _orientation;
	int _lockdepth;
	int _locktotal;
};

int ref_fputs_unlocked(const char *__restrict, struct pbsd_shim_file *__restrict);
int ref_fputs(const char *__restrict, struct pbsd_shim_file *__restrict);

extern int pbsd_shim_orient_calls;
extern int pbsd_shim_orient_last_dir;
extern int pbsd_shim_sfvwrite_calls;
extern int pbsd_shim_sfvwrite_entry_iovcnt;
extern int pbsd_shim_sfvwrite_entry_resid;
extern size_t pbsd_shim_sfvwrite_entry_iovlen;
extern void *pbsd_shim_sfvwrite_entry_iovbase;
extern int pbsd_shim_flockfile_calls;
extern int pbsd_shim_funlockfile_calls;

size_t pbsd_shim_file_layout(int);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr std::size_t BUF_PRE = 16;
constexpr std::size_t BUF_USER = 512;
constexpr std::size_t BUF_POST = 16;
constexpr std::size_t BUF_CAP = BUF_PRE + BUF_USER + BUF_POST;
constexpr std::size_t STR_PRE = 16;
constexpr std::size_t STR_USER = 512;
constexpr std::size_t STR_POST = 16;
constexpr std::size_t STR_CAP = STR_PRE + STR_USER + STR_POST;
constexpr unsigned RAND_ITERS = 200000u;
constexpr int MAXPRINT = 8;

constexpr short shim_SWR = 0x0008;
constexpr short shim_SERR = 0x0040;

enum StatId {
	S_FPUTS_UNLOCKED,
	S_FPUTS,
	NSTAT
};

struct Stats {
	const char *name;
	long long cases;
	long long fails;
	int printed;
};

Stats g_stat[NSTAT] = {
	{ "fputs_unlocked", 0, 0, 0 },
	{ "fputs",          0, 0, 0 },
};

std::uint64_t rng_state = 0xb01062facefeedULL;

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
		std::printf("  FAIL %-14s %-24s %s\n", g_stat[which].name, label,
		    detail);
}

void
case_inc(StatId which)
{
	g_stat[which].cases++;
}

struct GuardedStr {
	unsigned char data[STR_CAP];

	void fill_guard(void)
	{
		std::memset(data, GUARD, STR_CAP);
	}

	char *user(void)
	{
		return reinterpret_cast<char *>(data + STR_PRE);
	}

	void set_bytes(const unsigned char *src, std::size_t n)
	{
		fill_guard();
		if (n > STR_USER)
			n = STR_USER;
		std::memcpy(user(), src, n);
		user()[n] = '\0';
	}
};

struct StreamPair {
	unsigned char backing_r[BUF_CAP];
	unsigned char backing_p[BUF_CAP];
	struct pbsd_shim_file rf;
	struct pbsd_shim_file pf;
	int orient_calls_r;
	int orient_calls_p;
	int sfvwrite_calls_r;
	int sfvwrite_calls_p;
	int flock_calls_r;
	int flock_calls_p;
	int funlock_calls_r;
	int funlock_calls_p;
};

void
reset_shim_counters(void)
{
	pbsd_shim_orient_calls = 0;
	pbsd_shim_sfvwrite_calls = 0;
	pbsd_shim_flockfile_calls = 0;
	pbsd_shim_funlockfile_calls = 0;
}

void
init_stream(struct pbsd_shim_file *fp, unsigned char *backing,
    std::size_t p_offset, int w_room, short flags, int orientation)
{
	std::memset(backing, GUARD, BUF_CAP);
	std::memset(fp, 0, sizeof(*fp));
	fp->_bf._base = backing + BUF_PRE;
	fp->_bf._size = (int)BUF_USER;
	fp->_p = fp->_bf._base + p_offset;
	fp->_w = w_room;
	fp->_flags = flags;
	fp->_orientation = orientation;
	fp->_lockdepth = 0;
	fp->_locktotal = 0;
}

bool
layout_ok(void)
{
	struct pbsd_shim_file probe;

	if (pbsd_shim_file_layout(0) != sizeof(probe))
		return false;
	if (pbsd_shim_file_layout(1) != offsetof(struct pbsd_shim_file, _p))
		return false;
	if (pbsd_shim_file_layout(2) != offsetof(struct pbsd_shim_file, _w))
		return false;
	if (pbsd_shim_file_layout(3) != offsetof(struct pbsd_shim_file, _flags))
		return false;
	return true;
}

bool
fp_state_eq(const struct pbsd_shim_file *a, const struct pbsd_shim_file *b,
    const unsigned char *base_a, const unsigned char *base_b)
{
	if (a->_p - base_a != b->_p - base_b)
		return false;
	if (a->_w != b->_w)
		return false;
	if (a->_flags != b->_flags)
		return false;
	if (a->_orientation != b->_orientation)
		return false;
	if (a->_lockdepth != b->_lockdepth)
		return false;
	if (a->_locktotal != b->_locktotal)
		return false;
	return true;
}

bool
test_fputs_one(StatId which, const char *label, const char *s,
    std::size_t p_offset, int w_room, short flags, int orientation)
{
	StreamPair sp;
	int rr, rp;
	bool ok = true;

	reset_shim_counters();
	init_stream(&sp.rf, sp.backing_r, p_offset, w_room, flags, orientation);
	init_stream(&sp.pf, sp.backing_p, p_offset, w_room, flags, orientation);

	if (which == S_FPUTS_UNLOCKED) {
		rr = ref_fputs_unlocked(s, &sp.rf);
		sp.orient_calls_r = pbsd_shim_orient_calls;
		sp.sfvwrite_calls_r = pbsd_shim_sfvwrite_calls;
		sp.flock_calls_r = pbsd_shim_flockfile_calls;
		sp.funlock_calls_r = pbsd_shim_funlockfile_calls;

		reset_shim_counters();
		rp = port::fputs_unlocked(s,
		    reinterpret_cast<port::shim_file *>(&sp.pf));
		sp.orient_calls_p = pbsd_shim_orient_calls;
		sp.sfvwrite_calls_p = pbsd_shim_sfvwrite_calls;
		sp.flock_calls_p = pbsd_shim_flockfile_calls;
		sp.funlock_calls_p = pbsd_shim_funlockfile_calls;
	} else {
		rr = ref_fputs(s, &sp.rf);
		sp.orient_calls_r = pbsd_shim_orient_calls;
		sp.sfvwrite_calls_r = pbsd_shim_sfvwrite_calls;
		sp.flock_calls_r = pbsd_shim_flockfile_calls;
		sp.funlock_calls_r = pbsd_shim_funlockfile_calls;

		reset_shim_counters();
		rp = port::fputs(s, reinterpret_cast<port::shim_file *>(&sp.pf));
		sp.orient_calls_p = pbsd_shim_orient_calls;
		sp.sfvwrite_calls_p = pbsd_shim_sfvwrite_calls;
		sp.flock_calls_p = pbsd_shim_flockfile_calls;
		sp.funlock_calls_p = pbsd_shim_funlockfile_calls;
	}

	if (rr != rp) {
		fail_msg(which, label, "return mismatch");
		ok = false;
	}

	if (std::memcmp(sp.backing_r, sp.backing_p, BUF_CAP) != 0) {
		fail_msg(which, label, "backing buffer mismatch");
		ok = false;
	}

	if (!fp_state_eq(&sp.rf, &sp.pf, sp.backing_r + BUF_PRE,
	    sp.backing_p + BUF_PRE)) {
		fail_msg(which, label, "stream state mismatch");
		ok = false;
	}

	if (sp.orient_calls_r != sp.orient_calls_p) {
		fail_msg(which, label, "ORIENT call count mismatch");
		ok = false;
	}
	if (sp.sfvwrite_calls_r != sp.sfvwrite_calls_p) {
		fail_msg(which, label, "__sfvwrite call count mismatch");
		ok = false;
	}
	if (sp.flock_calls_r != sp.flock_calls_p) {
		fail_msg(which, label, "flockfile call count mismatch");
		ok = false;
	}
	if (sp.funlock_calls_r != sp.funlock_calls_p) {
		fail_msg(which, label, "funlockfile call count mismatch");
		ok = false;
	}

	if (which == S_FPUTS) {
		if (sp.flock_calls_r != 1 || sp.funlock_calls_r != 1) {
			fail_msg(which, label, "expected lock/unlock once");
			ok = false;
		}
		if (sp.rf._lockdepth != 0 || sp.pf._lockdepth != 0) {
			fail_msg(which, label, "lockdepth not zero");
			ok = false;
		}
	} else {
		if (sp.flock_calls_r != 0 || sp.funlock_calls_r != 0) {
			fail_msg(which, label, "unlocked should not lock");
			ok = false;
		}
	}

	if (pbsd_shim_orient_last_dir != -1) {
		fail_msg(which, label, "ORIENT direction mismatch");
		ok = false;
	}

	case_inc(which);
	return ok;
}

void
run_fputs_edges(StatId which)
{
	static const char empty[] = "";
	static const char one[] = "a";
	static const char hi[] = "\x80\xff\x7f\xfe";
	static const char mix[] = "abc\000def";
	static const unsigned char nulheavy[] = {
		0x00, 0x00, 'x', 0x00, 0xff, 0x80, 0x00
	};
	char longbuf[300];
	GuardedStr gs;

	std::memset(longbuf, 'Z', sizeof(longbuf) - 1);
	longbuf[sizeof(longbuf) - 1] = '\0';

	test_fputs_one(which, "empty", empty, 0, (int)BUF_USER, shim_SWR, 0);
	test_fputs_one(which, "one", one, 0, (int)BUF_USER, shim_SWR, 0);
	test_fputs_one(which, "highbit", hi, 0, (int)BUF_USER, shim_SWR, 0);
	test_fputs_one(which, "nul in middle", mix, 0, (int)BUF_USER, shim_SWR, 0);
	test_fputs_one(which, "nul heavy", reinterpret_cast<const char *>(nulheavy),
	    0, (int)BUF_USER, shim_SWR, 0);
	test_fputs_one(which, "long", longbuf, 0, (int)BUF_USER, shim_SWR, 0);
	test_fputs_one(which, "exact fit", "12345", 0, 5, shim_SWR, 0);
	test_fputs_one(which, "one short", "123456", 0, 5, shim_SWR, 0);
	test_fputs_one(which, "zero room", one, 0, 0, shim_SWR, 0);
	test_fputs_one(which, "readonly", one, 0, (int)BUF_USER, 0, 0);
	test_fputs_one(which, "readonly zero", empty, 0, 0, 0, 0);
	test_fputs_one(which, "mid buffer p", "ab", 10, (int)BUF_USER - 10,
	    shim_SWR, 0);
	test_fputs_one(which, "orient preset", one, 0, (int)BUF_USER, shim_SWR, 1);

	gs.set_bytes(nulheavy, sizeof(nulheavy));
	test_fputs_one(which, "guarded nulheavy", gs.user(), 0, (int)BUF_USER,
	    shim_SWR, 0);

	{
		static const unsigned char single_hi[] = { (unsigned char)0xfe };
		gs.set_bytes(single_hi, 1);
		test_fputs_one(which, "single 0xfe", gs.user(), 0,
		    (int)BUF_USER, shim_SWR, 0);
	}

	test_fputs_one(which, "err already", one, 0, (int)BUF_USER,
	    (short)(shim_SWR | shim_SERR), 0);
}

void
run_fputs_random(StatId which)
{
	GuardedStr gs;

	for (unsigned i = 0; i < RAND_ITERS; i++) {
		std::size_t n = rnd_u32() % (STR_USER - 1);
		for (std::size_t j = 0; j < n; j++) {
			unsigned v = rnd_u32();
			if ((v & 7u) == 0)
				gs.user()[j] = (char)(0x80 + (v & 0x7fu));
			else if ((v & 7u) == 1)
				gs.user()[j] = '\0';
			else
				gs.user()[j] = (char)('a' + (v % 26u));
		}
		gs.user()[n] = '\0';
		gs.data[STR_PRE - 1] = GUARD;
		gs.data[STR_PRE + STR_USER] = GUARD;

		std::size_t slen = std::strlen(gs.user());
		std::size_t poff = rnd_u32() % 32u;
		int w_room;
		short flags;
		int orient;

		switch (rnd_u32() % 8u) {
		case 0:
			w_room = 0;
			flags = shim_SWR;
			orient = 0;
			break;
		case 1:
			w_room = (int)BUF_USER;
			flags = 0;
			orient = 0;
			break;
		case 2:
			w_room = (slen > 0) ? (int)slen - 1 : 0;
			flags = shim_SWR;
			orient = 0;
			break;
		case 3:
			w_room = (int)slen;
			flags = shim_SWR;
			orient = 0;
			break;
		case 4:
			w_room = (int)slen + 1;
			flags = shim_SWR;
			orient = (int)(rnd_u32() % 3u) - 1;
			break;
		case 5:
			w_room = (int)(rnd_u32() % (BUF_USER + 1u));
			flags = (short)(shim_SWR | shim_SERR);
			orient = 0;
			break;
		case 6:
			w_room = 1;
			flags = shim_SWR;
			orient = 0;
			break;
		default:
			w_room = (int)(rnd_u32() % (BUF_USER + 1u));
			flags = shim_SWR;
			orient = (int)(rnd_u32() % 5u) - 2;
			break;
		}

		char label[48];
		std::snprintf(label, sizeof(label), "rnd%u", i);
		test_fputs_one(which, label, gs.user(), poff, w_room, flags,
		    orient);
	}
}

void
report(void)
{
	long long cases = 0, fails = 0;

	std::printf("\n%-14s %12s %12s\n", "function", "cases", "failures");
	std::printf("----------------------------------------------\n");
	for (int i = 0; i < NSTAT; i++) {
		std::printf("%-14s %12lld %12lld\n", g_stat[i].name,
		    g_stat[i].cases, g_stat[i].fails);
		cases += g_stat[i].cases;
		fails += g_stat[i].fails;
	}
	std::printf("----------------------------------------------\n");
	std::printf("%-14s %12lld %12lld\n", "TOTAL", cases, fails);
	std::printf("\n%s\n", fails == 0 ? "PASS" : "FAIL");
	std::fflush(stdout);
}

} /* namespace */

int
main(void)
{
	if (!layout_ok()) {
		std::fprintf(stderr, "layout mismatch between port and oracle\n");
		return 1;
	}

	run_fputs_edges(S_FPUTS_UNLOCKED);
	run_fputs_random(S_FPUTS_UNLOCKED);
	run_fputs_edges(S_FPUTS);
	run_fputs_random(S_FPUTS);

	report();

	long long fails = 0;
	for (int i = 0; i < NSTAT; i++)
		fails += g_stat[i].fails;
	return fails == 0 ? 0 : 1;
}
