/*
 * Differential test for PBSD batch b0105s1 (lib/libc/string/strspn.c).
 *
 * Every case is run through both the C++23 port and the unmodified C oracle
 * and the results are compared exactly.  Inputs live in guarded buffers so
 * that a port which walks outside the string, or writes to it, is caught as
 * well as one which merely returns the wrong number.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libc.string.b0105s1;

extern "C" size_t ref_strspn(const char *s, const char *charset);

namespace port = pbsd::lib_libc_string::b0105s1;

/* ------------------------------------------------------------------ */
/* Guarded buffers                                                     */
/* ------------------------------------------------------------------ */

static const size_t BUFSZ = 512;
static const size_t OFF = 64;		/* string starts here */
static const size_t MAXLEN = 320;	/* longest content we ever place */
static const unsigned char GUARD = 0x7f;

struct Buf {
	unsigned char b[BUFSZ];
	const char *p() const { return (const char *)(b + OFF); }
};

static void
fill(Buf &x, const unsigned char *data, size_t len)
{
	memset(x.b, GUARD, BUFSZ);
	/*
	 * A NUL immediately in front of the string keeps a port that walks
	 * backwards inside the buffer instead of off into the weeds; the
	 * divergence still shows up in the return value.
	 */
	x.b[OFF - 1] = 0;
	memcpy(x.b + OFF, data, len);
	x.b[OFF + len] = 0;
}

/* ------------------------------------------------------------------ */
/* Bookkeeping                                                         */
/* ------------------------------------------------------------------ */

static unsigned long long g_cases = 0;
static unsigned long long g_fails = 0;
static unsigned long long g_reported = 0;

static void
dump(const char *tag, const unsigned char *d, size_t len)
{
	fprintf(stderr, "    %s (len %zu):", tag, len);
	for (size_t i = 0; i < len && i < 48; i++)
		fprintf(stderr, " %02x", d[i]);
	if (len > 48)
		fprintf(stderr, " ...");
	fprintf(stderr, "\n");
}

static void
check(const unsigned char *s, size_t slen,
      const unsigned char *cs, size_t cslen, const char *origin)
{
	Buf sa, sb, ca, cb, sp, cp;

	if (slen > MAXLEN || cslen > MAXLEN) {
		fprintf(stderr, "harness bug: input too long\n");
		g_fails++;
		return;
	}

	fill(sa, s, slen);
	fill(sb, s, slen);
	fill(ca, cs, cslen);
	fill(cb, cs, cslen);
	fill(sp, s, slen);	/* pristine copies */
	fill(cp, cs, cslen);

	size_t rref = ref_strspn(sa.p(), ca.p());
	size_t rport = port::strspn(sb.p(), cb.p());

	g_cases++;

	const char *why = nullptr;
	if (rref != rport)
		why = "return value";
	else if (memcmp(sa.b, sb.b, BUFSZ) != 0)
		why = "subject buffer differs between oracle and port";
	else if (memcmp(ca.b, cb.b, BUFSZ) != 0)
		why = "charset buffer differs between oracle and port";
	else if (memcmp(sp.b, sb.b, BUFSZ) != 0)
		why = "port modified the subject buffer";
	else if (memcmp(cp.b, cb.b, BUFSZ) != 0)
		why = "port modified the charset buffer";
	else if (memcmp(sp.b, sa.b, BUFSZ) != 0)
		why = "oracle modified the subject buffer";
	else if (memcmp(cp.b, ca.b, BUFSZ) != 0)
		why = "oracle modified the charset buffer";

	if (why == nullptr)
		return;

	g_fails++;
	if (g_reported < 20) {
		g_reported++;
		fprintf(stderr, "FAIL strspn [%s]: %s\n", origin, why);
		fprintf(stderr, "    ref = %zu  port = %zu\n", rref, rport);
		dump("s      ", s, slen);
		dump("charset", cs, cslen);
	}
}

static void
check_str(const char *s, const char *cs, const char *origin)
{
	check((const unsigned char *)s, strlen(s),
	      (const unsigned char *)cs, strlen(cs), origin);
}

/* ------------------------------------------------------------------ */
/* Deterministic PRNG (xorshift64*, fixed seed)                        */
/* ------------------------------------------------------------------ */

static uint64_t rng_state = 0x0123456789abcdefULL;

static uint64_t
rnd(void)
{
	uint64_t x = rng_state;
	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rng_state = x;
	return x * 0x2545f4914f6cdd1dULL;
}

static uint32_t
rnd_below(uint32_t n)
{
	return (uint32_t)(rnd() % n);
}

/*
 * Bytes that sit on the boundaries of the 64-bit bitmap words, of the signed
 * char range, and of the byte range itself.
 */
static const unsigned char interesting[] = {
	0x01, 0x02, 0x1f, 0x20, 0x3e, 0x3f, 0x40, 0x41, 0x42,
	0x5f, 0x60, 0x7e, 0x7f, 0x80, 0x81, 0x82,
	0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xfd, 0xfe, 0xff,
	'a', 'b', 'A', 'z', '0', '9'
};

static unsigned char
rnd_byte(void)
{
	if (rnd() & 1)
		return interesting[rnd_below((uint32_t)(sizeof(interesting)))];
	return (unsigned char)(1 + rnd_below(255));	/* never NUL */
}

/* ------------------------------------------------------------------ */
/* Hand-written edge cases                                             */
/* ------------------------------------------------------------------ */

static void
edge_cases(void)
{
	unsigned char s[MAXLEN + 1];
	unsigned char cs[MAXLEN + 1];

	/* Empty subject and/or empty charset. */
	check_str("", "", "empty/empty");
	check_str("", "abc", "empty/nonempty");
	check_str("", "\x80\xff", "empty/high");
	check_str("a", "", "single/empty");
	check_str("\xff", "", "high/empty");

	/* Single characters, in and out of the set. */
	check_str("a", "a", "single hit");
	check_str("a", "b", "single miss");
	check_str("a", "ba", "single hit, second position");
	check_str("b", "ab", "single hit, second position");

	/* Mismatch at each position of a short run. */
	check_str("ab", "a", "prefix 1");
	check_str("aab", "a", "prefix 2");
	check_str("aaab", "a", "prefix 3");
	check_str("baaa", "a", "prefix 0");
	check_str("aaaa", "a", "whole string");

	/* Duplicated charset entries. */
	check_str("aaab", "aaaaaa", "duplicate charset");
	check_str("abcabc", "cba", "permuted charset");
	check_str("abcabcd", "abc", "permuted charset, miss at end");

	/*
	 * Every byte 1..255 as the sole member of the set, against a subject
	 * of that byte followed by every other byte.  This drives IDX() and
	 * BIT() over the whole table and over both sides of every word
	 * boundary.
	 */
	for (unsigned c = 1; c <= 255; c++) {
		for (unsigned d = 1; d <= 255; d++) {
			s[0] = (unsigned char)c;
			s[1] = (unsigned char)d;
			cs[0] = (unsigned char)c;
			check(s, 2, cs, 1, "pairwise c/d");
		}
	}

	/* Every byte against a set built from all the others. */
	for (unsigned c = 1; c <= 255; c++) {
		size_t n = 0;
		for (unsigned d = 1; d <= 255; d++)
			if (d != c)
				cs[n++] = (unsigned char)d;
		s[0] = (unsigned char)c;
		s[1] = (unsigned char)c;
		check(s, 2, cs, n, "byte vs complement");

		/* And the same set with the byte added back. */
		cs[n++] = (unsigned char)c;
		check(s, 2, cs, n, "byte vs full set");
	}

	/* Full alphabet subject, full alphabet set: span is the whole string. */
	{
		size_t n = 0;
		for (unsigned c = 1; c <= 255; c++)
			s[n++] = (unsigned char)c;
		memcpy(cs, s, n);
		check(s, n, cs, n, "all bytes / all bytes");

		/* Remove one set member at a time: span must stop there. */
		for (unsigned drop = 1; drop <= 255; drop++) {
			size_t m = 0;
			for (unsigned c = 1; c <= 255; c++)
				if (c != drop)
					cs[m++] = (unsigned char)c;
			check(s, n, cs, m, "all bytes / one dropped");
		}
	}

	/* Boundary bytes of every bitmap word, on both sides. */
	{
		static const unsigned char bnd[] = {
			0x01, 0x3f, 0x40, 0x41, 0x7f, 0x80, 0x81,
			0xbf, 0xc0, 0xc1, 0xff
		};
		for (size_t i = 0; i < sizeof(bnd); i++) {
			for (size_t j = 0; j < sizeof(bnd); j++) {
				s[0] = bnd[i];
				s[1] = bnd[j];
				s[2] = bnd[i];
				cs[0] = bnd[i];
				cs[1] = bnd[j];
				check(s, 3, cs, 1, "boundary, one member");
				check(s, 3, cs, 2, "boundary, two members");
			}
		}
	}

	/* High-bit only subjects and sets. */
	{
		size_t n = 0;
		for (unsigned c = 0x80; c <= 0xff; c++)
			s[n++] = (unsigned char)c;
		size_t m = 0;
		for (unsigned c = 0x80; c <= 0xff; c++)
			cs[m++] = (unsigned char)c;
		check(s, n, cs, m, "high half / high half");
		check(s, n, cs, m - 1, "high half / high half less 0xff");
		cs[0] = 0x41;
		check(s, n, cs, m, "high half / 0x41 substituted");
	}

	/* Boundary lengths: a run of length L followed by a non-member. */
	for (size_t len = 0; len <= 64; len++) {
		for (size_t i = 0; i < len; i++)
			s[i] = 'a';
		s[len] = 'b';
		s[len + 1] = 'a';
		s[len + 2] = 'a';
		cs[0] = 'a';
		check(s, len + 3, cs, 1, "run length sweep");
		/* Same run with no terminator inside the string at all. */
		check(s, len, cs, 1, "exact run length");
	}

	/* NUL-heavy: the subject stops at the first NUL, whatever follows. */
	{
		unsigned char raw[16];
		memset(raw, 'a', sizeof(raw));
		raw[0] = 'a'; raw[1] = 'a'; raw[2] = 0; raw[3] = 'a';
		raw[4] = 'a'; raw[5] = 0; raw[6] = 'b';
		/* fill() appends its own NUL; embedded ones are inside. */
		check(raw, sizeof(raw), (const unsigned char *)"a", 1,
		    "embedded NUL in subject");
		check(raw, sizeof(raw), (const unsigned char *)"ab", 2,
		    "embedded NUL in subject, wider set");

		unsigned char rawcs[8];
		rawcs[0] = 'a'; rawcs[1] = 0; rawcs[2] = 'b'; rawcs[3] = 'c';
		rawcs[4] = 0; rawcs[5] = 'd'; rawcs[6] = 'e'; rawcs[7] = 'f';
		check((const unsigned char *)"abcabc", 6, rawcs, sizeof(rawcs),
		    "embedded NUL in charset");
		check((const unsigned char *)"aaab", 4, rawcs, sizeof(rawcs),
		    "embedded NUL in charset, hit");
	}

	/* Longest strings we handle, entirely inside the set. */
	{
		for (size_t i = 0; i < MAXLEN - 1; i++)
			s[i] = (unsigned char)(1 + (i % 255));
		size_t n = 0;
		for (unsigned c = 1; c <= 255; c++)
			cs[n++] = (unsigned char)c;
		check(s, MAXLEN - 1, cs, n, "max length, full set");
		s[MAXLEN - 2] = 0x00;	/* becomes an early terminator */
		check(s, MAXLEN - 1, cs, n, "max length, early NUL");
	}
}

/* ------------------------------------------------------------------ */
/* Randomised sweep                                                    */
/* ------------------------------------------------------------------ */

static void
random_sweep(unsigned long iters)
{
	unsigned char s[MAXLEN + 1];
	unsigned char cs[MAXLEN + 1];
	unsigned char alpha[16];

	for (unsigned long it = 0; it < iters; it++) {
		/* A small alphabet keeps spans long enough to be interesting. */
		uint32_t na = 1 + rnd_below(8);
		for (uint32_t i = 0; i < na; i++)
			alpha[i] = rnd_byte();

		uint32_t slen = rnd_below(41);
		for (uint32_t i = 0; i < slen; i++) {
			if (rnd_below(8) != 0)
				s[i] = alpha[rnd_below(na)];
			else
				s[i] = rnd_byte();
		}

		/* Charset: the alphabet, shuffled, sometimes short, sometimes
		 * padded with unrelated bytes. */
		uint32_t take = na;
		uint32_t mode = rnd_below(4);
		if (mode == 0 && na > 1)
			take = na - 1;		/* drop a member */
		else if (mode == 1)
			take = 0;		/* empty set */

		unsigned char tmp[16];
		memcpy(tmp, alpha, na);
		for (uint32_t i = na; i > 1; i--) {
			uint32_t j = rnd_below(i);
			unsigned char t = tmp[i - 1];
			tmp[i - 1] = tmp[j];
			tmp[j] = t;
		}

		uint32_t cslen = 0;
		for (uint32_t i = 0; i < take; i++)
			cs[cslen++] = tmp[i];
		uint32_t extra = rnd_below(6);
		for (uint32_t i = 0; i < extra; i++)
			cs[cslen++] = rnd_byte();

		check(s, slen, cs, cslen, "random");
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	edge_cases();
	random_sweep(250000UL);

	printf("%-12s %12s %12s\n", "function", "cases", "failures");
	printf("%-12s %12llu %12llu\n", "strspn", g_cases, g_fails);

	if (g_fails != 0) {
		printf("\nFAILED: %llu of %llu cases diverged\n",
		    g_fails, g_cases);
		return 1;
	}
	printf("\nOK: all %llu cases matched\n", g_cases);
	return 0;
}
