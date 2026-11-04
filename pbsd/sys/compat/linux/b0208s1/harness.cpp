/*
 * PBSD batch b0208s1 differential harness: sys/compat/linux/linux_errno.c.
 *
 * Every exported function of the port is driven against the ref_ oracle in
 * oracle.c.  KASSERT predicates are recorded through hbsd_kassert_observe() so
 * linux_check_errtbl() -- otherwise a pure no-op -- is differentially
 * testable.  Invalid errno inputs that trip KASSERT are recovered via longjmp
 * from the panic() mock; both sides must panic on the same inputs after
 * recording identical assertion sequences.
 */

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <setjmp.h>
#include <vector>

import pbsd.sys.compat.linux.b0208s1;

namespace P = pbsd::sys_compat_linux::b0208s1;

extern "C" {

int ref_bsd_to_linux_errno(int error);
void ref_linux_check_errtbl(void);

}

/* ------------------------------------------------------------------ */
/* KASSERT / panic mocks                                               */
/* ------------------------------------------------------------------ */

static std::vector<int> g_kassert_log;
static jmp_buf g_panic_jmp;

extern "C" void
hbsd_kassert_observe(int cond)
{
	g_kassert_log.push_back(cond);
}

extern "C" void
panic(const char *fmt, ...)
{
	(void)fmt;
	longjmp(g_panic_jmp, 1);
}

static void
kassert_reset(void)
{
	g_kassert_log.clear();
}

/* ------------------------------------------------------------------ */
/* Bookkeeping                                                         */
/* ------------------------------------------------------------------ */

enum {
	S_BSD_TO_LINUX_ERRNO,
	S_LINUX_CHECK_ERRTBL,
	S_NSTATS
};

struct Stat {
	const char *name;
	long long cases;
	long long fails;
};

static Stat stats[S_NSTATS] = {
	{ "bsd_to_linux_errno", 0, 0 },
	{ "linux_check_errtbl", 0, 0 },
};

static void
report(int which, const char *fmt, ...)
{
	Stat &s = stats[which];

	s.fails++;
	if (s.fails <= 10) {
		va_list ap;

		std::fprintf(stderr, "FAIL %s: ", s.name);
		va_start(ap, fmt);
		std::vfprintf(stderr, fmt, ap);
		va_end(ap);
		std::fputc('\n', stderr);
	}
}

static bool
kassert_eq(const std::vector<int> &a, const std::vector<int> &b)
{
	if (a.size() != b.size())
		return false;
	for (std::size_t i = 0; i < a.size(); i++) {
		if (a[i] != b[i])
			return false;
	}
	return true;
}

/* ------------------------------------------------------------------ */
/* bsd_to_linux_errno                                                  */
/* ------------------------------------------------------------------ */

static const int ELAST = 97;

struct ErrnoCall {
	int retval;
	bool panicked;
	std::vector<int> kasserts;
};

static ErrnoCall
call_ref_errno(int error)
{
	ErrnoCall r{};
	int got;

	kassert_reset();
	if (setjmp(g_panic_jmp) == 0) {
		got = ref_bsd_to_linux_errno(error);
		r.retval = got;
		r.panicked = false;
	} else {
		r.panicked = true;
	}
	r.kasserts = g_kassert_log;
	return r;
}

static ErrnoCall
call_port_errno(int error)
{
	ErrnoCall r{};
	int got;

	kassert_reset();
	if (setjmp(g_panic_jmp) == 0) {
		got = P::bsd_to_linux_errno(error);
		r.retval = got;
		r.panicked = false;
	} else {
		r.panicked = true;
	}
	r.kasserts = g_kassert_log;
	return r;
}

static void
test_errno_one(int which, int error, const char *tag)
{
	stats[which].cases++;

	const ErrnoCall ref = call_ref_errno(error);
	const ErrnoCall port = call_port_errno(error);

	if (ref.panicked != port.panicked) {
		report(which, "%s error=%d panicked ref=%d port=%d",
		    tag, error, ref.panicked, port.panicked);
		return;
	}
	if (!kassert_eq(ref.kasserts, port.kasserts)) {
		report(which, "%s error=%d kassert mismatch (ref %zu port %zu)",
		    tag, error, ref.kasserts.size(), port.kasserts.size());
		return;
	}
	if (!ref.panicked && ref.retval != port.retval) {
		report(which, "%s error=%d retval ref=%d port=%d",
		    tag, error, ref.retval, port.retval);
	}
}

static void
test_bsd_to_linux_errno_edges(void)
{
	const int which = S_BSD_TO_LINUX_ERRNO;

	/* Every valid table index. */
	for (int i = 0; i <= ELAST; i++)
		test_errno_one(which, i, "edge");

	/* KASSERT lower boundary: error >= 0 */
	test_errno_one(which, -1, "edge");
	test_errno_one(which, -2, "edge");
	test_errno_one(which, -97, "edge");
	test_errno_one(which, -98, "edge");
	test_errno_one(which, -2147483647 - 1, "edge");

	/* KASSERT upper boundary: error <= ELAST */
	test_errno_one(which, ELAST + 1, "edge");
	test_errno_one(which, ELAST + 2, "edge");
	test_errno_one(which, 98, "edge");
	test_errno_one(which, 99, "edge");
	test_errno_one(which, 133, "edge");
	test_errno_one(which, 512, "edge");
	test_errno_one(which, 2147483647, "edge");

	/* High-bit int patterns (signed comparison load-bearing). */
	test_errno_one(which, static_cast<int>(0x80000000u), "edge");
	test_errno_one(which, static_cast<int>(0xFFFFFFFFu), "edge");
	test_errno_one(which, static_cast<int>(0xFFFFFF9Bu), "edge"); /* -101 */
	test_errno_one(which, static_cast<int>(0xFFFFFF61u), "edge"); /* -159 */
}

/* Fixed-seed xorshift32 PRNG (seed 0xb0208s1). */
static std::uint32_t
rng_u32(std::uint32_t &state)
{
	state ^= state << 13;
	state ^= state >> 17;
	state ^= state << 5;
	return state;
}

static int
rng_errno(std::uint32_t &state)
{
	const std::uint32_t r = rng_u32(state);

	/*
	 * Mix valid [0, ELAST], one-past-valid, and large invalid values so
	 * every comparison arm in KASSERT and every table slot is exercised.
	 */
	switch (r % 7) {
	case 0:
		return static_cast<int>(r % (ELAST + 1));
	case 1:
		return static_cast<int>(r % (ELAST + 2)); /* includes ELAST+1 */
	case 2:
		return -static_cast<int>(r % 256);
	case 3:
		return static_cast<int>(r);
	case 4:
		return static_cast<int>(r | 0x80000000u);
	case 5:
		return ELAST;
	case 6:
		return 0;
	default:
		return 0;
	}
}

static void
test_bsd_to_linux_errno_random(void)
{
	const int which = S_BSD_TO_LINUX_ERRNO;
	std::uint32_t state = 0x0208b001u;

	for (int n = 0; n < 200000; n++) {
		const int error = rng_errno(state);
		test_errno_one(which, error, "rand");
	}
}

/* ------------------------------------------------------------------ */
/* linux_check_errtbl                                                  */
/* ------------------------------------------------------------------ */

struct CheckCall {
	bool panicked;
	std::vector<int> kasserts;
};

static CheckCall
call_ref_check(void)
{
	CheckCall r{};

	kassert_reset();
	if (setjmp(g_panic_jmp) == 0) {
		ref_linux_check_errtbl();
		r.panicked = false;
	} else {
		r.panicked = true;
	}
	r.kasserts = g_kassert_log;
	return r;
}

static CheckCall
call_port_check(void)
{
	CheckCall r{};

	kassert_reset();
	if (setjmp(g_panic_jmp) == 0) {
		P::linux_check_errtbl();
		r.panicked = false;
	} else {
		r.panicked = true;
	}
	r.kasserts = g_kassert_log;
	return r;
}

static void
test_check_one(int which, const char *tag)
{
	stats[which].cases++;

	const CheckCall ref = call_ref_check();
	const CheckCall port = call_port_check();

	if (ref.panicked != port.panicked) {
		report(which, "%s panicked ref=%d port=%d",
		    tag, ref.panicked, port.panicked);
		return;
	}
	if (!kassert_eq(ref.kasserts, port.kasserts)) {
		report(which, "%s kassert mismatch (ref %zu port %zu)",
		    tag, ref.kasserts.size(), port.kasserts.size());
	}
}

static void
test_linux_check_errtbl_edges(void)
{
	const int which = S_LINUX_CHECK_ERRTBL;

	test_check_one(which, "edge");
	test_check_one(which, "edge");
	test_check_one(which, "edge");
}

static void
test_linux_check_errtbl_random(void)
{
	const int which = S_LINUX_CHECK_ERRTBL;

	for (int n = 0; n < 200000; n++)
		test_check_one(which, "rand");
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int
main(void)
{
	test_bsd_to_linux_errno_edges();
	test_bsd_to_linux_errno_random();
	test_linux_check_errtbl_edges();
	test_linux_check_errtbl_random();

	int failed = 0;
	std::printf("%-24s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < S_NSTATS; i++) {
		std::printf("%-24s %12lld %12lld\n",
		    stats[i].name, stats[i].cases, stats[i].fails);
		if (stats[i].fails != 0)
			failed = 1;
	}

	return failed;
}
