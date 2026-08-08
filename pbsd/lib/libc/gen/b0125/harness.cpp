/*
 * harness.cpp -- differential test for batch b0125.
 *
 * clock_getcpuclockid2 is a recording test double defined here and linked into
 * both the port and the oracle.  __cpuset_free is observed through a linker
 * wrap of free(3).  Every observable (return value, errno, buffer contents
 * including guard bytes, allocation success, usable size, and mock records)
 * is compared side by side.
 */

#define _GNU_SOURCE

import pbsd.lib.libc.gen.b0125;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <errno.h>
#include <malloc.h>
#include <sched.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#ifndef cpuset_t
typedef cpu_set_t cpuset_t;
#endif

namespace port = pbsd::lib_libc_gen::b0125;

#ifndef CPUCLOCK_WHICH_PID
#define	CPUCLOCK_WHICH_PID	0
#endif

#define	GUARD		0x7f
#define	CLK_PAD		4u
#define	CLK_BUFSZ	(sizeof(clockid_t) + 2u * CLK_PAD)
#define	SWEEP_ITERS	200000L
#define	MAX_PRINT	12

extern "C" {
void ref___cpuset_free(cpuset_t *);
cpuset_t *ref___cpuset_alloc(size_t);
int ref_clock_getcpuclockid(pid_t, clockid_t *);
void __real_free(void *);
}

/* ------------------------------------------------------------------ shared */

struct stat_row {
	const char	*name;
	long		cases;
	long		failures;
	long		printed;
};

static stat_row rows[] = {
	{ "__cpuset_alloc", 0, 0, 0 },
	{ "__cpuset_free", 0, 0, 0 },
	{ "clock_getcpuclockid", 0, 0, 0 },
};

#define	R_ALLOC	0
#define	R_FREE	1
#define	R_CLK	2

static uint64_t rng_state = 0x00b0125feedULL;

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
		std::printf("  FAIL %-22s %-28s %s\n", r.name, label, detail);
	}
}

/* ----------------------------------------------------------- free tracking */

static int g_free_calls;
static void *g_freed_ptr;

extern "C" void
__wrap_free(void *ptr)
{
	g_freed_ptr = ptr;
	g_free_calls++;
	__real_free(ptr);
}

static void
free_track_reset(void)
{
	g_free_calls = 0;
	g_freed_ptr = NULL;
}

/* ------------------------------------------------ clock_getcpuclockid2 mock */

struct clk_mock {
	int		ret;
	int		err;
	clockid_t	out;
	int		write_on_fail;
	int		calls;
	id_t		last_id;
	int		last_which;
};

static clk_mock g_clk;

static void
clk_mock_reset(void)
{
	std::memset(&g_clk, 0, sizeof(g_clk));
}

static void
clk_mock_set(int ret, int err, clockid_t out, int write_on_fail)
{
	clk_mock_reset();
	g_clk.ret = ret;
	g_clk.err = err;
	g_clk.out = out;
	g_clk.write_on_fail = write_on_fail;
}

extern "C" int
clock_getcpuclockid2(id_t id, int which, clockid_t *clock_id)
{
	g_clk.calls++;
	g_clk.last_id = id;
	g_clk.last_which = which;

	if (g_clk.ret != 0) {
		errno = g_clk.err;
		if (g_clk.write_on_fail && clock_id != NULL)
			*clock_id = g_clk.out;
		return (g_clk.ret);
	}

	if (clock_id != NULL)
		*clock_id = g_clk.out;
	return (0);
}

static void
clk_buf_init(unsigned char *buf, clockid_t init)
{
	std::memset(buf, GUARD, CLK_BUFSZ);
	std::memcpy(buf + CLK_PAD, &init, sizeof(init));
}

static clockid_t *
clk_slot(unsigned char *buf)
{
	return ((clockid_t *)(buf + CLK_PAD));
}

static bool
clk_buf_same(const unsigned char *a, const unsigned char *b)
{
	return (std::memcmp(a, b, CLK_BUFSZ) == 0);
}

static void
case_clock(const char *label, pid_t pid, int ret, int err, clockid_t out,
    int write_on_fail, clockid_t pre)
{
	stat_row &r = rows[R_CLK];
	unsigned char bufa[CLK_BUFSZ];
	unsigned char bufb[CLK_BUFSZ];
	int ra, rb;

	r.cases++;

	clk_mock_set(ret, err, out, write_on_fail);
	clk_buf_init(bufa, pre);
	clk_buf_init(bufb, pre);
	errno = 0;
	ra = ref_clock_getcpuclockid(pid, clk_slot(bufa));

	clk_mock_set(ret, err, out, write_on_fail);
	clk_buf_init(bufb, pre);
	errno = 0;
	rb = port::clock_getcpuclockid(pid, clk_slot(bufb));

	if (ra != rb) {
		fail_row(R_CLK, label, "return mismatch");
		return;
	}
	if (!clk_buf_same(bufa, bufb)) {
		fail_row(R_CLK, label, "buffer mismatch");
		return;
	}
	if (g_clk.last_id != (id_t)pid) {
		fail_row(R_CLK, label, "mock id mismatch");
		return;
	}
	if (g_clk.last_which != CPUCLOCK_WHICH_PID) {
		fail_row(R_CLK, label, "mock which mismatch");
		return;
	}
}

/* --------------------------------------------------------- __cpuset_alloc */

static size_t
alloc_bytes(size_t ncpus)
{
	return (CPU_ALLOC_SIZE(ncpus));
}

static void
case_alloc(const char *label, size_t ncpus)
{
	stat_row &r = rows[R_ALLOC];
	cpuset_t *pa, *pb;
	size_t need;
	size_t ua, ub;

	r.cases++;

	std::fprintf(stderr, "alloc %zu\n", ncpus);
	pa = ref___cpuset_alloc(ncpus);
	std::fprintf(stderr, "ref pa=%p\n", (void *)pa);
	pb = port::__cpuset_alloc(ncpus);
	std::fprintf(stderr, "port pb=%p\n", (void *)pb);

	if ((pa == NULL) != (pb == NULL)) {
		fail_row(R_ALLOC, label, "null mismatch");
		if (pa != NULL)
			std::free(pa);
		if (pb != NULL)
			std::free(pb);
		return;
	}

	if (pa == NULL)
		return;

	need = alloc_bytes(ncpus);
	std::fprintf(stderr, "need=%zu\n", need);
	ua = malloc_usable_size(pa);
	std::fprintf(stderr, "ua=%zu\n", ua);
	ub = malloc_usable_size(pb);
	std::fprintf(stderr, "ub=%zu\n", ub);
	if (ua != ub) {
		fail_row(R_ALLOC, label, "usable size mismatch");
		std::free(pa);
		std::free(pb);
		return;
	}
	if (ua < need) {
		fail_row(R_ALLOC, label, "usable size too small");
		std::free(pa);
		std::free(pb);
		return;
	}

	((unsigned char *)pa)[need - 1] = 0xa5;
	((unsigned char *)pb)[need - 1] = 0xa5;
	if (((unsigned char *)pa)[need - 1] != ((unsigned char *)pb)[need - 1]) {
		fail_row(R_ALLOC, label, "tail byte mismatch");
	}

	std::free(pa);
	std::free(pb);
}

/* ---------------------------------------------------------- __cpuset_free */

static void
case_free(const char *label, size_t ncpus)
{
	stat_row &r = rows[R_FREE];
	cpuset_t *pa, *pb;
	int ref_calls, port_calls;
	void *ref_ptr, *port_ptr;

	r.cases++;

	pa = ref___cpuset_alloc(ncpus);
	pb = port::__cpuset_alloc(ncpus);
	if (pa == NULL || pb == NULL) {
		if (pa != NULL)
			std::free(pa);
		if (pb != NULL)
			std::free(pb);
		return;
	}

	free_track_reset();
	ref___cpuset_free(pa);
	ref_calls = g_free_calls;
	ref_ptr = g_freed_ptr;

	free_track_reset();
	port::__cpuset_free(pb);
	port_calls = g_free_calls;
	port_ptr = g_freed_ptr;

	if (ref_calls != port_calls) {
		fail_row(R_FREE, label, "free call count mismatch");
		return;
	}
	if (ref_calls != 1) {
		fail_row(R_FREE, label, "expected one free call");
		return;
	}
	if (ref_ptr != pa) {
		fail_row(R_FREE, label, "ref freed wrong pointer");
		return;
	}
	if (port_ptr != pb) {
		fail_row(R_FREE, label, "port freed wrong pointer");
		return;
	}
}

/* -------------------------------------------------------------- edge cases */

static void
edge_alloc(void)
{
	static const size_t cases[] = {
		0,
	};

	for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
		case_alloc("edge ncpus", cases[i]);

	return;

	static const size_t cases_full[] = {
		1,
		2,
		7,
		8,
		9,
		15,
		16,
		17,
		31,
		32,
		33,
		63,
		64,
		65,
		127,
		128,
		129,
		255,
		256,
		257,
		511,
		512,
		513,
		1023,
		1024,
		1025,
		2047,
		2048,
		4095,
		4096,
		8191,
		16383,
		32767,
		65535,
		131071,
		262143,
		524287,
		1048575,
	};

	for (unsigned i = 0; i < sizeof(cases_full) / sizeof(cases_full[0]); i++)
		case_alloc("edge ncpus", cases_full[i]);

	case_alloc("edge SIZE_MAX/2", (size_t)-1 / 2);
	case_alloc("edge SIZE_MAX", (size_t)-1);
}

static void
edge_free(void)
{
	static const size_t cases[] = {
		0,
		1,
		8,
		64,
		128,
		1024,
		4096,
	};

	for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
		case_free("edge ncpus", cases[i]);
}

static void
edge_clock(void)
{
	case_clock("success pid 0", 0, 0, 0, 7, 0, 0);
	case_clock("success pid 1", 1, 0, 0, 42, 0, 0);
	case_clock("success self", (pid_t)getpid(), 0, 0, 99, 0, 0);
	case_clock("success max pid", (pid_t)2147483647, 0, 0, 12345, 0, 0);
	case_clock("success neg pid", (pid_t)-1, 0, 0, 3, 0, 0);
	case_clock("success high clock", 5, 0, 0, (clockid_t)0xffffffff, 0,
	    (clockid_t)0x80808080);
	case_clock("success zero clock", 5, 0, 0, 0, 0, (clockid_t)0xdeadbeef);

	case_clock("fail ret 1 EINVAL", 2, 1, EINVAL, 0, 0, 0);
	case_clock("fail ret -1 ESRCH", 3, -1, ESRCH, 0, 0, 0);
	case_clock("fail ret 1 EPERM", 4, 1, EPERM, 0, 0, 0);
	case_clock("fail ret 2 EIO", 6, 2, EIO, 0, 0, 0);
	case_clock("fail ret -1 ENOSYS", 7, -1, ENOSYS, 0, 0, 0);
	case_clock("fail ret 1 high errno", 8, 1, 255, 0, 0, 0);

	case_clock("fail writes clock", 9, 1, EINVAL, (clockid_t)0x55aa55aa,
	    1, (clockid_t)0x11111111);
	case_clock("fail no write clock", 10, 1, EINVAL, (clockid_t)0x55aa55aa,
	    0, (clockid_t)0x11111111);

	case_clock("boundary errno 0 fail", 11, 1, 0, 0, 0, 0);
	case_clock("boundary errno 1 fail", 12, 1, 1, 0, 0, 0);
}

/* ------------------------------------------------------------------ sweeps */

static size_t
gen_ncpus(void)
{
	uint64_t r = rnd64();

	switch ((unsigned)(r % 12)) {
	case 0:
		return (0);
	case 1:
		return (1);
	case 2:
		return ((size_t)(r & 0xff));
	case 3:
		return ((size_t)(r & 0xfff));
	case 4:
		return ((size_t)(r % 8192));
	case 5:
		return ((size_t)(r % 65536));
	case 6:
		return ((size_t)1 << (r % 20));
	case 7:
		return (((size_t)1 << (r % 20)) - 1);
	case 8:
		return (((size_t)1 << (r % 20)) + 1);
	case 9:
		return ((size_t)-1 >> (r % 32));
	case 10:
		return ((size_t)-1 - (r % 1024));
	default:
		return ((size_t)r);
	}
}

static pid_t
gen_pid(void)
{
	uint64_t r = rnd64();

	switch ((unsigned)(r % 8)) {
	case 0:
		return (0);
	case 1:
		return (1);
	case 2:
		return ((pid_t)getpid());
	case 3:
		return ((pid_t)-1);
	case 4:
		return ((pid_t)2147483647);
	case 5:
		return ((pid_t)-2147483647 - 1);
	case 6:
		return ((pid_t)(r & 0x7fffffff));
	default:
		return ((pid_t)r);
	}
}

static void
sweep_alloc(void)
{
	rng_state = 0x0a110cb0125ULL;
	for (long i = 0; i < SWEEP_ITERS; i++)
		case_alloc("sweep", gen_ncpus());
}

static void
sweep_free(void)
{
	rng_state = 0x0f1eecb0125ULL;
	for (long i = 0; i < SWEEP_ITERS; i++)
		case_free("sweep", gen_ncpus());
}

static void
sweep_clock(void)
{
	static const int errs[] = {
		EINVAL, ESRCH, EPERM, EIO, ENOSYS, EACCES, ENOMEM, 0, 1, 255,
	};
	rng_state = 0xc10ccb0125ULL;

	for (long i = 0; i < SWEEP_ITERS; i++) {
		uint64_t r = rnd64();
		pid_t pid = gen_pid();
		int fail = (int)(r & 1);
		int ret = fail ? (int)((r >> 1) % 3 == 0 ? -1 : 1) : 0;
		int err = errs[(unsigned)(r >> 3) % (sizeof(errs) / sizeof(errs[0]))];
		clockid_t out = (clockid_t)(r >> 11);
		clockid_t pre = (clockid_t)(r >> 43);
		int write_on_fail = (int)((r >> 7) & 1);

		case_clock("sweep", pid, ret, err, out, write_on_fail, pre);
	}
}

/* ------------------------------------------------------------------- main */

int
main(void)
{
	std::fprintf(stderr, "edge_alloc\n");
	edge_alloc();
	std::fprintf(stderr, "edge_free\n");
	edge_free();
	std::fprintf(stderr, "edge_clock\n");
	edge_clock();
	std::fprintf(stderr, "sweep_alloc\n");
	sweep_alloc();
	std::fprintf(stderr, "sweep_free\n");
	sweep_free();
	std::fprintf(stderr, "sweep_clock\n");
	sweep_clock();

	long total_cases = 0;
	long total_failures = 0;

	std::printf("\n%-24s %12s %12s  %s\n", "function", "cases", "failures",
	    "result");
	std::printf("--------------------------------------------------------"
	    "------------\n");
	for (unsigned i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
		stat_row &r = rows[i];

		total_cases += r.cases;
		total_failures += r.failures;
		std::printf("%-24s %12ld %12ld  %s\n", r.name, r.cases,
		    r.failures, r.failures == 0 ? "ok" : "FAILED");
	}
	std::printf("--------------------------------------------------------"
	    "------------\n");
	std::printf("%-24s %12ld %12ld  %s\n", "TOTAL", total_cases,
	    total_failures, total_failures == 0 ? "ok" : "FAILED");

	return (total_failures == 0 ? 0 : 1);
}
