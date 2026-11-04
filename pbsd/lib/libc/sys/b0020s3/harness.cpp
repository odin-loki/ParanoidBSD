// b0020s3 differential test: port vs. ref_ oracle.
//
// sigwait() is libc's interposable syscall stub: it forwards both arguments
// through an interposing-table slot and returns whatever the slot's callee
// returned.  The test installs the same instrumented callee in the port's slot
// and in the oracle's slot, then compares, for every case:
//
//   - the return value;
//   - errno afterwards;
//   - that the slot was entered exactly as many times on each side;
//   - the OFFSETS (never raw addresses) of both forwarded pointers, relative
//     to each side's own buffer base, including the null/non-null distinction;
//   - the sigset_t bytes the callee actually observed through `set';
//   - the ENTIRE buffer of each side, guard bytes included, so a write that
//     lands outside the nominal `sig' window is caught.

#include <cerrno>
#include <climits>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <initializer_list>

import pbsd.lib.libc.sys.b0020s3;

extern "C" {
extern int (*ref_interpos_sigwait_slot)(const sigset_t *, int *);
int ref_sigwait(const sigset_t *set, int *sig);
}

namespace port = pbsd::lib_libc_sys::b0020s3;

namespace {

constexpr std::size_t SETSZ = sizeof(sigset_t);
constexpr std::size_t TAIL = 64;
constexpr std::size_t BUFSZ = SETSZ + TAIL;
constexpr unsigned char GUARD = 0x7f;
constexpr int ERRNO_SENTINEL = 0x5eed;

struct alignas(64) Buf {
	unsigned char b[BUFSZ];
};

struct Record {
	long calls;
	long set_off; // -1 when the pointer was null
	long sig_off; // -1 when the pointer was null
	bool set_seen_valid;
	unsigned char set_seen[SETSZ];
};

Record g_rec;
const unsigned char *g_base;
int g_ret;
int g_write_val;
bool g_do_write;
int g_errno_val;

long off_of(const void *p, const unsigned char *base)
{
	if (p == nullptr)
		return -1;
	return static_cast<long>(static_cast<const unsigned char *>(p) - base);
}

// The interposed callee, shared by both sides so that any difference in
// observed behaviour is attributable to the caller under test.
int shim(const sigset_t *set, int *sig)
{
	g_rec.calls++;
	g_rec.set_off = off_of(set, g_base);
	g_rec.sig_off = off_of(sig, g_base);
	g_rec.set_seen_valid = false;
	if (set != nullptr) {
		std::memcpy(g_rec.set_seen, set, SETSZ);
		g_rec.set_seen_valid = true;
	}
	if (sig != nullptr && g_do_write)
		*sig = g_write_val;
	errno = g_errno_val;
	return g_ret;
}

struct Case {
	unsigned char fill[SETSZ];
	std::size_t set_off;
	std::size_t sig_off;
	bool set_null;
	bool sig_null;
	bool do_write;
	int ret;
	int write_val;
	int err;
};

unsigned long g_cases;
unsigned long g_fails;
unsigned long g_reported;

void fail(const Case &c, const char *what, long a, long b)
{
	g_fails++;
	if (g_reported++ < 20u) {
		std::printf("  FAIL sigwait: %s port=%ld ref=%ld"
		    " (set_off=%zu sig_off=%zu set_null=%d sig_null=%d"
		    " do_write=%d ret=%d write=%d err=%d fill[0]=0x%02x)\n",
		    what, a, b, c.set_off, c.sig_off,
		    static_cast<int>(c.set_null), static_cast<int>(c.sig_null),
		    static_cast<int>(c.do_write), c.ret, c.write_val, c.err,
		    static_cast<unsigned>(c.fill[0]));
	}
}

void run(const Case &c)
{
	g_cases++;

	Buf A, B;
	std::memset(A.b, GUARD, BUFSZ);
	std::memset(B.b, GUARD, BUFSZ);
	std::memcpy(A.b + c.set_off, c.fill, SETSZ);
	std::memcpy(B.b + c.set_off, c.fill, SETSZ);

	g_ret = c.ret;
	g_write_val = c.write_val;
	g_do_write = c.do_write;
	g_errno_val = c.err;

	port::interpos_sigwait_slot = &shim;
	ref_interpos_sigwait_slot = &shim;

	std::memset(&g_rec, 0, sizeof g_rec);
	g_base = A.b;
	errno = ERRNO_SENTINEL;
	const int prv = port::sigwait(
	    c.set_null ? nullptr
	               : reinterpret_cast<const sigset_t *>(A.b + c.set_off),
	    c.sig_null ? nullptr : reinterpret_cast<int *>(A.b + c.sig_off));
	const int perrno = errno;
	const Record prec = g_rec;

	std::memset(&g_rec, 0, sizeof g_rec);
	g_base = B.b;
	errno = ERRNO_SENTINEL;
	const int orv = ref_sigwait(
	    c.set_null ? nullptr
	               : reinterpret_cast<const sigset_t *>(B.b + c.set_off),
	    c.sig_null ? nullptr : reinterpret_cast<int *>(B.b + c.sig_off));
	const int oerrno = errno;
	const Record orec = g_rec;

	if (prv != orv)
		fail(c, "return", prv, orv);
	if (perrno != oerrno)
		fail(c, "errno", perrno, oerrno);
	if (prec.calls != orec.calls)
		fail(c, "slot calls", prec.calls, orec.calls);
	if (prec.set_off != orec.set_off)
		fail(c, "set offset", prec.set_off, orec.set_off);
	if (prec.sig_off != orec.sig_off)
		fail(c, "sig offset", prec.sig_off, orec.sig_off);
	if (prec.set_seen_valid != orec.set_seen_valid)
		fail(c, "set null-ness", prec.set_seen_valid,
		    orec.set_seen_valid);
	if (prec.set_seen_valid && orec.set_seen_valid &&
	    std::memcmp(prec.set_seen, orec.set_seen, SETSZ) != 0)
		fail(c, "sigset contents", 1, 0);
	if (std::memcmp(A.b, B.b, BUFSZ) != 0) {
		std::size_t i = 0;
		while (i < BUFSZ && A.b[i] == B.b[i])
			i++;
		fail(c, "buffer bytes", static_cast<long>(A.b[i]),
		    static_cast<long>(B.b[i]));
	}
}

// ---------------------------------------------------------------- fill modes

void fill_const(unsigned char *p, unsigned char v)
{
	std::memset(p, v, SETSZ);
}

void fill_from_sigset(unsigned char *p, const sigset_t &s)
{
	std::memcpy(p, &s, SETSZ);
}

void fill_empty(unsigned char *p)
{
	sigset_t s;
	sigemptyset(&s);
	fill_from_sigset(p, s);
}

void fill_full(unsigned char *p)
{
	sigset_t s;
	sigfillset(&s);
	fill_from_sigset(p, s);
}

void fill_one(unsigned char *p, int signo)
{
	sigset_t s;
	sigemptyset(&s);
	sigaddset(&s, signo);
	fill_from_sigset(p, s);
}

// ------------------------------------------------------------------ hand set

Case base_case()
{
	Case c{};
	fill_const(c.fill, 0x00);
	c.set_off = 0;
	c.sig_off = SETSZ; // first slot past the sigset image
	c.set_null = false;
	c.sig_null = false;
	c.do_write = true;
	c.ret = 0;
	c.write_val = 0;
	c.err = 0;
	return c;
}

constexpr std::size_t LAST_SIG_OFF =
    ((BUFSZ - sizeof(int)) / alignof(int)) * alignof(int);

void edge_cases()
{
	// Empty / all-zero sigset, no write, both boundaries of ret.
	for (int r : {0, 1, -1, INT_MIN, INT_MAX}) {
		Case c = base_case();
		c.ret = r;
		c.do_write = false;
		run(c);
		c.do_write = true;
		run(c);
	}

	// Every byte pattern that matters for the sigset image, including the
	// guard byte itself (so a copy that overruns cannot hide) and the
	// high-bit range 0x80..0xff.
	for (unsigned v = 0; v <= 0xffu; v++) {
		Case c = base_case();
		fill_const(c.fill, static_cast<unsigned char>(v));
		c.write_val = static_cast<int>(v);
		c.ret = (v & 1u) ? -1 : 1;
		run(c);
	}
	for (unsigned v = 0x80u; v <= 0xffu; v++) {
		Case c = base_case();
		fill_const(c.fill, static_cast<unsigned char>(v));
		c.set_off = 16;
		c.sig_off = 0;
		c.ret = static_cast<int>(v) - 0x100;
		c.write_val = static_cast<int>(v) - 0x100;
		run(c);
	}

	// NUL-heavy: zeros with a single high-bit byte walked across the image.
	for (std::size_t i = 0; i < SETSZ; i++) {
		Case c = base_case();
		fill_const(c.fill, 0x00);
		c.fill[i] = 0xff;
		c.write_val = static_cast<int>(i) - 1;
		run(c);
		c.fill[i] = 0x80;
		c.do_write = false;
		run(c);
	}

	// Real sigsets.
	{
		Case c = base_case();
		fill_empty(c.fill);
		run(c);
		fill_full(c.fill);
		c.ret = -1;
		c.err = EINTR;
		run(c);
	}
	for (int signo : {SIGHUP, SIGINT, SIGQUIT, SIGKILL, SIGUSR1, SIGUSR2,
	         SIGTERM, SIGCHLD, SIGSTOP, SIGCONT}) {
		Case c = base_case();
		fill_one(c.fill, signo);
		c.write_val = signo;
		c.ret = 0;
		run(c);
		c.ret = -1;
		c.err = EAGAIN;
		c.do_write = false;
		run(c);
	}

	// Null-pointer combinations, both arguments independently.
	for (int sn = 0; sn < 2; sn++) {
		for (int gn = 0; gn < 2; gn++) {
			for (int r : {0, -1}) {
				Case c = base_case();
				c.set_null = (sn != 0);
				c.sig_null = (gn != 0);
				c.ret = r;
				c.err = (r == 0) ? 0 : EINVAL;
				c.write_val = 0x7f7f7f7f;
				run(c);
			}
		}
	}

	// Boundary write positions: the very first slot, the slot immediately
	// before and after the sigset image, and the last slot that still fits
	// inside the buffer.
	for (std::size_t off : {std::size_t{0}, alignof(int),
	         SETSZ - sizeof(int), SETSZ, SETSZ + alignof(int),
	         LAST_SIG_OFF - alignof(int), LAST_SIG_OFF}) {
		for (int wv : {0, 1, -1, INT_MIN, INT_MAX, 0x7f7f7f7f,
		         static_cast<int>(0x80808080u)}) {
			Case c = base_case();
			c.sig_off = off;
			c.write_val = wv;
			run(c);
			c.do_write = false;
			run(c);
		}
	}

	// Boundary `set' positions, including the one where the image ends
	// exactly at the end of the buffer.
	for (std::size_t so = 0; so + SETSZ <= BUFSZ; so += alignof(sigset_t)) {
		Case c = base_case();
		fill_const(c.fill, 0xa5);
		c.set_off = so;
		c.sig_off = 0;
		c.ret = static_cast<int>(so);
		c.write_val = static_cast<int>(so) - 1;
		run(c);
	}

	// errno values, including 0 and the sentinel itself.
	for (int e : {0, 1, EINTR, EINVAL, EAGAIN, EFAULT, ERRNO_SENTINEL,
	         INT_MAX}) {
		Case c = base_case();
		c.err = e;
		c.ret = (e == 0) ? 0 : -1;
		run(c);
	}
}

// ------------------------------------------------------------------- sweep

std::uint64_t g_state = 0x243f6a8885a308d3ULL;

std::uint64_t next_u64()
{
	g_state += 0x9e3779b97f4a7c15ULL;
	std::uint64_t z = g_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

unsigned next_u32()
{
	return static_cast<unsigned>(next_u64() >> 32);
}

std::size_t next_below(std::size_t n)
{
	return static_cast<std::size_t>(next_u64() % n);
}

int pick_int()
{
	switch (next_below(8)) {
	case 0:
		return 0;
	case 1:
		return 1;
	case 2:
		return -1;
	case 3:
		return INT_MIN;
	case 4:
		return INT_MAX;
	case 5:
		return 0x7f7f7f7f;
	case 6:
		return static_cast<int>(0x80808080u);
	default:
		return static_cast<int>(next_u32());
	}
}

void sweep(unsigned long iters)
{
	const std::size_t set_slots = (BUFSZ - SETSZ) / alignof(sigset_t) + 1;
	const std::size_t sig_slots = LAST_SIG_OFF / alignof(int) + 1;
	const int signos[] = {SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGABRT, SIGFPE,
	    SIGKILL, SIGSEGV, SIGPIPE, SIGALRM, SIGTERM, SIGUSR1, SIGUSR2,
	    SIGCHLD, SIGCONT, SIGSTOP};

	for (unsigned long i = 0; i < iters; i++) {
		Case c{};

		switch (next_below(9)) {
		case 0:
			fill_const(c.fill, 0x00);
			break;
		case 1:
			fill_const(c.fill, 0xff);
			break;
		case 2:
			fill_const(c.fill, 0x80);
			break;
		case 3:
			fill_const(c.fill, GUARD);
			break;
		case 4:
			fill_const(c.fill, 0x00);
			c.fill[next_below(SETSZ)] =
			    static_cast<unsigned char>(0x80u | next_below(0x80));
			break;
		case 5:
			fill_empty(c.fill);
			break;
		case 6:
			fill_full(c.fill);
			break;
		case 7:
			fill_one(c.fill,
			    signos[next_below(sizeof signos / sizeof signos[0])]);
			break;
		default:
			for (std::size_t k = 0; k < SETSZ; k++)
				c.fill[k] = static_cast<unsigned char>(
				    next_u32() & 0xffu);
			break;
		}

		c.set_off = next_below(set_slots) * alignof(sigset_t);
		c.sig_off = next_below(sig_slots) * alignof(int);
		c.set_null = (next_below(17) == 0);
		c.sig_null = (next_below(19) == 0);
		c.do_write = (next_below(3) != 0);
		c.ret = pick_int();
		c.write_val = pick_int();

		switch (next_below(6)) {
		case 0:
			c.err = 0;
			break;
		case 1:
			c.err = EINTR;
			break;
		case 2:
			c.err = EINVAL;
			break;
		case 3:
			c.err = EAGAIN;
			break;
		case 4:
			c.err = ERRNO_SENTINEL;
			break;
		default:
			c.err = static_cast<int>(next_below(4096));
			break;
		}

		run(c);
	}
}

} // namespace

int
main()
{
	edge_cases();
	sweep(250000ul);

	std::printf("%-12s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-12s %12s %12s\n", "------------", "------------",
	    "------------");
	std::printf("%-12s %12lu %12lu\n", "sigwait", g_cases, g_fails);
	std::printf("%-12s %12lu %12lu\n", "TOTAL", g_cases, g_fails);
	std::printf("%s\n", g_fails == 0 ? "PASS" : "FAIL");

	return g_fails == 0 ? 0 : 1;
}
