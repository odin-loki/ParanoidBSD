/*
 * Differential test for batch b0254.
 */

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libthr.thread.b0254;

namespace P = pbsd::lib_libthr_thread::b0254;

extern "C" {
struct pthread {
	long tid;
};
struct tcb {
	struct pthread *tcb_thread;
};
typedef int clockid_t;

#define CPUCLOCK_WHICH_TID 1
#define TLS_TCB_SIZE 16
#define TLS_TCB_ALIGN 16
#define EINVAL 22

void ref__pthread_yield(void);
int ref__pthread_getcpuclockid(struct pthread *pthread, clockid_t *clock_id);
int ref__thr_getthreadid_np(void);
struct tcb *ref__tcb_ctor(struct pthread *thread, int initial);
void ref__tcb_dtor(struct tcb *tcb);

void b0254_reset_mocks(void);
void b0254_set_curthread(struct pthread *t);
void b0254_set_clock_getcpuclockid2(int ret, int err, clockid_t out);
void b0254_set_tcb_get_result(int ok);
unsigned b0254_get_sched_yield_count(void);
unsigned b0254_get_check_init_count(void);
long b0254_get_last_tid(void);
int b0254_get_last_which(void);
unsigned b0254_get_alloc_count(void);
unsigned b0254_get_free_count(void);
struct tcb *b0254_get_last_freed(void);
size_t b0254_get_last_free_size(void);
size_t b0254_get_last_free_align(void);
}

static struct pthread g_thr0;
static struct pthread g_thr1;
static struct pthread g_thr2;

namespace {

enum Fn {
	F_YIELD,
	F_GETCPUCLOCKID,
	F_GETTHREADID_NP,
	F_TCB_CTOR,
	F_TCB_DTOR,
	F_COUNT
};

static const char *const fn_name[F_COUNT] = {
	"_pthread_yield",
	"_pthread_getcpuclockid",
	"_thr_getthreadid_np",
	"_tcb_ctor",
	"_tcb_dtor",
};

static unsigned long long n_cases[F_COUNT];
static unsigned long long n_fails[F_COUNT];
static unsigned long long reported[F_COUNT];

static const unsigned SWEEP_ITERS = 200000u;
static const unsigned char GUARD = 0x7f;

static uint64_t rng_state = 0xb0254decafbad01ULL;

static uint64_t
nextr(void)
{
	rng_state ^= rng_state << 13;
	rng_state ^= rng_state >> 7;
	rng_state ^= rng_state << 17;
	return (rng_state);
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

struct GuardedClock {
	unsigned char pre[8];
	clockid_t val;
	unsigned char post[8];
};

static void
init_guarded_clock(GuardedClock *g, clockid_t seed)
{
	std::memset(g->pre, GUARD, sizeof(g->pre));
	g->val = seed;
	std::memset(g->post, GUARD, sizeof(g->post));
}

static bool
guards_intact(const GuardedClock *g)
{
	unsigned i;

	for (i = 0; i < sizeof(g->pre); i++)
		if (g->pre[i] != GUARD)
			return (false);
	for (i = 0; i < sizeof(g->post); i++)
		if (g->post[i] != GUARD)
			return (false);
	return (true);
}

static bool
guards_equal(const GuardedClock *a, const GuardedClock *b)
{
	return std::memcmp(a, b, sizeof(*a)) == 0;
}

static void
check_yield(const char *label)
{
	unsigned before_p, after_p;
	unsigned before_r, after_r;
	bool ok;

	b0254_reset_mocks();
	before_p = b0254_get_sched_yield_count();
	P::_pthread_yield();
	after_p = b0254_get_sched_yield_count();

	b0254_reset_mocks();
	before_r = b0254_get_sched_yield_count();
	ref__pthread_yield();
	after_r = b0254_get_sched_yield_count();

	ok = ((after_p - before_p) == 1u) && ((after_r - before_r) == 1u);
	record_case(F_YIELD, ok, "%s port_delta=%u ref_delta=%u",
	    label, after_p - before_p, after_r - before_r);
}

struct CpuclockResult {
	int ret;
	GuardedClock clk;
	long last_tid;
	int last_which;
};

static CpuclockResult
run_getcpuclockid_port(struct pthread *pthread, const GuardedClock *in)
{
	CpuclockResult r;

	r.clk = *in;
	r.ret = P::_pthread_getcpuclockid(pthread, &r.clk.val);
	r.last_tid = b0254_get_last_tid();
	r.last_which = b0254_get_last_which();
	return (r);
}

static CpuclockResult
run_getcpuclockid_ref(struct pthread *pthread, const GuardedClock *in)
{
	CpuclockResult r;

	r.clk = *in;
	r.ret = ref__pthread_getcpuclockid(pthread, &r.clk.val);
	r.last_tid = b0254_get_last_tid();
	r.last_which = b0254_get_last_which();
	return (r);
}

static void
check_getcpuclockid(const char *label, struct pthread *pthread, int clock_ret,
    int clock_err, clockid_t clock_out, clockid_t seed)
{
	GuardedClock in;
	CpuclockResult port, ref;
	bool ok;

	init_guarded_clock(&in, seed);
	b0254_reset_mocks();
	b0254_set_clock_getcpuclockid2(clock_ret, clock_err, clock_out);
	port = run_getcpuclockid_port(pthread, &in);

	init_guarded_clock(&in, seed);
	b0254_reset_mocks();
	b0254_set_clock_getcpuclockid2(clock_ret, clock_err, clock_out);
	ref = run_getcpuclockid_ref(pthread, &in);

	ok = (port.ret == ref.ret) &&
	    guards_equal(&port.clk, &ref.clk) &&
	    guards_intact(&port.clk) &&
	    guards_intact(&ref.clk) &&
	    (port.last_tid == ref.last_tid) &&
	    (port.last_which == ref.last_which);

	if (pthread != nullptr) {
		ok = ok && (port.last_tid == pthread->tid) &&
		    (port.last_which == CPUCLOCK_WHICH_TID);
	}

	record_case(F_GETCPUCLOCKID, ok,
	    "%s pthread=%p ret_p=%d ret_r=%d clk_p=%d clk_r=%d tid_p=%ld",
	    label, (void *)pthread, port.ret, ref.ret, port.clk.val,
	    ref.clk.val, port.last_tid);
}

static void
check_getthreadid_np(const char *label, struct pthread *cur, long tid)
{
	int ret_p, ret_r;
	unsigned init_p, init_r;
	bool ok;

	cur->tid = tid;
	b0254_reset_mocks();
	b0254_set_curthread(cur);
	init_p = b0254_get_check_init_count();
	ret_p = P::_thr_getthreadid_np();
	init_p = b0254_get_check_init_count() - init_p;

	b0254_reset_mocks();
	b0254_set_curthread(cur);
	init_r = b0254_get_check_init_count();
	ret_r = ref__thr_getthreadid_np();
	init_r = b0254_get_check_init_count() - init_r;

	ok = (ret_p == ret_r) && (init_p == init_r) && (init_p == 1u);
	record_case(F_GETTHREADID_NP, ok,
	    "%s cur=%p tid=%ld ret_p=%d ret_r=%d init_p=%u init_r=%u",
	    label, (void *)cur, tid, ret_p, ret_r, init_p, init_r);
}

struct TcbCtorResult {
	struct tcb *tcb;
	struct pthread *thread_field;
	unsigned alloc_count;
};

static TcbCtorResult
run_tcb_ctor_port(struct pthread *thread, int initial, int get_ok)
{
	TcbCtorResult r;

	b0254_reset_mocks();
	b0254_set_tcb_get_result(get_ok);
	r.alloc_count = b0254_get_alloc_count();
	r.tcb = P::_tcb_ctor(thread, initial);
	r.alloc_count = b0254_get_alloc_count() - r.alloc_count;
	if (r.tcb != nullptr)
		r.thread_field = r.tcb->tcb_thread;
	else
		r.thread_field = nullptr;
	return (r);
}

static TcbCtorResult
run_tcb_ctor_ref(struct pthread *thread, int initial, int get_ok)
{
	TcbCtorResult r;

	b0254_reset_mocks();
	b0254_set_tcb_get_result(get_ok);
	r.alloc_count = b0254_get_alloc_count();
	r.tcb = ref__tcb_ctor(thread, initial);
	r.alloc_count = b0254_get_alloc_count() - r.alloc_count;
	if (r.tcb != nullptr)
		r.thread_field = r.tcb->tcb_thread;
	else
		r.thread_field = nullptr;
	return (r);
}

static void
check_tcb_ctor(const char *label, struct pthread *thread, int initial,
    int get_ok)
{
	TcbCtorResult port, ref;
	bool ok;

	port = run_tcb_ctor_port(thread, initial, get_ok);
	ref = run_tcb_ctor_ref(thread, initial, get_ok);

	ok = ((port.tcb == nullptr) == (ref.tcb == nullptr)) &&
	    (port.thread_field == ref.thread_field) &&
	    (port.alloc_count == ref.alloc_count) &&
	    (port.tcb == ref.tcb);

	record_case(F_TCB_CTOR, ok,
	    "%s thread=%p initial=%d get_ok=%d null_p=%d null_r=%d "
	    "field_p=%p field_r=%p alloc_p=%u alloc_r=%u",
	    label, (void *)thread, initial, get_ok, port.tcb == nullptr,
	    ref.tcb == nullptr, (void *)port.thread_field,
	    (void *)ref.thread_field, port.alloc_count, ref.alloc_count);
}

struct TcbDtorResult {
	unsigned free_count;
	struct tcb *last_freed;
	size_t free_size;
	size_t free_align;
};

static TcbDtorResult
run_tcb_dtor_port(struct tcb *tcb)
{
	TcbDtorResult r;

	b0254_reset_mocks();
	r.free_count = b0254_get_free_count();
	P::_tcb_dtor(tcb);
	r.free_count = b0254_get_free_count() - r.free_count;
	r.last_freed = b0254_get_last_freed();
	r.free_size = b0254_get_last_free_size();
	r.free_align = b0254_get_last_free_align();
	return (r);
}

static TcbDtorResult
run_tcb_dtor_ref(struct tcb *tcb)
{
	TcbDtorResult r;

	b0254_reset_mocks();
	r.free_count = b0254_get_free_count();
	ref__tcb_dtor(tcb);
	r.free_count = b0254_get_free_count() - r.free_count;
	r.last_freed = b0254_get_last_freed();
	r.free_size = b0254_get_last_free_size();
	r.free_align = b0254_get_last_free_align();
	return (r);
}

static void
check_tcb_dtor(const char *label, struct tcb *tcb)
{
	TcbDtorResult port, ref;
	bool ok;

	port = run_tcb_dtor_port(tcb);
	ref = run_tcb_dtor_ref(tcb);

	ok = (port.free_count == ref.free_count) &&
	    (port.free_count == 1u) &&
	    (port.last_freed == ref.last_freed) &&
	    (port.last_freed == tcb) &&
	    (port.free_size == ref.free_size) &&
	    (port.free_size == TLS_TCB_SIZE) &&
	    (port.free_align == ref.free_align) &&
	    (port.free_align == TLS_TCB_ALIGN);

	record_case(F_TCB_DTOR, ok,
	    "%s tcb=%p free_p=%u free_r=%u freed_p=%p freed_r=%p",
	    label, (void *)tcb, port.free_count, ref.free_count,
	    (void *)port.last_freed, (void *)ref.last_freed);
}

static struct tcb *
make_tcb_for_dtor(void)
{
	struct tcb *tcb;

	b0254_reset_mocks();
	b0254_set_tcb_get_result(1);
	tcb = ref__tcb_ctor(&g_thr2, 0);
	return (tcb);
}

static void
drain_alloc_pool(void)
{
	unsigned i;

	for (i = 0; i < 8u; i++)
		(void)make_tcb_for_dtor();
}

static void
test_edges(void)
{
	struct tcb *dtor_tcb;

	check_yield("single");
	check_yield("repeat");

	check_getcpuclockid("null_pthread", nullptr, 0, 0, 0, 0);
	g_thr0.tid = 1;
	check_getcpuclockid("success_tid1", &g_thr0, 0, 0, 7, 0x11111111);
	check_getcpuclockid("success_tid0", &g_thr0, 0, 0, 0, 0);
	g_thr0.tid = -1;
	check_getcpuclockid("success_tid_neg", &g_thr0, 0, 0, 3, 0x22222222);
	g_thr0.tid = 0x7fffffff;
	check_getcpuclockid("success_tid_max", &g_thr0, 0, 0, 42,
	    0x7fffffff);
	g_thr0.tid = (long)0x80000000;
	check_getcpuclockid("success_tid_highbit", &g_thr0, 0, 0, 99,
	    (clockid_t)0x80808080);
	check_getcpuclockid("fail_einval", &g_thr0, 1, EINVAL, 0, 0x33333333);
	check_getcpuclockid("fail_custom", &g_thr0, 1, 123, 0, 0x44444444);
	check_getcpuclockid("fail_errno_high", &g_thr0, 2, 0x7f, 0xff,
	    (clockid_t)0xffffffff);

	check_getthreadid_np("tid_zero", &g_thr0, 0);
	check_getthreadid_np("tid_one", &g_thr1, 1);
	check_getthreadid_np("tid_neg", &g_thr1, -42);
	check_getthreadid_np("tid_max", &g_thr2, 0x7fffffff);
	check_getthreadid_np("tid_min", &g_thr2, (long)0x80000000);

	check_tcb_ctor("initial_zero_ok", &g_thr0, 0, 1);
	check_tcb_ctor("initial_one_ok", &g_thr0, 1, 1);
	check_tcb_ctor("initial_neg_ok", &g_thr1, -1, 1);
	check_tcb_ctor("initial_two_ok", &g_thr1, 2, 1);
	check_tcb_ctor("initial_one_fail", &g_thr0, 1, 0);
	check_tcb_ctor("initial_zero_null_thread", nullptr, 0, 1);
	check_tcb_ctor("initial_one_null_thread", nullptr, 1, 1);

	drain_alloc_pool();
	check_tcb_ctor("alloc_exhausted", &g_thr0, 0, 1);

	dtor_tcb = make_tcb_for_dtor();
	check_tcb_dtor("allocated", dtor_tcb);
}

static void
test_random(unsigned iters)
{
	unsigned i;

	for (i = 0; i < iters; i++) {
		unsigned op = rnd_below(5u);
		char label[64];

		std::snprintf(label, sizeof(label), "rand%u", i);

		switch (op) {
		case 0:
			check_yield(label);
			break;
		case 1: {
			struct pthread *pt;
			int clock_ret, clock_err;
			clockid_t clock_out, seed;
			long tid;

			tid = (long)(nextr() & 0xffffffffu);
			if ((i % 19u) == 0u)
				pt = nullptr;
			else {
				g_thr0.tid = tid;
				pt = &g_thr0;
			}
			clock_ret = (int)(nextr() & 1u);
			clock_err = (int)(nextr() & 0x7ffu);
			clock_out = (clockid_t)(nextr() & 0xffffffffu);
			seed = (clockid_t)(nextr() & 0xffffffffu);
			check_getcpuclockid(label, pt, clock_ret, clock_err,
			    clock_out, seed);
			break;
		}
		case 2: {
			struct pthread *cur;
			long tid;

			tid = (long)(nextr() & 0xffffffffu);
			switch (rnd_below(3u)) {
			case 0:
				cur = &g_thr0;
				break;
			case 1:
				cur = &g_thr1;
				break;
			default:
				cur = &g_thr2;
				break;
			}
			check_getthreadid_np(label, cur, tid);
			break;
		}
		case 3: {
			struct pthread *thread;
			int initial, get_ok;

			initial = (int)((int)nextr() % 5) - 2;
			get_ok = (int)(nextr() & 1u);
			thread = (rnd_below(2u) == 0u) ? nullptr :
			    &g_thr0;
			check_tcb_ctor(label, thread, initial, get_ok);
			break;
		}
		default: {
			struct tcb *tcb;

			tcb = make_tcb_for_dtor();
			if (tcb != nullptr)
				check_tcb_dtor(label, tcb);
			break;
		}
		}
	}
}

} /* namespace */

int
main(void)
{
	unsigned fn;
	unsigned long long total_cases = 0, total_fails = 0;

	g_thr0.tid = 0;
	g_thr1.tid = 0;
	g_thr2.tid = 0;

	test_edges();
	test_random(SWEEP_ITERS);

	std::printf("\nbatch b0254 differential results\n");
	std::printf("%-24s %12s %10s %s\n", "function", "cases", "failures",
	    "status");
	for (fn = 0; fn < F_COUNT; fn++) {
		std::printf("%-24s %12llu %10llu %s\n", fn_name[fn],
		    n_cases[fn], n_fails[fn],
		    n_fails[fn] == 0u ? "ok" : "FAIL");
		total_cases += n_cases[fn];
		total_fails += n_fails[fn];
	}
	std::printf("%-24s %12llu %10llu %s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0u ? "ok" : "FAIL");

	return total_fails == 0u ? 0 : 1;
}
