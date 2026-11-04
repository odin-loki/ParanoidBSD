/*
 * Differential test for PBSD batch b0021.
 *
 * Every case is executed against BOTH the C++23 port (module
 * pbsd.lib.libc.stdlib.b0021) and the ref_ oracle compiled from the original
 * C sources, and every observable is compared:
 *
 *	l64a_r	return value + the ENTIRE guarded destination region, including
 *		16 guard bytes in front of the nominal buffer (so a backwards
 *		walking write is caught) and 16 behind it.
 *	l64a	all 7 bytes of the static buffer (bytes past the terminating
 *		NUL included: both sides are driven by the identical call
 *		sequence, so stale bytes must agree too) plus the stability of
 *		the returned pointer, compared as an offset from the buffer
 *		base rather than as an address.
 *	remque	the whole element region after the call: every next/prev field
 *		encoded as a byte offset from that region's base (never a raw
 *		address) plus the raw guard padding on both sides.
 *	_Exit	the wait(2) status of a forked child, plus shared-memory flags
 *		that detect the call returning instead of terminating and
 *		detect exit-time handlers running (which _exit() must not do).
 */
#include <climits>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

import pbsd.lib.libc.stdlib.b0021;

extern "C" {
void ref__Exit(int code);
void ref_remque(void *element);
char *ref_l64a(long value);
int ref_l64a_r(long value, char *buffer, int buflen);
}

namespace port = pbsd::lib_libc_stdlib::b0021;

namespace {

/* ------------------------------------------------------------------ */
/* bookkeeping							      */
/* ------------------------------------------------------------------ */

struct Counter {
	const char *name;
	long cases;
	long fails;
	int reported;
};

Counter c_exit = { "_Exit", 0, 0, 0 };
Counter c_l64a = { "l64a", 0, 0, 0 };
Counter c_l64a_r = { "l64a_r", 0, 0, 0 };
Counter c_remque = { "remque", 0, 0, 0 };

void
chk(Counter &c, bool okay, const char *fmt, ...)
{
	c.cases++;
	if (okay)
		return;
	c.fails++;
	if (c.reported < 10) {
		c.reported++;
		va_list ap;
		va_start(ap, fmt);
		fprintf(stderr, "FAIL %s: ", c.name);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
		fputc('\n', stderr);
	} else if (c.reported == 10) {
		c.reported++;
		fprintf(stderr, "FAIL %s: (further failures suppressed)\n",
		    c.name);
	}
}

/* Fixed-seed xorshift64; identical stream on every run and every host. */
uint64_t rng_state = 0x9e3779b97f4a7c15ULL;

uint64_t
nextr(void)
{
	rng_state ^= rng_state << 13;
	rng_state ^= rng_state >> 7;
	rng_state ^= rng_state << 17;
	return rng_state;
}

void
hexdump(const char *tag, const unsigned char *p, int n)
{
	fprintf(stderr, "    %s:", tag);
	for (int i = 0; i < n; i++)
		fprintf(stderr, " %02x", p[i]);
	fputc('\n', stderr);
}

/* ------------------------------------------------------------------ */
/* l64a_r								      */
/* ------------------------------------------------------------------ */

/*
 * Nominal buffer lives at REGION+LEAD.  The whole REGION is compared, so
 * writes before the buffer (a mutated *buffer-- ) and writes past buflen
 * (a mutated loop bound) both show up.
 */
enum { LEAD = 16, WIN = 48, TRAIL = 16, REG = LEAD + WIN + TRAIL };
const int MAX_BUFLEN = 40; /* keeps even an over-running mutant inside REG */

void
case_l64a_r(long value, int buflen)
{
	unsigned char a[REG], b[REG];

	memset(a, 0x7f, sizeof(a));
	memset(b, 0x7f, sizeof(b));

	int ra = port::l64a_r(value, reinterpret_cast<char *>(a) + LEAD, buflen);
	int rb = ref_l64a_r(value, reinterpret_cast<char *>(b) + LEAD, buflen);

	bool same_buf = memcmp(a, b, sizeof(a)) == 0;
	bool okay = (ra == rb) && same_buf;

	chk(c_l64a_r, okay, "value=0x%lx buflen=%d: ret port=%d ref=%d buf %s",
	    static_cast<unsigned long>(value), buflen, ra, rb,
	    same_buf ? "equal" : "DIFFER");
	if (!okay && c_l64a_r.reported <= 10) {
		hexdump("port", a, REG);
		hexdump("ref ", b, REG);
	}
}

/* value whose low 32 bits contain exactly k non-empty 6-bit groups */
long
value_with_groups(int k, uint64_t bits)
{
	if (k <= 0)
		return 0;
	uint64_t mask = (k >= 11) ? ~0ULL : ((1ULL << (6 * k)) - 1);
	uint64_t v = (bits & mask) | (1ULL << (6 * (k - 1)));
	return static_cast<long>(v);
}

void
test_l64a_r_edges(void)
{
	static const long values[] = {
		0L, 1L, 2L, 3L, 0x3fL, 0x40L, 0x41L, 0x7fL, 0x80L, 0xffL,
		0x100L, 0xfffL, 0x1000L, 0x3ffffffL, 0x4000000L,
		0x7fffffffL, 0x80000000L, 0xfffffffeL, 0xffffffffL,
		0x100000000L, 0x100000001L, 0x1ffffffffL,
		-1L, -2L, -64L, -65L, -0x7fffffffL - 1L,
		LONG_MAX, LONG_MIN, LONG_MIN + 1L,
	};
	static const int buflens[] = {
		-100, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
		20, 32, 39, 40,
	};

	for (unsigned i = 0; i < sizeof(values) / sizeof(values[0]); i++)
		for (unsigned j = 0; j < sizeof(buflens) / sizeof(buflens[0]);
		    j++)
			case_l64a_r(values[i], buflens[j]);

	/*
	 * Both sides of the "buffer is exactly big enough" boundary for every
	 * possible number of 6-bit groups: buflen == k returns -1 with no
	 * terminator, buflen == k + 1 terminates and returns 0.
	 */
	for (int k = 0; k <= 6; k++) {
		long v_lo = value_with_groups(k, 0);
		long v_hi = value_with_groups(k, ~0ULL);
		for (int d = -2; d <= 2; d++) {
			case_l64a_r(v_lo, k + d);
			case_l64a_r(v_hi, k + d);
		}
	}
}

void
test_l64a_r_random(long iters)
{
	for (long i = 0; i < iters; i++) {
		uint64_t r = nextr();
		long value;

		switch (r & 3) {
		case 0:
			value = static_cast<long>(nextr());
			break;
		case 1:
			value = static_cast<long>(
			    static_cast<uint32_t>(nextr()));
			break;
		case 2:
			value = value_with_groups(
			    static_cast<int>(nextr() % 7), nextr());
			break;
		default:
			value = static_cast<long>(nextr() & 0xff) - 128L;
			break;
		}

		uint64_t s = nextr();
		int buflen;
		if ((s & 3) == 0)
			buflen = static_cast<int>((s >> 2) % 45) - 4;
		else
			buflen = static_cast<int>((s >> 2) % 13) - 3;
		if (buflen > MAX_BUFLEN)
			buflen = MAX_BUFLEN;

		case_l64a_r(value, buflen);
	}
}

/* ------------------------------------------------------------------ */
/* l64a								      */
/* ------------------------------------------------------------------ */

char *first_port_buf = nullptr;
char *first_ref_buf = nullptr;

void
case_l64a(long value)
{
	char *pa = port::l64a(value);
	char *pb = ref_l64a(value);

	if (first_port_buf == nullptr) {
		first_port_buf = pa;
		first_ref_buf = pb;
	}

	bool same_ptr = (pa - first_port_buf) == (pb - first_ref_buf);
	bool same_buf = pa != nullptr && pb != nullptr &&
	    memcmp(pa, pb, 7) == 0;
	bool okay = same_ptr && same_buf;

	chk(c_l64a, okay, "value=0x%lx: offset port=%ld ref=%ld buf %s",
	    static_cast<unsigned long>(value),
	    static_cast<long>(pa - first_port_buf),
	    static_cast<long>(pb - first_ref_buf),
	    same_buf ? "equal" : "DIFFER");
	if (!okay && c_l64a.reported <= 10) {
		hexdump("port", reinterpret_cast<unsigned char *>(pa), 7);
		hexdump("ref ", reinterpret_cast<unsigned char *>(pb), 7);
	}
}

void
test_l64a_edges(void)
{
	static const long values[] = {
		0L, 1L, 2L, 3L, 0x3fL, 0x40L, 0x41L, 0x7fL, 0x80L, 0xffL,
		0xfffL, 0x3ffffffL, 0x4000000L, 0x7fffffffL, 0x80000000L,
		0xfffffffeL, 0xffffffffL, 0x100000000L, 0x100000001L,
		-1L, -2L, -64L, LONG_MAX, LONG_MIN,
		/* long run followed by a short one: stale bytes must agree */
		0xffffffffL, 1L, 0xffffffffL, 0L, 0xffffffffL, 0x40L,
	};

	for (unsigned i = 0; i < sizeof(values) / sizeof(values[0]); i++)
		case_l64a(values[i]);

	for (int k = 0; k <= 6; k++) {
		case_l64a(value_with_groups(k, 0));
		case_l64a(value_with_groups(k, ~0ULL));
		case_l64a(0L); /* force a short write between long ones */
	}
}

void
test_l64a_random(long iters)
{
	for (long i = 0; i < iters; i++) {
		uint64_t r = nextr();
		long value;

		switch (r & 3) {
		case 0:
			value = static_cast<long>(nextr());
			break;
		case 1:
			value = static_cast<long>(
			    static_cast<uint32_t>(nextr()));
			break;
		case 2:
			value = value_with_groups(
			    static_cast<int>(nextr() % 7), nextr());
			break;
		default:
			value = static_cast<long>(nextr() & 0xff) - 128L;
			break;
		}
		case_l64a(value);
	}
}

/* ------------------------------------------------------------------ */
/* remque								      */
/* ------------------------------------------------------------------ */

struct Elem {
	Elem *next;
	Elem *prev;
};

enum { NEL = 6, PAD = 16 };

struct Region {
	unsigned char pre[PAD];
	Elem el[NEL];
	unsigned char post[PAD];
};

void
build(Region &r, const int *nx, const int *pv)
{
	memset(&r, 0x7f, sizeof(r));
	for (int i = 0; i < NEL; i++) {
		r.el[i].next = (nx[i] < 0) ? nullptr : &r.el[nx[i]];
		r.el[i].prev = (pv[i] < 0) ? nullptr : &r.el[pv[i]];
	}
}

/* Pointers are compared as offsets from the region base, never as addresses. */
long
enc(const Region &r, const void *p)
{
	if (p == nullptr)
		return -1;
	const unsigned char *base = reinterpret_cast<const unsigned char *>(&r);
	const unsigned char *q = reinterpret_cast<const unsigned char *>(p);
	if (q >= base && q < base + sizeof(Region))
		return q - base;
	return -2;
}

void
case_remque(const int *nx, const int *pv, int k)
{
	Region a, b;

	build(a, nx, pv);
	build(b, nx, pv);

	port::remque(&a.el[k]);
	ref_remque(&b.el[k]);

	bool okay = memcmp(a.pre, b.pre, PAD) == 0 &&
	    memcmp(a.post, b.post, PAD) == 0;
	int bad = -1;
	for (int i = 0; i < NEL; i++) {
		if (enc(a, a.el[i].next) != enc(b, b.el[i].next) ||
		    enc(a, a.el[i].prev) != enc(b, b.el[i].prev)) {
			okay = false;
			if (bad < 0)
				bad = i;
		}
	}

	chk(c_remque, okay,
	    "k=%d nx={%d,%d,%d,%d,%d,%d} pv={%d,%d,%d,%d,%d,%d}: "
	    "first differing element %d",
	    k, nx[0], nx[1], nx[2], nx[3], nx[4], nx[5],
	    pv[0], pv[1], pv[2], pv[3], pv[4], pv[5], bad);
	if (!okay && c_remque.reported <= 10) {
		for (int i = 0; i < NEL; i++)
			fprintf(stderr,
			    "    el[%d] port{next=%ld,prev=%ld} "
			    "ref{next=%ld,prev=%ld}\n",
			    i, enc(a, a.el[i].next), enc(a, a.el[i].prev),
			    enc(b, b.el[i].next), enc(b, b.el[i].prev));
		hexdump("port.pre ", a.pre, PAD);
		hexdump("ref .pre ", b.pre, PAD);
		hexdump("port.post", a.post, PAD);
		hexdump("ref .post", b.post, PAD);
	}
}

void
test_remque_edges(void)
{
	int nx[NEL], pv[NEL];

	/* isolated element: prev == NULL && next == NULL */
	for (int i = 0; i < NEL; i++) {
		nx[i] = -1;
		pv[i] = -1;
	}
	for (int k = 0; k < NEL; k++)
		case_remque(nx, pv, k);

	/* every combination of element 1's two links, including self-links */
	for (int n = -1; n < 3; n++) {
		for (int p = -1; p < 3; p++) {
			for (int i = 0; i < NEL; i++) {
				nx[i] = -1;
				pv[i] = -1;
			}
			nx[1] = n;
			pv[1] = p;
			case_remque(nx, pv, 1);
		}
	}

	/* the same, but with the neighbours already cross-linked */
	for (int n = -1; n < 3; n++) {
		for (int p = -1; p < 3; p++) {
			for (int i = 0; i < NEL; i++) {
				nx[i] = (i + 1) % NEL;
				pv[i] = (i + NEL - 1) % NEL;
			}
			nx[1] = n;
			pv[1] = p;
			case_remque(nx, pv, 1);
		}
	}

	/* NULL-terminated doubly linked list, remove head/middle/tail */
	for (int i = 0; i < NEL; i++) {
		nx[i] = (i == NEL - 1) ? -1 : i + 1;
		pv[i] = (i == 0) ? -1 : i - 1;
	}
	for (int k = 0; k < NEL; k++)
		case_remque(nx, pv, k);

	/* circular list, remove each element */
	for (int i = 0; i < NEL; i++) {
		nx[i] = (i + 1) % NEL;
		pv[i] = (i + NEL - 1) % NEL;
	}
	for (int k = 0; k < NEL; k++)
		case_remque(nx, pv, k);

	/* every element points at itself both ways */
	for (int i = 0; i < NEL; i++) {
		nx[i] = i;
		pv[i] = i;
	}
	for (int k = 0; k < NEL; k++)
		case_remque(nx, pv, k);

	/* prev == next (two-element circular list) */
	for (int i = 0; i < NEL; i++) {
		nx[i] = -1;
		pv[i] = -1;
	}
	nx[1] = 2;
	pv[1] = 2;
	nx[2] = 1;
	pv[2] = 1;
	case_remque(nx, pv, 1);
	case_remque(nx, pv, 2);

	/* only one side linked, both orientations, every position */
	for (int k = 0; k < NEL; k++) {
		for (int i = 0; i < NEL; i++) {
			nx[i] = -1;
			pv[i] = -1;
		}
		nx[k] = (k + 1) % NEL;
		case_remque(nx, pv, k);
		nx[k] = -1;
		pv[k] = (k + NEL - 1) % NEL;
		case_remque(nx, pv, k);
	}
}

int
pick_link(void)
{
	uint64_t r = nextr();
	if ((r & 3) == 0)
		return -1;
	return static_cast<int>((r >> 2) % NEL);
}

void
test_remque_random(long iters)
{
	int nx[NEL], pv[NEL];

	for (long i = 0; i < iters; i++) {
		for (int j = 0; j < NEL; j++) {
			nx[j] = pick_link();
			pv[j] = pick_link();
		}
		int k = static_cast<int>(nextr() % NEL);
		case_remque(nx, pv, k);
	}
}

/* ------------------------------------------------------------------ */
/* _Exit								      */
/* ------------------------------------------------------------------ */

/*
 * _Exit() terminates the caller, so each observation costs one process.  The
 * observable output space is the 8-bit wait status, and it is covered
 * exhaustively below (every code 0..255, plus the interesting out-of-range
 * ints); the randomised sweep on top of that is capped because each of its
 * iterations forks twice and adds no reachable state.
 */
const long EXIT_RANDOM_ITERS = 25000;

unsigned char *shared_flag = nullptr;

/*
 * Inherited by every child; _exit(2) must not run it, so it distinguishes
 * termination that skips exit-time processing from termination that does not.
 */
void
atexit_marker(void)
{
	if (shared_flag != nullptr)
		shared_flag[1] = 0x5a;
}

struct ExitObs {
	int exited;
	int code;
	int signaled;
	int sig;
	int returned;
	int atexit_ran;
};

ExitObs
run_exit(int which, int code)
{
	ExitObs o = { -1, -1, -1, -1, -1, -1 };
	pid_t p;

	shared_flag[0] = 0;
	shared_flag[1] = 0;
	fflush(nullptr);
	while ((p = fork()) < 0)
		;
	if (p == 0) {
		shared_flag[0] = 1;
		if (which == 0)
			port::_Exit(code);
		else
			ref__Exit(code);
		shared_flag[0] = 2;
		_exit(0);
	}

	int st = 0;
	while (waitpid(p, &st, 0) < 0)
		;
	o.exited = WIFEXITED(st) ? 1 : 0;
	o.code = o.exited ? WEXITSTATUS(st) : -1;
	o.signaled = WIFSIGNALED(st) ? 1 : 0;
	o.sig = o.signaled ? WTERMSIG(st) : -1;
	o.returned = (shared_flag[0] == 2) ? 1 : 0;
	o.atexit_ran = (shared_flag[1] == 0x5a) ? 1 : 0;
	return o;
}

void
case_exit(int code)
{
	ExitObs a = run_exit(0, code);
	ExitObs b = run_exit(1, code);

	bool okay = a.exited == b.exited && a.code == b.code &&
	    a.signaled == b.signaled && a.sig == b.sig &&
	    a.returned == b.returned && a.atexit_ran == b.atexit_ran;

	chk(c_exit, okay,
	    "code=%d: port{exited=%d,status=%d,sig=%d,returned=%d,atexit=%d} "
	    "ref{exited=%d,status=%d,sig=%d,returned=%d,atexit=%d}",
	    code, a.exited, a.code, a.sig, a.returned, a.atexit_ran,
	    b.exited, b.code, b.sig, b.returned, b.atexit_ran);
}

void
test_exit_edges(void)
{
	for (int code = 0; code <= 255; code++)
		case_exit(code);

	static const int extra[] = {
		256, 257, 300, 511, 512, 513, 1000, 65535, 65536, 65792,
		-1, -2, -127, -128, -255, -256, -257, -1000,
		INT_MAX, INT_MAX - 1, INT_MIN, INT_MIN + 1,
	};
	for (unsigned i = 0; i < sizeof(extra) / sizeof(extra[0]); i++)
		case_exit(extra[i]);
}

void
test_exit_random(long iters)
{
	for (long i = 0; i < iters; i++) {
		uint64_t r = nextr();
		int code;

		switch (r & 3) {
		case 0:
			code = static_cast<int>(static_cast<uint32_t>(nextr()));
			break;
		case 1:
			code = static_cast<int>(nextr() & 0xff);
			break;
		case 2:
			code = -static_cast<int>(nextr() & 0x3ff);
			break;
		default:
			code = static_cast<int>(nextr() & 0xffff) - 0x8000;
			break;
		}
		case_exit(code);
	}
}

void
report(const Counter &c)
{
	printf("  %-10s %12ld %12ld  %s\n", c.name, c.cases, c.fails,
	    c.fails == 0 ? "ok" : "FAILED");
}

} /* anonymous namespace */

int
main(void)
{
	shared_flag = static_cast<unsigned char *>(mmap(nullptr, 4096,
	    PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
	if (shared_flag == MAP_FAILED) {
		perror("mmap");
		return 1;
	}
	if (atexit(atexit_marker) != 0) {
		perror("atexit");
		return 1;
	}

	test_l64a_r_edges();
	test_l64a_r_random(200000);

	test_l64a_edges();
	test_l64a_random(200000);

	test_remque_edges();
	test_remque_random(200000);

	test_exit_edges();
	test_exit_random(EXIT_RANDOM_ITERS);

	long cases = c_exit.cases + c_l64a.cases + c_l64a_r.cases +
	    c_remque.cases;
	long fails = c_exit.fails + c_l64a.fails + c_l64a_r.fails +
	    c_remque.fails;

	printf("\nbatch b0021: port vs. oracle\n");
	printf("  %-10s %12s %12s\n", "function", "cases", "failures");
	printf("  ---------- ------------ ------------\n");
	report(c_l64a_r);
	report(c_l64a);
	report(c_remque);
	report(c_exit);
	printf("  ---------- ------------ ------------\n");
	printf("  %-10s %12ld %12ld  %s\n", "TOTAL", cases, fails,
	    fails == 0 ? "PASS" : "FAIL");

	return fails == 0 ? 0 : 1;
}
