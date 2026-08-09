/*
 * PBSD batch b0196s1 -- differential harness.
 *
 * Every call site drives BOTH the C++23 port and the ref_ oracle and compares
 * the complete observable result:
 *
 *   - the class of the returned pointer (process-global / thread-specific /
 *     NULL).  Raw addresses are never compared; each side is classified
 *     against its OWN rpc_createerr base, which is the pointer-offset
 *     comparison this interface admits.
 *   - the identity of the returned pointer relative to the previous call made
 *     on the same side from the same thread (the iterator-stability check).
 *   - the ENTIRE contents of the returned struct, byte for byte, after every
 *     call -- including the bytes that no code path is supposed to touch.
 *
 * The paths exercised are: main thread, worker threads, thread churn (key
 * reuse across many threads), a fixed-seed randomised sweep of 200000
 * iterations, and -- in a forked child -- thread-key exhaustion so that
 * rce_key_error becomes non-zero and the second operand of the
 * "thr_once(...) != 0 || rce_key_error != 0" test is the one that decides.
 */

import pbsd.lib.libc.rpc.b0196s1;

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>

namespace P = pbsd::lib_libc_rpc::b0196s1;

/* ---- the oracle, as C ------------------------------------------------- */
extern "C" {

enum clnt_stat {
	RPC_SUCCESS = 0,
	RPC_CANTENCODEARGS = 1,
	RPC_CANTDECODERES = 2,
	RPC_CANTSEND = 3,
	RPC_CANTRECV = 4,
	RPC_TIMEDOUT = 5,
	RPC_INTR = 18,
	RPC_UDERROR = 23,
	RPC_VERSMISMATCH = 6,
	RPC_AUTHERROR = 7,
	RPC_PROGUNAVAIL = 8,
	RPC_PROGVERSMISMATCH = 9,
	RPC_PROCUNAVAIL = 10,
	RPC_CANTDECODEARGS = 11,
	RPC_SYSTEMERROR = 12,
	RPC_UNKNOWNHOST = 13,
	RPC_UNKNOWNPROTO = 17,
	RPC_UNKNOWNADDR = 19,
	RPC_NOBROADCAST = 21,
	RPC_RPCBFAILURE = 14,
	RPC_PROGNOTREGISTERED = 15,
	RPC_N2AXLATEFAILURE = 22,
	RPC_TLIERROR = 20,
	RPC_FAILED = 16,
	RPC_INPROGRESS = 24,
	RPC_STALERACHANDLE = 25,
	RPC_CANTCONNECT = 26,
	RPC_XPRTFAILED = 27,
	RPC_CANTCREATESTREAM = 28
};

enum auth_stat {
	AUTH_OK = 0,
	AUTH_BADCRED = 1,
	AUTH_REJECTEDCRED = 2,
	AUTH_BADVERF = 3,
	AUTH_REJECTEDVERF = 4,
	AUTH_TOOWEAK = 5,
	AUTH_INVALIDRESP = 6,
	AUTH_FAILED = 7,
	AUTH_KERB_GENERIC = 8,
	AUTH_TIMEEXPIRE = 9,
	AUTH_TKT_FILE = 10,
	AUTH_DECODE = 11,
	AUTH_NET_ADDR = 12,
	RPCSEC_GSS_CREDPROBLEM = 13,
	RPCSEC_GSS_CTXPROBLEM = 14,
	RPCSEC_GSS_NODISPATCH = 15
};

typedef uint32_t rpcvers_t;

struct rpc_err {
	enum clnt_stat re_status;
	union {
		int RE_errno;
		enum auth_stat RE_why;
		struct {
			rpcvers_t low;
			rpcvers_t high;
		} RE_vers;
		struct {
			int32_t s1;
			int32_t s2;
		} RE_lb;
	} ru;
};

struct rpc_createerr {
	enum clnt_stat cf_stat;
	struct rpc_err cf_error;
};

extern struct rpc_createerr rpc_createerr;
struct rpc_createerr *ref___rpc_createerr(void);

extern pthread_rwlock_t __svc_lock;
extern pthread_rwlock_t __svc_fd_lock;
extern pthread_rwlock_t __rpcbaddr_cache_lock;
extern pthread_mutex_t __authdes_ops_lock;
extern pthread_mutex_t svcauthdesstats_lock;
extern pthread_mutex_t __authnone_lock;
extern pthread_mutex_t __authsvc_lock;
extern pthread_mutex_t __clnt_fd_lock;
extern pthread_mutex_t __clntraw_lock;
extern pthread_mutex_t __dupreq_lock;
extern pthread_mutex_t __loopnconf_lock;
extern pthread_mutex_t __ops_lock;
extern pthread_mutex_t __proglst_lock;
extern pthread_mutex_t __rpcsoc_lock;
extern pthread_mutex_t __svcraw_lock;
extern pthread_mutex_t __xprtlist_lock;

} /* extern "C" */

static const size_t RSZ = sizeof(struct ::rpc_createerr);
static_assert(sizeof(struct ::rpc_createerr) == sizeof(P::rpc_createerr),
    "port and oracle rpc_createerr must have identical layout");

/* ---- bookkeeping ------------------------------------------------------ */

enum {
	G_MAIN = 0,
	G_WORKER,
	G_KEYINIT,
	G_SWEEP,
	G_EXHAUST,
	G_LOCKS,
	G_COUNT
};

static const char *const g_names[G_COUNT] = {
	"__rpc_createerr  main-thread edge cases",
	"__rpc_createerr  worker-thread edge cases",
	"rce_key_init     tsd key reuse / churn",
	"__rpc_createerr  randomised sweep",
	"rce_key_init     key-create failure (forked)",
	"mt_misc          lock object definitions"
};

static long g_cases[G_COUNT];
static long g_fails[G_COUNT];
static int g_shown[G_COUNT];

struct Snap {
	int cls;	/* 0 = own global, 1 = other non-NULL, 2 = NULL */
	int stable;	/* -1 no previous, 1 same pointer, 0 different */
	unsigned char b[32];
};

static void
show(const char *tag, const Snap &s)
{
	fprintf(stderr, "        %-5s cls=%d stable=%2d bytes=", tag, s.cls,
	    s.stable);
	for (size_t i = 0; i < RSZ; i++)
		fprintf(stderr, "%02x", s.b[i]);
	fprintf(stderr, "\n");
}

static void
record(int gi, bool ok, const char *what, const Snap *a = nullptr,
    const Snap *b = nullptr)
{
	g_cases[gi]++;
	if (ok)
		return;
	g_fails[gi]++;
	if (g_shown[gi] >= 10)
		return;
	g_shown[gi]++;
	fprintf(stderr, "FAIL [%s] %s\n", g_names[gi], what);
	if (a != nullptr)
		show("ref", *a);
	if (b != nullptr)
		show("port", *b);
}

static bool
eq(const Snap &a, const Snap &b)
{
	return a.cls == b.cls && a.stable == b.stable &&
	    memcmp(a.b, b.b, sizeof a.b) == 0;
}

/* ---- snapshots -------------------------------------------------------- */

static void *
snap_ref(void *prev, Snap &s)
{
	struct ::rpc_createerr *p = ref___rpc_createerr();

	s.cls = (p == nullptr) ? 2 : ((p == &::rpc_createerr) ? 0 : 1);
	s.stable = (prev == nullptr) ? -1 : ((void *)p == prev ? 1 : 0);
	memset(s.b, 0, sizeof s.b);
	if (p != nullptr)
		memcpy(s.b, p, RSZ);
	return (void *)p;
}

static void *
snap_port(void *prev, Snap &s)
{
	auto *p = P::__rpc_createerr();

	s.cls = (p == nullptr) ? 2 : ((p == &P::rpc_createerr) ? 0 : 1);
	s.stable = (prev == nullptr) ? -1 : ((void *)p == prev ? 1 : 0);
	memset(s.b, 0, sizeof s.b);
	if (p != nullptr)
		memcpy(s.b, p, RSZ);
	return (void *)p;
}

/* Write the same bytes through both sides, when both handed back memory. */
static void
poke(void *pr, void *pp, const unsigned char *pat)
{
	if (pr != nullptr)
		memcpy(pr, pat, RSZ);
	if (pp != nullptr)
		memcpy(pp, pat, RSZ);
}

/* ---- hand written patterns -------------------------------------------- */

#define NPAT 26
static unsigned char pats[NPAT][32];

static void
build_patterns(void)
{
	memset(pats, 0, sizeof pats);
	for (size_t i = 0; i < RSZ; i++) {
		pats[0][i] = 0x00;			/* NUL heavy */
		pats[1][i] = 0xff;			/* all bits */
		pats[2][i] = (unsigned char)(0x80 + i);	/* high-bit bytes */
		pats[3][i] = 0x7f;			/* guard byte */
		pats[4][i] = (unsigned char)((i & 1) ? 0xff : 0x00);
		pats[5][i] = (unsigned char)i;		/* ascending */
		pats[6][i] = (unsigned char)(0xff - i);	/* descending */
		pats[7][i] = 0x01;
	}
	/* single bit set at each byte offset: boundary of the write window */
	for (size_t i = 0; i < RSZ && 8 + i < NPAT; i++)
		pats[8 + i][i] = 0x80;
	pats[24][0] = 0xff;				/* first byte only */
	pats[25][RSZ - 1] = 0xff;			/* last byte only */
}

/* ---- deterministic rng ------------------------------------------------ */

struct Rng {
	uint64_t s;

	explicit Rng(uint64_t seed)
	    : s(seed ? seed : 0x9e3779b97f4a7c15ull)
	{
	}

	uint64_t next()
	{
		s ^= s << 13;
		s ^= s >> 7;
		s ^= s << 17;
		return s;
	}

	uint32_t u32() { return (uint32_t)(next() >> 32); }
	unsigned below(unsigned n) { return u32() % n; }
};

/* ---- main thread ------------------------------------------------------ */

static void
test_main_thread(void)
{
	Snap sr, sp;
	void *pr = nullptr, *pp = nullptr;

	pr = snap_ref(pr, sr);
	pp = snap_port(pp, sp);
	record(G_MAIN, eq(sr, sp), "first call from main thread", &sr, &sp);
	record(G_MAIN, sr.cls == 0,
	    "oracle main thread returns process global (environment sanity)",
	    &sr, &sp);
	record(G_MAIN, sp.cls == 0,
	    "port main thread returns process global", &sr, &sp);

	/* the returned object must be the one the module exports */
	record(G_MAIN, pr == (void *)&::rpc_createerr, "oracle base identity");
	record(G_MAIN, pp == (void *)&P::rpc_createerr, "port base identity");

	for (int rep = 0; rep < 4; rep++) {
		Snap ar, ap;
		void *qr = snap_ref(pr, ar);
		void *qp = snap_port(pp, ap);
		record(G_MAIN, eq(ar, ap), "repeat call is stable", &ar, &ap);
		record(G_MAIN, ar.stable == 1 && ap.stable == 1,
		    "repeat call returns same object", &ar, &ap);
		pr = qr;
		pp = qp;
	}

	for (int k = 0; k < NPAT; k++) {
		Snap ar, ap;
		poke(pr, pp, pats[k]);
		pr = snap_ref(pr, ar);
		pp = snap_port(pp, ap);
		record(G_MAIN, eq(ar, ap), "pattern round-trip (main)", &ar,
		    &ap);
		record(G_MAIN, memcmp(ar.b, pats[k], RSZ) == 0 &&
		    memcmp(ap.b, pats[k], RSZ) == 0,
		    "pattern survives the call", &ar, &ap);
	}

	/* leave the globals in a known state for the sweep */
	poke(pr, pp, pats[0]);
}

/* ---- worker threads --------------------------------------------------- */

static void *
worker_edge(void *vp)
{
	int gi = *(int *)vp;
	Snap sr, sp;
	void *pr = nullptr, *pp = nullptr;
	static const unsigned char zero[32] = { 0 };

	pr = snap_ref(pr, sr);
	pp = snap_port(pp, sp);
	record(gi, eq(sr, sp), "first call from worker thread", &sr, &sp);
	record(gi, sr.cls == 1,
	    "oracle worker gets thread-specific storage (sanity)", &sr, &sp);
	record(gi, sp.cls == 1,
	    "port worker gets thread-specific storage", &sr, &sp);
	record(gi, memcmp(sr.b, zero, RSZ) == 0,
	    "oracle fresh thread-specific storage is zeroed", &sr, &sp);
	record(gi, memcmp(sp.b, zero, RSZ) == 0,
	    "port fresh thread-specific storage is zeroed", &sr, &sp);

	for (int k = 0; k < NPAT; k++) {
		Snap ar, ap;
		poke(pr, pp, pats[k]);
		pr = snap_ref(pr, ar);
		pp = snap_port(pp, ap);
		record(gi, eq(ar, ap), "pattern round-trip (worker)", &ar, &ap);
		record(gi, ar.stable == 1 && ap.stable == 1,
		    "worker keeps handing back the same object", &ar, &ap);
		record(gi, memcmp(ar.b, pats[k], RSZ) == 0 &&
		    memcmp(ap.b, pats[k], RSZ) == 0,
		    "worker pattern survives the call", &ar, &ap);
	}
	return nullptr;
}

static void
run_worker(int gi)
{
	pthread_t t;
	int arg = gi;

	if (pthread_create(&t, nullptr, worker_edge, &arg) != 0) {
		record(gi, false, "pthread_create failed");
		return;
	}
	pthread_join(t, nullptr);
}

/* ---- randomised sweep ------------------------------------------------- */

struct SweepArg {
	uint64_t seed;
	long iters;
};

static void
sweep_body(uint64_t seed, long iters)
{
	Rng rng(seed);
	Snap sr, sp;
	void *pr = nullptr, *pp = nullptr;
	unsigned char buf[32];

	for (long i = 0; i < iters; i++) {
		unsigned op = rng.below(4);

		switch (op) {
		case 0:
			break;
		case 1:
			for (size_t j = 0; j < RSZ; j++)
				buf[j] = (unsigned char)rng.u32();
			poke(pr, pp, buf);
			break;
		case 2: {
			unsigned char fill = (unsigned char)rng.u32();
			memset(buf, fill, RSZ);
			poke(pr, pp, buf);
			break;
		}
		case 3:
			poke(pr, pp, pats[rng.below(NPAT)]);
			break;
		}

		pr = snap_ref(pr, sr);
		pp = snap_port(pp, sp);
		if (!eq(sr, sp)) {
			record(G_SWEEP, false, "sweep divergence", &sr, &sp);
			continue;
		}
		record(G_SWEEP, true, "sweep");
	}
}

static void *
sweep_thread(void *vp)
{
	SweepArg *a = (SweepArg *)vp;

	sweep_body(a->seed, a->iters);
	return nullptr;
}

/* ---- thread churn: the key created by rce_key_init must keep working --- */

static void *
churn_thread(void *vp)
{
	int gi = *(int *)vp;
	Snap sr, sp;
	void *pr = nullptr, *pp = nullptr;
	static const unsigned char zero[32] = { 0 };

	pr = snap_ref(pr, sr);
	pp = snap_port(pp, sp);
	record(gi, eq(sr, sp), "churn: first call", &sr, &sp);
	record(gi, memcmp(sp.b, zero, RSZ) == 0 &&
	    memcmp(sr.b, zero, RSZ) == 0,
	    "churn: storage re-zeroed for the new thread", &sr, &sp);

	for (int k = 0; k < 5; k++) {
		Snap ar, ap;
		unsigned char pat[32];
		memset(pat, (unsigned char)(0x80 + k), RSZ);
		poke(pr, pp, pat);
		pr = snap_ref(pr, ar);
		pp = snap_port(pp, ap);
		record(gi, eq(ar, ap), "churn: repeat call", &ar, &ap);
		record(gi, ar.stable == 1 && ap.stable == 1,
		    "churn: pointer stable within thread", &ar, &ap);
	}
	return nullptr;
}

/* ---- forked child: thread-key exhaustion ------------------------------ */

static int child_ref_cls;
static int child_port_cls;
static unsigned char child_ref_b[32];
static unsigned char child_port_b[32];

static void *
child_worker(void *)
{
	Snap sr, sp;

	(void)snap_ref(nullptr, sr);
	(void)snap_port(nullptr, sp);
	child_ref_cls = sr.cls;
	child_port_cls = sp.cls;
	memcpy(child_ref_b, sr.b, sizeof child_ref_b);
	memcpy(child_port_b, sp.b, sizeof child_port_b);
	return nullptr;
}

#define MAXKEYS 8192
static pthread_key_t hog[MAXKEYS];

static int
child_body(void)
{
	size_t n = 0;
	pthread_t t;
	Snap sr, sp;

	/*
	 * Take every thread specific key in the process.  rce_key_init() will
	 * then fail, rce_key_error becomes non-zero, and __rpc_createerr()
	 * must take the "|| rce_key_error != 0" exit even though thr_once()
	 * itself succeeded.
	 */
	while (n < MAXKEYS) {
		pthread_key_t k;
		if (pthread_key_create(&k, nullptr) != 0)
			break;
		hog[n++] = k;
	}
	if (n == MAXKEYS) {
		fprintf(stderr,
		    "child: could not exhaust thread keys (%zu taken)\n", n);
		return 3;
	}

	if (pthread_create(&t, nullptr, child_worker, nullptr) != 0) {
		fprintf(stderr, "child: pthread_create failed\n");
		return 4;
	}
	pthread_join(t, nullptr);

	if (child_ref_cls != child_port_cls ||
	    memcmp(child_ref_b, child_port_b, RSZ) != 0) {
		fprintf(stderr,
		    "child: key-exhausted worker diverged: ref cls=%d "
		    "port cls=%d\n", child_ref_cls, child_port_cls);
		return 1;
	}
	if (child_ref_cls != 0) {
		fprintf(stderr,
		    "child: oracle did not fall back to the global "
		    "(cls=%d)\n", child_ref_cls);
		return 5;
	}

	(void)snap_ref(nullptr, sr);
	(void)snap_port(nullptr, sp);
	if (!eq(sr, sp) || sp.cls != 0) {
		fprintf(stderr, "child: main-thread call diverged\n");
		return 1;
	}
	return 0;
}

static void
test_key_exhaustion(void)
{
	pid_t pid = fork();
	int st = 0;

	if (pid < 0) {
		record(G_EXHAUST, false, "fork failed");
		return;
	}
	if (pid == 0)
		_exit(child_body());
	if (waitpid(pid, &st, 0) != pid) {
		record(G_EXHAUST, false, "waitpid failed");
		return;
	}
	if (!WIFEXITED(st)) {
		record(G_EXHAUST, false,
		    "key-exhausted child terminated abnormally");
		return;
	}
	if (WEXITSTATUS(st) != 0) {
		char msg[128];
		snprintf(msg, sizeof msg,
		    "key-exhausted child reported status %d",
		    WEXITSTATUS(st));
		record(G_EXHAUST, false, msg);
		return;
	}
	record(G_EXHAUST, true, "key-exhausted fall back to global");
}

/* ---- lock objects ----------------------------------------------------- */

static void
cmp_rwlock(const char *name, pthread_rwlock_t *r, pthread_rwlock_t *p)
{
	char msg[160];

	snprintf(msg, sizeof msg, "%s: initialiser bytes", name);
	record(G_LOCKS, memcmp(r, p, sizeof(pthread_rwlock_t)) == 0, msg);
	snprintf(msg, sizeof msg, "%s: usable as a rwlock", name);
	record(G_LOCKS, pthread_rwlock_trywrlock(r) == 0 &&
	    pthread_rwlock_unlock(r) == 0 &&
	    pthread_rwlock_trywrlock(p) == 0 &&
	    pthread_rwlock_unlock(p) == 0, msg);
}

static void
cmp_mutex(const char *name, pthread_mutex_t *r, pthread_mutex_t *p)
{
	char msg[160];

	snprintf(msg, sizeof msg, "%s: initialiser bytes", name);
	record(G_LOCKS, memcmp(r, p, sizeof(pthread_mutex_t)) == 0, msg);
	snprintf(msg, sizeof msg, "%s: usable as a mutex", name);
	record(G_LOCKS, pthread_mutex_trylock(r) == 0 &&
	    pthread_mutex_unlock(r) == 0 &&
	    pthread_mutex_trylock(p) == 0 &&
	    pthread_mutex_unlock(p) == 0, msg);
}

static void
test_locks(void)
{
	cmp_rwlock("__svc_lock", &__svc_lock, &P::__svc_lock);
	cmp_rwlock("__svc_fd_lock", &__svc_fd_lock, &P::__svc_fd_lock);
	cmp_rwlock("__rpcbaddr_cache_lock", &__rpcbaddr_cache_lock,
	    &P::__rpcbaddr_cache_lock);
	cmp_mutex("__authdes_ops_lock", &__authdes_ops_lock,
	    &P::__authdes_ops_lock);
	cmp_mutex("svcauthdesstats_lock", &svcauthdesstats_lock,
	    &P::svcauthdesstats_lock);
	cmp_mutex("__authnone_lock", &__authnone_lock, &P::__authnone_lock);
	cmp_mutex("__authsvc_lock", &__authsvc_lock, &P::__authsvc_lock);
	cmp_mutex("__clnt_fd_lock", &__clnt_fd_lock, &P::__clnt_fd_lock);
	cmp_mutex("__clntraw_lock", &__clntraw_lock, &P::__clntraw_lock);
	cmp_mutex("__dupreq_lock", &__dupreq_lock, &P::__dupreq_lock);
	cmp_mutex("__loopnconf_lock", &__loopnconf_lock, &P::__loopnconf_lock);
	cmp_mutex("__ops_lock", &__ops_lock, &P::__ops_lock);
	cmp_mutex("__proglst_lock", &__proglst_lock, &P::__proglst_lock);
	cmp_mutex("__rpcsoc_lock", &__rpcsoc_lock, &P::__rpcsoc_lock);
	cmp_mutex("__svcraw_lock", &__svcraw_lock, &P::__svcraw_lock);
	cmp_mutex("__xprtlist_lock", &__xprtlist_lock, &P::__xprtlist_lock);
}

/* ---- driver ----------------------------------------------------------- */

int
main(void)
{
	build_patterns();

	/*
	 * Fork first, while both sides are still pristine: the child needs a
	 * process in which no thread key has been created yet.
	 */
	test_key_exhaustion();

	test_locks();
	test_main_thread();

	for (int i = 0; i < 8; i++)
		run_worker(G_WORKER);

	for (int i = 0; i < 64; i++) {
		pthread_t t;
		int arg = G_KEYINIT;
		if (pthread_create(&t, nullptr, churn_thread, &arg) != 0) {
			record(G_KEYINIT, false, "pthread_create failed");
			continue;
		}
		pthread_join(t, nullptr);
	}

	/* 120000 main-thread iterations + 4 x 20000 worker iterations */
	sweep_body(0x0196000151515151ull, 120000);
	for (int i = 0; i < 4; i++) {
		pthread_t t;
		SweepArg a;
		a.seed = 0xb019600000000001ull +
		    (uint64_t)i * 0x9e3779b97f4a7c15ull;
		a.iters = 20000;
		if (pthread_create(&t, nullptr, sweep_thread, &a) != 0) {
			record(G_SWEEP, false, "pthread_create failed");
			continue;
		}
		pthread_join(t, nullptr);
	}

	long tot_cases = 0, tot_fails = 0;
	printf("\n%-46s %10s %10s\n", "function / group", "cases", "failures");
	printf("%-46s %10s %10s\n",
	    "----------------------------------------------",
	    "----------", "----------");
	for (int i = 0; i < G_COUNT; i++) {
		printf("%-46s %10ld %10ld\n", g_names[i], g_cases[i],
		    g_fails[i]);
		tot_cases += g_cases[i];
		tot_fails += g_fails[i];
	}
	printf("%-46s %10s %10s\n",
	    "----------------------------------------------",
	    "----------", "----------");
	printf("%-46s %10ld %10ld\n", "TOTAL", tot_cases, tot_fails);

	return tot_fails == 0 ? 0 : 1;
}
