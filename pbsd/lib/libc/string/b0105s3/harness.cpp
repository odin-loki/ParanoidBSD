// b0105s3 differential test harness.
//
// Every case is run through both the C++23 port (module
// pbsd.lib.libc.string.b0105s3) and the untouched C oracle (ref_* in oracle.c),
// and every observable is compared: return values, pointer offsets relative to
// the buffer base, and the FULL contents of every buffer handed to the
// function -- including the bytes past the nominal write window and the
// nominally read-only inputs.

#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <unistd.h>

import pbsd.lib.libc.string.b0105s3;

namespace P = pbsd::lib_libc_string::b0105s3;

extern "C" {
char *ref_strsep(char **stringp, const char *delim);
}

using std::size_t;
using std::uint32_t;
using std::uint64_t;

/* ------------------------------------------------------------------ */
/* bookkeeping                                                        */
/* ------------------------------------------------------------------ */

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	int shown;
};

static Stat st_strsep = { "strsep", 0, 0, 0 };

static const int MAX_SHOW = 8;

static bool
begin_fail(Stat &st, const char *what)
{
	st.fails++;
	if (st.shown >= MAX_SHOW)
		return false;
	st.shown++;
	std::printf("FAIL %s: %s\n", st.name, what);
	return true;
}

static void
dump_bytes(const char *label, const unsigned char *p, size_t n)
{
	std::printf("    %s[%zu] =", label, n);
	for (size_t i = 0; i < n; i++)
		std::printf(" %02x", (unsigned)p[i]);
	std::printf("\n");
}

/* index of the first differing byte between two equally sized buffers, or -1 */
static long
first_diff(const void *a, const void *b, size_t n)
{
	const unsigned char *pa = (const unsigned char *)a;
	const unsigned char *pb = (const unsigned char *)b;

	for (size_t i = 0; i < n; i++)
		if (pa[i] != pb[i])
			return (long)i;
	return -1;
}

/* ------------------------------------------------------------------ */
/* deterministic RNG (splitmix64, fixed seed)                         */
/* ------------------------------------------------------------------ */

struct Rng {
	uint64_t s;

	explicit Rng(uint64_t seed) : s(seed) {}

	uint64_t next()
	{
		s += 0x9E3779B97F4A7C15ull;
		uint64_t z = s;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}
	uint32_t below(uint32_t n) { return (uint32_t)(next() % n); }
	bool pct(uint32_t p) { return below(100) < p; }
};

/* ------------------------------------------------------------------ */
/* byte buffers                                                       */
/* ------------------------------------------------------------------ */

static const unsigned char GUARD = 0x7f;

static const size_t SBUF = 128;		/* char buffer capacity */
static const size_t MAXS = 48;		/* longest generated string */

/*
 * Fill an entire buffer with the guard byte, then lay a NUL-terminated string
 * at its base.  Everything from buf[len+1] on stays 0x7f, so a port that walks
 * off the end of the string reads a well-defined byte, and a port that writes
 * off the end is caught by the whole-buffer comparison.
 */
static void
fill_str(char *buf, size_t cap, const unsigned char *body, size_t len)
{
	std::memset(buf, GUARD, cap);
	for (size_t i = 0; i < len; i++)
		buf[i] = (char)body[i];
	buf[len] = '\0';
}

/* ------------------------------------------------------------------ */
/* strsep                                                             */
/* ------------------------------------------------------------------ */

static void
t_strsep(const unsigned char *body, size_t blen, const unsigned char *dl,
    size_t dlen)
{
	char a[SBUF], b[SBUF], da[SBUF], db[SBUF];

	fill_str(a, SBUF, body, blen);
	fill_str(b, SBUF, body, blen);
	fill_str(da, SBUF, dl, dlen);
	fill_str(db, SBUF, dl, dlen);

	char *pa = a;
	char *pb = b;
	size_t limit = blen + 8;	/* a correct run yields at most blen+1 tokens */
	bool done = false;

	for (size_t it = 0; it <= limit; it++) {
		char *ta = P::strsep(&pa, da);
		char *tb = ref_strsep(&pb, db);

		/* offsets only -- never raw addresses */
		long toka = (ta == nullptr) ? -1 : (long)(ta - a);
		long tokb = (tb == nullptr) ? -1 : (long)(tb - b);
		long sta = (pa == nullptr) ? -1 : (long)(pa - a);
		long stb = (pb == nullptr) ? -1 : (long)(pb - b);

		st_strsep.cases++;
		if (toka != tokb || sta != stb ||
		    std::memcmp(a, b, SBUF) != 0 ||
		    std::memcmp(da, db, SBUF) != 0) {
			if (begin_fail(st_strsep, "mismatch")) {
				dump_bytes("s", body, blen);
				dump_bytes("delim", dl, dlen);
				std::printf("    iter=%zu tok port=%ld ref=%ld "
				    "state port=%ld ref=%ld buf_diff@%ld "
				    "delim_diff@%ld\n", it, toka, tokb, sta, stb,
				    first_diff(a, b, SBUF),
				    first_diff(da, db, SBUF));
			}
			return;
		}
		if (toka < 0) {
			done = true;
			break;
		}
	}

	if (!done) {
		st_strsep.cases++;
		if (begin_fail(st_strsep, "iteration limit exceeded")) {
			dump_bytes("s", body, blen);
			dump_bytes("delim", dl, dlen);
		}
	}
}

static void
t_strsep_null(const unsigned char *dl, size_t dlen)
{
	char da[SBUF], db[SBUF];

	fill_str(da, SBUF, dl, dlen);
	fill_str(db, SBUF, dl, dlen);

	char *pa = nullptr;
	char *pb = nullptr;
	char *ta = P::strsep(&pa, da);
	char *tb = ref_strsep(&pb, db);

	st_strsep.cases++;
	if (ta != nullptr || tb != nullptr || pa != nullptr || pb != nullptr ||
	    std::memcmp(da, db, SBUF) != 0) {
		if (begin_fail(st_strsep, "NULL *stringp")) {
			dump_bytes("delim", dl, dlen);
			std::printf("    tok_null port=%d ref=%d state_null "
			    "port=%d ref=%d delim_diff@%ld\n", ta == nullptr,
			    tb == nullptr, pa == nullptr, pb == nullptr,
			    first_diff(da, db, SBUF));
		}
	}
}

/* ------------------------------------------------------------------ */
/* hand-written cases                                                 */
/* ------------------------------------------------------------------ */

#define BYTES(lit) (const unsigned char *)(lit), (sizeof(lit) - 1)

static void
hand_strsep(void)
{
	/* *stringp == NULL */
	t_strsep_null(BYTES(","));
	t_strsep_null(BYTES(""));

	/* empty inputs */
	t_strsep(BYTES(""), BYTES(""));
	t_strsep(BYTES(""), BYTES(","));
	t_strsep(BYTES("a"), BYTES(""));
	t_strsep(BYTES("abc"), BYTES(""));

	/* single separator, every position */
	t_strsep(BYTES("a,b"), BYTES(","));
	t_strsep(BYTES(",a"), BYTES(","));
	t_strsep(BYTES("a,"), BYTES(","));
	t_strsep(BYTES(","), BYTES(","));
	t_strsep(BYTES(",,"), BYTES(","));
	t_strsep(BYTES(",,,"), BYTES(","));
	t_strsep(BYTES("a,b,c,d"), BYTES(","));
	t_strsep(BYTES("a,,b"), BYTES(","));

	/* no separator present at all: one token, then NULL */
	t_strsep(BYTES("abc"), BYTES(","));
	t_strsep(BYTES("abc"), BYTES("xyz"));

	/* a delimiter set containing the guard byte: walking past the
	 * terminating NUL changes the token boundaries observably */
	t_strsep(BYTES("abc"), BYTES("\x7f"));
	t_strsep(BYTES("a,b"), BYTES(",\x7f"));

	/* multi-character delimiter sets: the match has to be found at
	 * delim[1], delim[2], ... not only at delim[0] */
	t_strsep(BYTES("a;b"), BYTES(",;"));
	t_strsep(BYTES("a;b"), BYTES("xy;"));
	t_strsep(BYTES("a;b,c"), BYTES(",;"));
	t_strsep(BYTES("a;b,c"), BYTES(";,"));
	t_strsep(BYTES("a.b:c;d,e"), BYTES(",;:."));
	t_strsep(BYTES("abcdefg"), BYTES("uvwxyz"));

	/* high-bit delimiters: c and sc are int-promoted plain chars, so the
	 * sign extension has to match on both sides */
	t_strsep(BYTES("a\x80" "b"), BYTES("\x80"));
	t_strsep(BYTES("a\xff" "b"), BYTES("\xff"));
	t_strsep(BYTES("a\xff" "b"), BYTES("\x7f\xff"));
	t_strsep(BYTES("\x80\xff\xc0"), BYTES("\x80\xc0"));
	t_strsep(BYTES("\xff\xff\xff"), BYTES("\xff"));
	t_strsep(BYTES("a\xc0" "b\xbf" "c"), BYTES("\xc0\xbf"));
	t_strsep(BYTES("abc"), BYTES("\x80\x81\x82"));

	/* every byte a separator, and long runs */
	t_strsep(BYTES("aaaaaaaa"), BYTES("a"));
	t_strsep(BYTES("aaaaaaaab"), BYTES("a"));
	t_strsep(BYTES("baaaaaaaa"), BYTES("a"));
	t_strsep(BYTES("a,b;c,d;e,f;g,h"), BYTES(",;"));

	/* embedded NUL terminates the buffer early */
	t_strsep(BYTES("ab\0cd"), BYTES(","));
	t_strsep(BYTES("a,b\0c,d"), BYTES(","));

	/* NUL as delimiter: exercises c == 0 branch and s = NULL path */
	t_strsep(BYTES("abc"), BYTES("\0"));
	t_strsep(BYTES("a\0b"), BYTES("\0"));
	t_strsep(BYTES("\0a"), BYTES("\0"));
	t_strsep(BYTES("a\0\0b"), BYTES("\0"));

	/* single-char string, every delimiter variant */
	t_strsep(BYTES("a"), BYTES("a"));
	t_strsep(BYTES("a"), BYTES("b"));
	t_strsep(BYTES("a"), BYTES("\xff"));
	t_strsep(BYTES("\xff"), BYTES("\xff"));

	/* boundary lengths near SBUF guard region */
	{
		unsigned char longs[MAXS];
		unsigned char longd[8];
		for (size_t i = 0; i < MAXS; i++)
			longs[i] = (unsigned char)('a' + (i % 26));
		for (size_t i = 0; i < 8; i++)
			longd[i] = (unsigned char)(i % 2 ? ',' : ';');
		for (size_t n = MAXS - 4; n <= MAXS; n++)
			t_strsep(longs, n, longd, 2);
	}
}

/* ------------------------------------------------------------------ */
/* randomised sweeps (fixed seeds)                                    */
/* ------------------------------------------------------------------ */

static const unsigned char AL_SEP[] = {
	'a', 'b', 'c', ',', ';', 0x7f, 0x80, 0xc0, 0xff
};

static unsigned char
gen_byte(Rng &r, int mode)
{
	switch (mode) {
	case 0:
		return (unsigned char)(1 + r.below(255));	/* 0x01..0xff */
	case 1:
		return (unsigned char)(0x80 + r.below(0x80));	/* high bit set */
	default:
		return AL_SEP[r.below((uint32_t)sizeof AL_SEP)];
	}
}

static void
sweep_strsep(void)
{
	Rng r(0x5345505F4231ull);
	unsigned char s[MAXS + 1], dl[MAXS + 1];

	for (int iter = 0; iter < 200000; iter++) {
		int mode = (iter % 5 == 0) ? 0 : 2;
		size_t dlen = r.below(5);		/* 0..4 */
		for (size_t i = 0; i < dlen; i++)
			dl[i] = gen_byte(r, mode);

		size_t slen = r.below(21);		/* 0..20 */
		for (size_t i = 0; i < slen; i++) {
			if (dlen != 0 && r.pct(40))
				s[i] = dl[r.below((uint32_t)dlen)];
			else
				s[i] = gen_byte(r, mode);
		}
		t_strsep(s, slen, dl, dlen);
		if ((iter & 0x3ff) == 0)
			t_strsep_null(dl, dlen);
	}
}

/* ------------------------------------------------------------------ */

/*
 * A broken port can scan off the end of a buffer, which either faults or never
 * stops.  Both are failures, but a raw crash or a hang is a lousy way to report
 * one, so turn them into a diagnosed exit(1).  Async-signal-safe calls only.
 */
extern "C" void
bail(int sig)
{
	static const char msg[] =
	    "\nFAIL b0105s3: harness aborted -- the port ran away "
	    "(runaway loop or out-of-bounds access)\n";

	(void)sig;
	ssize_t n = ::write(2, msg, sizeof msg - 1);
	(void)n;
	::_exit(1);
}

int
main(void)
{
	std::signal(SIGALRM, bail);
	std::signal(SIGSEGV, bail);
	std::signal(SIGBUS, bail);
	alarm(60);			/* a clean run takes well under a second */

	hand_strsep();
	sweep_strsep();

	const Stat *all[] = { &st_strsep };
	unsigned long long tc = 0, tf = 0;

	std::printf("\n");
	std::printf("+-----------+--------------+--------------+--------+\n");
	std::printf("| %-9s | %12s | %12s | %-6s |\n", "function", "cases",
	    "failures", "result");
	std::printf("+-----------+--------------+--------------+--------+\n");
	for (size_t i = 0; i < sizeof all / sizeof all[0]; i++) {
		std::printf("| %-9s | %12llu | %12llu | %-6s |\n", all[i]->name,
		    all[i]->cases, all[i]->fails,
		    all[i]->fails == 0 ? "PASS" : "FAIL");
		tc += all[i]->cases;
		tf += all[i]->fails;
	}
	std::printf("+-----------+--------------+--------------+--------+\n");
	std::printf("| %-9s | %12llu | %12llu | %-6s |\n", "TOTAL", tc, tf,
	    tf == 0 ? "PASS" : "FAIL");
	std::printf("+-----------+--------------+--------------+--------+\n");

	return tf == 0 ? 0 : 1;
}
