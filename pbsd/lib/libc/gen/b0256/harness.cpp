/*
 * Differential harness for batch b0256 (__unvis_44bsd, kqueue1).
 * Every case drives both the C++23 port and the C oracle; return values,
 * errno, mock call records, and full guard buffers are compared.
 */

#define _GNU_SOURCE

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fcntl.h>

import pbsd.lib.libc.gen.b0256;

namespace P = pbsd::lib_libc_gen::b0256;

extern "C" {
int ref___unvis_44bsd(char *cp, int c, int *astate, int flag);
int ref_kqueue1(int openflags);
}

#ifndef KQUEUE_CLOEXEC
#define KQUEUE_CLOEXEC 0x00000001
#endif

#ifndef UNVIS_END
#define UNVIS_END 0x1000
#endif

/* ------------------------------------------------------------------ */
/* Statistics                                                         */
/* ------------------------------------------------------------------ */

enum { F_UNVIS, F_KQUEUE1, NFUNC };

static const char *const fname[NFUNC] = {
	"__unvis_44bsd", "kqueue1"
};

static unsigned long long ncases[NFUNC];
static unsigned long long nfails[NFUNC];

static constexpr int MAXREPORT = 16;
static int nreported;

static void
report(int f, const char *ctx, const char *detail)
{
	nfails[f]++;
	if (nreported < MAXREPORT) {
		nreported++;
		std::printf("FAIL %-20s %s: %s\n", fname[f], ctx, detail);
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

static std::uint32_t
randu32(void)
{
	return (std::uint32_t)(nextrand() & 0xffffffffu);
}

/* ------------------------------------------------------------------ */
/* unvis mock (linker --wrap)                                         */
/* ------------------------------------------------------------------ */

struct UnvisCall {
	int valid;
	ptrdiff_t cp_off;
	int c;
	int astate_before;
	int flag;
};

static UnvisCall unvis_log[8];
static int unvis_log_n;
static int unvis_call_seq;
static ptrdiff_t unvis_cp_off_ctx = -1;

static void
unvis_reset(void)
{
	unvis_log_n = 0;
	unvis_call_seq = 0;
	unvis_cp_off_ctx = -1;
	std::memset(unvis_log, 0, sizeof unvis_log);
}

static int
unvis_stub(char *cp, int c, int *astate, int flag)
{
	int ret;
	int state_in = astate ? *astate : 0;
	UnvisCall *rec;

	if (unvis_log_n < (int)(sizeof unvis_log / sizeof unvis_log[0])) {
		rec = &unvis_log[unvis_log_n++];
		rec->valid = 1;
		rec->cp_off = unvis_cp_off_ctx;
		rec->c = c;
		rec->astate_before = state_in;
		rec->flag = flag;
	}

	if ((flag & UNVIS_END) != 0)
		ret = 1;
	else if (flag == 0)
		ret = (c < 0) ? -1 : 3;
	else if ((flag & 0xffu) == (unsigned)c)
		ret = 2;
	else
		ret = (c & 1) ? -2 : 1;

	if (cp != nullptr) {
		*cp = (char)(c + (flag & 0x7f) + unvis_call_seq);
		cp[1] = (char)(0x80 | (flag >> 4));
	}
	if (astate != nullptr)
		*astate = (state_in * 17 + c + flag + unvis_call_seq) | 0x200;
	unvis_call_seq++;
	return ret;
}

extern "C" int
__wrap_unvis(char *cp, int c, int *astate, int flag)
{
	return unvis_stub(cp, c, astate, flag);
}

static int
unvis_logs_equal(const UnvisCall *a, int na, const UnvisCall *b, int nb)
{
	if (na != nb)
		return 0;
	for (int i = 0; i < na; i++) {
		if (!a[i].valid || !b[i].valid)
			return 0;
		if (a[i].cp_off != b[i].cp_off)
			return 0;
		if (a[i].c != b[i].c)
			return 0;
		if (a[i].astate_before != b[i].astate_before)
			return 0;
		if (a[i].flag != b[i].flag)
			return 0;
	}
	return 1;
}

/* ------------------------------------------------------------------ */
/* kqueuex mock (linker --wrap)                                      */
/* ------------------------------------------------------------------ */

static unsigned kqueuex_flags_log[8];
static int kqueuex_log_n;
static int kqueuex_ret = 7;

static void
kqueuex_reset(void)
{
	kqueuex_log_n = 0;
	std::memset(kqueuex_flags_log, 0, sizeof kqueuex_flags_log);
}

extern "C" int
__wrap_kqueuex(unsigned flags)
{
	if (kqueuex_log_n < (int)(sizeof kqueuex_flags_log /
	    sizeof kqueuex_flags_log[0]))
		kqueuex_flags_log[kqueuex_log_n++] = flags;
	return kqueuex_ret;
}

/* ------------------------------------------------------------------ */
/* Guard buffer helpers                                               */
/* ------------------------------------------------------------------ */

struct GuardBuf {
	static constexpr size_t PRE = 8;
	static constexpr size_t WIN = 48;
	static constexpr size_t POST = 8;
	static constexpr size_t TOTAL = PRE + WIN + POST;
	static constexpr unsigned char GUARD = 0x7f;

	unsigned char raw[TOTAL];
	char *win;

	void
	init(void)
	{
		std::memset(raw, GUARD, TOTAL);
		win = reinterpret_cast<char *>(raw + PRE);
	}

	void
	fill_input(unsigned seed)
	{
		for (size_t i = 0; i < WIN; i++)
			raw[PRE + i] = (unsigned char)((seed + (unsigned)i * 37u) &
			    0xffu);
	}

	int
	guards_intact(void) const
	{
		for (size_t i = 0; i < PRE; i++) {
			if (raw[i] != GUARD)
				return 0;
		}
		for (size_t i = PRE + WIN; i < TOTAL; i++) {
			if (raw[i] != GUARD)
				return 0;
		}
		return 1;
	}

	int
	equals(const GuardBuf &o) const
	{
		return std::memcmp(raw, o.raw, TOTAL) == 0;
	}
};

/* ------------------------------------------------------------------ */
/* __unvis_44bsd tests                                                */
/* ------------------------------------------------------------------ */

static void
unvis_case(int fidx, ptrdiff_t cp_off, int c, int state, int flag,
    const char *ctx)
{
	UnvisCall log_p[8], log_r[8];
	int np, nr;
	int ret_p, ret_r;
	GuardBuf buf_p, buf_r;
	char *use_cp_p = nullptr;
	char *use_cp_r = nullptr;
	int st_p = state;
	int st_r = state;

	if (cp_off >= 0) {
		buf_p.init();
		buf_r.init();
		buf_p.fill_input((unsigned)(c ^ flag));
		buf_r.fill_input((unsigned)(c ^ flag));
		use_cp_p = buf_p.win + cp_off;
		use_cp_r = buf_r.win + cp_off;
	}

	unvis_reset();
	unvis_cp_off_ctx = cp_off;
	ret_p = P::__unvis_44bsd(use_cp_p, c, &st_p, flag);
	np = unvis_log_n;
	std::memcpy(log_p, unvis_log, (size_t)np * sizeof log_p[0]);

	unvis_reset();
	unvis_cp_off_ctx = cp_off;
	ret_r = ref___unvis_44bsd(use_cp_r, c, &st_r, flag);
	nr = unvis_log_n;
	std::memcpy(log_r, unvis_log, (size_t)nr * sizeof log_r[0]);

	if (ret_p != ret_r)
		report(fidx, ctx, "return value mismatch");
	if (st_p != st_r)
		report(fidx, ctx, "astate mismatch");
	if (!unvis_logs_equal(log_p, np, log_r, nr))
		report(fidx, ctx, "unvis call record mismatch");
	if (cp_off >= 0 && (!buf_p.guards_intact() || !buf_r.guards_intact()))
		report(fidx, ctx, "guard corrupted");
	if (cp_off >= 0 && !buf_p.equals(buf_r))
		report(fidx, ctx, "buffer mismatch");
}

static void
unvis_ok(int c, int state, int flag, ptrdiff_t cp_off, const char *ctx)
{
	ncases[F_UNVIS]++;
	unvis_case(F_UNVIS, cp_off, c, state, flag, ctx);
}

static void
test_unvis_edges(void)
{
	/* flag without _UNVIS_END (bit 0 clear) */
	unvis_ok(0, 0, 0, -1, "null cp flag 0");
	unvis_ok('A', 5, 0, 0, "cp offset 0 flag 0");
	unvis_ok(-1, 0, 2, 3, "cp off 3 flag 2");
	unvis_ok(0x7f, 0x11, UNVIS_END, 10, "UNVIS_END no bit0");
	unvis_ok(0xff, -3, 0x1000, 20, "flag 0x1000 only");
	unvis_ok(0x80, 0x40, 0x1002, 1, "flag 0x1002");

	/* flag with _UNVIS_END (bit 0 set) -> transforms before unvis */
	unvis_ok(0, 0, 1, -1, "flag 1 null cp");
	unvis_ok('z', 9, 1, 0, "flag 1 -> UNVIS_END");
	unvis_ok(0x7f, 1, 3, 5, "flag 3");
	unvis_ok(0x80, 0, 0x1001, 7, "flag 0x1001 -> 0");
	unvis_ok('\0', 4, 0x0001 | 0x0002, 2, "flag 3 low bits");

	/* high-bit char values as int (sign extension load-bearing) */
	unvis_ok(0x80, 0, 0, 4, "c=0x80");
	unvis_ok(0xff, 0, 1, 6, "c=0xff flag 1");
	unvis_ok((unsigned char)0xfe, 7, 0, -1, "c=0xfe");

	/* NUL-heavy */
	unvis_ok('\0', 0, 0, 0, "NUL c flag 0");
	unvis_ok('\0', 0, 1, 1, "NUL c flag 1");
}

static void
test_unvis_random(void)
{
	for (int i = 0; i < 200000; i++) {
		char ctx[48];
		int c = (int)(randu32() & 0xffu);
		if ((nextrand() & 3u) == 0u)
			c = (int)(randu32() | 0xffffff00u);
		int state = (int)randu32();
		int flag = (int)randu32();
		ptrdiff_t cp_off = -1;

		if ((nextrand() & 1u) != 0u)
			cp_off = (ptrdiff_t)(nextrand() % 32u);

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		ncases[F_UNVIS]++;
		unvis_case(F_UNVIS, cp_off, c, state, flag, ctx);
	}
}

/* ------------------------------------------------------------------ */
/* kqueue1 tests                                                      */
/* ------------------------------------------------------------------ */

static void
kqueue1_case(int openflags, const char *ctx)
{
	int ret_p, ret_r;
	int err_p, err_r;
	unsigned log_p[8], log_r[8];
	int np, nr;

	errno = 0;
	kqueuex_reset();
	ret_p = P::kqueue1(openflags);
	err_p = errno;
	np = kqueuex_log_n;
	std::memcpy(log_p, kqueuex_flags_log, (size_t)np * sizeof log_p[0]);

	errno = 0;
	kqueuex_reset();
	ret_r = ref_kqueue1(openflags);
	err_r = errno;
	nr = kqueuex_log_n;
	std::memcpy(log_r, kqueuex_flags_log, (size_t)nr * sizeof log_r[0]);

	if (ret_p != ret_r)
		report(F_KQUEUE1, ctx, "return value mismatch");
	if (err_p != err_r)
		report(F_KQUEUE1, ctx, "errno mismatch");
	if (np != nr)
		report(F_KQUEUE1, ctx, "kqueuex call count mismatch");
	else {
		for (int j = 0; j < np; j++) {
			if (log_p[j] != log_r[j])
				report(F_KQUEUE1, ctx, "kqueuex flags mismatch");
		}
	}
}

static void
test_kqueue1_edges(void)
{
	ncases[F_KQUEUE1]++;
	kqueue1_case(0, "openflags 0");

	ncases[F_KQUEUE1]++;
	kqueue1_case(O_CLOEXEC, "O_CLOEXEC only");

	ncases[F_KQUEUE1]++;
	kqueue1_case(O_NONBLOCK, "O_NONBLOCK only");

	ncases[F_KQUEUE1]++;
	kqueue1_case(O_CLOEXEC | O_NONBLOCK, "O_CLOEXEC|O_NONBLOCK");

	ncases[F_KQUEUE1]++;
	kqueue1_case(1, "invalid bit 0");

	ncases[F_KQUEUE1]++;
	kqueue1_case(O_RDONLY, "O_RDONLY invalid");

	ncases[F_KQUEUE1]++;
	kqueue1_case(O_CLOEXEC | 1, "O_CLOEXEC|invalid");

	ncases[F_KQUEUE1]++;
	kqueue1_case(~0, "all bits set");

	ncases[F_KQUEUE1]++;
	kqueue1_case((O_CLOEXEC | O_NONBLOCK) ^ O_NONBLOCK,
	    "O_CLOEXEC exact");

	ncases[F_KQUEUE1]++;
	kqueue1_case(O_NONBLOCK | 2, "O_NONBLOCK|bit1 invalid");
}

static int
rand_openflags(void)
{
	std::uint32_t r = randu32();
	unsigned allowed = (unsigned)(O_CLOEXEC | O_NONBLOCK);
	unsigned extra = r & 0xfffffu;
	unsigned base = r & allowed;

	switch (r & 7u) {
	case 0:
		return 0;
	case 1:
		return (int)O_CLOEXEC;
	case 2:
		return (int)O_NONBLOCK;
	case 3:
		return (int)(O_CLOEXEC | O_NONBLOCK);
	case 4:
		return (int)base;
	case 5:
		return (int)(base | extra);
	case 6:
		return (int)(extra | 1u);
	default:
		return (int)r;
	}
}

static void
test_kqueue1_random(void)
{
	for (int i = 0; i < 200000; i++) {
		char ctx[48];
		int openflags = rand_openflags();

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		ncases[F_KQUEUE1]++;
		kqueue1_case(openflags, ctx);
	}
}

/* ------------------------------------------------------------------ */
/* main                                                               */
/* ------------------------------------------------------------------ */

int
main(void)
{
	test_unvis_edges();
	test_unvis_random();
	test_kqueue1_edges();
	test_kqueue1_random();

	std::printf("\n%-22s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < NFUNC; i++)
		std::printf("%-22s %12llu %12llu\n", fname[i],
		    ncases[i], nfails[i]);

	unsigned long long total_fail = 0;
	for (int i = 0; i < NFUNC; i++)
		total_fail += nfails[i];

	return total_fail == 0 ? 0 : 1;
}
