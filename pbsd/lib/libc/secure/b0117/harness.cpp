/*
 * harness.cpp -- differential test for PBSD batch b0117.
 */

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.secure.b0117;

namespace port = pbsd::lib_libc_secure::b0117;

extern "C" {
int oracle_chk_fail_setjmp(void);
int oracle_chk_fail_happened(void);

char *ref___stpncpy_chk(char * __restrict, const char * __restrict,
    std::size_t, std::size_t);
char *ref___strcpy_chk(char * __restrict, const char * __restrict,
    std::size_t);
int ref___snprintf_chk(char * __restrict, std::size_t, int, std::size_t,
    const char * __restrict, ...);
}

static constexpr unsigned char GUARD = 0x7f;
static constexpr std::size_t DBUF = 128;
static constexpr std::size_t SBUF = 128;
static constexpr long long SWEEP = 200000;
static constexpr int MAX_REPORT = 8;

enum {
	F_STPNCPY,
	F_STRCPY,
	F_SNPRINTF,
	F_COUNT
};

static const char *const fname[F_COUNT] = {
	"__stpncpy_chk",
	"__strcpy_chk",
	"__snprintf_chk",
};

static long long ncase[F_COUNT];
static long long nfail[F_COUNT];
static int reported[F_COUNT];

static std::uint64_t rng = 0xb0117ULL;

static std::uint64_t
rnd(void)
{
	rng ^= rng << 13;
	rng ^= rng >> 7;
	rng ^= rng << 17;
	return (rng);
}

static std::size_t
rnd_mod(std::size_t m)
{
	if (m == 0)
		return (0);
	return ((std::size_t)(rnd() % (std::uint64_t)m));
}

static void
fill_guard(unsigned char *p, std::size_t n)
{
	std::memset(p, GUARD, n);
}

static void
record_fail(int fn, const char *tag)
{
	nfail[fn]++;
	if (reported[fn] < MAX_REPORT) {
		reported[fn]++;
		std::printf("  FAIL %s [%s]\n", fname[fn], tag);
	}
}

static void
dump_buf(const char *label, const unsigned char *p, std::size_t n)
{
	std::printf("      %s:", label);
	for (std::size_t i = 0; i < n; i++)
		std::printf(" %02x", (unsigned)p[i]);
	std::printf("\n");
}

static int
call_stpncpy_port(char *dst, const char *src, std::size_t len, std::size_t slen,
    char **out, int *failed)
{
	if (oracle_chk_fail_setjmp() == 0) {
		*out = port::__stpncpy_chk(dst, src, len, slen);
		*failed = oracle_chk_fail_happened();
		return (0);
	}
	*out = nullptr;
	*failed = 1;
	return (1);
}

static int
call_stpncpy_ref(char *dst, const char *src, std::size_t len, std::size_t slen,
    char **out, int *failed)
{
	if (oracle_chk_fail_setjmp() == 0) {
		*out = ref___stpncpy_chk(dst, src, len, slen);
		*failed = oracle_chk_fail_happened();
		return (0);
	}
	*out = nullptr;
	*failed = 1;
	return (1);
}

static void
test_stpncpy_case(const char *src, std::size_t slen, std::size_t len,
    const char *tag)
{
	unsigned char da[DBUF], db[DBUF];
	unsigned char sa[SBUF], sb[SBUF];
	char *ra, *rb;
	int fa, fb;

	ncase[F_STPNCPY]++;

	fill_guard(da, sizeof(da));
	fill_guard(db, sizeof(db));
	fill_guard(sa, sizeof(sa));
	fill_guard(sb, sizeof(sb));

	std::size_t srclen = std::strlen(src);
	if (srclen + 1 > SBUF) {
		std::fprintf(stderr, "harness bug: stpncpy src overflow\n");
		std::exit(2);
	}
	std::memcpy(sa, src, srclen + 1);
	std::memcpy(sb, src, srclen + 1);

	call_stpncpy_port((char *)da, (const char *)sa, len, slen, &ra, &fa);
	call_stpncpy_ref((char *)db, (const char *)sb, len, slen, &rb, &fb);

	int bad_fail = (fa != fb);
	std::ptrdiff_t oa = fa ? -1 : (ra - (char *)da);
	std::ptrdiff_t ob = fb ? -1 : (rb - (char *)db);
	int bad_ret = (!bad_fail && (oa != ob));
	int bad_dst = (std::memcmp(da, db, sizeof(da)) != 0);
	int bad_src = (std::memcmp(sa, sb, sizeof(sa)) != 0);

	if (bad_fail || bad_ret || bad_dst || bad_src) {
		record_fail(F_STPNCPY, tag);
		if (reported[F_STPNCPY] <= MAX_REPORT) {
			std::printf("      len=%zu slen=%zu src=\"%s\" fa=%d fb=%d\n",
			    len, slen, src, fa, fb);
			std::printf("      port_off=%td ref_off=%td\n", oa, ob);
			dump_buf("port_dst", da, sizeof(da));
			dump_buf("ref_dst ", db, sizeof(db));
		}
	}
}

static void
test_stpncpy_overlap(std::size_t dst_off, std::size_t src_off,
    std::size_t len, std::size_t slen, const char *tag)
{
	unsigned char space_a[96], space_b[96];
	char *ra, *rb;
	int fa, fb;

	ncase[F_STPNCPY]++;

	fill_guard(space_a, sizeof(space_a));
	fill_guard(space_b, sizeof(space_b));
	const char *pat = "abcdefghijklmnop";
	std::memcpy(space_a + 16, pat, 17);
	std::memcpy(space_b + 16, pat, 17);

	char *da = (char *)space_a + dst_off;
	char *db = (char *)space_b + dst_off;
	const char *sa = (const char *)space_a + src_off;
	const char *sb = (const char *)space_b + src_off;

	call_stpncpy_port(da, sa, len, slen, &ra, &fa);
	call_stpncpy_ref(db, sb, len, slen, &rb, &fb);

	if (fa != fb || std::memcmp(space_a, space_b, sizeof(space_a)) != 0) {
		record_fail(F_STPNCPY, tag);
	}
}

static void
test_stpncpy_edges(void)
{
	static const char *const srcs[] = {
		"",
		"a",
		"z",
		"\x80",
		"\xff",
		"\x7f",
		"\x80\xff",
		"a\x00" "b",
		"abc",
		"abcdefghijklmnop",
		"\x80\x81\x82\x83",
		"\xfe\xfd\xfc",
	};

	for (const char *src : srcs) {
		test_stpncpy_case(src, 0, 0, "empty0");
		test_stpncpy_case(src, 1, 0, "slen1_len0");
		test_stpncpy_case(src, 1, 1, "slen1_len1");
		test_stpncpy_case(src, 2, 1, "slen2_len1");
		test_stpncpy_case(src, 2, 2, "slen2_len2");
		test_stpncpy_case(src, 32, 0, "len0");
		test_stpncpy_case(src, 32, 1, "len1");
		test_stpncpy_case(src, 32, 16, "len16");
		test_stpncpy_case(src, 32, 31, "len31");
		test_stpncpy_case(src, 32, 32, "len_eq_slen");
		std::size_t n = std::strlen(src);
		test_stpncpy_case(src, 32, n, "len_nul");
		test_stpncpy_case(src, 32, n + 1, "len_nul1");
		test_stpncpy_case(src, 32, n + 5, "len_past");
	}

	test_stpncpy_case("fail", 3, 4, "len_gt_slen");
	test_stpncpy_case("x", 0, 1, "len_gt_slen0");
	test_stpncpy_overlap(20, 22, 8, 32, "overlap");
	test_stpncpy_overlap(24, 20, 12, 32, "overlap_rev");
}

static void
test_stpncpy_sweep(void)
{
	for (long long i = 0; i < SWEEP; i++) {
		unsigned char da[DBUF], db[DBUF];
		unsigned char sa[SBUF], sb[SBUF];
		char *ra, *rb;
		int fa, fb;

		ncase[F_STPNCPY]++;

		fill_guard(da, sizeof(da));
		fill_guard(db, sizeof(db));
		fill_guard(sa, sizeof(sa));
		fill_guard(sb, sizeof(sb));

		std::size_t slen = rnd_mod(65);
		std::size_t len = rnd_mod(slen + 1);
		std::size_t fill = rnd_mod(SBUF - 1);
		for (std::size_t j = 0; j < fill; j++) {
			unsigned char c = (unsigned char)(rnd() & 0xff);
			sa[j] = sb[j] = c;
		}
		std::size_t nul = rnd_mod(fill + 1);
		sa[nul] = sb[nul] = '\0';

		call_stpncpy_port((char *)da, (const char *)sa, len, slen, &ra, &fa);
		call_stpncpy_ref((char *)db, (const char *)sb, len, slen, &rb, &fb);

		std::ptrdiff_t oa = fa ? -1 : (ra - (char *)da);
		std::ptrdiff_t ob = fb ? -1 : (rb - (char *)db);
		if (fa != fb || oa != ob ||
		    std::memcmp(da, db, sizeof(da)) != 0 ||
		    std::memcmp(sa, sb, sizeof(sa)) != 0)
			record_fail(F_STPNCPY, "sweep");
	}
}

static int
call_strcpy_port(char *dst, const char *src, std::size_t slen, char **out,
    int *failed)
{
	if (oracle_chk_fail_setjmp() == 0) {
		*out = port::__strcpy_chk(dst, src, slen);
		*failed = oracle_chk_fail_happened();
		return (0);
	}
	*out = nullptr;
	*failed = 1;
	return (1);
}

static int
call_strcpy_ref(char *dst, const char *src, std::size_t slen, char **out,
    int *failed)
{
	if (oracle_chk_fail_setjmp() == 0) {
		*out = ref___strcpy_chk(dst, src, slen);
		*failed = oracle_chk_fail_happened();
		return (0);
	}
	*out = nullptr;
	*failed = 1;
	return (1);
}

static void
test_strcpy_case(const char *src, std::size_t slen, const char *tag)
{
	unsigned char da[DBUF], db[DBUF];
	unsigned char sa[SBUF], sb[SBUF];
	char *ra, *rb;
	int fa, fb;

	ncase[F_STRCPY]++;

	fill_guard(da, sizeof(da));
	fill_guard(db, sizeof(db));
	fill_guard(sa, sizeof(sa));
	fill_guard(sb, sizeof(sb));

	std::size_t srclen = std::strlen(src);
	std::memcpy(sa, src, srclen + 1);
	std::memcpy(sb, src, srclen + 1);

	call_strcpy_port((char *)da, (const char *)sa, slen, &ra, &fa);
	call_strcpy_ref((char *)db, (const char *)sb, slen, &rb, &fb);

	std::ptrdiff_t oa = fa ? -1 : (ra - (char *)da);
	std::ptrdiff_t ob = fb ? -1 : (rb - (char *)db);
	if (fa != fb || oa != ob ||
	    std::memcmp(da, db, sizeof(da)) != 0 ||
	    std::memcmp(sa, sb, sizeof(sa)) != 0) {
		record_fail(F_STRCPY, tag);
	}
}

static void
test_strcpy_overlap(std::size_t dst_off, std::size_t src_off,
    std::size_t slen, const char *tag)
{
	unsigned char space_a[96], space_b[96];
	char *ra, *rb;
	int fa, fb;

	ncase[F_STRCPY]++;

	fill_guard(space_a, sizeof(space_a));
	fill_guard(space_b, sizeof(space_b));
	const char *pat = "overlap-test";
	std::memcpy(space_a + 16, pat, 13);
	std::memcpy(space_b + 16, pat, 13);

	call_strcpy_port((char *)space_a + dst_off, (const char *)space_a + src_off,
	    slen, &ra, &fa);
	call_strcpy_ref((char *)space_b + dst_off, (const char *)space_b + src_off,
	    slen, &rb, &fb);

	if (fa != fb || std::memcmp(space_a, space_b, sizeof(space_a)) != 0)
		record_fail(F_STRCPY, tag);
}

static void
test_strcpy_edges(void)
{
	static const char *const srcs[] = {
		"",
		"a",
		"z",
		"\x80",
		"\xff",
		"\x7f",
		"\x80\xff",
		"abc",
		"abcdefghijklmnop",
		"\x80\x81\x82",
		"\xfe\xfd\xfc\xfb",
	};

	for (const char *src : srcs) {
		std::size_t need = std::strlen(src) + 1;
		test_strcpy_case(src, need, "exact");
		test_strcpy_case(src, need + 1, "exact+1");
		test_strcpy_case(src, need + 8, "roomy");
		test_strcpy_case(src, 0, "slen0");
		if (need > 1)
			test_strcpy_case(src, need - 1, "too_small");
	}

	test_strcpy_overlap(20, 24, 32, "overlap");
	test_strcpy_overlap(24, 20, 32, "overlap_rev");
}

static void
test_strcpy_sweep(void)
{
	for (long long i = 0; i < SWEEP; i++) {
		unsigned char da[DBUF], db[DBUF];
		unsigned char sa[SBUF], sb[SBUF];
		char *ra, *rb;
		int fa, fb;

		ncase[F_STRCPY]++;

		fill_guard(da, sizeof(da));
		fill_guard(db, sizeof(db));
		fill_guard(sa, sizeof(sa));
		fill_guard(sb, sizeof(sb));

		std::size_t fill = rnd_mod(SBUF - 1);
		for (std::size_t j = 0; j < fill; j++) {
			unsigned char c = (unsigned char)(rnd() & 0xff);
			sa[j] = sb[j] = c;
		}
		std::size_t nul = rnd_mod(fill + 1);
		sa[nul] = sb[nul] = '\0';
		std::size_t need = std::strlen((char *)sa) + 1;
		std::size_t slen = need + rnd_mod(16);

		call_strcpy_port((char *)da, (const char *)sa, slen, &ra, &fa);
		call_strcpy_ref((char *)db, (const char *)sb, slen, &rb, &fb);

		std::ptrdiff_t oa = fa ? -1 : (ra - (char *)da);
		std::ptrdiff_t ob = fb ? -1 : (rb - (char *)db);
		if (fa != fb || oa != ob ||
		    std::memcmp(da, db, sizeof(da)) != 0 ||
		    std::memcmp(sa, sb, sizeof(sa)) != 0)
			record_fail(F_STRCPY, "sweep");
	}
}

template<typename... Args>
static void
test_snprintf_case(std::size_t len, int flags, std::size_t slen,
    const char *tag, const char *fmt, Args... args)
{
	unsigned char ba[DBUF], bb[DBUF];
	int ra, rb;
	int fa, fb;

	ncase[F_SNPRINTF]++;

	fill_guard(ba, sizeof(ba));
	fill_guard(bb, sizeof(bb));

	if (oracle_chk_fail_setjmp() == 0) {
		ra = port::__snprintf_chk((char *)ba, len, flags, slen, fmt,
		    args...);
		fa = oracle_chk_fail_happened();
	} else {
		ra = -1;
		fa = 1;
	}

	if (oracle_chk_fail_setjmp() == 0) {
		rb = ref___snprintf_chk((char *)bb, len, flags, slen, fmt,
		    args...);
		fb = oracle_chk_fail_happened();
	} else {
		rb = -1;
		fb = 1;
	}

	if (fa != fb || (!fa && ra != rb) ||
	    std::memcmp(ba, bb, sizeof(ba)) != 0) {
		record_fail(F_SNPRINTF, tag);
		if (reported[F_SNPRINTF] <= MAX_REPORT) {
			std::printf("      len=%zu slen=%zu fmt=\"%s\" fa=%d fb=%d\n",
			    len, slen, fmt, fa, fb);
			std::printf("      port_ret=%d ref_ret=%d\n", ra, rb);
			dump_buf("port_buf", ba, sizeof(ba));
			dump_buf("ref_buf ", bb, sizeof(bb));
		}
	}
}

static void
test_snprintf_edges(void)
{
	test_snprintf_case(0, 0, 0, "empty0", "");
	test_snprintf_case(0, 1, 0, "len0_slen0", "");
	test_snprintf_case(1, 0, 1, "len1_empty", "");
	test_snprintf_case(8, 0, 8, "len8_empty", "");
	test_snprintf_case(8, 0, 8, "char", "x");
	test_snprintf_case(8, 0, 8, "pct_c", "%c", 'Z');
	test_snprintf_case(8, 0, 8, "pct_d", "%d", -42);
	test_snprintf_case(8, 0, 8, "pct_u", "%u", 4294967295U);
	test_snprintf_case(16, 0, 16, "pct_s", "%s", "hello");
	test_snprintf_case(16, 0, 16, "pct_prec", "%.3s", "abcdef");
	test_snprintf_case(16, 0, 16, "pct_pct", "%%");
	test_snprintf_case(16, 0, 16, "pct_x", "%x", 0xdeadbeef);
	test_snprintf_case(16, 0, 16, "pct_p", "%p", (void *)0x1234);
	test_snprintf_case(32, 7, 32, "flags7", "%d %s", 123, "ok");
	test_snprintf_case(4, 0, 4, "trunc", "%s", "toolong");
	test_snprintf_case(8, 0, 8, "high", "%s", "\x80\xff");
	test_snprintf_case(8, 0, 7, "len_eq_slen", "%d", 1);
	test_snprintf_case(8, 0, 8, "len_eq_slen8", "%d", 1);
	test_snprintf_case(5, 0, 4, "len_gt_slen", "%d", 9);
	test_snprintf_case(1, 0, 0, "len_gt_slen0", "%d", 0);
}

static void
test_snprintf_sweep(void)
{
	for (long long i = 0; i < SWEEP; i++) {
		unsigned char ba[DBUF], bb[DBUF];
		int ra, rb;
		int fa, fb;
		std::size_t slen = rnd_mod(65);
		std::size_t len = rnd_mod(slen + 1);
		int flags = (int)(rnd() & 0x7);
		char strbuf[16];
		unsigned j;
		unsigned pick = (unsigned)rnd_mod(12);

		ncase[F_SNPRINTF]++;

		fill_guard(ba, sizeof(ba));
		fill_guard(bb, sizeof(bb));

		for (j = 0; j < sizeof(strbuf) - 1; j++)
			strbuf[j] = (char)(unsigned char)(rnd() & 0xff);
		strbuf[j] = '\0';

		int a0 = (int)(rnd() & 0xffff);
		unsigned u0 = (unsigned)(rnd() & 0xffffffffu);
		int prec = (int)(rnd_mod(8));
		int ch = (int)(unsigned char)(rnd() & 0xff);

		if (oracle_chk_fail_setjmp() == 0) {
			switch (pick) {
			case 0:
				ra = port::__snprintf_chk((char *)ba, len, flags,
				    slen, "");
				break;
			case 1:
				ra = port::__snprintf_chk((char *)ba, len, flags,
				    slen, "%d", a0);
				break;
			case 2:
				ra = port::__snprintf_chk((char *)ba, len, flags,
				    slen, "%u", u0);
				break;
			case 3:
				ra = port::__snprintf_chk((char *)ba, len, flags,
				    slen, "%x", u0);
				break;
			case 4:
				ra = port::__snprintf_chk((char *)ba, len, flags,
				    slen, "%c", ch);
				break;
			case 5:
				ra = port::__snprintf_chk((char *)ba, len, flags,
				    slen, "%s", strbuf);
				break;
			case 6:
				ra = port::__snprintf_chk((char *)ba, len, flags,
				    slen, "%%");
				break;
			case 7:
				ra = port::__snprintf_chk((char *)ba, len, flags,
				    slen, "%d %s", a0, strbuf);
				break;
			case 8:
				ra = port::__snprintf_chk((char *)ba, len, flags,
				    slen, "<%d>", a0);
				break;
			case 9:
				ra = port::__snprintf_chk((char *)ba, len, flags,
				    slen, "%.*s", prec, strbuf);
				break;
			case 10:
				ra = port::__snprintf_chk((char *)ba, len, flags,
				    slen, "%X", u0);
				break;
			default:
				ra = port::__snprintf_chk((char *)ba, len, flags,
				    slen, "%i", a0);
				break;
			}
			fa = oracle_chk_fail_happened();
		} else {
			ra = -1;
			fa = 1;
		}

		if (oracle_chk_fail_setjmp() == 0) {
			switch (pick) {
			case 0:
				rb = ref___snprintf_chk((char *)bb, len, flags,
				    slen, "");
				break;
			case 1:
				rb = ref___snprintf_chk((char *)bb, len, flags,
				    slen, "%d", a0);
				break;
			case 2:
				rb = ref___snprintf_chk((char *)bb, len, flags,
				    slen, "%u", u0);
				break;
			case 3:
				rb = ref___snprintf_chk((char *)bb, len, flags,
				    slen, "%x", u0);
				break;
			case 4:
				rb = ref___snprintf_chk((char *)bb, len, flags,
				    slen, "%c", ch);
				break;
			case 5:
				rb = ref___snprintf_chk((char *)bb, len, flags,
				    slen, "%s", strbuf);
				break;
			case 6:
				rb = ref___snprintf_chk((char *)bb, len, flags,
				    slen, "%%");
				break;
			case 7:
				rb = ref___snprintf_chk((char *)bb, len, flags,
				    slen, "%d %s", a0, strbuf);
				break;
			case 8:
				rb = ref___snprintf_chk((char *)bb, len, flags,
				    slen, "<%d>", a0);
				break;
			case 9:
				rb = ref___snprintf_chk((char *)bb, len, flags,
				    slen, "%.*s", prec, strbuf);
				break;
			case 10:
				rb = ref___snprintf_chk((char *)bb, len, flags,
				    slen, "%X", u0);
				break;
			default:
				rb = ref___snprintf_chk((char *)bb, len, flags,
				    slen, "%i", a0);
				break;
			}
			fb = oracle_chk_fail_happened();
		} else {
			rb = -1;
			fb = 1;
		}

		if (fa != fb || (!fa && ra != rb) ||
		    std::memcmp(ba, bb, sizeof(ba)) != 0)
			record_fail(F_SNPRINTF, "sweep");
	}
}

int
main(void)
{
	test_stpncpy_edges();
	test_stpncpy_sweep();
	test_strcpy_edges();
	test_strcpy_sweep();
	test_snprintf_edges();
	test_snprintf_sweep();

	std::printf("\n%-20s %10s %10s\n", "function", "cases", "failures");
	for (int i = 0; i < F_COUNT; i++)
		std::printf("%-20s %10lld %10lld\n", fname[i], ncase[i],
		    nfail[i]);

	long long total_fail = 0;
	for (int i = 0; i < F_COUNT; i++)
		total_fail += nfail[i];

	return (total_fail == 0 ? 0 : 1);
}
