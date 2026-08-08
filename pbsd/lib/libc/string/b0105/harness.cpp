/*
 * harness.cpp -- differential test for PBSD batch b0105.
 *
 * Every case is executed twice: once against the C++ port (module
 * pbsd.lib.libc.string.b0105) and once against the unmodified C reference in
 * oracle.c.  Return values, pointer *offsets* (never raw addresses) and the
 * complete contents of every buffer -- including the guard bytes past the
 * nominal write window -- are compared after each call.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.string.b0105;

namespace port = pbsd::lib_libc_string::b0105;

extern "C" std::size_t ref_strspn(const char *s, const char *charset);
extern "C" std::size_t ref_strcspn(const char *s, const char *charset);
extern "C" char *ref_strsep(char **stringp, const char *delim);
extern "C" std::size_t ref_wcslcpy(wchar_t *dst, const wchar_t *src,
    std::size_t siz);

/* ------------------------------------------------------------------------ */
/* infrastructure                                                            */
/* ------------------------------------------------------------------------ */

static const unsigned char GUARD = 0x7f;
static const int MAX_REPORT = 8;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

static Stat st_strspn = { "strspn", 0, 0, 0 };
static Stat st_strcspn = { "strcspn", 0, 0, 0 };
static Stat st_strsep = { "strsep", 0, 0, 0 };
static Stat st_wcslcpy = { "wcslcpy", 0, 0, 0 };

static std::uint64_t rng_state;

static inline std::uint64_t
rnd(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static inline std::size_t
rnd_mod(std::size_t m)
{
	return (std::size_t)(rnd() % (std::uint64_t)m);
}

static void
dump_bytes(const char *label, const unsigned char *p, std::size_t n)
{
	std::printf("      %s:", label);
	for (std::size_t i = 0; i < n; i++)
		std::printf(" %02x", (unsigned)p[i]);
	std::printf("\n");
}

static void
dump_wide(const char *label, const wchar_t *p, std::size_t n)
{
	std::printf("      %s:", label);
	for (std::size_t i = 0; i < n; i++)
		std::printf(" %08lx", (unsigned long)(std::uint32_t)p[i]);
	std::printf("\n");
}

/* ------------------------------------------------------------------------ */
/* strspn / strcspn                                                          */
/* ------------------------------------------------------------------------ */

static const std::size_t SBUF = 96;

static void
span_run(Stat &st,
    std::size_t (*port_fn)(const char *, const char *),
    std::size_t (*ref_fn)(const char *, const char *),
    const char *s, std::size_t slen, const char *cs, std::size_t cslen,
    const char *tag)
{
	char sa[SBUF], sb[SBUF], ca[SBUF], cb[SBUF];

	if (slen + 1 > SBUF || cslen + 1 > SBUF) {
		std::fprintf(stderr,
		    "harness bug: %s would overflow (slen=%zu cslen=%zu)\n",
		    tag, slen, cslen);
		std::exit(2);
	}

	std::memset(sa, GUARD, sizeof(sa));
	std::memset(sb, GUARD, sizeof(sb));
	std::memset(ca, GUARD, sizeof(ca));
	std::memset(cb, GUARD, sizeof(cb));
	std::memcpy(sa, s, slen + 1);
	std::memcpy(sb, s, slen + 1);
	std::memcpy(ca, cs, cslen + 1);
	std::memcpy(cb, cs, cslen + 1);

	std::size_t ra = port_fn(sa, ca);
	std::size_t rb = ref_fn(sb, cb);

	int bad_ret = (ra != rb);
	int bad_s = (std::memcmp(sa, sb, sizeof(sa)) != 0);
	int bad_cs = (std::memcmp(ca, cb, sizeof(ca)) != 0);

	st.cases++;
	if (bad_ret || bad_s || bad_cs) {
		st.fails++;
		if (st.reported < MAX_REPORT) {
			st.reported++;
			std::printf("  FAIL %s [%s] slen=%zu cslen=%zu%s%s%s\n",
			    st.name, tag, slen, cslen,
			    bad_ret ? " ret" : "", bad_s ? " sbuf" : "",
			    bad_cs ? " csbuf" : "");
			std::printf("      port_ret=%zu ref_ret=%zu\n", ra, rb);
			dump_bytes("port_s ", (unsigned char *)sa, slen + 1);
			dump_bytes("ref_s  ", (unsigned char *)sb, slen + 1);
			dump_bytes("port_cs", (unsigned char *)ca, cslen + 1);
			dump_bytes("ref_cs ", (unsigned char *)cb, cslen + 1);
		}
	}
}

static const char *const span_s[] = {
	"",
	"a",
	"b",
	"z",
	"ab",
	"ba",
	"abc",
	"cba",
	"abcdefghij",
	"aaaaaaaaaa",
	"bbbbbbbbbb",
	"\x80",
	"\xff",
	"\x7f",
	"\x01",
	"\x40",
	"\x3f",
	"\x80\xff\x7f",
	"a\x80" "b",
	"\xff" "a\xff",
	"aaaaaaaaaaq",
	"qaaaaaaaaaa",
	"\x80\x80\x80\x80",
	"\xfe\xfd\xfc\xfb",
	"MNOP",
	"\x00",
	"a\x00" "b",
};

static const char *const span_cs[] = {
	"",
	"a",
	"b",
	"c",
	"z",
	"q",
	"Z",
	"\x80",
	"\xff",
	"\x7f",
	"\x01",
	"\x40",
	"\x3f",
	"ac",
	"ca",
	"\x80\xff",
	"\xff\x80",
	"abcdefghij",
	"XYZ",
	"\xfb\xfc",
	"MNOP",
	"\x7f\x01",
	"abc",
	"a",
	"\xfe",
};

static void
span_edge(Stat &st,
    std::size_t (*port_fn)(const char *, const char *),
    std::size_t (*ref_fn)(const char *, const char *))
{
	std::size_t ns = sizeof(span_s) / sizeof(span_s[0]);
	std::size_t nc = sizeof(span_cs) / sizeof(span_cs[0]);

	for (std::size_t i = 0; i < ns; i++)
		for (std::size_t j = 0; j < nc; j++)
			span_run(st, port_fn, ref_fn, span_s[i],
			    std::strlen(span_s[i]), span_cs[j],
			    std::strlen(span_cs[j]), "grid");

	/* every byte value on both sides */
	{
		char s[2], cs[2];

		s[1] = '\0';
		cs[1] = '\0';
		for (int v = 0; v < 256; v++) {
			s[0] = (char)v;
			cs[0] = (char)v;
			span_run(st, port_fn, ref_fn, s, 1, cs, 1, "byte-eq");
			cs[0] = (char)((v + 1) & 0xff);
			if (cs[0] == '\0')
				cs[0] = '\x01';
			span_run(st, port_fn, ref_fn, s, 1, cs, 1, "byte-ne");
			span_run(st, port_fn, ref_fn, s, 1, "", 0, "byte-empty-cs");
		}
	}

	/* LONG_BIT bucket boundaries (64-bit: 0, 63, 64, 127, 128, 255) */
	{
		static const int bounds[] = {
			0, 1, 62, 63, 64, 65, 126, 127, 128, 129, 254, 255
		};
		char s[4], cs[4];

		for (std::size_t bi = 0; bi < sizeof(bounds) / sizeof(bounds[0]);
		    bi++) {
			int v = bounds[bi];
			s[0] = (char)v;
			s[1] = 'x';
			s[2] = 'y';
			s[3] = '\0';
			cs[0] = (char)v;
			cs[1] = '\0';
			span_run(st, port_fn, ref_fn, s, 3, cs, 1, "bound-hit");
			cs[0] = (char)((v + 1) & 0xff);
			if (cs[0] == '\0')
				cs[0] = '\x02';
			span_run(st, port_fn, ref_fn, s, 3, cs, 1, "bound-miss");
		}
	}

	/* prefix lengths flush against buffer end */
	{
		char s[SBUF], cs[2];

		cs[1] = '\0';
		for (std::size_t len = 0; len + 1 < SBUF; len++) {
			for (std::size_t i = 0; i < len; i++)
				s[i] = 'a';
			s[len] = '\0';
			cs[0] = 'a';
			span_run(st, port_fn, ref_fn, s, len, cs, 1, "full-prefix");
			cs[0] = 'b';
			span_run(st, port_fn, ref_fn, s, len, cs, 1, "zero-prefix");
		}
	}

	/* NUL-heavy strings */
	{
		char s[8] = { 'a', '\0', 'b', '\0', 'c', '\0', 'd', '\0' };
		char cs[4] = { 'a', 'b', 'c', '\0' };
		span_run(st, port_fn, ref_fn, s, 7, cs, 3, "nul-heavy");
		span_run(st, port_fn, ref_fn, s, 7, "", 0, "nul-heavy-empty-cs");
	}

	/* duplicate charset entries */
	{
		char cs[8];
		for (int i = 0; i < 7; i++)
			cs[i] = (char)('a' + (i % 3));
		cs[7] = '\0';
		span_run(st, port_fn, ref_fn, "abcabc", 6, cs, 7, "dup-cs");
	}
}

static const unsigned char span_alpha[] = {
	'a', 'b', 'c', 0x01, 0x3f, 0x40, 0x7f, 0x80, 0xfe, 0xff, 'z', '\0'
};

static void
span_random(Stat &st,
    std::size_t (*port_fn)(const char *, const char *),
    std::size_t (*ref_fn)(const char *, const char *),
    long iters)
{
	char s[SBUF], cs[SBUF];
	std::size_t na = sizeof(span_alpha) / sizeof(span_alpha[0]);

	for (long it = 0; it < iters; it++) {
		std::size_t slen, cslen;
		int wide = (rnd_mod(5) == 0);

		if (rnd_mod(25) == 0)
			slen = 60 + rnd_mod(35);
		else
			slen = rnd_mod(20);
		if (rnd_mod(25) == 0)
			cslen = 40 + rnd_mod(55);
		else
			cslen = rnd_mod(16);

		for (std::size_t i = 0; i < slen; i++)
			s[i] = wide ? (char)(rnd_mod(256))
			    : (char)span_alpha[rnd_mod(na)];
		s[slen] = '\0';
		for (std::size_t i = 0; i < cslen; i++)
			cs[i] = wide ? (char)(rnd_mod(256))
			    : (char)span_alpha[rnd_mod(na)];
		cs[cslen] = '\0';

		span_run(st, port_fn, ref_fn, s, slen, cs, cslen, "rand");
	}
}

/* ------------------------------------------------------------------------ */
/* strsep                                                                    */
/* ------------------------------------------------------------------------ */

static const std::size_t SEPBUF = 96;
static const std::size_t DELIMBUF = 32;

static long
ptr_off(char *base, char *p)
{
	return (p == nullptr) ? -1L : (long)(p - base);
}

static void
sep_fail(Stat &st, const char *tag, const char *why, long step,
    long tok_a, long tok_b, long sp_a, long sp_b,
    const unsigned char *a, const unsigned char *b, std::size_t n)
{
	st.fails++;
	if (st.reported < MAX_REPORT) {
		st.reported++;
		std::printf("  FAIL strsep [%s] step=%ld %s\n", tag, step, why);
		std::printf("      port_tok=%ld ref_tok=%ld port_sp=%ld "
		    "ref_sp=%ld\n", tok_a, tok_b, sp_a, sp_b);
		dump_bytes("port_buf", a, n);
		dump_bytes("ref_buf ", b, n);
	}
}

static void
sep_run(const char *input, std::size_t inlen, const char *delim,
    std::size_t dlen, const char *tag)
{
	unsigned char a[SEPBUF], b[SEPBUF];
	unsigned char da[DELIMBUF], db[DELIMBUF];
	char *spa, *spb;

	if (inlen + 1 > SEPBUF || dlen + 1 > DELIMBUF) {
		std::fprintf(stderr,
		    "harness bug: %s would overflow (inlen=%zu dlen=%zu)\n",
		    tag, inlen, dlen);
		std::exit(2);
	}

	std::memset(a, GUARD, sizeof(a));
	std::memset(b, GUARD, sizeof(b));
	std::memset(da, GUARD, sizeof(da));
	std::memset(db, GUARD, sizeof(db));
	std::memcpy(a, input, inlen + 1);
	std::memcpy(b, input, inlen + 1);
	std::memcpy(da, delim, dlen + 1);
	std::memcpy(db, delim, dlen + 1);

	spa = (char *)a;
	spb = (char *)b;

	for (long step = 0;; step++) {
		char *ra = port::strsep(&spa, (char *)da);
		char *rb = ref_strsep(&spb, (char *)db);

		long tok_a = ptr_off((char *)a, ra);
		long tok_b = ptr_off((char *)b, rb);
		long sp_off_a = ptr_off((char *)a, spa);
		long sp_off_b = ptr_off((char *)b, spb);

		st_strsep.cases++;
		if (tok_a != tok_b || sp_off_a != sp_off_b ||
		    std::memcmp(a, b, sizeof(a)) != 0 ||
		    std::memcmp(da, db, sizeof(da)) != 0) {
			const char *why = "mismatch";
			if (tok_a != tok_b)
				why = "tok_off";
			else if (sp_off_a != sp_off_b)
				why = "sp_off";
			else if (std::memcmp(a, b, sizeof(a)) != 0)
				why = "buf";
			else
				why = "delim";
			sep_fail(st_strsep, tag, why, step, tok_a, tok_b,
			    sp_off_a, sp_off_b, a, b, sizeof(a));
		}

		if (ra == nullptr && rb == nullptr)
			break;
	}
}

static void
sep_null_stringp(const char *delim, std::size_t dlen, const char *tag)
{
	unsigned char da[DELIMBUF], db[DELIMBUF];
	char *spa = nullptr;
	char *spb = nullptr;

	std::memset(da, GUARD, sizeof(da));
	std::memset(db, GUARD, sizeof(db));
	std::memcpy(da, delim, dlen + 1);
	std::memcpy(db, delim, dlen + 1);

	char *ra = port::strsep(&spa, (char *)da);
	char *rb = ref_strsep(&spb, (char *)db);

	st_strsep.cases++;
	if (ra != nullptr || rb != nullptr || spa != nullptr || spb != nullptr ||
	    std::memcmp(da, db, sizeof(da)) != 0) {
		st_strsep.fails++;
		if (st_strsep.reported < MAX_REPORT) {
			st_strsep.reported++;
			std::printf("  FAIL strsep [%s] null-stringp\n", tag);
			std::printf("      port=%p ref=%p spa=%p spb=%p\n",
			    (void *)ra, (void *)rb, (void *)spa, (void *)spb);
		}
	}
}

static const char *const sep_in[] = {
	"",
	"a",
	",",
	",,",
	"a,",
	",a",
	"a,b",
	"a,,b",
	"a,b,c",
	"no-delims",
	"\x80,\xff",
	"a\x80" "b,c",
	"foo;bar;baz",
	";;;",
	"x",
	"a\x00" "b",
	"\xff" "a\xff" "b",
	"token",
	" leading",
	"trail ",
};

static const char *const sep_delim[] = {
	"",
	",",
	";",
	",;",
	"\x80",
	"\xff",
	"a",
	"ab",
	" ,",
	"\x7f",
	"\x01",
	"token",
	"xyz",
	"\xfe\xfd",
};

static void
sep_edge(void)
{
	std::size_t ni = sizeof(sep_in) / sizeof(sep_in[0]);
	std::size_t nd = sizeof(sep_delim) / sizeof(sep_delim[0]);

	for (std::size_t i = 0; i < ni; i++)
		for (std::size_t j = 0; j < nd; j++)
			sep_run(sep_in[i], std::strlen(sep_in[i]),
			    sep_delim[j], std::strlen(sep_delim[j]), "grid");

	for (std::size_t j = 0; j < nd; j++)
		sep_null_stringp(sep_delim[j], std::strlen(sep_delim[j]),
		    "grid");

	/* every byte as single-char delimiter */
	{
		char in[4] = { 'a', 'b', 'c', '\0' };
		char d[2];

		d[1] = '\0';
		for (int v = 0; v < 256; v++) {
			d[0] = (char)v;
			sep_run(in, 3, d, 1, "byte-delim");
		}
	}

	/* consecutive empty tokens */
	{
		char in[16];
		for (int n = 0; n < 8; n++) {
			std::size_t k = 0;
			for (int i = 0; i < n; i++)
				in[k++] = ',';
			in[k++] = 'x';
			in[k] = '\0';
			sep_run(in, k, ",", 1, "empty-tok");
		}
	}

	/* changing delimiter between calls within one exhaustion */
	{
		unsigned char a[SEPBUF], b[SEPBUF];
		unsigned char da[DELIMBUF], db[DELIMBUF];
		const char *input = "a,b;c,d";
		const char *delims[] = { ",", ";" };
		char *spa, *spb;
		std::size_t inlen = std::strlen(input);

		std::memset(a, GUARD, sizeof(a));
		std::memset(b, GUARD, sizeof(b));
		std::memcpy(a, input, inlen + 1);
		std::memcpy(b, input, inlen + 1);
		spa = (char *)a;
		spb = (char *)b;

		for (long step = 0;; step++) {
			const char *d = delims[step & 1];
			std::memset(da, GUARD, sizeof(da));
			std::memset(db, GUARD, sizeof(db));
			std::memcpy(da, d, 2);
			std::memcpy(db, d, 2);

			char *ra = port::strsep(&spa, (char *)da);
			char *rb = ref_strsep(&spb, (char *)db);

			long tok_a = ptr_off((char *)a, ra);
			long tok_b = ptr_off((char *)b, rb);
			long sp_off_a = ptr_off((char *)a, spa);
			long sp_off_b = ptr_off((char *)b, spb);

			st_strsep.cases++;
			if (tok_a != tok_b || sp_off_a != sp_off_b ||
			    std::memcmp(a, b, sizeof(a)) != 0) {
				sep_fail(st_strsep, "swap-delim", "mismatch",
				    step, tok_a, tok_b, sp_off_a, sp_off_b,
				    a, b, sizeof(a));
			}
			if (ra == nullptr && rb == nullptr)
				break;
		}
	}
}

static const unsigned char sep_alpha[] = {
	'a', 'b', ',', ';', 0x80, 0xff, 0x7f, 0x01, '\0'
};

static void
sep_random(long iters)
{
	char in[SEPBUF], d[DELIMBUF];
	std::size_t na = sizeof(sep_alpha) / sizeof(sep_alpha[0]);

	for (long it = 0; it < iters; it++) {
		std::size_t inlen = rnd_mod(24);
		std::size_t dlen = rnd_mod(8);
		int wide = (rnd_mod(6) == 0);

		for (std::size_t i = 0; i < inlen; i++)
			in[i] = wide ? (char)(rnd_mod(256))
			    : (char)sep_alpha[rnd_mod(na)];
		in[inlen] = '\0';
		for (std::size_t i = 0; i < dlen; i++)
			d[i] = wide ? (char)(rnd_mod(256))
			    : (char)sep_alpha[rnd_mod(na)];
		d[dlen] = '\0';

		sep_run(in, inlen, d, dlen, "rand");

		if (rnd_mod(50) == 0)
			sep_null_stringp(d, dlen, "rand");
	}
}

/* ------------------------------------------------------------------------ */
/* wcslcpy                                                                   */
/* ------------------------------------------------------------------------ */

static const std::size_t WDST = 96;
static const std::size_t WSRC = 64;

static void
lcp_run(const wchar_t *src, std::size_t slen, std::size_t siz, const char *tag)
{
	wchar_t a[WDST], b[WDST];
	wchar_t sa[WSRC], sb[WSRC];

	if (slen + 1 > WSRC) {
		std::fprintf(stderr,
		    "harness bug: %s would overflow (slen=%zu)\n", tag, slen);
		std::exit(2);
	}

	std::memset(a, GUARD, sizeof(a));
	std::memset(b, GUARD, sizeof(b));
	std::memset(sa, GUARD, sizeof(sa));
	std::memset(sb, GUARD, sizeof(sb));
	std::memcpy(sa, src, (slen + 1) * sizeof(wchar_t));
	std::memcpy(sb, src, (slen + 1) * sizeof(wchar_t));

	std::size_t ra = port::wcslcpy(a, sa, siz);
	std::size_t rb = ref_wcslcpy(b, sb, siz);

	int bad_ret = (ra != rb);
	int bad_dst = (std::memcmp(a, b, sizeof(a)) != 0);
	int bad_src = (std::memcmp(sa, sb, sizeof(sa)) != 0);

	st_wcslcpy.cases++;
	if (bad_ret || bad_dst || bad_src) {
		st_wcslcpy.fails++;
		if (st_wcslcpy.reported < MAX_REPORT) {
			st_wcslcpy.reported++;
			std::printf("  FAIL wcslcpy [%s] slen=%zu siz=%zu%s%s%s\n",
			    tag, slen, siz, bad_ret ? " ret" : "",
			    bad_dst ? " dst" : "", bad_src ? " src" : "");
			std::printf("      port_ret=%zu ref_ret=%zu\n", ra, rb);
			dump_wide("port_dst", a, WDST);
			dump_wide("ref_dst ", b, WDST);
		}
	}
}

static const wchar_t *const lcp_srcs[] = {
	L"",
	L"a",
	L"ab",
	L"abc",
	L"abcd",
	L"hello",
	L"\x7f",
	L"\x80",
	L"\xffff",
	L"\x10000",
	L"a\x000" "b",
};

static const std::size_t lcp_siz[] = { 0, 1, 2, 3, 4, 5, 8, 16, 32, 64 };

static void
lcp_edge(void)
{
	std::size_t ns = sizeof(lcp_srcs) / sizeof(lcp_srcs[0]);
	std::size_t nz = sizeof(lcp_siz) / sizeof(lcp_siz[0]);

	for (std::size_t i = 0; i < ns; i++) {
		std::size_t slen = std::wcslen(lcp_srcs[i]);
		for (std::size_t j = 0; j < nz; j++)
			lcp_run(lcp_srcs[i], slen, lcp_siz[j], "grid");

		if (slen > 0) {
			lcp_run(lcp_srcs[i], slen, slen - 1, "siz=slen-1");
			lcp_run(lcp_srcs[i], slen, slen, "siz=slen");
			lcp_run(lcp_srcs[i], slen, slen + 1, "siz=slen+1");
		}
	}

	/* exact-fill and near-fill against buffer end */
	{
		wchar_t src[WSRC];
		for (std::size_t slen = 0; slen <= 12; slen++) {
			for (std::size_t i = 0; i < slen; i++)
				src[i] = (wchar_t)(0x100 + i);
			src[slen] = L'\0';

			for (std::size_t siz = 0; siz <= slen + 2; siz++)
				lcp_run(src, slen, siz, "exact-fill");
		}
	}

	/* siz==0 must not write dst */
	{
		wchar_t src[] = L"xyz";
		lcp_run(src, 3, 0, "siz0");
	}
}

static const wchar_t lcp_alpha[] = {
	(wchar_t)0x0001, (wchar_t)0x007f, (wchar_t)0x0080, (wchar_t)0x00ff,
	(wchar_t)0x0100, (wchar_t)0xffff, (wchar_t)0x41, (wchar_t)0x61,
};

static void
lcp_random(long iters)
{
	wchar_t src[WSRC];
	std::size_t na = sizeof(lcp_alpha) / sizeof(lcp_alpha[0]);

	for (long it = 0; it < iters; it++) {
		std::size_t slen = rnd_mod(20);
		for (std::size_t i = 0; i < slen; i++)
			src[i] = lcp_alpha[rnd_mod(na)];
		src[slen] = L'\0';

		std::size_t siz;
		switch ((int)rnd_mod(10)) {
		case 0: case 1: case 2: case 3:
			siz = rnd_mod(24);
			break;
		case 4: case 5:
			siz = slen;
			break;
		case 6:
			siz = slen ? slen - 1 : 0;
			break;
		case 7:
			siz = slen + 1;
			break;
		case 8:
			siz = 0;
			break;
		default:
			siz = (std::size_t)-1;
			break;
		}

		lcp_run(src, slen, siz, "rand");
	}
}

/* ------------------------------------------------------------------------ */

int
main(void)
{
	rng_state = 0x0105b0105ULL;
	span_edge(st_strspn, port::strspn, ref_strspn);
	span_random(st_strspn, port::strspn, ref_strspn, 200000);

	rng_state = 0x0105c0105ULL;
	span_edge(st_strcspn, port::strcspn, ref_strcspn);
	span_random(st_strcspn, port::strcspn, ref_strcspn, 200000);

	rng_state = 0x0105d0105ULL;
	sep_edge();
	sep_random(200000);

	rng_state = 0x0105e0105ULL;
	lcp_edge();
	lcp_random(200000);

	std::printf("\n%-12s %12s %12s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-12s %12s %12s   %s\n", "------------", "------------",
	    "------------", "------");

	const Stat *all[] = {
		&st_strspn, &st_strcspn, &st_strsep, &st_wcslcpy
	};
	long total_fail = 0;
	long total_case = 0;

	for (std::size_t i = 0; i < sizeof(all) / sizeof(all[0]); i++) {
		std::printf("%-12s %12ld %12ld   %s\n", all[i]->name,
		    all[i]->cases, all[i]->fails,
		    all[i]->fails == 0 ? "PASS" : "FAIL");
		total_fail += all[i]->fails;
		total_case += all[i]->cases;
	}
	std::printf("%-12s %12s %12s   %s\n", "------------", "------------",
	    "------------", "------");
	std::printf("%-12s %12ld %12ld   %s\n", "TOTAL", total_case,
	    total_fail, total_fail == 0 ? "PASS" : "FAIL");

	return total_fail == 0 ? 0 : 1;
}
