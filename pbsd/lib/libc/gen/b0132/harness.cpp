/*
 * harness.cpp -- differential test for batch b0132 (utxdb.c).
 *
 * gettimeofday is a recording test double defined here and linked into both
 * the port and the oracle.  futx_to_utx(3) allocation failure is observed
 * through a linker wrap of calloc(3).  Every observable (buffer contents
 * including guard bytes, return values, and mock records) is compared side by
 * side.
 */

import pbsd.lib.libc.gen.b0132;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/time.h>

namespace port = pbsd::lib_libc_gen::b0132;

#define	GUARD		0x7f
#define	PAD		32u
#define	SWEEP_ITERS	200000L
#define	MAX_PRINT	12

#define	EMPTY		0
#define	BOOT_TIME	1
#define	OLD_TIME	2
#define	NEW_TIME	3
#define	USER_PROCESS	4
#define	INIT_PROCESS	5
#define	LOGIN_PROCESS	6
#define	DEAD_PROCESS	7
#define	SHUTDOWN_TIME	8

using utmpx = port::utmpx;
using futx = port::futx;

struct guarded_futx {
	unsigned char	pre[PAD];
	struct futx	fu;
	unsigned char	post[PAD];
};

struct guarded_utmpx {
	unsigned char	pre[PAD];
	struct utmpx	ut;
	unsigned char	post[PAD];
};

extern "C" {
void ref_utx_to_futx(const struct utmpx *, struct futx *);
struct utmpx *ref_futx_to_utx(const struct futx *);
void *__real_calloc(size_t, size_t);
}

struct stat_row {
	const char	*name;
	long		cases;
	long		failures;
	long		printed;
};

static stat_row rows[] = {
	{ "utx_to_futx", 0, 0, 0 },
	{ "futx_to_utx", 0, 0, 0 },
};

#define	R_UTX2FUTX	0
#define	R_FUTX2UTX	1

static uint64_t rng_state = 0x00b0132faceULL;

static inline uint64_t
rnd64(void)
{
	uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static inline uint32_t
rnd32(void)
{
	return ((uint32_t)(rnd64() >> 32));
}

static void
fail_row(int row, const char *label, const char *detail)
{
	stat_row &r = rows[row];

	r.failures++;
	if (r.printed < MAX_PRINT) {
		r.printed++;
		std::printf("  FAIL %-14s %-28s %s\n", r.name, label, detail);
	}
}

/* ------------------------------------------------------------- gettimeofday */

struct tv_mock {
	int		calls;
	struct timeval	tv;
};

static tv_mock g_tv;

static void
tv_mock_reset(void)
{
	std::memset(&g_tv, 0, sizeof(g_tv));
}

static void
tv_mock_set(time_t sec, suseconds_t usec)
{
	tv_mock_reset();
	g_tv.tv.tv_sec = sec;
	g_tv.tv.tv_usec = usec;
}

extern "C" int
gettimeofday(struct timeval *tv, void *tz)
{
	(void)tz;

	g_tv.calls++;
	if (tv != NULL)
		*tv = g_tv.tv;
	return (0);
}

/* ------------------------------------------------------------------ calloc */

static int g_calloc_fail;
static int g_calloc_calls;

extern "C" void *
__wrap_calloc(size_t nmemb, size_t size)
{
	g_calloc_calls++;
	if (g_calloc_fail != 0)
		return (NULL);
	return (__real_calloc(nmemb, size));
}

static void
calloc_track_reset(void)
{
	g_calloc_fail = 0;
	g_calloc_calls = 0;
}

/* ----------------------------------------------------------- buffer helpers */

static void
guard_init(void *buf, size_t len)
{
	std::memset(buf, GUARD, len);
}

static bool
guards_intact(const unsigned char *pre, const unsigned char *post)
{
	for (size_t i = 0; i < PAD; i++) {
		if (pre[i] != GUARD || post[i] != GUARD)
			return (false);
	}
	return (true);
}

static void
utmpx_zero(struct utmpx *ut)
{
	std::memset(ut, 0, sizeof(*ut));
}

static void
futx_zero(struct futx *fu)
{
	std::memset(fu, 0, sizeof(*fu));
}

static void
fill_bytes(char *dst, size_t len, uint32_t seed)
{
	for (size_t i = 0; i < len; i++)
		dst[i] = (char)((seed + (uint32_t)i * 17u) & 0xffu);
	if (len > 0)
		dst[len - 1] = '\0';
}

static void
fill_bytes_nulheavy(char *dst, size_t len, uint32_t seed)
{
	for (size_t i = 0; i < len; i++) {
		uint32_t v = seed + (uint32_t)i * 31u;

		dst[i] = (char)((v % 5u == 0u) ? 0 : (v & 0xffu));
	}
	if (len > 0)
		dst[len - 1] = '\0';
}

static void
utmpx_fill(struct utmpx *ut, short type, uint32_t seed)
{
	utmpx_zero(ut);
	ut->ut_type = type;
	ut->ut_pid = (pid_t)(seed & 0x7fffffffu);
	fill_bytes(ut->ut_id, sizeof(ut->ut_id), seed ^ 0x1111u);
	fill_bytes(ut->ut_user, sizeof(ut->ut_user), seed ^ 0x2222u);
	fill_bytes(ut->ut_line, sizeof(ut->ut_line), seed ^ 0x3333u);
	fill_bytes(ut->ut_host, sizeof(ut->ut_host), seed ^ 0x4444u);
	ut->ut_tv.tv_sec = (time_t)(seed & 0xffffu);
	ut->ut_tv.tv_usec = (suseconds_t)((seed >> 16) & 0xfffffu);
}

static void
futx_fill(struct futx *fu, uint8_t type, uint32_t seed)
{
	futx_zero(fu);
	fu->fu_type = type;
	fu->fu_pid = seed;
	fu->fu_tv = ((uint64_t)seed << 32) | (uint64_t)(seed ^ 0xdeadbeefu);
	fill_bytes(fu->fu_id, sizeof(fu->fu_id), seed ^ 0x5555u);
	fill_bytes(fu->fu_user, sizeof(fu->fu_user), seed ^ 0x6666u);
	fill_bytes(fu->fu_line, sizeof(fu->fu_line), seed ^ 0x7777u);
	fill_bytes(fu->fu_host, sizeof(fu->fu_host), seed ^ 0x8888u);
}

static bool
futx_bufs_match(const guarded_futx &a, const guarded_futx &b)
{
	return (std::memcmp(&a, &b, sizeof(a)) == 0);
}

static bool
utmpx_bufs_match(const struct utmpx *a, const struct utmpx *b)
{
	return (std::memcmp(a, b, sizeof(*a)) == 0);
}

/* ----------------------------------------------------------- utx_to_futx */

static void
test_utx_to_futx_one(int row, const char *label, const struct utmpx *ut,
    time_t tv_sec, suseconds_t tv_usec)
{
	guarded_utmpx gua, gub;
	guarded_futx ga, gb;

	rows[row].cases++;
	guard_init(&gua, sizeof(gua));
	guard_init(&gub, sizeof(gub));
	guard_init(&ga, sizeof(ga));
	guard_init(&gb, sizeof(gb));
	std::memcpy(&gua.ut, ut, sizeof(*ut));
	std::memcpy(&gub.ut, ut, sizeof(*ut));

	tv_mock_reset();
	tv_mock_set(tv_sec, tv_usec);
	port::utx_to_futx(&gua.ut, &ga.fu);
	tv_mock_reset();
	tv_mock_set(tv_sec, tv_usec);
	ref_utx_to_futx(&gub.ut, &gb.fu);

	if (!guards_intact(gua.pre, gua.post))
		fail_row(row, label, "port ut guard clobbered");
	if (!guards_intact(gub.pre, gub.post))
		fail_row(row, label, "ref ut guard clobbered");
	if (!guards_intact(ga.pre, ga.post))
		fail_row(row, label, "port fu guard clobbered");
	if (!guards_intact(gb.pre, gb.post))
		fail_row(row, label, "ref fu guard clobbered");
	if (!futx_bufs_match(ga, gb))
		fail_row(row, label, "futx mismatch");
	if (g_tv.calls != 1)
		fail_row(row, label, "gettimeofday call count");
}

static void
test_utx_to_futx_edges(void)
{
	const int row = R_UTX2FUTX;
	struct utmpx ut;
	const short primitive_types[] = {
		BOOT_TIME, OLD_TIME, NEW_TIME, SHUTDOWN_TIME
	};

	for (short t : primitive_types) {
		utmpx_fill(&ut, t, (uint32_t)t);
		test_utx_to_futx_one(row, "primitive", &ut, 1, 2);
	}

	utmpx_zero(&ut);
	ut.ut_type = USER_PROCESS;
	test_utx_to_futx_one(row, "user-empty", &ut, 0, 0);

	utmpx_fill(&ut, USER_PROCESS, 0x80u);
	fill_bytes_nulheavy(ut.ut_user, sizeof(ut.ut_user), 0x80);
	fill_bytes_nulheavy(ut.ut_line, sizeof(ut.ut_line), 0x81);
	fill_bytes_nulheavy(ut.ut_host, sizeof(ut.ut_host), 0x82);
	ut.ut_pid = 0xffffffff;
	test_utx_to_futx_one(row, "user-nulheavy", &ut, 100, 999999);

	utmpx_fill(&ut, USER_PROCESS, 0xbeef);
	fill_bytes(ut.ut_user, sizeof(ut.ut_user), 0xff);
	fill_bytes(ut.ut_line, sizeof(ut.ut_line), 0xfe);
	fill_bytes(ut.ut_host, sizeof(ut.ut_host), 0xfd);
	test_utx_to_futx_one(row, "user-hibit", &ut, 1234567890, 1);

	utmpx_fill(&ut, INIT_PROCESS, 42);
	test_utx_to_futx_one(row, "init", &ut, 3, 4);

	utmpx_fill(&ut, LOGIN_PROCESS, 99);
	test_utx_to_futx_one(row, "login", &ut, 5, 6);

	utmpx_fill(&ut, DEAD_PROCESS, 7);
	test_utx_to_futx_one(row, "dead", &ut, 7, 8);

	for (short bad : { (short)9, (short)127, (short)-1, (short)255 }) {
		utmpx_fill(&ut, bad, (uint32_t)bad);
		test_utx_to_futx_one(row, "default-type", &ut, 0, 0);
	}

	utmpx_fill(&ut, USER_PROCESS, 0x1234);
	test_utx_to_futx_one(row, "tv-boundary", &ut, 0, 999999);
	test_utx_to_futx_one(row, "tv-large", &ut, 2000000, 0);
}

static void
test_utx_to_futx_sweep(void)
{
	const int row = R_UTX2FUTX;
	const short types[] = {
		EMPTY, BOOT_TIME, OLD_TIME, NEW_TIME, USER_PROCESS, INIT_PROCESS,
		LOGIN_PROCESS, DEAD_PROCESS, SHUTDOWN_TIME, 9, 10, 63, -2
	};

	for (long i = 0; i < SWEEP_ITERS; i++) {
		struct utmpx ut;
		uint32_t seed = rnd32();
		short type = types[rnd32() % (sizeof(types) / sizeof(types[0]))];

		if ((seed & 3u) == 0u)
			utmpx_zero(&ut);
		else if ((seed & 3u) == 1u)
			utmpx_fill(&ut, type, seed);
		else if ((seed & 3u) == 2u) {
			utmpx_fill(&ut, type, seed);
			fill_bytes_nulheavy(ut.ut_user, sizeof(ut.ut_user), seed);
			fill_bytes_nulheavy(ut.ut_line, sizeof(ut.ut_line), seed ^ 1u);
			fill_bytes_nulheavy(ut.ut_host, sizeof(ut.ut_host), seed ^ 2u);
		} else {
			utmpx_fill(&ut, type, seed);
			if ((seed & 8u) != 0u)
				ut.ut_pid = (pid_t)(seed | 0x80000000u);
		}
		ut.ut_type = type;

		test_utx_to_futx_one(row, "sweep", &ut,
		    (time_t)(seed & 0xfffffu), (suseconds_t)(seed % 1000000));
	}
}

/* ----------------------------------------------------------- futx_to_utx */

static void
test_futx_to_utx_one(int row, const char *label, const struct futx *fu)
{
	guarded_futx ga, gb;
	const struct utmpx *pa;
	const struct utmpx *pb;

	rows[row].cases++;
	guard_init(&ga, sizeof(ga));
	guard_init(&gb, sizeof(gb));
	std::memcpy(&ga.fu, fu, sizeof(*fu));
	std::memcpy(&gb.fu, fu, sizeof(*fu));

	pa = port::futx_to_utx(&ga.fu);
	pb = ref_futx_to_utx(&gb.fu);

	if ((pa == NULL) != (pb == NULL)) {
		fail_row(row, label, "NULL mismatch");
		return;
	}
	if (pa == NULL)
		return;

	if (!guards_intact(ga.pre, ga.post))
		fail_row(row, label, "port input guard clobbered");
	if (!guards_intact(gb.pre, gb.post))
		fail_row(row, label, "ref input guard clobbered");
	if (!utmpx_bufs_match(pa, pb))
		fail_row(row, label, "utmpx mismatch");
}

static void
test_futx_to_utx_calloc_fail(void)
{
	const int row = R_FUTX2UTX;
	struct futx fu;
	const struct utmpx *pa;
	const struct utmpx *pb;

	rows[row].cases++;
	futx_fill(&fu, USER_PROCESS, 0xc0ffee);
	calloc_track_reset();
	g_calloc_fail = 1;

	pa = port::futx_to_utx(&fu);
	pb = ref_futx_to_utx(&fu);

	if ((pa == NULL) != (pb == NULL))
		fail_row(row, "calloc-fail", "NULL mismatch");
	if (g_calloc_calls < 2)
		fail_row(row, "calloc-fail", "calloc not invoked");

	calloc_track_reset();
}

static void
test_futx_to_utx_edges(void)
{
	const int row = R_FUTX2UTX;
	struct futx fu;
	const uint8_t primitive_types[] = {
		BOOT_TIME, OLD_TIME, NEW_TIME, SHUTDOWN_TIME
	};

	for (uint8_t t : primitive_types) {
		futx_fill(&fu, t, t);
		test_futx_to_utx_one(row, "primitive", &fu);
	}

	futx_zero(&fu);
	fu.fu_type = USER_PROCESS;
	test_futx_to_utx_one(row, "user-empty", &fu);

	futx_fill(&fu, USER_PROCESS, 0x80);
	fill_bytes_nulheavy(fu.fu_user, sizeof(fu.fu_user), 0x80);
	fill_bytes_nulheavy(fu.fu_line, sizeof(fu.fu_line), 0x81);
	fill_bytes_nulheavy(fu.fu_host, sizeof(fu.fu_host), 0x82);
	fu.fu_pid = 0xffffffffu;
	fu.fu_tv = 0x0000000005f5e0ffULL;
	test_futx_to_utx_one(row, "user-nulheavy", &fu);

	futx_fill(&fu, INIT_PROCESS, 1);
	test_futx_to_utx_one(row, "init", &fu);

	futx_fill(&fu, LOGIN_PROCESS, 2);
	test_futx_to_utx_one(row, "login", &fu);

	futx_fill(&fu, DEAD_PROCESS, 3);
	test_futx_to_utx_one(row, "dead", &fu);

	for (uint8_t bad : { (uint8_t)9, (uint8_t)127, (uint8_t)255 }) {
		futx_fill(&fu, bad, bad);
		test_futx_to_utx_one(row, "default-type", &fu);
	}

	futx_fill(&fu, USER_PROCESS, 0xabcd);
	fu.fu_tv = 0ULL;
	test_futx_to_utx_one(row, "tv-zero", &fu);
	fu.fu_tv = 999999ULL;
	test_futx_to_utx_one(row, "tv-usec-only", &fu);
	fu.fu_tv = 1000000ULL;
	test_futx_to_utx_one(row, "tv-sec-one", &fu);
	fu.fu_tv = 1000000000001ULL;
	test_futx_to_utx_one(row, "tv-large", &fu);

	/* Exercise the memset reuse path (second and later calls). */
	futx_fill(&fu, LOGIN_PROCESS, 0x4242);
	test_futx_to_utx_one(row, "reuse-1", &fu);
	futx_fill(&fu, DEAD_PROCESS, 0x4343);
	test_futx_to_utx_one(row, "reuse-2", &fu);
}

static void
test_futx_to_utx_sweep(void)
{
	const int row = R_FUTX2UTX;
	const uint8_t types[] = {
		EMPTY, BOOT_TIME, OLD_TIME, NEW_TIME, USER_PROCESS, INIT_PROCESS,
		LOGIN_PROCESS, DEAD_PROCESS, SHUTDOWN_TIME, 9, 11, 200, 255
	};

	for (long i = 0; i < SWEEP_ITERS; i++) {
		struct futx fu;
		uint32_t seed = rnd32();
		uint8_t type = types[rnd32() % (sizeof(types) / sizeof(types[0]))];

		if ((seed & 3u) == 0u)
			futx_zero(&fu);
		else if ((seed & 3u) == 1u)
			futx_fill(&fu, type, seed);
		else if ((seed & 3u) == 2u) {
			futx_fill(&fu, type, seed);
			fill_bytes_nulheavy(fu.fu_user, sizeof(fu.fu_user), seed);
			fill_bytes_nulheavy(fu.fu_line, sizeof(fu.fu_line), seed ^ 1u);
			fill_bytes_nulheavy(fu.fu_host, sizeof(fu.fu_host), seed ^ 2u);
		} else {
			futx_fill(&fu, type, seed);
			fu.fu_tv = rnd64();
			fu.fu_pid = rnd32();
		}
		fu.fu_type = type;

		test_futx_to_utx_one(row, "sweep", &fu);
	}
}

/* ------------------------------------------------------------------- main */

int
main(void)
{
	tv_mock_reset();
	calloc_track_reset();

	test_futx_to_utx_calloc_fail();
	test_utx_to_futx_edges();
	test_futx_to_utx_edges();
	test_utx_to_futx_sweep();
	test_futx_to_utx_sweep();

	std::printf("\n%-16s %10s %10s\n", "function", "cases", "failures");
	for (const stat_row &r : rows)
		std::printf("%-16s %10ld %10ld\n", r.name, r.cases, r.failures);

	long total_fail = 0;
	for (const stat_row &r : rows)
		total_fail += r.failures;

	return (total_fail == 0 ? 0 : 1);
}
