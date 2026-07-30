/*
 * harness.cpp -- differential test for PBSD batch b0020s3.
 *
 * Compares pbsd::lib_libc_sys::b0020s3::sigwait() against the unmodified
 * reference ref_sigwait() from oracle.c.
 *
 * The only observable behaviour of sigwait.c is the indirect dispatch through
 * libc's interposing table: the table slot that is selected, the arguments
 * that reach it unchanged, the value that comes back, and the absence of any
 * other memory effect.  So the harness owns the table (as libc does, in
 * interposing_table.c), fills EVERY slot with a distinct recording target and
 * checks, for both implementations:
 *
 *   - the returned int;
 *   - errno afterwards;
 *   - how many times a slot was entered;
 *   - which slot was entered;
 *   - the classification/offset of both pointer arguments as they arrived;
 *   - the ENTIRE guard-filled buffer holding the sigset_t and the int, past
 *     the nominal write window, byte for byte.
 *
 * Two separate buffers are used, one per implementation, so pointers are only
 * ever compared as offsets from their own base -- never as raw addresses.
 * The table is also driven with the host's real sigwait() so the live success
 * path and the live EFAULT error path are exercised end to end.
 */

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <unistd.h>

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <utility>

import pbsd.lib.libc.sys.b0020s3;

/* ------------------------------------------------------------------ */
/* the interposing table (lib/libc/gen/interposing_table.c)            */
/* ------------------------------------------------------------------ */

/*
 * Deliberately an independent copy of the enum: if a planted bug perturbs the
 * port's own copy, the harness still knows which slot the real sigwait.c must
 * reach.
 */
enum {
	H_INTERPOS_accept,
	H_INTERPOS_accept4,
	H_INTERPOS_aio_suspend,
	H_INTERPOS_close,
	H_INTERPOS_connect,
	H_INTERPOS_fcntl,
	H_INTERPOS_fsync,
	H_INTERPOS_fork,
	H_INTERPOS_msync,
	H_INTERPOS_nanosleep,
	H_INTERPOS_openat,
	H_INTERPOS_poll,
	H_INTERPOS_pselect,
	H_INTERPOS_recvfrom,
	H_INTERPOS_recvmsg,
	H_INTERPOS_select,
	H_INTERPOS_sendmsg,
	H_INTERPOS_sendto,
	H_INTERPOS_setcontext,
	H_INTERPOS_sigaction,
	H_INTERPOS_sigprocmask,
	H_INTERPOS_sigsuspend,
	H_INTERPOS_sigwait,
	H_INTERPOS_sigtimedwait,
	H_INTERPOS_sigwaitinfo,
	H_INTERPOS_swapcontext,
	H_INTERPOS_system,
	H_INTERPOS_tcdrain,
	H_INTERPOS_read,
	H_INTERPOS_readv,
	H_INTERPOS_wait4,
	H_INTERPOS_write,
	H_INTERPOS_writev,
	H_INTERPOS__pthread_mutex_init_calloc_cb,
	H_INTERPOS_spinlock,
	H_INTERPOS_wait6,
	H_INTERPOS_kevent,
	H_INTERPOS_wait,
	H_INTERPOS_pdfork,
	H_INTERPOS_MAX
};

/* Slack past the last real slot, so that a bug which indexes off the end of
 * the table lands on a recognisable trap instead of on whatever the linker
 * happened to place next to it. */
#define	H_INTERPOS_SLACK	64

extern "C" {
typedef int (*interpos_func_t)(void);

int trap_target(const sigset_t *set, int *sig);
interpos_func_t __libc_interposing[H_INTERPOS_MAX + H_INTERPOS_SLACK];
int ref_sigwait(const sigset_t *set, int *sig);
}

using sigwait_func_t = int (*)(const sigset_t *, int *);

/* ------------------------------------------------------------------ */
/* recording targets                                                   */
/* ------------------------------------------------------------------ */

namespace {

const sigset_t *const SET_UNSET = reinterpret_cast<const sigset_t *>(-1);
int *const SIG_UNSET = reinterpret_cast<int *>(-1);

struct Record {
	int calls;
	int slot;
	const sigset_t *set;
	int *sig;
};

Record g_rec;
int g_ret_base;
int g_write_val;
bool g_do_write;

void
reset_record(void)
{
	g_rec.calls = 0;
	g_rec.slot = -1;
	g_rec.set = SET_UNSET;
	g_rec.sig = SIG_UNSET;
}

void
record(int slot, const sigset_t *set, int *sig)
{
	g_rec.calls++;
	g_rec.slot = slot;
	g_rec.set = set;
	g_rec.sig = sig;
}

/*
 * Slot N's answer -- and the value it stores through sig -- both depend on N,
 * so dispatching to the wrong slot shows up in the return value AND in the
 * buffer, not just in g_rec.slot.
 */
template <int N>
int
slot_target(const sigset_t *set, int *sig)
{
	record(N, set, sig);
	if (g_do_write && sig != nullptr)
		*sig = static_cast<int>(static_cast<unsigned>(g_write_val) +
		    static_cast<unsigned>(N) * 7u + 1u);
	return static_cast<int>(static_cast<unsigned>(g_ret_base) +
	    static_cast<unsigned>(N) * 131u + 1u);
}

template <std::size_t... Is>
void
install_all(std::index_sequence<Is...>)
{
	((__libc_interposing[Is] = reinterpret_cast<interpos_func_t>(
	    &slot_target<static_cast<int>(Is)>)), ...);
}

void
install_recorders(void)
{
	std::size_t i;

	for (i = H_INTERPOS_MAX; i < H_INTERPOS_MAX + H_INTERPOS_SLACK; i++)
		__libc_interposing[i] =
		    reinterpret_cast<interpos_func_t>(&trap_target);
	install_all(std::make_index_sequence<H_INTERPOS_MAX>{});
}

/* ------------------------------------------------------------------ */
/* guarded buffers                                                     */
/* ------------------------------------------------------------------ */

constexpr unsigned char GUARD = 0x7f;
constexpr std::size_t SS = sizeof(sigset_t);
constexpr std::size_t BUFSZ = SS * 2 + 64;

alignas(16) unsigned char g_bufA[BUFSZ];
alignas(16) unsigned char g_bufB[BUFSZ];

struct Args {
	bool have_set;
	bool have_sig;
	std::size_t set_off;
	std::size_t sig_off;
	unsigned char set_fill[SS];
	int sig_init;
	int ret_base;
	bool do_write;
	int write_val;
	/*
	 * When set, the pointer handed over is this raw value instead of a
	 * pointer into the buffer.  Never dereferenced by the recorders.
	 */
	const void *odd_set;
	const void *odd_sig;
	/* When set, the real sigwait() occupies the sigwait slot. */
	bool live;
	int live_raise;
};

Args
default_args(void)
{
	Args a;
	a.have_set = true;
	a.have_sig = true;
	a.set_off = 0;
	a.sig_off = SS;
	std::memset(a.set_fill, 0, sizeof(a.set_fill));
	a.sig_init = 0;
	a.ret_base = 0;
	a.do_write = true;
	a.write_val = 0;
	a.odd_set = nullptr;
	a.odd_sig = nullptr;
	a.live = false;
	a.live_raise = 0;
	return a;
}

void
lay_out(unsigned char *buf, const Args &a)
{
	std::memset(buf, GUARD, BUFSZ);
	if (a.have_set)
		std::memcpy(buf + a.set_off, a.set_fill, SS);
	if (a.have_sig)
		std::memcpy(buf + a.sig_off, &a.sig_init, sizeof(int));
}

/*
 * A pointer is compared as (class, value): NULL, an offset inside its own
 * buffer, or -- only for the deliberately bogus pointers -- the raw value.
 */
struct PtrId {
	int cls; /* 0 = null, 1 = inside buffer, 2 = outside */
	std::intptr_t val;
};

PtrId
identify(const void *p, const unsigned char *base)
{
	PtrId id;

	if (p == nullptr) {
		id.cls = 0;
		id.val = 0;
		return id;
	}
	const unsigned char *q = static_cast<const unsigned char *>(p);
	if (q >= base && q < base + BUFSZ) {
		id.cls = 1;
		id.val = q - base;
		return id;
	}
	id.cls = 2;
	id.val = reinterpret_cast<std::intptr_t>(p);
	return id;
}

bool
same(const PtrId &x, const PtrId &y)
{
	return (x.cls == y.cls && x.val == y.val);
}

/* ------------------------------------------------------------------ */
/* the comparison                                                      */
/* ------------------------------------------------------------------ */

struct Tally {
	const char *name;
	long cases;
	long failures;
};

Tally t_edge = { "sigwait/edge", 0, 0 };
Tally t_live = { "sigwait/live", 0, 0 };
Tally t_rand = { "sigwait/random", 0, 0 };

int g_printed;

/*
 * A bug in the port can leave a live sigwait() waiting for a signal that will
 * never arrive.  The watchdog turns that into a failure instead of a hang.
 * SIGALRM stays unblocked and is never a member of any wait set used here, so
 * it is delivered to this handler rather than dequeued by sigwait().
 */
void
watchdog_expired(int)
{
	static const char msg[] = "\nFAIL: watchdog expired -- a call never "
	    "returned\nRESULT: FAIL\n";

	ssize_t n = write(STDERR_FILENO, msg, sizeof(msg) - 1);

	(void)n;
	_exit(1);
}

void
signal_setup(void)
{
	struct sigaction sa;
	sigset_t s;

	sigemptyset(&s);
	sigaddset(&s, SIGUSR1);
	sigaddset(&s, SIGUSR2);
	sigaddset(&s, SIGWINCH);
	sigprocmask(SIG_BLOCK, &s, nullptr);

	std::memset(&sa, 0, sizeof(sa));
	sa.sa_handler = watchdog_expired;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGALRM, &sa, nullptr);

	sigemptyset(&s);
	sigaddset(&s, SIGALRM);
	sigprocmask(SIG_UNBLOCK, &s, nullptr);

	/* Generous: the whole run is well under a second of CPU. */
	alarm(60);
}

void
fail(Tally &t, const char *what, const char *detail)
{
	t.failures++;
	if (g_printed < 20) {
		g_printed++;
		std::printf("FAIL [%s] case %ld: %s%s%s\n", t.name, t.cases,
		    what, detail != nullptr ? " -- " : "",
		    detail != nullptr ? detail : "");
	}
}

void
run_case(Tally &t, const Args &a)
{
	char buf[256];

	t.cases++;

	lay_out(g_bufA, a);
	lay_out(g_bufB, a);

	const sigset_t *setA = nullptr;
	const sigset_t *setB = nullptr;
	if (a.odd_set != nullptr) {
		setA = static_cast<const sigset_t *>(a.odd_set);
		setB = setA;
	} else if (a.have_set) {
		setA = reinterpret_cast<const sigset_t *>(g_bufA + a.set_off);
		setB = reinterpret_cast<const sigset_t *>(g_bufB + a.set_off);
	}

	int *sigA = nullptr;
	int *sigB = nullptr;
	if (a.odd_sig != nullptr) {
		sigA = const_cast<int *>(static_cast<const int *>(a.odd_sig));
		sigB = sigA;
	} else if (a.have_sig) {
		sigA = reinterpret_cast<int *>(g_bufA + a.sig_off);
		sigB = reinterpret_cast<int *>(g_bufB + a.sig_off);
	}

	g_ret_base = a.ret_base;
	g_write_val = a.write_val;
	g_do_write = a.do_write;

	install_recorders();
	if (a.live)
		__libc_interposing[H_INTERPOS_sigwait] =
		    reinterpret_cast<interpos_func_t>(
		    static_cast<sigwait_func_t>(&::sigwait));

	reset_record();
	if (a.live && a.live_raise != 0)
		raise(a.live_raise);
	errno = 0;
	int rp = pbsd::lib_libc_sys::b0020s3::sigwait(setA, sigA);
	int ep = errno;
	Record recP = g_rec;

	reset_record();
	if (a.live && a.live_raise != 0)
		raise(a.live_raise);
	errno = 0;
	int rr = ref_sigwait(setB, sigB);
	int er = errno;
	Record recR = g_rec;

	if (rp != rr) {
		std::snprintf(buf, sizeof(buf), "port=%d ref=%d", rp, rr);
		fail(t, "return value differs", buf);
	}
	if (ep != er) {
		std::snprintf(buf, sizeof(buf), "port=%d ref=%d", ep, er);
		fail(t, "errno differs", buf);
	}

	if (a.live) {
		/*
		 * The real sigwait() does not record, so a mutant that skips
		 * the sigwait slot lands on a recorder or on trap_target and
		 * is caught here as well as by the return value.
		 */
		if (recP.calls != recR.calls) {
			std::snprintf(buf, sizeof(buf), "port=%d ref=%d",
			    recP.calls, recR.calls);
			fail(t, "live dispatch count differs", buf);
		}
		if (recP.calls != 0) {
			std::snprintf(buf, sizeof(buf), "slot=%d", recP.slot);
			fail(t, "port bypassed the real sigwait", buf);
		}
	} else {
		if (recP.calls != recR.calls) {
			std::snprintf(buf, sizeof(buf), "port=%d ref=%d",
			    recP.calls, recR.calls);
			fail(t, "dispatch count differs", buf);
		}
		if (recP.slot != recR.slot) {
			std::snprintf(buf, sizeof(buf), "port=%d ref=%d",
			    recP.slot, recR.slot);
			fail(t, "dispatched slot differs", buf);
		}
		if (recP.calls != 1 || recP.slot != H_INTERPOS_sigwait) {
			std::snprintf(buf, sizeof(buf), "calls=%d slot=%d",
			    recP.calls, recP.slot);
			fail(t, "port did not dispatch via INTERPOS_sigwait",
			    buf);
		}
		if (recR.calls != 1 || recR.slot != H_INTERPOS_sigwait) {
			std::snprintf(buf, sizeof(buf), "calls=%d slot=%d",
			    recR.calls, recR.slot);
			fail(t, "ref did not dispatch via INTERPOS_sigwait",
			    buf);
		}
		PtrId sP = identify(recP.set, g_bufA);
		PtrId sR = identify(recR.set, g_bufB);
		if (!same(sP, sR)) {
			std::snprintf(buf, sizeof(buf),
			    "port=(%d,%ld) ref=(%d,%ld)", sP.cls,
			    static_cast<long>(sP.val), sR.cls,
			    static_cast<long>(sR.val));
			fail(t, "set argument differs", buf);
		}
		PtrId gP = identify(recP.sig, g_bufA);
		PtrId gR = identify(recR.sig, g_bufB);
		if (!same(gP, gR)) {
			std::snprintf(buf, sizeof(buf),
			    "port=(%d,%ld) ref=(%d,%ld)", gP.cls,
			    static_cast<long>(gP.val), gR.cls,
			    static_cast<long>(gR.val));
			fail(t, "sig argument differs", buf);
		}
		/*
		 * The arguments must arrive exactly where they were sent,
		 * measured against each implementation's own buffer.
		 */
		if (!same(sP, identify(setA, g_bufA)))
			fail(t, "port perturbed the set argument", nullptr);
		if (!same(gP, identify(sigA, g_bufA)))
			fail(t, "port perturbed the sig argument", nullptr);
		if (!same(sR, identify(setB, g_bufB)))
			fail(t, "ref perturbed the set argument", nullptr);
		if (!same(gR, identify(sigB, g_bufB)))
			fail(t, "ref perturbed the sig argument", nullptr);
	}

	if (std::memcmp(g_bufA, g_bufB, BUFSZ) != 0) {
		std::size_t i = 0;
		while (i < BUFSZ && g_bufA[i] == g_bufB[i])
			i++;
		std::snprintf(buf, sizeof(buf),
		    "first difference at byte %zu: port=0x%02x ref=0x%02x", i,
		    g_bufA[i], g_bufB[i]);
		fail(t, "buffer differs", buf);
	}
}

/* ------------------------------------------------------------------ */
/* edge cases                                                          */
/* ------------------------------------------------------------------ */

struct Layout {
	bool have_set;
	bool have_sig;
	std::size_t set_off;
	std::size_t sig_off;
};

const Layout LAYOUTS[] = {
	{ true, true, 0, SS },			/* set at buffer start */
	{ true, true, BUFSZ - SS, 0 },		/* set at end, sig at start */
	{ true, true, 8, 0 },			/* sig at start, set adjacent */
	{ true, true, 0, BUFSZ - sizeof(int) },	/* sig at the very end */
	{ true, true, SS, SS * 2 },		/* both in the middle */
	{ false, true, 0, 0 },			/* no set, sig at start */
	{ false, true, 0, BUFSZ - sizeof(int) },/* no set, sig at end */
	{ true, false, 0, 0 },			/* no sig, set at start */
	{ true, false, BUFSZ - SS, 0 },		/* no sig, set at end */
	{ false, false, 0, 0 },			/* both null */
};
constexpr std::size_t NLAYOUT = sizeof(LAYOUTS) / sizeof(LAYOUTS[0]);

const int RETS[] = { 0, 1, -1, 2, EINVAL, EFAULT, EINTR, INT_MAX, INT_MIN,
	0x7f7f7f7f, static_cast<int>(0x80000001u), 255, 256 };
constexpr std::size_t NRET = sizeof(RETS) / sizeof(RETS[0]);

const int SIGINITS[] = { 0, 1, -1, 31, 32, 63, 64, 65, SIGUSR1, INT_MAX,
	INT_MIN, 0x7f7f7f7f, static_cast<int>(0x80808080u),
	static_cast<int>(0xffffffffu) };
constexpr std::size_t NSIGINIT = sizeof(SIGINITS) / sizeof(SIGINITS[0]);

const int WRITES[] = { 0, 1, -1, SIGUSR1, SIGUSR2, 64, INT_MAX, INT_MIN,
	0x7f7f7f7f, static_cast<int>(0x80000000u) };
constexpr std::size_t NWRITE = sizeof(WRITES) / sizeof(WRITES[0]);

constexpr int NPATTERN = 12;

void
apply_pattern(unsigned char *p, std::size_t n, int which)
{
	std::size_t i;

	switch (which) {
	case 0:
		std::memset(p, 0x00, n);
		break;
	case 1:
		std::memset(p, 0xff, n);
		break;
	case 2:
		std::memset(p, 0x80, n);
		break;
	case 3:
		std::memset(p, GUARD, n);
		break;
	case 4:
		std::memset(p, 0x01, n);
		break;
	case 5:
		for (i = 0; i < n; i++)
			p[i] = (i % 2 == 0) ? 0x80 : 0x00;
		break;
	case 6:
		for (i = 0; i < n; i++)
			p[i] = (i % 2 == 0) ? 0xff : GUARD;
		break;
	case 7:
		for (i = 0; i < n; i++)
			p[i] = static_cast<unsigned char>(i & 0xff);
		break;
	case 8:
		std::memset(p, 0x00, n);
		p[0] = 0x80;
		break;
	case 9:
		std::memset(p, 0x00, n);
		p[n - 1] = 0x80;
		break;
	case 10:
		for (i = 0; i < n; i++)
			p[i] = static_cast<unsigned char>(0x80 + (i & 0x7f));
		break;
	default:
		std::memset(p, 0x5a, n);
		break;
	}
}

void
edge_cases(void)
{
	std::size_t li;
	std::size_t ri;
	std::size_t si;
	int w;
	int pi;
	int k;

	/* layouts x return values x write/no-write */
	for (li = 0; li < NLAYOUT; li++) {
		for (ri = 0; ri < NRET; ri++) {
			for (w = 0; w < 2; w++) {
				Args a = default_args();
				a.have_set = LAYOUTS[li].have_set;
				a.have_sig = LAYOUTS[li].have_sig;
				a.set_off = LAYOUTS[li].set_off;
				a.sig_off = LAYOUTS[li].sig_off;
				apply_pattern(a.set_fill, SS,
				    static_cast<int>((li + ri) % NPATTERN));
				a.sig_init = SIGINITS[(li + ri) % NSIGINIT];
				a.ret_base = RETS[ri];
				a.do_write = (w != 0);
				a.write_val = WRITES[(ri + li) % NWRITE];
				run_case(t_edge, a);
			}
		}
	}

	/* every sigset_t pattern x every initial *sig value */
	for (pi = 0; pi < NPATTERN; pi++) {
		for (si = 0; si < NSIGINIT; si++) {
			Args a = default_args();
			apply_pattern(a.set_fill, SS, pi);
			a.sig_init = SIGINITS[si];
			a.ret_base = RETS[(static_cast<std::size_t>(pi) + si) %
			    NRET];
			a.do_write = ((static_cast<std::size_t>(pi) + si) %
			    2) == 0;
			a.write_val = WRITES[si % NWRITE];
			run_case(t_edge, a);
		}
	}

	/* every write value, at the two extreme sig offsets */
	for (w = 0; w < static_cast<int>(NWRITE); w++) {
		Args a = default_args();
		a.set_off = SS;
		a.sig_off = 0;
		apply_pattern(a.set_fill, SS, w % NPATTERN);
		a.sig_init = 0x7f7f7f7f;
		a.ret_base = RETS[static_cast<std::size_t>(w) % NRET];
		a.write_val = WRITES[w];
		run_case(t_edge, a);
		a.set_off = 0;
		a.sig_off = BUFSZ - sizeof(int);
		run_case(t_edge, a);
	}

	/* pointers that are not buffer pointers, passed straight through */
	{
		Args a = default_args();
		a.do_write = false;
		a.odd_set = reinterpret_cast<const void *>(
		    static_cast<std::uintptr_t>(0x1));
		run_case(t_edge, a);
		a.odd_set = reinterpret_cast<const void *>(UINTPTR_MAX - 7);
		run_case(t_edge, a);
		a.odd_set = nullptr;
		a.have_set = false;
		a.odd_sig = reinterpret_cast<const void *>(
		    static_cast<std::uintptr_t>(0xdeadbee0u));
		run_case(t_edge, a);
		a.have_set = true;
		run_case(t_edge, a);
		a.odd_set = reinterpret_cast<const void *>(
		    static_cast<std::uintptr_t>(0x1234));
		run_case(t_edge, a);
	}

	/* repeated dispatch must stay stateless */
	for (k = 0; k < 8; k++) {
		Args a = default_args();
		a.ret_base = RETS[static_cast<std::size_t>(k) % NRET];
		a.write_val = WRITES[static_cast<std::size_t>(k) % NWRITE];
		a.sig_init = SIGINITS[static_cast<std::size_t>(k) % NSIGINIT];
		apply_pattern(a.set_fill, SS, k % NPATTERN);
		run_case(t_edge, a);
	}
}

/* ------------------------------------------------------------------ */
/* live cases: the host's real sigwait() in the slot                   */
/* ------------------------------------------------------------------ */

void
live_cases(void)
{
	sigset_t want;

	/* success: a blocked signal is already pending */
	{
		Args a = default_args();
		a.live = true;
		a.live_raise = SIGUSR1;
		a.set_off = 0;
		a.sig_off = SS;
		sigemptyset(&want);
		sigaddset(&want, SIGUSR1);
		std::memcpy(a.set_fill, &want, SS);
		a.sig_init = 0x7f7f7f7f;
		run_case(t_live, a);

		a.set_off = BUFSZ - SS;
		a.sig_off = 0;
		a.live_raise = SIGUSR2;
		sigemptyset(&want);
		sigaddset(&want, SIGUSR2);
		std::memcpy(a.set_fill, &want, SS);
		run_case(t_live, a);

		a.set_off = 0;
		a.sig_off = BUFSZ - sizeof(int);
		a.live_raise = SIGWINCH;
		sigemptyset(&want);
		sigaddset(&want, SIGUSR1);
		sigaddset(&want, SIGUSR2);
		sigaddset(&want, SIGWINCH);
		std::memcpy(a.set_fill, &want, SS);
		run_case(t_live, a);
	}

	/* error: the kernel rejects an unreadable set, *sig stays untouched */
	{
		Args a = default_args();
		a.live = true;
		a.live_raise = 0;
		a.have_set = false;
		a.odd_set = reinterpret_cast<const void *>(
		    static_cast<std::uintptr_t>(0x8));
		a.sig_off = SS;
		a.sig_init = 0x7f7f7f7f;
		run_case(t_live, a);
	}
}

/* ------------------------------------------------------------------ */
/* randomised sweep                                                    */
/* ------------------------------------------------------------------ */

std::uint64_t g_state;

std::uint64_t
next64(void)
{
	g_state += 0x9e3779b97f4a7c15ull;
	std::uint64_t z = g_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
	return (z ^ (z >> 31));
}

std::uint32_t
below(std::uint64_t n)
{
	return static_cast<std::uint32_t>(next64() % n);
}

void
random_cases(long iters)
{
	long i;

	g_state = 0x0020530000b0020ull;

	for (i = 0; i < iters; i++) {
		Args a = default_args();

		a.have_set = below(8) != 0;
		a.have_sig = below(8) != 0;
		a.do_write = below(3) != 0;

		/* place the objects, keeping them disjoint */
		bool sig_first = below(2) != 0;
		if (a.have_set) {
			if (a.have_sig && sig_first) {
				a.set_off = 8 + 8 * below((BUFSZ - SS) / 8);
				a.sig_off = 4 * below(a.set_off / 4);
			} else if (a.have_sig) {
				a.set_off = 8 * below((BUFSZ - SS -
				    sizeof(int)) / 8 + 1);
				std::size_t lo = a.set_off + SS;
				a.sig_off = lo + 4 * below((BUFSZ -
				    sizeof(int) - lo) / 4 + 1);
			} else {
				a.set_off = 8 * below((BUFSZ - SS) / 8 + 1);
			}
		} else if (a.have_sig) {
			a.sig_off = 4 * below((BUFSZ - sizeof(int)) / 4 + 1);
		}

		/* contents: mostly structured patterns, sometimes pure noise */
		if (below(4) != 0) {
			apply_pattern(a.set_fill, SS,
			    static_cast<int>(below(NPATTERN)));
		} else {
			std::size_t k;
			for (k = 0; k < SS; k++)
				a.set_fill[k] = static_cast<unsigned char>(
				    next64() & 0xff);
		}

		switch (below(4)) {
		case 0:
			a.sig_init = SIGINITS[below(NSIGINIT)];
			break;
		case 1:
			a.sig_init = static_cast<int>(
			    static_cast<std::uint32_t>(next64()));
			break;
		case 2:
			a.sig_init = static_cast<int>(below(70)) - 4;
			break;
		default:
			a.sig_init = 0x7f7f7f7f;
			break;
		}

		switch (below(3)) {
		case 0:
			a.ret_base = RETS[below(NRET)];
			break;
		case 1:
			a.ret_base = static_cast<int>(
			    static_cast<std::uint32_t>(next64()));
			break;
		default:
			a.ret_base = static_cast<int>(below(300)) - 150;
			break;
		}

		switch (below(3)) {
		case 0:
			a.write_val = WRITES[below(NWRITE)];
			break;
		case 1:
			a.write_val = static_cast<int>(
			    static_cast<std::uint32_t>(next64()));
			break;
		default:
			a.write_val = static_cast<int>(below(70)) - 4;
			break;
		}

		/* occasionally hand over pointers that are not ours */
		if (below(64) == 0) {
			a.have_set = false;
			a.odd_set = reinterpret_cast<const void *>(
			    static_cast<std::uintptr_t>(next64() | 1u));
		}
		if (!a.do_write && below(64) == 0) {
			a.have_sig = false;
			a.odd_sig = reinterpret_cast<const void *>(
			    static_cast<std::uintptr_t>(next64() | 1u));
		}

		run_case(t_rand, a);
	}
}

void
print_row(const char *label, long cases, long failures)
{
	std::printf("  %-24s %10ld %10ld\n", label, cases, failures);
}

} // namespace

/*
 * Reached only by a bug that indexes past the end of the interposing table.
 * It never touches sig, so both the recorded slot and the untouched guard
 * bytes give it away.
 */
extern "C" int
trap_target(const sigset_t *set, int *sig)
{
	record(-2, set, sig);
	return (0x5eadbeef);
}

int
main(void)
{
	signal_setup();

	std::printf("=== PBSD b0020s3 differential test ===\n");
	std::printf("sigset_t=%zu bytes, guarded buffer=%zu bytes, "
	    "interposing slots=%d\n\n",
	    SS, BUFSZ, static_cast<int>(H_INTERPOS_MAX));

	edge_cases();
	live_cases();
	random_cases(250000);

	long cases = t_edge.cases + t_live.cases + t_rand.cases;
	long failures = t_edge.failures + t_live.failures + t_rand.failures;

	std::printf("\n  %-24s %10s %10s\n", "section", "cases", "failures");
	std::printf("  ------------------------------------------------\n");
	print_row(t_edge.name, t_edge.cases, t_edge.failures);
	print_row(t_live.name, t_live.cases, t_live.failures);
	print_row(t_rand.name, t_rand.cases, t_rand.failures);

	std::printf("\n  %-24s %10s %10s\n", "function", "cases", "failures");
	std::printf("  ------------------------------------------------\n");
	print_row("sigwait", cases, failures);
	std::printf("  ------------------------------------------------\n");
	print_row("TOTAL", cases, failures);

	std::printf("\nRESULT: %s\n", failures == 0 ? "PASS" : "FAIL");
	return failures == 0 ? 0 : 1;
}
