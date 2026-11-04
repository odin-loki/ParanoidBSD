/*
 * Differential test harness for batch b0105s2 (strcspn).
 *
 * Compares the C++23 module port against the unmodified C oracle
 * (ref_strcspn) over hand-written edge cases and a fixed-seed random sweep.
 */

#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>

import pbsd.lib.libc.string.b0105s2;

extern "C" size_t ref_strcspn(const char *s, const char *charset);

namespace port = pbsd::lib_libc_string::b0105s2;

typedef std::vector<unsigned char> Bytes;

static const unsigned char GUARD = 0x7f;
static const size_t PADRUN = 48;   /* trap region right after the NUL */
static const size_t GUARDRUN = 48; /* untouched guard region at the end */

static long long g_cases = 0;
static long long g_fails = 0;
static int g_reported = 0;

/*
 * Effective first byte of a NUL-terminated view of `v`.  Used as the filler
 * for the trap region that follows the terminator of the *other* argument, so
 * that any port which walks past a terminator produces a different answer
 * instead of silently agreeing with the oracle.
 */
static unsigned char
first_or_guard(const Bytes &v)
{
	if (v.empty() || v[0] == 0)
		return (GUARD);
	return (v[0]);
}

static void
build(Bytes &buf, const Bytes &body, unsigned char pad)
{
	buf.assign(body.size() + 1 + PADRUN + GUARDRUN, GUARD);
	for (size_t i = 0; i < body.size(); i++)
		buf[i] = body[i];
	buf[body.size()] = 0;
	for (size_t i = 0; i < PADRUN; i++)
		buf[body.size() + 1 + i] = pad;
}

static void
dump(const char *label, const Bytes &v)
{
	std::printf("    %s (%zu):", label, v.size());
	size_t n = v.size() < 48 ? v.size() : 48;
	for (size_t i = 0; i < n; i++)
		std::printf(" %02x", v[i]);
	if (n < v.size())
		std::printf(" ...");
	std::printf("\n");
}

static void
check(const Bytes &s, const Bytes &cs, const char *tag)
{
	Bytes sa, sb, ca, cb;
	unsigned char s_pad = first_or_guard(cs);
	unsigned char c_pad = first_or_guard(s);

	build(sa, s, s_pad);
	build(sb, s, s_pad);
	build(ca, cs, c_pad);
	build(cb, cs, c_pad);

	size_t rp = port::strcspn((const char *)sa.data(),
	    (const char *)ca.data());
	size_t ro = ref_strcspn((const char *)sb.data(),
	    (const char *)cb.data());

	bool ok = (rp == ro) && (sa == sb) && (ca == cb);

	g_cases++;
	if (!ok) {
		g_fails++;
		if (g_reported < 12) {
			g_reported++;
			std::printf("  FAIL [%s] port=%zu ref=%zu "
			    "s_buf_same=%d charset_buf_same=%d\n", tag, rp, ro,
			    (int)(sa == sb), (int)(ca == cb));
			dump("s      ", s);
			dump("charset", cs);
		}
	}
}

static Bytes
bstr(const char *p)
{
	Bytes v;
	for (; *p != '\0'; p++)
		v.push_back((unsigned char)*p);
	return (v);
}

static Bytes
rep(unsigned char c, size_t n)
{
	return (Bytes(n, c));
}

/* ------------------------------------------------------------------ */
/* fixed-seed PRNG (splitmix64)						*/
/* ------------------------------------------------------------------ */
static uint64_t g_state = 0x9e3779b97f4a7c15ULL;

static uint64_t
rnd(void)
{
	uint64_t z = (g_state += 0x9e3779b97f4a7c15ULL);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static uint32_t
rnd_below(uint32_t n)
{
	return ((uint32_t)(rnd() % n));
}

/* ------------------------------------------------------------------ */
/* hand-written edge cases						*/
/* ------------------------------------------------------------------ */
static void
edge_cases(void)
{
	Bytes empty;

	/* empty / single-character basics */
	check(empty, empty, "empty/empty");
	check(empty, bstr("abc"), "empty/abc");
	check(empty, rep(0xff, 1), "empty/highbit");
	check(bstr("a"), empty, "a/empty");
	check(bstr("a"), bstr("a"), "a/a");
	check(bstr("a"), bstr("b"), "a/b");
	check(bstr("ab"), bstr("b"), "ab/b");
	check(bstr("ab"), bstr("a"), "ab/a");
	check(bstr("abc"), bstr("c"), "abc/c");
	check(bstr("abc"), bstr("cba"), "abc/cba");
	check(bstr("abc"), bstr("xyz"), "abc/xyz");
	check(bstr("hello world"), bstr(" "), "hello/space");
	check(bstr("hello world"), bstr("dlrow"), "hello/dlrow");

	/* NUL-heavy: embedded terminators truncate both operands */
	check(Bytes{0}, Bytes{0}, "nul/nul");
	check(Bytes{0}, bstr("a"), "nul/a");
	check(bstr("a"), Bytes{0}, "a/nul");
	check(Bytes{'a', 0, 'b'}, bstr("b"), "a.nul.b/b");
	check(Bytes{'a', 0, 'b'}, Bytes{'z', 0, 'a'}, "a.nul.b/z.nul.a");
	check(Bytes{0, 'a'}, Bytes{0, 'a'}, "nul.a/nul.a");
	check(rep(0, 8), rep(0, 8), "nul8/nul8");

	/* every byte value on its own, against every byte value */
	for (unsigned b = 1; b <= 255; b++) {
		for (unsigned c = 1; c <= 255; c++) {
			Bytes s{(unsigned char)b};
			Bytes cs{(unsigned char)c};
			check(s, cs, "byte-pair");
		}
	}

	/*
	 * Word-selector coverage: 0x00-0x3f -> tbl[0], 0x40-0x7f -> tbl[1],
	 * 0x80-0xbf -> tbl[2], 0xc0-0xff -> tbl[3].  Put one representative of
	 * every word ahead of the real match so that a stray bit in any table
	 * word changes the answer.
	 */
	for (unsigned b = 1; b <= 255; b++) {
		Bytes s{0x01, 0x3f, 0x40, 0x7f, 0x80, 0xbf, 0xc0, 0xff,
		    (unsigned char)b};
		Bytes cs{(unsigned char)b};
		check(s, cs, "wordsel-tail");
		Bytes s2{(unsigned char)b, 0x01, 0x40, 0x80, 0xc0};
		check(s2, cs, "wordsel-head");
	}

	/* bit-position coverage within each word: neighbours of every bit */
	for (unsigned b = 1; b <= 255; b++) {
		Bytes s;
		for (unsigned k = 1; k <= 255; k++)
			if (k != b)
				s.push_back((unsigned char)k);
		s.push_back((unsigned char)b);
		Bytes cs{(unsigned char)b};
		check(s, cs, "all-but-b");
	}

	/* ordered full alphabet: match index must equal c-1 exactly */
	{
		Bytes all;
		for (unsigned k = 1; k <= 255; k++)
			all.push_back((unsigned char)k);
		for (unsigned c = 1; c <= 255; c++) {
			Bytes cs{(unsigned char)c};
			check(all, cs, "all/one");
		}
		check(all, empty, "all/empty");
		check(all, all, "all/all");
		/* charset holding every byte except one */
		for (unsigned c = 1; c <= 255; c++) {
			Bytes cs;
			for (unsigned k = 1; k <= 255; k++)
				if (k != c)
					cs.push_back((unsigned char)k);
			check(Bytes{(unsigned char)c}, cs, "one/all-but-one");
			check(all, cs, "all/all-but-one");
		}
	}

	/* boundary lengths, match present and absent */
	{
		static const size_t lens[] = { 0, 1, 2, 3, 4, 5, 7, 8, 9, 15,
		    16, 17, 31, 32, 33, 62, 63, 64, 65, 66, 127, 128, 129, 254,
		    255, 256, 257, 511, 512, 513 };
		for (size_t i = 0; i < sizeof(lens) / sizeof(lens[0]); i++) {
			size_t L = lens[i];
			Bytes s = rep('a', L);
			s.push_back('z');
			check(s, bstr("z"), "len/hit-at-L");
			check(s, bstr("q"), "len/no-hit");
			check(s, empty, "len/empty-charset");
			check(rep('a', L), bstr("a"), "len/hit-at-0");
			check(rep('a', L), bstr("z"), "len/all-miss");
			Bytes hs = rep(0x80, L);
			hs.push_back(0xff);
			check(hs, Bytes{0xff}, "len/high-hit-at-L");
			check(hs, Bytes{0x01}, "len/high-no-hit");
			check(hs, empty, "len/high-empty-charset");
		}
	}

	/* high-bit-only strings and charsets */
	for (unsigned b = 0x80; b <= 0xff; b++) {
		Bytes s;
		for (unsigned k = 0x80; k <= 0xff; k++)
			s.push_back((unsigned char)k);
		check(s, Bytes{(unsigned char)b}, "highrange/one");
		Bytes cs;
		for (unsigned k = 0x80; k <= 0xff; k++)
			if (k != b)
				cs.push_back((unsigned char)k);
		check(s, cs, "highrange/all-but-one");
	}

	/* charset longer than the string, and vice versa */
	{
		Bytes s = bstr("The quick brown fox jumps over the lazy dog");
		check(s, bstr("gd"), "pangram/gd");
		check(s, bstr("T"), "pangram/T");
		check(s, bstr("z"), "pangram/z");
		check(s, bstr("."), "pangram/dot");
		Bytes big;
		for (unsigned k = 1; k <= 255; k++)
			if (k != 'T')
				big.push_back((unsigned char)k);
		check(s, big, "pangram/all-but-T");
	}
}

/* ------------------------------------------------------------------ */
/* randomised sweep							*/
/* ------------------------------------------------------------------ */
static void
random_sweep(long long iters)
{
	for (long long it = 0; it < iters; it++) {
		uint32_t mode = rnd_below(6);
		size_t slen, clen;

		switch (mode) {
		case 0:
			slen = rnd_below(5);
			clen = rnd_below(5);
			break;
		case 1:
			slen = rnd_below(17);
			clen = rnd_below(4);
			break;
		case 2:
			slen = rnd_below(65);
			clen = rnd_below(9);
			break;
		case 3:
			slen = rnd_below(9);
			clen = rnd_below(65);
			break;
		case 4:
			slen = rnd_below(130);
			clen = rnd_below(130);
			break;
		default:
			slen = rnd_below(300);
			clen = rnd_below(3);
			break;
		}

		Bytes s(slen), cs(clen);

		/* byte-value distribution: full range, sometimes narrowed so
		 * that collisions between s and charset are common */
		uint32_t dist = rnd_below(4);
		for (size_t i = 0; i < slen; i++) {
			switch (dist) {
			case 0: s[i] = (unsigned char)(1 + rnd_below(255)); break;
			case 1: s[i] = (unsigned char)(0x80 + rnd_below(128)); break;
			case 2: s[i] = (unsigned char)(1 + rnd_below(6)); break;
			default: s[i] = (unsigned char)rnd_below(256); break;
			}
		}
		for (size_t i = 0; i < clen; i++) {
			switch (dist) {
			case 0: cs[i] = (unsigned char)(1 + rnd_below(255)); break;
			case 1: cs[i] = (unsigned char)(0x80 + rnd_below(128)); break;
			case 2: cs[i] = (unsigned char)(1 + rnd_below(6)); break;
			default: cs[i] = (unsigned char)rnd_below(256); break;
			}
		}

		/*
		 * Half the time, seed the charset with a byte lifted out of s
		 * so that the scan terminates somewhere in the middle rather
		 * than always at the terminator.
		 */
		if (slen != 0 && clen != 0 && (rnd() & 1) != 0) {
			size_t si = rnd_below((uint32_t)slen);
			size_t ci = rnd_below((uint32_t)clen);
			if (s[si] != 0)
				cs[ci] = s[si];
		}

		check(s, cs, "random");
	}
}

int
main(void)
{
	edge_cases();
	random_sweep(200000);

	std::printf("\n");
	std::printf("%-16s %12s %12s %8s\n", "function", "cases", "failures",
	    "result");
	std::printf("%-16s %12lld %12lld %8s\n", "strcspn", g_cases, g_fails,
	    g_fails == 0 ? "PASS" : "FAIL");
	std::printf("\n");
	std::printf("total: %lld cases, %lld failures -> %s\n", g_cases,
	    g_fails, g_fails == 0 ? "PASS" : "FAIL");

	return (g_fails == 0 ? 0 : 1);
}
