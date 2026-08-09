/*
 * Differential test for batch b0271.
 */

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libthr.thread.b0271;

namespace P = pbsd::lib_libthr_thread::b0271;

extern "C" {
struct pthread_attr_part {
	int sched_policy;
	int prio;
};
struct pthread {
	struct pthread_attr_part attr;
	long tid;
};
typedef struct pthread *pthread_t;

#define SCHED_OTHER 0
#define EINVAL 22
#define ENOTSUP 95
#define _SIG_MAXSIG 64

typedef void (*pthread_switch_routine_t)(pthread_t, pthread_t);

int ref__pthread_getconcurrency(void);
int ref__pthread_setconcurrency(int new_level);
int ref__pthread_setprio(pthread_t pthread, int prio);
int ref__pthread_switch_add_np(pthread_switch_routine_t routine);
int ref__pthread_switch_delete_np(pthread_switch_routine_t routine);
int ref__Tthr_kill(pthread_t pthread, int sig);

void b0271_reset_mocks(void);
void b0271_set_curthread(struct pthread *t);
void b0271_set_find_thread_ret(int ret);
void b0271_set_setscheduler(int ret, int err);
unsigned b0271_get_thr_lock_count(void);
struct pthread *b0271_get_thr_lock_last_thread(void);
unsigned b0271_get_thread_unlock_count(void);
struct pthread *b0271_get_thread_unlock_last_cur(void);
pthread_t b0271_get_thread_unlock_last_target(void);
int b0271_get_find_last_include_dead(void);
struct pthread *b0271_get_find_last_cur(void);
pthread_t b0271_get_find_last_target(void);
long b0271_get_setscheduler_last_tid(void);
int b0271_get_setscheduler_last_policy(void);
int b0271_get_setscheduler_last_prio(void);
unsigned b0271_get_send_sig_count(void);
pthread_t b0271_get_send_sig_last_pthread(void);
int b0271_get_send_sig_last_sig(void);
}

static GuardedPthread g_cur_g;
static GuardedPthread g_target0_g;
static GuardedPthread g_target1_g;

static void dummy_switch(pthread_t, pthread_t) {}

namespace {

enum Fn {
	F_GETCONCURRENCY,
	F_SETCONCURRENCY,
	F_SETPRIO,
	F_SWITCH_ADD,
	F_SWITCH_DELETE,
	F_KILL,
	F_COUNT
};

static const char *const fn_name[F_COUNT] = {
	"_pthread_getconcurrency",
	"_pthread_setconcurrency",
	"_pthread_setprio",
	"_pthread_switch_add_np",
	"_pthread_switch_delete_np",
	"_Tthr_kill",
};

static unsigned long long n_cases[F_COUNT];
static unsigned long long n_fails[F_COUNT];
static unsigned long long reported[F_COUNT];

static const unsigned SWEEP_ITERS = 200000u;
static const unsigned char GUARD = 0x7f;

static uint64_t rng_state = 0xb0271decafbad01ULL;

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
init_guarded_pthread(GuardedPthread *g, int policy, int prio, long tid)
{
	std::memset(g->pre, GUARD, sizeof(g->pre));
	g->thr.attr.sched_policy = policy;
	g->thr.attr.prio = prio;
	g->thr.tid = tid;
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

static void
normalize_concurrency_port(void)
{
	P::_pthread_setconcurrency(0);
}

static void
normalize_concurrency_ref(void)
{
	ref__pthread_setconcurrency(0);
}

struct ConcurrencySetResult {
	int set_ret;
	int get_after;
};

static ConcurrencySetResult
run_setconcurrency_port(int new_level)
{
	ConcurrencySetResult r;

	b0271_reset_mocks();
	normalize_concurrency_port();
	r.set_ret = P::_pthread_setconcurrency(new_level);
	r.get_after = P::_pthread_getconcurrency();
	return (r);
}

static ConcurrencySetResult
run_setconcurrency_ref(int new_level)
{
	ConcurrencySetResult r;

	b0271_reset_mocks();
	normalize_concurrency_ref();
	r.set_ret = ref__pthread_setconcurrency(new_level);
	r.get_after = ref__pthread_getconcurrency();
	return (r);
}

static void
check_setconcurrency(const char *label, int new_level)
{
	ConcurrencySetResult port, ref;
	bool ok;

	port = run_setconcurrency_port(new_level);
	ref = run_setconcurrency_ref(new_level);

	ok = (port.set_ret == ref.set_ret) &&
	    (port.get_after == ref.get_after);

	if (new_level < 0) {
		ok = ok && (port.set_ret == EINVAL) && (ref.set_ret == EINVAL) &&
		    (port.get_after == 0) && (ref.get_after == 0);
	} else {
		ok = ok && (port.set_ret == 0) && (ref.set_ret == 0) &&
		    (port.get_after == new_level) &&
		    (ref.get_after == new_level);
	}

	record_case(F_SETCONCURRENCY, ok,
	    "%s level=%d set_p=%d set_r=%d get_p=%d get_r=%d",
	    label, new_level, port.set_ret, ref.set_ret, port.get_after,
	    ref.get_after);
}

struct ConcurrencyGetResult {
	int get_val;
};

static ConcurrencyGetResult
run_getconcurrency_port(int setup_level)
{
	ConcurrencyGetResult r;

	b0271_reset_mocks();
	normalize_concurrency_port();
	if (setup_level >= 0)
		P::_pthread_setconcurrency(setup_level);
	r.get_val = P::_pthread_getconcurrency();
	return (r);
}

static ConcurrencyGetResult
run_getconcurrency_ref(int setup_level)
{
	ConcurrencyGetResult r;

	b0271_reset_mocks();
	normalize_concurrency_ref();
	if (setup_level >= 0)
		ref__pthread_setconcurrency(setup_level);
	r.get_val = ref__pthread_getconcurrency();
	return (r);
}

static void
check_getconcurrency(const char *label, int setup_level)
{
	ConcurrencyGetResult port, ref;
	bool ok;

	port = run_getconcurrency_port(setup_level);
	ref = run_getconcurrency_ref(setup_level);

	ok = (port.get_val == ref.get_val);
	if (setup_level >= 0)
		ok = ok && (port.get_val == setup_level) &&
		    (ref.get_val == setup_level);

	record_case(F_GETCONCURRENCY, ok,
	    "%s setup=%d get_p=%d get_r=%d", label, setup_level,
	    port.get_val, ref.get_val);
}

struct SetprioResult {
	int ret;
	GuardedPthread target;
	unsigned lock_delta;
	unsigned unlock_delta;
	struct pthread *lock_last;
	struct pthread *unlock_cur;
	pthread_t unlock_target;
	long sched_tid;
	int sched_policy;
	int sched_prio;
};

static SetprioResult
run_setprio_port(struct pthread *cur, pthread_t target, int find_ret,
    int sched_ret, int sched_err, const GuardedPthread *in, int prio)
{
	SetprioResult r;
	unsigned lock_before, unlock_before;

	b0271_reset_mocks();
	b0271_set_curthread(cur);
	b0271_set_find_thread_ret(find_ret);
	b0271_set_setscheduler(sched_ret, sched_err);
	r.target = *in;
	lock_before = b0271_get_thr_lock_count();
	unlock_before = b0271_get_thread_unlock_count();
	r.ret = P::_pthread_setprio(target, prio);
	r.lock_delta = b0271_get_thr_lock_count() - lock_before;
	r.unlock_delta = b0271_get_thread_unlock_count() - unlock_before;
	r.lock_last = b0271_get_thr_lock_last_thread();
	r.unlock_cur = b0271_get_thread_unlock_last_cur();
	r.unlock_target = b0271_get_thread_unlock_last_target();
	r.sched_tid = b0271_get_setscheduler_last_tid();
	r.sched_policy = b0271_get_setscheduler_last_policy();
	r.sched_prio = b0271_get_setscheduler_last_prio();
	return (r);
}

static SetprioResult
run_setprio_ref(struct pthread *cur, pthread_t target, int find_ret,
    int sched_ret, int sched_err, const GuardedPthread *in, int prio)
{
	SetprioResult r;
	unsigned lock_before, unlock_before;

	b0271_reset_mocks();
	b0271_set_curthread(cur);
	b0271_set_find_thread_ret(find_ret);
	b0271_set_setscheduler(sched_ret, sched_err);
	r.target = *in;
	lock_before = b0271_get_thr_lock_count();
	unlock_before = b0271_get_thread_unlock_count();
	r.ret = ref__pthread_setprio(target, prio);
	r.lock_delta = b0271_get_thr_lock_count() - lock_before;
	r.unlock_delta = b0271_get_thread_unlock_count() - unlock_before;
	r.lock_last = b0271_get_thr_lock_last_thread();
	r.unlock_cur = b0271_get_thread_unlock_last_cur();
	r.unlock_target = b0271_get_thread_unlock_last_target();
	r.sched_tid = b0271_get_setscheduler_last_tid();
	r.sched_policy = b0271_get_setscheduler_last_policy();
	r.sched_prio = b0271_get_setscheduler_last_prio();
	return (r);
}

static void
check_setprio(const char *label, struct pthread *cur, pthread_t target,
    int find_ret, int sched_ret, int sched_err, const GuardedPthread *in,
    int prio)
{
	SetprioResult port, ref;
	bool ok;
	bool self, find_ok, sched_other, same_prio, expect_sched;
	unsigned expect_lock, expect_unlock;

	self = (cur == target);
	find_ok = (self || find_ret == 0);
	sched_other = (in->thr.attr.sched_policy == SCHED_OTHER);
	same_prio = (in->thr.attr.prio == prio);
	expect_sched = find_ok && !sched_other && !same_prio;
	expect_lock = self ? 1u : 0u;
	expect_unlock = find_ok ? 1u : 0u;

	port = run_setprio_port(cur, target, find_ret, sched_ret, sched_err,
	    in, prio);
	ref = run_setprio_ref(cur, target, find_ret, sched_ret, sched_err,
	    in, prio);

	ok = (port.ret == ref.ret) &&
	    guarded_equal(&port.target, &ref.target) &&
	    guards_intact(&port.target) && guards_intact(&ref.target) &&
	    (port.lock_delta == ref.lock_delta) &&
	    (port.unlock_delta == ref.unlock_delta) &&
	    (port.lock_last == ref.lock_last) &&
	    (port.unlock_cur == ref.unlock_cur) &&
	    (port.unlock_target == ref.unlock_target) &&
	    (port.sched_tid == ref.sched_tid) &&
	    (port.sched_policy == ref.sched_policy) &&
	    (port.sched_prio == ref.sched_prio);

	if (!find_ok) {
		ok = ok && (port.ret == find_ret) && (ref.ret == find_ret) &&
		    (port.lock_delta == 0u) && (port.unlock_delta == 0u) &&
		    (port.target.thr.attr.prio == in->thr.attr.prio);
	} else {
		ok = ok && (port.lock_delta == expect_lock) &&
		    (port.unlock_delta == expect_unlock) &&
		    (port.unlock_cur == cur) &&
		    (port.unlock_target == target) &&
		    (port.target.thr.attr.prio == prio) &&
		    (ref.target.thr.attr.prio == prio);

		if (expect_sched) {
			if (sched_ret == -1) {
				ok = ok && (port.ret == sched_err) &&
				    (ref.ret == sched_err) &&
				    (port.target.thr.attr.prio ==
					in->thr.attr.prio);
			} else {
				ok = ok && (port.ret == 0) && (ref.ret == 0) &&
				    (port.sched_tid == in->thr.tid) &&
				    (port.sched_policy ==
					in->thr.attr.sched_policy) &&
				    (port.sched_prio == prio);
			}
		} else {
			ok = ok && (port.ret == 0) && (ref.ret == 0) &&
			    (port.sched_tid == 0) && (port.sched_prio == 0);
		}
	}

	record_case(F_SETPRIO, ok,
	    "%s cur=%p tgt=%p find=%d prio=%d ret_p=%d ret_r=%d "
	    "lock_p=%u lock_r=%u unlock_p=%u unlock_r=%u attr_p=%d attr_r=%d",
	    label, (void *)cur, (void *)target, find_ret, prio, port.ret,
	    ref.ret, port.lock_delta, ref.lock_delta, port.unlock_delta,
	    ref.unlock_delta, port.target.thr.attr.prio,
	    ref.target.thr.attr.prio);
}

struct SwitchResult {
	int ret;
};

static SwitchResult
run_switch_add_port(pthread_switch_routine_t fn)
{
	SwitchResult r;

	b0271_reset_mocks();
	r.ret = P::_pthread_switch_add_np(fn);
	return (r);
}

static SwitchResult
run_switch_add_ref(pthread_switch_routine_t fn)
{
	SwitchResult r;

	b0271_reset_mocks();
	r.ret = ref__pthread_switch_add_np(fn);
	return (r);
}

static void
check_switch_add(const char *label, pthread_switch_routine_t fn)
{
	SwitchResult port, ref;
	bool ok;

	port = run_switch_add_port(fn);
	ref = run_switch_add_ref(fn);
	ok = (port.ret == ref.ret) && (port.ret == ENOTSUP) &&
	    (ref.ret == ENOTSUP);
	record_case(F_SWITCH_ADD, ok, "%s ret_p=%d ret_r=%d", label, port.ret,
	    ref.ret);
}

static SwitchResult
run_switch_delete_port(pthread_switch_routine_t fn)
{
	SwitchResult r;

	b0271_reset_mocks();
	r.ret = P::_pthread_switch_delete_np(fn);
	return (r);
}

static SwitchResult
run_switch_delete_ref(pthread_switch_routine_t fn)
{
	SwitchResult r;

	b0271_reset_mocks();
	r.ret = ref__pthread_switch_delete_np(fn);
	return (r);
}

static void
check_switch_delete(const char *label, pthread_switch_routine_t fn)
{
	SwitchResult port, ref;
	bool ok;

	port = run_switch_delete_port(fn);
	ref = run_switch_delete_ref(fn);
	ok = (port.ret == ref.ret) && (port.ret == ENOTSUP) &&
	    (ref.ret == ENOTSUP);
	record_case(F_SWITCH_DELETE, ok, "%s ret_p=%d ret_r=%d", label,
	    port.ret, ref.ret);
}

struct KillResult {
	int ret;
	unsigned send_sig_delta;
	pthread_t send_last_pthread;
	int send_last_sig;
	unsigned unlock_delta;
	struct pthread *find_cur;
	pthread_t find_target;
	int find_include_dead;
};

static KillResult
run_kill_port(struct pthread *cur, pthread_t target, int find_ret, int sig)
{
	KillResult r;
	unsigned send_before, unlock_before;

	b0271_reset_mocks();
	b0271_set_curthread(cur);
	b0271_set_find_thread_ret(find_ret);
	send_before = b0271_get_send_sig_count();
	unlock_before = b0271_get_thread_unlock_count();
	r.ret = P::_Tthr_kill(target, sig);
	r.send_sig_delta = b0271_get_send_sig_count() - send_before;
	r.send_last_pthread = b0271_get_send_sig_last_pthread();
	r.send_last_sig = b0271_get_send_sig_last_sig();
	r.unlock_delta = b0271_get_thread_unlock_count() - unlock_before;
	r.find_cur = b0271_get_find_last_cur();
	r.find_target = b0271_get_find_last_target();
	r.find_include_dead = b0271_get_find_last_include_dead();
	return (r);
}

static KillResult
run_kill_ref(struct pthread *cur, pthread_t target, int find_ret, int sig)
{
	KillResult r;
	unsigned send_before, unlock_before;

	b0271_reset_mocks();
	b0271_set_curthread(cur);
	b0271_set_find_thread_ret(find_ret);
	send_before = b0271_get_send_sig_count();
	unlock_before = b0271_get_thread_unlock_count();
	r.ret = ref__Tthr_kill(target, sig);
	r.send_sig_delta = b0271_get_send_sig_count() - send_before;
	r.send_last_pthread = b0271_get_send_sig_last_pthread();
	r.send_last_sig = b0271_get_send_sig_last_sig();
	r.unlock_delta = b0271_get_thread_unlock_count() - unlock_before;
	r.find_cur = b0271_get_find_last_cur();
	r.find_target = b0271_get_find_last_target();
	r.find_include_dead = b0271_get_find_last_include_dead();
	return (r);
}

static void
check_kill(const char *label, struct pthread *cur, pthread_t target,
    int find_ret, int sig)
{
	KillResult port, ref;
	bool ok;
	bool invalid_sig, self, find_ok, expect_send, expect_unlock;

	invalid_sig = (sig < 0 || sig > _SIG_MAXSIG);
	self = (cur == target);
	find_ok = (!invalid_sig && !self && find_ret == 0);
	expect_send = !invalid_sig && sig > 0 && (self || find_ok);
	expect_unlock = find_ok;

	port = run_kill_port(cur, target, find_ret, sig);
	ref = run_kill_ref(cur, target, find_ret, sig);

	ok = (port.ret == ref.ret) &&
	    (port.send_sig_delta == ref.send_sig_delta) &&
	    (port.send_last_pthread == ref.send_last_pthread) &&
	    (port.send_last_sig == ref.send_last_sig) &&
	    (port.unlock_delta == ref.unlock_delta) &&
	    (port.find_cur == ref.find_cur) &&
	    (port.find_target == ref.find_target) &&
	    (port.find_include_dead == ref.find_include_dead);

	if (invalid_sig) {
		ok = ok && (port.ret == EINVAL) && (ref.ret == EINVAL) &&
		    (port.send_sig_delta == 0u) &&
		    (port.unlock_delta == 0u);
	} else if (self) {
		ok = ok && (port.ret == 0) && (ref.ret == 0) &&
		    (port.send_sig_delta == (expect_send ? 1u : 0u)) &&
		    (port.unlock_delta == 0u);
		if (expect_send) {
			ok = ok && (port.send_last_pthread == target) &&
			    (port.send_last_sig == sig);
		}
	} else if (find_ret != 0) {
		ok = ok && (port.ret == find_ret) && (ref.ret == find_ret) &&
		    (port.send_sig_delta == 0u) &&
		    (port.unlock_delta == 0u) &&
		    (port.find_include_dead == 0) &&
		    (port.find_cur == cur) && (port.find_target == target);
	} else {
		ok = ok && (port.ret == 0) && (ref.ret == 0) &&
		    (port.send_sig_delta == (expect_send ? 1u : 0u)) &&
		    (port.unlock_delta == 1u);
		if (expect_send) {
			ok = ok && (port.send_last_pthread == target) &&
			    (port.send_last_sig == sig);
		}
	}

	record_case(F_KILL, ok,
	    "%s cur=%p tgt=%p find=%d sig=%d ret_p=%d ret_r=%d "
	    "send_p=%u send_r=%u unlock_p=%u unlock_r=%u",
	    label, (void *)cur, (void *)target, find_ret, sig, port.ret,
	    ref.ret, port.send_sig_delta, ref.send_sig_delta,
	    port.unlock_delta, ref.unlock_delta);
}

static void
test_edges(void)
{
	GuardedPthread gtarget;

	check_setconcurrency("set_neg_one", -1);
	check_setconcurrency("set_zero", 0);
	check_setconcurrency("set_one", 1);
	check_setconcurrency("set_max", 0x7fffffff);
	check_setconcurrency("set_min_neg", (int)0x80000000);
	check_setconcurrency("set_highbit", 0x80808080);

	check_getconcurrency("get_after_zero", 0);
	check_getconcurrency("get_after_one", 1);
	check_getconcurrency("get_after_large", 12345);
	check_getconcurrency("get_after_max", 0x7fffffff);

	init_guarded_pthread(&gtarget, SCHED_OTHER, 0, 1);
	check_setprio("prio_self_other", &g_cur, &g_cur, 0, 0, 0, &gtarget, 5);

	init_guarded_pthread(&gtarget, SCHED_OTHER, 10, 2);
	check_setprio("prio_other_policy", &g_cur, &gtarget.thr, 0, 0, 0,
	    &gtarget, 20);

	init_guarded_pthread(&gtarget, 1, 10, 3);
	check_setprio("prio_same", &g_cur, &gtarget.thr, 0, 0, 0, &gtarget, 10);

	init_guarded_pthread(&gtarget, 2, 5, 4);
	check_setprio("prio_sched_ok", &g_cur, &gtarget.thr, 0, 0, 0, &gtarget,
	    15);

	init_guarded_pthread(&gtarget, 3, 5, 5);
	check_setprio("prio_sched_fail", &g_cur, &gtarget.thr, 0, -1, EINVAL,
	    &gtarget, 25);

	init_guarded_pthread(&gtarget, 4, 0, 6);
	check_setprio("prio_find_fail", &g_cur, &gtarget.thr, EINVAL, 0, 0,
	    &gtarget, 7);

	init_guarded_pthread(&gtarget, 5, 0x7f, 7);
	check_setprio("prio_high", &g_cur, &gtarget.thr, 0, 0, 0, &gtarget,
	    0x7f7f7f7f);

	init_guarded_pthread(&gtarget, 6, (int)0x80808080, 8);
	check_setprio("prio_neg", &g_cur, &gtarget.thr, 0, -1, 123, &gtarget,
	    -1);

	check_switch_add("switch_add_null", nullptr);
	check_switch_add("switch_add_fn", dummy_switch);
	check_switch_delete("switch_del_null", nullptr);
	check_switch_delete("switch_del_fn", dummy_switch);

	check_kill("kill_sig_neg", &g_cur, &g_target0, 0, -1);
	check_kill("kill_sig_over", &g_cur, &g_target0, 0, _SIG_MAXSIG + 1);
	check_kill("kill_sig_max", &g_cur, &g_target0, 0, _SIG_MAXSIG);
	check_kill("kill_sig_zero_self", &g_cur, &g_cur, 0, 0);
	check_kill("kill_sig_zero_other", &g_cur, &g_target0, 0, 0);
	check_kill("kill_self_sig", &g_cur, &g_cur, 0, 9);
	check_kill("kill_other_ok", &g_cur, &g_target1, 0, 15);
	check_kill("kill_find_fail", &g_cur, &g_target1, EINVAL, 10);
	check_kill("kill_sig_one", &g_cur, &g_target0, 0, 1);
}

static void
test_random(unsigned iters)
{
	unsigned i;

	for (i = 0; i < iters; i++) {
		unsigned op = rnd_below(6u);
		char label[64];

		std::snprintf(label, sizeof(label), "rand%u", i);

		switch (op) {
		case 0:
			check_setconcurrency(label,
			    (int)(nextr() & 0xffffffffu));
			break;
		case 1: {
			int setup;

			setup = (int)(nextr() & 0x7fffffffu);
			if ((i % 11u) == 0u)
				setup = -1;
			check_getconcurrency(label, setup);
			break;
		}
		case 2: {
			GuardedPthread gtarget;
			struct pthread *cur;
			pthread_t target;
			int find_ret, sched_ret, sched_err, prio, policy, old_prio;
			long tid;

			policy = (int)(nextr() & 0xffu);
			if ((i % 5u) == 0u)
				policy = SCHED_OTHER;
			old_prio = (int)(nextr() & 0xffffffffu);
			tid = (long)(nextr() & 0xffffffffu);
			init_guarded_pthread(&gtarget, policy, old_prio, tid);
			cur = (rnd_below(2u) == 0u) ? &g_cur : &g_target1;
			if ((i % 13u) == 0u)
				target = cur;
			else
				target = &gtarget.thr;
			find_ret = (int)(nextr() & 0xffu);
			if ((i % 7u) == 0u)
				find_ret = 0;
			sched_ret = ((i % 3u) == 0u) ? -1 : 0;
			sched_err = (int)(nextr() & 0x7ffu);
			prio = (int)(nextr() & 0xffffffffu);
			check_setprio(label, cur, target, find_ret, sched_ret,
			    sched_err, &gtarget, prio);
			break;
		}
		case 3:
			check_switch_add(label,
			    (rnd_below(2u) == 0u) ? nullptr : dummy_switch);
			break;
		case 4:
			check_switch_delete(label,
			    (rnd_below(2u) == 0u) ? nullptr : dummy_switch);
			break;
		default: {
			struct pthread *cur;
			pthread_t target;
			int find_ret, sig;

			cur = (rnd_below(2u) == 0u) ? &g_cur : &g_target1;
			if ((i % 17u) == 0u)
				target = cur;
			else
				target = (rnd_below(2u) == 0u) ? &g_target0 :
				    &g_target1;
			find_ret = (int)(nextr() & 0xffu);
			if ((i % 9u) == 0u)
				find_ret = 0;
			sig = (int)(nextr() & 0xffffffffu);
			if ((i % 19u) == 0u)
				sig = -1;
			else if ((i % 23u) == 0u)
				sig = _SIG_MAXSIG + (int)(nextr() & 3u);
			else if ((i % 29u) == 0u)
				sig = 0;
			check_kill(label, cur, target, find_ret, sig);
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
	std::memset(&g_target0, 0, sizeof(g_target0));
	std::memset(&g_target1, 0, sizeof(g_target1));

	test_edges();
	test_random(SWEEP_ITERS);

	std::printf("\nbatch b0271 differential results\n");
	std::printf("%-28s %12s %10s %s\n", "function", "cases", "failures",
	    "status");
	for (fn = 0; fn < F_COUNT; fn++) {
		std::printf("%-28s %12llu %10llu %s\n", fn_name[fn],
		    n_cases[fn], n_fails[fn],
		    n_fails[fn] == 0u ? "ok" : "FAIL");
		total_cases += n_cases[fn];
		total_fails += n_fails[fn];
	}
	std::printf("%-28s %12llu %10llu %s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0u ? "ok" : "FAIL");

	return total_fails == 0u ? 0 : 1;
}
