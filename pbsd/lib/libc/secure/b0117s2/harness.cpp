/*
 * harness.cpp -- differential test for PBSD batch b0117s2.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.secure.b0117s2;

namespace port = pbsd::lib_libc_secure::b0117s2;

extern "C" {
int oracle_chk_fail_setjmp(void);
int oracle_chk_fail_happened(void);

char *ref___strcpy_chk(char * __restrict, const char * __restrict,
    std::size_t);
}

static constexpr unsigned char GUARD = 0x7f;
static constexpr std::size_t DBUF = 128;
static constexpr std::size_t SBUF = 128;
static constexpr long long SWEEP = 200000;
static constexpr int MAX_REPORT = 8;

enum {
	F_STRCPY,
	F_COUNT
};

static const char *const fname[F_COUNT] = {
	"__strcpy_chk",
};

static long long ncase[F_COUNT];
static long long nfail[F_COUNT];
static int reported[F_COUNT];

static std::uint64_t rng = 0xb0117s2ULL;

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
	if (srclen + 1 > SBUF) {
		std::fprintf(stderr, "harness bug: strcpy src overflow\n");
		std::exit(2);
	}
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
		"\x00\x00\x00",
		"a\x00b",
		"\xff\xff\xff\xff",
	};

	for (const char *src : srcs) {
		std::size_t need = std::strlen(src) + 1;
		test_strcpy_case(src, need, "exact");
		test_strcpy_case(src, need + 1, "exact+1");
		test_strcpy_case(src, need + 8, "roomy");
		test_strcpy_case(src, 0, "slen0");
		if (need > 1) {
			test_strcpy_case(src, need - 1, "too_small");
			test_strcpy_case(src, 1, "slen1");
		}
	}

	test_strcpy_overlap(20, 24, 32, "overlap");
	test_strcpy_overlap(24, 20, 32, "overlap_rev");
	test_strcpy_overlap(20, 28, 32, "overlap_tight");
	test_strcpy_overlap(28, 20, 32, "overlap_tight_rev");
	test_strcpy_overlap(16, 40, 64, "no_overlap");
	test_strcpy_overlap(40, 16, 64, "no_overlap_rev");

	/* Adjacent: dst ends where src begins (non-overlapping). */
	test_strcpy_overlap(16, 29, 64, "adjacent");
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
			record_fail(F_STRCPY, "sweep_sep");
	}

	for (long long i = 0; i < SWEEP / 4; i++) {
		unsigned char space_a[96], space_b[96];
		char *ra, *rb;
		int fa, fb;

		ncase[F_STRCPY]++;

		fill_guard(space_a, sizeof(space_a));
		fill_guard(space_b, sizeof(space_b));

		std::size_t base = 8 + rnd_mod(32);
		std::size_t srclen = rnd_mod(24);
		for (std::size_t j = 0; j < srclen; j++) {
			unsigned char c = (unsigned char)(rnd() & 0xff);
			space_a[base + j] = space_b[base + j] = c;
		}
		space_a[base + srclen] = space_b[base + srclen] = '\0';
		std::size_t need = srclen + 1;

		std::size_t gap = rnd_mod(8);
		std::size_t dst_off = base;
		std::size_t src_off = base + need + gap;
		if (src_off + need > sizeof(space_a) - 4)
			src_off = base + 4;
		if (rnd() & 1) {
			std::size_t tmp = dst_off;
			dst_off = src_off;
			src_off = tmp;
		}

		std::size_t slen = need + rnd_mod(12);
		if ((rnd() & 3) == 0 && slen > 0)
			slen--;

		call_strcpy_port((char *)space_a + dst_off,
		    (const char *)space_a + src_off, slen, &ra, &fa);
		call_strcpy_ref((char *)space_b + dst_off,
		    (const char *)space_b + src_off, slen, &rb, &fb);

		if (fa != fb || std::memcmp(space_a, space_b, sizeof(space_a)) != 0)
			record_fail(F_STRCPY, "sweep_overlap");
	}
}

int
main(void)
{
	test_strcpy_edges();
	test_strcpy_sweep();

	std::printf("\n%-20s %10s %10s\n", "function", "cases", "failures");
	for (int i = 0; i < F_COUNT; i++)
		std::printf("%-20s %10lld %10lld\n", fname[i], ncase[i],
		    nfail[i]);

	long long total_fail = 0;
	for (int i = 0; i < F_COUNT; i++)
		total_fail += nfail[i];

	return (total_fail == 0 ? 0 : 1);
}
