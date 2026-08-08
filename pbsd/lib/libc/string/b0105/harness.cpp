// b0105 differential test harness.
//
// Every case is run through both the C++23 port (module
// pbsd.lib.libc.string.b0105) and the untouched C oracle (ref_* in oracle.c),
// and every observable is compared: return values, pointer offsets relative to
// the buffer base, and the FULL contents of every buffer handed to the
// function -- including the bytes past the nominal write window and the
// nominally read-only inputs.

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>

import pbsd.lib.libc.string.b0105;

namespace P = pbsd::lib_libc_string::b0105;

extern "C" {
std::size_t ref_strspn(const char *s, const char *charset);
std::size_t ref_strcspn(const char *s, const char *charset);
char *ref_strsep(char **stringp, const char *delim);
std::size_t ref_wcslcpy(wchar_t *dst, const wchar_t *src, std::size_t siz);
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

static Stat st_strspn  = { "strspn",  0, 0, 0 };
static Stat st_strcspn = { "strcspn", 0, 0, 0 };
static Stat st_strsep  = { "strsep",  0, 0, 0 };
static Stat st_wcslcpy = { "wcslcpy", 0, 0, 0 };

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

static void
dump_wide(const char *label, const wchar_t *p, size_t n)
{
	std::printf("    %s[%zu] =", label, n);
	for (size_t i = 0; i < n; i++)
		std::printf(" %08lx", (unsigned long)(std::uint32_t)p[i]);
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
/* strspn / strcspn                                                   */
/* ------------------------------------------------------------------ */

static void
t_strspn(const unsigned char *body, size_t blen, const unsigned char *cs,
    size_t clen)
{
	char sa[SBUF], sb[SBUF], ca[SBUF], cb[SBUF];

	fill_str(sa, SBUF, body, blen);
	fill_str(sb, SBUF, body, blen);
	fill_str(ca, SBUF, cs, clen);
	fill_str(cb, SBUF, cs, clen);

	size_t rp = P::strspn(sa, ca);
	size_t ro = ref_strspn(sb, cb);

	st_strspn.cases++;
	if (rp != ro || std::memcmp(sa, sb, SBUF) != 0 ||
	    std::memcmp(ca, cb, SBUF) != 0) {
		if (begin_fail(st_strspn, "mismatch")) {
			dump_bytes("s", body, blen);
			dump_bytes("charset", cs, clen);
			std::printf("    port=%zu ref=%zu s_diff@%ld charset_diff@%ld\n",
			    rp, ro, first_diff(sa, sb, SBUF),
			    first_diff(ca, cb, SBUF));
		}
	}
}

static void
t_strcspn(const unsigned char *body, size_t blen, const unsigned char *cs,
    size_t clen)
{
	char sa[SBUF], sb[SBUF], ca[SBUF], cb[SBUF];

	fill_str(sa, SBUF, body, blen);
	fill_str(sb, SBUF, body, blen);
	fill_str(ca, SBUF, cs, clen);
	fill_str(cb, SBUF, cs, clen);

	size_t rp = P::strcspn(sa, ca);
	size_t ro = ref_strcspn(sb, cb);

	st_strcspn.cases++;
	if (rp != ro || std::memcmp(sa, sb, SBUF) != 0 ||
	    std::memcmp(ca, cb, SBUF) != 0) {
		if (begin_fail(st_strcspn, "mismatch")) {
			dump_bytes("s", body, blen);
			dump_bytes("charset", cs, clen);
			std::printf("    port=%zu ref=%zu s_diff@%ld charset_diff@%ld\n",
			    rp, ro, first_diff(sa, sb, SBUF),
			    first_diff(ca, cb, SBUF));
		}
	}
}

static void
t_span_both(const unsigned char *body, size_t blen, const unsigned char *cs,
    size_t clen)
{
	t_strspn(body, blen, cs, clen);
	t_strcspn(body, blen, cs, clen);
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
/* wcslcpy                                                            */
/* ------------------------------------------------------------------ */

static const size_t WDCAP = 96;		/* dst capacity, wchar_t units */
static const size_t WSCAP = 64;		/* src capacity, wchar_t units */
static const size_t MAXW = 40;		/* longest generated src */

static void
t_wcslcpy(const wchar_t *body, size_t blen, size_t siz)
{
	wchar_t da[WDCAP], db[WDCAP], sa[WSCAP], sb[WSCAP];

	std::memset(da, GUARD, sizeof da);
	std::memset(db, GUARD, sizeof db);
	std::memset(sa, GUARD, sizeof sa);
	std::memset(sb, GUARD, sizeof sb);
	for (size_t i = 0; i < blen; i++) {
		sa[i] = body[i];
		sb[i] = body[i];
	}
	sa[blen] = 0;
	sb[blen] = 0;

	size_t rp = P::wcslcpy(da, sa, siz);
	size_t ro = ref_wcslcpy(db, sb, siz);

	st_wcslcpy.cases++;
	if (rp != ro || std::memcmp(da, db, sizeof da) != 0 ||
	    std::memcmp(sa, sb, sizeof sa) != 0) {
		if (begin_fail(st_wcslcpy, "mismatch")) {
			dump_wide("src", body, blen);
			std::printf("    siz=%zu port=%zu ref=%zu dst_diff@%ld "
			    "src_diff@%ld\n", siz, rp, ro,
			    first_diff(da, db, sizeof da),
			    first_diff(sa, sb, sizeof sa));
			size_t show = blen + 4 < WDCAP ? blen + 4 : WDCAP;
			dump_wide("dst_port", da, show);
			dump_wide("dst_ref ", db, show);
		}
	}
}

/* sweep every interesting size for one src */
static void
t_wcslcpy_sizes(const wchar_t *body, size_t blen)
{
	t_wcslcpy(body, blen, 0);
	t_wcslcpy(body, blen, 1);
	t_wcslcpy(body, blen, 2);
	if (blen >= 1)
		t_wcslcpy(body, blen, blen - 1);
	t_wcslcpy(body, blen, blen);
	t_wcslcpy(body, blen, blen + 1);
	t_wcslcpy(body, blen, blen + 2);
	t_wcslcpy(body, blen, MAXW + 4);
}

/* ------------------------------------------------------------------ */
/* hand-written cases                                                 */
/* ------------------------------------------------------------------ */

#define BYTES(lit) (const unsigned char *)(lit), (sizeof(lit) - 1)

static void
hand_span(void)
{
	/* empty string / empty charset -- both early-return paths */
	t_span_both(BYTES(""), BYTES(""));
	t_span_both(BYTES(""), BYTES("abc"));
	t_span_both(BYTES("a"), BYTES(""));
	t_span_both(BYTES("abc"), BYTES(""));

	/* single character, both sides of the membership test */
	t_span_both(BYTES("a"), BYTES("a"));
	t_span_both(BYTES("a"), BYTES("b"));
	t_span_both(BYTES("b"), BYTES("a"));

	/* whole string in the set: forces the scan to stop on the NUL */
	t_span_both(BYTES("aaa"), BYTES("a"));
	t_span_both(BYTES("abc"), BYTES("cba"));
	t_span_both(BYTES("abcd"), BYTES("cba"));
	t_span_both(BYTES("abcdefghijklmnopqrstuvwxyz"),
	    BYTES("zyxwvutsrqponmlkjihgfedcba"));

	/* a charset containing the guard byte makes any walk past the
	 * terminating NUL observable in the return value */
	t_span_both(BYTES("abc"), BYTES("\x7f"));
	t_span_both(BYTES("abc"), BYTES("\x7f" "x"));
	t_span_both(BYTES("\x7f\x7f"), BYTES("\x7f"));
	t_span_both(BYTES("\x7f\x7f" "a"), BYTES("\x7f"));

	/* LONG_BIT lane boundaries: 0x3f|0x40 (tbl[0]|tbl[1]),
	 * 0x7f|0x80 (tbl[1]|tbl[2]), 0xbf|0xc0 (tbl[2]|tbl[3]) */
	t_span_both(BYTES("\x01"), BYTES("\x01"));
	t_span_both(BYTES("\x3f"), BYTES("\x3f"));
	t_span_both(BYTES("\x40"), BYTES("\x40"));
	t_span_both(BYTES("\x3f"), BYTES("\x40"));
	t_span_both(BYTES("\x40"), BYTES("\x3f"));
	t_span_both(BYTES("\x7f"), BYTES("\x80"));
	t_span_both(BYTES("\x80"), BYTES("\x7f"));
	t_span_both(BYTES("\x80"), BYTES("\x80"));
	t_span_both(BYTES("\xbf"), BYTES("\xc0"));
	t_span_both(BYTES("\xc0"), BYTES("\xbf"));
	t_span_both(BYTES("\xc0"), BYTES("\xc0"));
	t_span_both(BYTES("\xff"), BYTES("\xff"));
	t_span_both(BYTES("\xfe"), BYTES("\xff"));
	t_span_both(BYTES("\xff"), BYTES("\xfe"));

	/* the bytes whose bit is bit 0 of each table lane: a bogus lane
	 * initialiser shows up here and nowhere else */
	t_span_both(BYTES("\x40\x80\xc0"), BYTES("\x40\x80\xc0"));
	t_span_both(BYTES("\x40\x80\xc0"), BYTES("x"));
	t_span_both(BYTES("@"), BYTES("x"));
	t_span_both(BYTES("@"), BYTES("\x7f"));
	t_span_both(BYTES("\x80"), BYTES("x"));
	t_span_both(BYTES("\xc0"), BYTES("x"));
	t_span_both(BYTES("@@@"), BYTES("@"));

	/* high-bit bytes across the whole 0x80..0xff range */
	t_span_both(BYTES("\x80\x81\x82\x83\xfd\xfe\xff"),
	    BYTES("\xff\xfe\xfd\x83\x82\x81\x80"));
	t_span_both(BYTES("\x80\x81\x82\x83\xfd\xfe\xff"), BYTES("\x80\x81"));
	t_span_both(BYTES("\x80\x81\x82\x83\xfd\xfe\xff"), BYTES("\xff"));

	/* match at the front, the middle and the end of a longer string */
	t_span_both(BYTES("aaaaaaaab"), BYTES("a"));
	t_span_both(BYTES("baaaaaaaa"), BYTES("a"));
	t_span_both(BYTES("aaaabaaaa"), BYTES("a"));
	t_span_both(BYTES("xxxxxxxxa"), BYTES("a"));
	t_span_both(BYTES("axxxxxxxx"), BYTES("a"));
	t_span_both(BYTES("xxxxaxxxx"), BYTES("a"));

	/* embedded NUL: only the leading substring is visible */
	t_span_both(BYTES("ab\0cd"), BYTES("abcd"));
	t_span_both(BYTES("ab\0cd"), BYTES("cd"));
}

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
}

static void
hand_wcslcpy(void)
{
	wchar_t buf[MAXW + 1];

	/* empty src, every size */
	buf[0] = 0;
	t_wcslcpy_sizes(buf, 0);

	/* one wide char, every size */
	static const wchar_t singles[] = {
		1, L'a', 0x7f, 0x80, 0xff, 0x100, 0xfffd, 0xffff, 0x10ffff,
		0x7f7f7f7f
	};
	for (size_t i = 0; i < sizeof singles / sizeof singles[0]; i++) {
		buf[0] = singles[i];
		t_wcslcpy_sizes(buf, 1);
	}

	/* short ascending runs (covers the siz == len truncation boundary) */
	for (size_t n = 1; n <= 10; n++) {
		for (size_t i = 0; i < n; i++)
			buf[i] = (wchar_t)(L'a' + i);
		t_wcslcpy_sizes(buf, n);
	}

	/* runs of the guard value itself, so a stale guard word left in dst
	 * cannot masquerade as copied data */
	for (size_t n = 1; n <= 6; n++) {
		for (size_t i = 0; i < n; i++)
			buf[i] = (wchar_t)0x7f7f7f7f;
		t_wcslcpy_sizes(buf, n);
	}

	/* high code points */
	for (size_t n = 1; n <= 6; n++) {
		for (size_t i = 0; i < n; i++)
			buf[i] = (wchar_t)(0x10fff0 + i);
		t_wcslcpy_sizes(buf, n);
	}

	/* longest src, exhaustive size sweep including 0 and len +/- 1 */
	for (size_t i = 0; i < MAXW; i++)
		buf[i] = (wchar_t)(1 + i * 7);
	for (size_t siz = 0; siz <= MAXW + 4; siz++)
		t_wcslcpy(buf, MAXW, siz);

	/* mid-length src, exhaustive size sweep */
	for (size_t i = 0; i < 7; i++)
		buf[i] = (wchar_t)(0x80 + i);
	for (size_t siz = 0; siz <= 12; siz++)
		t_wcslcpy(buf, 7, siz);
}

/* ------------------------------------------------------------------ */
/* randomised sweeps (fixed seeds)                                    */
/* ------------------------------------------------------------------ */

static const unsigned char AL_SMALL[] = { 'a', 'b', 'c' };
static const unsigned char AL_SEP[] = {
	'a', 'b', 'c', ',', ';', 0x7f, 0x80, 0xc0, 0xff
};
static const unsigned char AL_BOUND[] = {
	0x01, 0x3e, 0x3f, 0x40, 0x41, 0x7e, 0x7f, 0x80, 0x81,
	0xbe, 0xbf, 0xc0, 0xc1, 0xfe, 0xff
};

static unsigned char
gen_byte(Rng &r, int mode)
{
	switch (mode) {
	case 0:
		return (unsigned char)(1 + r.below(255));	/* 0x01..0xff */
	case 1:
		return AL_SMALL[r.below((uint32_t)sizeof AL_SMALL)];
	case 2:
		return AL_BOUND[r.below((uint32_t)sizeof AL_BOUND)];
	case 3:
		return (unsigned char)(0x80 + r.below(0x80));	/* high bit set */
	default:
		return AL_SEP[r.below((uint32_t)sizeof AL_SEP)];
	}
}

static void
sweep_span(void)
{
	Rng r(0x5350414E5F4231ull);
	unsigned char s[MAXS + 1], cs[MAXS + 1];

	for (int iter = 0; iter < 200000; iter++) {
		int mode = (int)r.below(5);
		size_t clen = r.below(9);		/* 0..8 */
		for (size_t i = 0; i < clen; i++)
			cs[i] = gen_byte(r, mode);

		size_t slen = r.below(25);		/* 0..24 */
		for (size_t i = 0; i < slen; i++) {
			/* mostly draw from the charset so the scan really runs,
			 * sometimes from outside so that it stops early */
			if (clen != 0 && r.pct(75))
				s[i] = cs[r.below((uint32_t)clen)];
			else
				s[i] = gen_byte(r, mode);
		}
		t_span_both(s, slen, cs, clen);
	}
}

static void
sweep_strsep(void)
{
	Rng r(0x5345505F4231ull);
	unsigned char s[MAXS + 1], dl[MAXS + 1];

	for (int iter = 0; iter < 200000; iter++) {
		int mode = (iter % 5 == 0) ? 0 : 4;
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

static void
sweep_wcslcpy(void)
{
	Rng r(0x57435343505931ull);
	wchar_t src[MAXW + 1];

	for (int iter = 0; iter < 200000; iter++) {
		size_t blen = r.below((uint32_t)MAXW + 1);	/* 0..MAXW */
		for (size_t i = 0; i < blen; i++) {
			wchar_t c;
			switch (r.below(4)) {
			case 0:
				c = (wchar_t)(1 + r.below(255));
				break;
			case 1:
				c = (wchar_t)(0x80 + r.below(0x80));
				break;
			case 2:
				c = (wchar_t)(1 + r.below(0x10ffff));
				break;
			default:
				c = (wchar_t)0x7f7f7f7f;
				break;
			}
			src[i] = c;
		}

		/* sizes clustered on the truncation boundary, plus 0 and
		 * values well past the end of src */
		size_t siz;
		switch (r.below(6)) {
		case 0:
			siz = 0;
			break;
		case 1:
			siz = 1;
			break;
		case 2:
			siz = blen;
			break;
		case 3:
			siz = blen + 1;
			break;
		case 4:
			siz = (blen == 0) ? 0 : blen - 1;
			break;
		default:
			siz = r.below((uint32_t)MAXW + 5);
			break;
		}
		t_wcslcpy(src, blen, siz);
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	hand_span();
	hand_strsep();
	hand_wcslcpy();

	sweep_span();
	sweep_strsep();
	sweep_wcslcpy();

	const Stat *all[] = { &st_strspn, &st_strcspn, &st_strsep, &st_wcslcpy };
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
