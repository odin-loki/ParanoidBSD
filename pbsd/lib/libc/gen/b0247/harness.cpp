/*
 * Differential harness for batch b0247 (_thread_init_stub,
 * _thread_autoinit_dummy_decl_stub, _pthread_mutex_init_calloc_cb_stub,
 * cap_sandboxed).  Every case drives both the C++23 port and the C oracle.
 */

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <pthread.h>

import pbsd.lib.libc.gen.b0247;

namespace P = pbsd::lib_libc_gen::b0247;

extern "C" {
void ref__thread_init_stub(void);
extern int ref__thread_autoinit_dummy_decl_stub;
int ref__pthread_mutex_init_calloc_cb_stub(pthread_mutex_t *mutex,
    void *(calloc_cb)(size_t, size_t));
bool ref_cap_sandboxed(void);
}

/* ------------------------------------------------------------------ */
/* Mock cap_getmode (shared by port and oracle)                       */
/* ------------------------------------------------------------------ */

typedef unsigned int u_int;

static int mock_cap_getmode_ret;
static u_int mock_cap_getmode_mode;
static int mock_cap_getmode_errno;

extern "C" int
cap_getmode(u_int *modep)
{

	if (mock_cap_getmode_ret != 0) {
		errno = mock_cap_getmode_errno;
		return (-1);
	}
	if (modep != nullptr)
		*modep = mock_cap_getmode_mode;
	return (0);
}

static void
cap_mock_reset(int ret, u_int mode, int err)
{

	mock_cap_getmode_ret = ret;
	mock_cap_getmode_mode = mode;
	mock_cap_getmode_errno = err;
}

/* ------------------------------------------------------------------ */
/* Statistics                                                         */
/* ------------------------------------------------------------------ */

enum {
	F_THREAD_INIT,
	F_AUTOINIT_DECL,
	F_MUTEX_INIT_CALLOC_CB,
	F_CAP_SANDBOXED,
	NFUNC
};

static const char *const fname[NFUNC] = {
	"_thread_init_stub",
	"_thread_autoinit_dummy_decl_stub",
	"_pthread_mutex_init_calloc_cb_stub",
	"cap_sandboxed"
};

static unsigned long long ncases[NFUNC];
static unsigned long long nfails[NFUNC];

static constexpr int MAXREPORT = 12;
static int nreported;

static void
report(int f, const char *ctx, const char *detail)
{
	nfails[f]++;
	if (nreported < MAXREPORT) {
		nreported++;
		std::printf("FAIL %-32s %s: %s\n", fname[f], ctx, detail);
	} else if (nreported == MAXREPORT) {
		nreported++;
		std::printf("... further failures suppressed\n");
	}
}

/* ------------------------------------------------------------------ */
/* PRNG (fixed seed)                                                  */
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state = 0xc0ffeebaddecade1ULL;

static std::uint64_t
nextrand(void)
{
	std::uint64_t x = rng_state;

	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	rng_state = x;
	return x * 0x2545f4914f6cdd1dULL;
}

/* ------------------------------------------------------------------ */
/* _thread_init_stub                                                  */
/* ------------------------------------------------------------------ */

static bool
thread_init_stub_ok(const char *ctx)
{
	P::_thread_init_stub();
	ref__thread_init_stub();
	return true;
}

static void
test_thread_init_stub_edges(void)
{
	const char *cases[] = {
		"empty",
		"after autoinit read",
		"twice",
		"triple",
	};

	for (const char *ctx : cases) {
		ncases[F_THREAD_INIT]++;
		if (!thread_init_stub_ok(ctx))
			report(F_THREAD_INIT, ctx, "unexpected failure");
	}
}

static void
test_thread_init_stub_random(void)
{

	for (int i = 0; i < 200000; i++) {
		char ctx[48];

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		ncases[F_THREAD_INIT]++;
		if (!thread_init_stub_ok(ctx))
			report(F_THREAD_INIT, ctx, "unexpected failure");
	}
}

/* ------------------------------------------------------------------ */
/* _thread_autoinit_dummy_decl_stub                                   */
/* ------------------------------------------------------------------ */

static bool
autoinit_decl_ok(const char *ctx)
{
	int a = P::_thread_autoinit_dummy_decl_stub;
	int b = ref__thread_autoinit_dummy_decl_stub;

	if (a != b) {
		char msg[80];

		std::snprintf(msg, sizeof msg, "port=%d ref=%d", a, b);
		report(F_AUTOINIT_DECL, ctx, msg);
		return false;
	}
	return true;
}

static void
test_autoinit_decl_edges(void)
{
	const char *cases[] = {
		"initial zero",
		"after thread_init_stub",
		"after cap_sandboxed false",
		"after cap_sandboxed true",
	};

	for (const char *ctx : cases) {
		if (std::strcmp(ctx, "after thread_init_stub") == 0)
			P::_thread_init_stub();
		if (std::strcmp(ctx, "after cap_sandboxed false") == 0) {
			cap_mock_reset(0, 0, 0);
			(void)P::cap_sandboxed();
		}
		if (std::strcmp(ctx, "after cap_sandboxed true") == 0) {
			cap_mock_reset(0, 1, 0);
			(void)P::cap_sandboxed();
		}

		ncases[F_AUTOINIT_DECL]++;
		autoinit_decl_ok(ctx);
	}
}

static void
test_autoinit_decl_random(void)
{

	for (int i = 0; i < 200000; i++) {
		char ctx[48];

		if ((nextrand() & 3u) == 0u)
			P::_thread_init_stub();
		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		ncases[F_AUTOINIT_DECL]++;
		autoinit_decl_ok(ctx);
	}
}

/* ------------------------------------------------------------------ */
/* _pthread_mutex_init_calloc_cb_stub                                 */
/* ------------------------------------------------------------------ */

typedef void *(calloc_fn)(size_t, size_t);

static void *
dummy_calloc0(size_t n, size_t s)
{
	(void)n;
	(void)s;
	return nullptr;
}

static void *
dummy_calloc1(size_t n, size_t s)
{
	static char buf[16];

	(void)n;
	(void)s;
	return buf;
}

static void *
dummy_calloc_highbit(size_t n, size_t s)
{
	(void)n;
	(void)s;
	return (void *)(std::uintptr_t)0x80808080u;
}

static bool
mutex_init_calloc_cb_ok(pthread_mutex_t *mutex, calloc_fn *calloc_cb,
    const char *ctx)
{
	int a = P::_pthread_mutex_init_calloc_cb_stub(
	    reinterpret_cast<P::pthread_mutex_t *>(mutex), calloc_cb);
	int b = ref__pthread_mutex_init_calloc_cb_stub(mutex, calloc_cb);

	if (a != b) {
		char msg[80];

		std::snprintf(msg, sizeof msg, "port=%d ref=%d", a, b);
		report(F_MUTEX_INIT_CALLOC_CB, ctx, msg);
		return false;
	}
	return true;
}

static void
test_mutex_init_calloc_cb_edges(void)
{
	pthread_mutex_t m0 = {};
	pthread_mutex_t m1;
	unsigned char raw[sizeof(pthread_mutex_t)];

	std::memset(&m1, 0x7f, sizeof(m1));
	std::memset(raw, 0x80, sizeof(raw));

	struct Case {
		pthread_mutex_t *mutex;
		calloc_fn *calloc_cb;
		const char *label;
	} cases[] = {
		{ nullptr, nullptr, "null mutex null cb" },
		{ nullptr, dummy_calloc0, "null mutex valid cb" },
		{ &m0, nullptr, "zero mutex null cb" },
		{ &m0, dummy_calloc0, "zero mutex calloc0" },
		{ &m1, dummy_calloc1, "0x7f mutex calloc1" },
		{ reinterpret_cast<pthread_mutex_t *>(raw), dummy_calloc_highbit,
		    "0x80 mutex highbit cb" },
	};

	for (const auto &c : cases) {
		ncases[F_MUTEX_INIT_CALLOC_CB]++;
		mutex_init_calloc_cb_ok(c.mutex, c.calloc_cb, c.label);
	}
}

static void
test_mutex_init_calloc_cb_random(void)
{
	alignas(pthread_mutex_t) unsigned char storage[sizeof(pthread_mutex_t) +
	    128];

	for (int i = 0; i < 200000; i++) {
		pthread_mutex_t *mutex;
		calloc_fn *calloc_cb;
		char ctx[48];

		switch (nextrand() % 5u) {
		case 0:
			mutex = nullptr;
			break;
		case 1:
			mutex = reinterpret_cast<pthread_mutex_t *>(storage);
			std::memset(mutex, 0, sizeof(pthread_mutex_t));
			break;
		default:
			mutex = reinterpret_cast<pthread_mutex_t *>(storage);
			for (size_t j = 0; j < sizeof(pthread_mutex_t); j++)
				reinterpret_cast<unsigned char *>(mutex)[j] =
				    (unsigned char)(nextrand() & 0xffu);
			break;
		}

		switch (nextrand() % 4u) {
		case 0:
			calloc_cb = nullptr;
			break;
		case 1:
			calloc_cb = dummy_calloc0;
			break;
		case 2:
			calloc_cb = dummy_calloc1;
			break;
		default:
			calloc_cb = reinterpret_cast<calloc_fn *>(
			    (std::uintptr_t)(nextrand() | 1u));
			break;
		}

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		ncases[F_MUTEX_INIT_CALLOC_CB]++;
		mutex_init_calloc_cb_ok(mutex, calloc_cb, ctx);
	}
}

/* ------------------------------------------------------------------ */
/* cap_sandboxed                                                      */
/* ------------------------------------------------------------------ */

static bool
cap_sandboxed_ok(int ret, u_int mode, int err, const char *ctx)
{
	bool a, b;

	cap_mock_reset(ret, mode, err);
	a = P::cap_sandboxed();
	cap_mock_reset(ret, mode, err);
	b = ref_cap_sandboxed();

	if (a != b) {
		char msg[96];

		std::snprintf(msg, sizeof msg,
		    "ret=%d mode=%u err=%d port=%d ref=%d", ret, mode, err,
		    (int)a, (int)b);
		report(F_CAP_SANDBOXED, ctx, msg);
		return false;
	}
	return true;
}

static void
test_cap_sandboxed_edges(void)
{
	struct Case {
		int ret;
		u_int mode;
		int err;
		const char *label;
	} cases[] = {
		{ 0, 0, 0, "success mode 0" },
		{ 0, 1, 0, "success mode 1" },
		{ -1, 0, ENOSYS, "fail ENOSYS" },
		{ 0, 0, 0, "repeat mode 0" },
		{ 0, 1, 0, "repeat mode 1" },
		{ -1, 1, ENOSYS, "fail ENOSYS mode 1 unused" },
	};

	for (const auto &c : cases) {
		ncases[F_CAP_SANDBOXED]++;
		cap_sandboxed_ok(c.ret, c.mode, c.err, c.label);
	}
}

static void
test_cap_sandboxed_random(void)
{

	for (int i = 0; i < 200000; i++) {
		int ret;
		u_int mode;
		int err;
		char ctx[48];

		if ((nextrand() & 1u) == 0u) {
			ret = 0;
			mode = (u_int)(nextrand() & 1u);
			err = 0;
		} else {
			ret = -1;
			mode = (u_int)(nextrand() & 1u);
			err = ENOSYS;
		}

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		ncases[F_CAP_SANDBOXED]++;
		cap_sandboxed_ok(ret, mode, err, ctx);
	}
}

/* ------------------------------------------------------------------ */
/* main                                                               */
/* ------------------------------------------------------------------ */

int
main(void)
{
	test_thread_init_stub_edges();
	test_thread_init_stub_random();
	test_autoinit_decl_edges();
	test_autoinit_decl_random();
	test_mutex_init_calloc_cb_edges();
	test_mutex_init_calloc_cb_random();
	test_cap_sandboxed_edges();
	test_cap_sandboxed_random();

	std::printf("\n%-34s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < NFUNC; i++)
		std::printf("%-34s %12llu %12llu\n", fname[i],
		    ncases[i], nfails[i]);

	unsigned long long total_fail = 0;
	for (int i = 0; i < NFUNC; i++)
		total_fail += nfails[i];

	return total_fail == 0 ? 0 : 1;
}
