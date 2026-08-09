/*
 * harness.cpp -- differential test for PBSD batch b0117s1.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.secure.b0117s1;

namespace port = pbsd::lib_libc_secure::b0117s1;

extern "C" {
int oracle_chk_fail_setjmp(void);
int oracle_chk_fail_happened(void);

char *ref___stpncpy_chk(char * __restrict, const char * __restrict,
    std::size_t, std::size_t);
}

static constexpr unsigned char GUARD = 0x7f;
static constexpr std::size_t DBUF = 128;
static constexpr std::size_t SBUF = 128;
static constexpr long long SWEEP = 200000;
static constexpr int MAX_REPORT = 8;

enum {
	F_STPNCPY,
	F_COUNT
};

static const char *const fname[F_COUNT] = {
	"__stpncpy_chk",
};

static long long ncase[F_COUNT];
static long long nfail[F_COUNT];
static int reported[F_COUNT];

static std::uint64_t rng = 0xb011701ULL;

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
test_stpncpy_same_ptr(std::size_t len, std::size_t slen, const char *tag)
{
	unsigned char space_a[64], space_b[64];
	char *ra, *rb;
	int fa, fb;

	ncase[F_STPNCPY]++;

	fill_guard(space_a, sizeof(space_a));
	fill_guard(space_b, sizeof(space_b));
	const char *pat = "same-pointer";
	std::memcpy(space_a + 8, pat, 13);
	std::memcpy(space_b + 8, pat, 13);

	char *da = (char *)space_a + 8;
	char *db = (char *)space_b + 8;

	call_stpncpy_port(da, da, len, slen, &ra, &fa);
	call_stpncpy_ref(db, db, len, slen, &rb, &fb);

	if (fa != fb || std::memcmp(space_a, space_b, sizeof(space_a)) != 0)
		record_fail(F_STPNCPY, tag);
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
		"\x80\x80\x80\x80\x80",
		"no-nul-in-prefix\xff",
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
	test_stpncpy_case("ab", 2, 2, "len_eq_slen_exact");
	test_stpncpy_case("ab", 2, 3, "len_gt_slen_by1");

	test_stpncpy_overlap(20, 22, 8, 32, "overlap_fwd");
	test_stpncpy_overlap(24, 20, 12, 32, "overlap_rev");
	test_stpncpy_overlap(20, 20, 4, 32, "overlap_touch");
	test_stpncpy_overlap(20, 28, 1, 32, "overlap_one");
	test_stpncpy_overlap(28, 20, 1, 32, "overlap_one_rev");

	test_stpncpy_same_ptr(4, 32, "same_ptr_len4");
	test_stpncpy_same_ptr(1, 32, "same_ptr_len1");
	test_stpncpy_same_ptr(0, 32, "same_ptr_len0");

	test_stpncpy_overlap(16, 40, 8, 64, "no_overlap_far");
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

static void
test_stpncpy_overlap_sweep(void)
{
	for (long long i = 0; i < SWEEP / 10; i++) {
		unsigned char space_a[96], space_b[96];
		char *ra, *rb;
		int fa, fb;

		ncase[F_STPNCPY]++;

		fill_guard(space_a, sizeof(space_a));
		fill_guard(space_b, sizeof(space_b));
		const char *pat = "overlap-sweep-data";
		std::memcpy(space_a + 8, pat, 19);
		std::memcpy(space_b + 8, pat, 19);

		std::size_t dst_off = 8 + rnd_mod(40);
		std::size_t src_off = 8 + rnd_mod(40);
		std::size_t slen = 32 + rnd_mod(32);
		std::size_t len = rnd_mod(slen + 1);

		char *da = (char *)space_a + dst_off;
		char *db = (char *)space_b + dst_off;
		const char *sa = (const char *)space_a + src_off;
		const char *sb = (const char *)space_b + src_off;

		call_stpncpy_port(da, sa, len, slen, &ra, &fa);
		call_stpncpy_ref(db, sb, len, slen, &rb, &fb);

		if (fa != fb || std::memcmp(space_a, space_b, sizeof(space_a)) != 0)
			record_fail(F_STPNCPY, "overlap_sweep");
	}
}

int
main(void)
{
	test_stpncpy_edges();
	test_stpncpy_sweep();
	test_stpncpy_overlap_sweep();

	std::printf("\n%-20s %10s %10s\n", "function", "cases", "failures");
	for (int i = 0; i < F_COUNT; i++)
		std::printf("%-20s %10lld %10lld\n", fname[i], ncase[i],
		    nfail[i]);

	long long total_fail = 0;
	for (int i = 0; i < F_COUNT; i++)
		total_fail += nfail[i];

	return (total_fail == 0 ? 0 : 1);
}
