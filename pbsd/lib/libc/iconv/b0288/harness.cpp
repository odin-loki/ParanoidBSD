/*
 * Differential harness for PBSD batch b0288 (citrus_bcs, citrus_memstream).
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.iconv.b0288;

namespace P = pbsd::lib_libc_iconv::b0288;

extern "C" {
int ref__citrus_bcs_strcasecmp(const char *, const char *);
int ref__citrus_bcs_strncasecmp(const char *, const char *, size_t);
const char *ref__citrus_bcs_skip_ws(const char *);
const char *ref__citrus_bcs_skip_nonws(const char *);
const char *ref__citrus_bcs_skip_ws_len(const char *, size_t *);
const char *ref__citrus_bcs_skip_nonws_len(const char *, size_t *);
void ref__citrus_bcs_trunc_rws_len(const char *, size_t *);
void ref__citrus_bcs_convert_to_lower(char *);
void ref__citrus_bcs_convert_to_upper(char *);
const char *ref__citrus_memory_stream_getln(P::_citrus_memory_stream *, size_t *);
const char *ref__citrus_memory_stream_matchline(P::_citrus_memory_stream *,
    const char *, size_t *, int);
void *ref__citrus_memory_stream_chr(P::_citrus_memory_stream *,
    P::_citrus_region *, char);
void ref__citrus_memory_stream_skip_ws(P::_citrus_memory_stream *);
}

enum FuncId {
	F_STRCASECMP, F_STRNCASECMP, F_SKIP_WS, F_SKIP_NONWS,
	F_SKIP_WS_LEN, F_SKIP_NONWS_LEN, F_TRUNC_RWS_LEN,
	F_CONVERT_LOWER, F_CONVERT_UPPER,
	F_MS_GETLN, F_MS_MATCHLINE, F_MS_CHR, F_MS_SKIP_WS,
	F_COUNT
};

static const char *const FNAME[F_COUNT] = {
	"bcs_strcasecmp", "bcs_strncasecmp", "bcs_skip_ws", "bcs_skip_nonws",
	"bcs_skip_ws_len", "bcs_skip_nonws_len", "bcs_trunc_rws_len",
	"bcs_convert_to_lower", "bcs_convert_to_upper",
	"memstream_getln", "memstream_matchline", "memstream_chr",
	"memstream_skip_ws",
};

static unsigned long long NCASE[F_COUNT];
static unsigned long long NFAIL[F_COUNT];
static int NPRINT[F_COUNT];

static constexpr unsigned char GUARD = 0x7f;
static constexpr size_t PRE = 32, BODY = 256, POST = 32;
static constexpr size_t TOT = PRE + BODY + POST;

static std::uint64_t rng = 0xb0288b0288b0288ULL;

static std::uint64_t rng_next(void)
{
	std::uint64_t z = (rng += 0x9e3779b97f4a7c15ULL);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static unsigned char rb(void) { return (unsigned char)(rng_next() & 0xffu); }

static void bump(FuncId f) { NCASE[f]++; }

static void fail(FuncId f, const char *why)
{
	NFAIL[f]++;
	if (NPRINT[f]++ < 8)
		std::fprintf(stderr, "  FAIL %-22s : %s\n", FNAME[f], why);
}

struct GuardBuf {
	unsigned char storage[TOT];
	void init(void) { std::memset(storage, GUARD, sizeof storage); }
	char *body(void) { return (char *)(storage + PRE); }
	bool guards_ok(void) const {
		for (size_t i = 0; i < PRE; i++)
			if (storage[i] != GUARD) return false;
		for (size_t i = PRE + BODY; i < TOT; i++)
			if (storage[i] != GUARD) return false;
		return true;
	}
	bool eq(const GuardBuf &o) const {
		return std::memcmp(storage, o.storage, sizeof storage) == 0;
	}
	void copy_from(const GuardBuf &o) {
		std::memcpy(storage, o.storage, sizeof storage);
	}
};

static void fill_pattern(char *p, size_t n, unsigned seed)
{
	for (size_t i = 0; i < n; i++) p[i] = (char)((seed + i * 17) & 0xff);
	p[n] = '\0';
}

static void fill_random(char *p, size_t maxlen)
{
	size_t n = (size_t)(rng_next() % (maxlen + 1));
	for (size_t i = 0; i < n; i++) {
		unsigned v = rb();
		if (v == 0 && (rng_next() & 3u) != 0) v = (unsigned)('a' + (rng_next() % 26));
		p[i] = (char)v;
	}
	p[n] = '\0';
}

static ptrdiff_t off(const void *base, const void *p)
{
	return ((const char *)p - (const char *)base);
}

static void case_strcasecmp(const char *s1, const char *s2)
{
	int rp = P::_citrus_bcs_strcasecmp(s1, s2);
	int rr = ref__citrus_bcs_strcasecmp(s1, s2);
	if (rp != rr) fail(F_STRCASECMP, "rv"); else bump(F_STRCASECMP);
}

static void case_strncasecmp(const char *s1, const char *s2, size_t sz)
{
	int rp = P::_citrus_bcs_strncasecmp(s1, s2, sz);
	int rr = ref__citrus_bcs_strncasecmp(s1, s2, sz);
	if (rp != rr) fail(F_STRNCASECMP, "rv"); else bump(F_STRNCASECMP);
}

static void test_strcasecmp_edges(void)
{
	static const char *pairs[][2] = {
		{"", ""}, {"a", ""}, {"", "a"}, {"a", "a"}, {"a", "A"},
		{"ABC", "abc"}, {"abc", "abd"}, {"abd", "abc"},
		{"\x80", "\x80"}, {"\xff", "\xff"}, {"a\x80", "A\x80"},
		{"test", "TEST"}, {"\t\n", "\t\n"}, {"a", "b"}, {"b", "a"},
	};
	GuardBuf b1, b2;
	for (auto &p : pairs) case_strcasecmp(p[0], p[1]);
	b1.init(); b2.init();
	fill_pattern(b1.body(), 64, 0x80);
	fill_pattern(b2.body(), 64, 0x80);
	case_strcasecmp(b1.body(), b2.body());
	b1.init(); fill_pattern(b1.body(), 128, 0xff);
	case_strcasecmp(b1.body(), b1.body());
}

static void test_strncasecmp_edges(void)
{
	static const struct { const char *a, *b; size_t n; } cases[] = {
		{"", "", 0}, {"", "", 1}, {"a", "", 0}, {"a", "", 1},
		{"abc", "abd", 0}, {"abc", "abd", 2}, {"abc", "abd", 3},
		{"ABC", "abc", 3}, {"a\x80", "A\x80", 2}, {"aa", "ab", 1},
	};
	for (auto &c : cases) case_strncasecmp(c.a, c.b, c.n);
}

static char rs1[128], rs2[128];

static void test_strcasecmp_random(void)
{
	for (unsigned i = 0; i < 200000u; i++) {
		fill_random(rs1, sizeof(rs1) - 1);
		fill_random(rs2, sizeof(rs2) - 1);
		case_strcasecmp(rs1, rs2);
	}
}

static void test_strncasecmp_random(void)
{
	for (unsigned i = 0; i < 200000u; i++) {
		fill_random(rs1, sizeof(rs1) - 1);
		fill_random(rs2, sizeof(rs2) - 1);
		case_strncasecmp(rs1, rs2, (size_t)(rng_next() % 200u));
	}
}

static void case_skip_ws(const char *s, const char *base)
{
	const char *rp = P::_citrus_bcs_skip_ws(s);
	const char *rr = ref__citrus_bcs_skip_ws(s);
	if (off(base, rp) != off(base, rr)) fail(F_SKIP_WS, "offset");
	else bump(F_SKIP_WS);
}

static void case_skip_nonws(const char *s, const char *base)
{
	const char *rp = P::_citrus_bcs_skip_nonws(s);
	const char *rr = ref__citrus_bcs_skip_nonws(s);
	if (off(base, rp) != off(base, rr)) fail(F_SKIP_NONWS, "offset");
	else bump(F_SKIP_NONWS);
}

static void test_skip_ws_edges(void)
{
	GuardBuf b;
	b.init(); std::strcpy(b.body(), ""); case_skip_ws(b.body(), b.body());
	b.init(); std::strcpy(b.body(), "   \t\n\r\f\v"); case_skip_ws(b.body(), b.body());
	b.init(); std::strcpy(b.body(), "   hello"); case_skip_ws(b.body(), b.body());
	b.init(); std::strcpy(b.body(), "hello"); case_skip_ws(b.body(), b.body());
	b.init(); b.body()[0] = (char)0x80; b.body()[1] = ' '; b.body()[2] = 'a';
	b.body()[3] = '\0'; case_skip_ws(b.body(), b.body());
}

static void test_skip_nonws_edges(void)
{
	GuardBuf b;
	b.init(); std::strcpy(b.body(), ""); case_skip_nonws(b.body(), b.body());
	b.init(); std::strcpy(b.body(), "hello   "); case_skip_nonws(b.body(), b.body());
	b.init(); std::strcpy(b.body(), "   hello"); case_skip_nonws(b.body(), b.body());
	b.init(); std::strcpy(b.body(), " \t\n"); case_skip_nonws(b.body(), b.body());
	b.init(); std::strcpy(b.body(), "\x80\x81 "); case_skip_nonws(b.body(), b.body());
}

static void test_skip_ws_random(void)
{
	GuardBuf b;
	for (unsigned i = 0; i < 200000u; i++) {
		b.init();
		size_t n = (size_t)(rng_next() % (BODY - 1));
		for (size_t j = 0; j < n; j++) b.body()[j] = (char)rb();
		b.body()[n] = '\0';
		case_skip_ws(b.body(), b.body());
	}
}

static void test_skip_nonws_random(void)
{
	GuardBuf b;
	for (unsigned i = 0; i < 200000u; i++) {
		b.init();
		size_t n = (size_t)(rng_next() % (BODY - 1));
		for (size_t j = 0; j < n; j++) b.body()[j] = (char)rb();
		b.body()[n] = '\0';
		case_skip_nonws(b.body(), b.body());
	}
}

static void case_skip_ws_len(const char *p, size_t len, const char *base)
{
	size_t lp = len, lr = len;
	const char *rp = P::_citrus_bcs_skip_ws_len(p, &lp);
	const char *rr = ref__citrus_bcs_skip_ws_len(p, &lr);
	if (off(base, rp) != off(base, rr) || lp != lr) fail(F_SKIP_WS_LEN, "offset/len");
	else bump(F_SKIP_WS_LEN);
}

static void case_skip_nonws_len(const char *p, size_t len, const char *base)
{
	size_t lp = len, lr = len;
	const char *rp = P::_citrus_bcs_skip_nonws_len(p, &lp);
	const char *rr = ref__citrus_bcs_skip_nonws_len(p, &lr);
	if (off(base, rp) != off(base, rr) || lp != lr) fail(F_SKIP_NONWS_LEN, "offset/len");
	else bump(F_SKIP_NONWS_LEN);
}

static void case_trunc_rws_len(const char *p, size_t len)
{
	size_t lp = len, lr = len;
	ref__citrus_bcs_trunc_rws_len(p, &lr);
	P::_citrus_bcs_trunc_rws_len(p, &lp);
	if (lp != lr) fail(F_TRUNC_RWS_LEN, "len"); else bump(F_TRUNC_RWS_LEN);
}

static void test_skip_ws_len_edges(void)
{
	GuardBuf b;
	b.init(); std::strcpy(b.body(), "   abc");
	case_skip_ws_len(b.body(), 6, b.body());
	case_skip_ws_len(b.body(), 0, b.body());
	case_skip_ws_len(b.body(), 3, b.body());
	b.init(); std::strcpy(b.body(), "abc");
	case_skip_ws_len(b.body(), 3, b.body());
}

static void test_skip_nonws_len_edges(void)
{
	GuardBuf b;
	b.init(); std::strcpy(b.body(), "abc   ");
	case_skip_nonws_len(b.body(), 6, b.body());
	case_skip_nonws_len(b.body(), 0, b.body());
	b.init(); std::strcpy(b.body(), "   ");
	case_skip_nonws_len(b.body(), 3, b.body());
}

static void test_trunc_rws_len_edges(void)
{
	GuardBuf b;
	b.init(); std::strcpy(b.body(), "abc   ");
	case_trunc_rws_len(b.body(), 6);
	case_trunc_rws_len(b.body(), 0);
	b.init(); std::strcpy(b.body(), "  \t\n\r");
	case_trunc_rws_len(b.body(), 5);
	b.init(); std::strcpy(b.body(), " \x80 ");
	case_trunc_rws_len(b.body(), 4);
}

static void test_skip_ws_len_random(void)
{
	GuardBuf b;
	for (unsigned i = 0; i < 200000u; i++) {
		b.init();
		size_t n = (size_t)(rng_next() % (BODY - 1));
		for (size_t j = 0; j < n; j++) b.body()[j] = (char)rb();
		case_skip_ws_len(b.body(), (size_t)(rng_next() % (n + 1)), b.body());
	}
}

static void test_skip_nonws_len_random(void)
{
	GuardBuf b;
	for (unsigned i = 0; i < 200000u; i++) {
		b.init();
		size_t n = (size_t)(rng_next() % (BODY - 1));
		for (size_t j = 0; j < n; j++) b.body()[j] = (char)rb();
		case_skip_nonws_len(b.body(), (size_t)(rng_next() % (n + 1)), b.body());
	}
}

static void test_trunc_rws_len_random(void)
{
	GuardBuf b;
	for (unsigned i = 0; i < 200000u; i++) {
		b.init();
		size_t n = (size_t)(rng_next() % (BODY - 1));
		for (size_t j = 0; j < n; j++) b.body()[j] = (char)rb();
		case_trunc_rws_len(b.body(), (size_t)(rng_next() % (n + 1)));
	}
}

static void case_convert_lower(const char *in)
{
	GuardBuf bp, br;
	bp.init(); br.init(); bp.copy_from(br);
	std::strcpy(bp.body(), in); std::strcpy(br.body(), in);
	P::_citrus_bcs_convert_to_lower(bp.body());
	ref__citrus_bcs_convert_to_lower(br.body());
	if (!bp.guards_ok() || !br.guards_ok()) fail(F_CONVERT_LOWER, "guards");
	else if (!bp.eq(br)) fail(F_CONVERT_LOWER, "buf");
	else bump(F_CONVERT_LOWER);
}

static void case_convert_upper(const char *in)
{
	GuardBuf bp, br;
	bp.init(); br.init(); bp.copy_from(br);
	std::strcpy(bp.body(), in); std::strcpy(br.body(), in);
	P::_citrus_bcs_convert_to_upper(bp.body());
	ref__citrus_bcs_convert_to_upper(br.body());
	if (!bp.guards_ok() || !br.guards_ok()) fail(F_CONVERT_UPPER, "guards");
	else if (!bp.eq(br)) fail(F_CONVERT_UPPER, "buf");
	else bump(F_CONVERT_UPPER);
}

static void test_convert_edges(void)
{
	static const char *const cases[] = {
		"", "a", "A", "AbC", "ABCdef", "\x80\xff", "a\x80Z", " \t\n", "123",
	};
	for (const char *s : cases) {
		case_convert_lower(s);
		case_convert_upper(s);
	}
}

static void test_convert_lower_random(void)
{
	char tmp[128];
	for (unsigned i = 0; i < 200000u; i++) {
		fill_random(tmp, sizeof(tmp) - 1);
		case_convert_lower(tmp);
	}
}

static void test_convert_upper_random(void)
{
	char tmp[128];
	for (unsigned i = 0; i < 200000u; i++) {
		fill_random(tmp, sizeof(tmp) - 1);
		case_convert_upper(tmp);
	}
}

static void ms_bind(P::_citrus_memory_stream *ms, unsigned char *data, size_t sz)
{
	P::_citrus_region r;
	P::_citrus_region_init(&r, data, sz);
	P::_citrus_memory_stream_bind(ms, &r);
}

static void ref_ms_bind(P::_citrus_memory_stream *ms, unsigned char *data, size_t sz)
{
	P::_citrus_region r;
	P::_citrus_region_init(&r, data, sz);
	ms->ms_region = r;
	ms->ms_pos = 0;
}

static void case_getln(unsigned char *data, size_t sz, size_t start)
{
	P::_citrus_memory_stream mp, mr;
	size_t lp = 0, lr = 0;
	unsigned char dp[512], dr[512];
	std::memcpy(dp, data, sz); std::memcpy(dr, data, sz);
	ms_bind(&mp, dp, sz); ref_ms_bind(&mr, dr, sz);
	mp.ms_pos = start; mr.ms_pos = start;
	const char *pp = P::_citrus_memory_stream_getln(&mp, &lp);
	const char *pr = ref__citrus_memory_stream_getln(&mr, &lr);
	if ((pp == nullptr) != (pr == nullptr)) fail(F_MS_GETLN, "null");
	else if (pp && (off(dp, pp) != off(dr, pr) || lp != lr)) fail(F_MS_GETLN, "ptr/len");
	else if (mp.ms_pos != mr.ms_pos) fail(F_MS_GETLN, "pos");
	else if (std::memcmp(dp, dr, sz) != 0) fail(F_MS_GETLN, "buf");
	else bump(F_MS_GETLN);
}

static void test_getln_edges(void)
{
	unsigned char d2[] = "hello\nworld";
	case_getln(d2, sizeof(d2) - 1, 0);
	case_getln(d2, sizeof(d2) - 1, 6);
	unsigned char d3[] = "noeol";
	case_getln(d3, sizeof(d3) - 1, 0);
	unsigned char d4[] = "\r\n";
	case_getln(d4, sizeof(d4) - 1, 0);
	unsigned char d5[] = "a\nb\nc";
	case_getln(d5, sizeof(d5) - 1, 0);
	case_getln(d5, sizeof(d5) - 1, 2);
	case_getln(d5, sizeof(d5) - 1, 99);
	unsigned char d6[] = "\x80\xff\n";
	case_getln(d6, sizeof(d6) - 1, 0);
	unsigned char d1[] = "";
	case_getln(d1, 0, 0);
}

static void test_getln_random(void)
{
	unsigned char data[256];
	for (unsigned i = 0; i < 200000u; i++) {
		size_t sz = (size_t)(rng_next() % (sizeof(data) + 1));
		for (size_t j = 0; j < sz; j++) data[j] = rb();
		case_getln(data, sz, sz > 0 ? (size_t)(rng_next() % (sz + 1)) : 0);
	}
}

static void case_matchline(const unsigned char *content, size_t sz,
    const char *key, int cs)
{
	unsigned char dp[512], dr[512];
	std::memcpy(dp, content, sz); std::memcpy(dr, content, sz);
	P::_citrus_memory_stream mp, mr;
	ms_bind(&mp, dp, sz); ref_ms_bind(&mr, dr, sz);
	size_t lp = 0, lr = 0;
	const char *pp = P::_citrus_memory_stream_matchline(&mp, key, &lp, cs);
	const char *pr = ref__citrus_memory_stream_matchline(&mr, key, &lr, cs);
	if ((pp == nullptr) != (pr == nullptr)) fail(F_MS_MATCHLINE, "null");
	else if (pp && (off(dp, pp) != off(dr, pr) || lp != lr)) fail(F_MS_MATCHLINE, "ptr/len");
	else if (mp.ms_pos != mr.ms_pos) fail(F_MS_MATCHLINE, "pos");
	else if (std::memcmp(dp, dr, sz) != 0) fail(F_MS_MATCHLINE, "buf");
	else bump(F_MS_MATCHLINE);
}

static void test_matchline_edges(void)
{
	static const char content[] =
	    "key value1\n# comment\n\n  KEY  value2  \nother stuff\nKey\tdata\n";
	case_matchline((const unsigned char *)content, sizeof(content) - 1, "key", 0);
	case_matchline((const unsigned char *)content, sizeof(content) - 1, "key", 1);
	case_matchline((const unsigned char *)content, sizeof(content) - 1, "KEY", 0);
	case_matchline((const unsigned char *)content, sizeof(content) - 1, "missing", 0);
	case_matchline((const unsigned char *)content, sizeof(content) - 1, "", 0);
	case_matchline((const unsigned char *)content, sizeof(content) - 1, "other", 1);
}

static char rkey[64];
static unsigned char rcontent[256];

static void test_matchline_random(void)
{
	for (unsigned i = 0; i < 200000u; i++) {
		size_t sz = (size_t)(rng_next() % sizeof(rcontent));
		for (size_t j = 0; j < sz; j++) rcontent[j] = rb();
		fill_random(rkey, sizeof(rkey) - 1);
		case_matchline(rcontent, sz, rkey, (int)(rng_next() & 1u));
	}
}

static void case_chr(unsigned char *data, size_t sz, size_t start, char ch)
{
	unsigned char dp[512], dr[512];
	std::memcpy(dp, data, sz); std::memcpy(dr, data, sz);
	P::_citrus_memory_stream mp, mr;
	P::_citrus_region rp, rr;
	ms_bind(&mp, dp, sz); ref_ms_bind(&mr, dr, sz);
	mp.ms_pos = start; mr.ms_pos = start;
	void *pp = P::_citrus_memory_stream_chr(&mp, &rp, ch);
	void *pr = ref__citrus_memory_stream_chr(&mr, &rr, ch);
	if ((pp == nullptr) != (pr == nullptr)) fail(F_MS_CHR, "null");
	else if (pp && off(dp, pp) != off(dr, pr)) fail(F_MS_CHR, "ptr");
	else if (mp.ms_pos != mr.ms_pos) fail(F_MS_CHR, "pos");
	else if (P::_citrus_region_size(&rp) != P::_citrus_region_size(&rr) ||
	    (P::_citrus_region_size(&rp) > 0 &&
	    std::memcmp(P::_citrus_region_head(&rp), P::_citrus_region_head(&rr),
	    P::_citrus_region_size(&rp)) != 0)) fail(F_MS_CHR, "region");
	else if (std::memcmp(dp, dr, sz) != 0) fail(F_MS_CHR, "buf");
	else bump(F_MS_CHR);
}

static void test_chr_edges(void)
{
	unsigned char d1[] = "hello,world";
	case_chr(d1, sizeof(d1) - 1, 0, ',');
	case_chr(d1, sizeof(d1) - 1, 0, 'z');
	case_chr(d1, sizeof(d1) - 1, 6, 'w');
	case_chr(d1, sizeof(d1) - 1, 99, 'h');
	unsigned char d2[] = "\x80,\xff";
	case_chr(d2, sizeof(d2) - 1, 0, ',');
	unsigned char d3[] = "";
	case_chr(d3, 0, 0, 'a');
	unsigned char d4[] = "aaa";
	case_chr(d4, sizeof(d4) - 1, 0, 'a');
}

static void test_chr_random(void)
{
	unsigned char data[256];
	for (unsigned i = 0; i < 200000u; i++) {
		size_t sz = (size_t)(rng_next() % sizeof(data));
		for (size_t j = 0; j < sz; j++) data[j] = rb();
		case_chr(data, sz, sz > 0 ? (size_t)(rng_next() % (sz + 1)) : 0, (char)rb());
	}
}

static void case_skip_ws_ms(unsigned char *data, size_t sz, size_t start)
{
	unsigned char dp[512], dr[512];
	std::memcpy(dp, data, sz); std::memcpy(dr, data, sz);
	P::_citrus_memory_stream mp, mr;
	ms_bind(&mp, dp, sz); ref_ms_bind(&mr, dr, sz);
	mp.ms_pos = start; mr.ms_pos = start;
	P::_citrus_memory_stream_skip_ws(&mp);
	ref__citrus_memory_stream_skip_ws(&mr);
	if (mp.ms_pos != mr.ms_pos) fail(F_MS_SKIP_WS, "pos");
	else if (std::memcmp(dp, dr, sz) != 0) fail(F_MS_SKIP_WS, "buf");
	else bump(F_MS_SKIP_WS);
}

static void test_skip_ws_ms_edges(void)
{
	unsigned char d1[] = "   hello";
	case_skip_ws_ms(d1, sizeof(d1) - 1, 0);
	unsigned char d2[] = "hello";
	case_skip_ws_ms(d2, sizeof(d2) - 1, 0);
	unsigned char d3[] = " \t\n\r\f\v";
	case_skip_ws_ms(d3, sizeof(d3) - 1, 0);
	unsigned char d4[] = "";
	case_skip_ws_ms(d4, 0, 0);
	unsigned char d5[] = "\x80 ";
	case_skip_ws_ms(d5, sizeof(d5) - 1, 0);
}

static void test_skip_ws_ms_random(void)
{
	unsigned char data[256];
	for (unsigned i = 0; i < 200000u; i++) {
		size_t sz = (size_t)(rng_next() % sizeof(data));
		for (size_t j = 0; j < sz; j++) data[j] = rb();
		case_skip_ws_ms(data, sz, sz > 0 ? (size_t)(rng_next() % (sz + 1)) : 0);
	}
}

int main(void)
{
	test_strcasecmp_edges();
	test_strncasecmp_edges();
	test_skip_ws_edges();
	test_skip_nonws_edges();
	test_skip_ws_len_edges();
	test_skip_nonws_len_edges();
	test_trunc_rws_len_edges();
	test_convert_edges();
	test_getln_edges();
	test_matchline_edges();
	test_chr_edges();
	test_skip_ws_ms_edges();

	test_strcasecmp_random();
	test_strncasecmp_random();
	test_skip_ws_random();
	test_skip_nonws_random();
	test_skip_ws_len_random();
	test_skip_nonws_len_random();
	test_trunc_rws_len_random();
	test_convert_lower_random();
	test_convert_upper_random();
	test_getln_random();
	test_matchline_random();
	test_chr_random();
	test_skip_ws_ms_random();

	int anyfail = 0;
	std::printf("\n%-24s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-24s %12s %12s\n", "--------", "-----", "--------");
	for (int i = 0; i < F_COUNT; i++) {
		std::printf("%-24s %12llu %12llu\n", FNAME[i], NCASE[i], NFAIL[i]);
		if (NFAIL[i] != 0) anyfail = 1;
	}
	return anyfail ? 1 : 0;
}
