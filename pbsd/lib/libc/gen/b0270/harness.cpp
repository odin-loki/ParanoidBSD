/*
 * harness.cpp -- differential test for batch b0270.
 *
 * __isnan, __isnanf, _once, and __sleep are driven side by side with the ref_
 * oracle.  Every observable is compared: return values, errno, once-control
 * state, init-routine invocation counts, and nanosleep mock arguments.
 */

import pbsd.lib.libc.gen.b0270;

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <errno.h>
#include <time.h>

namespace port = pbsd::lib_libc_gen::b0270;

#define	PTHREAD_DONE_INIT	2
#define	GUARD			0x7f
#define	SWEEP_ITERS		200000L
#define	MAX_PRINT		12

struct ref_once_control {
	int	state;
	void	*mutex;
};

extern "C" {
int ref___isnan(double);
int ref___isnanf(float);
int ref__once(ref_once_control *, void (*)(void));
unsigned int ref___sleep(unsigned int);
}

enum {
	F_ISNAN,
	F_ISNANF,
	F_ONCE,
	F_SLEEP,
	NFUNC
};

static const char *const fname[NFUNC] = {
	"__isnan",
	"__isnanf",
	"_once",
	"__sleep",
};

static unsigned long long ncases[NFUNC];
static unsigned long long nfails[NFUNC];
static int nprinted;

static void
report(int f, const char *ctx, const char *detail)
{
	nfails[f]++;
	if (nprinted < MAX_PRINT) {
		nprinted++;
		std::printf("FAIL %-10s %-28s %s\n", fname[f], ctx, detail);
	} else if (nprinted == MAX_PRINT) {
		nprinted++;
		std::printf("... further failures suppressed\n");
	}
}

/* ------------------------------------------------------------------ PRNG */

static std::uint64_t rng_state = 0xb0270decafbad26ULL;

static std::uint64_t
rnd64(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static std::uint32_t
rnd32(void)
{
	return ((std::uint32_t)(rnd64() >> 32));
}

/* --------------------------------------------------------- IEEE helpers */

union d_bits {
	double	d;
	struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		unsigned int	manl	:32;
		unsigned int	manh	:20;
		unsigned int	exp	:11;
		unsigned int	sign	:1;
#else
		unsigned int	sign	:1;
		unsigned int	exp	:11;
		unsigned int	manh	:20;
		unsigned int	manl	:32;
#endif
	} bits;
};

union f_bits {
	float	f;
	struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		unsigned int	man	:23;
		unsigned int	exp	:8;
		unsigned int	sign	:1;
#else
		unsigned int	sign	:1;
		unsigned int	exp	:8;
		unsigned int	man	:23;
#endif
	} bits;
};

static double
mkdouble(unsigned exp, unsigned manh, unsigned manl, unsigned sign)
{
	union d_bits u;

	u.bits.manl = manl;
	u.bits.manh = manh;
	u.bits.exp = exp;
	u.bits.sign = sign;
	return (u.d);
}

static float
mkfloat(unsigned exp, unsigned man, unsigned sign)
{
	union f_bits u;

	u.bits.man = man;
	u.bits.exp = exp;
	u.bits.sign = sign;
	return (u.f);
}

static bool
check_isnan_case(int f, const char *ctx, double d)
{
	int ref_rv, port_rv;

	ref_rv = ref___isnan(d);
	port_rv = port::__isnan(d);
	ncases[f]++;
	if (ref_rv != port_rv) {
		report(f, ctx, "return mismatch");
		return (false);
	}
	return (true);
}

static bool
check_isnanf_case(int f, const char *ctx, float fl)
{
	int ref_rv, port_rv;

	ref_rv = ref___isnanf(fl);
	port_rv = port::__isnanf(fl);
	ncases[f]++;
	if (ref_rv != port_rv) {
		report(f, ctx, "return mismatch");
		return (false);
	}
	return (true);
}

static void
test_isnan_handwritten(void)
{
	const int f = F_ISNAN;

	(void)check_isnan_case(f, "zero", 0.0);
	(void)check_isnan_case(f, "one", 1.0);
	(void)check_isnan_case(f, "neg", -1.0);
	(void)check_isnan_case(f, "quiet-nan", mkdouble(2047, 0, 1, 0));
	(void)check_isnan_case(f, "quiet-nan-manh", mkdouble(2047, 1, 0, 0));
	(void)check_isnan_case(f, "quiet-nan-both", mkdouble(2047, 0xfffff, 0xffffffffu, 0));
	(void)check_isnan_case(f, "pos-inf", mkdouble(2047, 0, 0, 0));
	(void)check_isnan_case(f, "neg-inf", mkdouble(2047, 0, 0, 1));
	(void)check_isnan_case(f, "exp-2046", mkdouble(2046, 0xfffff, 0xffffffffu, 0));
	(void)check_isnan_case(f, "exp-0", mkdouble(0, 0, 1, 0));
	(void)check_isnan_case(f, "subnormal", mkdouble(0, 0, 1, 0));
	(void)check_isnan_case(f, "neg-zero", -0.0);
}

static void
test_isnan_random(void)
{
	const int f = F_ISNAN;
	long i;

	for (i = 0; i < SWEEP_ITERS; i++) {
		char ctx[48];
		union d_bits u;

		u.bits.manl = rnd32();
		u.bits.manh = (unsigned)(rnd32() & 0xfffffu);
		u.bits.exp = (unsigned)(rnd32() % 2048u);
		u.bits.sign = (unsigned)(rnd32() & 1u);

		if ((rnd32() & 7u) == 0u) {
			u.bits.exp = 2047;
			switch (rnd32() % 4) {
			case 0:
				u.bits.manl = 0;
				u.bits.manh = 0;
				break;
			case 1:
				u.bits.manl = rnd32() | 1u;
				u.bits.manh = 0;
				break;
			case 2:
				u.bits.manl = 0;
				u.bits.manh = (unsigned)(rnd32() & 0xfffffu) | 1u;
				break;
			default:
				u.bits.manl = rnd32() | 1u;
				u.bits.manh = (unsigned)(rnd32() & 0xfffffu) | 1u;
				break;
			}
		}

		std::snprintf(ctx, sizeof(ctx), "rand#%ld", i);
		(void)check_isnan_case(f, ctx, u.d);
	}
}

static void
test_isnanf_handwritten(void)
{
	const int f = F_ISNANF;

	(void)check_isnanf_case(f, "zero", 0.0f);
	(void)check_isnanf_case(f, "one", 1.0f);
	(void)check_isnanf_case(f, "quiet-nan", mkfloat(255, 1, 0));
	(void)check_isnanf_case(f, "max-man-nan", mkfloat(255, 0x7fffff, 0));
	(void)check_isnanf_case(f, "pos-inf", mkfloat(255, 0, 0));
	(void)check_isnanf_case(f, "neg-inf", mkfloat(255, 0, 1));
	(void)check_isnanf_case(f, "exp-254", mkfloat(254, 0x7fffff, 0));
	(void)check_isnanf_case(f, "exp-0", mkfloat(0, 1, 0));
	(void)check_isnanf_case(f, "neg-zero", -0.0f);
}

static void
test_isnanf_random(void)
{
	const int f = F_ISNANF;
	long i;

	for (i = 0; i < SWEEP_ITERS; i++) {
		char ctx[48];
		union f_bits u;

		u.bits.man = (unsigned)(rnd32() & 0x7fffffu);
		u.bits.exp = (unsigned)(rnd32() % 256u);
		u.bits.sign = (unsigned)(rnd32() & 1u);

		if ((rnd32() & 7u) == 0u) {
			u.bits.exp = 255;
			if ((rnd32() & 1u) == 0u)
				u.bits.man = 0;
			else
				u.bits.man = (unsigned)(rnd32() & 0x7fffffu) | 1u;
		}

		std::snprintf(ctx, sizeof(ctx), "rand#%ld", i);
		(void)check_isnanf_case(f, ctx, u.f);
	}
}

/* --------------------------------------------------------- _once mocks */

int __isthreaded;

static int g_init_count;
static int g_pthread_once_calls;
static int g_pthread_once_rv;
static ref_once_control *g_pthread_once_arg;
static void (*g_pthread_once_init)(void);

static void
test_init(void)
{
	g_init_count++;
}

extern "C" int
_pthread_once(void *once_control, void (*init_routine)(void))
{
	g_pthread_once_calls++;
	g_pthread_once_arg = (ref_once_control *)once_control;
	g_pthread_once_init = init_routine;
	return (g_pthread_once_rv);
}

struct once_obs {
	int	rv;
	int	state;
	void	*mutex;
	int	init_count;
	int	pthread_calls;
};

static once_obs
snap_once(int rv, int state, void *mutex)
{
	once_obs o;

	o.rv = rv;
	o.state = state;
	o.mutex = mutex;
	o.init_count = g_init_count;
	o.pthread_calls = g_pthread_once_calls;
	return (o);
}

static bool
once_obs_eq(const once_obs &a, const once_obs &b)
{
	return (a.rv == b.rv && a.state == b.state && a.mutex == b.mutex &&
	    a.init_count == b.init_count && a.pthread_calls == b.pthread_calls);
}

static bool
check_once_case(int f, const char *ctx, int threaded, int start_state,
    int pthread_rv)
{
	ref_once_control ref_oc;
	port::pthread_once_t port_oc;
	int ref_rv, port_rv;
	once_obs ref_o, port_o;
	void *marker = (void *)0xdeadbeef;

	ref_oc.state = start_state;
	ref_oc.mutex = marker;
	port_oc.state = start_state;
	port_oc.mutex = marker;

	__isthreaded = threaded;
	g_init_count = 0;
	g_pthread_once_calls = 0;
	g_pthread_once_rv = pthread_rv;
	g_pthread_once_arg = nullptr;
	g_pthread_once_init = nullptr;

	ref_rv = ref__once(&ref_oc, test_init);
	ref_o = snap_once(ref_rv, ref_oc.state, ref_oc.mutex);

	__isthreaded = threaded;
	g_init_count = 0;
	g_pthread_once_calls = 0;
	g_pthread_once_rv = pthread_rv;
	g_pthread_once_arg = nullptr;
	g_pthread_once_init = nullptr;

	port_rv = port::_once(&port_oc, test_init);
	port_o = snap_once(port_rv, port_oc.state, port_oc.mutex);

	ncases[f]++;
	if (!once_obs_eq(ref_o, port_o)) {
		report(f, ctx, "observable mismatch");
		return (false);
	}
	return (true);
}

static void
test_once_handwritten(void)
{
	const int f = F_ONCE;

	(void)check_once_case(f, "single init", 0, 0, 0);
	(void)check_once_case(f, "already done", 0, PTHREAD_DONE_INIT, 0);
	(void)check_once_case(f, "state-1", 0, 1, 0);
	(void)check_once_case(f, "state-3", 0, 3, 0);
	(void)check_once_case(f, "threaded ok", 1, 0, 0);
	(void)check_once_case(f, "threaded done", 1, PTHREAD_DONE_INIT, 0);
	(void)check_once_case(f, "threaded fail", 1, 0, EAGAIN);
	(void)check_once_case(f, "threaded -1", 1, 0, -1);

	/* second call after init must not re-run routine */
	{
		ref_once_control ref_oc;
	port::pthread_once_t port_oc;
		int ref_rv, port_rv;
		once_obs ref_o, port_o;

		ref_oc.state = 0;
		ref_oc.mutex = nullptr;
		port_oc.state = 0;
		port_oc.mutex = nullptr;

		__isthreaded = 0;
		g_init_count = 0;
		g_pthread_once_calls = 0;
		(void)ref__once(&ref_oc, test_init);
		(void)ref__once(&ref_oc, test_init);
		ref_rv = 0;
		ref_o = snap_once(ref_rv, ref_oc.state, ref_oc.mutex);

		__isthreaded = 0;
		g_init_count = 0;
		g_pthread_once_calls = 0;
		(void)port::_once(&port_oc, test_init);
		(void)port::_once(&port_oc, test_init);
		port_rv = 0;
		port_o = snap_once(port_rv, port_oc.state, port_oc.mutex);

		ncases[f]++;
		if (!once_obs_eq(ref_o, port_o))
			report(f, "double-call", "observable mismatch");
	}
}

static void
test_once_random(void)
{
	const int f = F_ONCE;
	long i;

	for (i = 0; i < SWEEP_ITERS; i++) {
		char ctx[48];
		int threaded, state, pthread_rv;

		threaded = (rnd32() & 1u) ? 1 : 0;
		switch (rnd32() % 5) {
		case 0:
			state = 0;
			break;
		case 1:
			state = PTHREAD_DONE_INIT;
			break;
		case 2:
			state = 1;
			break;
		case 3:
			state = -1;
			break;
		default:
			state = (int)rnd32();
			break;
		}
		pthread_rv = (int)(rnd32() % 8u) - 2;

		std::snprintf(ctx, sizeof(ctx), "rand#%ld", i);
		(void)check_once_case(f, ctx, threaded, state, pthread_rv);
	}
}

/* --------------------------------------------------------- __sleep mock */

enum ns_script {
	NS_OK = 0,
	NS_EINTR,
	NS_EINVAL,
	NS_EAGAIN,
	NS_SCRIPT_COUNT
};

struct ns_mock {
	int		script;
	long		calls;
	time_t		last_sec;
	long		last_nsec;
	time_t		rem_sec;
	long		rem_nsec;
};

static ns_mock g_ns;

static void
ns_reset(int script, time_t rem_sec, long rem_nsec)
{
	std::memset(&g_ns, 0, sizeof(g_ns));
	g_ns.script = script;
	g_ns.rem_sec = rem_sec;
	g_ns.rem_nsec = rem_nsec;
}

extern "C" int
nanosleep(const struct timespec *req, struct timespec *rem)
{
	g_ns.calls++;
	if (req != nullptr) {
		g_ns.last_sec = req->tv_sec;
		g_ns.last_nsec = req->tv_nsec;
	}

	switch (g_ns.script) {
	case NS_OK:
		errno = 0;
		return (0);
	case NS_EINTR:
		errno = EINTR;
		if (rem != nullptr) {
			rem->tv_sec = g_ns.rem_sec;
			rem->tv_nsec = g_ns.rem_nsec;
		}
		return (-1);
	case NS_EINVAL:
		errno = EINVAL;
		return (-1);
	case NS_EAGAIN:
		errno = EAGAIN;
		return (-1);
	default:
		errno = EINVAL;
		return (-1);
	}
}

struct sleep_obs {
	unsigned int	rv;
	int		errno_copy;
	long		calls;
	time_t		last_sec;
	long		last_nsec;
};

static sleep_obs
snap_sleep(unsigned int rv)
{
	sleep_obs o;

	o.rv = rv;
	o.errno_copy = errno;
	o.calls = g_ns.calls;
	o.last_sec = g_ns.last_sec;
	o.last_nsec = g_ns.last_nsec;
	return (o);
}

static bool
sleep_obs_eq(const sleep_obs &a, const sleep_obs &b)
{
	return (a.rv == b.rv && a.errno_copy == b.errno_copy &&
	    a.calls == b.calls && a.last_sec == b.last_sec &&
	    a.last_nsec == b.last_nsec);
}

static bool
check_sleep_case(int f, const char *ctx, unsigned int seconds, int script,
    time_t rem_sec, long rem_nsec)
{
	unsigned int ref_rv, port_rv;
	sleep_obs ref_o, port_o;

	ns_reset(script, rem_sec, rem_nsec);
	errno = 0;
	ref_rv = ref___sleep(seconds);
	ref_o = snap_sleep(ref_rv);

	ns_reset(script, rem_sec, rem_nsec);
	errno = 0;
	port_rv = port::__sleep(seconds);
	port_o = snap_sleep(port_rv);

	ncases[f]++;
	if (!sleep_obs_eq(ref_o, port_o)) {
		report(f, ctx, "observable mismatch");
		return (false);
	}
	return (true);
}

static void
test_sleep_handwritten(void)
{
	const int f = F_SLEEP;

	(void)check_sleep_case(f, "zero ok", 0u, NS_OK, 0, 0);
	(void)check_sleep_case(f, "one ok", 1u, NS_OK, 0, 0);
	(void)check_sleep_case(f, "int_max ok", (unsigned int)INT_MAX, NS_OK, 0, 0);
	(void)check_sleep_case(f, "int_max+1 ok", (unsigned int)INT_MAX + 1u,
	    NS_OK, 0, 0);
	(void)check_sleep_case(f, "uint_max ok", 0xffffffffu, NS_OK, 0, 0);
	(void)check_sleep_case(f, "eintr sec only", 5u, NS_EINTR, 3, 0);
	(void)check_sleep_case(f, "eintr round up", 5u, NS_EINTR, 3, 1);
	(void)check_sleep_case(f, "eintr zero nsec", 7u, NS_EINTR, 0, 0);
	(void)check_sleep_case(f, "eintr zero sec+nsec", 7u, NS_EINTR, 0, 500);
	(void)check_sleep_case(f, "einval", 9u, NS_EINVAL, 0, 0);
	(void)check_sleep_case(f, "eagain", 11u, NS_EAGAIN, 0, 0);
	(void)check_sleep_case(f, "large+eintr", 100u, NS_EINTR, 42, 999999999L);
}

static void
test_sleep_random(void)
{
	const int f = F_SLEEP;
	long i;
	static const int scripts[] = {
		NS_OK, NS_EINTR, NS_EINVAL, NS_EAGAIN
	};

	for (i = 0; i < SWEEP_ITERS; i++) {
		char ctx[48];
		unsigned int seconds;
		int script;
		time_t rem_sec;
		long rem_nsec;

		switch (rnd32() % 10) {
		case 0:
			seconds = 0u;
			break;
		case 1:
			seconds = 1u;
			break;
		case 2:
			seconds = (unsigned int)INT_MAX;
			break;
		case 3:
			seconds = (unsigned int)INT_MAX + 1u;
			break;
		case 4:
			seconds = (unsigned int)INT_MAX + (unsigned int)(rnd32() % 1000u);
			break;
		case 5:
			seconds = 0xffffffffu;
			break;
		case 6:
			seconds = (unsigned int)(rnd32() % 10000u);
			break;
		default:
			seconds = rnd32();
			break;
		}

		script = scripts[rnd32() % (sizeof(scripts) / sizeof(scripts[0]))];
		rem_sec = (time_t)(rnd32() % 1000u);
		rem_nsec = (long)(rnd32() % 1000000000L);

		std::snprintf(ctx, sizeof(ctx), "rand#%ld", i);
		(void)check_sleep_case(f, ctx, seconds, script, rem_sec, rem_nsec);
	}
}

/* ------------------------------------------------------------------ main */

int
main(void)
{
	int i, failed = 0;

	test_isnan_handwritten();
	test_isnan_random();
	test_isnanf_handwritten();
	test_isnanf_random();
	test_once_handwritten();
	test_once_random();
	test_sleep_handwritten();
	test_sleep_random();

	std::printf("\n%-10s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-10s %12s %12s\n", "--------", "-----", "--------");
	for (i = 0; i < NFUNC; i++) {
		std::printf("%-10s %12llu %12llu\n", fname[i],
		    ncases[i], nfails[i]);
		if (nfails[i] != 0)
			failed = 1;
	}

	return (failed ? 1 : 0);
}
