/*
 * b0208s4 differential harness.
 *
 * For every case the reference (ref_linux_getcwd, compiled from oracle.c) and
 * the C++23 module port are each run against the SAME deterministic kernel
 * environment, and everything observable is compared:
 *
 *   - the returned errno;
 *   - both slots of td_retval (so a mutation from td_retval[0] to
 *     td_retval[1] cannot hide);
 *   - the whole user buffer.  Each side gets its OWN buffer, pre-filled with
 *     the guard byte 0x7f, and the comparison covers every byte including the
 *     ones past the nominal copyout window;
 *   - the whole kernel arena, i.e. what the ported code left behind in the
 *     block it allocated;
 *   - the trace of kernel calls: allocation size and flags, the buflen handed
 *     to vn_getcwd and the one it handed back, the copyout source OFFSET (never
 *     a raw address) and length, the offset handed to free, and the
 *     allocate/free balance.
 *
 * The trace is what makes a return-value-only pass impossible: a port that
 * mallocs the wrong size, clamps at the wrong boundary, copies out from the
 * wrong offset or copies the wrong number of bytes diverges even when the
 * errno happens to agree.
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstddef>
#include <cerrno>
#include <cstdint>

import pbsd.sys.compat.linux.b0208s4;

namespace P = pbsd::sys_compat_linux::b0208s4;

/* ------------------------------------------------------------------ */
/* The oracle side and the shared kernel environment (oracle.c).       */
/* ------------------------------------------------------------------ */

#define KMOCK_TRACE_MAX		16
#define KMOCK_ARENA_SIZE	4352

#define KMOCK_EV_MALLOC		1
#define KMOCK_EV_VNGETCWD	2
#define KMOCK_EV_COPYOUT	3
#define KMOCK_EV_FREE		4

extern "C" {

struct kmock_event {
	int kind;
	unsigned long a;
	unsigned long b;
	unsigned long c;
	long r;
};

struct kthread {
	long td_retval[2];
};

struct klinux_getcwd_args {
	char *buf;
	unsigned long bufsize;
};

int ref_linux_getcwd(struct kthread *td, struct klinux_getcwd_args *uap);

extern struct kmock_event kmock_trace[KMOCK_TRACE_MAX];
extern int kmock_trace_n;
extern int kmock_trace_overflow;
extern int kmock_live;
extern char kmock_arena[KMOCK_ARENA_SIZE];

extern int kmock_vn_error;
extern unsigned long kmock_vn_off;
extern unsigned long kmock_vn_len;
extern unsigned char kmock_vn_fill;
extern int kmock_copyout_error;
extern size_t kmock_user_size;

void kmock_reset(void);

} /* extern "C" */

/* ------------------------------------------------------------------ */

static const size_t USER_TOTAL = 5120;	/* > LINUX_PATH_MAX + arena slack */
static const unsigned char GUARD = 0x7f;
static const long SENT0 = (long)0x5a5a5a5a5a5a5a5aLL;
static const long SENT1 = (long)0x3c3c3c3c3c3c3c3cLL;

struct Case {
	unsigned long bufsize;
	int vn_error;
	unsigned long vn_off;
	unsigned long vn_len;
	unsigned char vn_fill;
	int co_error;
};

static unsigned char user_ref[USER_TOTAL];
static unsigned char user_port[USER_TOTAL];

static int ref_ret;
static long ref_retval[2];
static int ref_trace_n;
static int ref_trace_ovf;
static int ref_live;
static struct kmock_event ref_trace[KMOCK_TRACE_MAX];
static unsigned char ref_arena[KMOCK_ARENA_SIZE];

static long total_cases;
static long total_fail;
static int printed;

static void
set_knobs(const Case &c)
{

	kmock_vn_error = c.vn_error;
	kmock_vn_off = c.vn_off;
	kmock_vn_len = c.vn_len;
	kmock_vn_fill = c.vn_fill;
	kmock_copyout_error = c.co_error;
	kmock_user_size = USER_TOTAL;
}

static void
show_case(const char *phase, const Case &c, const char *what)
{

	if (printed >= 25) {
		if (printed == 25)
			printf("  ... further failures suppressed\n");
		printed++;
		return;
	}
	printed++;
	printf("  FAIL [%s] bufsize=%lu (0x%lx) vn_error=%d vn_off=%lu "
	    "vn_len=%lu fill=0x%02x co_error=%d : %s\n",
	    phase, c.bufsize, c.bufsize, c.vn_error, c.vn_off, c.vn_len,
	    (unsigned)c.vn_fill, c.co_error, what);
}

static const char *
ev_name(int kind)
{

	switch (kind) {
	case KMOCK_EV_MALLOC:	return "malloc";
	case KMOCK_EV_VNGETCWD:	return "vn_getcwd";
	case KMOCK_EV_COPYOUT:	return "copyout";
	case KMOCK_EV_FREE:	return "free";
	default:		return "?";
	}
}

static bool
run_case(const char *phase, const Case &c)
{
	char what[512];
	bool ok = true;

	total_cases++;

	memset(user_ref, GUARD, sizeof(user_ref));
	memset(user_port, GUARD, sizeof(user_port));

	/* ---------------- reference ---------------- */
	set_knobs(c);
	kmock_reset();

	struct kthread td_r;
	td_r.td_retval[0] = SENT0;
	td_r.td_retval[1] = SENT1;
	struct klinux_getcwd_args ua_r;
	ua_r.buf = (char *)user_ref;
	ua_r.bufsize = c.bufsize;

	ref_ret = ref_linux_getcwd(&td_r, &ua_r);
	ref_retval[0] = td_r.td_retval[0];
	ref_retval[1] = td_r.td_retval[1];
	ref_trace_n = kmock_trace_n;
	ref_trace_ovf = kmock_trace_overflow;
	ref_live = kmock_live;
	memcpy(ref_trace, kmock_trace, sizeof(ref_trace));
	memcpy(ref_arena, kmock_arena, sizeof(ref_arena));

	/* ---------------- port ---------------- */
	set_knobs(c);
	kmock_reset();

	P::thread td_p;
	td_p.td_retval[0] = SENT0;
	td_p.td_retval[1] = SENT1;
	P::linux_getcwd_args ua_p;
	ua_p.buf = (char *)user_port;
	ua_p.bufsize = c.bufsize;

	int port_ret = P::linux_getcwd(&td_p, &ua_p);

	/* ---------------- compare ---------------- */
	if (port_ret != ref_ret) {
		snprintf(what, sizeof(what), "return: ref=%d port=%d",
		    ref_ret, port_ret);
		show_case(phase, c, what);
		ok = false;
	}
	for (int i = 0; i < 2 && ok; i++) {
		if (td_p.td_retval[i] != ref_retval[i]) {
			snprintf(what, sizeof(what),
			    "td_retval[%d]: ref=%ld port=%ld", i,
			    ref_retval[i], td_p.td_retval[i]);
			show_case(phase, c, what);
			ok = false;
		}
	}
	if (ok && kmock_live != ref_live) {
		snprintf(what, sizeof(what),
		    "alloc/free balance: ref=%d port=%d", ref_live,
		    kmock_live);
		show_case(phase, c, what);
		ok = false;
	}
	if (ok && kmock_trace_overflow != ref_trace_ovf) {
		snprintf(what, sizeof(what),
		    "trace overflow: ref=%d port=%d", ref_trace_ovf,
		    kmock_trace_overflow);
		show_case(phase, c, what);
		ok = false;
	}
	if (ok && kmock_trace_n != ref_trace_n) {
		snprintf(what, sizeof(what),
		    "kernel call count: ref=%d port=%d", ref_trace_n,
		    kmock_trace_n);
		show_case(phase, c, what);
		ok = false;
	}
	if (ok) {
		for (int i = 0; i < ref_trace_n; i++) {
			const struct kmock_event &a = ref_trace[i];
			const struct kmock_event &b = kmock_trace[i];
			if (a.kind == b.kind && a.a == b.a && a.b == b.b &&
			    a.c == b.c && a.r == b.r)
				continue;
			snprintf(what, sizeof(what),
			    "kernel call %d: ref=%s(%lu,%lu,%lu)->%ld "
			    "port=%s(%lu,%lu,%lu)->%ld", i,
			    ev_name(a.kind), a.a, a.b, a.c, a.r,
			    ev_name(b.kind), b.a, b.b, b.c, b.r);
			show_case(phase, c, what);
			ok = false;
			break;
		}
	}
	if (ok) {
		for (size_t i = 0; i < sizeof(ref_arena); i++) {
			if (ref_arena[i] == (unsigned char)kmock_arena[i])
				continue;
			snprintf(what, sizeof(what),
			    "kernel arena byte %zu: ref=0x%02x port=0x%02x",
			    i, ref_arena[i],
			    (unsigned)(unsigned char)kmock_arena[i]);
			show_case(phase, c, what);
			ok = false;
			break;
		}
	}
	if (ok) {
		for (size_t i = 0; i < USER_TOTAL; i++) {
			if (user_ref[i] == user_port[i])
				continue;
			snprintf(what, sizeof(what),
			    "user buffer byte %zu%s: ref=0x%02x port=0x%02x",
			    i, i >= 4096 ? " (past nominal window)" : "",
			    user_ref[i], user_port[i]);
			show_case(phase, c, what);
			ok = false;
			break;
		}
	}

	if (!ok)
		total_fail++;
	return (ok);
}

/* ------------------------------------------------------------------ */
/* Hand-written edge cases.                                            */
/* ------------------------------------------------------------------ */

/*
 * Both sides of every boundary the port tests.  bufsize 0/1 take the ERANGE
 * exit, 2/3 are the first values that do not; 4095/4096/4097 straddle the
 * LINUX_PATH_MAX clamp; the 32-bit and 64-bit extremes make sure a missing
 * clamp shows up as an absurd allocation size rather than as nothing.
 */
static const unsigned long edge_bufsize[] = {
	0UL, 1UL, 2UL, 3UL, 4UL, 5UL, 6UL, 7UL, 8UL, 9UL,
	15UL, 16UL, 17UL, 31UL, 32UL, 63UL, 64UL, 127UL, 128UL,
	255UL, 256UL, 511UL, 512UL, 1023UL, 1024UL, 2047UL, 2048UL,
	4094UL, 4095UL, 4096UL, 4097UL, 4098UL, 4099UL, 4351UL, 4352UL,
	4353UL, 5000UL, 8191UL, 8192UL, 8193UL, 65535UL, 65536UL,
	0x7fffffffUL, 0x80000000UL, 0xffffffffUL, 0x100000000UL,
	0x7fffffffffffffffUL, 0x8000000000000000UL, 0xffffffffffffffffUL,
};

/* 0 and 1 both matter: 1 separates `error == 0' from `error == 1'. */
static const int edge_vn_error[] = { 0, 1, ENOMEM, ERANGE, EFAULT, EACCES };
static const int edge_co_error[] = { 0, 1, EFAULT, ERANGE };

static const unsigned long edge_off[] = {
	0UL, 1UL, 2UL, 4095UL, 4351UL, 0xffffffffffffffffUL,
};
static const unsigned long edge_len[] = {
	0UL, 1UL, 2UL, 4095UL, 4351UL, 0xffffffffffffffffUL,
};
static const unsigned char edge_fill[] = { 0x00, 0x80 };

static void
run_edge_cases(void)
{
	Case c;

	for (size_t a = 0; a < sizeof(edge_bufsize) / sizeof(edge_bufsize[0]); a++)
	for (size_t b = 0; b < sizeof(edge_vn_error) / sizeof(edge_vn_error[0]); b++)
	for (size_t d = 0; d < sizeof(edge_co_error) / sizeof(edge_co_error[0]); d++)
	for (size_t e = 0; e < sizeof(edge_off) / sizeof(edge_off[0]); e++)
	for (size_t f = 0; f < sizeof(edge_len) / sizeof(edge_len[0]); f++)
	for (size_t g = 0; g < sizeof(edge_fill) / sizeof(edge_fill[0]); g++) {
		c.bufsize = edge_bufsize[a];
		c.vn_error = edge_vn_error[b];
		c.co_error = edge_co_error[d];
		c.vn_off = edge_off[e];
		c.vn_len = edge_len[f];
		c.vn_fill = edge_fill[g];
		run_case("edge", c);
	}

	/*
	 * NUL-heavy and high-bit path contents: the pattern generator steps by
	 * 7, which is coprime with 256, so every fill seed sweeps the whole
	 * 0x00-0xff byte range including embedded NULs and 0x80-0xff.
	 */
	for (unsigned fill = 0; fill < 256; fill++) {
		for (size_t a = 0; a < sizeof(edge_bufsize) / sizeof(edge_bufsize[0]); a++) {
			c.bufsize = edge_bufsize[a];
			c.vn_error = 0;
			c.co_error = 0;
			c.vn_off = fill % 5;
			c.vn_len = 0xffffffffffffffffUL;	/* longest legal */
			c.vn_fill = (unsigned char)fill;
			run_case("bytes", c);
		}
	}
}

/* ------------------------------------------------------------------ */
/* Fixed-seed randomised sweep.                                        */
/* ------------------------------------------------------------------ */

static uint64_t rng_state = 0x0123456789abcdefULL;

static uint64_t
xs64(void)
{

	rng_state ^= rng_state << 13;
	rng_state ^= rng_state >> 7;
	rng_state ^= rng_state << 17;
	return (rng_state);
}

static unsigned long
rand_bufsize(void)
{
	uint64_t r = xs64();

	switch (r % 12u) {
	case 0:
		return ((unsigned long)(xs64() % 16));		/* tiny */
	case 1:
		return ((unsigned long)(4090 + xs64() % 15));	/* clamp edge */
	case 2:
		return ((unsigned long)(xs64() % 8704));
	case 3:
		return ((unsigned long)xs64());			/* full 64 bit */
	case 4: {
		unsigned sh = (unsigned)(xs64() % 64);
		unsigned long v = 1UL << sh;
		return (v + (unsigned long)(xs64() % 3) - 1UL);
	}
	case 5:
		return ((unsigned long)(xs64() % 5));		/* 0..4 */
	case 6:
		return (4096UL);
	case 7:
		return ((unsigned long)(xs64() % 300));
	case 8:
		return (0xffffffffUL + (unsigned long)(xs64() % 5) - 2UL);
	case 9:
		return ((unsigned long)(xs64() % 70000));
	case 10:
		return ((unsigned long)(xs64() & 0xffffffffUL));
	default:
		return ((unsigned long)(xs64() % 4400));
	}
}

static unsigned long
rand_sel(void)
{
	uint64_t r = xs64();

	switch (r % 6u) {
	case 0:
		return ((unsigned long)(xs64() % 8));
	case 1:
		return ((unsigned long)(xs64() % 4400));
	case 2:
		return (0UL);
	case 3:
		return (0xffffffffffffffffUL);
	case 4:
		return ((unsigned long)(4090 + xs64() % 15));
	default:
		return ((unsigned long)xs64());
	}
}

static const int rand_vn_error[] = {
	0, 0, 0, 0, 1, ENOMEM, ENOMEM, ERANGE, EFAULT, EACCES, EINVAL, -1,
};
static const int rand_co_error[] = {
	0, 0, 0, 0, 0, 1, EFAULT, ENOMEM, ERANGE, -5,
};

static void
run_random_sweep(long iterations)
{
	Case c;

	for (long i = 0; i < iterations; i++) {
		c.bufsize = rand_bufsize();
		c.vn_error = rand_vn_error[xs64() %
		    (sizeof(rand_vn_error) / sizeof(rand_vn_error[0]))];
		c.co_error = rand_co_error[xs64() %
		    (sizeof(rand_co_error) / sizeof(rand_co_error[0]))];
		c.vn_off = rand_sel();
		c.vn_len = rand_sel();
		c.vn_fill = (unsigned char)(xs64() & 0xffu);
		run_case("random", c);
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	long edge_cases, edge_fail, rand_cases, rand_fail;

	printf("b0208s4 differential test: linux_getcwd\n");
	printf("----------------------------------------------------------\n");

	run_edge_cases();
	edge_cases = total_cases;
	edge_fail = total_fail;

	run_random_sweep(220000);
	rand_cases = total_cases - edge_cases;
	rand_fail = total_fail - edge_fail;

	printf("\n");
	printf("%-24s %12s %12s\n", "function", "cases", "failures");
	printf("%-24s %12ld %12ld\n", "linux_getcwd [edge]", edge_cases,
	    edge_fail);
	printf("%-24s %12ld %12ld\n", "linux_getcwd [random]", rand_cases,
	    rand_fail);
	printf("----------------------------------------------------------\n");
	printf("%-24s %12ld %12ld\n", "TOTAL", total_cases, total_fail);
	printf("\nRESULT: %s\n", total_fail == 0 ? "PASS" : "FAIL");

	return (total_fail == 0 ? 0 : 1);
}
