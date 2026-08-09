/*
 * Differential test for batch b0278.
 */

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libthr.thread.b0278;

namespace P = pbsd::lib_libthr_thread::b0278;

extern "C" {
typedef struct {
	unsigned char bits[128];
} cpuset_t;

typedef struct pthread *pthread_t;

struct sched_param {
	int sched_priority;
};

struct pthread_attr_part {
	int sched_policy;
	int prio;
};

struct pthread {
	struct pthread_attr_part attr;
	long tid;
};

struct pthread_rwlockattr {
	int pshared;
};

typedef struct pthread_rwlockattr *pthread_rwlockattr_t;

struct pthread_barrierattr {
	int pshared;
};

typedef struct pthread_barrierattr *pthread_barrierattr_t;

#define CPU_LEVEL_WHICH 1
#define CPU_WHICH_TID 2
#define SCHED_OTHER 0
#define EINVAL 22
#define ENOMEM 12
#define PTHREAD_PROCESS_PRIVATE 0
#define PTHREAD_PROCESS_SHARED 1

int ref__pthread_setaffinity_np(pthread_t td, size_t cpusetsize,
    const cpuset_t *cpusetp);
int ref__pthread_getaffinity_np(pthread_t td, size_t cpusetsize,
    cpuset_t *cpusetp);
int ref__pthread_rwlockattr_destroy(pthread_rwlockattr_t *rwlockattr);
int ref__pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *rwlockattr,
    int *pshared);
int ref__pthread_rwlockattr_init(pthread_rwlockattr_t *rwlockattr);
int ref__pthread_rwlockattr_setpshared(pthread_rwlockattr_t *rwlockattr,
    int pshared);
int ref__pthread_barrierattr_destroy(pthread_barrierattr_t *attr);
int ref__pthread_barrierattr_getpshared(const pthread_barrierattr_t *attr,
    int *pshared);
int ref__pthread_barrierattr_init(pthread_barrierattr_t *attr);
int ref__pthread_barrierattr_setpshared(pthread_barrierattr_t *attr,
    int pshared);
int ref__pthread_setschedparam(pthread_t pthread, int policy,
    const struct sched_param *param);

void b0278_reset_mocks(void);
void b0278_set_curthread(struct pthread *t);
void b0278_set_find_thread_ret(int ret);
void b0278_set_setscheduler(int ret, int err);
void b0278_set_setaffinity(int ret, int err);
void b0278_set_getaffinity(int ret, int err, unsigned char fill);
unsigned b0278_get_thr_lock_count(void);
struct pthread *b0278_get_thr_lock_last_thread(void);
unsigned b0278_get_thread_unlock_count(void);
struct pthread *b0278_get_thread_unlock_last_cur(void);
pthread_t b0278_get_thread_unlock_last_target(void);
int b0278_get_find_last_include_dead(void);
struct pthread *b0278_get_find_last_cur(void);
pthread_t b0278_get_find_last_target(void);
long b0278_get_setscheduler_last_tid(void);
int b0278_get_setscheduler_last_policy(void);
int b0278_get_setscheduler_last_prio(void);
int b0278_get_setaffinity_last_level(void);
int b0278_get_setaffinity_last_which(void);
long b0278_get_setaffinity_last_id(void);
size_t b0278_get_setaffinity_last_size(void);
const cpuset_t *b0278_get_setaffinity_last_cpuset(void);
int b0278_get_getaffinity_last_level(void);
int b0278_get_getaffinity_last_which(void);
long b0278_get_getaffinity_last_id(void);
size_t b0278_get_getaffinity_last_size(void);
}

static struct pthread g_cur;
static struct pthread g_target0;
static struct pthread g_target1;

namespace {

enum Fn {
	F_SETAFFINITY,
	F_GETAFFINITY,
	F_RWLOCKATTR_DESTROY,
	F_RWLOCKATTR_GETPSHARED,
	F_RWLOCKATTR_INIT,
	F_RWLOCKATTR_SETPSHARED,
	F_BARRIERATTR_DESTROY,
	F_BARRIERATTR_INIT,
	F_BARRIERATTR_SETPSHARED,
	F_BARRIERATTR_GETPSHARED,
	F_SETSCHEDPARAM,
	F_COUNT
};

static const char *const fn_name[F_COUNT] = {
	"_pthread_setaffinity_np",
	"_pthread_getaffinity_np",
	"_pthread_rwlockattr_destroy",
	"_pthread_rwlockattr_getpshared",
	"_pthread_rwlockattr_init",
	"_pthread_rwlockattr_setpshared",
	"_pthread_barrierattr_destroy",
	"_pthread_barrierattr_init",
	"_pthread_barrierattr_setpshared",
	"_pthread_barrierattr_getpshared",
	"_pthread_setschedparam",
};

static unsigned long long n_cases[F_COUNT];
static unsigned long long n_fails[F_COUNT];
static unsigned long long reported[F_COUNT];

static const unsigned SWEEP_ITERS = 200000u;
static const unsigned char GUARD = 0x7f;

static uint64_t rng_state = 0xb0278decafbad01ULL;

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

struct GuardedCpuset {
	unsigned char pre[16];
	cpuset_t set;
	unsigned char post[16];
};

struct GuardedInt {
	unsigned char pre[16];
	int val;
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

static void
init_guarded_cpuset(GuardedCpuset *g, unsigned char fill)
{
	unsigned i;

	std::memset(g->pre, GUARD, sizeof(g->pre));
	for (i = 0; i < sizeof(g->set.bits); i++)
		g->set.bits[i] = fill;
	std::memset(g->post, GUARD, sizeof(g->post));
}

static void
init_guarded_int(GuardedInt *g, int v)
{
	std::memset(g->pre, GUARD, sizeof(g->pre));
	g->val = v;
	std::memset(g->post, GUARD, sizeof(g->post));
}

static bool
guards_intact_cpuset(const GuardedCpuset *g)
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
guards_intact_int(const GuardedInt *g)
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
guards_intact_pthread(const GuardedPthread *g)
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

struct AffinityResult {
	int ret;
	unsigned unlock_delta;
	struct pthread *find_cur;
	pthread_t find_target;
	int find_include_dead;
	int aff_level;
	int aff_which;
	long aff_id;
	size_t aff_size;
};

static AffinityResult
run_setaffinity_port(struct pthread *cur, pthread_t td, int find_ret,
    int aff_ret, int aff_err, size_t size, const GuardedCpuset *in)
{
	AffinityResult r;
	unsigned unlock_before;

	b0278_reset_mocks();
	b0278_set_curthread(cur);
	b0278_set_find_thread_ret(find_ret);
	b0278_set_setaffinity(aff_ret, aff_err);
	unlock_before = b0278_get_thread_unlock_count();
	r.ret = P::_pthread_setaffinity_np(td, size, &in->set);
	r.unlock_delta = b0278_get_thread_unlock_count() - unlock_before;
	r.find_cur = b0278_get_find_last_cur();
	r.find_target = b0278_get_find_last_target();
	r.find_include_dead = b0278_get_find_last_include_dead();
	r.aff_level = b0278_get_setaffinity_last_level();
	r.aff_which = b0278_get_setaffinity_last_which();
	r.aff_id = b0278_get_setaffinity_last_id();
	r.aff_size = b0278_get_setaffinity_last_size();
	return (r);
}

static AffinityResult
run_setaffinity_ref(struct pthread *cur, pthread_t td, int find_ret,
    int aff_ret, int aff_err, size_t size, const GuardedCpuset *in)
{
	AffinityResult r;
	unsigned unlock_before;

	b0278_reset_mocks();
	b0278_set_curthread(cur);
	b0278_set_find_thread_ret(find_ret);
	b0278_set_setaffinity(aff_ret, aff_err);
	unlock_before = b0278_get_thread_unlock_count();
	r.ret = ref__pthread_setaffinity_np(td, size, &in->set);
	r.unlock_delta = b0278_get_thread_unlock_count() - unlock_before;
	r.find_cur = b0278_get_find_last_cur();
	r.find_target = b0278_get_find_last_target();
	r.find_include_dead = b0278_get_find_last_include_dead();
	r.aff_level = b0278_get_setaffinity_last_level();
	r.aff_which = b0278_get_setaffinity_last_which();
	r.aff_id = b0278_get_setaffinity_last_id();
	r.aff_size = b0278_get_setaffinity_last_size();
	return (r);
}

static void
check_setaffinity(const char *label, struct pthread *cur, pthread_t td,
    int find_ret, int aff_ret, int aff_err, size_t size, unsigned char fill)
{
	AffinityResult port, ref;
	GuardedCpuset in;
	bool ok, self, find_ok;
	long expect_id;
	unsigned expect_unlock;

	init_guarded_cpuset(&in, fill);
	self = (cur == td);
	find_ok = (self || find_ret == 0);
	expect_unlock = (self || find_ok) ? 0u : 0u;
	if (!self && find_ok)
		expect_unlock = 1u;
	expect_id = self ? -1L : (find_ok ? td->tid : 0L);

	port = run_setaffinity_port(cur, td, find_ret, aff_ret, aff_err, size,
	    &in);
	ref = run_setaffinity_ref(cur, td, find_ret, aff_ret, aff_err, size,
	    &in);

	ok = (port.ret == ref.ret) &&
	    (port.unlock_delta == ref.unlock_delta) &&
	    (port.find_cur == ref.find_cur) &&
	    (port.find_target == ref.find_target) &&
	    (port.find_include_dead == ref.find_include_dead) &&
	    guards_intact_cpuset(&in);

	if (!self && !find_ok) {
		ok = ok && (port.ret == find_ret) && (ref.ret == find_ret) &&
		    (port.unlock_delta == 0u) && (port.aff_level == 0);
	} else {
		ok = ok && (port.aff_level == CPU_LEVEL_WHICH) &&
		    (port.aff_which == CPU_WHICH_TID) &&
		    (port.aff_size == size) &&
		    (port.unlock_delta == expect_unlock);
		if (self || find_ok) {
			ok = ok && (port.aff_id == expect_id) &&
			    (ref.aff_id == expect_id);
			if (aff_ret == -1) {
				ok = ok && (port.ret == aff_err) &&
				    (ref.ret == aff_err);
			} else {
				ok = ok && (port.ret == 0) && (ref.ret == 0);
			}
		}
	}

	record_case(F_SETAFFINITY, ok,
	    "%s cur=%p td=%p find=%d aff=%d err=%d size=%zu ret_p=%d ret_r=%d "
	    "unlock_p=%u unlock_r=%u id_p=%ld id_r=%ld",
	    label, (void *)cur, (void *)td, find_ret, aff_ret, aff_err, size,
	    port.ret, ref.ret, port.unlock_delta, ref.unlock_delta,
	    port.aff_id, ref.aff_id);
}

struct GetAffinityResult {
	int ret;
	GuardedCpuset buf;
	unsigned unlock_delta;
	struct pthread *find_cur;
	pthread_t find_target;
	int find_include_dead;
	int aff_level;
	int aff_which;
	long aff_id;
	size_t aff_size;
};

static GetAffinityResult
run_getaffinity_port(struct pthread *cur, pthread_t td, int find_ret,
    int aff_ret, int aff_err, unsigned char fill, size_t size)
{
	GetAffinityResult r;
	unsigned unlock_before;

	b0278_reset_mocks();
	b0278_set_curthread(cur);
	b0278_set_find_thread_ret(find_ret);
	b0278_set_getaffinity(aff_ret, aff_err, fill);
	init_guarded_cpuset(&r.buf, 0x55);
	unlock_before = b0278_get_thread_unlock_count();
	r.ret = P::_pthread_getaffinity_np(td, size, &r.buf.set);
	r.unlock_delta = b0278_get_thread_unlock_count() - unlock_before;
	r.find_cur = b0278_get_find_last_cur();
	r.find_target = b0278_get_find_last_target();
	r.find_include_dead = b0278_get_find_last_include_dead();
	r.aff_level = b0278_get_getaffinity_last_level();
	r.aff_which = b0278_get_getaffinity_last_which();
	r.aff_id = b0278_get_getaffinity_last_id();
	r.aff_size = b0278_get_getaffinity_last_size();
	return (r);
}

static GetAffinityResult
run_getaffinity_ref(struct pthread *cur, pthread_t td, int find_ret,
    int aff_ret, int aff_err, unsigned char fill, size_t size)
{
	GetAffinityResult r;
	unsigned unlock_before;

	b0278_reset_mocks();
	b0278_set_curthread(cur);
	b0278_set_find_thread_ret(find_ret);
	b0278_set_getaffinity(aff_ret, aff_err, fill);
	init_guarded_cpuset(&r.buf, 0x55);
	unlock_before = b0278_get_thread_unlock_count();
	r.ret = ref__pthread_getaffinity_np(td, size, &r.buf.set);
	r.unlock_delta = b0278_get_thread_unlock_count() - unlock_before;
	r.find_cur = b0278_get_find_last_cur();
	r.find_target = b0278_get_find_last_target();
	r.find_include_dead = b0278_get_find_last_include_dead();
	r.aff_level = b0278_get_getaffinity_last_level();
	r.aff_which = b0278_get_getaffinity_last_which();
	r.aff_id = b0278_get_getaffinity_last_id();
	r.aff_size = b0278_get_getaffinity_last_size();
	return (r);
}

static void
check_getaffinity(const char *label, struct pthread *cur, pthread_t td,
    int find_ret, int aff_ret, int aff_err, unsigned char fill, size_t size)
{
	GetAffinityResult port, ref;
	bool ok, self, find_ok;
	long expect_id;
	unsigned expect_unlock;

	self = (cur == td);
	find_ok = (self || find_ret == 0);
	expect_unlock = (!self && find_ok) ? 1u : 0u;
	expect_id = self ? -1L : (find_ok ? td->tid : 0L);

	port = run_getaffinity_port(cur, td, find_ret, aff_ret, aff_err, fill,
	    size);
	ref = run_getaffinity_ref(cur, td, find_ret, aff_ret, aff_err, fill,
	    size);

	ok = (port.ret == ref.ret) &&
	    (std::memcmp(&port.buf, &ref.buf, sizeof(port.buf)) == 0) &&
	    (port.unlock_delta == ref.unlock_delta) &&
	    (port.find_cur == ref.find_cur) &&
	    (port.find_target == ref.find_target) &&
	    (port.find_include_dead == ref.find_include_dead) &&
	    guards_intact_cpuset(&port.buf) && guards_intact_cpuset(&ref.buf);

	if (!self && !find_ok) {
		ok = ok && (port.ret == find_ret) && (ref.ret == find_ret) &&
		    (port.unlock_delta == 0u) && (port.aff_level == 0);
	} else {
		ok = ok && (port.aff_level == CPU_LEVEL_WHICH) &&
		    (port.aff_which == CPU_WHICH_TID) &&
		    (port.aff_size == size) &&
		    (port.unlock_delta == expect_unlock) &&
		    (port.aff_id == expect_id) && (ref.aff_id == expect_id);
		if (aff_ret == -1) {
			ok = ok && (port.ret == aff_err) && (ref.ret == aff_err);
		} else if (size > 0) {
			ok = ok && (port.ret == 0) && (ref.ret == 0);
		}
	}

	record_case(F_GETAFFINITY, ok,
	    "%s cur=%p td=%p find=%d aff=%d err=%d fill=%u size=%zu ret_p=%d "
	    "ret_r=%d unlock_p=%u unlock_r=%u",
	    label, (void *)cur, (void *)td, find_ret, aff_ret, aff_err, fill,
	    size, port.ret, ref.ret, port.unlock_delta, ref.unlock_delta);
}

struct RwlockAttrResult {
	int ret;
	pthread_rwlockattr_t attr;
};

static void
check_rwlockattr_destroy_pair(const char *label, pthread_rwlockattr_t *port_attr,
    pthread_rwlockattr_t *ref_attr, bool init_first)
{
	int port_ret, ref_ret;

	b0278_reset_mocks();
	if (init_first) {
		P::_pthread_rwlockattr_init(port_attr);
		ref__pthread_rwlockattr_init(ref_attr);
	}
	port_ret = P::_pthread_rwlockattr_destroy(port_attr);
	ref_ret = ref__pthread_rwlockattr_destroy(ref_attr);
	record_case(F_RWLOCKATTR_DESTROY,
	    port_ret == ref_ret,
	    "%s port_ret=%d ref_ret=%d", label, port_ret, ref_ret);
}

static void
check_rwlockattr_init_pair(const char *label, pthread_rwlockattr_t *port_slot,
    pthread_rwlockattr_t *ref_slot)
{
	int port_ret, ref_ret;
	GuardedInt port_pshared, ref_pshared;
	int get_port, get_ref;

	b0278_reset_mocks();
	*port_slot = NULL;
	*ref_slot = NULL;
	port_ret = P::_pthread_rwlockattr_init(port_slot);
	ref_ret = ref__pthread_rwlockattr_init(ref_slot);

	init_guarded_int(&port_pshared, -1);
	init_guarded_int(&ref_pshared, -1);
	if (port_ret == 0)
		get_port = P::_pthread_rwlockattr_getpshared(port_slot,
		    &port_pshared.val);
	else
		get_port = -999;
	if (ref_ret == 0)
		get_ref = ref__pthread_rwlockattr_getpshared(ref_slot,
		    &ref_pshared.val);
	else
		get_ref = -999;

	record_case(F_RWLOCKATTR_INIT,
	    port_ret == ref_ret &&
	    (port_ret != 0 || (port_pshared.val == ref_pshared.val &&
	    port_pshared.val == PTHREAD_PROCESS_PRIVATE &&
	    get_port == 0 && get_ref == 0 &&
	    guards_intact_int(&port_pshared) &&
	    guards_intact_int(&ref_pshared))),
	    "%s port_ret=%d ref_ret=%d pshared_p=%d pshared_r=%d", label,
	    port_ret, ref_ret, port_pshared.val, ref_pshared.val);

	if (port_ret == 0)
		P::_pthread_rwlockattr_destroy(port_slot);
	if (ref_ret == 0)
		ref__pthread_rwlockattr_destroy(ref_slot);
}

static void
check_rwlockattr_getpshared_pair(const char *label, int set_val)
{
	pthread_rwlockattr_t port_attr = NULL, ref_attr = NULL;
	GuardedInt port_out, ref_out;
	int port_ret, ref_ret, set_port, set_ref;

	b0278_reset_mocks();
	P::_pthread_rwlockattr_init(&port_attr);
	ref__pthread_rwlockattr_init(&ref_attr);
	set_port = P::_pthread_rwlockattr_setpshared(&port_attr, set_val);
	set_ref = ref__pthread_rwlockattr_setpshared(&ref_attr, set_val);
	if (set_port != 0 || set_ref != 0) {
		P::_pthread_rwlockattr_destroy(&port_attr);
		ref__pthread_rwlockattr_destroy(&ref_attr);
		return;
	}
	init_guarded_int(&port_out, 0x7e7e7e7e);
	init_guarded_int(&ref_out, 0x7e7e7e7e);
	port_ret = P::_pthread_rwlockattr_getpshared(&port_attr, &port_out.val);
	ref_ret = ref__pthread_rwlockattr_getpshared(&ref_attr, &ref_out.val);
	record_case(F_RWLOCKATTR_GETPSHARED,
	    port_ret == ref_ret && port_out.val == ref_out.val &&
	    port_out.val == set_val && guards_intact_int(&port_out) &&
	    guards_intact_int(&ref_out),
	    "%s set=%d port_ret=%d ref_ret=%d out_p=%d out_r=%d", label,
	    set_val, port_ret, ref_ret, port_out.val, ref_out.val);
	P::_pthread_rwlockattr_destroy(&port_attr);
	ref__pthread_rwlockattr_destroy(&ref_attr);
}

static void
check_rwlockattr_setpshared_pair(const char *label, int pshared)
{
	pthread_rwlockattr_t port_attr = NULL, ref_attr = NULL;
	int port_ret, ref_ret, port_get, ref_get;

	b0278_reset_mocks();
	P::_pthread_rwlockattr_init(&port_attr);
	ref__pthread_rwlockattr_init(&ref_attr);
	port_ret = P::_pthread_rwlockattr_setpshared(&port_attr, pshared);
	ref_ret = ref__pthread_rwlockattr_setpshared(&ref_attr, pshared);
	port_get = P::_pthread_rwlockattr_getpshared(&port_attr, &port_get);
	ref_get = ref__pthread_rwlockattr_getpshared(&ref_attr, &ref_get);
	(void)port_get;
	(void)ref_get;
	record_case(F_RWLOCKATTR_SETPSHARED,
	    port_ret == ref_ret &&
	    (port_ret != 0 ||
	    (port_attr->pshared == ref_attr->pshared &&
	    port_attr->pshared == pshared)),
	    "%s pshared=%d port_ret=%d ref_ret=%d stored_p=%d stored_r=%d",
	    label, pshared, port_ret, ref_ret,
	    port_ret == 0 ? port_attr->pshared : -1,
	    ref_ret == 0 ? ref_attr->pshared : -1);
	if (port_ret == 0 || port_attr != NULL)
		P::_pthread_rwlockattr_destroy(&port_attr);
	if (ref_ret == 0 || ref_attr != NULL)
		ref__pthread_rwlockattr_destroy(&ref_attr);
}

static void
check_barrierattr_destroy_pair(const char *label, pthread_barrierattr_t *port_attr,
    pthread_barrierattr_t *ref_attr, bool init_first)
{
	int port_ret, ref_ret;

	b0278_reset_mocks();
	if (init_first) {
		P::_pthread_barrierattr_init(port_attr);
		ref__pthread_barrierattr_init(ref_attr);
	}
	port_ret = P::_pthread_barrierattr_destroy(port_attr);
	ref_ret = ref__pthread_barrierattr_destroy(ref_attr);
	record_case(F_BARRIERATTR_DESTROY,
	    port_ret == ref_ret,
	    "%s port_ret=%d ref_ret=%d", label, port_ret, ref_ret);
}

static void
check_barrierattr_init_pair(const char *label, pthread_barrierattr_t *port_slot,
    pthread_barrierattr_t *ref_slot)
{
	int port_ret, ref_ret;
	GuardedInt port_pshared, ref_pshared;
	int get_port, get_ref;

	b0278_reset_mocks();
	*port_slot = NULL;
	*ref_slot = NULL;
	port_ret = P::_pthread_barrierattr_init(port_slot);
	ref_ret = ref__pthread_barrierattr_init(ref_slot);
	init_guarded_int(&port_pshared, -1);
	init_guarded_int(&ref_pshared, -1);
	if (port_ret == 0)
		get_port = P::_pthread_barrierattr_getpshared(port_slot,
		    &port_pshared.val);
	else
		get_port = -999;
	if (ref_ret == 0)
		get_ref = ref__pthread_barrierattr_getpshared(ref_slot,
		    &ref_pshared.val);
	else
		get_ref = -999;
	record_case(F_BARRIERATTR_INIT,
	    port_ret == ref_ret &&
	    (port_ret != 0 || (port_pshared.val == ref_pshared.val &&
	    port_pshared.val == PTHREAD_PROCESS_PRIVATE &&
	    get_port == 0 && get_ref == 0 &&
	    guards_intact_int(&port_pshared) &&
	    guards_intact_int(&ref_pshared))),
	    "%s port_ret=%d ref_ret=%d pshared_p=%d pshared_r=%d", label,
	    port_ret, ref_ret, port_pshared.val, ref_pshared.val);
	if (port_ret == 0)
		P::_pthread_barrierattr_destroy(port_slot);
	if (ref_ret == 0)
		ref__pthread_barrierattr_destroy(ref_slot);
}

static void
check_barrierattr_getpshared_pair(const char *label, int set_val)
{
	pthread_barrierattr_t port_attr = NULL, ref_attr = NULL;
	GuardedInt port_out, ref_out;
	int port_ret, ref_ret;

	b0278_reset_mocks();
	P::_pthread_barrierattr_init(&port_attr);
	ref__pthread_barrierattr_init(&ref_attr);
	P::_pthread_barrierattr_setpshared(&port_attr, set_val);
	ref__pthread_barrierattr_setpshared(&ref_attr, set_val);
	init_guarded_int(&port_out, 0x80808080);
	init_guarded_int(&ref_out, 0x80808080);
	port_ret = P::_pthread_barrierattr_getpshared(&port_attr, &port_out.val);
	ref_ret = ref__pthread_barrierattr_getpshared(&ref_attr, &ref_out.val);
	record_case(F_BARRIERATTR_GETPSHARED,
	    port_ret == ref_ret && port_out.val == ref_out.val &&
	    port_out.val == set_val && guards_intact_int(&port_out) &&
	    guards_intact_int(&ref_out),
	    "%s set=%d port_ret=%d ref_ret=%d out_p=%d out_r=%d", label,
	    set_val, port_ret, ref_ret, port_out.val, ref_out.val);
	P::_pthread_barrierattr_destroy(&port_attr);
	ref__pthread_barrierattr_destroy(&ref_attr);
}

static void
check_barrierattr_setpshared_pair(const char *label, int pshared)
{
	pthread_barrierattr_t port_attr = NULL, ref_attr = NULL;
	int port_ret, ref_ret;

	b0278_reset_mocks();
	P::_pthread_barrierattr_init(&port_attr);
	ref__pthread_barrierattr_init(&ref_attr);
	port_ret = P::_pthread_barrierattr_setpshared(&port_attr, pshared);
	ref_ret = ref__pthread_barrierattr_setpshared(&ref_attr, pshared);
	record_case(F_BARRIERATTR_SETPSHARED,
	    port_ret == ref_ret &&
	    (port_ret != 0 ||
	    (port_attr->pshared == ref_attr->pshared &&
	    port_attr->pshared == pshared)),
	    "%s pshared=%d port_ret=%d ref_ret=%d stored_p=%d stored_r=%d",
	    label, pshared, port_ret, ref_ret,
	    port_ret == 0 ? port_attr->pshared : -1,
	    ref_ret == 0 ? ref_attr->pshared : -1);
	if (port_attr != NULL)
		P::_pthread_barrierattr_destroy(&port_attr);
	if (ref_attr != NULL)
		ref__pthread_barrierattr_destroy(&ref_attr);
}

struct SetschedResult {
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

static SetschedResult
run_setschedparam_port(struct pthread *cur, GuardedPthread *in, int find_ret,
    int sched_ret, int sched_err, int policy, int prio)
{
	SetschedResult r;
	struct sched_param param;
	unsigned lock_before, unlock_before;

	b0278_reset_mocks();
	b0278_set_curthread(cur);
	b0278_set_find_thread_ret(find_ret);
	b0278_set_setscheduler(sched_ret, sched_err);
	param.sched_priority = prio;
	lock_before = b0278_get_thr_lock_count();
	unlock_before = b0278_get_thread_unlock_count();
	r.ret = P::_pthread_setschedparam(&in->thr, policy, &param);
	r.target = *in;
	r.lock_delta = b0278_get_thr_lock_count() - lock_before;
	r.unlock_delta = b0278_get_thread_unlock_count() - unlock_before;
	r.lock_last = b0278_get_thr_lock_last_thread();
	r.unlock_cur = b0278_get_thread_unlock_last_cur();
	r.unlock_target = b0278_get_thread_unlock_last_target();
	r.sched_tid = b0278_get_setscheduler_last_tid();
	r.sched_policy = b0278_get_setscheduler_last_policy();
	r.sched_prio = b0278_get_setscheduler_last_prio();
	return (r);
}

static SetschedResult
run_setschedparam_ref(struct pthread *cur, GuardedPthread *in, int find_ret,
    int sched_ret, int sched_err, int policy, int prio)
{
	SetschedResult r;
	struct sched_param param;
	unsigned lock_before, unlock_before;

	b0278_reset_mocks();
	b0278_set_curthread(cur);
	b0278_set_find_thread_ret(find_ret);
	b0278_set_setscheduler(sched_ret, sched_err);
	param.sched_priority = prio;
	lock_before = b0278_get_thr_lock_count();
	unlock_before = b0278_get_thread_unlock_count();
	r.ret = ref__pthread_setschedparam(&in->thr, policy, &param);
	r.target = *in;
	r.lock_delta = b0278_get_thr_lock_count() - lock_before;
	r.unlock_delta = b0278_get_thread_unlock_count() - unlock_before;
	r.lock_last = b0278_get_thr_lock_last_thread();
	r.unlock_cur = b0278_get_thread_unlock_last_cur();
	r.unlock_target = b0278_get_thread_unlock_last_target();
	r.sched_tid = b0278_get_setscheduler_last_tid();
	r.sched_policy = b0278_get_setscheduler_last_policy();
	r.sched_prio = b0278_get_setscheduler_last_prio();
	return (r);
}

static void
check_setschedparam(const char *label, struct pthread *cur, GuardedPthread *in,
    int find_ret, int sched_ret, int sched_err, int policy, int prio)
{
	SetschedResult port, ref;
	bool ok, self, find_ok, fast_path, expect_sched;
	unsigned expect_lock, expect_unlock;
	GuardedPthread in_port, in_ref;
	int initial_prio, initial_policy;
	long initial_tid;

	self = (cur == &in->thr);
	find_ok = (self || find_ret == 0);
	initial_policy = in->thr.attr.sched_policy;
	initial_prio = in->thr.attr.prio;
	initial_tid = in->thr.tid;
	fast_path = find_ok && (initial_policy == policy) &&
	    (policy == SCHED_OTHER || initial_prio == prio);
	expect_sched = find_ok && !fast_path;
	expect_lock = self ? 1u : 0u;
	expect_unlock = find_ok ? 1u : 0u;

	in_port = *in;
	in_ref = *in;
	port = run_setschedparam_port(self ? &in_port.thr : cur, &in_port, find_ret,
	    sched_ret, sched_err, policy, prio);
	ref = run_setschedparam_ref(self ? &in_ref.thr : cur, &in_ref, find_ret,
	    sched_ret, sched_err, policy, prio);

	ok = (port.ret == ref.ret) &&
	    (std::memcmp(&port.target, &ref.target, sizeof(port.target)) == 0) &&
	    guards_intact_pthread(&port.target) &&
	    guards_intact_pthread(&ref.target) &&
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
		    (port.target.thr.attr.prio == initial_prio) &&
		    (port.target.thr.attr.sched_policy == initial_policy);
	} else {
		ok = ok && (port.lock_delta == expect_lock) &&
		    (port.unlock_delta == expect_unlock) &&
		    (port.unlock_cur == cur) &&
		    (port.unlock_target == &in_port.thr);

		if (fast_path) {
			ok = ok && (port.ret == 0) && (ref.ret == 0) &&
			    (port.target.thr.attr.prio == prio) &&
			    (port.sched_tid == 0);
		} else if (expect_sched) {
			if (sched_ret == -1) {
				ok = ok && (port.ret == sched_err) &&
				    (ref.ret == sched_err) &&
				    (port.target.thr.attr.prio == initial_prio) &&
				    (port.target.thr.attr.sched_policy ==
				    initial_policy);
			} else {
				ok = ok && (port.ret == 0) && (ref.ret == 0) &&
				    (port.target.thr.attr.prio == prio) &&
				    (port.target.thr.attr.sched_policy == policy) &&
				    (port.sched_tid == initial_tid) &&
				    (port.sched_policy == policy) &&
				    (port.sched_prio == prio);
			}
		}
	}

	record_case(F_SETSCHEDPARAM, ok,
	    "%s cur=%p tgt=%p find=%d pol=%d prio=%d ret_p=%d ret_r=%d "
	    "lock_p=%u unlock_p=%u attr_pol_p=%d attr_prio_p=%d",
	    label, (void *)cur, (void *)&in_port.thr, find_ret, policy, prio,
	    port.ret, ref.ret, port.lock_delta, port.unlock_delta,
	    port.target.thr.attr.sched_policy, port.target.thr.attr.prio);
}

static void
test_edges(void)
{
	pthread_rwlockattr_t port_rw = NULL, ref_rw = NULL;
	pthread_barrierattr_t port_ba = NULL, ref_ba = NULL;
	GuardedPthread gcur, gtgt;

	init_guarded_pthread(&gcur, SCHED_OTHER, 0, 1);
	init_guarded_pthread(&gtgt, 1, 5, 42);

	check_setaffinity("setaff_self_ok", &gcur.thr, &gcur.thr, 0, 0, 0,
	    sizeof(cpuset_t), 0);
	check_setaffinity("setaff_self_fail", &gcur.thr, &gcur.thr, 0, -1,
	    EINVAL, sizeof(cpuset_t), 0xff);
	check_setaffinity("setaff_other_ok", &gcur.thr, &gtgt.thr, 0, 0, 0, 16, 0x80);
	check_setaffinity("setaff_other_fail", &gcur.thr, &gtgt.thr, 0, -1, 123, 0,
	    0x7f);
	check_setaffinity("setaff_find_fail", &gcur.thr, &gtgt.thr, EINVAL, 0, 0,
	    sizeof(cpuset_t), 0);
	check_setaffinity("setaff_size_zero", &gcur.thr, &gcur.thr, 0, 0, 0, 0, 0);
	check_setaffinity("setaff_size_one", &gcur.thr, &gtgt.thr, 0, 0, 0, 1, 0xaa);

	check_getaffinity("getaff_self_ok", &gcur.thr, &gcur.thr, 0, 0, 0, 0x11,
	    sizeof(cpuset_t));
	check_getaffinity("getaff_self_fail", &gcur.thr, &gcur.thr, 0, -1, ENOMEM,
	    0xff, sizeof(cpuset_t));
	check_getaffinity("getaff_other_ok", &gcur.thr, &gtgt.thr, 0, 0, 0, 0x80,
	    32);
	check_getaffinity("getaff_other_fail", &gcur.thr, &gtgt.thr, 0, -1, 5, 0,
	    64);
	check_getaffinity("getaff_find_fail", &gcur.thr, &gtgt.thr, 22, 0, 0, 0x7f,
	    8);
	check_getaffinity("getaff_size_zero", &gcur.thr, &gcur.thr, 0, 0, 0, 0x55,
	    0);

	check_rwlockattr_destroy_pair("rw_destroy_null", NULL, NULL, false);
	port_rw = NULL;
	ref_rw = NULL;
	check_rwlockattr_destroy_pair("rw_destroy_uninit", &port_rw, &ref_rw,
	    false);
	check_rwlockattr_destroy_pair("rw_destroy_ok", &port_rw, &ref_rw, true);

	check_rwlockattr_init_pair("rw_init_ok", &port_rw, &ref_rw);
	check_rwlockattr_init_pair("rw_init_ok2", &port_rw, &ref_rw);

	check_rwlockattr_getpshared_pair("rw_get_private",
	    PTHREAD_PROCESS_PRIVATE);
	check_rwlockattr_getpshared_pair("rw_get_shared",
	    PTHREAD_PROCESS_SHARED);

	check_rwlockattr_setpshared_pair("rw_set_private",
	    PTHREAD_PROCESS_PRIVATE);
	check_rwlockattr_setpshared_pair("rw_set_shared", PTHREAD_PROCESS_SHARED);
	check_rwlockattr_setpshared_pair("rw_set_invalid", 2);
	check_rwlockattr_setpshared_pair("rw_set_neg", -1);
	check_rwlockattr_setpshared_pair("rw_set_high", 0x80808080);

	check_barrierattr_destroy_pair("ba_destroy_null", NULL, NULL, false);
	port_ba = NULL;
	ref_ba = NULL;
	check_barrierattr_destroy_pair("ba_destroy_uninit", &port_ba, &ref_ba,
	    false);
	check_barrierattr_destroy_pair("ba_destroy_ok", &port_ba, &ref_ba, true);

	check_barrierattr_init_pair("ba_init_ok", &port_ba, &ref_ba);

	check_barrierattr_getpshared_pair("ba_get_private",
	    PTHREAD_PROCESS_PRIVATE);
	check_barrierattr_getpshared_pair("ba_get_shared",
	    PTHREAD_PROCESS_SHARED);

	check_barrierattr_setpshared_pair("ba_set_private",
	    PTHREAD_PROCESS_PRIVATE);
	check_barrierattr_setpshared_pair("ba_set_shared", PTHREAD_PROCESS_SHARED);
	check_barrierattr_setpshared_pair("ba_set_invalid", 99);
	check_barrierattr_setpshared_pair("ba_set_neg", -2);

	check_setschedparam("sched_self_other_fast", &gcur.thr, &gcur, 0, 0, 0,
	    SCHED_OTHER, 7);
	init_guarded_pthread(&gtgt, SCHED_OTHER, 3, 9);
	check_setschedparam("sched_other_same_prio", &gcur.thr, &gtgt, 0, 0, 0,
	    SCHED_OTHER, 3);
	init_guarded_pthread(&gtgt, 2, 10, 11);
	check_setschedparam("sched_same_policy_prio", &gcur.thr, &gtgt, 0, 0, 0,
	    2, 10);
	init_guarded_pthread(&gtgt, 3, 4, 12);
	check_setschedparam("sched_sched_ok", &gcur.thr, &gtgt, 0, 0, 0, 5, 20);
	init_guarded_pthread(&gtgt, 4, 4, 13);
	check_setschedparam("sched_sched_fail", &gcur.thr, &gtgt, 0, -1, EINVAL, 6,
	    30);
	init_guarded_pthread(&gtgt, 5, 1, 14);
	check_setschedparam("sched_find_fail", &gcur.thr, &gtgt, ENOMEM, 0, 0, 5,
	    2);
	init_guarded_pthread(&gtgt, 6, (int)0x80808080, 15);
	check_setschedparam("sched_high_prio", &gcur.thr, &gtgt, 0, 0, 0, 7,
	    0x7f7f7f7f);
}

static void
test_random(unsigned iters)
{
	unsigned i;

	for (i = 0; i < iters; i++) {
		unsigned op = rnd_below(11u);
		char label[64];

		std::snprintf(label, sizeof(label), "rand%u", i);

		switch (op) {
		case 0: {
			struct pthread *cur;
			pthread_t td;
			int find_ret, aff_ret, aff_err;
			size_t size;
			unsigned char fill;

			cur = (rnd_below(2u) == 0u) ? &g_cur : &g_target1;
			td = (rnd_below(3u) == 0u) ? cur :
			    ((rnd_below(2u) == 0u) ? &g_target0 : &g_target1);
			find_ret = (int)(nextr() & 0xffu);
			if ((i % 7u) == 0u)
				find_ret = 0;
			aff_ret = ((i % 5u) == 0u) ? -1 : 0;
			aff_err = (int)(nextr() & 0x7ffu);
			size = (size_t)(nextr() & 0xffu);
			if ((i % 11u) == 0u)
				size = sizeof(cpuset_t);
			fill = (unsigned char)(nextr() & 0xffu);
			check_setaffinity(label, cur, td, find_ret, aff_ret,
			    aff_err, size, fill);
			break;
		}
		case 1: {
			struct pthread *cur;
			pthread_t td;
			int find_ret, aff_ret, aff_err;
			size_t size;
			unsigned char fill;

			cur = (rnd_below(2u) == 0u) ? &g_cur : &g_target0;
			td = (rnd_below(4u) == 0u) ? cur : &g_target1;
			find_ret = (int)(nextr() & 0xffu);
			if ((i % 9u) == 0u)
				find_ret = 0;
			aff_ret = ((i % 6u) == 0u) ? -1 : 0;
			aff_err = (int)(nextr() & 0x3ffu);
			size = (size_t)(nextr() & 0x7fu);
			fill = (unsigned char)(nextr() & 0xffu);
			check_getaffinity(label, cur, td, find_ret, aff_ret,
			    aff_err, fill, size);
			break;
		}
		case 2: {
			pthread_rwlockattr_t port_rw = NULL, ref_rw = NULL;
			check_rwlockattr_init_pair(label, &port_rw, &ref_rw);
			break;
		}
		case 3: {
			pthread_rwlockattr_t port_rw = NULL, ref_rw = NULL;
			int pshared;

			check_rwlockattr_init_pair(label, &port_rw, &ref_rw);
			pshared = (int)(nextr() & 0xffu);
			if ((i % 13u) == 0u)
				pshared = PTHREAD_PROCESS_PRIVATE;
			else if ((i % 17u) == 0u)
				pshared = PTHREAD_PROCESS_SHARED;
			check_rwlockattr_setpshared_pair(label, pshared);
			break;
		}
		case 4: {
			pthread_rwlockattr_t port_rw = NULL, ref_rw = NULL;
			int val;

			P::_pthread_rwlockattr_init(&port_rw);
			ref__pthread_rwlockattr_init(&ref_rw);
			val = ((i % 2u) == 0u) ? PTHREAD_PROCESS_PRIVATE :
			    PTHREAD_PROCESS_SHARED;
			P::_pthread_rwlockattr_setpshared(&port_rw, val);
			ref__pthread_rwlockattr_setpshared(&ref_rw, val);
			check_rwlockattr_getpshared_pair(label, val);
			break;
		}
		case 5: {
			pthread_rwlockattr_t port_rw = NULL, ref_rw = NULL;
			check_rwlockattr_destroy_pair(label, &port_rw, &ref_rw,
			    (rnd_below(2u) == 0u));
			break;
		}
		case 6: {
			pthread_barrierattr_t port_ba = NULL, ref_ba = NULL;
			check_barrierattr_init_pair(label, &port_ba, &ref_ba);
			break;
		}
		case 7: {
			int pshared;

			pshared = (int)(nextr() & 0xffffu);
			check_barrierattr_setpshared_pair(label, pshared);
			break;
		}
		case 8: {
			int val;

			val = ((i % 3u) == 0u) ? PTHREAD_PROCESS_SHARED :
			    PTHREAD_PROCESS_PRIVATE;
			check_barrierattr_getpshared_pair(label, val);
			break;
		}
		case 9: {
			pthread_barrierattr_t port_ba = NULL, ref_ba = NULL;
			check_barrierattr_destroy_pair(label, &port_ba, &ref_ba,
			    (rnd_below(2u) != 0u));
			break;
		}
		default: {
			GuardedPthread gtarget;
			struct pthread *cur;
			int find_ret, sched_ret, sched_err, policy, prio, old_prio;
			long tid;

			policy = (int)(nextr() & 0xffu);
			if ((i % 5u) == 0u)
				policy = SCHED_OTHER;
			old_prio = (int)(nextr() & 0xffffffffu);
			tid = (long)(nextr() & 0xffffffffu);
			init_guarded_pthread(&gtarget, policy, old_prio, tid);
			cur = (rnd_below(2u) == 0u) ? &g_cur : &g_target1;
			if ((i % 19u) == 0u)
				cur = &gtarget.thr;
			find_ret = (int)(nextr() & 0xffu);
			if ((i % 7u) == 0u)
				find_ret = 0;
			sched_ret = ((i % 4u) == 0u) ? -1 : 0;
			sched_err = (int)(nextr() & 0x7ffu);
			prio = (int)(nextr() & 0xffffffffu);
			check_setschedparam(label, cur, &gtarget, find_ret,
			    sched_ret, sched_err, policy, prio);
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

	std::printf("\nbatch b0278 differential results\n");
	std::printf("%-32s %12s %10s %s\n", "function", "cases", "failures",
	    "status");
	for (fn = 0; fn < F_COUNT; fn++) {
		std::printf("%-32s %12llu %10llu %s\n", fn_name[fn],
		    n_cases[fn], n_fails[fn],
		    n_fails[fn] == 0u ? "ok" : "FAIL");
		total_cases += n_cases[fn];
		total_fails += n_fails[fn];
	}
	std::printf("%-32s %12llu %10llu %s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0u ? "ok" : "FAIL");

	return total_fails == 0u ? 0 : 1;
}
