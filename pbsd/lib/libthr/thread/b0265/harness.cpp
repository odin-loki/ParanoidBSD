/*
 * Differential test for batch b0265.
 */

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libthr.thread.b0265;

namespace P = pbsd::lib_libthr_thread::b0265;

extern "C" {
struct thr_event_buf {
	int event;
	uintptr_t th_p;
	uintptr_t data;
};
struct pthread {
	unsigned flags;
	struct pthread *joiner;
	struct thr_event_buf event_buf;
};
struct umutex {
	int dummy;
};
typedef struct pthread *pthread_t;

#define THR_FLAGS_DETACHED 0x01000000
#define TD_CREATE 1
#define TD_DEATH 2
#define EINVAL 22

extern struct umutex _thr_event_lock;
extern int errno;

int ref__pthread_single_np(void);
int ref__pthread_getprio(pthread_t pthread);
void ref__thread_bp_create(void);
void ref__thread_bp_death(void);
void ref__thr_report_creation(struct pthread *curthread, struct pthread *newthread);
void ref__thr_report_death(struct pthread *curthread);
int ref__thr_detach(pthread_t pthread);

void b0265_reset_mocks(void);
void b0265_set_curthread(struct pthread *t);
void b0265_set_getschedparam(int ret, int err, int policy, int prio);
void b0265_set_find_thread_ret(int ret);
unsigned b0265_get_suspend_all_count(void);
unsigned b0265_get_umutex_lock_count(void);
unsigned b0265_get_umutex_unlock_count(void);
unsigned b0265_get_thread_unlock_count(void);
unsigned b0265_get_try_gc_count(void);
struct pthread *b0265_get_thread_last_event(void);
pthread_t b0265_get_getschedparam_last_pthread(void);
int b0265_get_find_last_include_dead(void);
struct pthread *b0265_get_find_last_cur(void);
pthread_t b0265_get_find_last_target(void);
struct pthread *b0265_get_thread_unlock_last_cur(void);
pthread_t b0265_get_thread_unlock_last_target(void);
struct pthread *b0265_get_try_gc_last_cur(void);
pthread_t b0265_get_try_gc_last_target(void);
struct pthread *b0265_get_umutex_lock_last_thread(void);
struct umutex *b0265_get_umutex_lock_last_lock(void);
struct pthread *b0265_get_umutex_unlock_last_thread(void);
struct umutex *b0265_get_umutex_unlock_last_lock(void);
}

static struct pthread g_cur;
static struct pthread g_new;
static struct pthread g_joiner;
static struct pthread g_target0;
static struct pthread g_target1;

namespace {

enum Fn {
	F_SINGLE_NP,
	F_GETPRIO,
	F_BP_CREATE,
	F_BP_DEATH,
	F_REPORT_CREATION,
	F_REPORT_DEATH,
	F_DETACH,
	F_COUNT
};

static const char *const fn_name[F_COUNT] = {
	"_pthread_single_np",
	"_pthread_getprio",
	"_thread_bp_create",
	"_thread_bp_death",
	"_thr_report_creation",
	"_thr_report_death",
	"_thr_detach",
};

static unsigned long long n_cases[F_COUNT];
static unsigned long long n_fails[F_COUNT];
static unsigned long long reported[F_COUNT];

static const unsigned SWEEP_ITERS = 200000u;
static const unsigned char GUARD = 0x7f;

static uint64_t rng_state = 0xb0265decafbad01ULL;

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

struct GuardedPthread {
	unsigned char pre[16];
	struct pthread thr;
	unsigned char post[16];
};

static void
init_guarded_pthread(GuardedPthread *g, unsigned flags,
    struct pthread *joiner, int event, uintptr_t th_p, uintptr_t data)
{
	std::memset(g->pre, GUARD, sizeof(g->pre));
	g->thr.flags = flags;
	g->thr.joiner = joiner;
	g->thr.event_buf.event = event;
	g->thr.event_buf.th_p = th_p;
	g->thr.event_buf.data = data;
	std::memset(g->post, GUARD, sizeof(g->post));
}

static bool
guards_intact(const GuardedPthread *g)
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
guarded_equal(const GuardedPthread *a, const GuardedPthread *b)
{
	return std::memcmp(a, b, sizeof(*a)) == 0;
}

static bool
report_creation_bufs_equal(const GuardedPthread *a, const GuardedPthread *b,
    struct pthread *newthread)
{
	if (std::memcmp(a->pre, b->pre, sizeof(a->pre)) != 0)
		return (false);
	if (std::memcmp(a->post, b->post, sizeof(a->post)) != 0)
		return (false);
	if (a->thr.flags != b->thr.flags)
		return (false);
	if (a->thr.joiner != b->thr.joiner)
		return (false);
	if (a->thr.event_buf.event != b->thr.event_buf.event)
		return (false);
	if (a->thr.event_buf.data != b->thr.event_buf.data)
		return (false);
	if (a->thr.event_buf.th_p != (uintptr_t)newthread)
		return (false);
	if (b->thr.event_buf.th_p != (uintptr_t)newthread)
		return (false);
	return (true);
}

static bool
report_death_bufs_equal(const GuardedPthread *a, const GuardedPthread *b)
{
	if (std::memcmp(a->pre, b->pre, sizeof(a->pre)) != 0)
		return (false);
	if (std::memcmp(a->post, b->post, sizeof(a->post)) != 0)
		return (false);
	if (a->thr.flags != b->thr.flags)
		return (false);
	if (a->thr.joiner != b->thr.joiner)
		return (false);
	if (a->thr.event_buf.event != b->thr.event_buf.event)
		return (false);
	if (a->thr.event_buf.data != b->thr.event_buf.data)
		return (false);
	if (a->thr.event_buf.th_p != b->thr.event_buf.th_p)
		return (false);
	return (true);
}

struct SingleNpResult {
	int ret;
	unsigned suspend_delta;
};

static SingleNpResult
run_single_np_port(void)
{
	SingleNpResult r;
	unsigned before;

	b0265_reset_mocks();
	before = b0265_get_suspend_all_count();
	r.ret = P::_pthread_single_np();
	r.suspend_delta = b0265_get_suspend_all_count() - before;
	return (r);
}

static SingleNpResult
run_single_np_ref(void)
{
	SingleNpResult r;
	unsigned before;

	b0265_reset_mocks();
	before = b0265_get_suspend_all_count();
	r.ret = ref__pthread_single_np();
	r.suspend_delta = b0265_get_suspend_all_count() - before;
	return (r);
}

static void
check_single_np(const char *label)
{
	SingleNpResult port, ref;
	bool ok;

	port = run_single_np_port();
	ref = run_single_np_ref();
	ok = (port.ret == ref.ret) && (port.ret == 0) &&
	    (port.suspend_delta == ref.suspend_delta) &&
	    (port.suspend_delta == 1u);
	record_case(F_SINGLE_NP, ok,
	    "%s ret_p=%d ret_r=%d suspend_p=%u suspend_r=%u",
	    label, port.ret, ref.ret, port.suspend_delta, ref.suspend_delta);
}

struct GetprioResult {
	int ret;
	int errno_after;
	pthread_t last_pthread;
};

static GetprioResult
run_getprio_port(pthread_t pthread, int gs_ret, int gs_err, int gs_policy,
    int gs_prio)
{
	GetprioResult r;

	b0265_reset_mocks();
	b0265_set_getschedparam(gs_ret, gs_err, gs_policy, gs_prio);
	r.ret = P::_pthread_getprio(pthread);
	r.errno_after = errno;
	r.last_pthread = b0265_get_getschedparam_last_pthread();
	return (r);
}

static GetprioResult
run_getprio_ref(pthread_t pthread, int gs_ret, int gs_err, int gs_policy,
    int gs_prio)
{
	GetprioResult r;

	b0265_reset_mocks();
	b0265_set_getschedparam(gs_ret, gs_err, gs_policy, gs_prio);
	r.ret = ref__pthread_getprio(pthread);
	r.errno_after = errno;
	r.last_pthread = b0265_get_getschedparam_last_pthread();
	return (r);
}

static void
check_getprio(const char *label, pthread_t pthread, int gs_ret, int gs_err,
    int gs_policy, int gs_prio)
{
	GetprioResult port, ref;
	bool ok;

	port = run_getprio_port(pthread, gs_ret, gs_err, gs_policy, gs_prio);
	ref = run_getprio_ref(pthread, gs_ret, gs_err, gs_policy, gs_prio);

	ok = (port.ret == ref.ret) && (port.errno_after == ref.errno_after) &&
	    (port.last_pthread == ref.last_pthread);

	if (gs_ret == 0)
		ok = ok && (port.ret == gs_prio) && (ref.ret == gs_prio);
	else
		ok = ok && (port.ret == -1) && (ref.ret == -1) &&
		    (port.errno_after == gs_ret) && (ref.errno_after == gs_ret);

	if (pthread != nullptr)
		ok = ok && (port.last_pthread == pthread) &&
		    (ref.last_pthread == pthread);

	record_case(F_GETPRIO, ok,
	    "%s pthread=%p gs_ret=%d ret_p=%d ret_r=%d errno_p=%d errno_r=%d",
	    label, (void *)pthread, gs_ret, port.ret, ref.ret,
	    port.errno_after, ref.errno_after);
}

static void
check_bp_create(const char *label)
{
	unsigned lock_before_p, lock_after_p;
	unsigned lock_before_r, lock_after_r;
	bool ok;

	b0265_reset_mocks();
	lock_before_p = b0265_get_umutex_lock_count();
	P::_thread_bp_create();
	lock_after_p = b0265_get_umutex_lock_count();

	b0265_reset_mocks();
	lock_before_r = b0265_get_umutex_lock_count();
	ref__thread_bp_create();
	lock_after_r = b0265_get_umutex_lock_count();

	ok = ((lock_after_p - lock_before_p) == 0u) &&
	    ((lock_after_r - lock_before_r) == 0u);
	record_case(F_BP_CREATE, ok,
	    "%s lock_delta_p=%u lock_delta_r=%u",
	    label, lock_after_p - lock_before_p, lock_after_r - lock_before_r);
}

static void
check_bp_death(const char *label)
{
	unsigned unlock_before_p, unlock_after_p;
	unsigned unlock_before_r, unlock_after_r;
	bool ok;

	b0265_reset_mocks();
	unlock_before_p = b0265_get_umutex_unlock_count();
	P::_thread_bp_death();
	unlock_after_p = b0265_get_umutex_unlock_count();

	b0265_reset_mocks();
	unlock_before_r = b0265_get_umutex_unlock_count();
	ref__thread_bp_death();
	unlock_after_r = b0265_get_umutex_unlock_count();

	ok = ((unlock_after_p - unlock_before_p) == 0u) &&
	    ((unlock_after_r - unlock_before_r) == 0u);
	record_case(F_BP_DEATH, ok,
	    "%s unlock_delta_p=%u unlock_delta_r=%u",
	    label, unlock_after_p - unlock_before_p,
	    unlock_after_r - unlock_before_r);
}

struct ReportResult {
	GuardedPthread cur;
	unsigned lock_delta;
	unsigned unlock_delta;
	struct pthread *final_last_event;
	struct pthread *lock_last_thread;
	struct umutex *lock_last_lock;
	struct pthread *unlock_last_thread;
	struct umutex *unlock_last_lock;
};

static ReportResult
run_report_creation_port(struct pthread *newthread, const GuardedPthread *in)
{
	ReportResult r;
	unsigned lock_before, unlock_before;

	b0265_reset_mocks();
	r.cur = *in;
	lock_before = b0265_get_umutex_lock_count();
	unlock_before = b0265_get_umutex_unlock_count();
	P::_thr_report_creation(&r.cur.thr, newthread);
	r.lock_delta = b0265_get_umutex_lock_count() - lock_before;
	r.unlock_delta = b0265_get_umutex_unlock_count() - unlock_before;
	r.final_last_event = b0265_get_thread_last_event();
	r.lock_last_thread = b0265_get_umutex_lock_last_thread();
	r.lock_last_lock = b0265_get_umutex_lock_last_lock();
	r.unlock_last_thread = b0265_get_umutex_unlock_last_thread();
	r.unlock_last_lock = b0265_get_umutex_unlock_last_lock();
	return (r);
}

static ReportResult
run_report_creation_ref(struct pthread *newthread, const GuardedPthread *in)
{
	ReportResult r;
	unsigned lock_before, unlock_before;

	b0265_reset_mocks();
	r.cur = *in;
	lock_before = b0265_get_umutex_lock_count();
	unlock_before = b0265_get_umutex_unlock_count();
	ref__thr_report_creation(&r.cur.thr, newthread);
	r.lock_delta = b0265_get_umutex_lock_count() - lock_before;
	r.unlock_delta = b0265_get_umutex_unlock_count() - unlock_before;
	r.final_last_event = b0265_get_thread_last_event();
	r.lock_last_thread = b0265_get_umutex_lock_last_thread();
	r.lock_last_lock = b0265_get_umutex_lock_last_lock();
	r.unlock_last_thread = b0265_get_umutex_unlock_last_thread();
	r.unlock_last_lock = b0265_get_umutex_unlock_last_lock();
	return (r);
}

static void
check_report_creation(const char *label, struct pthread *newthread,
    const GuardedPthread *in)
{
	ReportResult port, ref;
	bool ok;

	port = run_report_creation_port(newthread, in);
	ref = run_report_creation_ref(newthread, in);

	ok = report_creation_bufs_equal(&port.cur, &ref.cur, newthread) &&
	    guards_intact(&port.cur) && guards_intact(&ref.cur) &&
	    (port.lock_delta == ref.lock_delta) &&
	    (port.unlock_delta == ref.unlock_delta) &&
	    (port.lock_delta == 1u) && (port.unlock_delta == 1u) &&
	    (port.final_last_event == ref.final_last_event) &&
	    (port.final_last_event == nullptr) &&
	    (port.lock_last_lock == ref.lock_last_lock) &&
	    (port.lock_last_lock == &_thr_event_lock) &&
	    (port.unlock_last_lock == ref.unlock_last_lock) &&
	    (port.unlock_last_lock == &_thr_event_lock) &&
	    (port.cur.thr.event_buf.event == TD_CREATE) &&
	    (port.cur.thr.event_buf.data == 0);

	record_case(F_REPORT_CREATION, ok,
	    "%s new=%p event=%d th_p=%p",
	    label, (void *)newthread, port.cur.thr.event_buf.event,
	    (void *)(uintptr_t)port.cur.thr.event_buf.th_p);
}

static ReportResult
run_report_death_port(const GuardedPthread *in)
{
	ReportResult r;
	unsigned lock_before, unlock_before;

	b0265_reset_mocks();
	r.cur = *in;
	lock_before = b0265_get_umutex_lock_count();
	unlock_before = b0265_get_umutex_unlock_count();
	P::_thr_report_death(&r.cur.thr);
	r.lock_delta = b0265_get_umutex_lock_count() - lock_before;
	r.unlock_delta = b0265_get_umutex_unlock_count() - unlock_before;
	r.final_last_event = b0265_get_thread_last_event();
	r.lock_last_thread = b0265_get_umutex_lock_last_thread();
	r.lock_last_lock = b0265_get_umutex_lock_last_lock();
	r.unlock_last_thread = b0265_get_umutex_unlock_last_thread();
	r.unlock_last_lock = b0265_get_umutex_unlock_last_lock();
	return (r);
}

static ReportResult
run_report_death_ref(const GuardedPthread *in)
{
	ReportResult r;
	unsigned lock_before, unlock_before;

	b0265_reset_mocks();
	r.cur = *in;
	lock_before = b0265_get_umutex_lock_count();
	unlock_before = b0265_get_umutex_unlock_count();
	ref__thr_report_death(&r.cur.thr);
	r.lock_delta = b0265_get_umutex_lock_count() - lock_before;
	r.unlock_delta = b0265_get_umutex_unlock_count() - unlock_before;
	r.final_last_event = b0265_get_thread_last_event();
	r.lock_last_thread = b0265_get_umutex_lock_last_thread();
	r.lock_last_lock = b0265_get_umutex_lock_last_lock();
	r.unlock_last_thread = b0265_get_umutex_unlock_last_thread();
	r.unlock_last_lock = b0265_get_umutex_unlock_last_lock();
	return (r);
}

static void
check_report_death(const char *label, const GuardedPthread *in)
{
	ReportResult port, ref;
	bool ok;

	port = run_report_death_port(in);
	ref = run_report_death_ref(in);

	ok = report_death_bufs_equal(&port.cur, &ref.cur) &&
	    guards_intact(&port.cur) && guards_intact(&ref.cur) &&
	    (port.lock_delta == ref.lock_delta) &&
	    (port.unlock_delta == ref.unlock_delta) &&
	    (port.lock_delta == 1u) && (port.unlock_delta == 1u) &&
	    (port.final_last_event == ref.final_last_event) &&
	    (port.final_last_event == nullptr) &&
	    (port.lock_last_lock == ref.lock_last_lock) &&
	    (port.lock_last_lock == &_thr_event_lock) &&
	    (port.unlock_last_lock == ref.unlock_last_lock) &&
	    (port.unlock_last_lock == &_thr_event_lock) &&
	    (port.cur.thr.event_buf.event == TD_DEATH) &&
	    (port.cur.thr.event_buf.data == 0);

	record_case(F_REPORT_DEATH, ok,
	    "%s event=%d th_p=%p",
	    label, port.cur.thr.event_buf.event,
	    (void *)(uintptr_t)port.cur.thr.event_buf.th_p);
}

struct DetachResult {
	int ret;
	GuardedPthread target;
	unsigned thread_unlock_delta;
	unsigned try_gc_delta;
	int find_include_dead;
	struct pthread *find_cur;
	pthread_t find_target;
	struct pthread *unlock_cur;
	pthread_t unlock_target;
	struct pthread *try_gc_cur;
	pthread_t try_gc_target;
};

static DetachResult
run_detach_port(struct pthread *cur, int find_ret, GuardedPthread in_copy,
    bool null_target)
{
	DetachResult r;
	unsigned unlock_before, gc_before;

	b0265_reset_mocks();
	b0265_set_curthread(cur);
	b0265_set_find_thread_ret(find_ret);
	r.target = in_copy;
	unlock_before = b0265_get_thread_unlock_count();
	gc_before = b0265_get_try_gc_count();
	if (null_target)
		r.ret = P::_thr_detach(nullptr);
	else
		r.ret = P::_thr_detach(&r.target.thr);
	r.thread_unlock_delta = b0265_get_thread_unlock_count() - unlock_before;
	r.try_gc_delta = b0265_get_try_gc_count() - gc_before;
	r.find_include_dead = b0265_get_find_last_include_dead();
	r.find_cur = b0265_get_find_last_cur();
	r.find_target = b0265_get_find_last_target();
	r.unlock_cur = b0265_get_thread_unlock_last_cur();
	r.unlock_target = b0265_get_thread_unlock_last_target();
	r.try_gc_cur = b0265_get_try_gc_last_cur();
	r.try_gc_target = b0265_get_try_gc_last_target();
	return (r);
}

static DetachResult
run_detach_ref(struct pthread *cur, int find_ret, GuardedPthread in_copy,
    bool null_target)
{
	DetachResult r;
	unsigned unlock_before, gc_before;

	b0265_reset_mocks();
	b0265_set_curthread(cur);
	b0265_set_find_thread_ret(find_ret);
	r.target = in_copy;
	unlock_before = b0265_get_thread_unlock_count();
	gc_before = b0265_get_try_gc_count();
	if (null_target)
		r.ret = ref__thr_detach(nullptr);
	else
		r.ret = ref__thr_detach(&r.target.thr);
	r.thread_unlock_delta = b0265_get_thread_unlock_count() - unlock_before;
	r.try_gc_delta = b0265_get_try_gc_count() - gc_before;
	r.find_include_dead = b0265_get_find_last_include_dead();
	r.find_cur = b0265_get_find_last_cur();
	r.find_target = b0265_get_find_last_target();
	r.unlock_cur = b0265_get_thread_unlock_last_cur();
	r.unlock_target = b0265_get_thread_unlock_last_target();
	r.try_gc_cur = b0265_get_try_gc_last_cur();
	r.try_gc_target = b0265_get_try_gc_last_target();
	return (r);
}

static void
check_detach(const char *label, struct pthread *cur, pthread_t target,
    int find_ret, const GuardedPthread *in)
{
	DetachResult port, ref;
	bool ok;
	unsigned flags_before;
	bool detached_before, joiner_before, expect_unlock, expect_gc,
	    expect_success;

	flags_before = in->thr.flags;
	detached_before = (flags_before & THR_FLAGS_DETACHED) != 0;
	joiner_before = in->thr.joiner != nullptr;
	expect_success = (target != nullptr) && (find_ret == 0) &&
	    !detached_before && !joiner_before;
	expect_unlock = (target != nullptr) && (find_ret == 0) &&
	    (detached_before || joiner_before);
	expect_gc = expect_success;

	port = run_detach_port(cur, find_ret, *in, target == nullptr);
	ref = run_detach_ref(cur, find_ret, *in, target == nullptr);

	ok = (port.ret == ref.ret) &&
	    guarded_equal(&port.target, &ref.target) &&
	    guards_intact(&port.target) && guards_intact(&ref.target) &&
	    (port.thread_unlock_delta == ref.thread_unlock_delta) &&
	    (port.try_gc_delta == ref.try_gc_delta) &&
	    (port.find_include_dead == ref.find_include_dead) &&
	    (port.find_cur == ref.find_cur) &&
	    (port.unlock_cur == ref.unlock_cur) &&
	    (port.try_gc_cur == ref.try_gc_cur);

	if (target == nullptr) {
		ok = ok && (port.ret == EINVAL) && (ref.ret == EINVAL) &&
		    (port.thread_unlock_delta == 0u) &&
		    (port.try_gc_delta == 0u) &&
		    (port.find_target == nullptr) && (ref.find_target == nullptr);
	} else if (find_ret != 0) {
		ok = ok && (port.ret == find_ret) && (ref.ret == find_ret) &&
		    (port.find_include_dead == 1) &&
		    (port.find_cur == cur) && (ref.find_cur == cur) &&
		    (port.find_target == ref.find_target) &&
		    (port.find_target != nullptr) &&
		    (port.thread_unlock_delta == 0u) &&
		    (port.try_gc_delta == 0u);
	} else if (detached_before || joiner_before) {
		ok = ok && (port.ret == EINVAL) && (ref.ret == EINVAL) &&
		    (port.thread_unlock_delta == 1u) &&
		    (port.unlock_cur == cur) && (ref.unlock_cur == cur) &&
		    (port.unlock_target == ref.unlock_target) &&
		    (port.unlock_target != nullptr) &&
		    (port.try_gc_delta == 0u);
	} else {
		ok = ok && (port.ret == 0) && (ref.ret == 0) &&
		    (port.try_gc_delta == 1u) &&
		    (port.try_gc_cur == cur) && (ref.try_gc_cur == cur) &&
		    (port.try_gc_target == ref.try_gc_target) &&
		    (port.try_gc_target != nullptr) &&
		    (port.find_target == ref.find_target) &&
		    (port.find_target != nullptr) &&
		    ((port.target.thr.flags & THR_FLAGS_DETACHED) != 0) &&
		    ((ref.target.thr.flags & THR_FLAGS_DETACHED) != 0);
	}

	(void)expect_unlock;
	(void)expect_gc;
	(void)expect_success;

	record_case(F_DETACH, ok,
	    "%s cur=%p target=%p find_ret=%d ret_p=%d ret_r=%d "
	    "unlock_p=%u unlock_r=%u gc_p=%u gc_r=%u flags_p=%x flags_r=%x",
	    label, (void *)cur, (void *)target, find_ret, port.ret, ref.ret,
	    port.thread_unlock_delta, ref.thread_unlock_delta,
	    port.try_gc_delta, ref.try_gc_delta, port.target.thr.flags,
	    ref.target.thr.flags);
}

static void
test_edges(void)
{
	GuardedPthread gin, gcur, gtarget;

	check_single_np("edge_once");
	check_single_np("edge_twice");

	check_getprio("prio_success_zero", &g_target0, 0, 0, 0, 0);
	check_getprio("prio_success_one", &g_target0, 0, 0, 1, 1);
	check_getprio("prio_success_neg", &g_target1, 0, 0, 2, -1);
	check_getprio("prio_success_max", &g_target1, 0, 0, 3, 0x7fffffff);
	check_getprio("prio_success_min", &g_target0, 0, 0, 4,
	    (int)0x80000000);
	check_getprio("prio_success_high", &g_target1, 0, 0, 5, 0x7f7f7f7f);
	check_getprio("prio_fail_einval", &g_target0, EINVAL, 0, 0, 99);
	check_getprio("prio_fail_custom", &g_target1, 123, 0, 0, 88);
	check_getprio("prio_fail_highbit", &g_target0, 0x80, 0, 0, 77);
	check_getprio("prio_null_thread", nullptr, 0, 0, 0, 42);

	check_bp_create("bp_create_edge");
	check_bp_death("bp_death_edge");

	init_guarded_pthread(&gcur, 0x80808080u, nullptr, TD_DEATH, 0xdead,
	    0xbeef);
	init_guarded_pthread(&gin, 0x40404040u, nullptr, TD_CREATE, 0x1111,
	    0x2222);
	check_report_creation("report_create_basic", &g_new, &gcur);
	check_report_creation("report_create_null_new", nullptr, &gcur);
	check_report_creation("report_create_high_flags", &g_target1, &gcur);

	init_guarded_pthread(&gcur, 0xff00ff00u, &g_joiner, TD_CREATE, 0,
	    0xffffffffu);
	check_report_death("report_death_basic", &gcur);
	check_report_death("report_death_high", &gcur);

	init_guarded_pthread(&gtarget, 0, nullptr, 0, 0, 0);
	check_detach("detach_null", &g_cur, nullptr, 0, &gtarget);

	init_guarded_pthread(&gtarget, 0, nullptr, 0, 0, 0);
	check_detach("detach_find_fail", &g_cur, &gtarget.thr, EINVAL, &gtarget);

	init_guarded_pthread(&gtarget, 0, nullptr, 0, 0, 0);
	check_detach("detach_find_custom", &g_cur, &gtarget.thr, 77, &gtarget);

	init_guarded_pthread(&gtarget, THR_FLAGS_DETACHED, nullptr, 0, 0, 0);
	check_detach("detach_already", &g_cur, &gtarget.thr, 0, &gtarget);

	init_guarded_pthread(&gtarget, 0, &g_joiner, 0, 0, 0);
	check_detach("detach_joiner", &g_cur, &gtarget.thr, 0, &gtarget);

	init_guarded_pthread(&gtarget, THR_FLAGS_DETACHED, &g_joiner, 0, 0, 0);
	check_detach("detach_both_flags", &g_cur, &gtarget.thr, 0, &gtarget);

	init_guarded_pthread(&gtarget, 0x80000000u, nullptr, 0, 0, 0);
	check_detach("detach_success_high", &g_cur, &gtarget.thr, 0, &gtarget);

	init_guarded_pthread(&gtarget, 0, nullptr, 0, 0x80808080u, 0x7f7f7f7f);
	check_detach("detach_success_guarded", &g_cur, &gtarget.thr, 0, &gtarget);
}

static void
test_random(unsigned iters)
{
	unsigned i;

	for (i = 0; i < iters; i++) {
		unsigned op = rnd_below(7u);
		char label[64];

		std::snprintf(label, sizeof(label), "rand%u", i);

		switch (op) {
		case 0:
			check_single_np(label);
			break;
		case 1: {
			pthread_t pt;
			int gs_ret, gs_err, gs_policy, gs_prio;

			if ((i % 17u) == 0u)
				pt = nullptr;
			else if ((i % 3u) == 0u)
				pt = &g_target0;
			else
				pt = &g_target1;
			gs_ret = (int)(nextr() & 0xffu);
			if ((i % 5u) == 0u)
				gs_ret = 0;
			gs_err = (int)(nextr() & 0x7ffu);
			gs_policy = (int)(nextr() & 0xffffffffu);
			gs_prio = (int)(nextr() & 0xffffffffu);
			check_getprio(label, pt, gs_ret, gs_err, gs_policy,
			    gs_prio);
			break;
		}
		case 2:
			check_bp_create(label);
			break;
		case 3:
			check_bp_death(label);
			break;
		case 4: {
			GuardedPthread gcur;
			struct pthread *newt;
			unsigned flags, ev;
			uintptr_t th_p, data;

			flags = (unsigned)(nextr() & 0xffffffffu);
			ev = (int)(nextr() & 0xffu);
			th_p = (uintptr_t)(nextr() & 0xffffffffu);
			data = (uintptr_t)(nextr() & 0xffffffffu);
			init_guarded_pthread(&gcur, flags, nullptr, ev, th_p,
			    data);
			newt = (rnd_below(3u) == 0u) ? nullptr :
			    ((rnd_below(2u) == 0u) ? &g_new : &g_target1);
			check_report_creation(label, newt, &gcur);
			break;
		}
		case 5: {
			GuardedPthread gcur;
			unsigned flags;
			uintptr_t th_p, data;
			struct pthread *joiner;

			flags = (unsigned)(nextr() & 0xffffffffu);
			th_p = (uintptr_t)(nextr() & 0xffffffffu);
			data = (uintptr_t)(nextr() & 0xffffffffu);
			joiner = (rnd_below(2u) == 0u) ? nullptr : &g_joiner;
			init_guarded_pthread(&gcur, flags, joiner,
			    (int)(nextr() & 0xffu), th_p, data);
			check_report_death(label, &gcur);
			break;
		}
		default: {
			GuardedPthread gtarget;
			struct pthread *cur;
			pthread_t target;
			int find_ret;
			unsigned flags;
			struct pthread *joiner;

			flags = (unsigned)(nextr() & 0xffffffffu);
			joiner = (rnd_below(4u) == 0u) ? &g_joiner : nullptr;
			init_guarded_pthread(&gtarget, flags, joiner,
			    (int)(nextr() & 0xffu),
			    (uintptr_t)(nextr() & 0xffffffffu),
			    (uintptr_t)(nextr() & 0xffffffffu));
			cur = (rnd_below(2u) == 0u) ? &g_cur : &g_target1;
			if ((i % 23u) == 0u)
				target = nullptr;
			else
				target = &gtarget.thr;
			find_ret = (int)(nextr() & 0xffu);
			if ((i % 7u) == 0u)
				find_ret = 0;
			check_detach(label, cur, target, find_ret, &gtarget);
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

	std::memset(&g_cur, 0, sizeof(g_cur));
	std::memset(&g_new, 0, sizeof(g_new));
	std::memset(&g_joiner, 0, sizeof(g_joiner));
	std::memset(&g_target0, 0, sizeof(g_target0));
	std::memset(&g_target1, 0, sizeof(g_target1));

	test_edges();
	test_random(SWEEP_ITERS);

	std::printf("\nbatch b0265 differential results\n");
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
