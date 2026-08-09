// Differential harness for PBSD batch b0293.

import pbsd.bin.sh.b0293;

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

namespace P = pbsd::bin_sh::b0293;

static const unsigned char GUARD = 0x7f;
static const long SWEEP = 200000L;

extern "C" {
void oracle_reset_all(void);
const char *oracle_get_optarg(void);
int oracle_optarg_was_set(void);
int oracle_optarg_was_unset(void);
const char *oracle_get_optind(void);
const char *oracle_get_optvar(void);
int oracle_get_exraised(void);
int oracle_get_shellparam_reset(void);
int oracle_get_error_thrown(void);
void oracle_set_argptr(char **ap);
char *oracle_get_shoptarg(void);
void oracle_set_nextopt_optptr(char *p);

bool ref_isbinary(const char *data, size_t len);
unsigned int ref_hashname(const char *p);
void ref_fdctx_init(int fd, P::fdctx *fdc);
ssize_t ref_fdgetc(P::fdctx *fdc, char *c);
void ref_fdctx_destroy(P::fdctx *fdc);
int ref_try_nextopt(const char *optstring, int *res);
void ref_getoptsreset(const char *value);
void ref_freeparam(P::shparam *param);
int ref_getopts(char *optstr, char *optvar, char **optfirst,
    char ***optnext, char **optptr);
}

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
};

static Stat stats[] = {
	{ "isbinary", 0, 0 },
	{ "hashname", 0, 0 },
	{ "fdctx_init", 0, 0 },
	{ "fdgetc", 0, 0 },
	{ "fdctx_destroy", 0, 0 },
	{ "nextopt", 0, 0 },
	{ "getoptsreset", 0, 0 },
	{ "freeparam", 0, 0 },
	{ "getopts", 0, 0 },
};
static const int NSTAT = (int)(sizeof(stats) / sizeof(stats[0]));

static Stat &
S(const char *n)
{
	for (int i = 0; i < NSTAT; i++)
		if (std::strcmp(stats[i].name, n) == 0)
			return stats[i];
	std::fprintf(stderr, "bad stat %s\n", n);
	std::exit(2);
}

static void
fail(Stat &st, const char *msg)
{
	st.fails++;
	if (st.fails <= 8)
		std::printf("  FAIL %s: %s\n", st.name, msg);
}

static void
ok(Stat &st)
{
	st.cases++;
}

struct Rng {
	std::uint64_t s;
	explicit Rng(std::uint64_t seed) : s(seed) {}
	std::uint64_t next()
	{
		s += 0x9E3779B97F4A7C15ull;
		std::uint64_t z = s;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}
	std::uint32_t u32() { return (std::uint32_t)next(); }
	int coin() { return (int)(next() & 1); }
};

static Rng rng(0x00b0293ull);

static void
fill_guard(unsigned char *b, size_t n)
{
	for (size_t i = 0; i < n; i++)
		b[i] = GUARD;
}

static int
bufs_equal(const unsigned char *a, const unsigned char *b, size_t n)
{
	for (size_t i = 0; i < n; i++)
		if (a[i] != b[i])
			return 0;
	return 1;
}

/* --- isbinary --- */

static void
test_isbinary_one(const unsigned char *data, size_t len, const char *tag)
{
	Stat &st = S("isbinary");
	bool r = ref_isbinary((const char *)data, len);
	bool p = P::isbinary((const char *)data, len);
	if (r != p)
		fail(st, tag);
	else
		ok(st);
}

static void
test_isbinary_hand()
{
	unsigned char b[256];
	fill_guard(b, sizeof(b));

	/* no NUL in buffer */
	b[0] = 'a';
	test_isbinary_one(b, 10, "no_nul");

	/* NUL at start */
	b[0] = '\0';
	test_isbinary_one(b, 5, "nul_start");

	/* script without newline before NUL */
	std::memcpy(b, "echo hi\0bin", 10);
	test_isbinary_one(b, 10, "script_no_nl");

	/* lowercase + newline before NUL -> not binary */
	std::memcpy(b, "abc\ndef\n\0", 8);
	test_isbinary_one(b, 8, "letter_nl");

	/* lowercase no newline -> binary */
	std::memcpy(b, "abc\0", 4);
	test_isbinary_one(b, 4, "letter_no_nl");

	/* $ before newline */
	std::memcpy(b, "$\n\0", 3);
	test_isbinary_one(b, 3, "dollar_nl");

	/* backtick */
	std::memcpy(b, "`x\0", 3);
	test_isbinary_one(b, 3, "backtick");

	/* high-bit bytes before NUL */
	for (int i = 0; i < 8; i++) {
		b[i] = (unsigned char)(0x80 + i);
	}
	b[8] = '\0';
	test_isbinary_one(b, 9, "highbit");

	/* empty NUL-only */
	b[0] = '\0';
	test_isbinary_one(b, 1, "only_nul");
}

static void
test_isbinary_sweep()
{
	Stat &st = S("isbinary");
	unsigned char b[128];
	for (long i = 0; i < SWEEP; i++) {
		size_t n = (size_t)(rng.u32() % 64) + 1;
		for (size_t j = 0; j < n; j++)
			b[j] = (unsigned char)(rng.u32() & 0xff);
		/* force a NUL somewhere in [0,n) */
		size_t nulpos = (size_t)(rng.u32() % n);
		b[nulpos] = '\0';
		bool r = ref_isbinary((const char *)b, n);
		bool p = P::isbinary((const char *)b, n);
		if (r != p)
			fail(st, "sweep");
		else
			ok(st);
	}
}

/* --- hashname --- */

static void
test_hashname_one(const char *s, const char *tag)
{
	Stat &st = S("hashname");
	unsigned int r = ref_hashname(s);
	unsigned int p = P::hashname(s);
	if (r != p)
		fail(st, tag);
	else
		ok(st);
}

static void
test_hashname_hand()
{
	test_hashname_one("", "empty");
	test_hashname_one("a", "one");
	test_hashname_one("hash", "hash");
	test_hashname_one("\x80\xff", "highbit");
	test_hashname_one("abcdefghijklmnopqrstuvwxyz", "alpha");
}

static void
test_hashname_sweep()
{
	Stat &st = S("hashname");
	char s[64];
	for (long i = 0; i < SWEEP; i++) {
		size_t n = (size_t)(rng.u32() % 40);
		for (size_t j = 0; j < n; j++)
			s[j] = (char)(rng.u32() & 0xff);
		s[n] = '\0';
		unsigned int r = ref_hashname(s);
		unsigned int p = P::hashname(s);
		if (r != p)
			fail(st, "sweep");
		else
			ok(st);
	}
}

/* --- fdctx --- */

static int
make_temp_file(const unsigned char *data, size_t len)
{
	char path[] = "/tmp/b0293fdXXXXXX";
	int fd = mkstemp(path);
	if (fd < 0)
		std::exit(3);
	if (len > 0)
		(void)write(fd, data, len);
	(void)lseek(fd, 0, SEEK_SET);
	unlink(path);
	return fd;
}

static void
test_fdctx_read(const unsigned char *data, size_t len, const char *tag)
{
	Stat &st_init = S("fdctx_init");
	Stat &st_getc = S("fdgetc");
	Stat &st_dest = S("fdctx_destroy");

	int fd = make_temp_file(data, len);
	int fd2 = make_temp_file(data, len);
	P::fdctx pf{}, rf{};

	ref_fdctx_init(fd, &rf);
	P::fdctx_init(fd2, &pf);
	if (rf.fd != fd || pf.fd != fd2 || rf.buflen != pf.buflen ||
	    rf.off != pf.off || (rf.ep - rf.buf) != (pf.ep - pf.buf))
		fail(st_init, tag);
	else
		ok(st_init);

	size_t pos = 0;
	while (pos < len) {
		char rc = 0, pc = 0;
		ssize_t rn = ref_fdgetc(&rf, &rc);
		ssize_t pn = P::fdgetc(&pf, &pc);
		if (rn != pn || rc != pc || rf.off != pf.off)
			fail(st_getc, tag);
		else
			ok(st_getc);
		if (rn != 1)
			break;
		pos++;
	}
	char rc = 0, pc = 0;
	ssize_t rn = ref_fdgetc(&rf, &rc);
	ssize_t pn = P::fdgetc(&pf, &pc);
	if (rn != pn)
		fail(st_getc, "eof");
	else
		ok(st_getc);

	off_t rpos = lseek(fd, 0, SEEK_CUR);
	ref_fdctx_destroy(&rf);
	P::fdctx_destroy(&pf);
	off_t ppos = lseek(fd2, 0, SEEK_CUR);
	if (rpos != ppos)
		fail(st_dest, tag);
	else
		ok(st_dest);

	close(fd);
	close(fd2);
}

static void
test_fdctx_hand()
{
	unsigned char d1[] = { 'a', 'b', 'c' };
	test_fdctx_read(d1, 3, "abc");
	unsigned char d2[] = { 0x80, 0xff, '\n', 0 };
	test_fdctx_read(d2, 3, "highbit");
	unsigned char d3[512];
	for (size_t i = 0; i < 512; i++)
		d3[i] = (unsigned char)(i & 0xff);
	test_fdctx_read(d3, 512, "big");

	/* pipe (non-seekable): buflen should be 1 */
	int pv[2];
	if (pipe(pv) == 0) {
		int pv2[2];
		if (pipe(pv2) == 0) {
			(void)write(pv[1], "xy", 2);
			(void)write(pv2[1], "xy", 2);
			close(pv[1]);
			close(pv2[1]);
			P::fdctx pf{}, rf{};
			ref_fdctx_init(pv[0], &rf);
			P::fdctx_init(pv2[0], &pf);
			Stat &st = S("fdctx_init");
			if (rf.buflen != 1 || pf.buflen != 1)
				fail(st, "pipe_buflen");
			else
				ok(st);
			char rc, pc;
			ref_fdgetc(&rf, &rc);
			P::fdgetc(&pf, &pc);
			Stat &stg = S("fdgetc");
			if (rc != pc)
				fail(stg, "pipe");
			else
				ok(stg);
			ref_fdctx_destroy(&rf);
			P::fdctx_destroy(&pf);
			close(pv[0]);
			close(pv2[0]);
		}
	}
}

static void
test_fdctx_sweep()
{
	for (long i = 0; i < SWEEP; i++) {
		size_t len = (size_t)(rng.u32() % 256);
		unsigned char data[256];
		for (size_t j = 0; j < len; j++)
			data[j] = (unsigned char)(rng.u32() & 0xff);
		test_fdctx_read(data, len, "sweep");
	}
}

/* --- nextopt --- */

static int
run_nextopt_ref(char **argv, const char *optstring, int *c_out, char **shopt_out,
    int *err_out)
{
	oracle_reset_all();
	oracle_set_argptr(argv);
	oracle_set_nextopt_optptr(NULL);
	int c = ref_nextopt(optstring);
	*c_out = c;
	*shopt_out = oracle_get_shoptarg();
	*err_out = oracle_get_error_thrown();
	return 0;
}

static int
run_nextopt_port(char **argv, const char *optstring, int *c_out, char **shopt_out,
    int *err_out)
{
	P::port_reset_all();
	P::argptr = argv;
	P::nextopt_optptr = NULL;
	P::shoptarg = NULL;
	int c = P::nextopt(optstring);
	*c_out = c;
	*shopt_out = P::shoptarg;
	*err_out = P::port_error_thrown();
	return 0;
}

static void
test_nextopt_case(char **argv, const char *optstring, const char *tag)
{
	Stat &st = S("nextopt");
	int rc = 0, pc = 0;
	int re = 0, pe = 0;
	char *rsh = NULL, *psh = NULL;
	run_nextopt_ref(argv, optstring, &rc, &rsh, &re);
	run_nextopt_port(argv, optstring, &pc, &psh, &pe);
	if (re != pe) {
		fail(st, tag);
		return;
	}
	if (re)
		return ok(st);
	if (rc != pc) {
		fail(st, tag);
		return;
	}
	if (rsh == NULL && psh == NULL) {
		ok(st);
		return;
	}
	if (rsh == NULL || psh == NULL || std::strcmp(rsh, psh) != 0)
		fail(st, tag);
	else
		ok(st);
}

static void
test_nextopt_hand()
{
	char a0[] = "-a";
	char a1[] = "-ab";
	char a2[] = "-a";
	char a3[] = "arg";
	char a4[] = "--";
	char a5[] = "-z";
	char a6[] = "-a";
	char *av0[] = { a0, NULL };
	char *av1[] = { a1, NULL };
	char *av2[] = { a2, a3, NULL };
	char *av3[] = { a4, NULL };
	char *av4[] = { NULL };
	char *av5[] = { a5, NULL };
	char *av6[] = { a6, NULL };

	test_nextopt_case(av0, "ab", "a");
	test_nextopt_case(av1, "ab", "ab");
	test_nextopt_case(av2, "a:", "a_arg");
	test_nextopt_case(av3, "ab", "dashdash");
	test_nextopt_case(av4, "ab", "nullargv");
	test_nextopt_case(av5, "ab", "illegal");
	test_nextopt_case(av6, "a:", "missing_arg");
}

static void
test_nextopt_sweep()
{
	Stat &st = S("nextopt");
	char optstring[16];
	char argbuf[8][64];
	char *argv[9];
	for (long i = 0; i < SWEEP; i++) {
		size_t on = (size_t)(rng.u32() % 6) + 1;
		for (size_t k = 0; k < on; k++)
			optstring[k] = (char)('a' + (rng.u32() % 6));
		optstring[on] = '\0';

		int argidx = 0;
		int opt_args = (int)(rng.u32() % 4);
		for (int j = 0; j < opt_args && argidx < 7; j++) {
			size_t n = (size_t)(rng.u32() % 4) + 1;
			argbuf[argidx][0] = '-';
			for (size_t k = 1; k <= n; k++) {
				size_t pick = (size_t)(rng.u32() % on);
				argbuf[argidx][k] = optstring[pick];
			}
			argbuf[argidx][n + 1] = '\0';
			argv[argidx++] = argbuf[argidx - 1];
			for (size_t k = 1; argbuf[argidx - 1][k] != '\0'; k++) {
				const char *q = optstring;
				while (*q && *q != argbuf[argidx - 1][k])
					q++;
				if (*q != '\0' && q[1] == ':' && argidx < 7) {
					size_t an = (size_t)(rng.u32() % 8);
					for (size_t a = 0; a < an; a++)
						argbuf[argidx][a] = (char)(rng.u32() & 0xff);
					argbuf[argidx][an] = '\0';
					argv[argidx++] = argbuf[argidx - 1];
					break;
				}
			}
		}
		int plain = (int)(rng.u32() % 3);
		for (int j = 0; j < plain && argidx < 7; j++) {
			size_t n = (size_t)(rng.u32() % 12);
			for (size_t k = 0; k < n; k++)
				argbuf[argidx][k] = (char)(rng.u32() & 0xff);
			argbuf[argidx][n] = '\0';
			argv[argidx++] = argbuf[argidx - 1];
		}
		argv[argidx] = NULL;

		int rc, pc, re, pe;
		char *rsh, *psh;
		run_nextopt_ref(argv, optstring, &rc, &rsh, &re);
		run_nextopt_port(argv, optstring, &pc, &psh, &pe);
		if (re != pe) {
			fail(st, "sweep");
			continue;
		}
		if (re)
			ok(st);
		else if (rc != pc || (rsh != psh && (rsh == NULL || psh == NULL ||
		    std::strcmp(rsh, psh) != 0)))
			fail(st, "sweep");
		else
			ok(st);
	}
}

/* --- getoptsreset --- *//* --- getoptsreset --- */

static void
test_getoptsreset_one(const char *v, int expect, const char *tag)
{
	Stat &st = S("getoptsreset");
	oracle_reset_all();
	P::port_reset_all();
	ref_getoptsreset(v);
	P::getoptsreset(v);
	if (oracle_get_shellparam_reset() != expect ||
	    P::port_get_shellparam_reset() != expect ||
	    oracle_get_shellparam_reset() != P::port_get_shellparam_reset())
		fail(st, tag);
	else
		ok(st);
}

static void
test_getoptsreset_hand()
{
	test_getoptsreset_one("1", 1, "one");
	test_getoptsreset_one("0001", 1, "zeros");
	test_getoptsreset_one("0", 0, "zero");
	test_getoptsreset_one("", 0, "empty");
	test_getoptsreset_one("01", 1, "leading_zero");
}

static void
test_getoptsreset_sweep()
{
	Stat &st = S("getoptsreset");
	char v[32];
	for (long i = 0; i < SWEEP; i++) {
		size_t n = (size_t)(rng.u32() % 20);
		for (size_t j = 0; j < n; j++)
			v[j] = (char)('0' + (rng.u32() % 10));
		v[n] = '\0';
		oracle_reset_all();
		P::port_reset_all();
		ref_getoptsreset(v);
		P::getoptsreset(v);
		if (oracle_get_shellparam_reset() != P::port_get_shellparam_reset())
			fail(st, "sweep");
		else
			ok(st);
	}
}

/* --- freeparam --- */

static void
test_freeparam_one(int malloc_p, int optp_set, const char *tag)
{
	Stat &st = S("freeparam");
	P::shparam rp, pp;
	char *rs1 = ::strdup("a");
	char *rs2 = ::strdup("b");
	char *ps1 = ::strdup("a");
	char *ps2 = ::strdup("b");
	char **rarr = (char **)std::malloc(3 * sizeof(char *));
	char **parr = (char **)std::malloc(3 * sizeof(char *));
	rarr[0] = rs1;
	rarr[1] = rs2;
	rarr[2] = NULL;
	parr[0] = ps1;
	parr[1] = ps2;
	parr[2] = NULL;
	std::memset(&rp, 0, sizeof(rp));
	std::memset(&pp, 0, sizeof(pp));
	rp.malloc = malloc_p;
	pp.malloc = malloc_p;
	rp.p = malloc_p ? rarr : NULL;
	pp.p = malloc_p ? parr : NULL;
	if (optp_set) {
		rp.optp = (char **)std::malloc(2 * sizeof(char *));
		pp.optp = (char **)std::malloc(2 * sizeof(char *));
		rp.optp[0] = ::strdup("x");
		rp.optp[1] = NULL;
		pp.optp[0] = ::strdup("x");
		pp.optp[1] = NULL;
	}
	ref_freeparam(&rp);
	P::freeparam(&pp);
	ok(st);
}

static void
test_freeparam_hand()
{
	test_freeparam_one(1, 0, "malloc");
	test_freeparam_one(0, 0, "nomalloc");
	test_freeparam_one(1, 1, "optp");
}

static void
test_freeparam_sweep()
{
	for (long i = 0; i < SWEEP; i++)
		test_freeparam_one(rng.coin(), rng.coin(), "sweep");
}

/* --- getopts --- */

struct GetoptsCtx {
	char **optfirst;
	char **optnext;
	char *optptr;
	char optstr[32];
	char optvar[16];
};

static void
reset_getopts_ctx(GetoptsCtx &ctx, char **argv, const char *optstr,
    const char *optvar)
{
	std::strncpy(ctx.optstr, optstr, sizeof(ctx.optstr) - 1);
	std::strncpy(ctx.optvar, optvar, sizeof(ctx.optvar) - 1);
	ctx.optfirst = argv;
	ctx.optnext = argv;
	ctx.optptr = NULL;
}

static void
drive_getopts_ref(GetoptsCtx &ctx, int *results, int max, int *n)
{
	oracle_reset_all();
	char **optnext = ctx.optfirst;
	char *optptr = NULL;
	*n = 0;
	while (*n < max) {
		results[*n] = ref_getopts(ctx.optstr, ctx.optvar, ctx.optfirst,
		    &optnext, &optptr);
		(*n)++;
		if (results[*n - 1] == 1)
			break;
	}
}

static void
drive_getopts_port(GetoptsCtx &ctx, int *results, int max, int *n)
{
	P::port_reset_all();
	char **optnext = ctx.optfirst;
	char *optptr = NULL;
	*n = 0;
	while (*n < max) {
		results[*n] = P::getopts(ctx.optstr, ctx.optvar, ctx.optfirst,
		    &optnext, &optptr);
		(*n)++;
		if (results[*n - 1] == 1)
			break;
	}
}

static void
test_getopts_case(char **argv, const char *optstr, const char *tag)
{
	Stat &st = S("getopts");
	GetoptsCtx ctx;
	reset_getopts_ctx(ctx, argv, optstr, "o");
	int rr[32], pr[32], nr, np;
	drive_getopts_ref(ctx, rr, 32, &nr);
	reset_getopts_ctx(ctx, argv, optstr, "o");
	drive_getopts_port(ctx, pr, 32, &np);
	if (nr != np) {
		fail(st, tag);
		return;
	}
	for (int i = 0; i < nr; i++) {
		if (rr[i] != pr[i]) {
			fail(st, tag);
			return;
		}
	}
	if (std::strcmp(oracle_get_optvar(), P::port_get_optvar()) != 0 ||
	    std::strcmp(oracle_get_optind(), P::port_get_optind()) != 0 ||
	    oracle_optarg_was_set() != P::port_optarg_was_set() ||
	    oracle_optarg_was_unset() != P::port_optarg_was_unset() ||
	    (oracle_optarg_was_set() &&
	     std::strcmp(oracle_get_optarg(), P::port_get_optarg()) != 0)) {
		fail(st, tag);
		return;
	}
	ok(st);
}

static void
test_getopts_hand()
{
	char a0[] = "-a";
	char a1[] = "-ab";
	char a2[] = "-a";
	char a3[] = "val";
	char a4[] = "plain";
	char a5[] = "-x";
	char a6[] = "--";
	char *av0[] = { a0, a4, NULL };
	char *av1[] = { a1, a4, NULL };
	char *av2[] = { a2, a3, a4, NULL };
	char *av3[] = { a5, a4, NULL };
	char *av4[] = { a6, a4, NULL };
	char *av5[] = { a4, NULL };

	test_getopts_case(av0, "ab", "a");
	test_getopts_case(av1, "ab", "ab");
	test_getopts_case(av2, "a:", "a_arg");
	test_getopts_case(av3, "ab", "illegal");
	test_getopts_case(av4, "ab", "dashdash");
	test_getopts_case(av5, "ab", "noopt");

	/* silent mode */
	char *av6[] = { a5, a4, NULL };
	test_getopts_case(av6, ":ab", "silent");
}

static void
test_getopts_sweep()
{
	Stat &st = S("getopts");
	char argbuf[8][64];
	char *argv[9];
	for (long i = 0; i < SWEEP; i++) {
		int nargs = (int)(rng.u32() % 5) + 1;
		for (int j = 0; j < nargs; j++) {
			if (rng.coin()) {
				size_t n = (size_t)(rng.u32() % 8) + 1;
				argbuf[j][0] = '-';
				for (size_t k = 1; k <= n; k++)
					argbuf[j][k] = (char)('a' + (rng.u32() % 4));
				argbuf[j][n + 1] = '\0';
			} else {
				size_t n = (size_t)(rng.u32() % 12);
				for (size_t k = 0; k < n; k++)
					argbuf[j][k] = (char)(rng.u32() & 0xff);
				argbuf[j][n] = '\0';
			}
			argv[j] = argbuf[j];
		}
		argv[nargs] = NULL;
		char optstr[12];
		size_t on = (size_t)(rng.u32() % 6) + 1;
		if (rng.coin())
			optstr[0] = ':';
		else
			optstr[0] = '\0';
		for (size_t k = 0; k < on; k++)
			optstr[k + (optstr[0] ? 1 : 0)] =
			    (char)('a' + (rng.u32() % 4));
		optstr[on + (optstr[0] ? 1 : 0)] = '\0';
		GetoptsCtx ctx;
		reset_getopts_ctx(ctx, argv, optstr, "v");
		int rr[32], pr[32], nr, np;
		drive_getopts_ref(ctx, rr, 32, &nr);
		reset_getopts_ctx(ctx, argv, optstr, "v");
		drive_getopts_port(ctx, pr, 32, &np);
		if (nr != np) {
			fail(st, "sweep");
			continue;
		}
		int bad = 0;
		for (int k = 0; k < nr && !bad; k++)
			if (rr[k] != pr[k])
				bad = 1;
		if (bad ||
		    std::strcmp(oracle_get_optvar(), P::port_get_optvar()) != 0 ||
		    std::strcmp(oracle_get_optind(), P::port_get_optind()) != 0)
			fail(st, "sweep");
		else
			ok(st);
	}
}

int
main()
{
	test_isbinary_hand();
	test_isbinary_sweep();
	test_hashname_hand();
	test_hashname_sweep();
	test_fdctx_hand();
	test_fdctx_sweep();
	test_nextopt_hand();
	test_nextopt_sweep();
	test_getoptsreset_hand();
	test_getoptsreset_sweep();
	test_freeparam_hand();
	test_freeparam_sweep();
	test_getopts_hand();
	test_getopts_sweep();

	std::printf("\n%-16s %12s %12s\n", "function", "cases", "failures");
	unsigned long long total_fails = 0;
	for (int i = 0; i < NSTAT; i++) {
		std::printf("%-16s %12llu %12llu\n",
		    stats[i].name, stats[i].cases, stats[i].fails);
		total_fails += stats[i].fails;
	}
	return total_fails == 0 ? 0 : 1;
}
