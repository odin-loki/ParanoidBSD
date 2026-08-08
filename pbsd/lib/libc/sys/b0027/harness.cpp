/*
 * harness.cpp -- differential test for PBSD batch b0027.
 *
 * read, write, msync and wait4 are libc interposition wrappers: each loads a
 * function pointer from the interposing table and tail-calls it with the
 * original argument list.  The harness installs instrumented mocks in both the
 * oracle (ref_*) and port tables, then compares the mock-visible syscall tag,
 * every forwarded argument and return value.  For read and wait4 the mock also
 * writes through caller-supplied buffers; those buffers are allocated with
 * 0x7f guard bytes and compared in their entirety.
 */

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern "C" {

typedef int (*interpos_func_t)(void);

enum {
	INTERPOS_read,
	INTERPOS_write,
	INTERPOS_msync,
	INTERPOS_wait4,
	INTERPOS_MAX
};

ssize_t ref_read(int fd, void *buf, size_t nbytes);
ssize_t ref_write(int fd, const void *buf, size_t nbytes);
int ref_msync(void *addr, size_t len, int flags);
pid_t ref_wait4(pid_t pid, int *status, int options, struct rusage *ru);

void ref_set_interpos(int slot, interpos_func_t func);

}

import pbsd.lib.libc.sys.b0027;

namespace port = pbsd::lib_libc_sys::b0027;

/* ------------------------------------------------------------------ */
/* constants                                                          */
/* ------------------------------------------------------------------ */

#define	GUARD		0x7f
#define	BUF_TOTAL	512
#define	DATA_OFF	64
#define	DIGEST_MAX	64

/* ------------------------------------------------------------------ */
/* instrumented mocks                                                 */
/* ------------------------------------------------------------------ */

#define	PBSD_TAG_NONE	0
#define	PBSD_TAG_READ	1
#define	PBSD_TAG_WRITE	2
#define	PBSD_TAG_MSYNC	3
#define	PBSD_TAG_WAIT4	4

struct MockState {
	unsigned long long	ncalls;
	int			tag;
	int			fd;
	size_t			nbytes;
	void			*addr;
	size_t			len;
	int			flags;
	pid_t			pid;
	int			options;
	int			status_is_null;
	int			ru_is_null;
	int			status_out;
	unsigned char		wdigest[DIGEST_MAX];
	size_t			wdigest_len;
	long long		prog_ret;
};

static MockState mock;

static void
mock_reset(long long ret)
{
	memset(&mock, 0, sizeof(mock));
	mock.prog_ret = ret;
}

static void
mock_digest(const void *buf, size_t nbytes)
{
	size_t n = nbytes;

	if (n > DIGEST_MAX)
		n = DIGEST_MAX;
	mock.wdigest_len = n;
	if (n > 0 && buf != NULL)
		memcpy(mock.wdigest, buf, n);
}

static ssize_t
mock_read(int fd, void *buf, size_t nbytes)
{
	mock.ncalls++;
	mock.tag = PBSD_TAG_READ;
	mock.fd = fd;
	mock.nbytes = nbytes;

	ssize_t ret = (ssize_t)mock.prog_ret;

	if (ret > 0 && buf != NULL) {
		size_t n = (size_t)ret;

		if (n > nbytes)
			n = nbytes;
		unsigned char *p = (unsigned char *)buf;

		for (size_t i = 0; i < n; i++)
			p[i] = (unsigned char)(0x80u | ((fd + (int)i) & 0x7f));
	}
	return (ret);
}

static ssize_t
mock_write(int fd, const void *buf, size_t nbytes)
{
	mock.ncalls++;
	mock.tag = PBSD_TAG_WRITE;
	mock.fd = fd;
	mock.nbytes = nbytes;
	mock_digest(buf, nbytes);
	return ((ssize_t)mock.prog_ret);
}

static int
mock_msync(void *addr, size_t len, int flags)
{
	mock.ncalls++;
	mock.tag = PBSD_TAG_MSYNC;
	mock.addr = addr;
	mock.len = len;
	mock.flags = flags;
	return ((int)mock.prog_ret);
}

static pid_t
mock_wait4(pid_t pid, int *status, int options, struct rusage *ru)
{
	mock.ncalls++;
	mock.tag = PBSD_TAG_WAIT4;
	mock.pid = pid;
	mock.options = options;
	mock.status_is_null = (status == NULL);
	mock.ru_is_null = (ru == NULL);
	mock.status_out = (int)(mock.prog_ret ^ 0x5a5a5a5a);

	if (status != NULL)
		*status = mock.status_out;
	if (ru != NULL)
		memset(ru, 0x42, sizeof(*ru));

	return ((pid_t)mock.prog_ret);
}

static void
install_ref_mocks(void)
{
	ref_set_interpos(INTERPOS_read, (interpos_func_t)mock_read);
	ref_set_interpos(INTERPOS_write, (interpos_func_t)mock_write);
	ref_set_interpos(INTERPOS_msync, (interpos_func_t)mock_msync);
	ref_set_interpos(INTERPOS_wait4, (interpos_func_t)mock_wait4);
}

static void
install_port_mocks(void)
{
	port::set_interpos(INTERPOS_read, (interpos_func_t)mock_read);
	port::set_interpos(INTERPOS_write, (interpos_func_t)mock_write);
	port::set_interpos(INTERPOS_msync, (interpos_func_t)mock_msync);
	port::set_interpos(INTERPOS_wait4, (interpos_func_t)mock_wait4);
}

struct Snap {
	unsigned long long	ncalls;
	int			tag;
	int			fd;
	size_t			nbytes;
	void			*addr;
	size_t			len;
	int			flags;
	pid_t			pid;
	int			options;
	int			status_is_null;
	int			ru_is_null;
	unsigned char		wdigest[DIGEST_MAX];
	size_t			wdigest_len;
};

static Snap
take_snap(void)
{
	Snap s;

	memset(&s, 0, sizeof(s));
	s.ncalls = mock.ncalls;
	s.tag = mock.tag;
	s.fd = mock.fd;
	s.nbytes = mock.nbytes;
	s.addr = mock.addr;
	s.len = mock.len;
	s.flags = mock.flags;
	s.pid = mock.pid;
	s.options = mock.options;
	s.status_is_null = mock.status_is_null;
	s.ru_is_null = mock.ru_is_null;
	s.wdigest_len = mock.wdigest_len;
	memcpy(s.wdigest, mock.wdigest, DIGEST_MAX);
	return (s);
}

/* ------------------------------------------------------------------ */
/* bookkeeping                                                        */
/* ------------------------------------------------------------------ */

enum {
	FN_READ,
	FN_WRITE,
	FN_MSYNC,
	FN_WAIT4,
	FN_COUNT
};

static const char *const fn_name[FN_COUNT] = {
	"read",
	"write",
	"msync",
	"wait4",
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

static bool
cmp_snap(int fn, const Snap &a, const Snap &b, const char *ctx)
{
	if (a.ncalls != b.ncalls || a.tag != b.tag || a.fd != b.fd ||
	    a.nbytes != b.nbytes || a.addr != b.addr || a.len != b.len ||
	    a.flags != b.flags || a.pid != b.pid || a.options != b.options ||
	    a.status_is_null != b.status_is_null ||
	    a.ru_is_null != b.ru_is_null ||
	    a.wdigest_len != b.wdigest_len ||
	    memcmp(a.wdigest, b.wdigest, DIGEST_MAX) != 0) {
		char msg[480];

		snprintf(msg, sizeof(msg),
		    "%s ref={tag=%d fd=%d nb=%zu len=%zu fl=%d pid=%d opt=%d "
		    "stnull=%d runull=%d wdl=%zu} "
		    "port={tag=%d fd=%d nb=%zu len=%zu fl=%d pid=%d opt=%d "
		    "stnull=%d runull=%d wdl=%zu}", ctx,
		    a.tag, a.fd, a.nbytes, a.len, a.flags, (int)a.pid,
		    a.options, a.status_is_null, a.ru_is_null, a.wdigest_len,
		    b.tag, b.fd, b.nbytes, b.len, b.flags, (int)b.pid,
		    b.options, b.status_is_null, b.ru_is_null, b.wdigest_len);
		fail(fn, "mock", msg);
		return (false);
	}
	return (true);
}

static void
fill_buf(unsigned char *buf, size_t off, size_t len, unsigned seed)
{
	for (size_t i = 0; i < BUF_TOTAL; i++)
		buf[i] = GUARD;
	for (size_t i = 0; i < len; i++) {
		unsigned v = (seed + (unsigned)i * 17u) & 0xffu;

		if ((i & 3u) == 0u)
			v = 0;
		buf[off + i] = (unsigned char)v;
	}
}

/* ------------------------------------------------------------------ */
/* PRNG (fixed seed)                                                  */
/* ------------------------------------------------------------------ */

static uint64_t rng_state;

static void
rng_seed(uint64_t seed)
{
	rng_state = seed;
}

static uint32_t
rng_u32(void)
{
	rng_state ^= rng_state << 13;
	rng_state ^= rng_state >> 7;
	rng_state ^= rng_state << 17;
	return ((uint32_t)rng_state);
}

static int
rnd_int(void)
{
	uint32_t u = rng_u32();

	return ((int)(u ^ (u >> 1)));
}

static unsigned
rnd_u32(void)
{
	return (rng_u32());
}

static long long
rnd_ret(void)
{
	uint32_t u = rng_u32();

	return ((long long)(int32_t)u);
}

static size_t
rnd_size(void)
{
	return ((size_t)rng_u32());
}

/* ------------------------------------------------------------------ */
/* per-function cases                                                 */
/* ------------------------------------------------------------------ */

static bool
case_read(int fd, size_t nbytes, long long ret)
{
	unsigned char buf_a[BUF_TOTAL], buf_b[BUF_TOTAL];
	Snap sa, sb;
	ssize_t ra, rb;
	char ctx[192];

	fn_cases[FN_READ]++;

	fill_buf(buf_a, DATA_OFF, BUF_TOTAL - DATA_OFF, (unsigned)fd ^ 0xa5u);
	fill_buf(buf_b, DATA_OFF, BUF_TOTAL - DATA_OFF, (unsigned)fd ^ 0xa5u);

	install_ref_mocks();
	mock_reset(ret);
	ra = ref_read(fd, buf_a + DATA_OFF, nbytes);
	sa = take_snap();

	install_port_mocks();
	mock_reset(ret);
	rb = port::read(fd, buf_b + DATA_OFF, nbytes);
	sb = take_snap();

	snprintf(ctx, sizeof(ctx), "fd=%d nb=%zu ret=%lld", fd, nbytes, ret);
	cmp_snap(FN_READ, sa, sb, ctx);

	if (ra != rb) {
		char msg[256];

		snprintf(msg, sizeof(msg), "%s ref=%lld port=%lld", ctx,
		    (long long)ra, (long long)rb);
		fail(FN_READ, "return", msg);
		return (false);
	}
	if (memcmp(buf_a, buf_b, BUF_TOTAL) != 0) {
		fail(FN_READ, "buffer", ctx);
		return (false);
	}
	return (true);
}

static bool
case_write(int fd, size_t nbytes, long long ret, unsigned pat_seed)
{
	unsigned char buf_a[BUF_TOTAL], buf_b[BUF_TOTAL];
	Snap sa, sb;
	ssize_t ra, rb;
	char ctx[192];

	fn_cases[FN_WRITE]++;

	fill_buf(buf_a, DATA_OFF, BUF_TOTAL - DATA_OFF, pat_seed);
	fill_buf(buf_b, DATA_OFF, BUF_TOTAL - DATA_OFF, pat_seed);

	install_ref_mocks();
	mock_reset(ret);
	ra = ref_write(fd, buf_a + DATA_OFF, nbytes);
	sa = take_snap();

	install_port_mocks();
	mock_reset(ret);
	rb = port::write(fd, buf_b + DATA_OFF, nbytes);
	sb = take_snap();

	snprintf(ctx, sizeof(ctx), "fd=%d nb=%zu ret=%lld seed=%u", fd,
	    nbytes, ret, pat_seed);
	cmp_snap(FN_WRITE, sa, sb, ctx);

	if (ra != rb) {
		char msg[256];

		snprintf(msg, sizeof(msg), "%s ref=%lld port=%lld", ctx,
		    (long long)ra, (long long)rb);
		fail(FN_WRITE, "return", msg);
		return (false);
	}
	if (memcmp(buf_a, buf_b, BUF_TOTAL) != 0) {
		fail(FN_WRITE, "buffer", ctx);
		return (false);
	}
	return (true);
}

static bool
case_msync(void *addr, size_t len, int flags, long long ret)
{
	Snap sa, sb;
	int ra, rb;
	char ctx[224];

	fn_cases[FN_MSYNC]++;

	install_ref_mocks();
	mock_reset(ret);
	ra = ref_msync(addr, len, flags);
	sa = take_snap();

	install_port_mocks();
	mock_reset(ret);
	rb = port::msync(addr, len, flags);
	sb = take_snap();

	snprintf(ctx, sizeof(ctx), "addr=%p len=%zu fl=%d ret=%lld",
	    addr, len, flags, ret);
	cmp_snap(FN_MSYNC, sa, sb, ctx);

	if (ra != rb) {
		char msg[256];

		snprintf(msg, sizeof(msg), "%s ref=%d port=%d", ctx, ra, rb);
		fail(FN_MSYNC, "return", msg);
		return (false);
	}
	return (true);
}

static bool
case_wait4(pid_t pid, int options, int use_status, int use_ru, long long ret)
{
	unsigned char st_a[sizeof(int) + 32], st_b[sizeof(int) + 32];
	unsigned char ru_buf_a[sizeof(struct rusage) + 32];
	unsigned char ru_buf_b[sizeof(struct rusage) + 32];
	int *status_a, *status_b;
	struct rusage *ru_a, *ru_b;
	Snap sa, sb;
	pid_t ra, rb;
	char ctx[256];

	fn_cases[FN_WAIT4]++;

	memset(st_a, GUARD, sizeof(st_a));
	memset(st_b, GUARD, sizeof(st_b));
	memset(ru_buf_a, GUARD, sizeof(ru_buf_a));
	memset(ru_buf_b, GUARD, sizeof(ru_buf_b));

	status_a = use_status ? (int *)(st_a + 16) : NULL;
	status_b = use_status ? (int *)(st_b + 16) : NULL;
	ru_a = use_ru ? (struct rusage *)(ru_buf_a + 16) : NULL;
	ru_b = use_ru ? (struct rusage *)(ru_buf_b + 16) : NULL;

	install_ref_mocks();
	mock_reset(ret);
	ra = ref_wait4(pid, status_a, options, ru_a);
	sa = take_snap();

	install_port_mocks();
	mock_reset(ret);
	rb = port::wait4(pid, status_b, options, ru_b);
	sb = take_snap();

	snprintf(ctx, sizeof(ctx),
	    "pid=%d opt=%d st=%d ru=%d ret=%lld", (int)pid, options,
	    use_status, use_ru, ret);
	cmp_snap(FN_WAIT4, sa, sb, ctx);

	if (ra != rb) {
		char msg[288];

		snprintf(msg, sizeof(msg), "%s ref=%d port=%d", ctx, (int)ra,
		    (int)rb);
		fail(FN_WAIT4, "return", msg);
		return (false);
	}
	if (use_status && memcmp(st_a, st_b, sizeof(st_a)) != 0) {
		fail(FN_WAIT4, "status_buf", ctx);
		return (false);
	}
	if (use_ru && memcmp(ru_buf_a, ru_buf_b, sizeof(ru_buf_a)) != 0) {
		fail(FN_WAIT4, "rusage_buf", ctx);
		return (false);
	}
	return (true);
}

static void
test_read(void)
{
	static const int fds[] = {
		INT_MIN, INT_MIN + 1, -2, -1, 0, 1, 2, 3,
		0x7e, 0x7f, 0x80, 0xfe, 0xff,
		INT_MAX - 1, INT_MAX,
	};
	static const size_t nbs[] = {
		0, 1, 2, DATA_OFF - 1, DATA_OFF, DATA_OFF + 1,
		BUF_TOTAL - DATA_OFF - 1, BUF_TOTAL - DATA_OFF,
		(size_t)SSIZE_MAX, (size_t)SIZE_MAX,
	};
	static const long long rets[] = {
		-1, 0, 1, 2, SSIZE_MAX, (long long)SSIZE_MAX - 1,
		INT_MIN, INT_MAX, 0x7f, 0x80, -2,
	};

	for (size_t f = 0; f < sizeof(fds) / sizeof(fds[0]); f++)
		for (size_t n = 0; n < sizeof(nbs) / sizeof(nbs[0]); n++)
			for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++) {
				fprintf(stderr, "case_read fd=%d nb=%zu ret=%lld\n",
				    fds[f], nbs[n], rets[r]);
				case_read(fds[f], nbs[n], rets[r]);
			}

	rng_seed(0x72'65'61'64ULL);
	for (int i = 0; i < 200000; i++) {
		int fd = rnd_int();
		size_t nb = rnd_size() & (BUF_TOTAL - 1);
		long long ret = rnd_ret();

		case_read(fd, nb, ret);
	}
}

static void
test_write(void)
{
	static const int fds[] = {
		INT_MIN, -1, 0, 1, 0x7f, 0x80, 0xff, INT_MAX,
	};
	static const size_t nbs[] = {
		0, 1, 2, DATA_OFF, BUF_TOTAL - DATA_OFF,
		(size_t)SSIZE_MAX,
	};
	static const unsigned seeds[] = {
		0, 1, 0x7f, 0x80, 0xff, 0xdeadbeef, 0xcafebabe,
	};
	static const long long rets[] = {
		-1, 0, 1, SSIZE_MAX, INT_MIN, INT_MAX, 0x7f, 0x80,
	};

	for (size_t f = 0; f < sizeof(fds) / sizeof(fds[0]); f++)
		for (size_t n = 0; n < sizeof(nbs) / sizeof(nbs[0]); n++)
			for (size_t s = 0; s < sizeof(seeds) / sizeof(seeds[0]); s++)
				for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
					case_write(fds[f], nbs[n], rets[r],
					    seeds[s]);

	rng_seed(0x77'72'69'74ULL);
	for (int i = 0; i < 200000; i++)
		case_write(rnd_int(), rnd_size() & (BUF_TOTAL - 1),
		    rnd_ret(), rnd_u32());
}

static unsigned char msync_arena[4096];

static void
test_msync(void)
{
	static const size_t lens[] = {
		0, 1, 2, 4095, 4096, (size_t)SIZE_MAX,
	};
	static const int flags[] = {
		0, MS_SYNC, MS_ASYNC, MS_INVALIDATE,
		-1, INT_MAX, 0x7f, 0x80,
	};
	static const long long rets[] = {
		-1, 0, 1, INT_MIN, INT_MAX,
	};
	static const size_t offs[] = { 0, 1, 16, 128, 2048, 4095 };

	for (size_t o = 0; o < sizeof(offs) / sizeof(offs[0]); o++)
		for (size_t l = 0; l < sizeof(lens) / sizeof(lens[0]); l++)
			for (size_t fl = 0; fl < sizeof(flags) / sizeof(flags[0]); fl++)
				for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
					case_msync(msync_arena + offs[o],
					    lens[l], flags[fl], rets[r]);

	rng_seed(0x6d'73'79'6eULL);
	for (int i = 0; i < 200000; i++) {
		size_t off = rnd_u32() & 4095u;
		size_t len = rnd_size();
		int fl = rnd_int();

		case_msync(msync_arena + off, len, fl, rnd_ret());
	}
}

static void
test_wait4(void)
{
	static const pid_t pids[] = {
		(pid_t)INT_MIN, (pid_t)-1, (pid_t)0, (pid_t)1,
		(pid_t)0x7f, (pid_t)0x80, (pid_t)INT_MAX,
	};
	static const int opts[] = {
		0, WNOHANG, WUNTRACED, WCONTINUED, -1, INT_MAX, 0x7f, 0x80,
	};
	static const long long rets[] = {
		-1, 0, 1, INT_MIN, INT_MAX, 0x7f, 0x80,
	};

	for (size_t p = 0; p < sizeof(pids) / sizeof(pids[0]); p++)
		for (size_t o = 0; o < sizeof(opts) / sizeof(opts[0]); o++)
			for (int st = 0; st < 2; st++)
				for (int ru = 0; ru < 2; ru++)
					for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
						case_wait4(pids[p], opts[o], st,
						    ru, rets[r]);

	rng_seed(0x77'61'69'74ULL);
	for (int i = 0; i < 200000; i++)
		case_wait4((pid_t)rnd_int(), rnd_int(), rnd_int() & 1,
		    rnd_int() & 1, rnd_ret());
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	unsigned long long total_cases = 0, total_fails = 0;

	fprintf(stderr, "test_read\n");
	test_read();
	fprintf(stderr, "test_write\n");
	test_write();
	fprintf(stderr, "test_msync\n");
	test_msync();
	fprintf(stderr, "test_wait4\n");
	test_wait4();

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
