/*
 * b0103 differential test: pbsd::lib_libc_stdio::b0103 vs. the ref_ oracle.
 */

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <xlocale.h>

import pbsd.lib.libc.stdio.b0103;

namespace port = pbsd::lib_libc_stdio::b0103;

extern "C" {
int ref_fwscanf(FILE *__restrict, const wchar_t *__restrict, ...);
int ref_fwscanf_l(FILE *__restrict, locale_t, const wchar_t *__restrict, ...);
int ref_fwprintf(FILE *__restrict, const wchar_t *__restrict, ...);
int ref_fwprintf_l(FILE *__restrict, locale_t, const wchar_t *__restrict, ...);
int ref_swscanf(const wchar_t *__restrict, const wchar_t *__restrict, ...);
int ref_swscanf_l(const wchar_t *__restrict, locale_t,
    const wchar_t *__restrict, ...);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr std::size_t BUF_PRE = 16;
constexpr std::size_t BUF_USER = 512;
constexpr std::size_t BUF_POST = 16;
constexpr std::size_t BUF_CAP = BUF_PRE + BUF_USER + BUF_POST;

constexpr int MAXPRINT = 8;
constexpr unsigned RAND_ITERS = 200000u;

enum StatId {
	S_FWSCANF,
	S_FWSCANF_L,
	S_FWPRINTF,
	S_FWPRINTF_L,
	S_SWSCANF,
	S_SWSCANF_L,
	NSTAT
};

struct Stats {
	const char *name;
	long long cases;
	long long fails;
	int printed;
};

Stats g_stat[NSTAT] = {
	{ "fwscanf",    0, 0, 0 },
	{ "fwscanf_l",  0, 0, 0 },
	{ "fwprintf",   0, 0, 0 },
	{ "fwprintf_l", 0, 0, 0 },
	{ "swscanf",    0, 0, 0 },
	{ "swscanf_l",  0, 0, 0 },
};

std::uint64_t rng_state = 0xb0103cafebabeULL;

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

struct GuardedBuf {
	unsigned char data[BUF_CAP];

	void fill_guard(void)
	{
		std::memset(data, GUARD, BUF_CAP);
	}

	bool eq(const GuardedBuf &o) const
	{
		return std::memcmp(data, o.data, BUF_CAP) == 0;
	}
};

struct GuardedWBuf {
	static constexpr std::size_t WPRE = 8;
	static constexpr std::size_t WUSER = 64;
	static constexpr std::size_t WPOST = 8;
	static constexpr std::size_t WCAP = WPRE + WUSER + WPOST;
	unsigned char bytes[WCAP * sizeof(wchar_t)];

	void fill_guard(void)
	{
		std::memset(bytes, GUARD, sizeof(bytes));
	}

	wchar_t *user(void)
	{
		return reinterpret_cast<wchar_t *>(bytes + WPRE * sizeof(wchar_t));
	}

	bool eq_bytes(const GuardedWBuf &o) const
	{
		return std::memcmp(bytes, o.bytes, sizeof(bytes)) == 0;
	}
};

struct GuardedInt {
	unsigned char pre[4];
	int val;
	unsigned char post[4];

	void fill_guard(void)
	{
		std::memset(pre, GUARD, 4);
		val = (int)0x7f7f7f7f;
		std::memset(post, GUARD, 4);
	}

	bool eq(const GuardedInt &o) const
	{
		return std::memcmp(pre, o.pre, 4) == 0 && val == o.val &&
		    std::memcmp(post, o.post, 4) == 0;
	}
};

FILE *
new_tmp_write(void)
{
	FILE *f = std::tmpfile();
	if (f != nullptr)
		fwide(f, 1);
	return f;
}

FILE *
mk_wchar_file(const wchar_t *ws, std::size_t n)
{
	FILE *f = new_tmp_write();
	if (!f)
		return nullptr;
	if (n > 0)
		std::fwrite(ws, sizeof(wchar_t), n, f);
	std::rewind(f);
	return f;
}

void
read_file_guarded(FILE *fp, GuardedBuf &gb)
{
	gb.fill_guard();
	std::rewind(fp);
	(void)std::fread(gb.data + BUF_PRE, 1, BUF_USER, fp);
}

locale_t
test_locale(void)
{
	locale_t loc = newlocale(LC_ALL_MASK, "C", nullptr);
	if (loc == nullptr)
		loc = duplocale(LC_GLOBAL_LOCALE);
	return loc;
}

/* ------------------------------------------------------------------------ */
/* fwscanf / fwscanf_l                                                      */
/* ------------------------------------------------------------------------ */

bool
test_fwscanf_int(StatId which, locale_t loc, const wchar_t *input,
    std::size_t n, const wchar_t *fmt, const char *label)
{
	FILE *rf = mk_wchar_file(input, n);
	FILE *pf = mk_wchar_file(input, n);
	GuardedInt gi_r, gi_p;
	int rr, rp;
	bool ok = true;

	if (!rf || !pf) {
		if (rf)
			std::fclose(rf);
		if (pf)
			std::fclose(pf);
		fail_msg(which, label, "tmpfile failed");
		case_inc(which);
		return false;
	}

	gi_r.fill_guard();
	gi_p.fill_guard();

	if (which == S_FWSCANF) {
		rr = ref_fwscanf(rf, fmt, &gi_r.val);
		rp = port::fwscanf(pf, fmt, &gi_p.val);
	} else {
		rr = ref_fwscanf_l(rf, loc, fmt, &gi_r.val);
		rp = port::fwscanf_l(pf, loc, fmt, &gi_p.val);
	}

	if (rr != rp) {
		fail_msg(which, label, "return mismatch");
		ok = false;
	}
	if (!gi_r.eq(gi_p)) {
		fail_msg(which, label, "int/guard mismatch");
		ok = false;
	}

	std::fclose(rf);
	std::fclose(pf);
	case_inc(which);
	return ok;
}

bool
test_fwscanf_wchar(StatId which, locale_t loc, const wchar_t *input,
    std::size_t n, const wchar_t *fmt, const char *label)
{
	FILE *rf = mk_wchar_file(input, n);
	FILE *pf = mk_wchar_file(input, n);
	GuardedWBuf gw_r, gw_p;
	int rr, rp;
	bool ok = true;

	if (!rf || !pf) {
		if (rf)
			std::fclose(rf);
		if (pf)
			std::fclose(pf);
		fail_msg(which, label, "tmpfile failed");
		case_inc(which);
		return false;
	}

	gw_r.fill_guard();
	gw_p.fill_guard();

	if (which == S_FWSCANF) {
		rr = ref_fwscanf(rf, fmt, gw_r.user());
		rp = port::fwscanf(pf, fmt, gw_p.user());
	} else {
		rr = ref_fwscanf_l(rf, loc, fmt, gw_r.user());
		rp = port::fwscanf_l(pf, loc, fmt, gw_p.user());
	}

	if (rr != rp) {
		fail_msg(which, label, "return mismatch");
		ok = false;
	}
	if (!gw_r.eq_bytes(gw_p)) {
		fail_msg(which, label, "wchar buffer mismatch");
		ok = false;
	}

	std::fclose(rf);
	std::fclose(pf);
	case_inc(which);
	return ok;
}

bool
test_fwscanf_two_int(StatId which, locale_t loc, const wchar_t *input,
    std::size_t n, const wchar_t *fmt, const char *label)
{
	FILE *rf = mk_wchar_file(input, n);
	FILE *pf = mk_wchar_file(input, n);
	GuardedInt gi_r1, gi_p1, gi_r2, gi_p2;
	int rr, rp;
	bool ok = true;

	if (!rf || !pf) {
		if (rf)
			std::fclose(rf);
		if (pf)
			std::fclose(pf);
		fail_msg(which, label, "tmpfile failed");
		case_inc(which);
		return false;
	}

	gi_r1.fill_guard();
	gi_p1.fill_guard();
	gi_r2.fill_guard();
	gi_p2.fill_guard();

	if (which == S_FWSCANF) {
		rr = ref_fwscanf(rf, fmt, &gi_r1.val, &gi_r2.val);
		rp = port::fwscanf(pf, fmt, &gi_p1.val, &gi_p2.val);
	} else {
		rr = ref_fwscanf_l(rf, loc, fmt, &gi_r1.val, &gi_r2.val);
		rp = port::fwscanf_l(pf, loc, fmt, &gi_p1.val, &gi_p2.val);
	}

	if (rr != rp) {
		fail_msg(which, label, "return mismatch");
		ok = false;
	}
	if (!gi_r1.eq(gi_p1) || !gi_r2.eq(gi_p2)) {
		fail_msg(which, label, "int pair mismatch");
		ok = false;
	}

	std::fclose(rf);
	std::fclose(pf);
	case_inc(which);
	return ok;
}

void
run_fwscanf_edges(StatId which, locale_t loc)
{
	static const wchar_t w42[] = { '4', '2', '\n' };
	static const wchar_t w12[] = { '1', ' ', '2', '\n' };
	static const wchar_t wempty[] = { '\n' };
	static const wchar_t wx[] = { 'x', '\n' };
	static const wchar_t whi[] = { (wchar_t)0x80, (wchar_t)0xfffe, '\n' };
	static const wchar_t wnone[] = { 'z', '\n' };
	static const wchar_t w0[] = { '0', '\n' };
	static const wchar_t wnul[] = { '\0', '7', '\n' };

	test_fwscanf_int(which, loc, w42, 3, L"%d", "d 42");
	test_fwscanf_two_int(which, loc, w12, 4, L"%d %d", "d d 1 2");
	test_fwscanf_int(which, loc, wempty, 1, L"%d", "d empty");
	test_fwscanf_wchar(which, loc, wx, 2, L"%lc", "lc x");
	test_fwscanf_wchar(which, loc, whi, 3, L"%lc", "lc hi");
	test_fwscanf_int(which, loc, wnone, 2, L"%d", "d nomatch");
	test_fwscanf_int(which, loc, w0, 2, L"%d", "d zero");
	test_fwscanf_int(which, loc, w42, 0, L"%d", "d eof");
	test_fwscanf_wchar(which, loc, wnul, 3, L"%lc", "lc nul");
}

void
run_fwscanf_random(StatId which, locale_t loc)
{
	wchar_t input[32];
	wchar_t fmt[8];

	for (unsigned i = 0; i < RAND_ITERS; i++) {
		unsigned kind = rnd_u32() % 5;
		std::size_t n = (rnd_u32() % 16u) + 1u;
		for (std::size_t j = 0; j < n; j++) {
			unsigned v = rnd_u32();
			if ((v & 7u) == 0)
				input[j] = (wchar_t)(0x80 + (v & 0x7fu));
			else if ((v & 7u) == 1)
				input[j] = (wchar_t)'\n';
			else if ((v & 7u) == 2)
				input[j] = (wchar_t)'\0';
			else
				input[j] = (wchar_t)('0' + (v % 10u));
		}
		input[n - 1] = '\n';

		char label[48];
		std::snprintf(label, sizeof(label), "rnd%u", i);

		if (kind == 0) {
			std::wcscpy(fmt, L"%d");
			test_fwscanf_int(which, loc, input, n, fmt, label);
		} else if (kind == 1) {
			std::wcscpy(fmt, L"%lc");
			test_fwscanf_wchar(which, loc, input, n, fmt, label);
		} else if (kind == 2) {
			std::wcscpy(fmt, L"%d %d");
			test_fwscanf_two_int(which, loc, input, n, fmt, label);
		} else if (kind == 3) {
			std::wcscpy(fmt, L"%1d");
			test_fwscanf_int(which, loc, input, n, fmt, label);
		} else {
			std::wcscpy(fmt, L"%2d");
			test_fwscanf_int(which, loc, input, n, fmt, label);
		}
	}
}

/* ------------------------------------------------------------------------ */
/* fwprintf / fwprintf_l                                                    */
/* ------------------------------------------------------------------------ */

bool
test_fwprintf_case(StatId which, locale_t loc, const wchar_t *fmt,
    int arg1, int arg2, bool two_args, const char *label)
{
	FILE *rf = new_tmp_write();
	FILE *pf = new_tmp_write();
	GuardedBuf gb_r, gb_p;
	int rr, rp;
	bool ok = true;

	if (!rf || !pf) {
		if (rf)
			std::fclose(rf);
		if (pf)
			std::fclose(pf);
		fail_msg(which, label, "tmpfile failed");
		case_inc(which);
		return false;
	}

	gb_r.fill_guard();
	gb_p.fill_guard();

	if (which == S_FWPRINTF) {
		if (two_args)
			rr = ref_fwprintf(rf, fmt, arg1, arg2);
		else
			rr = ref_fwprintf(rf, fmt, arg1);
	} else {
		if (two_args)
			rr = ref_fwprintf_l(rf, loc, fmt, arg1, arg2);
		else
			rr = ref_fwprintf_l(rf, loc, fmt, arg1);
	}

	if (which == S_FWPRINTF) {
		if (two_args)
			rp = port::fwprintf(pf, fmt, arg1, arg2);
		else
			rp = port::fwprintf(pf, fmt, arg1);
	} else {
		if (two_args)
			rp = port::fwprintf_l(pf, loc, fmt, arg1, arg2);
		else
			rp = port::fwprintf_l(pf, loc, fmt, arg1);
	}

	std::fflush(rf);
	std::fflush(pf);
	read_file_guarded(rf, gb_r);
	read_file_guarded(pf, gb_p);

	if (rr != rp) {
		fail_msg(which, label, "return mismatch");
		ok = false;
	}
	if (!gb_r.eq(gb_p)) {
		fail_msg(which, label, "file bytes mismatch");
		ok = false;
	}

	std::fclose(rf);
	std::fclose(pf);
	case_inc(which);
	return ok;
}

bool
test_fwprintf_wchar(StatId which, locale_t loc, const wchar_t *fmt,
    wchar_t wc, const char *label)
{
	FILE *rf = new_tmp_write();
	FILE *pf = new_tmp_write();
	GuardedBuf gb_r, gb_p;
	int rr, rp;
	bool ok = true;

	if (!rf || !pf) {
		if (rf)
			std::fclose(rf);
		if (pf)
			std::fclose(pf);
		fail_msg(which, label, "tmpfile failed");
		case_inc(which);
		return false;
	}

	gb_r.fill_guard();
	gb_p.fill_guard();

	if (which == S_FWPRINTF)
		rr = ref_fwprintf(rf, fmt, wc);
	else
		rr = ref_fwprintf_l(rf, loc, fmt, wc);

	if (which == S_FWPRINTF)
		rp = port::fwprintf(pf, fmt, wc);
	else
		rp = port::fwprintf_l(pf, loc, fmt, wc);

	std::fflush(rf);
	std::fflush(pf);
	read_file_guarded(rf, gb_r);
	read_file_guarded(pf, gb_p);

	if (rr != rp) {
		fail_msg(which, label, "return mismatch");
		ok = false;
	}
	if (!gb_r.eq(gb_p)) {
		fail_msg(which, label, "wchar file mismatch");
		ok = false;
	}

	std::fclose(rf);
	std::fclose(pf);
	case_inc(which);
	return ok;
}

void
run_fwprintf_edges(StatId which, locale_t loc)
{
	test_fwprintf_case(which, loc, L"", 0, 0, false, "empty fmt");
	test_fwprintf_case(which, loc, L"%d", 0, 0, false, "d zero");
	test_fwprintf_case(which, loc, L"%d", 42, 0, false, "d 42");
	test_fwprintf_case(which, loc, L"%d", -1, 0, false, "d -1");
	test_fwprintf_case(which, loc, L"%d %d", 1, 2, true, "d d 1 2");
	test_fwprintf_case(which, loc, L"%d", INT_MAX, 0, false, "d INT_MAX");
	test_fwprintf_case(which, loc, L"%d", INT_MIN, 0, false, "d INT_MIN");
	test_fwprintf_wchar(which, loc, L"%lc", L'x', "lc x");
	test_fwprintf_wchar(which, loc, L"%lc", (wchar_t)0x80, "lc 0x80");
	test_fwprintf_wchar(which, loc, L"%lc", (wchar_t)0xff, "lc 0xff");
	test_fwprintf_case(which, loc, L"%%", 0, 0, false, "pct");
}

void
run_fwprintf_random(StatId which, locale_t loc)
{
	wchar_t fmt[16];

	for (unsigned i = 0; i < RAND_ITERS; i++) {
		unsigned kind = rnd_u32() % 6;
		int a1 = (int)(rnd_u32() & 0xffffu) - 0x7fff;
		int a2 = (int)(rnd_u32() & 0xffffu) - 0x7fff;
		char label[48];

		std::snprintf(label, sizeof(label), "rnd%u", i);

		if (kind == 0) {
			std::wcscpy(fmt, L"%d");
			test_fwprintf_case(which, loc, fmt, a1, 0, false, label);
		} else if (kind == 1) {
			std::wcscpy(fmt, L"%d %d");
			test_fwprintf_case(which, loc, fmt, a1, a2, true, label);
		} else if (kind == 2) {
			std::wcscpy(fmt, L"%1d");
			test_fwprintf_case(which, loc, fmt, a1, 0, false, label);
		} else if (kind == 3) {
			std::wcscpy(fmt, L"%5d");
			test_fwprintf_case(which, loc, fmt, a1, 0, false, label);
		} else if (kind == 4) {
			wchar_t wc = (wchar_t)(0x80 + (rnd_u32() & 0x7fu));
			std::wcscpy(fmt, L"%lc");
			test_fwprintf_wchar(which, loc, fmt, wc, label);
		} else {
			std::wcscpy(fmt, L"%%");
			test_fwprintf_case(which, loc, fmt, 0, 0, false, label);
		}
	}
}

/* ------------------------------------------------------------------------ */
/* swscanf / swscanf_l                                                      */
/* ------------------------------------------------------------------------ */

bool
test_swscanf_int(StatId which, locale_t loc, const wchar_t *input,
    const wchar_t *fmt, const char *label)
{
	GuardedInt gi_r, gi_p;
	int rr, rp;
	bool ok = true;

	gi_r.fill_guard();
	gi_p.fill_guard();

	if (which == S_SWSCANF) {
		rr = ref_swscanf(input, fmt, &gi_r.val);
		rp = port::swscanf(input, fmt, &gi_p.val);
	} else {
		rr = ref_swscanf_l(input, loc, fmt, &gi_r.val);
		rp = port::swscanf_l(input, loc, fmt, &gi_p.val);
	}

	if (rr != rp) {
		fail_msg(which, label, "return mismatch");
		ok = false;
	}
	if (!gi_r.eq(gi_p)) {
		fail_msg(which, label, "int/guard mismatch");
		ok = false;
	}

	case_inc(which);
	return ok;
}

bool
test_swscanf_wchar(StatId which, locale_t loc, const wchar_t *input,
    const wchar_t *fmt, const char *label)
{
	GuardedWBuf gw_r, gw_p;
	int rr, rp;
	bool ok = true;

	gw_r.fill_guard();
	gw_p.fill_guard();

	if (which == S_SWSCANF) {
		rr = ref_swscanf(input, fmt, gw_r.user());
		rp = port::swscanf(input, fmt, gw_p.user());
	} else {
		rr = ref_swscanf_l(input, loc, fmt, gw_r.user());
		rp = port::swscanf_l(input, loc, fmt, gw_p.user());
	}

	if (rr != rp) {
		fail_msg(which, label, "return mismatch");
		ok = false;
	}
	if (!gw_r.eq_bytes(gw_p)) {
		fail_msg(which, label, "wchar buffer mismatch");
		ok = false;
	}

	case_inc(which);
	return ok;
}

bool
test_swscanf_two_int(StatId which, locale_t loc, const wchar_t *input,
    const wchar_t *fmt, const char *label)
{
	GuardedInt gi_r1, gi_p1, gi_r2, gi_p2;
	int rr, rp;
	bool ok = true;

	gi_r1.fill_guard();
	gi_p1.fill_guard();
	gi_r2.fill_guard();
	gi_p2.fill_guard();

	if (which == S_SWSCANF) {
		rr = ref_swscanf(input, fmt, &gi_r1.val, &gi_r2.val);
		rp = port::swscanf(input, fmt, &gi_p1.val, &gi_p2.val);
	} else {
		rr = ref_swscanf_l(input, loc, fmt, &gi_r1.val, &gi_r2.val);
		rp = port::swscanf_l(input, loc, fmt, &gi_p1.val, &gi_p2.val);
	}

	if (rr != rp) {
		fail_msg(which, label, "return mismatch");
		ok = false;
	}
	if (!gi_r1.eq(gi_p1) || !gi_r2.eq(gi_p2)) {
		fail_msg(which, label, "int pair mismatch");
		ok = false;
	}

	case_inc(which);
	return ok;
}

void
run_swscanf_edges(StatId which, locale_t loc)
{
	static const wchar_t w42[] = L"42";
	static const wchar_t w12[] = L"1 2";
	static const wchar_t wempty[] = L"";
	static const wchar_t wx[] = L"x";
	static const wchar_t whi[] = { (wchar_t)0x80, (wchar_t)0xfffe, L'\0' };
	static const wchar_t wnone[] = L"z";
	static const wchar_t w0[] = L"0";
	static const wchar_t wnul[] = { L'\0', L'7', L'\0' };

	test_swscanf_int(which, loc, w42, L"%d", "d 42");
	test_swscanf_two_int(which, loc, w12, L"%d %d", "d d 1 2");
	test_swscanf_int(which, loc, wempty, L"%d", "d empty");
	test_swscanf_wchar(which, loc, wx, L"%lc", "lc x");
	test_swscanf_wchar(which, loc, whi, L"%lc", "lc hi");
	test_swscanf_int(which, loc, wnone, L"%d", "d nomatch");
	test_swscanf_int(which, loc, w0, L"%d", "d zero");
	test_swscanf_wchar(which, loc, wnul, L"%lc", "lc nul");
}

void
run_swscanf_random(StatId which, locale_t loc)
{
	wchar_t input[32];
	wchar_t fmt[8];

	for (unsigned i = 0; i < RAND_ITERS; i++) {
		unsigned kind = rnd_u32() % 5;
		std::size_t n = rnd_u32() % 16u;
		for (std::size_t j = 0; j < n; j++) {
			unsigned v = rnd_u32();
			if ((v & 7u) == 0)
				input[j] = (wchar_t)(0x80 + (v & 0x7fu));
			else if ((v & 7u) == 1)
				input[j] = (wchar_t)'\0';
			else
				input[j] = (wchar_t)('0' + (v % 10u));
		}
		input[n] = L'\0';

		char label[48];
		std::snprintf(label, sizeof(label), "rnd%u", i);

		if (kind == 0) {
			std::wcscpy(fmt, L"%d");
			test_swscanf_int(which, loc, input, fmt, label);
		} else if (kind == 1) {
			std::wcscpy(fmt, L"%lc");
			test_swscanf_wchar(which, loc, input, fmt, label);
		} else if (kind == 2) {
			std::wcscpy(fmt, L"%d %d");
			test_swscanf_two_int(which, loc, input, fmt, label);
		} else if (kind == 3) {
			std::wcscpy(fmt, L"%1d");
			test_swscanf_int(which, loc, input, fmt, label);
		} else {
			std::wcscpy(fmt, L"%2d");
			test_swscanf_int(which, loc, input, fmt, label);
		}
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
	locale_t loc = test_locale();

	run_fwscanf_edges(S_FWSCANF, loc);
	run_fwscanf_random(S_FWSCANF, loc);
	run_fwscanf_edges(S_FWSCANF_L, loc);
	run_fwscanf_random(S_FWSCANF_L, loc);

	run_fwprintf_edges(S_FWPRINTF, loc);
	run_fwprintf_random(S_FWPRINTF, loc);
	run_fwprintf_edges(S_FWPRINTF_L, loc);
	run_fwprintf_random(S_FWPRINTF_L, loc);

	run_swscanf_edges(S_SWSCANF, loc);
	run_swscanf_random(S_SWSCANF, loc);
	run_swscanf_edges(S_SWSCANF_L, loc);
	run_swscanf_random(S_SWSCANF_L, loc);

	if (loc != nullptr && loc != LC_GLOBAL_LOCALE)
		freelocale(loc);

	report();

	long long fails = 0;
	for (int i = 0; i < NSTAT; i++)
		fails += g_stat[i].fails;
	return fails == 0 ? 0 : 1;
}
