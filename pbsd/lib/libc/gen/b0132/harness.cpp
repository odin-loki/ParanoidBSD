/*
 * b0132 differential harness.
 *
 * Every ported function is driven with hand-written edge cases and with a
 * fixed-seed randomised sweep of >= 200000 iterations, and the result of the
 * C++ port is compared against the untouched C oracle in oracle.c.
 *
 * The libc-internal services the original readdir.c relies on
 * (_getdirentries, _fixtelldir, _pthread_mutex_lock/_unlock, __isthreaded)
 * and gettimeofday() are provided here as deterministic, call-logging test
 * doubles, so that both implementations observe exactly the same environment
 * and any difference in the sequence or arguments of those calls is caught.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/time.h>
#include <endian.h>

#define PBSD_B0132_SHARED 1
#include "port.cppm"
#undef PBSD_B0132_SHARED

import pbsd.lib.libc.gen.b0132;

namespace P = pbsd::lib_libc_gen::b0132;

/* ------------------------------------------------------------------------ */
/* The oracle.                                                              */
/* ------------------------------------------------------------------------ */

extern "C" {
struct dirent *ref__readdir_unlocked(DIR *dirp, int flags);
struct dirent *ref_readdir(DIR *dirp);
int ref___readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);
void ref_utx_to_futx(const struct utmpx *ut, struct futx *fu);
struct utmpx *ref_futx_to_utx(const struct futx *fu);
int ref_gettype(char *t, const char **names);
}

/* ------------------------------------------------------------------------ */
/* Harness-private constants.  Deliberately *not* shared with the port, so   */
/* that a mutation of the port's own constants cannot silently move the      */
/* harness with it.                                                          */
/* ------------------------------------------------------------------------ */

#define H_DTF_HIDEW		0x0001
#define H_DTF_NODUP		0x0002
#define H_DTF_REWIND		0x0004
#define H_DTF_READALL		0x0008
#define H_DTF_SKIPREAD		0x0010

#define H_RDU_SKIP		0x0001
#define H_RDU_SHORT		0x0002

#define H_DT_WHT		14

#define H_EMPTY			0
#define H_BOOT_TIME		1
#define H_OLD_TIME		2
#define H_NEW_TIME		3
#define H_USER_PROCESS		4
#define H_INIT_PROCESS		5
#define H_LOGIN_PROCESS		6
#define H_DEAD_PROCESS		7
#define H_SHUTDOWN_TIME		8

#define H_DNAME_OFF		24u	/* offsetof(struct dirent, d_name) */
#define H_DIRLEN(namlen)	((H_DNAME_OFF + (size_t)(namlen) + 1 + 7) & ~(size_t)7)

#define ALLOC		4096	/* dd_buf allocation, guard bytes included */
#define EALLOC		1024	/* readdir_r entry allocation                */
#define ENTOFF		16	/* where in EALLOC the entry sits            */
#define MAXDDLEN	3000
#define SCRIPTMAX	4
#define DATAMAX		3072

/* ------------------------------------------------------------------------ */
/* Test doubles.                                                            */
/* ------------------------------------------------------------------------ */

struct LogEnt {
	int kind;
	long long a, b, c;
};

struct Log {
	int n;
	LogEnt e[64];
};

static Log g_log;

static void
logadd(int kind, long long a, long long b, long long c)
{
	if (g_log.n < 64) {
		g_log.e[g_log.n].kind = kind;
		g_log.e[g_log.n].a = a;
		g_log.e[g_log.n].b = b;
		g_log.e[g_log.n].c = c;
	}
	g_log.n++;
}

static bool
log_equal(const Log &x, const Log &y)
{
	if (x.n != y.n)
		return false;
	int n = x.n < 64 ? x.n : 64;
	for (int i = 0; i < n; i++)
		if (x.e[i].kind != y.e[i].kind || x.e[i].a != y.e[i].a ||
		    x.e[i].b != y.e[i].b || x.e[i].c != y.e[i].c)
			return false;
	return true;
}

struct GdRes {
	ssize_t ret;
	int seterrno;
	size_t nbytes;
	off_t newseek;
	unsigned char data[DATAMAX];
};

struct GdScript {
	int n;
	GdRes r[SCRIPTMAX];
};

static GdScript g_script;
static int g_gd_idx;
static long long g_gd_total;

extern "C" {
int __isthreaded = 0;
}

extern "C" ssize_t
_getdirentries(int fd, char *buf, size_t nbytes, off_t *basep)
{
	logadd(1, fd, (long long)nbytes, (long long)*basep);
	if (++g_gd_total > 200000000LL) {
		fprintf(stderr, "harness: runaway _getdirentries\n");
		_exit(1);
	}
	if (g_gd_idx >= g_script.n) {
		*basep = *basep + 1;
		return 0;
	}
	const GdRes &r = g_script.r[g_gd_idx++];
	if (r.ret > 0) {
		size_t n = r.nbytes < nbytes ? r.nbytes : nbytes;
		if (n > 0)
			memcpy(buf, r.data, n);
	}
	*basep = r.newseek;
	if (r.seterrno != 0)
		errno = r.seterrno;
	return r.ret;
}

extern "C" void
_fixtelldir(DIR *dirp, off_t oldseek, size_t oldloc)
{
	logadd(2, (long long)oldseek, (long long)oldloc,
	    (long long)dirp->dd_loc);
}

extern "C" int
_pthread_mutex_lock(pthread_mutex_t *m)
{
	logadd(3, 0, 0, 0);
	return pthread_mutex_lock(m);
}

extern "C" int
_pthread_mutex_unlock(pthread_mutex_t *m)
{
	logadd(4, 0, 0, 0);
	return pthread_mutex_unlock(m);
}

/* Deterministic clock, so that utx_to_futx()'s UTOF_TV is comparable. */
static struct timeval g_faketv;

extern "C" int
gettimeofday(struct timeval *__restrict tv, void *__restrict tz) noexcept
{
	(void)tz;
	logadd(5, 0, 0, 0);
	if (tv != NULL)
		*tv = g_faketv;
	return 0;
}

/* calloc interposition, so futx_to_utx()'s allocation-failure path is real. */
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

/* ------------------------------------------------------------------------ */
/* Bookkeeping.                                                             */
/* ------------------------------------------------------------------------ */

enum { S_GETTYPE, S_UTOF, S_FTOU, S_RDU, S_RD, S_RDR, S_NFUNC };

struct Stat {
	const char *name;
	long long cases;
	long long fails;
	int shown;
};

static Stat st[S_NFUNC] = {
	{ "gettype", 0, 0, 0 },
	{ "utx_to_futx", 0, 0, 0 },
	{ "futx_to_utx", 0, 0, 0 },
	{ "_readdir_unlocked", 0, 0, 0 },
	{ "readdir", 0, 0, 0 },
	{ "__readdir_r", 0, 0, 0 },
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

/* ------------------------------------------------------------------------ */
/* PRNG (splitmix64, fixed seed).                                           */
/* ------------------------------------------------------------------------ */

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

/* ------------------------------------------------------------------------ */
/* gettype()                                                                */
/* ------------------------------------------------------------------------ */

#define GT_ALLOC	256
#define GT_NAMES	16
#define GT_NAMELEN	72

static unsigned char gtA[GT_ALLOC], gtB[GT_ALLOC];
static char gnA[GT_NAMES][GT_NAMELEN], gnB[GT_NAMES][GT_NAMELEN];
static const char *gpA[GT_NAMES + 1], *gpB[GT_NAMES + 1];

static void
check_gettype(const char *t, size_t tlen, const char *const *names, int nnames)
{
	st[S_GETTYPE].cases++;

	memset(gtA, 0x7f, GT_ALLOC);
	memset(gtB, 0x7f, GT_ALLOC);
	memcpy(gtA + 8, t, tlen);
	gtA[8 + tlen] = '\0';
	memcpy(gtB + 8, t, tlen);
	gtB[8 + tlen] = '\0';

	memset(gnA, 0x7f, sizeof gnA);
	memset(gnB, 0x7f, sizeof gnB);
	for (int i = 0; i < nnames; i++) {
		size_t l = strlen(names[i]);
		memcpy(gnA[i], names[i], l + 1);
		memcpy(gnB[i], names[i], l + 1);
		gpA[i] = gnA[i];
		gpB[i] = gnB[i];
	}
	gpA[nnames] = NULL;
	gpB[nnames] = NULL;

	int ra = ref_gettype((char *)gtA + 8, gpA);
	int rb = P::gettype((char *)gtB + 8, gpB);

	if (ra != rb) {
		char msg[160];
		snprintf(msg, sizeof msg, "ret %d vs %d (t=\"%s\", %d names)",
		    ra, rb, (const char *)gtA + 8, nnames);
		note(S_GETTYPE, msg);
		return;
	}
	if (memcmp(gtA, gtB, GT_ALLOC) != 0) {
		note(S_GETTYPE, "argument buffer differs");
		return;
	}
	if (memcmp(gnA, gnB, sizeof gnA) != 0)
		note(S_GETTYPE, "names buffer differs");
}

static void
hand_gettype(void)
{
	static const char *n0[] = { NULL };
	static const char *n1[] = { "unknown" };
	static const char *n4[] = { "unknown", "SMD", "ESDI", "SCSI" };
	static const char *nempty[] = { "a", "", "b" };
	static const char *nhigh[] = { "\x80\x81", "\xff", "A\xc3\xa9" };
	static const char *ndigit[] = { "0", "1", "12" };
	static const char *nlong[] = {
		"0123456789012345678901234567890",	/* 31 */
		"01234567890123456789012345678901",	/* 32 */
		"a"
	};

	struct { const char *t; const char *const *nm; int n; } tab[] = {
		{ "", n0, 0 },
		{ "a", n0, 0 },
		{ "0", n0, 0 },
		{ "9", n0, 0 },
		{ "5xyz", n0, 0 },
		{ "-5", n0, 0 },
		{ "+7", n0, 0 },
		{ " 3", n0, 0 },
		{ "2147483647", n0, 0 },
		{ "\x80", n0, 0 },
		{ "\xff\xfe", n0, 0 },

		{ "", n1, 1 },
		{ "unknown", n1, 1 },
		{ "UNKNOWN", n1, 1 },
		{ "UnKnOwN", n1, 1 },
		{ "unknow", n1, 1 },
		{ "unknownx", n1, 1 },
		{ "3", n1, 1 },

		{ "unknown", n4, 4 },
		{ "smd", n4, 4 },
		{ "SMD", n4, 4 },
		{ "esdi", n4, 4 },
		{ "scsi", n4, 4 },
		{ "SCSi", n4, 4 },
		{ "nope", n4, 4 },
		{ "2", n4, 4 },
		{ "3zzz", n4, 4 },
		{ "", n4, 4 },
		{ "\x80", n4, 4 },

		{ "", nempty, 3 },
		{ "a", nempty, 3 },
		{ "b", nempty, 3 },
		{ "c", nempty, 3 },

		{ "\x80\x81", nhigh, 3 },
		{ "\xff", nhigh, 3 },
		{ "A\xc3\xa9", nhigh, 3 },
		{ "a\xc3\xa9", nhigh, 3 },
		{ "\x80", nhigh, 3 },

		{ "0", ndigit, 3 },
		{ "1", ndigit, 3 },
		{ "12", ndigit, 3 },
		{ "13", ndigit, 3 },
		{ "007", ndigit, 3 },

		{ "0123456789012345678901234567890", nlong, 3 },
		{ "01234567890123456789012345678901", nlong, 3 },
		{ "012345678901234567890123456789012", nlong, 3 },
		{ "a", nlong, 3 },
	};

	for (size_t i = 0; i < sizeof tab / sizeof tab[0]; i++)
		check_gettype(tab[i].t, strlen(tab[i].t), tab[i].nm, tab[i].n);

	/* NUL-heavy: t is empty because of a leading NUL, trailing junk. */
	{
		char t[8] = { '\0', '7', '7', '\0' };
		check_gettype(t, 0, n4, 4);
	}
	/* Single high-bit character against a one-name table. */
	{
		const char *nm[] = { "\x80" };
		check_gettype("\x80", 1, nm, 1);
	}
	/* Boundary lengths 0..3 against a table whose last entry matches. */
	{
		const char *nm[] = { "zz", "z", "" };
		check_gettype("", 0, nm, 3);
		check_gettype("z", 1, nm, 3);
		check_gettype("zz", 2, nm, 3);
		check_gettype("zzz", 3, nm, 3);
	}
}

static void
sweep_gettype(long long iters)
{
	Rng rng(0xB0132A11u);
	static const char alpha[] =
	    "abcABC019zZ\x80\xff\xa5 _-+";
	const size_t nalpha = sizeof alpha - 1;
	char names[GT_NAMES][GT_NAMELEN];
	const char *nm[GT_NAMES];
	char t[GT_NAMELEN];

	for (long long it = 0; it < iters; it++) {
		int nnames = (int)rng.below(GT_NAMES + 1);
		for (int i = 0; i < nnames; i++) {
			int l = (int)rng.below(14);
			for (int j = 0; j < l; j++)
				names[i][j] = alpha[rng.below((uint32_t)nalpha)];
			names[i][l] = '\0';
			nm[i] = names[i];
		}

		int mode = (int)rng.below(10);
		size_t tlen;
		if (mode < 4 && nnames > 0) {
			/* copy of some entry, with random case flipping */
			int j = (int)rng.below((uint32_t)nnames);
			tlen = strlen(names[j]);
			memcpy(t, names[j], tlen + 1);
			for (size_t k = 0; k < tlen; k++)
				if (rng.below(2)) {
					unsigned char ch = (unsigned char)t[k];
					if (isalpha(ch))
						t[k] = (char)(islower(ch) ?
						    toupper(ch) : tolower(ch));
				}
			/* sometimes perturb it by one byte or one length */
			if (rng.below(4) == 0 && tlen > 0)
				t[rng.below((uint32_t)tlen)] =
				    alpha[rng.below((uint32_t)nalpha)];
			else if (rng.below(8) == 0 && tlen + 1 < sizeof t) {
				t[tlen] = alpha[rng.below((uint32_t)nalpha)];
				t[++tlen] = '\0';
			}
		} else if (mode < 6) {
			/* digit-led */
			int l = 1 + (int)rng.below(9);
			for (int j = 0; j < l; j++)
				t[j] = (char)('0' + rng.below(10));
			if (rng.below(2))
				t[l - 1] = alpha[rng.below((uint32_t)nalpha)];
			t[l] = '\0';
			tlen = (size_t)l;
		} else if (mode < 7) {
			tlen = 0;
			t[0] = '\0';
		} else {
			int l = (int)rng.below(16);
			for (int j = 0; j < l; j++)
				t[j] = alpha[rng.below((uint32_t)nalpha)];
			t[l] = '\0';
			tlen = (size_t)l;
		}
		check_gettype(t, tlen, nm, nnames);
	}
}

/* ------------------------------------------------------------------------ */
/* utx_to_futx() / futx_to_utx()                                            */
/* ------------------------------------------------------------------------ */

#define UT_ALLOC	512
#define FU_ALLOC	512
#define UT_OFF		8
#define FU_OFF		8

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

/* Field writers that do not depend on the port's own struct definitions. */
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

/* struct futx is packed: type 0, tv 1..8, id 9..12, pid 13..16, user 17..48,
 * line 49..64, host 65..192. */
static void
fu_set(unsigned char *b, unsigned type, uint64_t tv_be_value,
    const unsigned char *id, uint32_t pid_be_value,
    const unsigned char *user, const unsigned char *line,
    const unsigned char *host)
{
	unsigned char *f = b + FU_OFF;
	memset(f, 0, 193);
	f[0] = (unsigned char)type;
	uint64_t tvbe = htobe64(tv_be_value);
	memcpy(f + 1, &tvbe, 8);
	memcpy(f + 9, id, 4);
	uint32_t pbe = htobe32(pid_be_value);
	memcpy(f + 13, &pbe, 4);
	memcpy(f + 17, user, 32);
	memcpy(f + 49, line, 16);
	memcpy(f + 65, host, 128);
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

	/* NUL placement boundaries inside every string field. */
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
	/* host NUL near its own boundary. */
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
	unsigned char id[4], user[32], line[16], host[128];
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

	/* NUL placement boundaries: catches an off-by-one in the MIN()s. */
	for (unsigned type = 0; type <= 9; type++) {
		for (int pos = 0; pos < 34; pos++) {
			fill(id, sizeof id, 0x5a);
			fill(user, sizeof user, 0x5a);
			fill(line, sizeof line, 0x5a);
			fill(host, sizeof host, 0x5a);
			if (pos < 4)
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
	Rng rng(0xB0132A7Fu);
	unsigned char id[8], user[32], line[16], host[128];

	for (long long it = 0; it < iters; it++) {
		short type;
		if (rng.below(2))
			type = (short)rng.below(11);	/* in-range types */
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
		/* Frequently make the fields NUL-free so truncation shows. */
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
	unsigned char id[4], user[32], line[16], host[128];

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

/* ------------------------------------------------------------------------ */
/* readdir family                                                           */
/* ------------------------------------------------------------------------ */

struct DirCase {
	int bufoff;
	int dd_len;
	size_t dd_loc;
	size_t dd_size;
	int dd_flags;
	off_t dd_seek;
	int fd;
	int flags;
	int isthreaded;
	int errno_in;
};

struct Outcome {
	long long retoff;
	size_t dd_loc, dd_size;
	int dd_len, dd_flags, dd_fd;
	off_t dd_seek;
	int err;
	int rc;
	long long resoff;
	int still_locked;
	Log log;
};

static unsigned char g_tmpl[ALLOC];
static unsigned char *dbufA, *dbufB, *ebufA, *ebufB;

static void
put16(unsigned char *p, unsigned v)
{
	p[0] = (unsigned char)(v & 0xff);
	p[1] = (unsigned char)((v >> 8) & 0xff);
}

static void
put64(unsigned char *p, uint64_t v)
{
	for (int i = 0; i < 8; i++)
		p[i] = (unsigned char)((v >> (8 * i)) & 0xff);
}

static void
run_side(bool use_ref, int which, const DirCase &c, unsigned char *dbuf,
    unsigned char *ebuf, Outcome &o)
{
	memcpy(dbuf, g_tmpl, ALLOC);
	memset(ebuf, 0x7f, EALLOC);

	DIR d;
	memset(&d, 0, sizeof d);
	d.dd_fd = c.fd;
	d.dd_loc = c.dd_loc;
	d.dd_size = c.dd_size;
	d.dd_buf = (char *)dbuf + c.bufoff;
	d.dd_len = c.dd_len;
	d.dd_seek = c.dd_seek;
	d.dd_flags = c.dd_flags;
	d.dd_td = NULL;
	pthread_mutex_init(&d.dd_lock, NULL);

	g_gd_idx = 0;
	g_log.n = 0;
	__isthreaded = c.isthreaded;
	errno = c.errno_in;

	struct dirent *dp = NULL;
	struct dirent *res = (struct dirent *)(intptr_t)-3;
	int rc = 0;

	switch (which) {
	case 0:
		dp = use_ref ? ref__readdir_unlocked(&d, c.flags)
			     : P::_readdir_unlocked(&d, c.flags);
		break;
	case 1:
		dp = use_ref ? ref_readdir(&d) : P::readdir(&d);
		break;
	default:
		rc = use_ref
		    ? ref___readdir_r(&d, (struct dirent *)(ebuf + ENTOFF), &res)
		    : P::__readdir_r(&d, (struct dirent *)(ebuf + ENTOFF), &res);
		break;
	}

	o.err = errno;
	o.rc = rc;
	o.retoff = (dp == NULL) ? -1 : (long long)((char *)dp - (char *)dbuf);
	if (which == 2) {
		if (res == NULL)
			o.resoff = -1;
		else if (res == (struct dirent *)(ebuf + ENTOFF))
			o.resoff = 0;
		else
			o.resoff = -2;
	} else
		o.resoff = 0;
	o.dd_loc = d.dd_loc;
	o.dd_size = d.dd_size;
	o.dd_len = d.dd_len;
	o.dd_flags = d.dd_flags;
	o.dd_fd = d.dd_fd;
	o.dd_seek = d.dd_seek;
	o.log = g_log;

	if (pthread_mutex_trylock(&d.dd_lock) == 0) {
		o.still_locked = 0;
		pthread_mutex_unlock(&d.dd_lock);
	} else
		o.still_locked = 1;
	pthread_mutex_destroy(&d.dd_lock);
}

static void
check_dir(int which, const DirCase &c)
{
	int idx = (which == 0) ? S_RDU : (which == 1) ? S_RD : S_RDR;
	st[idx].cases++;

	Outcome a, b;
	run_side(true, which, c, dbufA, ebufA, a);
	run_side(false, which, c, dbufB, ebufB, b);

	char msg[256];

	if (a.retoff != b.retoff) {
		snprintf(msg, sizeof msg, "return offset %lld vs %lld",
		    a.retoff, b.retoff);
		note(idx, msg);
		return;
	}
	if (a.rc != b.rc) {
		snprintf(msg, sizeof msg, "rc %d vs %d", a.rc, b.rc);
		note(idx, msg);
		return;
	}
	if (a.resoff != b.resoff) {
		snprintf(msg, sizeof msg, "*result %lld vs %lld", a.resoff,
		    b.resoff);
		note(idx, msg);
		return;
	}
	if (a.err != b.err) {
		snprintf(msg, sizeof msg, "errno %d vs %d", a.err, b.err);
		note(idx, msg);
		return;
	}
	if (a.dd_loc != b.dd_loc || a.dd_size != b.dd_size ||
	    a.dd_len != b.dd_len || a.dd_flags != b.dd_flags ||
	    a.dd_fd != b.dd_fd || a.dd_seek != b.dd_seek) {
		snprintf(msg, sizeof msg,
		    "DIR state loc %zu/%zu size %zu/%zu len %d/%d "
		    "flags %d/%d seek %lld/%lld",
		    a.dd_loc, b.dd_loc, a.dd_size, b.dd_size, a.dd_len,
		    b.dd_len, a.dd_flags, b.dd_flags, (long long)a.dd_seek,
		    (long long)b.dd_seek);
		note(idx, msg);
		return;
	}
	if (a.still_locked != b.still_locked) {
		note(idx, "mutex left in a different state");
		return;
	}
	if (!log_equal(a.log, b.log)) {
		snprintf(msg, sizeof msg, "libc call log differs (%d vs %d)",
		    a.log.n, b.log.n);
		note(idx, msg);
		return;
	}
	if (memcmp(dbufA, dbufB, ALLOC) != 0) {
		int off = 0;
		while (off < ALLOC && dbufA[off] == dbufB[off])
			off++;
		snprintf(msg, sizeof msg,
		    "dd_buf differs at byte %d (%02x vs %02x)", off,
		    dbufA[off], dbufB[off]);
		note(idx, msg);
		return;
	}
	if (memcmp(ebufA, ebufB, EALLOC) != 0) {
		int off = 0;
		while (off < EALLOC && ebufA[off] == ebufB[off])
			off++;
		snprintf(msg, sizeof msg,
		    "entry buffer differs at byte %d (%02x vs %02x)", off,
		    ebufA[off], ebufB[off]);
		note(idx, msg);
	}
}

/* Drive a DIR to exhaustion, comparing after every single step. */
static void
check_dir_iterate(int which, const DirCase &c0, int maxsteps)
{
	int idx = (which == 0) ? S_RDU : (which == 1) ? S_RD : S_RDR;

	memcpy(dbufA, g_tmpl, ALLOC);
	memcpy(dbufB, g_tmpl, ALLOC);
	memset(ebufA, 0x7f, EALLOC);
	memset(ebufB, 0x7f, EALLOC);

	DIR da, db;
	memset(&da, 0, sizeof da);
	da.dd_fd = c0.fd;
	da.dd_loc = c0.dd_loc;
	da.dd_size = c0.dd_size;
	da.dd_buf = (char *)dbufA + c0.bufoff;
	da.dd_len = c0.dd_len;
	da.dd_seek = c0.dd_seek;
	da.dd_flags = c0.dd_flags;
	da.dd_td = NULL;
	pthread_mutex_init(&da.dd_lock, NULL);
	db = da;
	db.dd_buf = (char *)dbufB + c0.bufoff;
	pthread_mutex_init(&db.dd_lock, NULL);

	int gd_a = 0, gd_b = 0;
	__isthreaded = c0.isthreaded;

	for (int step = 0; step < maxsteps; step++) {
		st[idx].cases++;

		struct dirent *dpa = NULL, *dpb = NULL;
		struct dirent *resa = (struct dirent *)(intptr_t)-3;
		struct dirent *resb = (struct dirent *)(intptr_t)-3;
		int rca = 0, rcb = 0;

		g_gd_idx = gd_a;
		g_log.n = 0;
		errno = c0.errno_in;
		switch (which) {
		case 0: dpa = ref__readdir_unlocked(&da, c0.flags); break;
		case 1: dpa = ref_readdir(&da); break;
		default:
			rca = ref___readdir_r(&da,
			    (struct dirent *)(ebufA + ENTOFF), &resa);
			break;
		}
		int erra = errno;
		Log la = g_log;
		gd_a = g_gd_idx;

		g_gd_idx = gd_b;
		g_log.n = 0;
		errno = c0.errno_in;
		switch (which) {
		case 0: dpb = P::_readdir_unlocked(&db, c0.flags); break;
		case 1: dpb = P::readdir(&db); break;
		default:
			rcb = P::__readdir_r(&db,
			    (struct dirent *)(ebufB + ENTOFF), &resb);
			break;
		}
		int errb = errno;
		Log lb = g_log;
		gd_b = g_gd_idx;

		long long oa = dpa == NULL ? -1
		    : (long long)((char *)dpa - (char *)dbufA);
		long long ob = dpb == NULL ? -1
		    : (long long)((char *)dpb - (char *)dbufB);
		/*
		 * __readdir_r()'s errno early-return leaves *result alone, so
		 * normalise instead of subtracting from two different bases.
		 */
		long long ra = resa == NULL ? -1
		    : (resa == (struct dirent *)(ebufA + ENTOFF) ? 0 : -2);
		long long rb = resb == NULL ? -1
		    : (resb == (struct dirent *)(ebufB + ENTOFF) ? 0 : -2);

		char msg[256];
		bool bad = false;
		if (oa != ob) {
			snprintf(msg, sizeof msg,
			    "step %d: token offset %lld vs %lld", step, oa, ob);
			note(idx, msg);
			bad = true;
		} else if (rca != rcb) {
			snprintf(msg, sizeof msg, "step %d: rc %d vs %d", step,
			    rca, rcb);
			note(idx, msg);
			bad = true;
		} else if (erra != errb) {
			snprintf(msg, sizeof msg, "step %d: errno %d vs %d",
			    step, erra, errb);
			note(idx, msg);
			bad = true;
		} else if (which == 2 && ra != rb) {
			snprintf(msg, sizeof msg,
			    "step %d: *result %lld vs %lld", step, ra, rb);
			note(idx, msg);
			bad = true;
		} else if (da.dd_loc != db.dd_loc || da.dd_size != db.dd_size ||
		    da.dd_len != db.dd_len || da.dd_flags != db.dd_flags ||
		    da.dd_seek != db.dd_seek || da.dd_fd != db.dd_fd) {
			snprintf(msg, sizeof msg,
			    "step %d: DIR state loc %zu/%zu size %zu/%zu "
			    "flags %d/%d seek %lld/%lld",
			    step, da.dd_loc, db.dd_loc, da.dd_size, db.dd_size,
			    da.dd_flags, db.dd_flags, (long long)da.dd_seek,
			    (long long)db.dd_seek);
			note(idx, msg);
			bad = true;
		} else if (gd_a != gd_b) {
			snprintf(msg, sizeof msg,
			    "step %d: script consumption %d vs %d", step, gd_a,
			    gd_b);
			note(idx, msg);
			bad = true;
		} else if (!log_equal(la, lb)) {
			snprintf(msg, sizeof msg, "step %d: call log differs",
			    step);
			note(idx, msg);
			bad = true;
		} else if (memcmp(dbufA, dbufB, ALLOC) != 0) {
			snprintf(msg, sizeof msg, "step %d: dd_buf differs",
			    step);
			note(idx, msg);
			bad = true;
		} else if (memcmp(ebufA, ebufB, EALLOC) != 0) {
			snprintf(msg, sizeof msg,
			    "step %d: entry buffer differs", step);
			note(idx, msg);
			bad = true;
		}
		if (bad)
			break;
		if (dpa == NULL && !(which == 2 && ra == 0))
			break;	/* exhausted */
	}
	pthread_mutex_destroy(&da.dd_lock);
	pthread_mutex_destroy(&db.dd_lock);
}

struct Chain {
	int npos;
	size_t pos[192];
};

static void
build_chain(unsigned char *dst, int bufoff, int dd_len, Rng &rng, int perturb,
    Chain *ch)
{
	memset(dst, 0x7f, ALLOC);
	unsigned char *b = dst + bufoff;
	size_t pos = 0;

	if (ch != NULL)
		ch->npos = 0;
	while (pos + H_DIRLEN(0) <= (size_t)dd_len) {
		unsigned namlen;
		unsigned r = rng.below(10);
		if (r < 3)
			namlen = rng.below(8);
		else if (r < 7)
			namlen = 248 + rng.below(16);	/* around 256 */
		else
			namlen = rng.below(401);
		size_t reclen = H_DIRLEN(namlen);
		if (pos + reclen > (size_t)dd_len) {
			namlen = rng.below(8);
			reclen = H_DIRLEN(namlen);
			if (pos + reclen > (size_t)dd_len)
				break;
		}

		size_t stored = reclen;
		if (perturb) {
			switch (rng.below(16)) {
			case 0: stored = 0; break;
			case 1: stored = (size_t)dd_len + 1 - pos; break;
			case 2: stored = (size_t)dd_len + 2 - pos; break;
			case 3: stored = reclen + 1; break;
			case 4: stored = 8; break;
			case 5: stored = 65535; break;
			case 6: stored = 1; break;
			default: break;
			}
		}

		uint64_t ino = rng.below(3) == 0 ? 0 : (rng.next() | 1);
		unsigned type = rng.below(3) == 0 ? H_DT_WHT : rng.below(16);

		put64(b + pos, ino);
		put64(b + pos + 8, (uint64_t)rng.next());
		put16(b + pos + 16, (unsigned)(stored & 0xffffu));
		b[pos + 18] = (unsigned char)type;
		b[pos + 19] = (unsigned char)rng.byte();
		put16(b + pos + 20, namlen & 0xffffu);
		put16(b + pos + 22, (unsigned)rng.below(65536));
		for (unsigned i = 0; i <= namlen && H_DNAME_OFF + i < reclen;
		    i++)
			b[pos + H_DNAME_OFF + i] = (unsigned char)rng.byte();

		if (ch != NULL && ch->npos < 192)
			ch->pos[ch->npos++] = pos;
		pos += reclen;
	}
}

static void
build_random(unsigned char *dst, int bufoff, int dd_len, Rng &rng)
{
	memset(dst, 0x7f, ALLOC);
	for (int i = 0; i < dd_len; i++)
		dst[bufoff + i] = (unsigned char)rng.byte();
}

static void
gen_script(int dd_len, Rng &rng, int perturb, int allow_random)
{
	static unsigned char scratch[ALLOC];

	g_script.n = 1 + (int)rng.below(SCRIPTMAX);
	for (int i = 0; i < g_script.n; i++) {
		GdRes &r = g_script.r[i];
		switch (rng.below(8)) {
		case 0: r.ret = -1; break;
		case 1: r.ret = 0; break;
		case 2: r.ret = dd_len; break;
		case 3: r.ret = 1; break;
		default: r.ret = (ssize_t)(1 + rng.below((uint32_t)dd_len));
			break;
		}
		if (!allow_random || rng.below(2))
			build_chain(scratch, 0, dd_len, rng, perturb, NULL);
		else
			build_random(scratch, 0, dd_len, rng);
		r.nbytes = (size_t)dd_len;
		memcpy(r.data, scratch, (size_t)dd_len);
		r.newseek = (off_t)(int64_t)(rng.next() & 0xffffff);
		r.seterrno = rng.below(4) == 0
		    ? (int)(1 + rng.below(40)) : 0;
	}
}

/* --- hand-written readdir cases ----------------------------------------- */

struct Rec {
	uint64_t ino;
	unsigned type;
	unsigned reclen;
	unsigned namlen;
};

static void
lay(unsigned char *b, size_t pos, const Rec &r)
{
	put64(b + pos, r.ino);
	put64(b + pos + 8, 0x1122334455667788ULL);
	put16(b + pos + 16, r.reclen & 0xffffu);
	b[pos + 18] = (unsigned char)r.type;
	b[pos + 19] = 0x22;
	put16(b + pos + 20, r.namlen & 0xffffu);
	put16(b + pos + 22, 0x3344);
	for (unsigned i = 0; i < 260; i++)
		b[pos + H_DNAME_OFF + i] = (unsigned char)(0x80 + (i & 0x7f));
}

static void
one_script(ssize_t ret, const unsigned char *data, size_t len, off_t newseek,
    int seterr)
{
	g_script.n = 1;
	g_script.r[0].ret = ret;
	g_script.r[0].nbytes = len;
	g_script.r[0].newseek = newseek;
	g_script.r[0].seterrno = seterr;
	memset(g_script.r[0].data, 0x7f, DATAMAX);
	if (data != NULL && len > 0)
		memcpy(g_script.r[0].data, data, len);
}

static void
hand_dir(void)
{
	static unsigned char scratch[ALLOC];
	DirCase c;
	const int LEN = 1024;

	memset(&c, 0, sizeof c);
	c.bufoff = 8;
	c.dd_len = LEN;
	c.fd = 5;
	c.dd_seek = 0x1000;
	c.errno_in = 0;

	/* 1. dd_loc >= dd_size with __DTF_READALL -> NULL, no read at all. */
	memset(g_tmpl, 0x7f, ALLOC);
	one_script(0, NULL, 0, 0, 0);
	c.dd_loc = 0;
	c.dd_size = 0;
	c.dd_flags = H_DTF_READALL;
	for (int f = 0; f < 4; f++) {
		c.flags = f;
		for (int th = 0; th < 2; th++) {
			c.isthreaded = th;
			check_dir(0, c);
			check_dir(1, c);
			check_dir(2, c);
		}
	}

	/* 2. read returning -1 / 0 / a good buffer. */
	memset(scratch, 0x7f, ALLOC);
	lay(scratch, 0, Rec{ 7, 1, (unsigned)H_DIRLEN(4), 4 });
	for (int k = 0; k < 4; k++) {
		ssize_t ret = (k == 0) ? -1 : (k == 1) ? 0
		    : (k == 2) ? (ssize_t)H_DIRLEN(4) : LEN;
		one_script(ret, scratch, LEN, 0x2000, (k == 0) ? EIO : 0);
		memset(g_tmpl, 0x7f, ALLOC);
		c.dd_loc = 0;
		c.dd_size = 0;
		c.dd_flags = 0;
		for (int f = 0; f < 4; f++) {
			c.flags = f;
			for (int th = 0; th < 2; th++) {
				c.isthreaded = th;
				for (int e = 0; e < 2; e++) {
					c.errno_in = e ? 42 : 0;
					check_dir(0, c);
					check_dir(1, c);
					check_dir(2, c);
				}
			}
		}
	}
	c.errno_in = 0;

	/* 3. dd_loc vs dd_size right at the boundary. */
	memset(g_tmpl, 0x7f, ALLOC);
	lay(g_tmpl + c.bufoff, 0, Rec{ 3, 1, (unsigned)H_DIRLEN(4), 4 });
	lay(g_tmpl + c.bufoff, H_DIRLEN(4),
	    Rec{ 9, 2, (unsigned)H_DIRLEN(8), 8 });
	one_script(64, scratch, LEN, 0x3000, 0);
	for (size_t loc = 0; loc <= 2 * H_DIRLEN(8); loc += 8) {
		for (int ds = -1; ds <= 1; ds++) {
			c.dd_loc = loc;
			long long sz = (long long)loc + ds;
			if (sz < 0)
				continue;
			c.dd_size = (size_t)sz;
			c.dd_flags = 0;
			c.flags = H_RDU_SKIP;
			c.isthreaded = 0;
			check_dir(0, c);
			c.dd_flags = H_DTF_SKIPREAD;
			check_dir(0, c);
			c.dd_flags = H_DTF_READALL;
			check_dir(0, c);
			c.dd_flags = 0;
			check_dir(2, c);
			c.dd_flags = H_DTF_SKIPREAD;
			check_dir(2, c);
			check_dir(1, c);
		}
	}

	/* 4. d_reclen exactly at, and one past, the dd_len+1-dd_loc limit. */
	for (size_t loc = 0; loc <= 64; loc += 8) {
		for (int delta = -1; delta <= 2; delta++) {
			memset(g_tmpl, 0x7f, ALLOC);
			unsigned rl = (unsigned)((size_t)LEN + 1 - loc +
			    (size_t)delta);
			lay(g_tmpl + c.bufoff, loc, Rec{ 5, 1, rl, 4 });
			one_script(0, NULL, 0, 0, 0);
			c.dd_loc = loc;
			c.dd_size = (size_t)LEN;
			c.dd_flags = H_DTF_SKIPREAD;
			c.flags = H_RDU_SKIP;
			c.isthreaded = 0;
			check_dir(0, c);
			c.dd_flags = 0;
			check_dir(2, c);
		}
	}

	/* 5. d_reclen 0, 1, 7, 8 -- the `<= 0' test and odd advances. */
	for (unsigned rl = 0; rl <= 9; rl++) {
		memset(g_tmpl, 0x7f, ALLOC);
		lay(g_tmpl + c.bufoff, 0, Rec{ 11, 1, rl, 4 });
		lay(g_tmpl + c.bufoff, 8, Rec{ 12, 1, (unsigned)H_DIRLEN(2), 2 });
		one_script(0, NULL, 0, 0, 0);
		c.dd_loc = 0;
		c.dd_size = 128;
		c.dd_flags = H_DTF_SKIPREAD;
		c.flags = 0;
		c.isthreaded = 0;
		check_dir(0, c);
		c.flags = H_RDU_SKIP | H_RDU_SHORT;
		check_dir(0, c);
	}

	/* 6. misaligned dd_buf / dd_loc for the bogus-pointer check. */
	for (int off = 0; off < 8; off++) {
		for (size_t loc = 0; loc < 8; loc++) {
			memset(g_tmpl, 0x7f, ALLOC);
			lay(g_tmpl + off, loc, Rec{ 13, 1,
			    (unsigned)H_DIRLEN(4), 4 });
			one_script(0, NULL, 0, 0, 0);
			c.bufoff = off;
			c.dd_loc = loc;
			c.dd_size = 512;
			c.dd_flags = H_DTF_SKIPREAD;
			c.flags = H_RDU_SKIP;
			c.isthreaded = 0;
			check_dir(0, c);
		}
	}
	c.bufoff = 8;

	/* 7. d_ino == 0 / d_type == DT_WHT / d_namlen boundaries. */
	for (int ino0 = 0; ino0 < 2; ino0++) {
		for (int wht = 0; wht < 2; wht++) {
			for (int nl = 253; nl <= 259; nl++) {
				memset(g_tmpl, 0x7f, ALLOC);
				lay(g_tmpl + c.bufoff, 0,
				    Rec{ ino0 ? 0u : 99u,
				    wht ? (unsigned)H_DT_WHT : 3u,
				    (unsigned)H_DIRLEN(nl), (unsigned)nl });
				lay(g_tmpl + c.bufoff, H_DIRLEN(nl),
				    Rec{ 77, 1, (unsigned)H_DIRLEN(3), 3 });
				one_script(0, NULL, 0, 0, 0);
				c.dd_loc = 0;
				c.dd_size = (size_t)LEN;
				c.flags = 0;
				for (int f = 0; f < 4; f++) {
					c.flags = f;
					for (int hw = 0; hw < 2; hw++) {
						c.dd_flags = H_DTF_SKIPREAD |
						    (hw ? H_DTF_HIDEW : 0);
						c.isthreaded = 0;
						check_dir(0, c);
					}
				}
				c.dd_flags = H_DTF_SKIPREAD;
				c.flags = 0;
				check_dir(2, c);
				check_dir(1, c);
				c.dd_flags = H_DTF_SKIPREAD | H_DTF_HIDEW;
				check_dir(2, c);
				check_dir(1, c);
			}
		}
	}

	/* 8. errno interactions for __readdir_r. */
	memset(scratch, 0x7f, ALLOC);
	lay(scratch, 0, Rec{ 21, 1, (unsigned)H_DIRLEN(5), 5 });
	for (int ret = 0; ret < 2; ret++) {
		for (int se = 0; se < 3; se++) {
			for (int ei = 0; ei < 3; ei++) {
				one_script(ret ? (ssize_t)H_DIRLEN(5) : 0,
				    scratch, LEN, 0x4000,
				    se == 0 ? 0 : (se == 1 ? EIO : EINTR));
				memset(g_tmpl, 0x7f, ALLOC);
				c.dd_loc = 0;
				c.dd_size = 0;
				c.dd_flags = 0;
				c.flags = H_RDU_SKIP | H_RDU_SHORT;
				c.errno_in = ei == 0 ? 0 : (ei == 1 ? 7 : 99);
				for (int th = 0; th < 2; th++) {
					c.isthreaded = th;
					check_dir(2, c);
					check_dir(1, c);
					check_dir(0, c);
				}
			}
		}
	}
	c.errno_in = 0;

	/* 9. full chains driven to exhaustion. */
	{
		Rng rng(0xDEADBEEFu);
		for (int trial = 0; trial < 400; trial++) {
			int len = 64 + (int)rng.below(900);
			Chain ch;
			build_chain(g_tmpl, 8, len, rng, trial % 3, &ch);
			gen_script(len, rng, trial % 3, 1);
			c.bufoff = 8;
			c.dd_len = len;
			c.dd_loc = 0;
			c.dd_size = (size_t)len;
			c.dd_flags = H_DTF_SKIPREAD;
			c.flags = H_RDU_SKIP;
			c.isthreaded = trial & 1;
			c.errno_in = 0;
			check_dir_iterate(0, c, 64);
			check_dir_iterate(1, c, 64);
		}
	}
	{
		Rng rng(0xFEEDFACEu);
		for (int trial = 0; trial < 400; trial++) {
			int len = 64 + (int)rng.below(900);
			Chain ch;
			build_chain(g_tmpl, 8, len, rng, 0, &ch);
			gen_script(len, rng, 0, 0);
			c.bufoff = 8;
			c.dd_len = len;
			c.dd_loc = 0;
			c.dd_size = (size_t)len;
			c.dd_flags = H_DTF_SKIPREAD;
			c.flags = H_RDU_SKIP | H_RDU_SHORT;
			c.isthreaded = trial & 1;
			c.errno_in = trial % 5;
			check_dir_iterate(2, c, 64);
		}
	}
}

static void
gen_dir_case(Rng &rng, DirCase &c, int which)
{
	static const int aligned_offs[] = { 0, 8, 16, 24, 32 };
	static const int any_offs[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 16, 32 };

	int perturb = (which == 2) ? 0 : 1;
	c.dd_len = 32 + (int)rng.below(MAXDDLEN - 32);

	if (which == 2 || rng.below(4) == 0)
		c.bufoff = aligned_offs[rng.below(5)];
	else
		c.bufoff = any_offs[rng.below(11)];

	Chain ch;
	ch.npos = 0;
	if (which != 2 && rng.below(4) == 0)
		build_random(g_tmpl, c.bufoff, c.dd_len, rng);
	else
		build_chain(g_tmpl, c.bufoff, c.dd_len, rng, perturb, &ch);

	/*
	 * __readdir_r() memcpy()s _GENERIC_DIRSIZ(dp) bytes out of the
	 * buffer, so its cases stay on well-formed record boundaries where
	 * d_namlen is bounded; every other landing site is still covered by
	 * the _readdir_unlocked() sweep, which does not copy anything.
	 */
	if (which == 2) {
		if (ch.npos == 0 || rng.below(4) == 0)
			c.dd_loc = 0;
		else
			c.dd_loc = ch.pos[rng.below((uint32_t)ch.npos)];
	} else {
		switch (rng.below(7)) {
		case 0: c.dd_loc = 0; break;
		case 1: c.dd_loc = ch.npos
		    ? ch.pos[rng.below((uint32_t)ch.npos)] : 0; break;
		case 2: c.dd_loc = rng.below((uint32_t)c.dd_len + 1); break;
		case 3: c.dd_loc = (size_t)c.dd_len; break;
		case 4: c.dd_loc = (ch.npos
		    ? ch.pos[rng.below((uint32_t)ch.npos)] : 0) + 1; break;
		case 5: c.dd_loc = rng.below((uint32_t)c.dd_len + 8); break;
		default: c.dd_loc = ch.npos ? ch.pos[ch.npos - 1] : 0; break;
		}
	}

	switch (rng.below(8)) {
	case 0: c.dd_size = 0; break;
	case 1: c.dd_size = (size_t)c.dd_len; break;
	case 2: c.dd_size = c.dd_loc; break;
	case 3: c.dd_size = c.dd_loc + 1; break;
	case 4: c.dd_size = c.dd_loc ? c.dd_loc - 1 : 0; break;
	case 5: c.dd_size = rng.below((uint32_t)c.dd_len + 1); break;
	case 6: c.dd_size = (size_t)c.dd_len + 1; break;
	default: c.dd_size = rng.below((uint32_t)c.dd_len * 2 + 2); break;
	}

	c.dd_flags = 0;
	if (rng.below(2))
		c.dd_flags |= H_DTF_HIDEW;
	if (rng.below(4) == 0)
		c.dd_flags |= H_DTF_READALL;
	if (rng.below(3) == 0)
		c.dd_flags |= H_DTF_SKIPREAD;
	if (rng.below(4) == 0)
		c.dd_flags |= H_DTF_NODUP | H_DTF_REWIND;

	c.dd_seek = (off_t)(int64_t)(rng.next() & 0xffffff);
	c.fd = (int)rng.below(64);
	c.flags = (int)rng.below(4);
	c.isthreaded = (int)rng.below(2);
	c.errno_in = rng.below(4) == 0 ? 0 : (int)(1 + rng.below(60));

	gen_script(c.dd_len, rng, perturb, which != 2);
}

static void
sweep_dir(int which, long long iters, uint64_t seed)
{
	Rng rng(seed);
	DirCase c;

	memset(&c, 0, sizeof c);
	for (long long it = 0; it < iters; it++) {
		gen_dir_case(rng, c, which);
		if (rng.below(8) == 0)
			check_dir_iterate(which, c, 6);
		else
			check_dir(which, c);
	}
}

/* ------------------------------------------------------------------------ */

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

	dbufA = (unsigned char *)aligned_alloc(64, ALLOC);
	dbufB = (unsigned char *)aligned_alloc(64, ALLOC);
	ebufA = (unsigned char *)aligned_alloc(64, EALLOC);
	ebufB = (unsigned char *)aligned_alloc(64, EALLOC);
	if (dbufA == NULL || dbufB == NULL || ebufA == NULL || ebufB == NULL) {
		fprintf(stderr, "harness: out of memory\n");
		return 1;
	}

	/*
	 * futx_to_utx()'s allocation-failure path must be exercised before
	 * anything else, because the thread-local buffer is allocated once.
	 */
	{
		unsigned char id[4], user[32], line[16], host[128];
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

	hand_gettype();
	hand_utx();
	hand_ftou();
	hand_dir();

	sweep_gettype(200000);
	sweep_utx(200000);
	sweep_ftou(200000);
	sweep_dir(0, 200000, 0x0132C0DEu);
	sweep_dir(1, 200000, 0x0132BEEFu);
	sweep_dir(2, 200000, 0x0132FACEu);

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
