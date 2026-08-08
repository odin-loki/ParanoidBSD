/*
 * b0132s2 differential harness for utxdb.c (utx_to_futx / futx_to_utx).
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <endian.h>

#define PBSD_B0132S2_SHARED 1
#include "port.cppm"
#undef PBSD_B0132S2_SHARED

import pbsd.lib.libc.gen.b0132s2;

namespace P = pbsd::lib_libc_gen::b0132s2;

extern "C" {
void ref_utx_to_futx(const struct utmpx *ut, struct futx *fu);
struct utmpx *ref_futx_to_utx(const struct futx *fu);
}

#define H_EMPTY			0
#define H_BOOT_TIME		1
#define H_OLD_TIME		2
#define H_NEW_TIME		3
#define H_USER_PROCESS		4
#define H_INIT_PROCESS		5
#define H_LOGIN_PROCESS		6
#define H_DEAD_PROCESS		7
#define H_SHUTDOWN_TIME		8

struct LogEnt {
	int kind;
};

struct Log {
	int n;
	LogEnt e[64];
};

static Log g_log;

static void
logadd(int kind)
{
	if (g_log.n < 64)
		g_log.e[g_log.n].kind = kind;
	g_log.n++;
}

static bool
log_equal(const Log &x, const Log &y)
{
	if (x.n != y.n)
		return false;
	int n = x.n < 64 ? x.n : 64;
	for (int i = 0; i < n; i++)
		if (x.e[i].kind != y.e[i].kind)
			return false;
	return true;
}

static struct timeval g_faketv;

extern "C" int
gettimeofday(struct timeval *__restrict tv, void *__restrict tz) noexcept
{
	(void)tz;
	logadd(1);
	if (tv != NULL)
		*tv = g_faketv;
	return 0;
}

static int g_calloc_fail;

extern "C" void *__real_calloc(size_t n, size_t s);

extern "C" void *
__wrap_calloc(size_t n, size_t s)
{
	if (g_calloc_fail > 0) {
		g_calloc_fail--;
		errno = ENOMEM;
		return NULL;
	}
	return __real_calloc(n, s);
}

enum { S_UTOF, S_FTOU, S_NFUNC };

struct Stat {
	const char *name;
	long long cases;
	long long fails;
	int shown;
};

static Stat st[S_NFUNC] = {
	{ "utx_to_futx", 0, 0, 0 },
	{ "futx_to_utx", 0, 0, 0 },
};

static void
note(int idx, const char *what)
{
	st[idx].fails++;
	if (st[idx].shown < 8) {
		st[idx].shown++;
		fprintf(stderr, "MISMATCH %-18s case #%lld: %s\n",
		    st[idx].name, st[idx].cases, what);
	}
}

struct Rng {
	uint64_t s;
	explicit Rng(uint64_t seed) : s(seed) {}
	uint64_t next()
	{
		s += 0x9E3779B97F4A7C15ULL;
		uint64_t z = s;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
		return z ^ (z >> 31);
	}
	uint32_t u32() { return (uint32_t)next(); }
	uint32_t below(uint32_t n) { return n ? u32() % n : 0; }
	int byte() { return (int)(next() & 0xff); }
};

#define UT_ALLOC	512
#define FU_ALLOC	512
#define UT_OFF		8
#define FU_OFF		8
#define FUTX_SIZE	197

static unsigned char utmplA[UT_ALLOC];
static unsigned char utbufA[UT_ALLOC], utbufB[UT_ALLOC];
static unsigned char fubufA[FU_ALLOC], fubufB[FU_ALLOC];
static unsigned char futmplA[FU_ALLOC];
static unsigned char fibufA[FU_ALLOC], fibufB[FU_ALLOC];

static void
check_utx_to_futx(void)
{
	st[S_UTOF].cases++;

	memcpy(utbufA, utmplA, UT_ALLOC);
	memcpy(utbufB, utmplA, UT_ALLOC);
	memset(fubufA, 0x7f, FU_ALLOC);
	memset(fubufB, 0x7f, FU_ALLOC);

	g_log.n = 0;
	ref_utx_to_futx((const struct utmpx *)(utbufA + UT_OFF),
	    (struct futx *)(fubufA + FU_OFF));
	Log la = g_log;

	g_log.n = 0;
	P::utx_to_futx((const struct utmpx *)(utbufB + UT_OFF),
	    (struct futx *)(fubufB + FU_OFF));
	Log lb = g_log;

	if (memcmp(fubufA, fubufB, FU_ALLOC) != 0) {
		char msg[128];
		int off = 0;
		while (off < FU_ALLOC && fubufA[off] == fubufB[off])
			off++;
		snprintf(msg, sizeof msg,
		    "futx buffer differs at byte %d (%02x vs %02x)", off,
		    fubufA[off], fubufB[off]);
		note(S_UTOF, msg);
		return;
	}
	if (memcmp(utbufA, utbufB, UT_ALLOC) != 0) {
		note(S_UTOF, "input utmpx buffer differs");
		return;
	}
	if (!log_equal(la, lb))
		note(S_UTOF, "gettimeofday call log differs");
}

static void
check_futx_to_utx(void)
{
	st[S_FTOU].cases++;

	memcpy(fibufA, futmplA, FU_ALLOC);
	memcpy(fibufB, futmplA, FU_ALLOC);

	struct utmpx *ra = ref_futx_to_utx((const struct futx *)(fibufA + FU_OFF));
	struct utmpx *rb = P::futx_to_utx((const struct futx *)(fibufB + FU_OFF));

	if ((ra == NULL) != (rb == NULL)) {
		note(S_FTOU, "NULL-ness of return differs");
		return;
	}
	if (ra != NULL && memcmp(ra, rb, sizeof(struct utmpx)) != 0) {
		char msg[128];
		const unsigned char *pa = (const unsigned char *)ra;
		const unsigned char *pb = (const unsigned char *)rb;
		size_t off = 0;
		while (off < sizeof(struct utmpx) && pa[off] == pb[off])
			off++;
		snprintf(msg, sizeof msg,
		    "utmpx differs at byte %zu (%02x vs %02x), fu_type=%u",
		    off, pa[off], pb[off], (unsigned)fibufA[FU_OFF]);
		note(S_FTOU, msg);
		return;
	}
	if (memcmp(fibufA, fibufB, FU_ALLOC) != 0)
		note(S_FTOU, "input futx buffer differs");
}

static void
ut_set(unsigned char *b, short type, const unsigned char *id,
    int32_t pid, const unsigned char *user, const unsigned char *line,
    const unsigned char *host, int64_t sec, int64_t usec)
{
	unsigned char *u = b + UT_OFF;
	memset(u, 0, sizeof(struct utmpx));
	memcpy(u + offsetof(struct utmpx, ut_type), &type, sizeof type);
	memcpy(u + offsetof(struct utmpx, ut_pid), &pid, sizeof pid);
	memcpy(u + offsetof(struct utmpx, ut_id), id, 8);
	memcpy(u + offsetof(struct utmpx, ut_user), user, 32);
	memcpy(u + offsetof(struct utmpx, ut_line), line, 16);
	memcpy(u + offsetof(struct utmpx, ut_host), host, 128);
	memcpy(u + offsetof(struct utmpx, ut_tv) +
	    offsetof(struct timeval, tv_sec), &sec, sizeof sec);
	memcpy(u + offsetof(struct utmpx, ut_tv) +
	    offsetof(struct timeval, tv_usec), &usec, sizeof usec);
}

/* struct futx is packed: type 0, tv 1..8, id 9..16, pid 17..20, user 21..52,
 * line 53..68, host 69..196. */
static void
fu_set(unsigned char *b, unsigned type, uint64_t tv_be_value,
    const unsigned char *id, uint32_t pid_be_value,
    const unsigned char *user, const unsigned char *line,
    const unsigned char *host)
{
	unsigned char *f = b + FU_OFF;
	memset(f, 0, FUTX_SIZE);
	f[0] = (unsigned char)type;
	uint64_t tvbe = htobe64(tv_be_value);
	memcpy(f + 1, &tvbe, 8);
	memcpy(f + 9, id, 8);
	uint32_t pbe = htobe32(pid_be_value);
	memcpy(f + 17, &pbe, 4);
	memcpy(f + 21, user, 32);
	memcpy(f + 53, line, 16);
	memcpy(f + 69, host, 128);
}

static void
fill(unsigned char *p, size_t n, int v)
{
	memset(p, v, n);
}

static void
hand_utx(void)
{
	unsigned char id[8], user[32], line[16], host[128];
	static const short types[] = {
		-32768, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 100, 255, 256,
		32767
	};
	static const int64_t secs[] = {
		0, 1, -1, 12345, -12345, 1000000, 4294967296LL,
		9223372036854775807LL
	};
	static const int64_t usecs[] = { 0, 1, 999999, 1000000, -1, 123456 };
	static const int32_t pids[] = {
		0, 1, -1, 255, 256, 65535, 0x01020304, 2147483647,
		(int32_t)-2147483648LL
	};
	static const int fills[] = { 0x00, 0xff, 0x41, 0x80, 0x7f };

	for (size_t ti = 0; ti < sizeof types / sizeof types[0]; ti++) {
		for (size_t fi = 0; fi < sizeof fills / sizeof fills[0]; fi++) {
			fill(id, sizeof id, fills[fi]);
			fill(user, sizeof user, fills[fi]);
			fill(line, sizeof line, fills[fi]);
			fill(host, sizeof host, fills[fi]);
			for (size_t si = 0;
			    si < sizeof secs / sizeof secs[0]; si++) {
				for (size_t ui = 0;
				    ui < sizeof usecs / sizeof usecs[0];
				    ui++) {
					for (size_t pi = 0;
					    pi < sizeof pids / sizeof pids[0];
					    pi++) {
						memset(utmplA, 0x7f, UT_ALLOC);
						ut_set(utmplA, types[ti], id,
						    pids[pi], user, line, host,
						    secs[si], usecs[ui]);
						g_faketv.tv_sec =
						    (time_t)secs[si];
						g_faketv.tv_usec =
						    (suseconds_t)usecs[ui];
						check_utx_to_futx();
					}
				}
			}
		}
	}

	for (short type = 0; type <= 9; type++) {
		for (int pos = 0; pos < 34; pos++) {
			fill(id, sizeof id, 0x5a);
			fill(user, sizeof user, 0x5a);
			fill(line, sizeof line, 0x5a);
			fill(host, sizeof host, 0x5a);
			if (pos < 8)
				id[pos] = 0;
			if (pos < 32)
				user[pos] = 0;
			if (pos < 16)
				line[pos] = 0;
			if (pos < 128)
				host[pos] = 0;
			memset(utmplA, 0x7f, UT_ALLOC);
			ut_set(utmplA, type, id, 0x0a0b0c0d, user, line, host,
			    1700000000LL, 654321);
			g_faketv.tv_sec = 1700000000;
			g_faketv.tv_usec = 654321;
			check_utx_to_futx();
		}
	}
	for (int pos = 120; pos < 128; pos++) {
		fill(id, sizeof id, 0xa5);
		fill(user, sizeof user, 0xa5);
		fill(line, sizeof line, 0xa5);
		fill(host, sizeof host, 0xa5);
		host[pos] = 0;
		memset(utmplA, 0x7f, UT_ALLOC);
		ut_set(utmplA, H_USER_PROCESS, id, 7, user, line, host, 3, 4);
		g_faketv.tv_sec = 3;
		g_faketv.tv_usec = 4;
		check_utx_to_futx();
	}
}

static void
hand_ftou(void)
{
	unsigned char id[8], user[32], line[16], host[128];
	static const unsigned types[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 127, 128, 200, 254, 255
	};
	static const uint64_t tvs[] = {
		0, 1, 999999, 1000000, 1000001, 1999999, 2000000,
		1700000000000000ULL, 0x7fffffffffffffffULL,
		0xffffffffffffffffULL, 0x0102030405060708ULL
	};
	static const uint32_t pids[] = {
		0, 1, 0xffffffffu, 0x01020304u, 0x80000000u, 65535
	};
	static const int fills[] = { 0x00, 0xff, 0x41, 0x80, 0x7f };

	for (size_t ti = 0; ti < sizeof types / sizeof types[0]; ti++) {
		for (size_t fi = 0; fi < sizeof fills / sizeof fills[0]; fi++) {
			fill(id, sizeof id, fills[fi]);
			fill(user, sizeof user, fills[fi]);
			fill(line, sizeof line, fills[fi]);
			fill(host, sizeof host, fills[fi]);
			for (size_t vi = 0;
			    vi < sizeof tvs / sizeof tvs[0]; vi++) {
				for (size_t pi = 0;
				    pi < sizeof pids / sizeof pids[0]; pi++) {
					memset(futmplA, 0x7f, FU_ALLOC);
					fu_set(futmplA, types[ti], tvs[vi], id,
					    pids[pi], user, line, host);
					check_futx_to_utx();
				}
			}
		}
	}

	for (unsigned type = 0; type <= 9; type++) {
		for (int pos = 0; pos < 34; pos++) {
			fill(id, sizeof id, 0x5a);
			fill(user, sizeof user, 0x5a);
			fill(line, sizeof line, 0x5a);
			fill(host, sizeof host, 0x5a);
			if (pos < 8)
				id[pos] = 0;
			if (pos < 32)
				user[pos] = 0;
			if (pos < 16)
				line[pos] = 0;
			if (pos < 128)
				host[pos] = 0;
			memset(futmplA, 0x7f, FU_ALLOC);
			fu_set(futmplA, type, 1234567891234ULL, id, 4242, user,
			    line, host);
			check_futx_to_utx();
		}
	}
	for (int pos = 120; pos < 128; pos++) {
		fill(id, sizeof id, 0xa5);
		fill(user, sizeof user, 0xa5);
		fill(line, sizeof line, 0xa5);
		fill(host, sizeof host, 0xa5);
		host[pos] = 0;
		memset(futmplA, 0x7f, FU_ALLOC);
		fu_set(futmplA, H_USER_PROCESS, 987654321, id, 9, user, line,
		    host);
		check_futx_to_utx();
	}
}

static void
sweep_utx(long long iters)
{
	Rng rng(0xB0132S2Au);
	unsigned char id[8], user[32], line[16], host[128];

	for (long long it = 0; it < iters; it++) {
		short type;
		if (rng.below(2))
			type = (short)rng.below(11);
		else
			type = (short)(int16_t)rng.u32();

		for (size_t i = 0; i < sizeof id; i++)
			id[i] = (unsigned char)rng.byte();
		for (size_t i = 0; i < sizeof user; i++)
			user[i] = (unsigned char)rng.byte();
		for (size_t i = 0; i < sizeof line; i++)
			line[i] = (unsigned char)rng.byte();
		for (size_t i = 0; i < sizeof host; i++)
			host[i] = (unsigned char)rng.byte();
		if (rng.below(2)) {
			for (size_t i = 0; i < sizeof id; i++)
				if (id[i] == 0)
					id[i] = 0xd7;
			for (size_t i = 0; i < sizeof user; i++)
				if (user[i] == 0)
					user[i] = 0xd7;
			for (size_t i = 0; i < sizeof line; i++)
				if (line[i] == 0)
					line[i] = 0xd7;
			for (size_t i = 0; i < sizeof host; i++)
				if (host[i] == 0)
					host[i] = 0xd7;
		}

		int32_t pid = (int32_t)rng.u32();
		int64_t sec, usec;
		switch (rng.below(4)) {
		case 0: sec = (int64_t)(rng.u32() & 0xffff); break;
		case 1: sec = (int64_t)(int32_t)rng.u32(); break;
		case 2: sec = (int64_t)rng.next(); break;
		default: sec = 0; break;
		}
		switch (rng.below(4)) {
		case 0: usec = (int64_t)rng.below(1000000); break;
		case 1: usec = (int64_t)(int32_t)rng.u32(); break;
		case 2: usec = (int64_t)rng.next(); break;
		default: usec = 0; break;
		}

		memset(utmplA, 0x7f, UT_ALLOC);
		ut_set(utmplA, type, id, pid, user, line, host, sec, usec);
		g_faketv.tv_sec = (time_t)sec;
		g_faketv.tv_usec = (suseconds_t)usec;
		check_utx_to_futx();
	}
}

static void
sweep_ftou(long long iters)
{
	Rng rng(0x5EED0132u);
	unsigned char id[8], user[32], line[16], host[128];

	for (long long it = 0; it < iters; it++) {
		unsigned type;
		if (rng.below(2))
			type = rng.below(11);
		else
			type = (unsigned)rng.byte();

		for (size_t i = 0; i < sizeof id; i++)
			id[i] = (unsigned char)rng.byte();
		for (size_t i = 0; i < sizeof user; i++)
			user[i] = (unsigned char)rng.byte();
		for (size_t i = 0; i < sizeof line; i++)
			line[i] = (unsigned char)rng.byte();
		for (size_t i = 0; i < sizeof host; i++)
			host[i] = (unsigned char)rng.byte();
		if (rng.below(2)) {
			for (size_t i = 0; i < sizeof id; i++)
				if (id[i] == 0)
					id[i] = 0x3b;
			for (size_t i = 0; i < sizeof user; i++)
				if (user[i] == 0)
					user[i] = 0x3b;
			for (size_t i = 0; i < sizeof line; i++)
				if (line[i] == 0)
					line[i] = 0x3b;
			for (size_t i = 0; i < sizeof host; i++)
				if (host[i] == 0)
					host[i] = 0x3b;
		}

		uint64_t tv;
		switch (rng.below(4)) {
		case 0: tv = rng.below(2000000); break;
		case 1: tv = rng.next() % 4000000000ULL; break;
		case 2: tv = rng.next(); break;
		default: tv = (uint64_t)rng.below(1000000) +
		    1000000ULL * rng.below(1000000); break;
		}
		uint32_t pid = rng.u32();

		memset(futmplA, 0x7f, FU_ALLOC);
		fu_set(futmplA, type, tv, id, pid, user, line, host);
		check_futx_to_utx();
	}
}

static void
on_alarm(int)
{
	const char m[] = "harness: watchdog expired\n";
	ssize_t r = write(2, m, sizeof m - 1);
	(void)r;
	_exit(1);
}

int
main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(600);

	setvbuf(stdout, NULL, _IOLBF, 0);

	{
		unsigned char id[8], user[32], line[16], host[128];
		memset(id, 0xa1, sizeof id);
		memset(user, 0xa2, sizeof user);
		memset(line, 0xa3, sizeof line);
		memset(host, 0xa4, sizeof host);
		memset(futmplA, 0x7f, FU_ALLOC);
		fu_set(futmplA, H_USER_PROCESS, 1234567, id, 4242, user, line,
		    host);
		memcpy(fibufA, futmplA, FU_ALLOC);
		memcpy(fibufB, futmplA, FU_ALLOC);

		st[S_FTOU].cases++;
		g_calloc_fail = 1;
		struct utmpx *ra =
		    ref_futx_to_utx((const struct futx *)(fibufA + FU_OFF));
		g_calloc_fail = 1;
		struct utmpx *rb =
		    P::futx_to_utx((const struct futx *)(fibufB + FU_OFF));
		g_calloc_fail = 0;
		if (ra != NULL || rb != NULL)
			note(S_FTOU,
			    "allocation failure did not yield NULL from both");
		if (memcmp(fibufA, fibufB, FU_ALLOC) != 0)
			note(S_FTOU, "input futx buffer differs (calloc fail)");
	}

	hand_utx();
	hand_ftou();

	sweep_utx(200000);
	sweep_ftou(200000);

	long long total_fail = 0;
	printf("\n%-22s %12s %10s  %s\n", "function", "cases", "failures",
	    "result");
	printf("%-22s %12s %10s  %s\n", "----------------------",
	    "------------", "----------", "------");
	for (int i = 0; i < S_NFUNC; i++) {
		total_fail += st[i].fails;
		printf("%-22s %12lld %10lld  %s\n", st[i].name, st[i].cases,
		    st[i].fails, st[i].fails == 0 ? "PASS" : "FAIL");
	}
	printf("\n%s: %lld failures\n", total_fail == 0 ? "ALL PASS" : "FAILED",
	    total_fail);
	return total_fail == 0 ? 0 : 1;
}
