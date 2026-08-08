/*
 * harness.cpp -- differential test for PBSD batch b0148s1.
 */

import pbsd.bin.ed.b0148s1;

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <regex.h>

namespace port = pbsd::bin_ed::b0148s1;

extern "C" {
extern char *ibuf;
extern char *ibufp;
extern int isbinary;
extern int patlock;
extern const char *errmsg;
extern int oracle_malloc_fail_at;
void oracle_reset_batch(void);
char *ref_parse_char_class(char *);
char *ref_extract_pattern(int);
regex_t *ref_get_compiled_pattern(void);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr long RANDOM_ITERS = 210000;

struct Stat { const char *name; long cases, fails; };
Stat stats[8];
int nstats;
std::uint64_t rng = 0x243f6a8885a308d3ULL;

std::uint64_t rnd()
{
	std::uint64_t z = (rng += 0x9e3779b97f4a7c15ULL);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

int rndb() { return (int)(rnd() & 0xff); }

Stat &reg(const char *n)
{
	stats[nstats++] = {n, 0, 0};
	return stats[nstats - 1];
}

void reset_both()
{
	oracle_reset_batch();
	port::reset_batch();
}

void setup_ibuf(const char *s)
{
	std::strcpy(ibuf, s);
	ibufp = ibuf;
	port::isbinary = isbinary;
	port::patlock = patlock;
	std::memcpy(port::ibuf, ibuf, 65536);
	port::ibufp = port::ibuf + (ibufp - ibuf);
}

struct GuardBuf {
	unsigned char pre[32];
	unsigned char data[512];
	unsigned char post[32];
};

void fill_guard(GuardBuf &g)
{
	std::memset(g.pre, GUARD, 32);
	std::memset(g.post, GUARD, 32);
	std::memset(g.data, 0, 512);
}

bool guard_ok(const GuardBuf &g)
{
	for (int i = 0; i < 32; i++) {
		if (g.pre[i] != GUARD || g.post[i] != GUARD)
			return false;
	}
	return true;
}

void test_parse_char_class()
{
	Stat &st = reg("parse_char_class");

	auto run = [&](const char *in) {
		GuardBuf rg, pg;
		fill_guard(rg);
		fill_guard(pg);
		std::strncpy((char *)rg.data, in, 511);
		std::strncpy((char *)pg.data, in, 511);
		char *r = ref_parse_char_class((char *)rg.data);
		char *p = port::parse_char_class((char *)pg.data);
		long ro = r ? (r - (char *)rg.data) : -1;
		long po = p ? (p - (char *)pg.data) : -1;
		st.cases++;
		if (ro != po || std::memcmp(rg.pre, pg.pre, 32) ||
		    std::memcmp(rg.data, pg.data, 512) ||
		    std::memcmp(rg.post, pg.post, 32) || !guard_ok(rg) ||
		    !guard_ok(pg))
			st.fails++;
	};

	const char *edge[] = {
	    "",
	    "]",
	    "a]",
	    "[a]",
	    "[]",
	    "[^]",
	    "[^a]",
	    "[[:alpha:]]",
	    "[\n",
	    "[a\n",
	    "[.[.]]",
	    "[:.:]",
	    "[=.=]",
	    "[^]]]",
	    "[\xff]",
	    "[\x80]",
	    "x",
	    "[[",
	    "[[:",
	    "[=x\n",
	    "[.x\n",
	    "[::\n",
	    "[a-z]",
	    "[\\]]",
	    "[^].]",
	};
	for (auto s : edge)
		run(s);

	for (long i = 0; i < RANDOM_ITERS / 3; i++) {
		char b[128];
		int n = (int)(rnd() % 100) + 1;
		for (int j = 0; j < n; j++) {
			b[j] = (char)rndb();
			if (b[j] == '\0')
				b[j] = 'a';
		}
		b[n] = '\0';
		run(b);
	}
}

void test_extract_pattern()
{
	Stat &st = reg("extract_pattern");

	auto run = [&](const char *cmd, int delim, int binary) {
		reset_both();
		isbinary = binary;
		setup_ibuf(cmd);
		const char *re_before = errmsg;
		char *r = ref_extract_pattern(delim);
		long ri = ibufp - ibuf;
		const char *re_after = errmsg;

		reset_both();
		port::isbinary = binary;
		setup_ibuf(cmd);
		const char *pe_before = port::errmsg;
		char *p = port::extract_pattern(delim);
		long pi = port::ibufp - port::ibuf;
		const char *pe_after = port::errmsg;

		st.cases++;
		bool bad = false;
		if ((r == nullptr) != (p == nullptr))
			bad = true;
		if (ri != pi)
			bad = true;
		if (std::strcmp(re_before, pe_before) || std::strcmp(re_after, pe_after))
			bad = true;
		if (r && p) {
			if (std::strcmp(r, p))
				bad = true;
			int len = (int)std::strlen(r);
			if (std::memcmp(r, p, (std::size_t)len + 1))
				bad = true;
		}
		if (bad)
			st.fails++;
	};

	const char *edge[] = {
	    "/foo/x",
	    "/[a]/x",
	    "/[/x",
	    "/\\\n",
	    "/a/x",
	    "/\n",
	    "#foo#x",
	    "/\xff/x",
	    "/\x80/x",
	    "/[]/x",
	    "/[[:alpha:]]/x",
	    "/[.[.]]/x",
	    "/\n",
	    "x\n",
	    "/a\n",
	    "/\\/x",
	    "/[a-z]/x",
	    "/[^]]/x",
	};
	for (auto s : edge)
		run(s, s[0], 0);
	for (auto s : edge)
		run(s, s[0], 1);

	for (long i = 0; i < RANDOM_ITERS / 3; i++) {
		char b[120];
		int d = (int)('!' + rnd() % 60);
		if (d == ' ' || d == '\n')
			d = '#';
		b[0] = (char)d;
		int n = (int)(rnd() % 40) + 1;
		for (int j = 1; j < n; j++)
			b[j] = (char)rndb();
		b[n] = (char)d;
		b[n + 1] = 'x';
		b[n + 2] = '\n';
		b[n + 3] = '\0';
		run(b, d, (int)(rnd() & 1));
	}
}

void test_get_compiled_pattern()
{
	Stat &st = reg("get_compiled_pattern");

	auto run = [&](const char *cmd, int fail_at) {
		reset_both();
		oracle_malloc_fail_at = fail_at;
		port::malloc_fail_at = fail_at;
		setup_ibuf(cmd);
		int pl = patlock;
		port::patlock = pl;
		regex_t *r = ref_get_compiled_pattern();
		const char *re = errmsg;
		int rpl = patlock;

		reset_both();
		oracle_malloc_fail_at = fail_at;
		port::malloc_fail_at = fail_at;
		setup_ibuf(cmd);
		port::patlock = pl;
		regex_t *p = port::get_compiled_pattern();
		const char *pe = port::errmsg;
		int ppl = port::patlock;

		st.cases++;
		if ((r == nullptr) != (p == nullptr) || std::strcmp(re, pe) ||
		    rpl != ppl)
			st.fails++;
	};

	run("/a/\n", 0);
	run(" \n", 0);
	run("/[/\n", 0);
	run("/\n", 0);
	run("/a/\n", 0);
	run("/\n", 0);
	run("/b/\n", 0);
	run("/a/a/\n", 0);
	run("\n", 0);
	run("/\\\n", 0);
	run("/[a]/\n", 0);
	run("/\xff/\n", 0);

	{
		reset_both();
		setup_ibuf("/a/\n");
		regex_t *r0 = ref_get_compiled_pattern();
		patlock = 1;
		setup_ibuf("/b/\n");
		regex_t *r1 = ref_get_compiled_pattern();
		const char *re = errmsg;
		int rpl = patlock;

		reset_both();
		setup_ibuf("/a/\n");
		regex_t *p0 = port::get_compiled_pattern();
		port::patlock = 1;
		setup_ibuf("/b/\n");
		regex_t *p1 = port::get_compiled_pattern();
		const char *pe = port::errmsg;
		int ppl = port::patlock;

		st.cases++;
		if ((r0 == nullptr) != (p0 == nullptr) ||
		    (r1 == nullptr) != (p1 == nullptr) ||
		    std::strcmp(re, pe) || rpl != ppl)
			st.fails++;
	}

	run("/c/\n", 1);

	for (long i = 0; i < RANDOM_ITERS / 3; i++) {
		char b[80];
		int d = (int)('a' + rnd() % 26);
		b[0] = (char)d;
		int n = (int)(rnd() % 20) + 1;
		for (int j = 1; j < n; j++)
			b[j] = (char)('a' + rnd() % 26);
		b[n] = (char)d;
		b[n + 1] = '\n';
		b[n + 2] = '\0';
		run(b, 0);
	}
}

} /* namespace */

int main()
{
	test_parse_char_class();
	test_extract_pattern();
	test_get_compiled_pattern();

	long total_cases = 0;
	long total_fails = 0;
	std::printf("function                 cases     fails\n");
	std::printf("----------------------------------------\n");
	for (int i = 0; i < nstats; i++) {
		std::printf("%-24s %7ld %7ld\n", stats[i].name, stats[i].cases,
		    stats[i].fails);
		total_cases += stats[i].cases;
		total_fails += stats[i].fails;
	}
	std::printf("----------------------------------------\n");
	std::printf("%-24s %7ld %7ld\n", "TOTAL", total_cases, total_fails);
	return total_fails ? 1 : 0;
}
