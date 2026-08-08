/*
 * harness.cpp -- differential test for PBSD batch b0106.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <locale.h>

import pbsd.lib.libc.stdio.b0106;

namespace port = pbsd::lib_libc_stdio::b0106;

extern "C" {
int ref_fputs_unlocked(const char *__restrict, FILE *__restrict);
int ref_fputs(const char *__restrict, FILE *__restrict);
int ref_scanf(char const *__restrict, ...);
int ref_scanf_l(locale_t, char const *__restrict, ...);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr std::size_t FILEBUF = 4096;
constexpr std::size_t STRBUF = 512;
constexpr std::size_t STR_PRE = 16;
constexpr std::size_t STR_POST = 16;
constexpr std::size_t STR_CAP = STR_PRE + STRBUF + STR_POST;
constexpr unsigned RAND_ITERS = 200000u;
constexpr int MAXPRINT = 8;

enum StatId {
	S_FPUTS_UNLOCKED,
	S_FPUTS,
	S_SCANF,
	S_SCANF_L,
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
	{ "scanf",          0, 0, 0 },
	{ "scanf_l",        0, 0, 0 },
};

std::uint64_t rng_state = 0xb0106cafebabeULL;

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

	void set_string(const char *s, std::size_t n)
	{
		fill_guard();
		if (n > STRBUF)
			n = STRBUF;
		std::memcpy(user(), s, n);
		user()[n] = '\0';
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

struct GuardedCharBuf {
	static constexpr std::size_t CPRE = 8;
	static constexpr std::size_t CUSER = 128;
	static constexpr std::size_t CPOST = 8;
	static constexpr std::size_t CCAP = CPRE + CUSER + CPOST;
	unsigned char bytes[CCAP];

	void fill_guard(void)
	{
		std::memset(bytes, GUARD, CCAP);
	}

	char *user(void)
	{
		return reinterpret_cast<char *>(bytes + CPRE);
	}

	bool eq_bytes(const GuardedCharBuf &o) const
	{
		return std::memcmp(bytes, o.bytes, CCAP) == 0;
	}
};

FILE *
new_tmp_write(void)
{
	return std::tmpfile();
}

FILE *
new_tmp_readonly(void)
{
	FILE *w = std::tmpfile();
	if (!w)
		return nullptr;
	std::fputs("x", w);
	std::fflush(w);
	std::rewind(w);
	return std::freopen(nullptr, "r", w);
}

bool
read_file_contents(FILE *fp, unsigned char *out, std::size_t cap,
    std::size_t *len_out)
{
	std::size_t n = 0;
	int c;

	std::rewind(fp);
	while ((c = std::fgetc(fp)) != EOF) {
		if (n < cap)
			out[n] = (unsigned char)c;
		n++;
	}
	*len_out = n;
	return true;
}

bool
test_fputs_one(StatId which, const char *label, const char *s,
    bool readonly_fp)
{
	unsigned char out_r[FILEBUF], out_p[FILEBUF];
	std::size_t len_r = 0, len_p = 0;
	FILE *rf, *pf;
	int rr, rp;
	bool ok = true;

	std::memset(out_r, GUARD, FILEBUF);
	std::memset(out_p, GUARD, FILEBUF);

	if (readonly_fp) {
		rf = new_tmp_readonly();
		pf = new_tmp_readonly();
	} else {
		rf = new_tmp_write();
		pf = new_tmp_write();
	}
	if (!rf || !pf) {
		if (rf)
			std::fclose(rf);
		if (pf)
			std::fclose(pf);
		fail_msg(which, label, "tmpfile failed");
		case_inc(which);
		return false;
	}

	if (which == S_FPUTS_UNLOCKED) {
		rr = ref_fputs_unlocked(s, rf);
		rp = port::fputs_unlocked(s, pf);
	} else {
		rr = ref_fputs(s, rf);
		rp = port::fputs(s, pf);
	}

	if (rr != rp) {
		fail_msg(which, label, "return mismatch");
		ok = false;
	}

	if (!readonly_fp) {
		read_file_contents(rf, out_r, FILEBUF, &len_r);
		read_file_contents(pf, out_p, FILEBUF, &len_p);
		if (len_r != len_p || std::memcmp(out_r, out_p, FILEBUF) != 0) {
			fail_msg(which, label, "file contents mismatch");
			ok = false;
		}
	}

	std::fclose(rf);
	std::fclose(pf);
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
	char longbuf[300];

	std::memset(longbuf, 'Z', sizeof(longbuf) - 1);
	longbuf[sizeof(longbuf) - 1] = '\0';

	test_fputs_one(which, "empty", empty, false);
	test_fputs_one(which, "one", one, false);
	test_fputs_one(which, "highbit", hi, false);
	test_fputs_one(which, "nul in middle", mix, false);
	test_fputs_one(which, "long", longbuf, false);
	test_fputs_one(which, "readonly fail", one, true);
}

void
run_fputs_random(StatId which)
{
	GuardedStr gs;

	for (unsigned i = 0; i < RAND_ITERS; i++) {
		std::size_t n = rnd_u32() % (STRBUF - 1);
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
		gs.data[STR_PRE + STRBUF] = GUARD;

		char label[48];
		std::snprintf(label, sizeof(label), "rnd%u", i);
		bool ro = (rnd_u32() % 997u) == 0;
		test_fputs_one(which, label, gs.user(), ro);
	}
}

int saved_stdin = -1;

bool
push_stdin_bytes(const unsigned char *data, std::size_t n)
{
	int pfd[2];

	if (pipe(pfd) != 0)
		return false;
	if (n > 0) {
		std::size_t off = 0;
		while (off < n) {
			ssize_t w = write(pfd[1], data + off, n - off);
			if (w <= 0) {
				close(pfd[0]);
				close(pfd[1]);
				return false;
			}
			off += (std::size_t)w;
		}
	}
	close(pfd[1]);
	if (saved_stdin < 0)
		saved_stdin = dup(STDIN_FILENO);
	dup2(pfd[0], STDIN_FILENO);
	close(pfd[0]);
	clearerr(stdin);
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

template<typename... Args>
int
call_ref_scanf(const unsigned char *input, std::size_t n, const char *fmt,
    Args... args)
{
	int r = -9999;

	if (!push_stdin_bytes(input, n))
		return -9999;
	r = ref_scanf(fmt, args...);
	pop_stdin();
	return r;
}

template<typename... Args>
int
call_port_scanf(const unsigned char *input, std::size_t n, const char *fmt,
    Args... args)
{
	int r = -9999;

	if (!push_stdin_bytes(input, n))
		return -9999;
	r = port::scanf(fmt, args...);
	pop_stdin();
	return r;
}

template<typename... Args>
int
call_ref_scanf_l(locale_t loc, const unsigned char *input, std::size_t n,
    const char *fmt, Args... args)
{
	int r = -9999;

	if (!push_stdin_bytes(input, n))
		return -9999;
	r = ref_scanf_l(loc, fmt, args...);
	pop_stdin();
	return r;
}

template<typename... Args>
int
call_port_scanf_l(locale_t loc, const unsigned char *input, std::size_t n,
    const char *fmt, Args... args)
{
	int r = -9999;

	if (!push_stdin_bytes(input, n))
		return -9999;
	r = port::scanf_l(loc, fmt, args...);
	pop_stdin();
	return r;
}

bool
test_scanf_int(StatId which, const char *label, const unsigned char *input,
    std::size_t n, const char *fmt)
{
	GuardedInt gi_r, gi_p;

	gi_r.fill_guard();
	gi_p.fill_guard();

	int rr, rp;
	if (which == S_SCANF) {
		rr = call_ref_scanf(input, n, fmt, &gi_r.val);
		rp = call_port_scanf(input, n, fmt, &gi_p.val);
	} else {
		locale_t loc = newlocale(LC_ALL_MASK, "C", NULL);
		rr = call_ref_scanf_l(loc, input, n, fmt, &gi_r.val);
		rp = call_port_scanf_l(loc, input, n, fmt, &gi_p.val);
		freelocale(loc);
	}

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
test_scanf_str(StatId which, const char *label, const unsigned char *input,
    std::size_t n, const char *fmt)
{
	GuardedCharBuf gb_r, gb_p;

	gb_r.fill_guard();
	gb_p.fill_guard();

	int rr, rp;
	if (which == S_SCANF) {
		rr = call_ref_scanf(input, n, fmt, gb_r.user());
		rp = call_port_scanf(input, n, fmt, gb_p.user());
	} else {
		locale_t loc = newlocale(LC_ALL_MASK, "C", NULL);
		rr = call_ref_scanf_l(loc, input, n, fmt, gb_r.user());
		rp = call_port_scanf_l(loc, input, n, fmt, gb_p.user());
		freelocale(loc);
	}

	bool ok = true;
	if (rr != rp) {
		fail_msg(which, label, "return mismatch");
		ok = false;
	}
	if (!gb_r.eq_bytes(gb_p)) {
		fail_msg(which, label, "str/guard mismatch");
		ok = false;
	}
	case_inc(which);
	return ok;
}

bool
test_scanf_two_int(StatId which, const char *label,
    const unsigned char *input, std::size_t n, const char *fmt)
{
	GuardedInt gi_r1, gi_p1, gi_r2, gi_p2;

	gi_r1.fill_guard();
	gi_p1.fill_guard();
	gi_r2.fill_guard();
	gi_p2.fill_guard();

	int rr, rp;
	if (which == S_SCANF) {
		rr = call_ref_scanf(input, n, fmt, &gi_r1.val, &gi_r2.val);
		rp = call_port_scanf(input, n, fmt, &gi_p1.val, &gi_p2.val);
	} else {
		locale_t loc = newlocale(LC_ALL_MASK, "C", NULL);
		rr = call_ref_scanf_l(loc, input, n, fmt, &gi_r1.val, &gi_r2.val);
		rp = call_port_scanf_l(loc, input, n, fmt, &gi_p1.val,
		    &gi_p2.val);
		freelocale(loc);
	}

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

bool
test_scanf_char(StatId which, const char *label, const unsigned char *input,
    std::size_t n, const char *fmt)
{
	unsigned char cr = 0x5a, cp = 0x5a;
	int rr, rp;

	if (which == S_SCANF) {
		rr = call_ref_scanf(input, n, fmt, &cr);
		rp = call_port_scanf(input, n, fmt, &cp);
	} else {
		locale_t loc = newlocale(LC_ALL_MASK, "C", NULL);
		rr = call_ref_scanf_l(loc, input, n, fmt, &cr);
		rp = call_port_scanf_l(loc, input, n, fmt, &cp);
		freelocale(loc);
	}

	bool ok = true;
	if (rr != rp) {
		fail_msg(which, label, "return mismatch");
		ok = false;
	}
	if (cr != cp) {
		fail_msg(which, label, "char mismatch");
		ok = false;
	}
	case_inc(which);
	return ok;
}

bool
test_scanf_int_null_locale(const char *label, const unsigned char *input,
    std::size_t n, const char *fmt)
{
	GuardedInt gi_r, gi_p;

	gi_r.fill_guard();
	gi_p.fill_guard();

	int rr = call_ref_scanf_l(nullptr, input, n, fmt, &gi_r.val);
	int rp = call_port_scanf_l(nullptr, input, n, fmt, &gi_p.val);

	bool ok = true;
	if (rr != rp) {
		fail_msg(S_SCANF_L, label, "return mismatch");
		ok = false;
	}
	if (!gi_r.eq(gi_p)) {
		fail_msg(S_SCANF_L, label, "int/guard mismatch");
		ok = false;
	}
	case_inc(S_SCANF_L);
	return ok;
}

void
run_scanf_edges(StatId which)
{
	static const unsigned char d42[] = { '4', '2', '\n' };
	static const unsigned char d12[] = { '1', ' ', '2', '\n' };
	static const unsigned char empty[] = { '\n' };
	static const unsigned char cx[] = { 'x', '\n' };
	static const unsigned char chi[] = { 0x80, 0xff, '\n' };
	static const unsigned char cnone[] = { 'z', '\n' };
	static const unsigned char c0[] = { '0', '\n' };
	static const unsigned char chex[] = { 'a', 'b', '\n' };
	static const unsigned char word[] = { 'h', 'i', '\n' };

	test_scanf_int(which, "d 42", d42, sizeof(d42), "%d");
	test_scanf_two_int(which, "d d 1 2", d12, sizeof(d12), "%d %d");
	test_scanf_int(which, "d empty", empty, sizeof(empty), "%d");
	test_scanf_char(which, "c x", cx, sizeof(cx), "%c");
	test_scanf_char(which, "c hi", chi, sizeof(chi), "%c");
	test_scanf_int(which, "d nomatch", cnone, sizeof(cnone), "%d");
	test_scanf_int(which, "d zero", c0, sizeof(c0), "%d");
	test_scanf_int(which, "d eof", d42, 0, "%d");
	test_scanf_int(which, "x ab", chex, sizeof(chex), "%x");
	test_scanf_str(which, "s hi", word, sizeof(word), "%s");
	if (which == S_SCANF_L)
		test_scanf_int_null_locale("null locale", d42, sizeof(d42), "%d");
}

void
run_scanf_random(StatId which)
{
	unsigned char input[64];
	char fmt[16];

	for (unsigned i = 0; i < RAND_ITERS; i++) {
		std::size_t n = (rnd_u32() % 32u) + 1u;
		for (std::size_t j = 0; j < n; j++) {
			unsigned v = rnd_u32();
			if ((v & 7u) == 0)
				input[j] = (unsigned char)(0x80 + (v & 0x7fu));
			else if ((v & 7u) == 1)
				input[j] = '\n';
			else
				input[j] = (unsigned char)('0' + (v % 10u));
		}
		input[n - 1] = '\n';

		char label[48];
		std::snprintf(label, sizeof(label), "rnd%u", i);

		switch (rnd_u32() % 6) {
		case 0:
			std::strcpy(fmt, "%d");
			test_scanf_int(which, label, input, n, fmt);
			break;
		case 1:
			std::strcpy(fmt, "%c");
			test_scanf_char(which, label, input, n, fmt);
			break;
		case 2:
			std::strcpy(fmt, "%d %d");
			test_scanf_two_int(which, label, input, n, fmt);
			break;
		case 3:
			std::strcpy(fmt, "%1d");
			test_scanf_int(which, label, input, n, fmt);
			break;
		case 4:
			std::strcpy(fmt, "%x");
			test_scanf_int(which, label, input, n, fmt);
			break;
		default:
			std::strcpy(fmt, "%2d");
			test_scanf_int(which, label, input, n, fmt);
			break;
		}
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
	run_fputs_edges(S_FPUTS_UNLOCKED);
	run_fputs_random(S_FPUTS_UNLOCKED);
	run_fputs_edges(S_FPUTS);
	run_fputs_random(S_FPUTS);
	run_scanf_edges(S_SCANF);
	run_scanf_random(S_SCANF);
	run_scanf_edges(S_SCANF_L);
	run_scanf_random(S_SCANF_L);

	report();
	long long fails = 0;
	for (int i = 0; i < NSTAT; i++)
		fails += g_stat[i].fails;
	return fails == 0 ? 0 : 1;
}
