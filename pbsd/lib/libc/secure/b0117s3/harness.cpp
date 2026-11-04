/*
 * harness.cpp -- differential test for PBSD batch b0117s3.
 */

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.secure.b0117s3;

namespace port = pbsd::lib_libc_secure::b0117s3;

extern "C" {
int oracle_chk_fail_setjmp(void);
int oracle_chk_fail_happened(void);

int ref___snprintf_chk(char * __restrict, std::size_t, int, std::size_t,
    const char * __restrict, ...);
}

static constexpr unsigned char GUARD = 0x7f;
static constexpr std::size_t DBUF = 128;
static constexpr long long SWEEP = 200000;
static constexpr int MAX_REPORT = 8;

enum {
	F_SNPRINTF,
	F_COUNT
};

static const char *const fname[F_COUNT] = {
	"__snprintf_chk",
};

static long long ncase[F_COUNT];
static long long nfail[F_COUNT];
static int reported[F_COUNT];

static std::uint64_t rng = 0xb011703ULL;

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
	test_snprintf_case(8, 0, 8, "pct_c_hi", "%c", (char)0x80);
	test_snprintf_case(8, 0, 8, "pct_c_ff", "%c", (char)0xff);
	test_snprintf_case(8, 0, 8, "pct_d", "%d", -42);
	test_snprintf_case(8, 0, 8, "pct_u", "%u", 4294967295U);
	test_snprintf_case(16, 0, 16, "pct_s", "%s", "hello");
	test_snprintf_case(16, 0, 16, "pct_s_empty", "%s", "");
	test_snprintf_case(16, 0, 16, "pct_prec", "%.3s", "abcdef");
	test_snprintf_case(16, 0, 16, "pct_pct", "%%");
	test_snprintf_case(16, 0, 16, "pct_x", "%x", 0xdeadbeef);
	test_snprintf_case(16, 0, 16, "pct_p", "%p", (void *)0x1234);
	test_snprintf_case(32, 7, 32, "flags7", "%d %s", 123, "ok");
	test_snprintf_case(4, 0, 4, "trunc", "%s", "toolong");
	test_snprintf_case(8, 0, 8, "high", "%s", "\x80\xff");
	test_snprintf_case(8, 0, 8, "nul_heavy", "%s", "a\x00b");
	test_snprintf_case(8, 0, 7, "len_eq_slen", "%d", 1);
	test_snprintf_case(8, 0, 8, "len_eq_slen8", "%d", 1);
	test_snprintf_case(5, 0, 4, "len_gt_slen", "%d", 9);
	test_snprintf_case(1, 0, 0, "len_gt_slen0", "%d", 0);
	test_snprintf_case(2, 0, 1, "len2_slen1", "%c", 'a');
	test_snprintf_case(16, 0, 15, "boundary_slen", "%s", "abcdefghijklmnop");
	test_snprintf_case(16, 0, 16, "boundary_ok", "%s", "abcdefghijklmnop");
	test_snprintf_case(1, 0, 1, "single_byte", "%c", '\x7f');
	test_snprintf_case(32, 0, 32, "high_bytes", "%s", "\x80\x81\x82\x83\xfe\xff");
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
