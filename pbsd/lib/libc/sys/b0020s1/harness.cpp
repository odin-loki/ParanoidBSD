/*
 * Differential test harness for batch b0020s1.
 *
 * Compares pbsd::lib_libc_sys::b0020s1::pdwait (the C++23 port) against
 * ref_pdwait (the unmodified C oracle).
 *
 * pdwait() carries no arithmetic of its own: its entire observable behaviour
 * is (a) which interposing slot it dereferences, (b) the argument values it
 * forwards, in order, (c) how many times it forwards them, and (d) the result
 * it propagates back.  The harness therefore *is* the interposing table: it
 * installs a distinct recording implementation in every slot, so dispatching
 * through the wrong slot changes both the recorded tag and every byte the
 * callee writes.  The callee writes through the caller-supplied pointers, so
 * the usual guard-byte buffer comparison applies to all three of them.
 */

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <utility>

/*
 * The opaque kernel structures __wrusage / __siginfo come from the module, so
 * that the harness names exactly the entities in the port's signature.
 */
import pbsd.lib.libc.sys.b0020s1;

extern "C" {

typedef int (*interpos_func_t)(void);

int ref_pdwait(int fd, int *status, int options, __wrusage *ru,
    __siginfo *infop);

}

/* ------------------------------------------------------------------ */
/* The interposing table that both the port and the oracle dispatch through. */
/* ------------------------------------------------------------------ */

#define	PBSD_INTERPOS_SLOTS	16

typedef int (*pdwait_impl_t)(int, int *, int, struct __wrusage *,
    struct __siginfo *);

/* Number of bytes the recording callee scribbles through ru / infop. */
static const int RU_WRITE = 24;
static const int INFO_WRITE = 20;

struct Rec {
	int calls;
	int slot;
	int fd;
	int options;
	const void *status_p;
	const void *ru_p;
	const void *infop_p;
	int ret;
};

static Rec g_rec;

static void
rec_reset(void)
{
	g_rec.calls = 0;
	g_rec.slot = -0x7fff;
	g_rec.fd = -0x7ffe;
	g_rec.options = -0x7ffd;
	g_rec.status_p = reinterpret_cast<const void *>(~static_cast<uintptr_t>(1));
	g_rec.ru_p = reinterpret_cast<const void *>(~static_cast<uintptr_t>(2));
	g_rec.infop_p = reinterpret_cast<const void *>(~static_cast<uintptr_t>(3));
	g_rec.ret = -0x7ffc;
}

/*
 * Deterministic, argument-order sensitive callee.  Every input participates in
 * both the return value and the bytes written, and with different weights, so
 * that transposing two arguments or dropping one is always observable.
 */
static int
mock_body(int slot, int fd, int *status, int options, struct __wrusage *ru,
    struct __siginfo *infop)
{
	g_rec.calls++;
	g_rec.slot = slot;
	g_rec.fd = fd;
	g_rec.options = options;
	g_rec.status_p = status;
	g_rec.ru_p = ru;
	g_rec.infop_p = infop;

	unsigned u_fd = static_cast<unsigned>(fd);
	unsigned u_opt = static_cast<unsigned>(options);
	unsigned u_slot = static_cast<unsigned>(slot);

	if (status != nullptr) {
		*status = static_cast<int>((u_fd * 1000003u) ^ (u_opt * 31u) ^
		    (u_slot * 0x9e3779b9u) ^ 0x5a5a1234u);
	}
	if (ru != nullptr) {
		unsigned char *p = reinterpret_cast<unsigned char *>(ru);
		for (int i = 0; i < RU_WRITE; i++) {
			p[i] = static_cast<unsigned char>(p[i] ^
			    (u_fd + static_cast<unsigned>(i) * 7u +
			     u_opt * 13u + u_slot * 101u + 0xa5u));
		}
	}
	if (infop != nullptr) {
		unsigned char *p = reinterpret_cast<unsigned char *>(infop);
		for (int i = 0; i < INFO_WRITE; i++) {
			p[i] = static_cast<unsigned char>(0x40u +
			    static_cast<unsigned>(i) * 3u + (u_fd >> 3) +
			    u_opt * 5u + u_slot * 17u);
		}
	}

	unsigned r = u_fd * 1000003u + u_opt * 31u + u_slot * 7919u;
	r += (status != nullptr) ? 1u : 0u;
	r += (ru != nullptr) ? 2u : 0u;
	r += (infop != nullptr) ? 4u : 0u;
	g_rec.ret = static_cast<int>(r);
	return static_cast<int>(r);
}

template <int Slot>
static int
mock_slot(int fd, int *status, int options, struct __wrusage *ru,
    struct __siginfo *infop)
{
	return mock_body(Slot, fd, status, options, ru, infop);
}

extern "C" {
interpos_func_t __libc_interposing[PBSD_INTERPOS_SLOTS];
}

template <int... I>
static void
fill_table(std::integer_sequence<int, I...>)
{
	((__libc_interposing[I] =
	    reinterpret_cast<interpos_func_t>(
	        static_cast<pdwait_impl_t>(&mock_slot<I>))), ...);
}

/* ------------------------------------------------------------------ */
/* Guarded buffers                                                     */
/* ------------------------------------------------------------------ */

#define	GUARD	0x7f

static const int SBUF = 64;	/* status buffer  */
static const int RBUF = 80;	/* __wrusage buffer */
static const int IBUF = 72;	/* __siginfo buffer */

struct Side {
	alignas(16) unsigned char sbuf[SBUF];
	alignas(16) unsigned char rbuf[RBUF];
	alignas(16) unsigned char ibuf[IBUF];
};

static long long
off_of(const void *p, const void *base)
{
	if (p == nullptr)
		return -1;
	return static_cast<long long>(reinterpret_cast<intptr_t>(p) -
	    reinterpret_cast<intptr_t>(base));
}

/* ------------------------------------------------------------------ */
/* Fixed-seed PRNG                                                     */
/* ------------------------------------------------------------------ */

static uint64_t g_state;

static void
rng_seed(uint64_t s)
{
	g_state = s;
}

static uint32_t
rng_next(void)
{
	uint64_t x = g_state;
	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	g_state = x;
	return static_cast<uint32_t>(x >> 32);
}

/* ------------------------------------------------------------------ */
/* Case driver                                                         */
/* ------------------------------------------------------------------ */

struct Stats {
	long long cases;
	long long failures;
};

static Stats g_pdwait = { 0, 0 };
static int g_reported = 0;

static void
fill_side(Side &s, uint32_t pattern, int soff, int roff, int ioff,
    bool use_status, bool use_ru, bool use_infop)
{
	memset(s.sbuf, GUARD, sizeof(s.sbuf));
	memset(s.rbuf, GUARD, sizeof(s.rbuf));
	memset(s.ibuf, GUARD, sizeof(s.ibuf));

	/*
	 * The nominal write windows are seeded with identical input bytes on
	 * both sides; everything outside stays at the guard byte so that a
	 * write landing past the window is caught by the full-buffer compare.
	 */
	uint32_t h = pattern;
	if (use_status) {
		for (int i = 0; i < 4; i++) {
			h = h * 1103515245u + 12345u;
			s.sbuf[soff + i] = static_cast<unsigned char>(h >> 16);
		}
	}
	if (use_ru) {
		for (int i = 0; i < RU_WRITE; i++) {
			h = h * 1103515245u + 12345u;
			s.rbuf[roff + i] = static_cast<unsigned char>(h >> 16);
		}
	}
	if (use_infop) {
		for (int i = 0; i < INFO_WRITE; i++) {
			h = h * 1103515245u + 12345u;
			s.ibuf[ioff + i] = static_cast<unsigned char>(h >> 16);
		}
	}
}

static void
report(const char *what, long long caseno, int fd, int options, int soff,
    int roff, int ioff, bool us, bool ur, bool ui)
{
	if (g_reported >= 20)
		return;
	g_reported++;
	printf("FAIL[%lld] pdwait: %s  (fd=%d options=%d soff=%d roff=%d "
	    "ioff=%d status=%d ru=%d infop=%d)\n", caseno, what, fd, options,
	    soff, roff, ioff, us ? 1 : 0, ur ? 1 : 0, ui ? 1 : 0);
}

static void
run_case(int fd, int options, bool use_status, bool use_ru, bool use_infop,
    int soff, int roff, int ioff, uint32_t pattern)
{
	Side a, b;
	Rec rec_port, rec_ref;
	int ret_port, ret_ref;
	long long n = g_pdwait.cases++;
	int bad = 0;

	fill_side(a, pattern, soff, roff, ioff, use_status, use_ru, use_infop);
	fill_side(b, pattern, soff, roff, ioff, use_status, use_ru, use_infop);

	int *a_status = use_status ?
	    reinterpret_cast<int *>(a.sbuf + soff) : nullptr;
	int *b_status = use_status ?
	    reinterpret_cast<int *>(b.sbuf + soff) : nullptr;
	struct __wrusage *a_ru = use_ru ?
	    reinterpret_cast<struct __wrusage *>(a.rbuf + roff) : nullptr;
	struct __wrusage *b_ru = use_ru ?
	    reinterpret_cast<struct __wrusage *>(b.rbuf + roff) : nullptr;
	struct __siginfo *a_info = use_infop ?
	    reinterpret_cast<struct __siginfo *>(a.ibuf + ioff) : nullptr;
	struct __siginfo *b_info = use_infop ?
	    reinterpret_cast<struct __siginfo *>(b.ibuf + ioff) : nullptr;

	rec_reset();
	ret_port = pbsd::lib_libc_sys::b0020s1::pdwait(fd, a_status, options,
	    a_ru, a_info);
	rec_port = g_rec;

	rec_reset();
	ret_ref = ref_pdwait(fd, b_status, options, b_ru, b_info);
	rec_ref = g_rec;

	if (ret_port != ret_ref) {
		report("return value", n, fd, options, soff, roff, ioff,
		    use_status, use_ru, use_infop);
		bad = 1;
	}
	if (rec_port.calls != rec_ref.calls) {
		report("dispatch count", n, fd, options, soff, roff, ioff,
		    use_status, use_ru, use_infop);
		bad = 1;
	}
	if (rec_port.slot != rec_ref.slot) {
		report("interposing slot", n, fd, options, soff, roff, ioff,
		    use_status, use_ru, use_infop);
		bad = 1;
	}
	if (rec_port.fd != rec_ref.fd) {
		report("forwarded fd", n, fd, options, soff, roff, ioff,
		    use_status, use_ru, use_infop);
		bad = 1;
	}
	if (rec_port.options != rec_ref.options) {
		report("forwarded options", n, fd, options, soff, roff, ioff,
		    use_status, use_ru, use_infop);
		bad = 1;
	}
	if (rec_port.ret != rec_ref.ret) {
		report("callee result", n, fd, options, soff, roff, ioff,
		    use_status, use_ru, use_infop);
		bad = 1;
	}
	if (off_of(rec_port.status_p, a.sbuf) != off_of(rec_ref.status_p, b.sbuf)) {
		report("status pointer offset", n, fd, options, soff, roff,
		    ioff, use_status, use_ru, use_infop);
		bad = 1;
	}
	if (off_of(rec_port.ru_p, a.rbuf) != off_of(rec_ref.ru_p, b.rbuf)) {
		report("ru pointer offset", n, fd, options, soff, roff, ioff,
		    use_status, use_ru, use_infop);
		bad = 1;
	}
	if (off_of(rec_port.infop_p, a.ibuf) != off_of(rec_ref.infop_p, b.ibuf)) {
		report("infop pointer offset", n, fd, options, soff, roff,
		    ioff, use_status, use_ru, use_infop);
		bad = 1;
	}
	if (memcmp(a.sbuf, b.sbuf, sizeof(a.sbuf)) != 0) {
		report("status buffer", n, fd, options, soff, roff, ioff,
		    use_status, use_ru, use_infop);
		bad = 1;
	}
	if (memcmp(a.rbuf, b.rbuf, sizeof(a.rbuf)) != 0) {
		report("wrusage buffer", n, fd, options, soff, roff, ioff,
		    use_status, use_ru, use_infop);
		bad = 1;
	}
	if (memcmp(a.ibuf, b.ibuf, sizeof(a.ibuf)) != 0) {
		report("siginfo buffer", n, fd, options, soff, roff, ioff,
		    use_status, use_ru, use_infop);
		bad = 1;
	}

	if (bad)
		g_pdwait.failures++;
}

/* ------------------------------------------------------------------ */

static const int SOFFS[] = { 0, 4, 32, SBUF - 4 };
static const int ROFFS[] = { 0, 8, 33, RBUF - RU_WRITE };
static const int IOFFS[] = { 0, 1, 27, IBUF - INFO_WRITE };
static const int NOFFS = 4;

static const int EDGE_INTS[] = {
	INT_MIN, INT_MIN + 1, -65536, -256, -128, -2, -1, 0, 1, 2, 3, 7,
	0x7f, 0x80, 0xff, 0x100, 65535, 65536, INT_MAX - 1, INT_MAX
};
static const int NEDGE = static_cast<int>(sizeof(EDGE_INTS) / sizeof(EDGE_INTS[0]));

static void
edge_cases(void)
{
	for (int i = 0; i < NEDGE; i++) {
		for (int j = 0; j < NEDGE; j++) {
			for (int mask = 0; mask < 8; mask++) {
				for (int k = 0; k < NOFFS; k++) {
					run_case(EDGE_INTS[i], EDGE_INTS[j],
					    (mask & 1) != 0, (mask & 2) != 0,
					    (mask & 4) != 0, SOFFS[k],
					    ROFFS[k], IOFFS[k],
					    static_cast<uint32_t>(i * 1315423911u +
						j * 2654435761u + mask * 40503u +
						k * 97u + 1u));
				}
			}
		}
	}

	/* fd == options, so an argument transposition must still be caught
	 * only by the other invariants; include it explicitly. */
	for (int i = 0; i < NEDGE; i++) {
		run_case(EDGE_INTS[i], EDGE_INTS[i], true, true, true,
		    SOFFS[i % NOFFS], ROFFS[i % NOFFS], IOFFS[i % NOFFS],
		    static_cast<uint32_t>(0xdead0000u + i));
	}
}

static void
random_sweep(long long iters)
{
	rng_seed(0x0020000100205310ULL);

	for (long long n = 0; n < iters; n++) {
		uint32_t r0 = rng_next();
		uint32_t r1 = rng_next();
		uint32_t r2 = rng_next();

		int fd;
		int options;

		/* Mix wide random values with small/near-boundary ones. */
		switch (r2 & 3u) {
		case 0:
			fd = static_cast<int>(r0);
			break;
		case 1:
			fd = static_cast<int>(r0 % 4096u) - 2048;
			break;
		case 2:
			fd = static_cast<int>(r0 & 0xffu);
			break;
		default:
			fd = static_cast<int>(r0 | 0x80000000u);
			break;
		}
		switch ((r2 >> 2) & 3u) {
		case 0:
			options = static_cast<int>(r1);
			break;
		case 1:
			options = static_cast<int>(r1 % 8u);
			break;
		case 2:
			options = -static_cast<int>(r1 % 4096u);
			break;
		default:
			options = static_cast<int>(r1 & 0xffffu);
			break;
		}
		if ((r2 & 0x3fu) == 0u)
			options = fd;

		int mask = static_cast<int>((r2 >> 4) & 7u);
		int soff = static_cast<int>(((r2 >> 7) % (SBUF / 4)) * 4);
		int roff = static_cast<int>((rng_next() % (RBUF - RU_WRITE + 1)));
		int ioff = static_cast<int>((rng_next() % (IBUF - INFO_WRITE + 1)));

		run_case(fd, options, (mask & 1) != 0, (mask & 2) != 0,
		    (mask & 4) != 0, soff, roff, ioff, rng_next());
	}
}

int
main(void)
{
	fill_table(std::make_integer_sequence<int, PBSD_INTERPOS_SLOTS>{});

	edge_cases();
	random_sweep(250000);

	printf("\n%-24s %12s %12s %s\n", "FUNCTION", "CASES", "FAILURES",
	    "RESULT");
	printf("%-24s %12s %12s %s\n", "------------------------",
	    "------------", "------------", "------");
	printf("%-24s %12lld %12lld %s\n", "pdwait", g_pdwait.cases,
	    g_pdwait.failures, g_pdwait.failures == 0 ? "PASS" : "FAIL");

	long long total_cases = g_pdwait.cases;
	long long total_fail = g_pdwait.failures;

	printf("%-24s %12lld %12lld %s\n", "TOTAL", total_cases, total_fail,
	    total_fail == 0 ? "PASS" : "FAIL");

	return total_fail == 0 ? 0 : 1;
}
