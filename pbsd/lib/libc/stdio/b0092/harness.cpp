/*
 * b0092 differential test: pbsd::lib_libc_stdio::b0092 vs. the ref_ oracle.
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
#include <unistd.h>

import pbsd.lib.libc.stdio.b0092;

namespace port = pbsd::lib_libc_stdio::b0092;

extern "C" {
void ref_setbuffer(FILE *, char *, int);
int ref_setlinebuf(FILE *);
int ref_wscanf(const wchar_t *__restrict, ...);
int ref_wscanf_l(locale_t, const wchar_t *__restrict, ...);
wint_t ref_getwc(FILE *);
wint_t ref_getwc_l(FILE *, locale_t);
int wscanf(const wchar_t *__restrict, ...);
int wscanf_l(locale_t, const wchar_t *__restrict, ...);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr std::size_t BUF_PRE = 16;
constexpr std::size_t BUF_USER = 256;
constexpr std::size_t BUF_POST = 16;
constexpr std::size_t BUF_CAP = BUF_PRE + BUF_USER + BUF_POST;

constexpr int MAXPRINT = 8;
constexpr unsigned RAND_ITERS = 200000u;

enum StatId {
	S_SETBUFFER,
	S_SETLINEBUF,
	S_WSCANF,
	S_WSCANF_L,
	S_GETWC,
	S_GETWC_L,
	NSTAT
};

struct Stats {
	const char *name;
	long long cases;
	long long fails;
	int printed;
};

Stats g_stat[NSTAT] = {
	{ "setbuffer",  0, 0, 0 },
	{ "setlinebuf", 0, 0, 0 },
	{ "wscanf",     0, 0, 0 },
	{ "wscanf_l",   0, 0, 0 },
	{ "getwc",      0, 0, 0 },
	{ "getwc_l",    0, 0, 0 },
};

std::uint64_t rng_state = 0xc0ffee0092ULL;

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

	char *user(void)
	{
		return reinterpret_cast<char *>(data + BUF_PRE);
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
	return std::tmpfile();
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
	fwide(f, 1);
	return f;
}

bool
probe_write_read(FILE *rf, FILE *pf, StatId which, const char *label)
{
	bool ok = true;
	static const char probe[] = {
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
		(char)0x00, (char)0x7f, (char)0x80, (char)0xff,
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
		'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
		'U', 'V', 'W', 'X', 'Y', 'Z'
	};

	for (std::size_t i = 0; i < sizeof(probe); i++) {
		int cr = std::fputc(probe[i], rf);
		int cp = std::fputc(probe[i], pf);
		if (cr != cp) {
			fail_msg(which, label, "fputc mismatch");
			ok = false;
			break;
		}
	}
	if (std::fflush(rf) != std::fflush(pf)) {
		fail_msg(which, label, "fflush mismatch");
		ok = false;
	}
	std::rewind(rf);
	std::rewind(pf);
	for (std::size_t i = 0; i < sizeof(probe); i++) {
		int cr = std::fgetc(rf);
		int cp = std::fgetc(pf);
		if (cr != cp) {
			fail_msg(which, label, "fgetc mismatch");
			ok = false;
			break;
		}
	}
	return ok;
}

bool
test_setbuffer_one(const char *label, int size, bool use_buf)
{
	GuardedBuf gb_r, gb_p;
	gb_r.fill_guard();
	gb_p.fill_guard();

	FILE *rf = new_tmp_write();
	FILE *pf = new_tmp_write();
	if (!rf || !pf) {
		if (rf)
			std::fclose(rf);
		if (pf)
			std::fclose(pf);
		fail_msg(S_SETBUFFER, label, "tmpfile failed");
		case_inc(S_SETBUFFER);
		return false;
	}

	char *rarg = use_buf ? gb_r.user() : nullptr;
	char *parg = use_buf ? gb_p.user() : nullptr;

	ref_setbuffer(rf, rarg, size);
	port::setbuffer(pf, parg, size);

	bool ok = true;
	if (use_buf && std::memcmp(gb_r.data, gb_p.data, BUF_CAP) != 0) {
		fail_msg(S_SETBUFFER, label, "buffer bytes mismatch");
		ok = false;
	}
	if (!probe_write_read(rf, pf, S_SETBUFFER, label))
		ok = false;

	std::fclose(rf);
	std::fclose(pf);
	case_inc(S_SETBUFFER);
	return ok;
}

void
run_setbuffer_edges(void)
{
	static const int sizes[] = {
		0, 1, 2, 63, 64, 127, 128, 255, 256, -1, INT_MIN, INT_MAX
	};

	for (int buf_mode = 0; buf_mode < 2; buf_mode++) {
		bool use_buf = buf_mode != 0;
		for (int sz : sizes) {
			char label[64];
			std::snprintf(label, sizeof(label), "edge buf=%d sz=%d",
			    (int)use_buf, sz);
			test_setbuffer_one(label, sz, use_buf);
		}
	}

	GuardedBuf gb_r, gb_p;
	gb_r.fill_guard();
	gb_p.fill_guard();
	FILE *rf = new_tmp_write();
	FILE *pf = new_tmp_write();
	ref_setbuffer(rf, gb_r.user(), 64);
	port::setbuffer(pf, gb_p.user(), 64);
	for (int i = 0; i < 192; i++) {
		unsigned char c = (unsigned char)(i & 0xff);
		std::fputc((int)c, rf);
		std::fputc((int)c, pf);
	}
	std::fflush(rf);
	std::fflush(pf);
	bool ok = std::memcmp(gb_r.data, gb_p.data, BUF_CAP) == 0;
	std::fclose(rf);
	std::fclose(pf);
	case_inc(S_SETBUFFER);
	if (!ok)
		fail_msg(S_SETBUFFER, "edge highbit sweep", "buffer mismatch");
}

void
run_setbuffer_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		bool use_buf = (rnd_u32() & 1) != 0;
		int sz;
		switch (rnd_u32() % 8) {
		case 0:
			sz = 0;
			break;
		case 1:
			sz = 1;
			break;
		case 2:
			sz = (int)(rnd_u32() % 256u);
			break;
		case 3:
			sz = 256;
			break;
		case 4:
			sz = -1;
			break;
		case 5:
			sz = INT_MAX;
			break;
		case 6:
			sz = INT_MIN;
			break;
		default:
			sz = (int)(rnd_u32() & 0xffffu);
			break;
		}
		char label[48];
		std::snprintf(label, sizeof(label), "rnd%u", i);
		test_setbuffer_one(label, sz, use_buf);
	}
}

bool
test_setlinebuf_one(const char *label)
{
	FILE *rf = new_tmp_write();
	FILE *pf = new_tmp_write();
	if (!rf || !pf) {
		if (rf)
			std::fclose(rf);
		if (pf)
			std::fclose(pf);
		fail_msg(S_SETLINEBUF, label, "tmpfile failed");
		case_inc(S_SETLINEBUF);
		return false;
	}

	int rr = ref_setlinebuf(rf);
	int rp = port::setlinebuf(pf);

	bool ok = true;
	if (rr != rp) {
		fail_msg(S_SETLINEBUF, label, "return mismatch");
		ok = false;
	}
	if (!probe_write_read(rf, pf, S_SETLINEBUF, label))
		ok = false;

	std::fputc('a', rf);
	std::fputc('\n', rf);
	std::fflush(rf);
	std::fputc('a', pf);
	std::fputc('\n', pf);
	std::fflush(pf);
	std::rewind(rf);
	std::rewind(pf);
	if (std::fgetc(rf) != std::fgetc(pf) ||
	    std::fgetc(rf) != std::fgetc(pf)) {
		fail_msg(S_SETLINEBUF, label, "line flush read mismatch");
		ok = false;
	}

	std::fclose(rf);
	std::fclose(pf);
	case_inc(S_SETLINEBUF);
	return ok;
}

void
run_setlinebuf_edges(void)
{
	for (int k = 0; k < 16; k++) {
		char label[32];
		std::snprintf(label, sizeof(label), "edge %d", k);
		test_setlinebuf_one(label);
	}
}

void
run_setlinebuf_random(void)
{
	for (unsigned i = 0; i < RAND_ITERS; i++) {
		char label[48];
		std::snprintf(label, sizeof(label), "rnd%u", i);
		test_setlinebuf_one(label);
	}
}

int saved_stdin = -1;
char stdin_tmp_path[] = "/tmp/pbsd_b0092_stdin_XXXXXX";

bool
push_stdin_wide(const wchar_t *ws, std::size_t n)
{
	int fd = mkstemp(stdin_tmp_path);
	if (fd < 0)
		return false;

	if (n > 0) {
		const unsigned char *bp =
		    reinterpret_cast<const unsigned char *>(ws);
		std::size_t nbytes = n * sizeof(wchar_t);
		std::size_t off = 0;
		while (off < nbytes) {
			ssize_t w = write(fd, bp + off, nbytes - off);
			if (w <= 0) {
				close(fd);
				unlink(stdin_tmp_path);
				return false;
			}
			off += (std::size_t)w;
		}
	}
	close(fd);

	if (saved_stdin < 0)
		saved_stdin = dup(STDIN_FILENO);
	if (freopen(stdin_tmp_path, "r", stdin) == nullptr) {
		unlink(stdin_tmp_path);
		return false;
	}
	unlink(stdin_tmp_path);
	clearerr(stdin);
	fwide(stdin, 1);
	return true;
}

void
pop_stdin(void)
{
	if (saved_stdin >= 0) {
		dup2(saved_stdin, STDIN_FILENO);
		close(saved_stdin);
		saved_stdin = -1;
	}
	clearerr(stdin);
}

bool
test_wscanf_int(StatId which, const char *label, const wchar_t *input,
    std::size_t n, const wchar_t *fmt)
{
	GuardedInt gi_r, gi_p;
	gi_r.fill_guard();
	gi_p.fill_guard();

	int rr = -9999;
	int rp = -9999;
	if (push_stdin_wide(input, n)) {
		if (which == S_WSCANF)
			rr = ref_wscanf(fmt, &gi_r.val);
		else {
			locale_t loc = newlocale(LC_ALL_MASK, "C", NULL);
			rr = ref_wscanf_l(loc, fmt, &gi_r.val);
			freelocale(loc);
		}
	}
	if (push_stdin_wide(input, n)) {
		if (which == S_WSCANF)
			rp = (wscanf)(fmt, &gi_p.val);
		else {
			locale_t loc = newlocale(LC_ALL_MASK, "C", NULL);
			rp = (wscanf_l)(loc, fmt, &gi_p.val);
			freelocale(loc);
		}
	}
	pop_stdin();

	bool ok = true;
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
test_wscanf_wchar(StatId which, const char *label, const wchar_t *input,
    std::size_t n, const wchar_t *fmt)
{
	GuardedWBuf gw_r, gw_p;
	gw_r.fill_guard();
	gw_p.fill_guard();

	int rr = -9999;
	int rp = -9999;
	if (push_stdin_wide(input, n)) {
		if (which == S_WSCANF)
			rr = ref_wscanf(fmt, gw_r.user());
		else {
			locale_t loc = newlocale(LC_ALL_MASK, "C", NULL);
			rr = ref_wscanf_l(loc, fmt, gw_r.user());
			freelocale(loc);
		}
	}
	if (push_stdin_wide(input, n)) {
		if (which == S_WSCANF)
			rp = (wscanf)(fmt, gw_p.user());
		else {
			locale_t loc = newlocale(LC_ALL_MASK, "C", NULL);
			rp = (wscanf_l)(loc, fmt, gw_p.user());
			freelocale(loc);
		}
	}
	pop_stdin();

	bool ok = true;
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
test_wscanf_two_int(StatId which, const char *label, const wchar_t *input,
    std::size_t n, const wchar_t *fmt)
{
	GuardedInt gi_r1, gi_p1, gi_r2, gi_p2;
	gi_r1.fill_guard();
	gi_p1.fill_guard();
	gi_r2.fill_guard();
	gi_p2.fill_guard();

	int rr = -9999;
	int rp = -9999;
	if (push_stdin_wide(input, n)) {
		if (which == S_WSCANF)
			rr = ref_wscanf(fmt, &gi_r1.val, &gi_r2.val);
		else {
			locale_t loc = newlocale(LC_ALL_MASK, "C", NULL);
			rr = ref_wscanf_l(loc, fmt, &gi_r1.val, &gi_r2.val);
			freelocale(loc);
		}
	}
	if (push_stdin_wide(input, n)) {
		if (which == S_WSCANF)
			rp = (wscanf)(fmt, &gi_p1.val, &gi_p2.val);
		else {
			locale_t loc = newlocale(LC_ALL_MASK, "C", NULL);
			rp = (wscanf_l)(loc, fmt, &gi_p1.val, &gi_p2.val);
			freelocale(loc);
		}
	}
	pop_stdin();

	bool ok = true;
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
run_wscanf_edges(StatId which)
{
	static const wchar_t w42[] = { '4', '2', '\n' };
	static const wchar_t w12[] = { '1', ' ', '2', '\n' };
	static const wchar_t wempty[] = { '\n' };
	static const wchar_t wx[] = { 'x', '\n' };
	static const wchar_t whi[] = { (wchar_t)0x80, (wchar_t)0xfffe, '\n' };
	static const wchar_t wnone[] = { 'z', '\n' };
	static const wchar_t w0[] = { '0', '\n' };

	test_wscanf_int(which, "d 42", w42, 3, L"%d");
	test_wscanf_two_int(which, "d d 1 2", w12, 4, L"%d %d");
	test_wscanf_int(which, "d empty", wempty, 1, L"%d");
	test_wscanf_wchar(which, "lc x", wx, 2, L"%lc");
	test_wscanf_wchar(which, "lc hi", whi, 3, L"%lc");
	test_wscanf_int(which, "d nomatch", wnone, 2, L"%d");
	test_wscanf_int(which, "d zero", w0, 2, L"%d");
	test_wscanf_int(which, "d eof", w42, 0, L"%d");
}

void
run_wscanf_random(StatId which)
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
			else
				input[j] = (wchar_t)('0' + (v % 10u));
		}
		input[n - 1] = '\n';

		char label[48];
		std::snprintf(label, sizeof(label), "rnd%u", i);

		if (kind == 0) {
			std::wcscpy(fmt, L"%d");
			test_wscanf_int(which, label, input, n, fmt);
		} else if (kind == 1) {
			std::wcscpy(fmt, L"%lc");
			test_wscanf_wchar(which, label, input, n, fmt);
		} else if (kind == 2) {
			std::wcscpy(fmt, L"%d %d");
			test_wscanf_two_int(which, label, input, n, fmt);
		} else if (kind == 3) {
			std::wcscpy(fmt, L"%1d");
			test_wscanf_int(which, label, input, n, fmt);
		} else {
			std::wcscpy(fmt, L"%2d");
			test_wscanf_int(which, label, input, n, fmt);
		}
	}
}

bool
test_getwc_stream(StatId which, const wchar_t *ws, std::size_t n,
    const char *label)
{
	FILE *rf = mk_wchar_file(ws, n);
	FILE *pf = mk_wchar_file(ws, n);
	if (!rf || !pf) {
		if (rf)
			std::fclose(rf);
		if (pf)
			std::fclose(pf);
		fail_msg(which, label, "tmpfile failed");
		case_inc(which);
		return false;
	}

	locale_t loc = newlocale(LC_ALL_MASK, "C", NULL);
	bool ok = true;
	for (;;) {
		wint_t wr = (which == S_GETWC) ? ref_getwc(rf) :
		    ref_getwc_l(rf, loc);
		wint_t wp = (which == S_GETWC) ? port::getwc(pf) :
		    port::getwc_l(pf, loc);
		case_inc(which);
		if (wr != wp) {
			fail_msg(which, label, "wint mismatch");
			ok = false;
			break;
		}
		if (wr == WEOF)
			break;
	}

	std::fclose(rf);
	std::fclose(pf);
	freelocale(loc);
	return ok;
}

void
run_getwc_edges(StatId which)
{
	static const wchar_t wempty[] = {};
	static const wchar_t wone[] = { 'A' };
	static const wchar_t wnul[] = { '\0', 'B' };
	static const wchar_t whi[] = {
		(wchar_t)0x80, (wchar_t)0xff, (wchar_t)0xffff
	};

	test_getwc_stream(which, wempty, 0, "empty");
	test_getwc_stream(which, wone, 1, "one");
	test_getwc_stream(which, wnul, 2, "nul heavy");
	test_getwc_stream(which, whi, 3, "highbit");
}

void
run_getwc_random(StatId which)
{
	wchar_t buf[64];

	for (unsigned i = 0; i < RAND_ITERS; i++) {
		std::size_t n = rnd_u32() % 48u;
		for (std::size_t j = 0; j < n; j++) {
			unsigned v = rnd_u32();
			if ((v & 3u) == 0)
				buf[j] = (wchar_t)(0x80 + (v & 0x7fu));
			else if ((v & 3u) == 1)
				buf[j] = (wchar_t)'\0';
			else
				buf[j] = (wchar_t)(v & 0xffffu);
		}
		char label[48];
		std::snprintf(label, sizeof(label), "rnd%u", i);
		test_getwc_stream(which, buf, n, label);
	}
}

void
reset_stdin_for_wscanf(void)
{
	if (saved_stdin < 0)
		saved_stdin = dup(STDIN_FILENO);
	(void)freopen("/dev/null", "r", stdin);
	clearerr(stdin);
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
	run_setbuffer_edges();
	run_setbuffer_random();
	run_setlinebuf_edges();
	run_setlinebuf_random();
	reset_stdin_for_wscanf();
	run_wscanf_edges(S_WSCANF);
	run_wscanf_random(S_WSCANF);
	run_wscanf_edges(S_WSCANF_L);
	run_wscanf_random(S_WSCANF_L);
	run_getwc_edges(S_GETWC);
	run_getwc_random(S_GETWC);
	run_getwc_edges(S_GETWC_L);
	run_getwc_random(S_GETWC_L);

	report();
	long long fails = 0;
	for (int i = 0; i < NSTAT; i++)
		fails += g_stat[i].fails;
	return fails == 0 ? 0 : 1;
}
