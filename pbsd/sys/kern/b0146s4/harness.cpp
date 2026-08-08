// Differential test for PBSD batch b0146s4 (kern_sema.c).

import pbsd.sys.kern.b0146s4;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>

namespace port = pbsd::sys_kern::b0146s4;

#define GUARD     0x7f
#define PAD       32u
#define SWEEP     200000L
#define MAX_PRINT 12
#define EWOULDBLOCK 35

struct stat_row {
	const char *name;
	long cases;
	long failures;
	long printed;
};

static stat_row rows[] = {
	{ "sema_init",       0, 0, 0 },
	{ "sema_destroy",    0, 0, 0 },
	{ "_sema_post",      0, 0, 0 },
	{ "_sema_wait",      0, 0, 0 },
	{ "_sema_timedwait", 0, 0, 0 },
	{ "_sema_trywait",   0, 0, 0 },
	{ "sema_value",      0, 0, 0 },
};

enum {
	R_SEMA_INIT = 0,
	R_SEMA_DESTROY,
	R_SEMA_POST,
	R_SEMA_WAIT,
	R_SEMA_TIMEDWAIT,
	R_SEMA_TRYWAIT,
	R_SEMA_VALUE,
};

static uint64_t rng_state = 0x00b0146004faceULL;

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
		std::printf("  FAIL %-24s %-24s %s\n", r.name, label, detail);
	}
}

static void
case_row(int row)
{
	rows[row].cases++;
}

extern "C" {
struct mtx {
	unsigned long long mtx_name_hash;
	unsigned long long mtx_type_hash;
	int mtx_flags;
	int mtx_locked;
};
struct cv {
	unsigned long long cv_desc_hash;
	int cv_waits;
	int cv_signals;
};
struct sema {
	struct mtx sema_mtx;
	struct cv sema_cv;
	int sema_value;
	int sema_waiters;
};

void ref_sema_init(struct sema *, int, const char *);
void ref_sema_destroy(struct sema *);
void ref__sema_post(struct sema *, const char *, int);
void ref__sema_wait(struct sema *, const char *, int);
int ref__sema_timedwait(struct sema *, int, const char *, int);
int ref__sema_trywait(struct sema *, const char *, int);
int ref_sema_value(struct sema *);

void oracle_reset(void);
void oracle_set_sema_target(void *);
void oracle_configure_cv(int, int, int, int);
}

struct sema_slot {
	unsigned char guard_before[PAD];
	port::sema port_s;
	unsigned char guard_mid[PAD];
	struct sema ref_s;
	unsigned char guard_after[PAD];
};

static void
env_reset(port::sema *ps, struct sema *rs)
{
	port::model_reset();
	oracle_reset();
	port::model_set_sema_target(ps);
	oracle_set_sema_target(rs);
}

static void
env_configure(int release_after, int post_amount, int tw_post_after,
    int tw_fail_after)
{
	port::model_configure_cv(release_after, post_amount, tw_post_after,
	    tw_fail_after);
	oracle_configure_cv(release_after, post_amount, tw_post_after,
	    tw_fail_after);
}

static bool
slot_intact(const sema_slot &sl)
{
	for (std::size_t i = 0; i < PAD; i++) {
		if (sl.guard_before[i] != GUARD || sl.guard_mid[i] != GUARD ||
		    sl.guard_after[i] != GUARD)
			return (false);
	}
	return (true);
}

static bool
sema_state_match(const port::sema &ps, const struct sema &rs)
{
	if (ps.sema_value != rs.sema_value)
		return (false);
	if (ps.sema_waiters != rs.sema_waiters)
		return (false);
	if (ps.sema_mtx.mtx_name_hash != rs.sema_mtx.mtx_name_hash)
		return (false);
	if (ps.sema_mtx.mtx_type_hash != rs.sema_mtx.mtx_type_hash)
		return (false);
	if (ps.sema_mtx.mtx_flags != rs.sema_mtx.mtx_flags)
		return (false);
	if (ps.sema_mtx.mtx_locked != rs.sema_mtx.mtx_locked)
		return (false);
	if (ps.sema_cv.cv_desc_hash != rs.sema_cv.cv_desc_hash)
		return (false);
	if (ps.sema_cv.cv_waits != rs.sema_cv.cv_waits)
		return (false);
	if (ps.sema_cv.cv_signals != rs.sema_cv.cv_signals)
		return (false);
	return (true);
}

static void
slot_init(sema_slot &sl)
{
	std::memset(&sl, GUARD, sizeof(sl));
	std::memset(&sl.port_s, 0, sizeof(sl.port_s));
	std::memset(&sl.ref_s, 0, sizeof(sl.ref_s));
}

static void
check_slot(int row, const char *label, const sema_slot &sl)
{
	if (!slot_intact(sl))
		fail_row(row, label, "guard bytes corrupted");
	if (!sema_state_match(sl.port_s, sl.ref_s))
		fail_row(row, label, "sema state mismatch");
}

static void
test_init_one(int value)
{
	case_row(R_SEMA_INIT);
	sema_slot sl;

	slot_init(sl);
	env_reset(&sl.port_s, &sl.ref_s);
	env_configure(1, 1, 0, 1);

	port::sema_init(&sl.port_s, value, "port");
	ref_sema_init(&sl.ref_s, value, "ref");

	check_slot(R_SEMA_INIT, "init", sl);

	case_row(R_SEMA_VALUE);
	int pv = port::sema_value(&sl.port_s);
	int rv = ref_sema_value(&sl.ref_s);
	if (pv != rv || pv != value)
		fail_row(R_SEMA_VALUE, "after-init", "value mismatch");
	check_slot(R_SEMA_VALUE, "after-init", sl);

	case_row(R_SEMA_DESTROY);
	port::sema_destroy(&sl.port_s);
	ref_sema_destroy(&sl.ref_s);
	check_slot(R_SEMA_DESTROY, "destroy", sl);
}

static void
test_post_one(int initial, int posts)
{
	case_row(R_SEMA_POST);
	sema_slot sl;

	slot_init(sl);
	env_reset(&sl.port_s, &sl.ref_s);
	env_configure(1, 1, 0, 1);

	port::sema_init(&sl.port_s, initial, "port");
	ref_sema_init(&sl.ref_s, initial, "ref");

	for (int i = 0; i < posts; i++) {
		port::_sema_post(&sl.port_s, __FILE__, __LINE__);
		ref__sema_post(&sl.ref_s, __FILE__, __LINE__);
		check_slot(R_SEMA_POST, "post", sl);
	}

	int pv = port::sema_value(&sl.port_s);
	int rv = ref_sema_value(&sl.ref_s);
	if (pv != rv || pv != initial + posts)
		fail_row(R_SEMA_POST, "post-value", "value mismatch");

	port::sema_destroy(&sl.port_s);
	ref_sema_destroy(&sl.ref_s);
}

static void
test_trywait_one(int initial, int expect_ret)
{
	case_row(R_SEMA_TRYWAIT);
	sema_slot sl;

	slot_init(sl);
	env_reset(&sl.port_s, &sl.ref_s);
	env_configure(1, 1, 0, 1);

	port::sema_init(&sl.port_s, initial, "port");
	ref_sema_init(&sl.ref_s, initial, "ref");

	int pt = port::_sema_trywait(&sl.port_s, __FILE__, __LINE__);
	int rt = ref__sema_trywait(&sl.ref_s, __FILE__, __LINE__);

	if (pt != rt || pt != expect_ret)
		fail_row(R_SEMA_TRYWAIT, "retval", "return mismatch");
	check_slot(R_SEMA_TRYWAIT, "trywait", sl);

	int expect_val = (initial > 0) ? initial - 1 : 0;
	if (port::sema_value(&sl.port_s) != expect_val ||
	    ref_sema_value(&sl.ref_s) != expect_val)
		fail_row(R_SEMA_TRYWAIT, "value", "value mismatch");

	port::sema_destroy(&sl.port_s);
	ref_sema_destroy(&sl.ref_s);
}

static void
test_wait_one(int initial)
{
	case_row(R_SEMA_WAIT);
	sema_slot sl;

	slot_init(sl);
	env_reset(&sl.port_s, &sl.ref_s);
	env_configure(1, 1, 0, 1);

	port::sema_init(&sl.port_s, initial, "port");
	ref_sema_init(&sl.ref_s, initial, "ref");

	port::_sema_wait(&sl.port_s, __FILE__, __LINE__);
	ref__sema_wait(&sl.ref_s, __FILE__, __LINE__);

	check_slot(R_SEMA_WAIT, "wait", sl);

	int expect = (initial > 0) ? initial - 1 : 0;
	if (port::sema_value(&sl.port_s) != expect ||
	    ref_sema_value(&sl.ref_s) != expect)
		fail_row(R_SEMA_WAIT, "value", "value mismatch");

	port::sema_destroy(&sl.port_s);
	ref_sema_destroy(&sl.ref_s);
}

static void
test_timedwait_one(int initial, int timo, int release_after, int post_amount,
    int tw_post_after, int tw_fail_after, int expect_err, int expect_val)
{
	case_row(R_SEMA_TIMEDWAIT);
	sema_slot sl;

	slot_init(sl);
	env_reset(&sl.port_s, &sl.ref_s);
	env_configure(release_after, post_amount, tw_post_after, tw_fail_after);

	port::sema_init(&sl.port_s, initial, "port");
	ref_sema_init(&sl.ref_s, initial, "ref");

	int pe = port::_sema_timedwait(&sl.port_s, timo, __FILE__, __LINE__);
	int re = ref__sema_timedwait(&sl.ref_s, timo, __FILE__, __LINE__);

	if (pe != re || pe != expect_err)
		fail_row(R_SEMA_TIMEDWAIT, "retval", "return mismatch");
	check_slot(R_SEMA_TIMEDWAIT, "timedwait", sl);

	if (port::sema_value(&sl.port_s) != expect_val ||
	    ref_sema_value(&sl.ref_s) != expect_val)
		fail_row(R_SEMA_TIMEDWAIT, "value", "value mismatch");

	port::sema_destroy(&sl.port_s);
	ref_sema_destroy(&sl.ref_s);
}

static void
test_post_no_waiters_signal(void)
{
	case_row(R_SEMA_POST);
	sema_slot sl;

	slot_init(sl);
	env_reset(&sl.port_s, &sl.ref_s);
	env_configure(1, 1, 0, 1);

	port::sema_init(&sl.port_s, 0, "port");
	ref_sema_init(&sl.ref_s, 0, "ref");

	port::_sema_post(&sl.port_s, __FILE__, __LINE__);
	ref__sema_post(&sl.ref_s, __FILE__, __LINE__);

	if (sl.port_s.sema_cv.cv_signals != 0 || sl.ref_s.sema_cv.cv_signals != 0)
		fail_row(R_SEMA_POST, "no-waiters", "cv_signal spurious");

	port::sema_destroy(&sl.port_s);
	ref_sema_destroy(&sl.ref_s);
}

struct wait_post_args {
	sema_slot *sl;
	int done;
};

static void *
port_wait_thread(void *arg)
{
	wait_post_args *a = static_cast<wait_post_args *>(arg);

	port::_sema_wait(&a->sl->port_s, __FILE__, __LINE__);
	a->done = 1;
	return (nullptr);
}

static void *
ref_wait_thread(void *arg)
{
	wait_post_args *a = static_cast<wait_post_args *>(arg);

	ref__sema_wait(&a->sl->ref_s, __FILE__, __LINE__);
	a->done = 1;
	return (nullptr);
}

static void
test_wait_post_concurrent(void)
{
	case_row(R_SEMA_WAIT);
	sema_slot sl;
	wait_post_args wa;
	pthread_t pt, rt;

	slot_init(sl);
	wa = { &sl, 0 };
	env_reset(&sl.port_s, &sl.ref_s);
	env_configure(100000, 1, 0, 1);

	port::sema_init(&sl.port_s, 0, "port");
	ref_sema_init(&sl.ref_s, 0, "ref");

	pthread_create(&pt, nullptr, port_wait_thread, &wa);
	pthread_create(&rt, nullptr, ref_wait_thread, &wa);

	struct timespec ts = { 0, 50000000L };
	nanosleep(&ts, nullptr);

	case_row(R_SEMA_POST);
	port::_sema_post(&sl.port_s, __FILE__, __LINE__);
	ref__sema_post(&sl.ref_s, __FILE__, __LINE__);

	pthread_join(pt, nullptr);
	pthread_join(rt, nullptr);

	if (wa.done != 1)
		fail_row(R_SEMA_WAIT, "blocking", "wait did not complete");
	check_slot(R_SEMA_WAIT, "blocking", sl);

	if (port::sema_value(&sl.port_s) != 0 || ref_sema_value(&sl.ref_s) != 0)
		fail_row(R_SEMA_WAIT, "blocking-val", "value mismatch");

	port::sema_destroy(&sl.port_s);
	ref_sema_destroy(&sl.ref_s);
}

static void
test_sequence_one(int initial, int posts, int waits, int trywaits, int timo,
    int tw_post, int tw_fail)
{
	sema_slot sl;

	slot_init(sl);
	env_reset(&sl.port_s, &sl.ref_s);
	env_configure(1, 1, tw_post, tw_fail);

	port::sema_init(&sl.port_s, initial, "p");
	ref_sema_init(&sl.ref_s, initial, "r");

	for (int i = 0; i < posts; i++) {
		case_row(R_SEMA_POST);
		port::_sema_post(&sl.port_s, __FILE__, __LINE__);
		ref__sema_post(&sl.ref_s, __FILE__, __LINE__);
		check_slot(R_SEMA_POST, "seq-post", sl);
	}

	for (int i = 0; i < trywaits; i++) {
		case_row(R_SEMA_TRYWAIT);
		int pt = port::_sema_trywait(&sl.port_s, __FILE__, __LINE__);
		int rt = ref__sema_trywait(&sl.ref_s, __FILE__, __LINE__);
		if (pt != rt)
			fail_row(R_SEMA_TRYWAIT, "seq", "trywait retval");
		check_slot(R_SEMA_TRYWAIT, "seq-try", sl);
	}

	if (timo >= 0) {
		case_row(R_SEMA_TIMEDWAIT);
		int pe = port::_sema_timedwait(&sl.port_s, timo, __FILE__, __LINE__);
		int re = ref__sema_timedwait(&sl.ref_s, timo, __FILE__, __LINE__);
		if (pe != re)
			fail_row(R_SEMA_TIMEDWAIT, "seq", "timedwait retval");
		check_slot(R_SEMA_TIMEDWAIT, "seq-tw", sl);
	}

	for (int i = 0; i < waits; i++) {
		case_row(R_SEMA_WAIT);
		port::_sema_wait(&sl.port_s, __FILE__, __LINE__);
		ref__sema_wait(&sl.ref_s, __FILE__, __LINE__);
		check_slot(R_SEMA_WAIT, "seq-wait", sl);
	}

	case_row(R_SEMA_VALUE);
	int pv = port::sema_value(&sl.port_s);
	int rv = ref_sema_value(&sl.ref_s);
	if (pv != rv)
		fail_row(R_SEMA_VALUE, "seq", "value mismatch");

	case_row(R_SEMA_DESTROY);
	port::sema_destroy(&sl.port_s);
	ref_sema_destroy(&sl.ref_s);
	check_slot(R_SEMA_DESTROY, "seq", sl);
}

static void
test_hand(void)
{
	const int vals[] = { 0, 1, 2, 5, 10, 100, 1000 };

	for (int v : vals)
		test_init_one(v);

	for (int v : vals)
		test_post_one(v, 1);

	test_post_one(0, 3);
	test_post_one(5, 10);

	test_trywait_one(0, 0);
	test_trywait_one(1, 1);
	test_trywait_one(2, 1);

	for (int v : vals)
		test_wait_one(v);

	test_timedwait_one(0, 100, 1, 1, 0, 1, EWOULDBLOCK, 0);
	test_timedwait_one(0, 100, 1, 1, 1, 0, 0, 0);
	test_timedwait_one(1, 100, 1, 1, 0, 1, 0, 0);
	test_timedwait_one(2, 0, 1, 1, 0, 1, 0, 1);

	test_post_no_waiters_signal();
	test_wait_post_concurrent();

	test_sequence_one(0, 2, 1, 1, 50, 0, 1);
	test_sequence_one(3, 0, 2, 0, -1, 0, 1);
	test_sequence_one(1, 1, 1, 1, 1, 1, 0);
	test_sequence_one(0, 5, 2, 2, 100, 1, 1);
}

static void
test_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		int initial = (int)(rnd32() % 20);
		int posts = (int)(rnd32() % 5);
		int trywaits = (int)(rnd32() % 4);
		int waits = (int)(rnd32() % 3);
		int timo = (int)(rnd32() % 500);
		int tw_post = (rnd32() & 3) == 0 ? 1 : 0;
		int tw_fail = (rnd32() & 3) == 0 ? 0 : 1;
		int do_tw = (rnd32() & 1) != 0;

		test_sequence_one(initial, posts, waits, trywaits,
		    do_tw ? timo : -1, tw_post, tw_fail);

		if ((rnd32() % 5000) == 0)
			test_post_no_waiters_signal();
		if ((rnd32() % 5000) == 0)
			test_wait_post_concurrent();
	}
}

int
main(void)
{
	test_hand();
	test_sweep();

	long total_cases = 0;
	long total_fail = 0;

	std::printf("\n%-24s %12s %12s\n", "function", "cases", "failures");
	for (const auto &r : rows) {
		std::printf("%-24s %12ld %12ld\n", r.name, r.cases, r.failures);
		total_cases += r.cases;
		total_fail += r.failures;
	}
	std::printf("%-24s %12ld %12ld\n", "TOTAL", total_cases, total_fail);

	return (total_fail == 0 ? 0 : 1);
}
