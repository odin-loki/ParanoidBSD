/*
 * harness.cpp -- differential test for batch b0261 (__usleep, aio_read2).
 */

import pbsd.lib.libc.gen.b0261;

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <errno.h>
#include <time.h>
#include <unistd.h>

namespace port = pbsd::lib_libc_gen::b0261;

#define	LIO_READ		0x2
#define	LIO_FOFFSET		0x40
#define	LIO_VECTORED		0x4
#define	LIO_NOWAIT		0x0
#define	AIO_OP2_FOFFSET		0x00000001
#define	AIO_OP2_VECTORED	0x00000002

#define	GUARD			0x7f
#define	SWEEP_ITERS		200000L
#define	MAX_PRINT		12

struct ora_aiocb {
	int	aio_lio_opcode;
};

extern "C" {
int ref___usleep(useconds_t);
int ref_aio_read2(ora_aiocb *, int);
}

struct stat_row {
	const char *name;
	long		cases;
	long		failures;
	long		printed;
};

static stat_row rows[] = {
	{ "__usleep", 0, 0, 0 },
	{ "aio_read2", 0, 0, 0 },
};

#define	R_USLEEP	0
#define	R_AIO		1

static void
fail_row(int row, const char *label, const char *detail)
{
	stat_row &r = rows[row];

	r.failures++;
	if (r.printed < MAX_PRINT) {
		r.printed++;
		std::printf("  FAIL %-10s %-24s %s\n", r.name, label, detail);
	} else if (r.printed == MAX_PRINT) {
		r.printed++;
		std::printf("... further failures suppressed\n");
	}
}

static std::uint64_t rng_state = 0x00b0261feedULL;

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

static int
rnd_i(void)
{
	return ((int)rnd32());
}

enum ns_script {
	NS_OK = 0,
	NS_FAIL_EINTR,
	NS_FAIL_EINVAL,
	NS_SCRIPT_COUNT
};

struct ns_mock {
	int		script;
	long		calls;
	time_t		last_sec;
	long		last_nsec;
	struct timespec *last_rem;
};

static ns_mock g_ns;

static void
ns_mock_reset(int script)
{
	std::memset(&g_ns, 0, sizeof(g_ns));
	g_ns.script = script;
}

extern "C" int
nanosleep(const struct timespec *req, struct timespec *rem)
{
	g_ns.calls++;
	if (req != nullptr) {
		g_ns.last_sec = req->tv_sec;
		g_ns.last_nsec = req->tv_nsec;
	}
	g_ns.last_rem = rem;

	switch (g_ns.script) {
	case NS_OK:
		errno = 0;
		return (0);
	case NS_FAIL_EINTR:
		errno = EINTR;
		return (-1);
	case NS_FAIL_EINVAL:
		errno = EINVAL;
		return (-1);
	default:
		errno = EINVAL;
		return (-1);
	}
}

struct usleep_capture {
	int	rv;
	int	errno_copy;
	long	calls;
	time_t	tv_sec;
	long	tv_nsec;
	struct timespec *rem_ptr;
};

static usleep_capture
snap_usleep(int rv)
{
	usleep_capture c;

	c.rv = rv;
	c.errno_copy = errno;
	c.calls = g_ns.calls;
	c.tv_sec = g_ns.last_sec;
	c.tv_nsec = g_ns.last_nsec;
	c.rem_ptr = g_ns.last_rem;
	return (c);
}

static bool
usleep_cap_eq(const usleep_capture &a, const usleep_capture &b)
{
	return (a.rv == b.rv && a.errno_copy == b.errno_copy &&
	    a.calls == b.calls && a.tv_sec == b.tv_sec &&
	    a.tv_nsec == b.tv_nsec && a.rem_ptr == b.rem_ptr);
}

static void
do_usleep(useconds_t usec, int script, const char *label)
{
	int rva, rvb;
	usleep_capture ca, cb;

	rows[R_USLEEP].cases++;

	ns_mock_reset(script);
	errno = 0;
	rva = port::__usleep(usec);
	ca = snap_usleep(rva);

	ns_mock_reset(script);
	errno = 0;
	rvb = ref___usleep(usec);
	cb = snap_usleep(rvb);

	if (!usleep_cap_eq(ca, cb))
		fail_row(R_USLEEP, label, "mismatch");
}

enum aio_script {
	AIO_OK = 0,
	AIO_FAIL_ENOENT,
	AIO_FAIL_EIO_AERR_NEG,
	AIO_FAIL_EIO_AERR_ZERO,
	AIO_FAIL_EIO_AERR_EBUSY,
	AIO_FAIL_EIO_AERR_EAGAIN,
	AIO_SCRIPT_COUNT
};

struct aio_mock {
	int	script;
	long	lio_calls;
	long	aio_error_calls;
	int	last_mode;
	int	last_nent;
	int	last_opcode;
};

static aio_mock g_aio;

static void
aio_mock_reset(int script)
{
	std::memset(&g_aio, 0, sizeof(g_aio));
	g_aio.script = script;
}

extern "C" int
lio_listio(int mode, aiocb *const list[], int nent, void *sig)
{
	(void)sig;

	g_aio.lio_calls++;
	g_aio.last_mode = mode;
	g_aio.last_nent = nent;
	if (list != nullptr && nent > 0 && list[0] != nullptr)
		g_aio.last_opcode = list[0]->aio_lio_opcode;

	switch (g_aio.script) {
	case AIO_OK:
		errno = 0;
		return (0);
	case AIO_FAIL_ENOENT:
		errno = ENOENT;
		return (-1);
	case AIO_FAIL_EIO_AERR_NEG:
	case AIO_FAIL_EIO_AERR_ZERO:
	case AIO_FAIL_EIO_AERR_EBUSY:
	case AIO_FAIL_EIO_AERR_EAGAIN:
		errno = EIO;
		return (-1);
	default:
		errno = EINVAL;
		return (-1);
	}
}

extern "C" int
aio_error(const aiocb *iocb)
{
	(void)iocb;

	g_aio.aio_error_calls++;
	switch (g_aio.script) {
	case AIO_FAIL_EIO_AERR_NEG:
		return (-1);
	case AIO_FAIL_EIO_AERR_ZERO:
		return (0);
	case AIO_FAIL_EIO_AERR_EBUSY:
		return (EBUSY);
	case AIO_FAIL_EIO_AERR_EAGAIN:
		return (EAGAIN);
	default:
		return (0);
	}
}

struct guarded_aiocb {
	unsigned char	pre[32];
	aiocb		cb;
	unsigned char	post[32];
};

struct aio_capture {
	int	rv;
	int	errno_copy;
	int	opcode;
	long	lio_calls;
	long	aio_error_calls;
	int	last_mode;
	int	last_nent;
	int	last_opcode;
	unsigned char	pre[32];
	unsigned char	post[32];
};

static aio_capture
snap_aio(int rv, const guarded_aiocb &g)
{
	aio_capture c;

	c.rv = rv;
	c.errno_copy = errno;
	c.opcode = g.cb.aio_lio_opcode;
	c.lio_calls = g_aio.lio_calls;
	c.aio_error_calls = g_aio.aio_error_calls;
	c.last_mode = g_aio.last_mode;
	c.last_nent = g_aio.last_nent;
	c.last_opcode = g_aio.last_opcode;
	std::memcpy(c.pre, g.pre, sizeof(c.pre));
	std::memcpy(c.post, g.post, sizeof(c.post));
	return (c);
}

static bool
aio_cap_eq(const aio_capture &a, const aio_capture &b)
{
	return (a.rv == b.rv && a.errno_copy == b.errno_copy &&
	    a.opcode == b.opcode && a.lio_calls == b.lio_calls &&
	    a.aio_error_calls == b.aio_error_calls &&
	    a.last_mode == b.last_mode && a.last_nent == b.last_nent &&
	    a.last_opcode == b.last_opcode &&
	    std::memcmp(a.pre, b.pre, sizeof(a.pre)) == 0 &&
	    std::memcmp(a.post, b.post, sizeof(b.post)) == 0);
}

static void
init_guarded_aiocb(guarded_aiocb &g, int seed_opcode)
{
	std::memset(&g, GUARD, sizeof(g));
	g.cb.aio_lio_opcode = seed_opcode;
}

static void
do_aio_read2(int flags, int script, int seed_opcode, const char *label)
{
	guarded_aiocb ga, gb;
	int rva, rvb;
	aio_capture ca, cb;

	rows[R_AIO].cases++;

	init_guarded_aiocb(ga, seed_opcode);
	init_guarded_aiocb(gb, seed_opcode);

	aio_mock_reset(script);
	errno = 0;
	rva = port::aio_read2(&ga.cb, flags);
	ca = snap_aio(rva, ga);

	aio_mock_reset(script);
	errno = 0;
	rvb = ref_aio_read2(reinterpret_cast<ora_aiocb *>(&gb.cb), flags);
	cb = snap_aio(rvb, gb);

	if (!aio_cap_eq(ca, cb))
		fail_row(R_AIO, label, "mismatch");
}

static void
edge_usleep(void)
{
	static const useconds_t usecs[] = {
		0U,
		1U,
		2U,
		999998U,
		999999U,
		1000000U,
		1000001U,
		1999999U,
		2000000U,
		5000000U,
		1000000U - 1U,
		1000000U + 1U,
		2000000U - 1U,
		2000000U + 1U,
		(UINT_MAX / 1000000U) * 1000000U,
		(UINT_MAX / 1000000U) * 1000000U + 999999U,
		UINT_MAX - 1U,
		UINT_MAX,
	};

	for (unsigned i = 0; i < sizeof(usecs) / sizeof(usecs[0]); i++) {
		for (int sc = 0; sc < NS_SCRIPT_COUNT; sc++)
			do_usleep(usecs[i], sc, "edge");
	}

	for (unsigned v = 0; v < 256u; v++) {
		useconds_t u = (useconds_t)v * 10000U + (useconds_t)(v ^ 0x55u);
		do_usleep(u, NS_OK, "byte-scale");
	}

	for (useconds_t u = 999990U; u <= 1000010U; u++)
		do_usleep(u, NS_OK, "sec-boundary");

	for (useconds_t u = 1999990U; u <= 2000010U; u++)
		do_usleep(u, NS_OK, "two-sec-boundary");
}

static void
edge_aio(void)
{
	static const int flags[] = {
		0,
		AIO_OP2_FOFFSET,
		AIO_OP2_VECTORED,
		AIO_OP2_FOFFSET | AIO_OP2_VECTORED,
		0x0003,
		0x0004,
		0x0005,
		0x0007,
		0x00ff,
		0x7fff,
		0x8000,
		0xffff,
		INT_MIN,
		INT_MAX,
		-1,
	};
	static const int seeds[] = {
		0, 1, 2, LIO_READ, LIO_FOFFSET, LIO_VECTORED, 0x7f, 0x80, 0xff,
		INT_MIN, INT_MAX,
	};

	for (unsigned f = 0; f < sizeof(flags) / sizeof(flags[0]); f++) {
		for (unsigned s = 0; s < sizeof(seeds) / sizeof(seeds[0]); s++) {
			for (int sc = 0; sc < AIO_SCRIPT_COUNT; sc++)
				do_aio_read2(flags[f], sc, seeds[s], "edge");
		}
	}

	for (unsigned v = 0; v < 256u; v++)
		do_aio_read2((int)v, AIO_OK, (int)(v ^ 0x55), "byte-flag");
}

static int
rnd_ns_script(void)
{
	return ((int)(rnd32() % (std::uint32_t)NS_SCRIPT_COUNT));
}

static int
rnd_aio_script(void)
{
	return ((int)(rnd32() % (std::uint32_t)AIO_SCRIPT_COUNT));
}

static useconds_t
rnd_useconds(void)
{
	std::uint32_t r = rnd32();

	switch (rnd32() % 8u) {
	case 0:
		return (0U);
	case 1:
		return (1U);
	case 2:
		return (999999U);
	case 3:
		return (1000000U);
	case 4:
		return (1000001U);
	case 5:
		return ((useconds_t)(r % 1000000U));
	case 6:
		return ((useconds_t)r);
	default:
		return ((useconds_t)(r ^ (rnd32() << 1)));
	}
}

static int
rnd_aio_flags(void)
{
	std::uint32_t v = rnd32();

	switch (rnd32() % 6u) {
	case 0:
		return ((int)(v & 0x0003u));
	case 1:
		return ((int)v);
	case 2:
		return ((int)(v & 0x00ffu));
	case 3:
		return ((int)(1u << (rnd32() % 16u)));
	case 4:
		return ((int)((rnd32() & 1u) ? AIO_OP2_FOFFSET : 0) |
		    (int)((rnd32() & 1u) ? AIO_OP2_VECTORED : 0) |
		    (int)(rnd32() & 0xfffcu));
	default:
		return ((int)(rnd32() % 512u));
	}
}

static void
sweep(void)
{
	for (long it = 0; it < SWEEP_ITERS; it++) {
		if ((rnd32() & 1u) != 0u)
			do_usleep(rnd_useconds(), rnd_ns_script(), "random");
		else
			do_aio_read2(rnd_aio_flags(), rnd_aio_script(),
			    rnd_i(), "random");
	}
}

int
main(void)
{
	edge_usleep();
	edge_aio();
	sweep();

	long total_cases = 0;
	long total_failures = 0;

	std::printf("\n%-12s %12s %12s\n", "function", "cases", "failures");
	std::printf("--------------------------------------------\n");
	for (unsigned i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
		std::printf("%-12s %12ld %12ld\n", rows[i].name, rows[i].cases,
		    rows[i].failures);
		total_cases += rows[i].cases;
		total_failures += rows[i].failures;
	}
	std::printf("--------------------------------------------\n");
	std::printf("%-12s %12ld %12ld\n", "TOTAL", total_cases,
	    total_failures);

	if (total_failures != 0) {
		std::printf("\nRESULT: FAIL (%ld mismatching cases)\n",
		    total_failures);
		return (1);
	}
	std::printf("\nRESULT: PASS\n");
	return (0);
}
