/*
 * PBSD batch b0061 -- differential test: port vs. ref_ oracle for
 * hbsd/src/bin/csh/iconv_stub.c (dl_iconv_open).
 *
 * Each case runs in a fresh fork so static initialisation state does not
 * leak between scenarios.  Return values and the resolved dl_iconv /
 * dl_iconv_close function pointers are compared after every call.
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/wait.h>
#include <unistd.h>

import pbsd.bin.csh.b0061;

namespace P = pbsd::bin_csh::b0061;

using iconv_t = void *;

extern "C" {
iconv_t ref_dl_iconv_open(const char *, const char *);
typedef size_t oracle_dl_iconv_t(iconv_t, char **, size_t *, char **,
    size_t *);
typedef int oracle_dl_iconv_close_t(iconv_t);
extern oracle_dl_iconv_t *dl_iconv;
extern oracle_dl_iconv_close_t *dl_iconv_close;
}

enum {
	F_DL_ICONV_OPEN,
	F_COUNT
};

static const char *const fname[F_COUNT] = {
	"dl_iconv_open",
};

static long long ncase[F_COUNT];
static long long nfail[F_COUNT];
static int nprinted[F_COUNT];

static const char *mock_base = "mocks";

enum MockKind {
	MOCK_GOOD,
	MOCK_NO_OPEN,
	MOCK_NO_ENGINE,
	MOCK_NO_CLOSE,
	MOCK_MISSING,
	MOCK_COUNT
};

static const char *const mock_dir[MOCK_COUNT] = {
	"good",
	"no_open",
	"no_engine",
	"no_close",
	"missing",
};

struct Obs {
	uintptr_t ret;
	uintptr_t iconv_fn;
	uintptr_t close_fn;
};

static void
report(int f, const char *why)
{

	nfail[f]++;
	if (nprinted[f]++ < 8)
		std::printf("  FAIL %-16s : %s\n", fname[f], why);
}

static uintptr_t
ptr_u(const void *p)
{

	return (reinterpret_cast<uintptr_t>(p));
}

template<typename T>
static uintptr_t
fp_u(T fn)
{
	uintptr_t u;

	static_assert(sizeof(T) == sizeof(u));
	std::memcpy(&u, &fn, sizeof(u));
	return (u);
}

static bool
obs_eq(const Obs &a, const Obs &b)
{

	return (a.ret == b.ret && a.iconv_fn == b.iconv_fn &&
	    a.close_fn == b.close_fn);
}

static void
snap_ref(Obs *o, iconv_t ret)
{

	o->ret = ptr_u(ret);
	o->iconv_fn = fp_u(dl_iconv);
	o->close_fn = fp_u(dl_iconv_close);
}

static void
snap_port(Obs *o, iconv_t ret)
{

	o->ret = ptr_u(ret);
	o->iconv_fn = fp_u(P::dl_iconv);
	o->close_fn = fp_u(P::dl_iconv_close);
}

static bool
set_mock_path(MockKind kind, char *buf, size_t bufsz)
{

	if (kind == MOCK_MISSING) {
		std::snprintf(buf, bufsz, "/nonexistent/pbsd-b0061-%ld",
		    (long)getpid());
		return (true);
	}
	std::snprintf(buf, bufsz, "%s/%s", mock_base, mock_dir[kind]);
	return (true);
}

/*
 * Run one or two paired call(s) in a child with a fresh address space.
 * ncalls is 1 or 2; the second call exercises the `initialized' branch.
 */
static bool
run_isolated(MockKind kind, const char *tocode, const char *fromcode,
    int ncalls)
{
	char ldpath[512];
	int pipefd[2];
	Obs ref_obs[2], port_obs[2];

	set_mock_path(kind, ldpath, sizeof(ldpath));
	if (pipe(pipefd) != 0)
		return (false);

	pid_t pid = fork();
	if (pid < 0)
		return (false);

	if (pid == 0) {
		Obs rout[2], pout[2];
		iconv_t rret, pret;

		if (setenv("LD_LIBRARY_PATH", ldpath, 1) != 0)
			_exit(2);

		rret = ref_dl_iconv_open(tocode, fromcode);
		snap_ref(&rout[0], rret);
		pret = P::dl_iconv_open(tocode, fromcode);
		snap_port(&pout[0], pret);

		if (ncalls >= 2) {
			rret = ref_dl_iconv_open(tocode, fromcode);
			snap_ref(&rout[1], rret);
			pret = P::dl_iconv_open(tocode, fromcode);
			snap_port(&pout[1], pret);
		}

		unsigned char msg[sizeof(rout) + sizeof(pout)];
		std::memcpy(msg, rout, sizeof(rout));
		std::memcpy(msg + sizeof(rout), pout, sizeof(pout));
		(void)write(pipefd[1], msg, sizeof(msg));
		_exit(0);
	}

	close(pipefd[1]);
	unsigned char msg[sizeof(ref_obs) + sizeof(port_obs)];
	ssize_t got = read(pipefd[0], msg, sizeof(msg));
	close(pipefd[0]);

	int status = 0;
	(void)waitpid(pid, &status, 0);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0 || got != (ssize_t)sizeof(msg))
		return (false);

	std::memcpy(ref_obs, msg, sizeof(ref_obs));
	std::memcpy(port_obs, msg + sizeof(ref_obs), sizeof(port_obs));

	bool ok = obs_eq(ref_obs[0], port_obs[0]);
	if (ncalls >= 2)
		ok = ok && obs_eq(ref_obs[1], port_obs[1]);
	return (ok);
}

static void
check(MockKind kind, const char *tocode, const char *fromcode, int ncalls,
    const char *tag)
{
	char why[256];

	ncase[F_DL_ICONV_OPEN]++;
	if (run_isolated(kind, tocode, fromcode, ncalls))
		return;

	if (nprinted[F_DL_ICONV_OPEN] < 2) {
		char ldpath[512];
		set_mock_path(kind, ldpath, sizeof(ldpath));
		std::fprintf(stderr, "debug: isolated failed kind=%d ld=%s\n",
		    (int)kind, ldpath);
	}

	std::snprintf(why, sizeof(why), "%s kind=%d ncalls=%d to=%s from=%s",
	    tag, (int)kind, ncalls,
	    tocode ? tocode : "(null)",
	    fromcode ? fromcode : "(null)");
	report(F_DL_ICONV_OPEN, why);
}

/* ------------------------------------------------------------------ */
/* deterministic PRNG							*/
/* ------------------------------------------------------------------ */

static uint64_t rstate = 0x243f6a8885a308d3ULL;

static inline uint64_t
nxt()
{

	rstate ^= rstate << 13;
	rstate ^= rstate >> 7;
	rstate ^= rstate << 17;
	return (rstate);
}

static inline uint32_t
u32(uint32_t m)
{

	return ((uint32_t)(nxt() % m));
}

static void
fill_random_string(char *buf, size_t cap, size_t len)
{
	size_t i;

	if (cap == 0)
		return;
	if (len >= cap)
		len = cap - 1;
	for (i = 0; i < len; i++) {
		uint32_t roll = u32(100);
		if (roll < 8)
			buf[i] = '\0';
		else if (roll < 20)
			buf[i] = (char)(0x80 + u32(128));
		else if (roll < 30)
			buf[i] = (char)u32(256);
		else
			buf[i] = (char)('a' + u32(26));
	}
	buf[len] = '\0';
}

static void
edge_cases()
{

	check(MOCK_GOOD, "", "", 1, "empty-empty");
	check(MOCK_GOOD, "A", "B", 1, "single-char");
	check(MOCK_GOOD, "UTF-8", "UTF-8", 2, "utf8-twice");
	check(MOCK_GOOD, "\x80", "\xff", 1, "high-bit-single");
	check(MOCK_GOOD, "\x80\xff\xfe", "\x7f\x80", 2, "high-bit-mix-twice");
	check(MOCK_GOOD, "US-ASCII", "ISO-8859-1", 1, "charset-names");
	check(MOCK_GOOD, "a", "", 2, "from-empty-twice");
	check(MOCK_GOOD, "", "b", 2, "to-empty-twice");

	{
		char longto[256];
		char longfrom[256];
		size_t i;

		for (i = 0; i < sizeof(longto) - 1; i++)
			longto[i] = (char)(0x80 + (i & 0x7f));
		longto[sizeof(longto) - 1] = '\0';
		for (i = 0; i < sizeof(longfrom) - 1; i++)
			longfrom[i] = (char)(i & 0xff);
		longfrom[sizeof(longfrom) - 1] = '\0';
		check(MOCK_GOOD, longto, longfrom, 2, "boundary-len-twice");
	}

	check(MOCK_MISSING, "x", "y", 1, "dlopen-fail-once");
	check(MOCK_MISSING, "x", "y", 2, "dlopen-fail-twice");
	check(MOCK_NO_OPEN, "p", "q", 1, "dlfunc-no-open-once");
	check(MOCK_NO_OPEN, "p", "q", 2, "dlfunc-no-open-twice");
	check(MOCK_NO_ENGINE, "r", "s", 1, "dlfunc-no-engine-once");
	check(MOCK_NO_ENGINE, "r", "s", 2, "dlfunc-no-engine-twice");
	check(MOCK_NO_CLOSE, "t", "u", 1, "dlfunc-no-close-once");
	check(MOCK_NO_CLOSE, "t", "u", 2, "dlfunc-no-close-twice");

	check(MOCK_GOOD, "\xff\xff", "\xfe\xfe", 2, "nul-free-high-twice");
	check(MOCK_GOOD, "shift_jis", "euc-jp", 1, "jp-charsets");
	check(MOCK_GOOD, "wchar_t", "char", 2, "wchar-char-twice");
}

static void
random_sweep()
{
	char tobuf[192];
	char frombuf[192];
	unsigned i;

	for (i = 0; i < 200000U; i++) {
		MockKind kind = (MockKind)u32(MOCK_COUNT);
		size_t lt = u32((uint32_t)sizeof(tobuf) - 1U);
		size_t lf = u32((uint32_t)sizeof(frombuf) - 1U);
		int ncalls = (u32(2) == 0) ? 2 : 1;

		fill_random_string(tobuf, sizeof(tobuf), lt);
		fill_random_string(frombuf, sizeof(frombuf), lf);
		check(kind, tobuf, frombuf, ncalls, "random");
	}
}

int
main()
{

	edge_cases();
	random_sweep();

	std::printf("\n%-20s %12s %12s\n", "function", "cases", "failures");
	for (int f = 0; f < F_COUNT; f++)
		std::printf("%-20s %12lld %12lld\n", fname[f], ncase[f],
		    nfail[f]);

	long long total_fail = 0;
	for (int f = 0; f < F_COUNT; f++)
		total_fail += nfail[f];

	return (total_fail == 0 ? 0 : 1);
}
