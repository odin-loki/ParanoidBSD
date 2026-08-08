/*
 * PBSD batch b0197 -- differential harness for hbsd/src/bin/timeout/timeout.c
 *
 * Every ported function is driven twice per case: once through the ref_ oracle
 * built from the untouched C source, once through the C++23 module port.  The
 * two runs are compared byte for byte over a capture record that holds every
 * observable effect the functions have -- return value bits, the err(3)/errx(3)
 * exit code and message, the itimerval handed to setitimer(2), the text handed
 * to vwarnx(3), and the file-scope signal state.
 *
 * These functions have no caller-supplied output buffer, so the guard-byte
 * discipline is applied to the capture record and to every input buffer: both
 * are filled with 0x7f and compared in full, so a write past the nominal window
 * (or any write at all to a const input) shows up as a mismatch.
 *
 * The libc entry points that would otherwise end the process (err, errx) or
 * touch global machine state (setitimer) are interposed here; oracle and port
 * both resolve to these definitions, so the diff stays honest.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <cerrno>
#include <cmath>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <err.h>
#include <signal.h>
#include <sys/time.h>

import pbsd.bin.timeout.b0197;

namespace port = pbsd::bin_timeout::b0197;

#ifndef SIGEMT
#define SIGEMT 32
#endif

/* ------------------------------------------------------------------ oracle */

extern "C" {
extern volatile sig_atomic_t sig_chld;
extern volatile sig_atomic_t sig_alrm;
extern volatile sig_atomic_t sig_term;
extern volatile sig_atomic_t sig_other;
extern int killsig;
extern bool verbose;

__attribute__((__format__(__printf__, 1, 2))) void ref_logv(const char *fmt, ...);
double ref_parse_duration(const char *duration);
void ref_sig_handler(int signo);
void ref_set_interval(double iv);
void ref_log_termination(const char *name, const siginfo_t *si);
}

/* ----------------------------------------------------------------- capture */

enum { MSGCAP = 256, LOGCAP = 512 };

struct Cap {
	int err_kind; /* 0 = returned normally, 1 = errx(3), 2 = err(3) */
	int err_code;
	int err_errno;
	int returned;
	int log_calls;
	int itimer_calls;
	int itimer_which;
	int itimer_old_null;
	int sig_chld_after;
	int sig_alrm_after;
	int sig_term_after;
	int sig_other_after;
	int killsig_after;
	int pad_guard;
	double ret;
	unsigned char itimer_new[sizeof(struct itimerval)];
	char err_msg[MSGCAP];
	char log_buf[LOGCAP];
};

static Cap g_a; /* oracle side */
static Cap g_b; /* port side */
static Cap *g_cur = &g_a;
static jmp_buf g_jmp;
static int g_setitimer_fail = 0;

static void
cap_reset(Cap *c)
{
	std::memset(c, 0x7f, sizeof(*c));
	c->err_kind = 0;
	c->err_code = 0;
	c->err_errno = 0;
	c->returned = 0;
	c->log_calls = 0;
	c->itimer_calls = 0;
	c->itimer_which = 0;
	c->itimer_old_null = 0;
	c->sig_chld_after = 0;
	c->sig_alrm_after = 0;
	c->sig_term_after = 0;
	c->sig_other_after = 0;
	c->killsig_after = 0;
	c->pad_guard = 0;
	c->ret = 0.0;
	errno = 0;
}

extern "C" void
errx(int eval, const char *fmt, ...)
{
	Cap *c = g_cur;
	va_list ap;

	c->err_kind = 1;
	c->err_code = eval;
	c->err_errno = errno;
	va_start(ap, fmt);
	std::vsnprintf(c->err_msg, sizeof(c->err_msg), fmt, ap);
	va_end(ap);
	std::longjmp(g_jmp, 1);
}

extern "C" void
err(int eval, const char *fmt, ...)
{
	Cap *c = g_cur;
	va_list ap;

	c->err_kind = 2;
	c->err_code = eval;
	c->err_errno = errno;
	va_start(ap, fmt);
	std::vsnprintf(c->err_msg, sizeof(c->err_msg), fmt, ap);
	va_end(ap);
	std::longjmp(g_jmp, 1);
}

extern "C" void
vwarnx(const char *fmt, va_list ap)
{
	Cap *c = g_cur;

	c->log_calls++;
	std::vsnprintf(c->log_buf, sizeof(c->log_buf), fmt, ap);
}

extern "C" int
setitimer(__itimer_which_t which, const struct itimerval *__restrict nv,
    struct itimerval *__restrict ov) noexcept
{
	Cap *c = g_cur;

	c->itimer_calls++;
	c->itimer_which = which;
	c->itimer_old_null = (ov == NULL);
	std::memcpy(c->itimer_new, nv, sizeof(c->itimer_new));
	if (g_setitimer_fail) {
		errno = EINVAL;
		return (-1);
	}
	return (0);
}

#define RUN_SIDE(cap, ...)				\
	do {						\
		g_cur = &(cap);				\
		cap_reset(&(cap));			\
		if (setjmp(g_jmp) == 0) {		\
			__VA_ARGS__;			\
			(cap).returned = 1;		\
		}					\
	} while (0)

/* ------------------------------------------------------------- bookkeeping */

enum { F_LOGV, F_PARSE, F_SIG, F_SETIV, F_LOGTERM, F_COUNT };

static const char *const fname[F_COUNT] = {
	"logv",
	"parse_duration",
	"sig_handler",
	"set_interval",
	"log_termination",
};

static long cases_run[F_COUNT];
static long fail_cnt[F_COUNT];
static int shown[F_COUNT];

enum { MAX_SHOW = 5 };

static void
hexdump(const char *label, const unsigned char *p, std::size_t n)
{
	std::printf("      %s [", label);
	for (std::size_t i = 0; i < n; i++)
		std::printf("%02x", p[i]);
	std::printf("]\n");
}

static void
show_cap(const char *label, const Cap *c)
{
	unsigned long long bits;

	std::memcpy(&bits, &c->ret, sizeof(bits));
	std::printf("      %-6s err_kind=%d code=%d errno=%d returned=%d\n",
	    label, c->err_kind, c->err_code, c->err_errno, c->returned);
	std::printf("             err_msg=\"%.*s\"\n", MSGCAP, c->err_msg);
	std::printf("             ret=%.17g bits=%016llx\n", c->ret, bits);
	std::printf("             log_calls=%d log=\"%.*s\"\n", c->log_calls,
	    LOGCAP, c->log_buf);
	std::printf("             itimer calls=%d which=%d old_null=%d\n",
	    c->itimer_calls, c->itimer_which, c->itimer_old_null);
	hexdump("itimerval", c->itimer_new, sizeof(c->itimer_new));
	std::printf("             sigs chld=%d alrm=%d term=%d other=%d "
	    "killsig=%d\n", c->sig_chld_after, c->sig_alrm_after,
	    c->sig_term_after, c->sig_other_after, c->killsig_after);
}

static bool
record(int f, bool ok, const char *desc)
{
	cases_run[f]++;
	if (ok)
		return (true);
	fail_cnt[f]++;
	if (shown[f] < MAX_SHOW) {
		shown[f]++;
		std::printf("  FAIL %s: %s\n", fname[f], desc);
		show_cap("oracle", &g_a);
		show_cap("port", &g_b);
	}
	return (false);
}

static bool
caps_match(void)
{
	return (std::memcmp(&g_a, &g_b, sizeof(Cap)) == 0);
}

/* -------------------------------------------------------------------- rng */

static std::uint64_t rng_state;

static inline std::uint64_t
rnd(void)
{
	std::uint64_t z = (rng_state += 0x9E3779B97F4A7C15ULL);

	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return (z ^ (z >> 31));
}

static inline std::uint32_t
rnd_below(std::uint32_t n)
{
	return ((std::uint32_t)(rnd() % n));
}

static inline int
rnd_int(void)
{
	return ((int)(std::uint32_t)rnd());
}

/* -------------------------------------------------------- shared fixtures */

enum { NAMEBUF = 96 };

static const char *const strpool[] = {
	"",
	"a",
	"child terminated",
	"collected zombie",
	"\x80\xff\x01",
	"%s%d%%",
	"\xc3\xa9\xc3\xa8 utf8",
	"0123456789012345678901234567890123456789",
	"\x7f\x7f\x7f",
	"tab\there",
};
enum { NSTRPOOL = (int)(sizeof(strpool) / sizeof(strpool[0])) };

/* ------------------------------------------------------------------- logv */

/*
 * The format strings must stay literal so the printf attribute on both logv
 * declarations still checks them; a macro keeps oracle and port in lockstep.
 */
#define LOGV_INVOKE(F, k, i1, i2, i3, s, ch)				\
	do {								\
		switch (k) {						\
		case 0:							\
			F("plain message");				\
			break;						\
		case 1:							\
			F("value %d", (i1));				\
			break;						\
		case 2:							\
			F("%s: %d", (s), (i2));				\
			break;						\
		case 3:							\
			F("%s", (s));					\
			break;						\
		case 4:							\
			F("%d %d %d", (i1), (i2), (i3));		\
			break;						\
		case 5:							\
			F("100%% done %c|", (ch));			\
			break;						\
		case 6:							\
			F("%ld %u %x", (long)(i1), (unsigned)(i2),	\
			    (unsigned)(i3));				\
			break;						\
		case 7:							\
			F("%s: pid=%d, exit=%d", (s), (i1), (i2));	\
			break;						\
		default:						\
			F("[%s][%d]", (s), (i3));			\
			break;						\
		}							\
	} while (0)

enum { NLOGV_FORMS = 9 };

static unsigned char sbuf_a[NAMEBUF];
static unsigned char sbuf_b[NAMEBUF];
static unsigned char sbuf_ref[NAMEBUF];

static void
fill_strbufs(const char *s)
{
	std::size_t len = std::strlen(s);

	if (len > NAMEBUF - 8)
		len = NAMEBUF - 8;
	std::memset(sbuf_ref, 0x7f, sizeof(sbuf_ref));
	std::memcpy(sbuf_ref, s, len);
	sbuf_ref[len] = '\0';
	std::memcpy(sbuf_a, sbuf_ref, sizeof(sbuf_a));
	std::memcpy(sbuf_b, sbuf_ref, sizeof(sbuf_b));
}

static bool
strbufs_intact(void)
{
	return (std::memcmp(sbuf_a, sbuf_ref, sizeof(sbuf_ref)) == 0 &&
	    std::memcmp(sbuf_b, sbuf_ref, sizeof(sbuf_ref)) == 0);
}

static void
logv_case(int k, int i1, int i2, int i3, const char *s, char ch, bool vb)
{
	char desc[256];

	fill_strbufs(s);
	verbose = vb;
	port::verbose = vb;

	RUN_SIDE(g_a, LOGV_INVOKE(ref_logv, k, i1, i2, i3,
	    (const char *)sbuf_a, ch));
	RUN_SIDE(g_b, LOGV_INVOKE(port::logv, k, i1, i2, i3,
	    (const char *)sbuf_b, ch));

	if (caps_match() && strbufs_intact())
		record(F_LOGV, true, "");
	else {
		std::snprintf(desc, sizeof(desc),
		    "form=%d i=(%d,%d,%d) ch=0x%02x verbose=%d s=\"%s\"",
		    k, i1, i2, i3, (unsigned)(unsigned char)ch, (int)vb, s);
		record(F_LOGV, false, desc);
	}
}

static void
test_logv(void)
{
	static const int ints[] = { 0, 1, -1, 2, -2, 127, 128, 255, 256, -128,
		-129, 65535, 65536, 2147483647, (-2147483647 - 1), 1000000 };
	static const char chars[] = { '\0', 'a', 'Z', '\x01', '\x7f',
		(char)0x80, (char)0x81, (char)0xfe, (char)0xff, '%', '\n' };
	int ni = (int)(sizeof(ints) / sizeof(ints[0]));
	int nc = (int)(sizeof(chars) / sizeof(chars[0]));
	int i, j, k, v;

	for (k = 0; k < NLOGV_FORMS; k++) {
		for (v = 0; v < 2; v++) {
			for (i = 0; i < ni; i++) {
				for (j = 0; j < NSTRPOOL; j++) {
					logv_case(k, ints[i],
					    ints[(i + 1) % ni],
					    ints[(i + 2) % ni], strpool[j],
					    chars[(i + j) % nc], v != 0);
				}
			}
		}
	}

	for (long it = 0; it < 250000; it++) {
		int form = (int)rnd_below(NLOGV_FORMS);
		int a = rnd_int();
		int b = rnd_int();
		int c = rnd_int();
		const char *s = strpool[rnd_below(NSTRPOOL)];
		char ch = (char)(unsigned char)rnd_below(256);
		bool vb = rnd_below(4) != 0;

		logv_case(form, a, b, c, s, ch, vb);
	}
}

/* --------------------------------------------------------- parse_duration */

enum { PBUF = 192 };

static unsigned char pbuf_ref[PBUF];
static unsigned char pbuf_a[PBUF];
static unsigned char pbuf_b[PBUF];

static void
parse_case(const char *data, std::size_t len)
{
	char desc[3 * PBUF + 64];
	std::size_t i, o;

	if (len > PBUF - 8)
		len = PBUF - 8;
	std::memset(pbuf_ref, 0x7f, sizeof(pbuf_ref));
	std::memcpy(pbuf_ref, data, len);
	pbuf_ref[len] = '\0';
	std::memcpy(pbuf_a, pbuf_ref, sizeof(pbuf_a));
	std::memcpy(pbuf_b, pbuf_ref, sizeof(pbuf_b));

	RUN_SIDE(g_a, g_a.ret = ref_parse_duration((const char *)pbuf_a));
	RUN_SIDE(g_b, g_b.ret = port::parse_duration((const char *)pbuf_b));

	if (caps_match() &&
	    std::memcmp(pbuf_a, pbuf_ref, sizeof(pbuf_ref)) == 0 &&
	    std::memcmp(pbuf_b, pbuf_ref, sizeof(pbuf_ref)) == 0) {
		record(F_PARSE, true, "");
		return;
	}

	o = (std::size_t)std::snprintf(desc, sizeof(desc), "len=%zu input=", len);
	for (i = 0; i < len && o + 4 < sizeof(desc); i++)
		o += (std::size_t)std::snprintf(desc + o, sizeof(desc) - o,
		    "%02x", pbuf_ref[i]);
	record(F_PARSE, false, desc);
}

static void
parse_str(const char *s)
{
	parse_case(s, std::strlen(s));
}

static void
parse_num(double v, const char *sfx)
{
	char buf[64];

	std::snprintf(buf, sizeof(buf), "%.17g%s", v, sfx);
	parse_str(buf);
}

static std::size_t
gen_duration(char *out, std::size_t cap)
{
	static const char pool[] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'.', 'e', 'E', '+', '-', ' ', '\t', 's', 'm', 'h',
		'd', 'x', 'S', 'M', 'H', 'D', '\0', 'i', 'n', 'f',
		'a', 'X', (char)0x80, (char)0xff, (char)0x7f, '\x01',
		'0', '9', '.', 'm', 'p', '0',
	};
	static const double divs[4] = { 1.0, 60.0, 3600.0, 86400.0 };
	static const char *const sfxs[4] = { "", "m", "h", "d" };
	std::size_t len = 0;
	int mode = (int)rnd_below(4);
	int i, n;

	if (mode == 0) {
		n = (int)rnd_below(25);
		for (i = 0; i < n && len < cap - 1; i++)
			out[len++] = (char)(unsigned char)rnd_below(256);
		return (len);
	}
	if (mode == 1) {
		n = (int)rnd_below(21);
		for (i = 0; i < n && len < cap - 1; i++)
			out[len++] = pool[rnd_below(sizeof(pool))];
		return (len);
	}
	if (mode == 2) {
		if (rnd_below(4) == 0)
			out[len++] = rnd_below(2) ? ' ' : '\t';
		if (rnd_below(3) == 0)
			out[len++] = rnd_below(2) ? '+' : '-';
		n = 1 + (int)rnd_below(9);
		for (i = 0; i < n; i++)
			out[len++] = (char)('0' + rnd_below(10));
		if (rnd_below(2) == 0) {
			out[len++] = '.';
			n = (int)rnd_below(8);
			for (i = 0; i < n; i++)
				out[len++] = (char)('0' + rnd_below(10));
		}
		if (rnd_below(6) == 0) {
			out[len++] = rnd_below(2) ? 'e' : 'E';
			if (rnd_below(2))
				out[len++] = rnd_below(2) ? '+' : '-';
			out[len++] = (char)('0' + rnd_below(10));
			if (rnd_below(2))
				out[len++] = (char)('0' + rnd_below(10));
		}
		switch (rnd_below(8)) {
		case 0:
			break;
		case 1:
			out[len++] = 's';
			break;
		case 2:
			out[len++] = 'm';
			break;
		case 3:
			out[len++] = 'h';
			break;
		case 4:
			out[len++] = 'd';
			break;
		case 5:
			out[len++] = pool[rnd_below(sizeof(pool))];
			break;
		case 6:
			out[len++] = "smhd"[rnd_below(4)];
			out[len++] = pool[rnd_below(sizeof(pool))];
			break;
		default:
			out[len++] = "smhd"[rnd_below(4)];
			out[len++] = '\0';
			out[len++] = "smhd"[rnd_below(4)];
			break;
		}
		return (len);
	}

	{
		int di = (int)rnd_below(4);
		double base;
		int k;

		switch (rnd_below(7)) {
		case 0:
			base = 100000000.0 / divs[di];
			break;
		case 1:
			base = 0.0;
			break;
		case 2:
			base = 100000000.0;
			break;
		case 3:
			base = 5e-324;
			break;
		case 4:
			base = (double)rnd_below(200000000u) / divs[di];
			break;
		case 5:
			base = 1.0;
			break;
		default:
			base = (double)rnd_below(1000000u) +
			    (double)rnd_below(1000000u) / 1000000.0;
			break;
		}
		k = (int)rnd_below(9) - 4;
		for (; k > 0; k--)
			base = std::nextafter(base, HUGE_VAL);
		for (; k < 0; k++)
			base = std::nextafter(base, -HUGE_VAL);
		if (rnd_below(3) == 0)
			base = -base;
		len = (std::size_t)std::snprintf(out, cap, "%.17g%s", base,
		    sfxs[di]);
		if (len > cap - 1)
			len = cap - 1;
		return (len);
	}
}

static void
test_parse_duration(void)
{
	static const char *const fixed[] = {
		"", " ", "  ", "\t", "\n", "+", "-", ".", "e", "e5", "E",
		"abc", "s", "m", "h", "d", "x", "-.", "+.", ".e", "0x", "0X",
		"0", "-0", "+0", "0.0", "-0.0", "00", "1", "-1", "+1",
		"1.5", "-1.5", "0.000001", "-0.000001",
		"5s", "5m", "5h", "5d", "5x", "5S", "5M", "5H", "5D",
		"-0s", "-0m", "-0h", "-0d",
		"0s", "0m", "0h", "0d",
		"-1s", "-1m", "-1h", "-1d",
		"5ss", "5sm", "5mm", "5 ", " 5", "5\t", "5s ", "5 s",
		"99999999", "99999999s", "100000000", "100000000s",
		"100000001", "99999999.5", "100000000.0",
		"1666666", "1666666m", "1666667m", "1666666.6666666m",
		"27777", "27777h", "27778h", "27777.777777h",
		"1157", "1157d", "1158d", "1157.4074074d",
		"-100000000", "-100000001", "-1e-300", "1e-300",
		"1e8", "1e8s", "1e8m", "9.9999999e7", "1.00000001e8",
		"1e7", "1e-7", "1e300", "-1e300", "1e400", "-1e400",
		"inf", "-inf", "INF", "infinity", "nan", "-nan", "NAN",
		"nan(1)", "inf s", "infs", "infm", "nans", "nand",
		"0x10", "0X10", "0x10p2", "0x1p-1", "0xg", "0x10s", "0x10m",
		"  12.5m", "\t12.5h", "--5", "++5", "5e", "5e+", "5e-",
		"1.7976931348623157e308", "-1.7976931348623157e308",
		"4.9406564584124654e-324", "-4.9406564584124654e-324",
		"2.2250738585072014e-308",
		"1666666.6666666665m", "1666666.6666666667m",
		"27777.777777777774h", "27777.777777777777h",
		"1157.4074074074074d", "1157.4074074074076d",
		"000000000000000000000000005",
		"5.000000000000000000000000000000001",
		"9999999999999999999999999999999",
		"-9999999999999999999999999999999",
	};
	static const char nulcases[][12] = {
		{ '\0', 'm', 0 },
		{ '5', '\0', 'm', 0 },
		{ '5', 'm', '\0', 'm', 0 },
		{ '5', 'm', '\0', '\0', 0 },
		{ '\0', '\0', 0 },
		{ '5', '.', '\0', '5', 0 },
	};
	static const std::size_t nullens[] = { 2, 3, 4, 4, 2, 4 };
	static const unsigned char highs[][6] = {
		{ 0x80, 0x00, 0, 0, 0, 0 },
		{ 0xff, 0x00, 0, 0, 0, 0 },
		{ 0x35, 0x80, 0x00, 0, 0, 0 },
		{ 0x35, 0xff, 0xfe, 0x00, 0, 0 },
		{ 0x35, 0x6d, 0x80, 0x00, 0, 0 },
		{ 0xc2, 0xb5, 0x00, 0, 0, 0 },
		{ 0x35, 0x00, 0x80, 0x00, 0, 0 },
		{ 0x7f, 0x35, 0x00, 0, 0, 0 },
	};
	static const std::size_t highlens[] = { 1, 1, 2, 3, 3, 2, 3, 2 };
	static const char *const sfxall[] = { "", "s", "m", "h", "d", "x",
		"ss", "mm", " " };
	char buf[PBUF];
	std::size_t i;
	int j;

	for (i = 0; i < sizeof(fixed) / sizeof(fixed[0]); i++)
		parse_str(fixed[i]);
	for (i = 0; i < sizeof(nulcases) / sizeof(nulcases[0]); i++)
		parse_case(nulcases[i], nullens[i]);
	for (i = 0; i < sizeof(highs) / sizeof(highs[0]); i++)
		parse_case((const char *)highs[i], highlens[i]);

	/* Every byte value as a lone input and as a one-character suffix. */
	for (i = 0; i < 256; i++) {
		buf[0] = (char)(unsigned char)i;
		parse_case(buf, 1);
		buf[0] = '5';
		buf[1] = (char)(unsigned char)i;
		parse_case(buf, 2);
		buf[0] = '5';
		buf[1] = 'm';
		buf[2] = (char)(unsigned char)i;
		parse_case(buf, 3);
	}

	/* Boundary lengths: runs of digits from empty up past the buffer. */
	for (i = 0; i <= PBUF - 8; i++) {
		std::memset(buf, '9', i);
		parse_case(buf, i);
		std::memset(buf, '0', i);
		parse_case(buf, i);
	}

	/* Both sides of the range limits, for each multiplier. */
	{
		static const double divs[4] = { 1.0, 60.0, 3600.0, 86400.0 };
		static const char *const sfx4[4] = { "s", "m", "h", "d" };
		int di, k;

		for (di = 0; di < 4; di++) {
			double base = 100000000.0 / divs[di];
			double v = base;

			for (k = 0; k < 6; k++)
				v = std::nextafter(v, -HUGE_VAL);
			for (k = 0; k < 13; k++) {
				parse_num(v, sfx4[di]);
				parse_num(-v, sfx4[di]);
				v = std::nextafter(v, HUGE_VAL);
			}
		}
		parse_num(0.0, "");
		parse_num(-0.0, "");
		for (di = 0; di < 4; di++) {
			parse_num(0.0, sfx4[di]);
			parse_num(-0.0, sfx4[di]);
			parse_num(5e-324, sfx4[di]);
			parse_num(-5e-324, sfx4[di]);
			parse_num(std::nextafter(0.0, HUGE_VAL), sfx4[di]);
		}
	}

	/* Assorted magnitudes crossed with every suffix shape. */
	{
		static const double mags[] = { 0.0, 1e-300, 1e-9, 0.5, 1.0,
			59.0, 60.0, 61.0, 3599.0, 3600.0, 86399.0, 86400.0,
			1666666.0, 1666667.0, 27777.0, 27778.0, 1157.0,
			1158.0, 99999999.0, 100000000.0, 100000001.0, 1e9,
			1e17, 1e300 };
		std::size_t m;

		for (m = 0; m < sizeof(mags) / sizeof(mags[0]); m++) {
			for (j = 0; j < (int)(sizeof(sfxall) /
			    sizeof(sfxall[0])); j++) {
				parse_num(mags[m], sfxall[j]);
				parse_num(-mags[m], sfxall[j]);
			}
		}
	}

	for (long it = 0; it < 250000; it++) {
		std::size_t len = gen_duration(buf, sizeof(buf) - 8);

		parse_case(buf, len);
	}
}

/* ------------------------------------------------------------ sig_handler */

static void
sig_case(int signo, int ks)
{
	char desc[128];

	g_cur = &g_a;
	cap_reset(&g_a);
	killsig = ks;
	sig_chld = 0;
	sig_alrm = 0;
	sig_term = 0;
	sig_other = 0;
	ref_sig_handler(signo);
	g_a.returned = 1;
	g_a.sig_chld_after = (int)sig_chld;
	g_a.sig_alrm_after = (int)sig_alrm;
	g_a.sig_term_after = (int)sig_term;
	g_a.sig_other_after = (int)sig_other;
	g_a.killsig_after = killsig;

	g_cur = &g_b;
	cap_reset(&g_b);
	port::killsig = ks;
	port::sig_chld = 0;
	port::sig_alrm = 0;
	port::sig_term = 0;
	port::sig_other = 0;
	port::sig_handler(signo);
	g_b.returned = 1;
	g_b.sig_chld_after = (int)port::sig_chld;
	g_b.sig_alrm_after = (int)port::sig_alrm;
	g_b.sig_term_after = (int)port::sig_term;
	g_b.sig_other_after = (int)port::sig_other;
	g_b.killsig_after = port::killsig;

	if (caps_match())
		record(F_SIG, true, "");
	else {
		std::snprintf(desc, sizeof(desc), "signo=%d killsig=%d",
		    signo, ks);
		record(F_SIG, false, desc);
	}
}

static void
test_sig_handler(void)
{
	static const int interesting[] = { SIGHUP, SIGINT, SIGQUIT, SIGILL,
		SIGTRAP, SIGABRT, SIGEMT, SIGFPE, SIGBUS, SIGSEGV, SIGSYS,
		SIGPIPE, SIGTERM, SIGXCPU, SIGXFSZ, SIGVTALRM, SIGPROF,
		SIGUSR1, SIGUSR2, SIGCHLD, SIGALRM, SIGKILL, SIGSTOP,
		SIGCONT, SIGTSTP, SIGTTIN, SIGTTOU, SIGURG, SIGWINCH, 0, -1,
		-2, 33, 34, 63, 64, 65, 100, 2147483647, (-2147483647 - 1) };
	int ni = (int)(sizeof(interesting) / sizeof(interesting[0]));
	int i, j;

	/* Full cross product of the interesting signal numbers. */
	for (i = 0; i < ni; i++)
		for (j = 0; j < ni; j++)
			sig_case(interesting[i], interesting[j]);

	/* Every signal number in and just past the usual range. */
	for (i = -4; i <= 70; i++) {
		for (j = 0; j < ni; j++)
			sig_case(i, interesting[j]);
		sig_case(i, i);
		sig_case(i, i + 1);
		sig_case(i, i - 1);
	}

	for (long it = 0; it < 250000; it++) {
		int signo;
		int ks;

		switch (rnd_below(4)) {
		case 0:
			signo = (int)rnd_below(80) - 8;
			break;
		case 1:
			signo = interesting[rnd_below((std::uint32_t)ni)];
			break;
		case 2:
			signo = rnd_int();
			break;
		default:
			signo = (int)rnd_below(40);
			break;
		}
		switch (rnd_below(4)) {
		case 0:
			ks = signo;
			break;
		case 1:
			ks = interesting[rnd_below((std::uint32_t)ni)];
			break;
		case 2:
			ks = rnd_int();
			break;
		default:
			ks = (int)rnd_below(40);
			break;
		}
		sig_case(signo, ks);
	}
}

/* ----------------------------------------------------------- set_interval */

static void
setiv_case(double iv, int fail)
{
	char desc[128];
	unsigned long long bits;

	g_setitimer_fail = fail;
	RUN_SIDE(g_a, ref_set_interval(iv));
	RUN_SIDE(g_b, port::set_interval(iv));

	if (caps_match())
		record(F_SETIV, true, "");
	else {
		std::memcpy(&bits, &iv, sizeof(bits));
		std::snprintf(desc, sizeof(desc),
		    "iv=%.17g bits=%016llx setitimer_fail=%d", iv, bits, fail);
		record(F_SETIV, false, desc);
	}
}

static void
setiv_both(double iv)
{
	setiv_case(iv, 0);
	setiv_case(iv, 1);
}

/*
 * Positive values are kept under 2^61 so the (time_t) conversion stays in
 * range; out-of-range float-to-integer conversion is undefined and would make
 * the two compilers legitimately disagree.  Negative values and NaN never
 * reach the conversion.
 */
static double
gen_interval(void)
{
	double f = (double)(rnd() >> 11) / 9007199254740992.0;
	double v;
	int e;

	switch (rnd_below(8)) {
	case 0:
		return ((double)((int)rnd_below(2001) - 1000));
	case 1:
		return (rnd_below(2) ? f : -f);
	case 2:
		e = (int)rnd_below(121) - 60;
		v = std::ldexp(f, e);
		return (rnd_below(2) ? v : -v);
	case 3:
		v = (double)(rnd() % 1000000u) + f;
		return (rnd_below(2) ? v : -v);
	case 4:
		v = (double)(rnd() % 1000u);
		for (e = (int)rnd_below(5); e > 0; e--)
			v = std::nextafter(v, rnd_below(2) ? HUGE_VAL :
			    -HUGE_VAL);
		return (rnd_below(2) ? v : -v);
	case 5:
		return (rnd_below(2) ? 0.0 : -0.0);
	case 6:
		v = (double)(rnd() % 100u) +
		    (double)(rnd() % 1000001u) / 1000000.0;
		return (rnd_below(2) ? v : -v);
	default:
		v = (double)(rnd() % 4000000000000000000ull) / 4.0;
		return (rnd_below(2) ? v : -v);
	}
}

static void
test_set_interval(void)
{
	static const double fixed[] = {
		0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 1.5, -1.5, 2.5, -2.5,
		1e-9, -1e-9, 1e-7, -1e-7, 5e-7, 9.9e-7, 1e-6, 2e-6,
		0.999999, 0.9999999, 0.99999999, 0.999999999999,
		1.000001, 1.0000001, 1.9999999999,
		4.9406564584124654e-324, -4.9406564584124654e-324,
		2.2250738585072014e-308,
		59.999999, 60.0, 60.000001, 86399.999999, 86400.0,
		99999999.999999, 100000000.0,
		1e9, 1e12, 1e15, 1e17, 1.5e18, 2e18, -1e18,
		2147483647.0, 2147483648.0, 4294967296.0,
		-2147483648.0, -4294967296.0,
		0.000001, 0.0000005, 0.0000015, 0.5000005,
		1.0 - 1e-16, 1.0 + 1e-16,
	};
	std::size_t i;
	int k;

	for (i = 0; i < sizeof(fixed) / sizeof(fixed[0]); i++)
		setiv_both(fixed[i]);

	/* NaN and -inf never reach the (time_t) conversion, so they are safe. */
	setiv_both(std::nan(""));
	setiv_both(-std::nan(""));
	setiv_both(-HUGE_VAL);

	/* Both sides of every integer boundary from 0 to 32. */
	for (k = 0; k <= 32; k++) {
		double v = (double)k;

		setiv_both(std::nextafter(v, -HUGE_VAL));
		setiv_both(v);
		setiv_both(std::nextafter(v, HUGE_VAL));
		setiv_both(-std::nextafter(v, -HUGE_VAL));
		setiv_both(-v);
		setiv_both(-std::nextafter(v, HUGE_VAL));
	}

	/* Microsecond truncation boundaries inside the first second. */
	for (k = 0; k <= 1000; k++)
		setiv_both((double)k / 1000.0);

	for (long it = 0; it < 250000; it++) {
		double iv = gen_interval();
		int fail = (rnd_below(4) == 0);

		setiv_case(iv, fail);
	}
}

/* -------------------------------------------------------- log_termination */

static void
logterm_case(int code, int pid, int status, const char *name, bool vb)
{
	char desc[256];
	siginfo_t si_a, si_b;

	fill_strbufs(name);
	verbose = vb;
	port::verbose = vb;

	std::memset(&si_a, 0x7f, sizeof(si_a));
	si_a.si_code = code;
	si_a.si_pid = (pid_t)pid;
	si_a.si_status = status;
	std::memcpy(&si_b, &si_a, sizeof(si_b));

	RUN_SIDE(g_a, ref_log_termination((const char *)sbuf_a, &si_a));
	RUN_SIDE(g_b, port::log_termination((const char *)sbuf_b, &si_b));

	if (caps_match() && strbufs_intact() &&
	    std::memcmp(&si_a, &si_b, sizeof(si_a)) == 0)
		record(F_LOGTERM, true, "");
	else {
		std::snprintf(desc, sizeof(desc),
		    "code=%d pid=%d status=%d verbose=%d name=\"%s\"",
		    code, pid, status, (int)vb, name);
		record(F_LOGTERM, false, desc);
	}
}

static void
test_log_termination(void)
{
	static const int codes[] = { CLD_EXITED, CLD_KILLED, CLD_DUMPED,
		CLD_TRAPPED, CLD_STOPPED, CLD_CONTINUED, 0, -1, 7, 8, 100,
		2147483647, (-2147483647 - 1) };
	static const int nums[] = { 0, 1, -1, 2, 127, 128, 255, 256, -128,
		65536, 2147483647, (-2147483647 - 1), 12345 };
	int ncodes = (int)(sizeof(codes) / sizeof(codes[0]));
	int nnums = (int)(sizeof(nums) / sizeof(nums[0]));
	int c, i, j, v;

	for (c = 0; c < ncodes; c++)
		for (i = 0; i < nnums; i++)
			for (j = 0; j < NSTRPOOL; j++)
				for (v = 0; v < 2; v++)
					logterm_case(codes[c], nums[i],
					    nums[(i + 3) % nnums], strpool[j],
					    v != 0);

	/* Every si_code from -8 to 40, both verbosity settings. */
	for (c = -8; c <= 40; c++) {
		logterm_case(c, 4242, -7, "child terminated", true);
		logterm_case(c, 4242, -7, "child terminated", false);
		logterm_case(c, 0, 0, "collected zombie", true);
	}

	for (long it = 0; it < 250000; it++) {
		int code;

		switch (rnd_below(3)) {
		case 0:
			code = codes[rnd_below((std::uint32_t)ncodes)];
			break;
		case 1:
			code = (int)rnd_below(24) - 8;
			break;
		default:
			code = rnd_int();
			break;
		}
		logterm_case(code, rnd_int(), rnd_int(),
		    strpool[rnd_below(NSTRPOOL)], rnd_below(4) != 0);
	}
}

/* --------------------------------------------------------------- selftest */

static void
probe_errx(void)
{
	errx(42, "probe %d", 7);
}

static void
probe_err(void)
{
	errno = ERANGE;
	err(43, "probe %s", "two");
}

__attribute__((__format__(__printf__, 1, 2))) static void
probe_vwarnx(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vwarnx(fmt, ap);
	va_end(ap);
}

static void
probe_setitimer(void)
{
	struct itimerval tv;

	std::memset(&tv, 0, sizeof(tv));
	tv.it_value.tv_sec = 7;
	if (setitimer(ITIMER_REAL, &tv, NULL) < 0)
		g_cur->err_code = -99;
}

/*
 * If the libc interposition ever stopped working the whole diff would compare
 * two empty records and pass vacuously, so prove it is live before testing.
 */
static bool
self_test(void)
{
	bool ok = true;

	RUN_SIDE(g_a, probe_errx());
	if (g_a.err_kind != 1 || g_a.err_code != 42 || g_a.returned != 0 ||
	    std::strcmp(g_a.err_msg, "probe 7") != 0) {
		std::printf("SELFTEST: errx(3) is not interposed\n");
		ok = false;
	}

	RUN_SIDE(g_a, probe_err());
	if (g_a.err_kind != 2 || g_a.err_code != 43 || g_a.returned != 0 ||
	    g_a.err_errno != ERANGE ||
	    std::strcmp(g_a.err_msg, "probe two") != 0) {
		std::printf("SELFTEST: err(3) is not interposed\n");
		ok = false;
	}

	RUN_SIDE(g_a, probe_vwarnx("probe %d %s", 5, "x"));
	if (g_a.log_calls != 1 || g_a.returned != 1 ||
	    std::strcmp(g_a.log_buf, "probe 5 x") != 0) {
		std::printf("SELFTEST: vwarnx(3) is not interposed\n");
		ok = false;
	}

	g_setitimer_fail = 0;
	RUN_SIDE(g_a, probe_setitimer());
	if (g_a.itimer_calls != 1 || g_a.itimer_which != ITIMER_REAL ||
	    g_a.itimer_old_null != 1 || g_a.err_code == -99) {
		std::printf("SELFTEST: setitimer(2) is not interposed\n");
		ok = false;
	}

	g_setitimer_fail = 1;
	RUN_SIDE(g_a, probe_setitimer());
	if (g_a.err_code != -99) {
		std::printf("SELFTEST: setitimer(2) failure injection is "
		    "dead\n");
		ok = false;
	}
	g_setitimer_fail = 0;

	/* A deliberate mismatch must be visible to the record comparison. */
	cap_reset(&g_a);
	cap_reset(&g_b);
	if (!caps_match()) {
		std::printf("SELFTEST: fresh records already differ\n");
		ok = false;
	}
	g_b.log_buf[LOGCAP - 1] = 0x00;
	if (caps_match()) {
		std::printf("SELFTEST: guard tail is not compared\n");
		ok = false;
	}

	return (ok);
}

/* ------------------------------------------------------------------- main */

int
main(void)
{
	long total_cases = 0;
	long total_fail = 0;
	int f;

	rng_state = 0x5eed1234abcd0197ULL;

	std::printf("pbsd b0197 timeout(1) differential harness\n");
	if (!self_test()) {
		std::printf("SELFTEST FAILED -- results would be meaningless\n");
		return (1);
	}

	test_logv();
	test_parse_duration();
	test_sig_handler();
	test_set_interval();
	test_log_termination();

	std::printf("\n%-24s %12s %12s\n", "function", "cases", "failures");
	std::printf("------------------------------------------------\n");
	for (f = 0; f < F_COUNT; f++) {
		std::printf("%-24s %12ld %12ld\n", fname[f], cases_run[f],
		    fail_cnt[f]);
		total_cases += cases_run[f];
		total_fail += fail_cnt[f];
	}
	std::printf("------------------------------------------------\n");
	std::printf("%-24s %12ld %12ld\n", "TOTAL", total_cases, total_fail);

	return (total_fail == 0 ? 0 : 1);
}
