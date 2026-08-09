/*
 * harness.cpp -- differential test for batch b0279.
 */

import pbsd.lib.libc.gen.b0279;

#include <cerrno>
#include <cfloat>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>

#if !defined(__FreeBSD__)
union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};
#endif

namespace port = pbsd::lib_libc_gen::b0279;

#define	GUARD		0x7f
#define	SWEEP_ITERS	200000L
#define	MAX_PRINT	12

using ref_uexterror = port::uexterror;

extern "C" {
void harness_set_issetugid(int);
void harness_set_getenv_value(const char *);
void harness_set_getlogin(int result, int errnov, const char *name);
void harness_set_semctl_return(int);
int harness_last_semctl_semid(void);
int harness_last_semctl_semnum(void);
int harness_last_semctl_cmd(void);
int harness_last_semctl_had_semun(void);
int harness_last_semctl_semun_val(void);

int ref___fpclassifyf(float);
int ref___fpclassifyd(double);
int ref___fpclassifyl(long double);
char *ref_getlogin(void);
int ref_getlogin_r(char *, size_t);
int ref___getlogin_r_fbsd12(char *, int);
int ref_semctl(int, int, int, ...);
int ref___uexterr_format(const ref_uexterror *, char *, size_t);
}

enum {
	F_FPCLASSIFYF,
	F_FPCLASSIFYD,
	F_FPCLASSIFYL,
	F_GETLOGIN,
	F_GETLOGIN_R,
	F_GETLOGIN_R_FBSD12,
	F_SEMCTL,
	F_UEXTERR_FORMAT,
	NFUNC
};

static const char *const fname[NFUNC] = {
	"__fpclassifyf",
	"__fpclassifyd",
	"__fpclassifyl",
	"getlogin",
	"getlogin_r",
	"__getlogin_r_fbsd12",
	"semctl",
	"__uexterr_format",
};

static unsigned long long ncases[NFUNC];
static unsigned long long nfails[NFUNC];
static int nprinted;

static void
report(int f, const char *ctx, const char *detail)
{
	nfails[f]++;
	if (nprinted < MAX_PRINT) {
		nprinted++;
		std::printf("FAIL %-22s %-24s %s\n", fname[f], ctx, detail);
	} else if (nprinted == MAX_PRINT) {
		nprinted++;
		std::printf("... further failures suppressed\n");
	}
}

/* ------------------------------------------------------------------ PRNG */

static std::uint64_t rng_state = 0xb0279decafbad26ULL;

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

/* ------------------------------------------------------------------ guards */

struct GuardBuf {
	static constexpr std::size_t PRE = 32;
	static constexpr std::size_t WIN = 512;
	static constexpr std::size_t POST = 32;
	unsigned char bytes[PRE + WIN + POST];

	void
	init(void)
	{
		std::memset(bytes, GUARD, sizeof(bytes));
	}

	char *
	win(void)
	{
		return ((char *)(bytes + PRE));
	}

	std::size_t
	winsz(void) const
	{
		return (WIN);
	}

	bool
	identical(const GuardBuf &o) const
	{
		return (std::memcmp(bytes, o.bytes, sizeof(bytes)) == 0);
	}
};

static void
copy_ue(const port::uexterror *src, ref_uexterror *dst)
{
	*dst = *src;
}

/* ------------------------------------------------------------------ float */

union f_bits {
	float	f;
	struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		unsigned int	man	:23;
		unsigned int	exp	:8;
		unsigned int	sign	:1;
#else
		unsigned int	sign	:1;
		unsigned int	exp	:8;
		unsigned int	man	:23;
#endif
	} bits;
};

union d_bits {
	double	d;
	struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		unsigned int	manl	:32;
		unsigned int	manh	:20;
		unsigned int	exp	:11;
		unsigned int	sign	:1;
#else
		unsigned int	sign	:1;
		unsigned int	exp	:11;
		unsigned int	manh	:20;
		unsigned int	manl	:32;
#endif
	} bits;
};

static float
mkfloat(unsigned exp, unsigned man, unsigned sign)
{
	union f_bits u;

	u.bits.man = man;
	u.bits.exp = exp;
	u.bits.sign = sign;
	return (u.f);
}

static double
mkdouble(unsigned exp, unsigned manh, unsigned manl, unsigned sign)
{
	union d_bits u;

	u.bits.manl = manl;
	u.bits.manh = manh;
	u.bits.exp = exp;
	u.bits.sign = sign;
	return (u.d);
}

#if LDBL_MANT_DIG == 64
static const std::size_t LD_SIG = 10;
#else
static const std::size_t LD_SIG = sizeof(long double);
#endif

static long double
mkld(std::uint16_t se, std::uint64_t m)
{
	unsigned char b[sizeof(long double)];
	long double x;

	std::memset(b, 0, sizeof(b));
	std::memcpy(b, &m, sizeof(m));
	std::memcpy(b + 8, &se, sizeof(se));
	std::memcpy(&x, b, sizeof(x));
	return (x);
}

static void
check_fpclassifyf(float f, const char *ctx)
{
	int ref_rv, port_rv;

	ncases[F_FPCLASSIFYF]++;
	ref_rv = ref___fpclassifyf(f);
	port_rv = port::__fpclassifyf(f);
	if (ref_rv != port_rv)
		report(F_FPCLASSIFYF, ctx, "return mismatch");
}

static void
check_fpclassifyd(double d, const char *ctx)
{
	int ref_rv, port_rv;

	ncases[F_FPCLASSIFYD]++;
	ref_rv = ref___fpclassifyd(d);
	port_rv = port::__fpclassifyd(d);
	if (ref_rv != port_rv)
		report(F_FPCLASSIFYD, ctx, "return mismatch");
}

static void
check_fpclassifyl(long double e, const char *ctx)
{
	int ref_rv, port_rv;

	ncases[F_FPCLASSIFYL]++;
	ref_rv = ref___fpclassifyl(e);
	port_rv = port::__fpclassifyl(e);
	if (ref_rv != port_rv)
		report(F_FPCLASSIFYL, ctx, "return mismatch");
}

static void
test_fpclassify_handwritten(void)
{
	check_fpclassifyf(0.0f, "zero");
	check_fpclassifyf(-0.0f, "neg-zero");
	check_fpclassifyf(mkfloat(0, 1, 0), "subnormal");
	check_fpclassifyf(mkfloat(0, 0, 1), "zero-sign");
	check_fpclassifyf(mkfloat(127, 0, 0), "normal");
	check_fpclassifyf(mkfloat(255, 0, 0), "inf");
	check_fpclassifyf(mkfloat(255, 0, 1), "neg-inf");
	check_fpclassifyf(mkfloat(255, 1, 0), "nan");
	check_fpclassifyf(mkfloat(255, 0x7fffff, 0), "quiet-nan");
	check_fpclassifyf(mkfloat(1, 0, 0), "exp-1");
	check_fpclassifyf(mkfloat(254, 0x7fffff, 0), "exp-254");

	check_fpclassifyd(0.0, "zero");
	check_fpclassifyd(-0.0, "neg-zero");
	check_fpclassifyd(mkdouble(0, 0, 1, 0), "subnormal");
	check_fpclassifyd(mkdouble(1023, 0, 0, 0), "normal");
	check_fpclassifyd(mkdouble(2047, 0, 0, 0), "inf");
	check_fpclassifyd(mkdouble(2047, 0, 0, 1), "neg-inf");
	check_fpclassifyd(mkdouble(2047, 0, 1, 0), "nan-manl");
	check_fpclassifyd(mkdouble(2047, 1, 0, 0), "nan-manh");
	check_fpclassifyd(mkdouble(2047, 0xfffff, 0xffffffffu, 0), "nan-both");
	check_fpclassifyd(mkdouble(1, 0, 0, 0), "exp-1");
	check_fpclassifyd(mkdouble(2046, 0xfffff, 0xffffffffu, 0), "exp-2046");

	check_fpclassifyl(0.0L, "zero");
	check_fpclassifyl(-0.0L, "neg-zero");
#if LDBL_MANT_DIG == 53
	check_fpclassifyl(mkld(0x3fff, 0), "normal-ld53");
	check_fpclassifyl(mkld(0x7fff, 0), "inf-ld53");
	check_fpclassifyl(mkld(0x7fff, 1), "nan-ld53");
	check_fpclassifyl(mkld(0, 1), "subnormal-ld53");
#else
	check_fpclassifyl(mkld(0x3fff, 0), "normal");
	check_fpclassifyl(mkld(0x7fff, 0), "inf");
	check_fpclassifyl(mkld(0x7fff, 1), "nan");
	check_fpclassifyl(mkld(0, 1), "subnormal");
	check_fpclassifyl(mkld(0x8000, 0), "nbit-inf");
	check_fpclassifyl(mkld(0x8000, 1), "nbit-nan");
	check_fpclassifyl(mkld(1, 0), "exp-1");
	check_fpclassifyl(mkld(0x7ffe, 0xffffffffffffffffULL), "exp-32766");
#endif
}

static void
test_fpclassify_random(void)
{
	long i;

	for (i = 0; i < SWEEP_ITERS; i++) {
		union f_bits fu;
		union d_bits du;
		std::uint32_t rawf = rnd32();
		std::uint64_t rawd = rnd64();

		std::memcpy(&fu.f, &rawf, sizeof(fu.f));
		std::memcpy(&du.d, &rawd, sizeof(du.d));
		check_fpclassifyf(fu.f, "rand-f");
		check_fpclassifyd(du.d, "rand-d");

		if ((i & 0xff) == 0) {
			std::uint16_t se = (std::uint16_t)(rnd32() & 0xffff);
			std::uint64_t m = rnd64();
			long double ld = mkld(se, m);

			check_fpclassifyl(ld, "rand-l");
		}
	}
}

/* ------------------------------------------------------------------ getlogin */

static void
check_getlogin_case(int result, int errnov, const char *name, const char *ctx)
{
	char refbuf[64], portbuf[64];

	ncases[F_GETLOGIN]++;
	harness_set_getlogin(result, errnov, name);
	char *refp = ref_getlogin();
	char *portp = port::getlogin();
	if ((refp == nullptr) != (portp == nullptr)) {
		report(F_GETLOGIN, ctx, "null mismatch");
		return;
	}
	if (refp != nullptr) {
		if (std::strcmp(refp, portp) != 0)
			report(F_GETLOGIN, ctx, "string mismatch");
	}
	(void)refbuf;
	(void)portbuf;
}

static void
check_getlogin_r_case(int result, int errnov, const char *name, size_t namelen,
    const char *ctx)
{
	GuardBuf refg, portg;
	int ref_rv, port_rv;

	ncases[F_GETLOGIN_R]++;
	refg.init();
	portg.init();
	harness_set_getlogin(result, errnov, name);
	ref_rv = ref_getlogin_r(refg.win(), namelen);
	port_rv = port::getlogin_r(portg.win(), namelen);
	if (ref_rv != port_rv)
		report(F_GETLOGIN_R, ctx, "return mismatch");
	if (!refg.identical(portg))
		report(F_GETLOGIN_R, ctx, "buffer mismatch");
}

static void
check_getlogin_r_fbsd12_case(int result, int errnov, const char *name,
    int namelen, const char *ctx)
{
	GuardBuf refg, portg;
	int ref_rv, port_rv;

	ncases[F_GETLOGIN_R_FBSD12]++;
	refg.init();
	portg.init();
	harness_set_getlogin(result, errnov, name);
	ref_rv = ref___getlogin_r_fbsd12(refg.win(), namelen);
	port_rv = port::__getlogin_r_fbsd12(portg.win(), namelen);
	if (ref_rv != port_rv)
		report(F_GETLOGIN_R_FBSD12, ctx, "return mismatch");
	if (!refg.identical(portg))
		report(F_GETLOGIN_R_FBSD12, ctx, "buffer mismatch");
}

static void
test_getlogin_handwritten(void)
{
	check_getlogin_case(0, 0, "alice", "ok");
	check_getlogin_case(0, 0, "", "empty");
	check_getlogin_case(-1, EIO, "", "fail");
	check_getlogin_case(-1, EPERM, "x", "fail-perm");
	check_getlogin_case(0, 0, "bob", "bob");

	check_getlogin_r_case(0, 0, "alice", 32, "ok");
	check_getlogin_r_case(0, 0, "alice", 6, "erange");
	check_getlogin_r_case(0, 0, "", 32, "empty");
	check_getlogin_r_case(0, 0, "", 1, "empty-n1");
	check_getlogin_r_case(-1, ENOENT, "", 32, "fail");
	check_getlogin_r_case(0, 0, "alice", 0, "namelen-0");
	check_getlogin_r_case(0, 0, "alice", 1, "namelen-1");

	check_getlogin_r_fbsd12_case(0, 0, "alice", 32, "ok");
	check_getlogin_r_fbsd12_case(0, 0, "alice", 6, "erange");
	check_getlogin_r_fbsd12_case(0, 0, "", 32, "empty");
	check_getlogin_r_fbsd12_case(-1, EACCES, "", 32, "fail");
	check_getlogin_r_fbsd12_case(0, 0, "alice", 0, "namelen-0");
}

static void
test_getlogin_random(void)
{
	long i;
	static const char alnum[] =
	    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

	for (i = 0; i < SWEEP_ITERS; i++) {
		char name[32];
		unsigned nlen = (unsigned)(rnd32() % 32);
		unsigned j;

		for (j = 0; j < nlen && j + 1 < sizeof(name); j++)
			name[j] = alnum[rnd32() % (sizeof(alnum) - 1)];
		name[j] = '\0';

		if ((rnd32() & 3) == 0)
			check_getlogin_case(-1, (int)(rnd32() % 40) + 1, name,
			    "rand-fail");
		else
			check_getlogin_case(0, 0, name, "rand-ok");

		check_getlogin_r_case((rnd32() & 7) == 0 ? -1 : 0,
		    (int)(rnd32() % 40) + 1, name, (size_t)(rnd32() % 40),
		    "rand-r");
		check_getlogin_r_fbsd12_case((rnd32() & 7) == 0 ? -1 : 0,
		    (int)(rnd32() % 40) + 1, name, (int)(rnd32() % 40),
		    "rand-fbsd12");
	}
}

/* ------------------------------------------------------------------ semctl */

static int
semctl_cmds[] = {
	IPC_SET, IPC_STAT, GETALL, SETVAL, SETALL,
	GETVAL, GETPID, GETNCNT, GETZCNT,
	0, 1, 2, 15, 16, 17, 18, 19, 20, 21, 22, 127, 255, -1, -128
};

static void
check_semctl_case(int semid, int semnum, int cmd, int semun_val,
    const char *ctx)
{
	int ref_rv, port_rv;
	union semun su;
	int ref_had, port_had, ref_sid, port_sid, ref_snum, port_snum;
	int ref_cmd, port_cmd, ref_val, port_val;
	int mock_ret;

	ncases[F_SEMCTL]++;
	mock_ret = (int)(rnd32() & 0x7fffffff);
	harness_set_semctl_return(mock_ret);
	su.val = semun_val;

	ref_rv = ref_semctl(semid, semnum, cmd, su);
	ref_had = harness_last_semctl_had_semun();
	ref_sid = harness_last_semctl_semid();
	ref_snum = harness_last_semctl_semnum();
	ref_cmd = harness_last_semctl_cmd();
	ref_val = harness_last_semctl_semun_val();

	port_rv = port::semctl(semid, semnum, cmd, su);
	port_had = harness_last_semctl_had_semun();
	port_sid = harness_last_semctl_semid();
	port_snum = harness_last_semctl_semnum();
	port_cmd = harness_last_semctl_cmd();
	port_val = harness_last_semctl_semun_val();

	if (ref_rv != port_rv)
		report(F_SEMCTL, ctx, "return mismatch");
	if (ref_had != port_had || ref_sid != port_sid || ref_snum != port_snum ||
	    ref_cmd != port_cmd || (ref_had && ref_val != port_val))
		report(F_SEMCTL, ctx, "mock-args mismatch");
}

static void
check_semctl_case_nosemun(int semid, int semnum, int cmd, const char *ctx)
{
	int ref_rv, port_rv;
	int ref_had, port_had;

	ncases[F_SEMCTL]++;
	harness_set_semctl_return(42);
	ref_rv = ref_semctl(semid, semnum, cmd);
	ref_had = harness_last_semctl_had_semun();

	port_rv = port::semctl(semid, semnum, cmd);
	port_had = harness_last_semctl_had_semun();

	if (ref_rv != port_rv)
		report(F_SEMCTL, ctx, "return mismatch");
	if (ref_had != port_had)
		report(F_SEMCTL, ctx, "semun-null mismatch");
}

static void
test_semctl_handwritten(void)
{
	unsigned i;

	for (i = 0; i < sizeof(semctl_cmds) / sizeof(semctl_cmds[0]); i++) {
		int cmd = semctl_cmds[i];

		check_semctl_case(1, 0, cmd, 7, "hand-semun");
		check_semctl_case_nosemun(2, 1, cmd, "hand-nosemun");
	}
}

static void
test_semctl_random(void)
{
	long i;

	for (i = 0; i < SWEEP_ITERS; i++) {
		int semid = (int)rnd32();
		int semnum = (int)rnd32();
		int cmd = (int)rnd32();
		union semun su;

		su.val = (int)rnd32();
		check_semctl_case(semid, semnum, cmd, su.val, "rand");
	}
}

/* ------------------------------------------------------------------ uexterr */

struct UexterrRunOut {
	int rv;
	unsigned char bytes[GuardBuf::PRE + GuardBuf::WIN + GuardBuf::POST];
};

static int
run_uexterr_isolated(int use_port, const ref_uexterror *rue,
    const port::uexterror *ue, size_t bufsz, int issetugid,
    const char *getenv_val, UexterrRunOut *out)
{
	GuardBuf g;
	int fd[2];
	pid_t pid;
	int status;
	ssize_t n;

	if (pipe(fd) != 0)
		return (-1);
	pid = fork();
	if (pid < 0) {
		close(fd[0]);
		close(fd[1]);
		return (-1);
	}
	if (pid == 0) {
		UexterrRunOut result;

		close(fd[0]);
		harness_set_issetugid(issetugid);
		harness_set_getenv_value(getenv_val);
		g.init();
		g.win()[0] = '\0';
		if (use_port)
			result.rv = port::__uexterr_format(ue, g.win(), bufsz);
		else
			result.rv = ref___uexterr_format(rue, g.win(), bufsz);
		std::memcpy(result.bytes, g.bytes, sizeof(result.bytes));
		(void)write(fd[1], &result, sizeof(result));
		close(fd[1]);
		_exit(0);
	}
	close(fd[1]);
	n = read(fd[0], out, sizeof(*out));
	close(fd[0]);
	waitpid(pid, &status, 0);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0 ||
	    n != (ssize_t)sizeof(*out))
		return (-1);
	return (0);
}

static void
run_uexterr_case(const port::uexterror *ue, size_t bufsz, int issetugid,
    const char *getenv_val, const char *ctx)
{
	ref_uexterror rue;
	UexterrRunOut ref_out, port_out;

	ncases[F_UEXTERR_FORMAT]++;
	copy_ue(ue, &rue);

	if (run_uexterr_isolated(0, &rue, ue, bufsz, issetugid, getenv_val,
	    &ref_out) != 0) {
		report(F_UEXTERR_FORMAT, ctx, "ref child failed");
		return;
	}
	if (run_uexterr_isolated(1, &rue, ue, bufsz, issetugid, getenv_val,
	    &port_out) != 0) {
		report(F_UEXTERR_FORMAT, ctx, "port child failed");
		return;
	}
	if (ref_out.rv != port_out.rv ||
	    std::memcmp(ref_out.bytes, port_out.bytes, sizeof(ref_out.bytes)) != 0)
		report(F_UEXTERR_FORMAT, ctx, "mismatch");
}

static void
fill_ue(port::uexterror *ue, std::uint32_t error, std::uint32_t cat,
    std::uint32_t line, std::uint64_t p1, std::uint64_t p2, const char *msg)
{
	std::memset(ue, 0, sizeof(*ue));
	ue->error = error;
	ue->cat = cat;
	ue->src_line = line;
	ue->p1 = p1;
	ue->p2 = p2;
	if (msg != nullptr)
		std::strncpy(ue->msg, msg, sizeof(ue->msg) - 1);
}

static void
test_uexterr_handwritten(void)
{
	port::uexterror ue;

	fill_ue(&ue, 0, 1, 10, 0, 0, "ignored");
	run_uexterr_case(&ue, 0, 0, nullptr, "error-zero-buf0");

	fill_ue(&ue, 5, 1, 42, 0x11, 0x22, "%#jx %#jx");
	run_uexterr_case(&ue, 0, 0, nullptr, "default-msg-buf0");

	fill_ue(&ue, 5, 1, 42, 0x11, 0x22, "%#jx %#jx");
	run_uexterr_case(&ue, 300, 0, nullptr, "default-msg");

	fill_ue(&ue, 5, 1, 42, 0x11, 0x22, "%#jx %#jx");
	run_uexterr_case(&ue, 300, 0, "brief", "brief-msg");

	fill_ue(&ue, 5, 1, 42, 0x11, 0x22, "%#jx %#jx");
	run_uexterr_case(&ue, 300, 0, "full", "full-msg");

	fill_ue(&ue, 9, 14, 99, 0xdead, 0xbeef, "");
	run_uexterr_case(&ue, 400, 0, nullptr, "no-msg-default");

	fill_ue(&ue, 9, 14, 99, 0xdead, 0xbeef, "");
	run_uexterr_case(&ue, 400, 0, "full", "no-msg-full");

	fill_ue(&ue, 3, 99, 7, 1, 2, "x");
	run_uexterr_case(&ue, 128, 1, "brief", "issetugid");

	fill_ue(&ue, 3, -1, 7, 1, 2, "plain");
	run_uexterr_case(&ue, 64, 0, nullptr, "bad-cat");

	fill_ue(&ue, 3, 1, 7, 1, 2, "plain");
	run_uexterr_case(&ue, 64, 0, nullptr, "good-cat");

	fill_ue(&ue, 4, 6, 8, 0x80, 0xff, "m%ju");
	run_uexterr_case(&ue, 512, 0, "brief", "high-bit");

	fill_ue(&ue, 4, 6, 8, 0x80, 0xff, "m%ju");
	run_uexterr_case(&ue, 3, 0, nullptr, "tiny-buf");

	fill_ue(&ue, 4, 6, 8, 0x80, 0xff, "m%ju");
	run_uexterr_case(&ue, 1000, 0, nullptr, "large-buf-cap");
}

static void
test_uexterr_random(void)
{
	long i;

	for (i = 0; i < SWEEP_ITERS; i++) {
		port::uexterror ue;
		unsigned char msgbytes[64];
		unsigned mlen = (unsigned)(rnd32() % 64);
		unsigned j;
		const char *env;
		int isset;

		std::memset(&ue, 0, sizeof(ue));
		ue.error = (rnd32() & 0xff) == 0 ? 0 : (rnd32() % 200) + 1;
		ue.cat = rnd32() % 32;
		ue.src_line = rnd32();
		ue.p1 = rnd64();
		ue.p2 = rnd64();
		for (j = 0; j < mlen; j++)
			msgbytes[j] = (unsigned char)(rnd32() & 0xff);
		msgbytes[mlen % 64] = '\0';
		std::memcpy(ue.msg, msgbytes, 64);

		switch (rnd32() % 4) {
		case 0:
			env = nullptr;
			break;
		case 1:
			env = "brief";
			break;
		default:
			env = "verbose";
			break;
		}
		isset = (rnd32() & 0xf) == 0 ? 1 : 0;
		run_uexterr_case(&ue, (size_t)(1 + (rnd32() % 599)), isset, env, "rand");
	}
}

/* ------------------------------------------------------------------ main */

int
main(void)
{
	test_fpclassify_handwritten();
	test_fpclassify_random();
	test_getlogin_handwritten();
	test_getlogin_random();
	test_semctl_handwritten();
	test_semctl_random();
	test_uexterr_handwritten();
	test_uexterr_random();

	std::printf("\n%-22s %12s %12s\n", "function", "cases", "failures");
	for (int f = 0; f < NFUNC; f++)
		std::printf("%-22s %12llu %12llu\n", fname[f],
		    ncases[f], nfails[f]);

	for (int f = 0; f < NFUNC; f++) {
		if (nfails[f] != 0)
			return (1);
	}
	return (0);
}
