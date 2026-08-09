/*
 * Differential test harness for PBSD batch b0191s2 (hbsd/src/bin/cp/cp.c).
 *
 * Every ported function is exercised against the ref_ oracle built from the
 * unmodified C source.  Buffers handed to the two implementations are kept
 * bit-identical and compared in full (guard bytes included) after each call.
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <fts.h>
#include <limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <initializer_list>
#include <vector>

import pbsd.bin.cp.b0191s2;

namespace P = pbsd::bin_cp::b0191s2;

extern "C" {
int ref_ftscmp(const FTSENT *const *a, const FTSENT *const *b);
void ref_siginfo(int sig);
extern volatile sig_atomic_t info;
}

/* ------------------------------------------------------------------ */
/* bookkeeping                                                        */
/* ------------------------------------------------------------------ */

struct Stats {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
	unsigned long long reported;
};

static Stats st_ftscmp = { "ftscmp", 0, 0, 0 };
static Stats st_siginfo = { "siginfo", 0, 0, 0 };

static const unsigned long long REPORT_LIMIT = 12;

/* ------------------------------------------------------------------ */
/* deterministic PRNG (splitmix64, fixed seed)                        */
/* ------------------------------------------------------------------ */

static uint64_t rng_state;

static void
rng_seed(uint64_t s)
{
	rng_state = s;
}

static uint64_t
rnd(void)
{
	uint64_t z = (rng_state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return (z ^ (z >> 31));
}

static uint64_t
rnd_below(uint64_t n)
{
	return (n == 0 ? 0 : rnd() % n);
}

/* ------------------------------------------------------------------ */
/* byte-string helpers                                                */
/* ------------------------------------------------------------------ */

typedef std::vector<unsigned char> Bytes;

static Bytes
B(std::initializer_list<int> il)
{
	Bytes b;
	for (int v : il)
		b.push_back((unsigned char)v);
	return b;
}

static Bytes
S(const char *s)
{
	Bytes b;
	for (const char *p = s; *p != '\0'; p++)
		b.push_back((unsigned char)*p);
	return b;
}

static Bytes
rep(unsigned char c, size_t n)
{
	return Bytes(n, c);
}

static void
hexdump(const Bytes &b)
{
	fputc('"', stdout);
	for (size_t i = 0; i < b.size(); i++)
		printf("\\x%02x", (unsigned)b[i]);
	fputc('"', stdout);
}

/* ------------------------------------------------------------------ */
/* ftscmp                                                             */
/* ------------------------------------------------------------------ */

/*
 * glibc declares FTSENT with a trailing "char fts_name[1]"; real entries are
 * over-allocated.  We build our own over-allocated entries in a fixed-size
 * blob so that the whole blob (name window plus everything around it) can be
 * memcmp'd after the call.
 */
#define NAME_CAP 48
#define ENT_BLOB (sizeof(FTSENT) + NAME_CAP + 16)

struct EntBuf {
	alignas(16) unsigned char raw[ENT_BLOB];
};

static const unsigned char GUARD = 0x7f;

static void
fill_ent(EntBuf &e, const Bytes &name)
{
	size_t off = offsetof(FTSENT, fts_name);

	memset(e.raw, GUARD, sizeof(e.raw));
	if (name.size() != 0)
		memcpy(e.raw + off, &name[0], name.size());
	e.raw[off + name.size()] = 0;
}

static const FTSENT *
ent_of(const EntBuf &e)
{
	return (const FTSENT *)(const void *)e.raw;
}

static void
ftscmp_case(const Bytes &na, const Bytes &nb)
{
	EntBuf aP, bP, aO, bO;

	st_ftscmp.cases++;

	/* Two independent sets of buffers, guard-filled, identical content. */
	fill_ent(aP, na);
	fill_ent(bP, nb);
	fill_ent(aO, na);
	fill_ent(bO, nb);

	const FTSENT *pa = ent_of(aP);
	const FTSENT *pb = ent_of(bP);
	const FTSENT *oa = ent_of(aO);
	const FTSENT *ob = ent_of(bO);
	const FTSENT *pa0 = pa, *pb0 = pb, *oa0 = oa, *ob0 = ob;

	int rp = P::ftscmp(&pa, &pb);
	int ro = ref_ftscmp(&oa, &ob);

	int bad = 0;
	const char *why = "";

	if (rp != ro) {
		bad = 1;
		why = "return value";
	} else if (memcmp(aP.raw, aO.raw, sizeof(aP.raw)) != 0) {
		bad = 1;
		why = "buffer a (incl. guard bytes)";
	} else if (memcmp(bP.raw, bO.raw, sizeof(bP.raw)) != 0) {
		bad = 1;
		why = "buffer b (incl. guard bytes)";
	} else if (pa != pa0 || pb != pb0 || oa != oa0 || ob != ob0) {
		bad = 1;
		why = "argument pointer clobbered";
	}

	if (bad) {
		st_ftscmp.failures++;
		if (st_ftscmp.reported < REPORT_LIMIT) {
			st_ftscmp.reported++;
			printf("  FAIL ftscmp [%s]: a=", why);
			hexdump(na);
			printf(" b=");
			hexdump(nb);
			printf(" port=%d oracle=%d\n", rp, ro);
		}
	}
}

static Bytes
rand_name(size_t cap)
{
	unsigned mode = (unsigned)rnd_below(6);
	size_t len = (size_t)rnd_below(cap + 1);
	Bytes b(len);

	for (size_t i = 0; i < len; i++) {
		unsigned char c;
		switch (mode) {
		case 0:
			c = (unsigned char)('a' + rnd_below(2));
			break;
		case 1:
			c = (unsigned char)(1 + rnd_below(255));
			break;
		case 2:
			c = (unsigned char)(0x80 + rnd_below(0x80));
			break;
		case 3:
			c = (unsigned char)rnd_below(256);
			break;
		case 4:
			c = (unsigned char)(0x7e + rnd_below(4));
			break;
		default:
			c = (unsigned char)('A' + rnd_below(26));
			break;
		}
		b[i] = c;
	}
	return b;
}

static Bytes
perturb(const Bytes &a, size_t cap)
{
	Bytes b = a;
	unsigned what = (unsigned)rnd_below(4);

	if (what == 0 && !b.empty()) {
		size_t i = (size_t)rnd_below(b.size());
		b[i] = (unsigned char)rnd_below(256);
	} else if (what == 1 && !b.empty()) {
		b.pop_back();
	} else if (what == 2 && b.size() < cap) {
		b.push_back((unsigned char)rnd_below(256));
	} else if (!b.empty()) {
		size_t i = b.size() - 1;
		b[i] = (unsigned char)(b[i] + 1);
	}
	return b;
}

static void
test_ftscmp(void)
{
	std::vector<Bytes> names;

	/* Hand-written edge cases. */
	names.push_back(Bytes());			/* empty */
	names.push_back(B({ 0x00 }));			/* leading NUL -> empty */
	names.push_back(B({ 0x00, 0x41, 0x42 }));	/* NUL then garbage */
	names.push_back(B({ 0x00, 0x00, 0x00, 0x00 }));	/* NUL heavy */
	names.push_back(B({ 0x61 }));			/* "a" */
	names.push_back(B({ 0x62 }));			/* "b" */
	names.push_back(B({ 0x01 }));
	names.push_back(B({ 0x7e }));
	names.push_back(B({ 0x7f }));
	names.push_back(B({ 0x80 }));			/* high bit */
	names.push_back(B({ 0x81 }));
	names.push_back(B({ 0xfe }));
	names.push_back(B({ 0xff }));
	names.push_back(B({ 0x61, 0x00, 0x62 }));	/* "a\0b" */
	names.push_back(B({ 0x61, 0x00, 0x63 }));	/* "a\0c" -> equal to above */
	names.push_back(B({ 0x61, 0x62, 0x63 }));	/* "abc" */
	names.push_back(B({ 0x61, 0x62, 0x64 }));	/* "abd" */
	names.push_back(B({ 0x61, 0x62, 0x63, 0x64 }));	/* "abcd" */
	names.push_back(B({ 0x61, 0x62, 0x63, 0x00, 0xff })); /* "abc" + junk */
	names.push_back(B({ 0x61, 0x80 }));
	names.push_back(B({ 0x61, 0x7f }));
	names.push_back(B({ 0x80, 0x61 }));
	names.push_back(B({ 0xff, 0xff, 0xff }));
	names.push_back(B({ 0xff, 0xff, 0xfe }));
	names.push_back(S("."));
	names.push_back(S(".."));
	names.push_back(S("/"));
	names.push_back(S("a.txt"));
	names.push_back(S("A.txt"));
	names.push_back(S("z"));
	names.push_back(S("Z"));

	/* Boundary lengths, including the longest name the blob can hold. */
	{
		const size_t cap = NAME_CAP - 1;
		size_t lens[] = { 0, 1, 2, 3, cap - 2, cap - 1, cap };
		for (size_t i = 0; i < sizeof(lens) / sizeof(lens[0]); i++) {
			size_t L = lens[i];
			Bytes x = rep('a', L);
			names.push_back(x);
			if (L > 0) {
				Bytes y = x;
				y[L - 1] = 'b';		/* differs in last byte */
				names.push_back(y);
				Bytes z = x;
				z[0] = 'b';		/* differs in first byte */
				names.push_back(z);
				Bytes w = x;
				w[L - 1] = 0xff;	/* high bit in last byte */
				names.push_back(w);
			}
		}
	}

	/* Full cross product of the hand-written set (both argument orders). */
	for (size_t i = 0; i < names.size(); i++)
		for (size_t j = 0; j < names.size(); j++)
			ftscmp_case(names[i], names[j]);

	/* Exhaustive single-byte cross product, 0x00-0xff on both sides. */
	for (unsigned i = 0; i < 256; i++) {
		for (unsigned j = 0; j < 256; j++) {
			Bytes a = B({ (int)i });
			Bytes b = B({ (int)j });
			ftscmp_case(a, b);
		}
	}

	/* Exhaustive two-byte cross product over sign-boundary values. */
	{
		static const int vals[] = { 0x00, 0x01, 0x41, 0x61, 0x7e,
		    0x7f, 0x80, 0x81, 0xfe, 0xff };
		const size_t nv = sizeof(vals) / sizeof(vals[0]);
		std::vector<Bytes> two;
		for (size_t i = 0; i < nv; i++)
			for (size_t j = 0; j < nv; j++)
				two.push_back(B({ vals[i], vals[j] }));
		for (size_t i = 0; i < two.size(); i++)
			for (size_t j = 0; j < two.size(); j++)
				ftscmp_case(two[i], two[j]);
	}

	/* Fixed-seed randomised sweep. */
	rng_seed(0xC0FFEE1234ABCD01ULL);
	for (unsigned long i = 0; i < 250000UL; i++) {
		Bytes a = rand_name(NAME_CAP - 1);
		Bytes b;
		if (rnd_below(2) == 0)
			b = perturb(a, NAME_CAP - 1);
		else
			b = rand_name(NAME_CAP - 1);
		ftscmp_case(a, b);
	}
}

/* ------------------------------------------------------------------ */
/* siginfo                                                            */
/* ------------------------------------------------------------------ */

static void
siginfo_case(int sig, int preset)
{
	st_siginfo.cases++;

	P::info = (sig_atomic_t)preset;
	info = (sig_atomic_t)preset;

	P::siginfo(sig);
	ref_siginfo(sig);

	sig_atomic_t vp = P::info;
	sig_atomic_t vo = info;

	if (vp != vo) {
		st_siginfo.failures++;
		if (st_siginfo.reported < REPORT_LIMIT) {
			st_siginfo.reported++;
			printf("  FAIL siginfo: sig=%d preset=%d port=%ld "
			    "oracle=%ld\n", sig, preset, (long)vp, (long)vo);
		}
	}
}

static void
test_siginfo(void)
{
	static const int sigs[] = { 0, 1, -1, 2, 9, 15, 29, 31, 63, 64, 65,
	    127, 128, 255, 256, -128, -129, INT_MIN, INT_MAX, INT_MIN + 1,
	    INT_MAX - 1 };
	static const int presets[] = { 0, 1, -1, 2, -2, 127, 128, -128, 255,
	    0x7f7f7f7f, INT_MIN, INT_MAX, INT_MIN + 1, INT_MAX - 1 };
	const size_t ns = sizeof(sigs) / sizeof(sigs[0]);
	const size_t np = sizeof(presets) / sizeof(presets[0]);

	for (size_t i = 0; i < ns; i++)
		for (size_t j = 0; j < np; j++)
			siginfo_case(sigs[i], presets[j]);

	/* Fixed-seed randomised sweep. */
	rng_seed(0x5EED0BADF00D0011ULL);
	for (unsigned long i = 0; i < 220000UL; i++) {
		int sig;
		int preset;

		if (rnd_below(4) == 0)
			sig = (int)(int32_t)rnd();
		else
			sig = (int)rnd_below(70);

		switch (rnd_below(3)) {
		case 0:
			preset = (int)rnd_below(3) - 1;
			break;
		case 1:
			preset = (int)(int32_t)rnd();
			break;
		default:
			preset = (int)rnd_below(256);
			break;
		}
		siginfo_case(sig, preset);
	}
}

/* ------------------------------------------------------------------ */

static void
row(const Stats &s)
{
	printf("  %-12s %12llu %12llu   %s\n", s.name, s.cases, s.failures,
	    s.failures == 0 ? "ok" : "FAILED");
}

int
main(void)
{
	printf("pbsd b0191s2 differential harness (hbsd/src/bin/cp/cp.c)\n");

	test_ftscmp();
	test_siginfo();

	unsigned long long tc = st_ftscmp.cases + st_siginfo.cases;
	unsigned long long tf = st_ftscmp.failures + st_siginfo.failures;

	printf("\n");
	printf("  %-12s %12s %12s\n", "function", "cases", "failures");
	printf("  ------------------------------------------------\n");
	row(st_ftscmp);
	row(st_siginfo);
	printf("  ------------------------------------------------\n");
	printf("  %-12s %12llu %12llu   %s\n", "TOTAL", tc, tf,
	    tf == 0 ? "ok" : "FAILED");

	return (tf == 0 ? 0 : 1);
}
