/*
 * b0175 differential harness.
 *
 * Every ported function is driven with hand written edge cases plus a fixed
 * seed randomised sweep, and the C++23 port is compared against the C oracle
 * on every observable:
 *
 *	parse_interval	- the exact bit pattern of the returned double (so that
 *			  NaN payloads, -0.0 and infinities are all distinct),
 *			  the bytes warnx() wrote to stderr, and both input
 *			  buffers in full (guard filled with 0x7f) so a write
 *			  through the const char * cannot go unnoticed.
 *	report_request	- the value of the flag after the call, for several
 *			  pre-call values, plus stderr.
 *	usage		- forked: exit status, terminating signal and the whole
 *			  stderr text are compared.
 *
 * stderr is redirected to an unlinked temporary file for the in-process tests;
 * the offset before/after each call delimits exactly what that call wrote.
 */

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

import pbsd.bin.sleep.b0175;

namespace P = pbsd::bin_sleep::b0175;

extern "C" {
double ref_parse_interval(const char *arg);
void ref_report_request(int signo);
__attribute__((__noreturn__)) void ref_usage(void);
extern volatile sig_atomic_t report_requested;
}

/* ------------------------------------------------------------------ stats */

struct Stats {
	const char *name;
	unsigned long cases;
	unsigned long fails;
};

static Stats S_parse = { "parse_interval", 0, 0 };
static Stats S_report = { "report_request", 0, 0 };
static Stats S_usage = { "usage", 0, 0 };

static const unsigned long MAX_REPORTED = 12;

/* ------------------------------------------------------------------- rng  */

static uint64_t rng_state;

static void
rng_seed(uint64_t s)
{
	rng_state = s ? s : 0x9e3779b97f4a7c15ULL;
}

static uint64_t
rnd(void)
{
	uint64_t x = rng_state;

	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rng_state = x;
	return (x);
}

static uint32_t
rnd_below(uint32_t n)
{
	return (n == 0 ? 0 : (uint32_t)(rnd() % n));
}

/* --------------------------------------------------------- stderr capture */

static int cap_fd = -1;
static char capA[8192];
static char capB[8192];

static int
capture_setup(void)
{
	char tmpl[] = "/tmp/pbsd-b0175-stderr-XXXXXX";
	int fd;

	fd = mkstemp(tmpl);
	if (fd < 0)
		return (-1);
	if (freopen(tmpl, "w+", stderr) == NULL) {
		close(fd);
		unlink(tmpl);
		return (-1);
	}
	unlink(tmpl);
	close(fd);
	setvbuf(stderr, NULL, _IONBF, 0);
	cap_fd = fileno(stderr);
	return (cap_fd < 0 ? -1 : 0);
}

static void
cap_begin(void)
{
	fflush(stderr);
	rewind(stderr);
}

/*
 * Returns the number of bytes the call wrote, or a negative value on capture
 * failure; *got is how many of them were read back into out.
 */
static long
cap_end(char *out, size_t cap, size_t *got)
{
	off_t pos;
	size_t n;
	ssize_t r;

	*got = 0;
	out[0] = '\0';
	fflush(stderr);
	pos = ftello(stderr);
	if (pos < 0)
		return (-1);
	n = (size_t)pos;
	if (n > cap - 1)
		n = cap - 1;
	if (n > 0) {
		r = pread(cap_fd, out, n, 0);
		if (r < 0)
			return (-2);
		n = (size_t)r;
	}
	out[n] = '\0';
	*got = n;
	return ((long)pos);
}

/* ------------------------------------------------------------- buffers    */

enum { BUFSZ = 384 };

static const unsigned char GUARD = 0x7f;

static unsigned char bufA[BUFSZ];
static unsigned char bufB[BUFSZ];
static unsigned char bufP[BUFSZ];	/* pristine copy */

static size_t
load(const unsigned char *in, size_t n)
{
	if (n > BUFSZ - 1)
		n = BUFSZ - 1;
	memset(bufP, GUARD, BUFSZ);
	memcpy(bufP, in, n);
	bufP[n] = '\0';
	memcpy(bufA, bufP, BUFSZ);
	memcpy(bufB, bufP, BUFSZ);
	return (n);
}

static void
escape(const unsigned char *p, size_t n, char *out, size_t cap)
{
	size_t i, o = 0;

	for (i = 0; i < n; i++) {
		unsigned char c = p[i];

		if (o + 6 >= cap)
			break;
		if (c >= 0x20 && c < 0x7f && c != '\\' && c != '"')
			out[o++] = (char)c;
		else
			o += (size_t)snprintf(out + o, cap - o, "\\x%02x", c);
	}
	out[o] = '\0';
}

static void
dump_buf_diff(const unsigned char *x, const unsigned char *y)
{
	size_t i;

	for (i = 0; i < BUFSZ; i++)
		if (x[i] != y[i]) {
			printf("      first differing byte at offset %zu: "
			    "0x%02x vs 0x%02x\n", i, x[i], y[i]);
			return;
		}
}

/* ------------------------------------------------------- parse_interval   */

static void
check_parse(const unsigned char *in, size_t n, const char *tag)
{
	double a, b;
	long la, lb;
	size_t ga, gb;
	const char *why = NULL;

	n = load(in, n);
	S_parse.cases++;

	cap_begin();
	a = P::parse_interval((const char *)bufA);
	la = cap_end(capA, sizeof(capA), &ga);

	cap_begin();
	b = ref_parse_interval((const char *)bufB);
	lb = cap_end(capB, sizeof(capB), &gb);

	if (memcmp(&a, &b, sizeof(a)) != 0)
		why = "return value";
	else if (la < 0 || lb < 0)
		why = "stderr capture failed";
	else if (la != lb)
		why = "stderr byte count";
	else if (ga != gb || memcmp(capA, capB, ga) != 0)
		why = "stderr text";
	else if (memcmp(bufA, bufB, BUFSZ) != 0)
		why = "argument buffer differs between port and oracle";
	else if (memcmp(bufA, bufP, BUFSZ) != 0)
		why = "port modified its argument buffer";
	else if (memcmp(bufB, bufP, BUFSZ) != 0)
		why = "oracle modified its argument buffer";

	if (why == NULL)
		return;

	S_parse.fails++;
	if (S_parse.fails > MAX_REPORTED)
		return;

	{
		char esc[1600];
		unsigned long long ba, bb;

		memcpy(&ba, &a, sizeof(ba));
		memcpy(&bb, &b, sizeof(bb));
		escape(in, n, esc, sizeof(esc));
		printf("  FAIL parse_interval (%s): %s\n", tag, why);
		printf("    arg      = \"%s\"  (len %zu)\n", esc, n);
		printf("    port ret = %.17g  (0x%016llx)\n", a, ba);
		printf("    ref  ret = %.17g  (0x%016llx)\n", b, bb);
		printf("    port err = [%s] (%ld bytes)\n", capA, la);
		printf("    ref  err = [%s] (%ld bytes)\n", capB, lb);
		if (memcmp(bufA, bufB, BUFSZ) != 0)
			dump_buf_diff(bufA, bufB);
	}
}

struct Lit {
	const char *s;
	size_t n;
};

#define	L(x)	{ x, sizeof(x) - 1 }

static const Lit edge_cases[] = {
	/* empty and whitespace */
	L(""), L(" "), L("  "), L("\t"), L("\n"), L("\r"), L("\v"), L("\f"),
	L(" \t "),
	/* bare numbers: sscanf returns 1 */
	L("0"), L("1"), L("-1"), L("+1"), L("-0"), L("+0"), L(".5"), L("5."),
	L("1.5"), L("00001"), L("123456789"), L("0.0"), L("-0.0"),
	/* every unit, both cases */
	L("1s"), L("1m"), L("1h"), L("1d"),
	L("1S"), L("1M"), L("1H"), L("1D"),
	L("0s"), L("0m"), L("0h"), L("0d"),
	L("-1s"), L("-1m"), L("-1h"), L("-1d"),
	L("2s"), L("2m"), L("2h"), L("2d"),
	/* the multiply chain must be distinguishable at each step */
	L("1d"), L("24h"), L("1440m"), L("86400s"),
	L("0.1"), L("0.1s"), L("0.1m"), L("0.1h"), L("0.1d"),
	L("1e-3d"), L("1e-3h"), L("1e-3m"), L("1e-3s"),
	L("7.25d"), L("7.25h"), L("7.25m"), L("7.25s"),
	/* units that match nothing: sscanf returns 2, switch falls out */
	L("1x"), L("1a"), L("1z"), L("1c"), L("1e"), L("1g"), L("1i"),
	L("1l"), L("1n"), L("1r"), L("1t"), L("1 "), L("1\t"), L("1\n"),
	L("1."), L("1-"), L("1+"), L("1/"), L("1:"), L("1["), L("1{"),
	/* sscanf returns 3 */
	L("1sx"), L("1dx"), L("1ss"), L("1dd"), L("1dh"), L("1hm"), L("1ms"),
	L("1 s"), L("1d "), L("1dxyz"), L("1xd"),
	/* NaN through every path */
	L("nan"), L("NAN"), L("NaN"), L("-nan"), L("+nan"), L("nan(0)"),
	L("nans"), L("nanm"), L("nanh"), L("nand"), L("nanx"), L("nan(1)s"),
	L("nan(1)d"), L("nansx"), L("nandx"),
	/* infinities */
	L("inf"), L("INF"), L("Inf"), L("-inf"), L("infinity"), L("INFINITY"),
	L("infs"), L("infm"), L("infh"), L("infd"), L("infx"), L("-infd"),
	L("infinityd"), L("infinitys"),
	/* overflow / underflow / denormals */
	L("1e400"), L("1e400s"), L("1e400d"), L("-1e400"), L("-1e400d"),
	L("1e-400"), L("1e-400d"), L("5e-324"), L("5e-324d"), L("5e-324m"),
	L("1.7976931348623157e308"), L("1.7976931348623157e308d"),
	L("1.7976931348623157e308s"), L("2.2250738585072014e-308"),
	L("2.2250738585072014e-308d"),
	/* hex floats */
	L("0x10"), L("0x10d"), L("0x1p3"), L("0x1p3d"), L("0X1P-3"),
	L("0x"), L("0xg"), L("0x1p"), L("0x1ps"),
	/* around the INT_MAX check in the caller, and around 1e-9 */
	L("2147483647"), L("2147483648"), L("2147483647.5"), L("1e9"),
	L("1e-9"), L("1e-10"), L("1e-9s"), L("1e-10d"),
	/* nothing numeric at all: sscanf returns 0 or EOF */
	L("abc"), L("d"), L("h"), L("m"), L("s"), L("x"), L("."), L("-"),
	L("+"), L("e"), L("E"), L("-."), L("+."), L(".e"), L("..1"),
	L("-abc"), L("_1d"), L("=1d"),
	/* high bit bytes */
	L("\x80"), L("\xff"), L("\xfe"), L("\x81"), L("\x7f"),
	L("1\x80"), L("1\xff"), L("1\x7f"), L("1\xfe"), L("5\x80"),
	L("\x80\x80"), L("\xff\xff"), L("1\xc3\xa9"), L("\xc3\xa9" "1d"),
	L("1d\x80"), L("1\x80" "d"), L("nan\x80"), L("inf\xff"),
	/* leading whitespace is skipped by %lf but not by %c */
	L("  1d"), L("\t1d"), L("\n1d"), L("1  d"), L(" 1 d"),
	/* NUL heavy */
	L("\0"), L("1\0"), L("1d\0"), L("1d\0s"), L("\0" "1d"),
	L("1\0" "d"), L("\0\0\0"), L("nan\0d"), L("1\0\0\0\0"),
	L("\0" "9999"), L("12\0" "34d"),
};

static const size_t N_EDGE = sizeof(edge_cases) / sizeof(edge_cases[0]);

static const char *const specials[] = {
	"nan", "NAN", "NaN", "nan(0)", "nan(1)", "-nan", "inf", "INF", "Inf",
	"-inf", "infinity", "INFINITY", "0x1p+3", "0x1P-3", "0x10", "0X1f",
	"0x", "1e400", "-1e400", "1e-400", "5e-324", "1.7976931348623157e308",
	"2.2250738585072014e-308", "2147483647", "2147483648", "86400",
	"1440", "24", "1e9", "1e-9", "1e-10", "0.1", "-0.0", "0", "1", "-1",
	"", ".", "-", "+", "e", "x", "abc", "d", "s", "m", "h",
};

static const size_t N_SPEC = sizeof(specials) / sizeof(specials[0]);

static const unsigned char unit_alphabet[] = {
	'd', 'h', 'm', 's', 'D', 'H', 'M', 'S', 'c', 'e', 'g', 'i', 'l', 'n',
	'r', 't', 'x', 'X', 'a', 'z', '0', '9', '.', 'E', '+', '-', ' ',
	'\t', '\n', '\r', '\v', '\f', '/', '(', ')', '\0', 0x01, 0x1f, 0x7f,
	0x80, 0x81, 0xa9, 0xc3, 0xfe, 0xff,
};

static const size_t N_UNIT = sizeof(unit_alphabet) / sizeof(unit_alphabet[0]);

static size_t
gen_input(unsigned char *out)
{
	size_t n = 0;
	uint32_t mode = rnd_below(100);
	uint32_t i, k;

	if (mode < 20) {
		/* free form bytes, full 0x00-0xff range */
		size_t len = rnd_below(16) == 0 ? 1 + rnd_below(200)
					        : rnd_below(13);
		for (i = 0; i < len; i++)
			out[n++] = (unsigned char)rnd_below(256);
		return (n);
	}
	if (mode < 45) {
		const char *sp = specials[rnd_below((uint32_t)N_SPEC)];
		size_t l = strlen(sp);

		memcpy(out + n, sp, l);
		n += l;
	} else {
		if (rnd_below(8) == 0)
			out[n++] = rnd_below(2) ? ' ' : '\t';
		if (rnd_below(3) == 0)
			out[n++] = rnd_below(2) ? '-' : '+';
		k = 1 + rnd_below(6);
		for (i = 0; i < k; i++)
			out[n++] = (unsigned char)('0' + rnd_below(10));
		if (rnd_below(2) == 0) {
			out[n++] = '.';
			k = rnd_below(5);
			for (i = 0; i < k; i++)
				out[n++] = (unsigned char)('0' + rnd_below(10));
		}
		if (rnd_below(4) == 0) {
			out[n++] = rnd_below(2) ? 'e' : 'E';
			if (rnd_below(2))
				out[n++] = rnd_below(2) ? '+' : '-';
			k = 1 + rnd_below(3);
			for (i = 0; i < k; i++)
				out[n++] = (unsigned char)('0' + rnd_below(10));
		}
	}
	if (rnd_below(4) != 0)
		out[n++] = unit_alphabet[rnd_below((uint32_t)N_UNIT)];
	k = rnd_below(4);
	if (k == 3)
		k = 1 + rnd_below(5);
	for (i = 0; i < k; i++)
		out[n++] = unit_alphabet[rnd_below((uint32_t)N_UNIT)];
	return (n);
}

static void
test_parse_interval(void)
{
	unsigned char in[BUFSZ];
	size_t i;
	unsigned v;
	long iter;

	for (i = 0; i < N_EDGE; i++)
		check_parse((const unsigned char *)edge_cases[i].s,
		    edge_cases[i].n, "edge");

	/* every single byte on its own */
	for (v = 0; v < 256; v++) {
		in[0] = (unsigned char)v;
		check_parse(in, 1, "byte");
	}
	/* every possible unit character, for several leading numbers */
	{
		static const char *const heads[] = { "1", "5", "-2", "0",
		    "nan", "inf", "1.5e2", "" };
		size_t h;

		for (h = 0; h < sizeof(heads) / sizeof(heads[0]); h++) {
			size_t hl = strlen(heads[h]);

			for (v = 0; v < 256; v++) {
				memcpy(in, heads[h], hl);
				in[hl] = (unsigned char)v;
				check_parse(in, hl + 1, "unit");
				in[hl + 1] = 'x';
				check_parse(in, hl + 2, "unit+extra");
			}
		}
	}
	/* boundary lengths */
	for (i = 0; i <= 40; i++) {
		size_t j;

		for (j = 0; j < i; j++)
			in[j] = (unsigned char)('0' + (j % 10));
		check_parse(in, i, "len-digits");
		if (i > 0) {
			in[i - 1] = 'd';
			check_parse(in, i, "len-d");
			in[i - 1] = '\0';
			check_parse(in, i, "len-nul");
		}
	}
	/* long inputs up to the buffer limit */
	for (i = BUFSZ - 8; i < BUFSZ + 4; i++) {
		size_t j, len = i < BUFSZ ? i : BUFSZ - 1;

		for (j = 0; j < len; j++)
			in[j] = '9';
		check_parse(in, len, "long-digits");
		in[len - 1] = 'd';
		check_parse(in, len, "long-d");
	}

	rng_seed(0xb0175ULL ^ 0x9e3779b97f4a7c15ULL);
	for (iter = 0; iter < 250000; iter++) {
		size_t n = gen_input(in);

		check_parse(in, n, "random");
	}
}

/* ------------------------------------------------------- report_request   */

static void
check_report(int signo, long pre)
{
	long va, vb, la, lb;
	size_t ga, gb;
	const char *why = NULL;

	S_report.cases++;
	P::report_requested = (sig_atomic_t)pre;
	report_requested = (sig_atomic_t)pre;

	cap_begin();
	P::report_request(signo);
	la = cap_end(capA, sizeof(capA), &ga);
	va = (long)P::report_requested;

	cap_begin();
	ref_report_request(signo);
	lb = cap_end(capB, sizeof(capB), &gb);
	vb = (long)report_requested;

	if (va != vb)
		why = "flag value after call";
	else if (la < 0 || lb < 0)
		why = "stderr capture failed";
	else if (la != lb)
		why = "stderr byte count";
	else if (ga != gb || memcmp(capA, capB, ga) != 0)
		why = "stderr text";

	if (why == NULL)
		return;
	S_report.fails++;
	if (S_report.fails > MAX_REPORTED)
		return;
	printf("  FAIL report_request: %s\n", why);
	printf("    signo = %d, flag before = %ld\n", signo, pre);
	printf("    port flag = %ld, ref flag = %ld\n", va, vb);
	printf("    port err = [%s], ref err = [%s]\n", capA, capB);
}

static void
test_report_request(void)
{
	static const int signos[] = { 0, 1, -1, 2, 15, 29, 31, 63, 64, 127,
	    128, 255, 256, -128, INT_MAX, INT_MIN, SIGUSR1, SIGINT };
	static const long pres[] = { 0, 1, -1, 2, 7, -7, 127, 128, 255,
	    (long)INT_MAX, (long)INT_MIN };
	size_t i, j;
	long iter;

	for (i = 0; i < sizeof(signos) / sizeof(signos[0]); i++)
		for (j = 0; j < sizeof(pres) / sizeof(pres[0]); j++)
			check_report(signos[i], pres[j]);

	rng_seed(0x5eed0175ULL);
	for (iter = 0; iter < 200000; iter++) {
		int signo = (int)(int32_t)(uint32_t)rnd();
		long pre;

		switch (rnd_below(4)) {
		case 0:
			pre = 0;
			break;
		case 1:
			pre = 1;
			break;
		case 2:
			pre = (long)(int32_t)(uint32_t)rnd();
			break;
		default:
			pre = (long)(rnd_below(5)) - 2;
			break;
		}
		check_report(signo, pre);
	}
}

/* ------------------------------------------------------------------ usage */

struct Child {
	int failed;
	int exited;
	int code;
	int sig;
	size_t len;
	char buf[4096];
};

static int
run_child(void (*fn)(void), Child *c)
{
	int pfd[2];
	pid_t pid;
	int st = 0;

	c->failed = 0;
	c->exited = -1;
	c->code = -1;
	c->sig = -1;
	c->len = 0;
	c->buf[0] = '\0';

	fflush(stdout);
	fflush(stderr);
	if (pipe(pfd) != 0)
		return (-1);
	pid = fork();
	if (pid < 0) {
		close(pfd[0]);
		close(pfd[1]);
		return (-1);
	}
	if (pid == 0) {
		close(pfd[0]);
		if (dup2(pfd[1], STDERR_FILENO) < 0)
			_exit(120);
		close(pfd[1]);
		fn();
		_exit(121);
	}
	close(pfd[1]);
	for (;;) {
		ssize_t r = read(pfd[0], c->buf + c->len,
		    sizeof(c->buf) - 1 - c->len);

		if (r <= 0)
			break;
		c->len += (size_t)r;
		if (c->len >= sizeof(c->buf) - 1)
			break;
	}
	c->buf[c->len] = '\0';
	close(pfd[0]);
	if (waitpid(pid, &st, 0) < 0)
		return (-1);
	if (WIFEXITED(st)) {
		c->exited = 1;
		c->code = WEXITSTATUS(st);
	} else if (WIFSIGNALED(st)) {
		c->exited = 0;
		c->sig = WTERMSIG(st);
	}
	return (0);
}

static void
port_usage_trampoline(void)
{
	P::usage();
}

static void
ref_usage_trampoline(void)
{
	ref_usage();
}

static void
test_usage(void)
{
	static Child a, b;
	long iter;

	for (iter = 0; iter < 512; iter++) {
		const char *why = NULL;

		S_usage.cases++;
		if (run_child(port_usage_trampoline, &a) != 0 ||
		    run_child(ref_usage_trampoline, &b) != 0)
			why = "fork/pipe failed";
		else if (a.exited != b.exited)
			why = "one exited, the other was signalled";
		else if (a.code != b.code)
			why = "exit status";
		else if (a.sig != b.sig)
			why = "terminating signal";
		else if (a.len != b.len)
			why = "stderr byte count";
		else if (memcmp(a.buf, b.buf, a.len) != 0)
			why = "stderr text";

		if (why == NULL)
			continue;
		S_usage.fails++;
		if (S_usage.fails > MAX_REPORTED)
			continue;
		printf("  FAIL usage: %s\n", why);
		printf("    port: exited=%d code=%d sig=%d len=%zu\n",
		    a.exited, a.code, a.sig, a.len);
		printf("    ref : exited=%d code=%d sig=%d len=%zu\n",
		    b.exited, b.code, b.sig, b.len);
		printf("    port err = [%s]\n", a.buf);
		printf("    ref  err = [%s]\n", b.buf);
	}
}

/* ------------------------------------------------------------------ main  */

static void
row(const Stats &s)
{
	printf("  %-20s %10lu %10lu   %s\n", s.name, s.cases, s.fails,
	    s.fails == 0 ? "ok" : "FAILED");
}

int
main(void)
{
	unsigned long total_cases, total_fails;

	printf("b0175 sleep: pbsd.bin.sleep.b0175 vs oracle.c\n");

	/* forks first: the children must inherit the real stderr */
	test_usage();

	if (capture_setup() != 0) {
		printf("harness: cannot redirect stderr for capture: %s\n",
		    strerror(errno));
		return (1);
	}

	test_parse_interval();
	test_report_request();

	fflush(stderr);

	total_cases = S_parse.cases + S_report.cases + S_usage.cases;
	total_fails = S_parse.fails + S_report.fails + S_usage.fails;

	printf("\n  %-20s %10s %10s\n", "function", "cases", "failures");
	printf("  %-20s %10s %10s\n", "--------------------", "----------",
	    "----------");
	row(S_parse);
	row(S_report);
	row(S_usage);
	printf("  %-20s %10s %10s\n", "--------------------", "----------",
	    "----------");
	printf("  %-20s %10lu %10lu   %s\n", "TOTAL", total_cases, total_fails,
	    total_fails == 0 ? "PASS" : "FAIL");

	return (total_fails == 0 ? 0 : 1);
}
