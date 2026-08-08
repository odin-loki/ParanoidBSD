// Differential test harness for PBSD batch b0239.
//
// Every ported function is exercised against the unmodified reference
// implementation in oracle.c.  Buffer-writing functions use two separate
// arenas pre-filled with guard byte 0x7f; the identical input is fed to both
// implementations and the WHOLE arena is compared afterwards.  Pointer results
// from malloc are compared by nullness and full string content.

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

import pbsd.lib.libc.string.b0239;

namespace P = pbsd::lib_libc_string::b0239;

extern "C" {
void ref_swab(const void *__restrict from, void *__restrict to, ssize_t len);
char *ref_strndup(const char *str, size_t maxlen);
int ref___timingsafe_bcmp(const void *b1, const void *b2, size_t n);
}

/* ------------------------------------------------------------------ */
/* bookkeeping                                                        */
/* ------------------------------------------------------------------ */

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	int shown;
};

static Stat st_swab = { "swab", 0, 0, 0 };
static Stat st_strndup = { "strndup", 0, 0, 0 };
static Stat st_timingsafe_bcmp = { "__timingsafe_bcmp", 0, 0, 0 };

static const int MAX_SHOW = 8;
static const unsigned char GUARD = 0x7f;

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
	std::uint64_t s;

	explicit Rng(std::uint64_t seed) : s(seed) {}

	std::uint64_t next()
	{
		s += 0x9E3779B97F4A7C15ull;
		std::uint64_t z = s;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}

	std::uint32_t below(std::uint32_t n) { return (std::uint32_t)(next() % n); }
};

/* ------------------------------------------------------------------ */
/* swab                                                               */
/* ------------------------------------------------------------------ */

static const size_t SWAB_ARENA = 320;
static const size_t SWAB_LEAD = 32;
static const size_t SWAB_MAXLEN = 96;

static void
check_swab(const unsigned char *src, size_t srclen, ssize_t len, bool overlap)
{
	unsigned char a[SWAB_ARENA], b[SWAB_ARENA];

	std::memset(a, GUARD, SWAB_ARENA);
	std::memset(b, GUARD, SWAB_ARENA);

	unsigned char *from_a = a + SWAB_LEAD;
	unsigned char *from_b = b + SWAB_LEAD;
	unsigned char *to_a = from_a + SWAB_MAXLEN;
	unsigned char *to_b = from_b + SWAB_MAXLEN;

	size_t copylen = srclen;
	if (copylen > SWAB_MAXLEN)
		copylen = SWAB_MAXLEN;
	std::memcpy(from_a, src, copylen);
	std::memcpy(from_b, src, copylen);

	if (overlap) {
		P::swab(from_a, from_a, len);
		ref_swab(from_b, from_b, len);
	} else {
		P::swab(from_a, to_a, len);
		ref_swab(from_b, to_b, len);
	}

	st_swab.cases++;
	if (std::memcmp(a, b, SWAB_ARENA) != 0) {
		if (begin_fail(st_swab, "arena mismatch")) {
			long d = first_diff(a, b, SWAB_ARENA);
			std::printf("    first diff at %ld len=%zd overlap=%d\n", d,
			    (size_t)len, overlap ? 1 : 0);
		}
	}
}

static void
swab_edge_cases(void)
{
	static const ssize_t lens[] = {
		(ssize_t)0, (ssize_t)1, (ssize_t)2, (ssize_t)3, (ssize_t)4,
		(ssize_t)5, (ssize_t)7, (ssize_t)8, (ssize_t)9, (ssize_t)15,
		(ssize_t)16, (ssize_t)17, (ssize_t)31, (ssize_t)32, (ssize_t)33,
		(ssize_t)63, (ssize_t)64, (ssize_t)65, (ssize_t)127, (ssize_t)128,
		(ssize_t)-1, (ssize_t)-2, (ssize_t)-3
	};

	unsigned char src[SWAB_MAXLEN];

	for (unsigned pat = 0; pat < 8; pat++) {
		for (size_t i = 0; i < SWAB_MAXLEN; i++) {
			switch (pat) {
			case 0:
				src[i] = 0x00;
				break;
			case 1:
				src[i] = 0xff;
				break;
			case 2:
				src[i] = 0x80;
				break;
			case 3:
				src[i] = (unsigned char)(0x80 + (i & 0x7f));
				break;
			case 4:
				src[i] = (unsigned char)((i & 1) ? 0x00 : 0xfe);
				break;
			case 5:
				src[i] = (unsigned char)(i * 3 + 1);
				break;
			case 6:
				src[i] = (unsigned char)((i % 2) ? 0x12 : 0x34);
				break;
			default:
				src[i] = (unsigned char)(0x55 ^ (unsigned char)i);
				break;
			}
		}

		for (size_t li = 0; li < sizeof(lens) / sizeof(lens[0]); li++) {
			ssize_t len = lens[li];
			check_swab(src, SWAB_MAXLEN, len, false);
			check_swab(src, SWAB_MAXLEN, len, true);
			check_swab(src, len > 0 ? (size_t)len : 0, len, false);
			check_swab(src, len > 0 ? (size_t)len : 0, len, true);
		}
	}

	/* empty / single-byte source */
	unsigned char empty = 0xab;
	check_swab(&empty, 1, 0, false);
	check_swab(&empty, 1, 1, false);
	check_swab(&empty, 1, 2, false);
	check_swab(&empty, 1, 2, true);
}

static void
swab_random_sweep(Rng &rng, unsigned long long iters)
{
	unsigned char src[SWAB_MAXLEN];

	for (unsigned long long it = 0; it < iters; it++) {
		size_t srclen = 1 + rng.below(SWAB_MAXLEN);
		ssize_t len = (ssize_t)rng.below(SWAB_MAXLEN + 1);
		if ((it & 7) == 0)
			len = (ssize_t)(rng.below(5)) - 2;
		if ((it & 15) == 1)
			len = (ssize_t)srclen;

		for (size_t i = 0; i < srclen; i++) {
			if ((it & 3) == 0)
				src[i] = (unsigned char)(0x80 | (rng.next() & 0x7f));
			else
				src[i] = (unsigned char)(rng.next() & 0xff);
		}

		check_swab(src, srclen, len, (it & 1) != 0);
	}
}

/* ------------------------------------------------------------------ */
/* strndup                                                            */
/* ------------------------------------------------------------------ */

static const size_t SD_BUF = 128;
static const size_t SD_MAXSTR = 96;

static void
check_strndup(const char *str, size_t maxlen)
{
	char *pa = P::strndup(str, maxlen);
	char *pb = ref_strndup(str, maxlen);

	st_strndup.cases++;
	bool bad = false;

	if ((pa == nullptr) != (pb == nullptr)) {
		bad = true;
		if (begin_fail(st_strndup, "null mismatch"))
			std::printf("    port=%p ref=%p maxlen=%zu\n",
			    (void *)pa, (void *)pb, maxlen);
	}

	if (!bad && pa != nullptr) {
		size_t expect = std::strnlen(str, maxlen);
		if (std::memcmp(pa, pb, expect + 1) != 0) {
			bad = true;
			if (begin_fail(st_strndup, "content mismatch")) {
				long d = first_diff(pa, pb, expect + 1);
				std::printf("    diff at %ld maxlen=%zu\n", d,
				    maxlen);
			}
		}
		if (pa[expect] != '\0') {
			bad = true;
			if (begin_fail(st_strndup, "missing NUL terminator"))
				std::printf("    maxlen=%zu len=%zu\n", maxlen,
				    expect);
		}
	}

	std::free(pa);
	std::free(pb);
}

static void
strndup_edge_cases(void)
{
	static const size_t maxlens[] = {
		0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65,
		96, 127, 128, SIZE_MAX
	};

	char buf[SD_BUF];

	for (unsigned pat = 0; pat < 10; pat++) {
		for (size_t slen = 0; slen <= SD_MAXSTR; slen++) {
			for (size_t i = 0; i < slen; i++) {
				switch (pat) {
				case 0:
					buf[i] = '\0';
					break;
				case 1:
					buf[i] = 'a';
					break;
				case 2:
					buf[i] = (char)(0x80 + (i & 0x3f));
					break;
				case 3:
					buf[i] = (char)(i & 1 ? '\xff' : 'Z');
					break;
				case 4:
					buf[i] = (char)(1 + (i % 126));
					break;
				case 5:
					buf[i] = '\xff';
					break;
				case 6:
					buf[i] = (char)((i % 3) == 0 ? '\0' : 'x');
					break;
				case 7:
					buf[i] = (char)(0x7f);
					break;
				case 8:
					buf[i] = (char)(i == slen / 2 ? '\0' : 'q');
					break;
				default:
					buf[i] = (char)(0x80 | (unsigned char)i);
					break;
				}
			}
			buf[slen] = '\0';

			for (size_t mi = 0;
			    mi < sizeof(maxlens) / sizeof(maxlens[0]); mi++)
				check_strndup(buf, maxlens[mi]);
		}
	}

	check_strndup("", 0);
	check_strndup("", 1);
	check_strndup("x", 0);
	check_strndup("\xff\xfe", 4);
	check_strndup("no-nul-in-prefix", 3);
}

static void
strndup_random_sweep(Rng &rng, unsigned long long iters)
{
	char buf[SD_BUF];

	for (unsigned long long it = 0; it < iters; it++) {
		size_t slen = rng.below(SD_MAXSTR + 1);
		size_t maxlen = rng.below(SD_BUF);

		for (size_t i = 0; i < slen; i++) {
			if ((it & 3) == 0)
				buf[i] = (char)(0x80 | (rng.next() & 0x7f));
			else if ((it & 3) == 1)
				buf[i] = (char)(rng.next() & 0xff);
			else
				buf[i] = (char)(1 + rng.below(255));
		}
		buf[slen] = '\0';

		if ((it & 31) == 0) {
			size_t early = rng.below(slen + 1);
			buf[early] = '\0';
		}

		check_strndup(buf, maxlen);
	}
}

/* ------------------------------------------------------------------ */
/* __timingsafe_bcmp                                                  */
/* ------------------------------------------------------------------ */

static const size_t TS_MAX = 128;

static void
check_timingsafe_bcmp(const unsigned char *a, const unsigned char *b,
    size_t n)
{
	int ra = P::__timingsafe_bcmp(a, b, n);
	int rb = ref___timingsafe_bcmp(a, b, n);

	st_timingsafe_bcmp.cases++;
	if (ra != rb) {
		if (begin_fail(st_timingsafe_bcmp, "return mismatch"))
			std::printf("    n=%zu port=%d ref=%d\n", n, ra, rb);
	}
}

static void
timingsafe_bcmp_edge_cases(void)
{
	unsigned char a[TS_MAX], b[TS_MAX];

	for (size_t n = 0; n <= TS_MAX; n++) {
		std::memset(a, 0x00, TS_MAX);
		std::memset(b, 0x00, TS_MAX);
		check_timingsafe_bcmp(a, b, n);

		std::memset(a, 0xff, TS_MAX);
		std::memset(b, 0xff, TS_MAX);
		check_timingsafe_bcmp(a, b, n);

		std::memset(a, 0x80, TS_MAX);
		std::memset(b, 0x80, TS_MAX);
		check_timingsafe_bcmp(a, b, n);
	}

	for (size_t n = 1; n <= TS_MAX; n++) {
		std::memset(a, 0x55, TS_MAX);
		std::memset(b, 0x55, TS_MAX);

		a[0] ^= 0x01;
		check_timingsafe_bcmp(a, b, n);

		a[0] = b[0];
		a[n - 1] ^= 0x80;
		check_timingsafe_bcmp(a, b, n);

		a[n - 1] = b[n - 1];
		if (n > 1) {
			a[n / 2] ^= 0xff;
			check_timingsafe_bcmp(a, b, n);
		}
	}

	/* NUL-heavy and high-bit single-byte compares */
	for (unsigned char v = 0; v < 255; v++) {
		a[0] = v;
		b[0] = (unsigned char)(v ^ 0x01);
		check_timingsafe_bcmp(a, b, 1);
		check_timingsafe_bcmp(a, a, 1);
	}
	a[0] = 0xff;
	b[0] = 0x7f;
	check_timingsafe_bcmp(a, b, 1);
}

static void
timingsafe_bcmp_random_sweep(Rng &rng, unsigned long long iters)
{
	unsigned char a[TS_MAX], b[TS_MAX];

	for (unsigned long long it = 0; it < iters; it++) {
		size_t n = rng.below(TS_MAX + 1);

		for (size_t i = 0; i < n; i++) {
			if ((it & 3) == 0) {
				a[i] = (unsigned char)(0x80 | (rng.next() & 0x7f));
				b[i] = (unsigned char)(0x80 | (rng.next() & 0x7f));
			} else {
				a[i] = (unsigned char)(rng.next() & 0xff);
				b[i] = (unsigned char)(rng.next() & 0xff);
			}
		}

		if ((it & 1) == 0)
			std::memcpy(b, a, n);
		else if (n > 0) {
			size_t pos = rng.below((std::uint32_t)n);
			b[pos] ^= (unsigned char)(1 + rng.below(255));
		}

		check_timingsafe_bcmp(a, b, n);
	}
}

/* ------------------------------------------------------------------ */
/* main                                                               */
/* ------------------------------------------------------------------ */

int
main()
{
	Rng rng(0xB0239ull);

	swab_edge_cases();
	strndup_edge_cases();
	timingsafe_bcmp_edge_cases();

	const unsigned long long RANDOM_ITERS = 200000ull;
	swab_random_sweep(rng, RANDOM_ITERS);
	strndup_random_sweep(rng, RANDOM_ITERS);
	timingsafe_bcmp_random_sweep(rng, RANDOM_ITERS);

	Stat stats[] = { st_swab, st_strndup, st_timingsafe_bcmp };
	unsigned long long total_cases = 0;
	unsigned long long total_fails = 0;

	std::printf("\n%-22s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-22s %12s %12s\n", "--------", "-----", "--------");
	for (size_t i = 0; i < sizeof(stats) / sizeof(stats[0]); i++) {
		std::printf("%-22s %12llu %12llu\n", stats[i].name,
		    stats[i].cases, stats[i].fails);
		total_cases += stats[i].cases;
		total_fails += stats[i].fails;
	}
	std::printf("%-22s %12llu %12llu\n", "TOTAL", total_cases,
	    total_fails);

	return total_fails == 0 ? 0 : 1;
}
