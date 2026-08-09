/*
 * Differential test for batch b0252 (_thr_main_np, _thr_equal, _Tthr_self,
 * _pthread_multi_np).
 */

#include <cstdarg>
#include <cstdint>
#include <cstdio>

struct pthread {
	int dummy;
};

extern "C" {
extern struct pthread *_thr_initial;
extern void b0252_set_curthread(struct pthread *t);
extern void b0252_reset_counters(void);
extern unsigned b0252_get_check_init_count(void);
extern unsigned b0252_get_resume_all_count(void);

int ref__thr_main_np(void);
int ref__thr_equal(struct pthread *t1, struct pthread *t2);
struct pthread *ref__Tthr_self(void);
int ref__pthread_multi_np(void);
}

import pbsd.lib.libthr.thread.b0252;

namespace P = pbsd::lib_libthr_thread::b0252;

static struct pthread g_main;
static struct pthread g_worker;
static struct pthread g_alt;

namespace {

enum Fn {
	F_THR_EQUAL,
	F_THR_SELF,
	F_THR_MAIN_NP,
	F_PTHREAD_MULTI_NP,
	F_COUNT
};

static const char *const fn_name[F_COUNT] = {
	"_thr_equal",
	"_Tthr_self",
	"_thr_main_np",
	"_pthread_multi_np",
};

static unsigned long long n_cases[F_COUNT];
static unsigned long long n_fails[F_COUNT];
static unsigned long long reported[F_COUNT];

static const unsigned SWEEP_ITERS = 200000u;

static uint64_t rng_state = 0xb0252decafbad01ULL;

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
check_thr_equal(const char *label, struct pthread *t1, struct pthread *t2)
{
	int rp, rr;
	bool ok;

	rp = P::_thr_equal(t1, t2);
	rr = ref__thr_equal(t1, t2);
	ok = (rp == rr);
	record_case(F_THR_EQUAL, ok,
	    "%s t1=%p t2=%p port=%d ref=%d", label, (void *)t1, (void *)t2,
	    rp, rr);
}

static void
check_thr_self(const char *label, struct pthread *cur)
{
	struct pthread *tp, *tr;
	unsigned cp, cr;
	bool ok;

	b0252_set_curthread(cur);
	b0252_reset_counters();

	tp = P::_Tthr_self();
	cp = b0252_get_check_init_count();

	b0252_set_curthread(cur);
	b0252_reset_counters();

	tr = ref__Tthr_self();
	cr = b0252_get_check_init_count();

	ok = (tp == tr) && (cp == cr) && (cp == 1u);
	record_case(F_THR_SELF, ok,
	    "%s cur=%p port=%p ref=%p check_port=%u check_ref=%u",
	    label, (void *)cur, (void *)tp, (void *)tr, cp, cr);
}

static void
check_thr_main_np(const char *label, struct pthread *initial, struct pthread *cur)
{
	int rp, rr;
	bool ok;

	_thr_initial = initial;
	b0252_set_curthread(cur);
	b0252_reset_counters();

	rp = P::_thr_main_np();

	_thr_initial = initial;
	b0252_set_curthread(cur);
	b0252_reset_counters();

	rr = ref__thr_main_np();

	ok = (rp == rr);
	record_case(F_THR_MAIN_NP, ok,
	    "%s initial=%p cur=%p port=%d ref=%d", label, (void *)initial,
	    (void *)cur, rp, rr);
}

static void
check_pthread_multi_np(const char *label)
{
	int rp, rr;
	unsigned cp, cr;
	bool ok;

	b0252_reset_counters();
	rp = P::_pthread_multi_np();
	cp = b0252_get_resume_all_count();

	b0252_reset_counters();
	rr = ref__pthread_multi_np();
	cr = b0252_get_resume_all_count();

	ok = (rp == rr) && (cp == cr) && (rp == 0) && (cp == 1u);
	record_case(F_PTHREAD_MULTI_NP, ok,
	    "%s port=%d ref=%d resume_port=%u resume_ref=%u", label, rp, rr,
	    cp, cr);
}

static void
test_thr_equal_edges(void)
{
	check_thr_equal("both_null", nullptr, nullptr);
	check_thr_equal("null_nonnull", nullptr, &g_main);
	check_thr_equal("nonnull_null", &g_main, nullptr);
	check_thr_equal("same_main", &g_main, &g_main);
	check_thr_equal("same_worker", &g_worker, &g_worker);
	check_thr_equal("main_worker", &g_main, &g_worker);
	check_thr_equal("worker_main", &g_worker, &g_main);
	check_thr_equal("main_alt", &g_main, &g_alt);
	check_thr_equal("alt_worker", &g_alt, &g_worker);
	check_thr_equal("worker_worker_diff_addr", &g_worker, &g_alt);
}

static void
test_thr_self_edges(void)
{
	check_thr_self("cur_null", nullptr);
	check_thr_self("cur_main", &g_main);
	check_thr_self("cur_worker", &g_worker);
	check_thr_self("cur_alt", &g_alt);
}

static void
test_thr_main_np_edges(void)
{
	check_thr_main_np("initial_null", nullptr, nullptr);
	check_thr_main_np("initial_null_cur_main", nullptr, &g_main);
	check_thr_main_np("is_main", &g_main, &g_main);
	check_thr_main_np("not_main", &g_main, &g_worker);
	check_thr_main_np("not_main_cur_null", &g_main, nullptr);
	check_thr_main_np("initial_worker_cur_main", &g_worker, &g_main);
	check_thr_main_np("initial_alt_cur_alt", &g_alt, &g_alt);
	check_thr_main_np("initial_alt_cur_main", &g_alt, &g_main);
}

static void
test_pthread_multi_np_edges(void)
{
	check_pthread_multi_np("once");
	check_pthread_multi_np("twice");
}

static struct pthread *
pick_thread(unsigned sel)
{
	switch (sel % 4u) {
	case 0:
		return (nullptr);
	case 1:
		return (&g_main);
	case 2:
		return (&g_worker);
	default:
		return (&g_alt);
	}
}

static void
test_thr_equal_random(unsigned iters)
{
	unsigned i;

	for (i = 0; i < iters; i++) {
		struct pthread *t1, *t2;
		char label[64];

		t1 = pick_thread(rnd_below(4u));
		t2 = pick_thread(rnd_below(4u));
		if ((i % 23u) == 0u)
			t2 = t1;
		std::snprintf(label, sizeof(label), "rand%u", i);
		check_thr_equal(label, t1, t2);
	}
}

static void
test_thr_self_random(unsigned iters)
{
	unsigned i;

	for (i = 0; i < iters; i++) {
		struct pthread *cur;
		char label[64];

		cur = pick_thread(rnd_below(4u));
		std::snprintf(label, sizeof(label), "rand%u", i);
		check_thr_self(label, cur);
	}
}

static void
test_thr_main_np_random(unsigned iters)
{
	unsigned i;

	for (i = 0; i < iters; i++) {
		struct pthread *initial, *cur;
		char label[64];

		initial = pick_thread(rnd_below(4u));
		cur = pick_thread(rnd_below(4u));
		if ((i % 31u) == 0u)
			initial = nullptr;
		if ((i % 37u) == 0u)
			cur = initial;
		std::snprintf(label, sizeof(label), "rand%u", i);
		check_thr_main_np(label, initial, cur);
	}
}

static void
test_pthread_multi_np_random(unsigned iters)
{
	unsigned i;

	for (i = 0; i < iters; i++) {
		char label[64];

		std::snprintf(label, sizeof(label), "rand%u", i);
		check_pthread_multi_np(label);
	}
}

} /* namespace */

int
main(void)
{
	unsigned fn;
	unsigned long long total_cases = 0, total_fails = 0;

	g_main.dummy = 1;
	g_worker.dummy = 2;
	g_alt.dummy = 3;

	test_thr_equal_edges();
	test_thr_self_edges();
	test_thr_main_np_edges();
	test_pthread_multi_np_edges();

	test_thr_equal_random(SWEEP_ITERS);
	test_thr_self_random(SWEEP_ITERS);
	test_thr_main_np_random(SWEEP_ITERS);
	test_pthread_multi_np_random(SWEEP_ITERS);

	std::printf("\nbatch b0252 differential results\n");
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
