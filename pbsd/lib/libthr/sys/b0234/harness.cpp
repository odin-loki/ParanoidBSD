/*
 * Differential test for batch b0234 (__error_threaded).
 */

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

import pbsd.lib.libthr.sys.b0234;

namespace P = pbsd::lib_libthr_sys::b0234;

extern "C" {
struct pthread {
	int error;
};
struct pthread *_thr_initial;
int __libsys_errno;
int *ref___error_threaded(void);
}

namespace {

enum Fn {
	F_ERROR_THREADED,
	F_COUNT
};

static const char *const fn_name[F_COUNT] = {
	"__error_threaded",
};

static unsigned long long n_cases[F_COUNT];
static unsigned long long n_fails[F_COUNT];
static unsigned long long reported[F_COUNT];

static const unsigned SWEEP_ITERS = 200000u;

static P::pthread g_initial;
static P::pthread g_worker;
static P::pthread g_alt;
static P::pthread *g_curthread;

static uint64_t rng_state = 0xb0234decafbad01ULL;

static uint64_t
nextr(void)
{
	rng_state ^= rng_state << 13;
	rng_state ^= rng_state >> 7;
	rng_state ^= rng_state << 17;
	return rng_state;
}

static unsigned
rnd_below(unsigned n)
{
	return (unsigned)(nextr() % (uint64_t)n);
}

extern "C" struct pthread *
_get_curthread(void)
{
	return (struct pthread *)g_curthread;
}

static void
record_case(int fn, bool ok, const char *fmt, ...)
{
	n_cases[fn]++;
	if (ok)
		return;
	n_fails[fn]++;
	if (reported[fn] < 10) {
		reported[fn]++;
		va_list ap;
		va_start(ap, fmt);
		std::fprintf(stderr, "FAIL %s: ", fn_name[fn]);
		std::vfprintf(stderr, fmt, ap);
		va_end(ap);
		std::fputc('\n', stderr);
	} else if (reported[fn] == 10) {
		reported[fn]++;
		std::fprintf(stderr, "FAIL %s: (further failures suppressed)\n",
		    fn_name[fn]);
	}
}

static void
setup_state(P::pthread *initial, P::pthread *cur, int lib_errno,
    int initial_err, int worker_err, int alt_err)
{
	g_initial.error = initial_err;
	g_worker.error = worker_err;
	g_alt.error = alt_err;
	__libsys_errno = lib_errno;

	_thr_initial = (struct pthread *)initial;
	g_curthread = cur;
}

static int
target_kind(int *p)
{
	if (p == &__libsys_errno)
		return 0;
	if (p == &g_initial.error)
		return 1;
	if (p == &g_worker.error)
		return 2;
	if (p == &g_alt.error)
		return 3;
	return -1;
}

static void
check_error_threaded(const char *label, P::pthread *initial, P::pthread *cur,
    int lib_errno, int initial_err, int worker_err, int alt_err)
{
	int *pp, *pr;
	int kind_p, kind_r;
	int val_p, val_r;
	bool ok;

	setup_state(initial, cur, lib_errno, initial_err, worker_err, alt_err);

	pp = P::__error_threaded();
	pr = ref___error_threaded();

	kind_p = target_kind(pp);
	kind_r = target_kind(pr);
	val_p = *pp;
	val_r = *pr;

	ok = (pp == pr) && (kind_p == kind_r) && (val_p == val_r);

	if (ok && initial != nullptr && cur != nullptr && cur != initial) {
		ok = (kind_p == 2 || kind_p == 3) && (val_p == *pp);
	}

	record_case(F_ERROR_THREADED, ok,
	    "%s initial=%p cur=%p port=%p(%d) ref=%p(%d) lib=%d",
	    label,
	    (void *)initial, (void *)cur,
	    (void *)pp, val_p, (void *)pr, val_r, __libsys_errno);
}

static void
test_edges(void)
{
	/* _thr_initial == NULL */
	check_error_threaded("thr_initial_null", nullptr, nullptr,
	    11, 22, 33, 44);
	check_error_threaded("thr_initial_null_cur_worker", nullptr, &g_worker,
	    12, 22, 33, 44);

	/* _thr_initial != NULL, curthread == NULL */
	check_error_threaded("curthread_null", &g_initial, nullptr,
	    13, 22, 33, 44);

	/* _thr_initial != NULL, curthread == _thr_initial */
	check_error_threaded("curthread_is_initial", &g_initial, &g_initial,
	    14, 22, 33, 44);

	/* _thr_initial != NULL, curthread != NULL && != initial */
	check_error_threaded("curthread_worker", &g_initial, &g_worker,
	    15, 22, 33, 44);
	check_error_threaded("curthread_alt", &g_initial, &g_alt,
	    16, 22, 33, 44);

	/* boundary errno values */
	check_error_threaded("lib_errno_zero", &g_initial, nullptr, 0, 0, 0, 0);
	check_error_threaded("worker_errno_zero", &g_initial, &g_worker,
	    0, 0, 0, 0);
	check_error_threaded("lib_errno_neg", &g_initial, nullptr, -1, -2, -3, -4);
	check_error_threaded("worker_errno_neg", &g_initial, &g_worker,
	    -1, -2, -3, -4);
	check_error_threaded("lib_errno_high", &g_initial, nullptr,
	    0x7fffffff, 0x40000000, 0x50000000, 0x60000000);
	check_error_threaded("worker_errno_high", &g_initial, &g_worker,
	    0x7fffffff, 0x40000000, 0x50000000, 0x60000000);
	check_error_threaded("lib_errno_lowbit", &g_initial, nullptr,
	    1, 2, 3, 4);
}

static void
test_random(unsigned iters)
{
	unsigned i;

	for (i = 0; i < iters; i++) {
		unsigned scenario = rnd_below(8u);
		P::pthread *initial;
		P::pthread *cur;
		int lib_errno, initial_err, worker_err, alt_err;
		char label[64];

		lib_errno = (int)(nextr() & 0x7fffffffu);
		if ((i % 17u) == 0u)
			lib_errno = (int)(0x80000000u | (nextr() & 0x7fffffffu));
		initial_err = (int)(nextr() & 0xffffffffu);
		worker_err = (int)(nextr() & 0xffffffffu);
		alt_err = (int)(nextr() & 0xffffffffu);

		switch (scenario) {
		case 0:
			initial = nullptr;
			cur = nullptr;
			break;
		case 1:
			initial = nullptr;
			cur = &g_worker;
			break;
		case 2:
			initial = &g_initial;
			cur = nullptr;
			break;
		case 3:
			initial = &g_initial;
			cur = &g_initial;
			break;
		case 4:
			initial = &g_initial;
			cur = &g_worker;
			break;
		case 5:
			initial = &g_initial;
			cur = &g_alt;
			break;
		case 6:
			initial = &g_alt;
			cur = &g_worker;
			break;
		default:
			initial = &g_worker;
			cur = &g_alt;
			break;
		}

		std::snprintf(label, sizeof(label), "rand%u", i);
		check_error_threaded(label, initial, cur, lib_errno, initial_err,
		    worker_err, alt_err);
	}
}

} /* namespace */

int
main(void)
{
	unsigned fn;
	unsigned long long total_cases = 0, total_fails = 0;

	test_edges();
	test_random(SWEEP_ITERS);

	std::printf("\nbatch b0234 differential results\n");
	std::printf("%-18s %12s %10s %s\n", "function", "cases", "failures",
	    "status");
	for (fn = 0; fn < F_COUNT; fn++) {
		std::printf("%-18s %12llu %10llu %s\n", fn_name[fn],
		    n_cases[fn], n_fails[fn],
		    n_fails[fn] == 0u ? "ok" : "FAIL");
		total_cases += n_cases[fn];
		total_fails += n_fails[fn];
	}
	std::printf("%-18s %12llu %10llu %s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0u ? "ok" : "FAIL");

	return total_fails == 0u ? 0 : 1;
}
