/*
 * harness.cpp -- differential test for PBSD batch b0020.
 *
 * Every function of the batch is a dispatcher: it forwards a transformed
 * argument list either to a kernel primitive (__sys_close_range,
 * __sys_getdirentries) or through the libc interposition table
 * (__libc_interposing[]).  oracle.c supplies both, instrumented so that the
 * argument list actually delivered, the effect on the caller's buffers and the
 * value handed back are all observable.
 *
 * For each case the reference (ref_*) and the ported (pbsd::lib_libc_sys::
 * b0020::*) implementation are run in turn against freshly reset state and
 * freshly guarded buffers, and *everything* observable is compared: the
 * primitive that was reached, how many times, the whole recorded argument
 * vector (pointers reduced to offsets from their own buffer base, never raw
 * addresses), the return value, and the entire caller buffer including the
 * bytes outside the nominal write window.
 */

#include <sys/types.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

struct __wrusage;
struct __siginfo;

extern "C" {

#define	PBSD_MAX_ARGS	8

#define	PBSD_TAG_NONE		0
#define	PBSD_TAG_CLOSE_RANGE	1
#define	PBSD_TAG_GETDIRENTRIES	2
#define	PBSD_TAG_SIGWAIT	3
#define	PBSD_TAG_PDWAIT		4
#define	PBSD_TAG_SYS_SIGWAIT	5
#define	PBSD_TAG_SYS_PDWAIT	6

struct pbsd_mock_state {
	int			ncalls;
	int			nargs;
	int			tag;
	unsigned long long	args[PBSD_MAX_ARGS];

	long long		prog_ret;
	int			prog_out_int;
	unsigned int		prog_fill_seed;
	size_t			prog_fill_len;
};

extern struct pbsd_mock_state pbsd_mock;
void pbsd_mock_reset(long long ret, int out_int, unsigned int fill_seed,
    size_t fill_len);

int ref_pdwait(int fd, int *status, int options, struct __wrusage *ru,
    struct __siginfo *infop);
void ref_closefrom(int lowfd);
int ref_sigwait(const sigset_t *set, int *sig);
ssize_t ref_getdents(int fd, char *buf, size_t nbytes);

}

import pbsd.lib.libc.sys.b0020;

namespace port = pbsd::lib_libc_sys::b0020;

/* ------------------------------------------------------------------ */
/* bookkeeping                                                        */
/* ------------------------------------------------------------------ */

enum {
	FN_PDWAIT,
	FN_CLOSEFROM,
	FN_SIGWAIT,
	FN_GETDENTS,
	FN_COUNT
};

static const char *const fn_name[FN_COUNT] = {
	"pdwait",
	"closefrom",
	"sigwait",
	"getdents",
};

static unsigned long long fn_cases[FN_COUNT];
static unsigned long long fn_fails[FN_COUNT];
static int fn_reported[FN_COUNT];

#define	MAX_REPORTS	8

static bool
fail(int fn, const char *what, const char *detail)
{
	fn_fails[fn]++;
	if (fn_reported[fn] < MAX_REPORTS) {
		fn_reported[fn]++;
		printf("  FAIL %-9s %-18s %s\n", fn_name[fn], what, detail);
	} else if (fn_reported[fn] == MAX_REPORTS) {
		fn_reported[fn]++;
		printf("  FAIL %-9s ... further failures suppressed\n",
		    fn_name[fn]);
	}
	return (false);
}

/* ------------------------------------------------------------------ */
/* snapshot of what the primitive layer saw                           */
/* ------------------------------------------------------------------ */

struct Snap {
	int			ncalls;
	int			nargs;
	int			tag;
	unsigned long long	args[PBSD_MAX_ARGS];
};

static Snap
take_snap(void)
{
	Snap s;

	s.ncalls = pbsd_mock.ncalls;
	s.nargs = pbsd_mock.nargs;
	s.tag = pbsd_mock.tag;
	memcpy(s.args, pbsd_mock.args, sizeof(s.args));
	return (s);
}

/* A pointer argument is only ever compared as an offset from its own base. */
static unsigned long long
ptr_norm(unsigned long long rec, const void *base)
{
	if (rec == 0)
		return (0);
	return (rec - (unsigned long long)(uintptr_t)base + 1);
}

static bool
snap_eq(const Snap &a, const Snap &b)
{
	return (a.ncalls == b.ncalls && a.nargs == b.nargs && a.tag == b.tag &&
	    memcmp(a.args, b.args, sizeof(a.args)) == 0);
}

static void
snap_desc(const Snap &s, char *out, size_t outlen)
{
	int n;

	n = snprintf(out, outlen, "tag=%d ncalls=%d nargs=%d args=[",
	    s.tag, s.ncalls, s.nargs);
	for (int i = 0; i < PBSD_MAX_ARGS && n > 0 && (size_t)n < outlen; i++)
		n += snprintf(out + n, outlen - (size_t)n, "%llx%s", s.args[i],
		    i + 1 == PBSD_MAX_ARGS ? "]" : ",");
}

static bool
cmp_snap(int fn, const Snap &a, const Snap &b, const char *ctx)
{
	char da[512], db[512], msg[1200];

	if (snap_eq(a, b))
		return (true);
	snap_desc(a, da, sizeof(da));
	snap_desc(b, db, sizeof(db));
	snprintf(msg, sizeof(msg), "%s\n      ref  %s\n      port %s", ctx, da,
	    db);
	return (fail(fn, "primitive args", msg));
}

/* ------------------------------------------------------------------ */
/* deterministic randomness                                           */
/* ------------------------------------------------------------------ */

static unsigned long long rng_state;

static void
rng_seed(unsigned long long s)
{
	rng_state = s;
}

static unsigned long long
rnd(void)
{
	unsigned long long z;

	rng_state += 0x9e3779b97f4a7c15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static unsigned long long
rnd_below(unsigned long long n)
{
	return (rnd() % n);
}

/*
 * An int drawn from a distribution that hits the interesting boundaries hard:
 * half the draws are within a few units of zero (where MAX(0, lowfd) turns
 * over), the rest spread over the whole range including both extremes.
 */
static int
rnd_int(void)
{
	unsigned long long r;

	switch (rnd_below(8)) {
	case 0:
	case 1:
	case 2:
	case 3:
		return ((int)(rnd_below(11)) - 5);
	case 4:
		return ((int)(rnd_below(512)) - 256);
	case 5:
		return (rnd_below(2) ? INT_MIN : INT_MAX);
	case 6:
		r = rnd();
		return ((int)(long long)(-(long long)(r % 2147483648ULL)));
	default:
		return ((int)(unsigned int)rnd());
	}
}

static long long
rnd_ret(void)
{
	switch (rnd_below(6)) {
	case 0:
		return (0);
	case 1:
		return (-1);
	case 2:
		return (1);
	case 3:
		return (INT_MIN);
	case 4:
		return (INT_MAX);
	default:
		return ((int)(unsigned int)rnd());
	}
}

/* ------------------------------------------------------------------ */
/* closefrom                                                          */
/* ------------------------------------------------------------------ */

static void
case_closefrom(int lowfd)
{
	Snap a, b;
	char ctx[128];

	fn_cases[FN_CLOSEFROM]++;

	pbsd_mock_reset(0, 0, 0, 0);
	ref_closefrom(lowfd);
	a = take_snap();

	pbsd_mock_reset(0, 0, 0, 0);
	port::closefrom(lowfd);
	b = take_snap();

	snprintf(ctx, sizeof(ctx), "lowfd=%d", lowfd);
	cmp_snap(FN_CLOSEFROM, a, b, ctx);
}

static void
test_closefrom(void)
{
	static const int edges[] = {
		INT_MIN, INT_MIN + 1, INT_MIN + 2, -2147483647,
		-1073741824, -1000000, -65537, -65536, -65535,
		-1024, -513, -512, -511, -257, -256, -255,
		-130, -129, -128, -127, -126, -4, -3, -2, -1,
		0, 1, 2, 3, 4, 5, 126, 127, 128, 129, 130,
		254, 255, 256, 257, 1023, 1024, 32766, 32767, 32768,
		65535, 65536, 65537, 1000000, 1073741823, 1073741824,
		INT_MAX - 2, INT_MAX - 1, INT_MAX,
	};

	for (size_t i = 0; i < sizeof(edges) / sizeof(edges[0]); i++)
		case_closefrom(edges[i]);

	rng_seed(0x63'6c'6f'73'65'66ULL);
	for (int i = 0; i < 200000; i++)
		case_closefrom(rnd_int());
}

/* ------------------------------------------------------------------ */
/* getdents                                                           */
/* ------------------------------------------------------------------ */

#define	GD_CAP		192
#define	GD_GUARD	0x7f

static unsigned char gd_input[GD_CAP];

static void
case_getdents(int fd, size_t off, size_t nbytes, bool null_buf,
    long long ret, unsigned int seed, size_t fill_len)
{
	unsigned char bufa[GD_CAP], bufb[GD_CAP];
	char *pa, *pb;
	ssize_t ra, rb;
	Snap a, b;
	char ctx[192];

	fn_cases[FN_GETDENTS]++;

	memset(bufa, GD_GUARD, sizeof(bufa));
	memset(bufb, GD_GUARD, sizeof(bufb));
	/* identical input in both, inside the nominal window */
	if (!null_buf && off + nbytes <= GD_CAP) {
		memcpy(bufa + off, gd_input, nbytes);
		memcpy(bufb + off, gd_input, nbytes);
	}

	pa = null_buf ? nullptr : (char *)bufa + off;
	pb = null_buf ? nullptr : (char *)bufb + off;

	pbsd_mock_reset(ret, (int)seed, seed, fill_len);
	ra = ref_getdents(fd, pa, nbytes);
	a = take_snap();

	pbsd_mock_reset(ret, (int)seed, seed, fill_len);
	rb = port::getdents(fd, pb, nbytes);
	b = take_snap();

	a.args[1] = ptr_norm(a.args[1], bufa);
	b.args[1] = ptr_norm(b.args[1], bufb);

	snprintf(ctx, sizeof(ctx),
	    "fd=%d off=%zu nbytes=%zu null_buf=%d ret=%lld seed=%u fill=%zu",
	    fd, off, nbytes, (int)null_buf, ret, seed, fill_len);

	cmp_snap(FN_GETDENTS, a, b, ctx);

	if (ra != rb) {
		char msg[256];

		snprintf(msg, sizeof(msg), "%s ref=%zd port=%zd", ctx, ra, rb);
		fail(FN_GETDENTS, "return", msg);
	}
	if (memcmp(bufa, bufb, GD_CAP) != 0) {
		char msg[256];
		size_t i;

		for (i = 0; i < GD_CAP && bufa[i] == bufb[i]; i++)
			;
		snprintf(msg, sizeof(msg),
		    "%s first diff at %zu ref=%02x port=%02x", ctx, i,
		    bufa[i], bufb[i]);
		fail(FN_GETDENTS, "buffer", msg);
	}
}

static void
test_getdents(void)
{
	static const int fds[] = { INT_MIN, -1, 0, 1, 3, INT_MAX };
	static const size_t offs[] = { 0, 1, 7, 8, 15, 16 };
	static const size_t nbs[] = { 0, 1, 2, 7, 8, 63, 64 };
	static const size_t fills[] = { 0, 1, 7, 8, 63, 64, 65, 96 };
	unsigned int seed;

	for (size_t i = 0; i < GD_CAP; i++)
		gd_input[i] = (unsigned char)(i * 37u + (i >> 2) + 3u);
	/* make sure NUL bytes and high-bit bytes are both present */
	gd_input[0] = 0x00;
	gd_input[1] = 0xff;
	gd_input[2] = 0x80;
	gd_input[3] = 0x7f;
	gd_input[4] = 0x00;

	seed = 1;
	for (size_t f = 0; f < sizeof(fds) / sizeof(fds[0]); f++)
		for (size_t o = 0; o < sizeof(offs) / sizeof(offs[0]); o++)
			for (size_t n = 0; n < sizeof(nbs) / sizeof(nbs[0]); n++)
				for (size_t l = 0;
				    l < sizeof(fills) / sizeof(fills[0]); l++) {
					case_getdents(fds[f], offs[o], nbs[n],
					    false, (long long)nbs[n], seed,
					    fills[l]);
					seed += 7;
				}

	/* NULL buffer, including a nbytes nobody could honour */
	case_getdents(0, 0, 0, true, 0, 5, 0);
	case_getdents(0, 0, 1, true, -1, 6, 1);
	case_getdents(-1, 0, 64, true, 64, 7, 64);
	case_getdents(INT_MAX, 0, (size_t)-1, true, -1, 8, 96);
	case_getdents(INT_MIN, 0, (size_t)-1 / 2, true, 1, 9, 0);

	rng_seed(0x67'64'65'6e'74'73ULL);
	for (int i = 0; i < 200000; i++) {
		int fd = rnd_int();
		size_t off = (size_t)rnd_below(17);
		size_t nbytes = (size_t)rnd_below(65);
		size_t fill_len = (size_t)rnd_below(97);
		long long ret = rnd_ret();
		unsigned int sd = (unsigned int)rnd();
		bool nb = rnd_below(32) == 0;

		if (nb && rnd_below(2))
			nbytes = (size_t)rnd();
		case_getdents(fd, off, nbytes, nb, ret, sd, fill_len);
	}
}

/* ------------------------------------------------------------------ */
/* sigwait                                                            */
/* ------------------------------------------------------------------ */

static void
case_sigwait(const unsigned char *setbytes, bool null_set, bool null_sig,
    long long ret, int out_int)
{
	sigset_t seta, setb;
	int cella[3], cellb[3];
	int ra, rb;
	Snap a, b;
	char ctx[192];

	fn_cases[FN_SIGWAIT]++;

	memcpy(&seta, setbytes, sizeof(sigset_t));
	memcpy(&setb, setbytes, sizeof(sigset_t));
	memset(cella, GD_GUARD, sizeof(cella));
	memset(cellb, GD_GUARD, sizeof(cellb));

	const sigset_t *psa = null_set ? nullptr : &seta;
	const sigset_t *psb = null_set ? nullptr : &setb;
	int *pia = null_sig ? nullptr : &cella[1];
	int *pib = null_sig ? nullptr : &cellb[1];

	pbsd_mock_reset(ret, out_int, 0, 0);
	ra = ref_sigwait(psa, pia);
	a = take_snap();

	pbsd_mock_reset(ret, out_int, 0, 0);
	rb = port::sigwait(psb, pib);
	b = take_snap();

	a.args[1] = ptr_norm(a.args[1], &seta);
	b.args[1] = ptr_norm(b.args[1], &setb);
	a.args[2] = ptr_norm(a.args[2], cella);
	b.args[2] = ptr_norm(b.args[2], cellb);

	snprintf(ctx, sizeof(ctx),
	    "set0=%02x null_set=%d null_sig=%d ret=%lld out=%d",
	    setbytes[0], (int)null_set, (int)null_sig, ret, out_int);

	cmp_snap(FN_SIGWAIT, a, b, ctx);

	if (ra != rb) {
		char msg[256];

		snprintf(msg, sizeof(msg), "%s ref=%d port=%d", ctx, ra, rb);
		fail(FN_SIGWAIT, "return", msg);
	}
	if (memcmp(cella, cellb, sizeof(cella)) != 0) {
		char msg[512];

		snprintf(msg, sizeof(msg), "%s ref=[%08x,%08x,%08x] "
		    "port=[%08x,%08x,%08x]", ctx, cella[0], cella[1], cella[2],
		    cellb[0], cellb[1], cellb[2]);
		fail(FN_SIGWAIT, "sig out", msg);
	}
	if (memcmp(&seta, &setb, sizeof(sigset_t)) != 0)
		fail(FN_SIGWAIT, "set clobbered", ctx);
}

static void
test_sigwait(void)
{
	unsigned char pat[5][sizeof(sigset_t)];
	static const long long rets[] = { -1, 0, 1, 4, INT_MIN, INT_MAX };
	static const int outs[] = {
		0, 1, -1, 127, 128, 255, 256, -128, INT_MIN, INT_MAX
	};

	memset(pat[0], 0x00, sizeof(pat[0]));
	memset(pat[1], 0xff, sizeof(pat[1]));
	memset(pat[2], 0x7f, sizeof(pat[2]));
	memset(pat[3], 0x80, sizeof(pat[3]));
	for (size_t i = 0; i < sizeof(pat[4]); i++)
		pat[4][i] = (unsigned char)(i * 91u + 17u);

	for (size_t p = 0; p < 5; p++)
		for (int ns = 0; ns < 2; ns++)
			for (int ng = 0; ng < 2; ng++)
				for (size_t r = 0;
				    r < sizeof(rets) / sizeof(rets[0]); r++)
					for (size_t o = 0;
					    o < sizeof(outs) / sizeof(outs[0]);
					    o++)
						case_sigwait(pat[p], ns != 0,
						    ng != 0, rets[r], outs[o]);

	rng_seed(0x73'69'67'77'61'69'74ULL);
	{
		unsigned char buf[sizeof(sigset_t)];

		for (int i = 0; i < 200000; i++) {
			for (size_t j = 0; j < sizeof(buf); j++)
				buf[j] = (unsigned char)rnd();
			case_sigwait(buf, rnd_below(16) == 0,
			    rnd_below(16) == 0, rnd_ret(), rnd_int());
		}
	}
}

/* ------------------------------------------------------------------ */
/* pdwait                                                             */
/* ------------------------------------------------------------------ */

static void
case_pdwait(int fd, bool null_status, int options, bool null_ru,
    bool null_infop, long long ret, int out_int)
{
	int cella[3], cellb[3];
	alignas(16) unsigned char rua[64], rub[64];
	alignas(16) unsigned char ina[64], inb[64];
	int ra, rb;
	Snap a, b;
	char ctx[224];

	fn_cases[FN_PDWAIT]++;

	memset(cella, GD_GUARD, sizeof(cella));
	memset(cellb, GD_GUARD, sizeof(cellb));
	memset(rua, GD_GUARD, sizeof(rua));
	memset(rub, GD_GUARD, sizeof(rub));
	memset(ina, GD_GUARD, sizeof(ina));
	memset(inb, GD_GUARD, sizeof(inb));

	int *sa = null_status ? nullptr : &cella[1];
	int *sb = null_status ? nullptr : &cellb[1];
	struct __wrusage *wa = null_ru ? nullptr :
	    reinterpret_cast<struct __wrusage *>(rua);
	struct __wrusage *wb = null_ru ? nullptr :
	    reinterpret_cast<struct __wrusage *>(rub);
	struct __siginfo *fa = null_infop ? nullptr :
	    reinterpret_cast<struct __siginfo *>(ina);
	struct __siginfo *fb = null_infop ? nullptr :
	    reinterpret_cast<struct __siginfo *>(inb);

	pbsd_mock_reset(ret, out_int, 0, 0);
	ra = ref_pdwait(fd, sa, options, wa, fa);
	a = take_snap();

	pbsd_mock_reset(ret, out_int, 0, 0);
	rb = port::pdwait(fd, sb, options, wb, fb);
	b = take_snap();

	a.args[2] = ptr_norm(a.args[2], cella);
	b.args[2] = ptr_norm(b.args[2], cellb);
	a.args[4] = ptr_norm(a.args[4], rua);
	b.args[4] = ptr_norm(b.args[4], rub);
	a.args[5] = ptr_norm(a.args[5], ina);
	b.args[5] = ptr_norm(b.args[5], inb);

	snprintf(ctx, sizeof(ctx),
	    "fd=%d options=%d null_status=%d null_ru=%d null_infop=%d "
	    "ret=%lld out=%d", fd, options, (int)null_status, (int)null_ru,
	    (int)null_infop, ret, out_int);

	cmp_snap(FN_PDWAIT, a, b, ctx);

	if (ra != rb) {
		char msg[288];

		snprintf(msg, sizeof(msg), "%s ref=%d port=%d", ctx, ra, rb);
		fail(FN_PDWAIT, "return", msg);
	}
	if (memcmp(cella, cellb, sizeof(cella)) != 0) {
		char msg[640];

		snprintf(msg, sizeof(msg), "%s ref=[%08x,%08x,%08x] "
		    "port=[%08x,%08x,%08x]", ctx, cella[0], cella[1], cella[2],
		    cellb[0], cellb[1], cellb[2]);
		fail(FN_PDWAIT, "status out", msg);
	}
	if (memcmp(rua, rub, sizeof(rua)) != 0)
		fail(FN_PDWAIT, "wrusage", ctx);
	if (memcmp(ina, inb, sizeof(ina)) != 0)
		fail(FN_PDWAIT, "siginfo", ctx);
}

static void
test_pdwait(void)
{
	static const int ints[] = { INT_MIN, -1, 0, 1, 2, INT_MAX };
	static const long long rets[] = { -1, 0, 1, INT_MIN, INT_MAX };
	static const int outs[] = { 0, 1, -1, 0x7f, 0x80, INT_MIN, INT_MAX };
	size_t k = 0;

	for (size_t f = 0; f < sizeof(ints) / sizeof(ints[0]); f++)
		for (size_t o = 0; o < sizeof(ints) / sizeof(ints[0]); o++)
			for (int ms = 0; ms < 2; ms++)
				for (int mr = 0; mr < 2; mr++)
					for (int mi = 0; mi < 2; mi++) {
						case_pdwait(ints[f], ms != 0,
						    ints[o], mr != 0, mi != 0,
						    rets[k % (sizeof(rets) /
						    sizeof(rets[0]))],
						    outs[k % (sizeof(outs) /
						    sizeof(outs[0]))]);
						k++;
					}

	rng_seed(0x70'64'77'61'69'74ULL);
	for (int i = 0; i < 200000; i++)
		case_pdwait(rnd_int(), rnd_below(16) == 0, rnd_int(),
		    rnd_below(16) == 0, rnd_below(16) == 0, rnd_ret(),
		    rnd_int());
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	unsigned long long total_cases = 0, total_fails = 0;

	test_pdwait();
	test_closefrom();
	test_sigwait();
	test_getdents();

	printf("\n%-12s %12s %12s\n", "function", "cases", "failures");
	printf("--------------------------------------\n");
	for (int i = 0; i < FN_COUNT; i++) {
		printf("%-12s %12llu %12llu\n", fn_name[i], fn_cases[i],
		    fn_fails[i]);
		total_cases += fn_cases[i];
		total_fails += fn_fails[i];
	}
	printf("--------------------------------------\n");
	printf("%-12s %12llu %12llu\n", "TOTAL", total_cases, total_fails);

	return (total_fails == 0 ? 0 : 1);
}
