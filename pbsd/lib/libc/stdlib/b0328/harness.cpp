/*
 * harness.cpp -- differential test for PBSD batch b0328.
 */

#include <cerrno>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <unistd.h>

import pbsd.lib.libc.stdlib.b0328;

namespace P = pbsd::lib_libc_stdlib::b0328;

extern "C" {
unsigned long long ref_strtoull_l(const char *, char **, int, P::locale_t);
unsigned long long ref_strtoull(const char *, char **, int);
int ref_getopt(int, char *const[], const char *);
void ref_getopt_reset_state(void);
extern int ref_opterr, ref_optind, ref_optopt, ref_optreset;
extern char *ref_optarg;

typedef struct {
	char *key;
	void *data;
} ref_ENTRY;

typedef enum { ref_FIND, ref_ENTER } ref_ACTION;

struct ref___hsearch {
	size_t offset_basis;
	size_t index_mask;
	size_t entries_used;
	ref_ENTRY *entries;
};

struct ref_hsearch_data {
	ref___hsearch *__hsearch;
};

int ref_hsearch_r(ref_ENTRY, ref_ACTION, ref_ENTRY **,
    struct ref_hsearch_data *);
}

namespace {

struct Stat {
	const char *name;
	unsigned long cases;
	unsigned long fails;
	unsigned reported;
};

static Stat st_strtoull_l = { "strtoull_l", 0, 0, 0 };
static Stat st_strtoull = { "strtoull", 0, 0, 0 };
static Stat st_getopt = { "getopt", 0, 0, 0 };
static Stat st_hsearch_r = { "hsearch_r", 0, 0, 0 };

static const unsigned char GUARD = 0x7f;
static const unsigned SWEEP_ITERS = 50000u;

static void
check(Stat &s, bool ok, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

static void
check(Stat &s, bool ok, const char *fmt, ...)
{
	va_list ap;

	s.cases++;
	if (ok)
		return;
	s.fails++;
	if (s.reported < 15) {
		s.reported++;
		std::fprintf(stderr, "FAIL [%s] ", s.name);
		va_start(ap, fmt);
		std::vfprintf(stderr, fmt, ap);
		va_end(ap);
		std::fputc('\n', stderr);
	} else if (s.reported == 15) {
		s.reported++;
		std::fprintf(stderr, "FAIL [%s] (further failures suppressed)\n",
		    s.name);
	}
}

static std::uint64_t rng_state = 0xc0ffeebaddecaf42ull;

static std::uint64_t
rng_next(void)
{
	std::uint64_t x = rng_state;

	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rng_state = x;
	return (x * 0x2545f4914f6cdd1dull);
}

static unsigned
rng_byte(void)
{
	return ((unsigned)(rng_next() & 0xffu));
}

enum { STRBUF = 128, STRPAD = 16 };

struct StrBuf {
	unsigned char pre[STRPAD];
	char body[STRBUF];
	unsigned char post[STRPAD];
};

static void
strbuf_init(StrBuf &b, const char *s)
{
	std::memset(&b, GUARD, sizeof(b));
	if (s != nullptr)
		std::strncpy(b.body, s, STRBUF - 1);
}

static long
str_off(const StrBuf &b, const char *p)
{
	if (p == nullptr)
		return (-1);
	const unsigned char *base =
	    reinterpret_cast<const unsigned char *>(b.body);
	const unsigned char *q = reinterpret_cast<const unsigned char *>(p);
	if (q < base || q >= base + STRBUF)
		return (-2);
	return (q - base);
}

static bool
strbuf_equal(const StrBuf &a, const StrBuf &b)
{
	return (std::memcmp(&a, &b, sizeof(a)) == 0);
}

using str_fn = unsigned long long (*)(const char *, char **, int);
using str_l_fn = unsigned long long (*)(const char *, char **, int,
    P::locale_t);

static void
case_str_l(const char *label, str_l_fn port_fn, str_l_fn ref_fn, Stat &st,
    const char *input, int base, bool use_endptr, P::locale_t loc)
{
	StrBuf pa, pb;
	char *endp_a = nullptr;
	char *endp_b = nullptr;
	int err_a, err_b;
	unsigned long long ra, rb;

	strbuf_init(pa, input);
	strbuf_init(pb, input);

	errno = 0;
	ra = port_fn(pa.body, use_endptr ? &endp_a : nullptr, base, loc);
	err_a = errno;

	errno = 0;
	rb = ref_fn(pb.body, use_endptr ? &endp_b : nullptr, base, loc);
	err_b = errno;

	check(st, ra == rb, "%s rv %llu != %llu", label,
	    (unsigned long long)ra, (unsigned long long)rb);
	check(st, err_a == err_b, "%s errno %d != %d", label, err_a, err_b);
	if (use_endptr) {
		check(st, str_off(pa, endp_a) == str_off(pb, endp_b),
		    "%s endptr off %ld != %ld", label, str_off(pa, endp_a),
		    str_off(pb, endp_b));
	}
	check(st, strbuf_equal(pa, pb), "%s buffer mutated", label);
}

static void
case_str(const char *label, str_fn port_fn, str_fn ref_fn, Stat &st,
    const char *input, int base, bool use_endptr)
{
	StrBuf pa, pb;
	char *endp_a = nullptr;
	char *endp_b = nullptr;
	int err_a, err_b;
	unsigned long long ra, rb;

	strbuf_init(pa, input);
	strbuf_init(pb, input);

	errno = 0;
	ra = port_fn(pa.body, use_endptr ? &endp_a : nullptr, base);
	err_a = errno;

	errno = 0;
	rb = ref_fn(pb.body, use_endptr ? &endp_b : nullptr, base);
	err_b = errno;

	check(st, ra == rb, "%s rv %llu != %llu", label,
	    (unsigned long long)ra, (unsigned long long)rb);
	check(st, err_a == err_b, "%s errno %d != %d", label, err_a, err_b);
	if (use_endptr) {
		check(st, str_off(pa, endp_a) == str_off(pb, endp_b),
		    "%s endptr off %ld != %ld", label, str_off(pa, endp_a),
		    str_off(pb, endp_b));
	}
	check(st, strbuf_equal(pa, pb), "%s buffer mutated", label);
}

static void
test_strtoull_l_edges(Stat &st)
{
	static const P::locale_t locales[] = {
		nullptr,
		reinterpret_cast<P::locale_t>(-1),
		&P::__xlocale_C_locale,
		&P::__xlocale_global_locale,
	};
	static const char *inputs[] = {
		"",
		" ",
		"\t\n\r\f\v",
		"0",
		"-0",
		"+0",
		"1",
		"-1",
		"+1",
		"0x",
		"0x0",
		"0XfF",
		"0b",
		"0b1",
		"0B10",
		"07",
		"08",
		"010",
		"9223372036854775807",
		"18446744073709551615",
		"18446744073709551616",
		"-9223372036854775808",
		"-18446744073709551615",
		"-18446744073709551616",
		"z",
		"123z",
		"0xz",
		"0b2",
		"\x80",
		"\xff",
		" \x80\x7f",
		"  -0x1aZ",
		"++1",
		"--1",
		"0xG",
		"0b0",
		"36z",
		"zzzz",
		" \t+0xdeadbeef",
		"0xdeadbeefdeadbeef",
		"184467440737095516150",
		nullptr
	};
	static const int bases[] = { 0, 2, 8, 10, 16, 36, 1, 37, -1, 99 };

	for (const char **p = inputs; *p != nullptr; ++p) {
		for (int base : bases) {
			for (P::locale_t loc : locales) {
				case_str_l(*p, P::strtoull_l, ref_strtoull_l, st,
				    *p, base, false, loc);
				case_str_l(*p, P::strtoull_l, ref_strtoull_l, st,
				    *p, base, true, loc);
			}
		}
	}
}

static void
test_strtoull_edges(str_fn port_fn, str_fn ref_fn, Stat &st)
{
	static const char *inputs[] = {
		"",
		"0",
		"-1",
		"18446744073709551615",
		nullptr
	};
	static const int bases[] = { 0, 10, 16 };

	for (const char **p = inputs; *p != nullptr; ++p) {
		for (int base : bases) {
			case_str(*p, port_fn, ref_fn, st, *p, base, false);
			case_str(*p, port_fn, ref_fn, st, *p, base, true);
		}
	}
}

static void
test_strtoull_l_sweep(Stat &st)
{
	for (unsigned i = 0; i < SWEEP_ITERS; ++i) {
		StrBuf pa, pb;
		char *endp_a = nullptr;
		char *endp_b = nullptr;
		unsigned len = 1u + (unsigned)(rng_next() % (STRBUF - 2));
		int base = (int)(rng_next() % 40u) - 2;
		bool use_endptr = (rng_next() & 1u) != 0;
		P::locale_t loc = (rng_next() & 1u) ?
		    &P::__xlocale_C_locale : &P::__xlocale_global_locale;
		int err_a, err_b;
		unsigned long long ra, rb;

		std::memset(&pa, GUARD, sizeof(pa));
		std::memset(&pb, GUARD, sizeof(pb));
		for (unsigned j = 0; j < len; ++j) {
			unsigned char c = (unsigned char)rng_byte();
			if (c == '\0')
				c = (unsigned char)('0' + (j % 10));
			pa.body[j] = (char)c;
			pb.body[j] = (char)c;
		}
		pa.body[len] = '\0';
		pb.body[len] = '\0';

		errno = 0;
		ra = P::strtoull_l(pa.body, use_endptr ? &endp_a : nullptr, base,
		    loc);
		err_a = errno;

		errno = 0;
		rb = ref_strtoull_l(pb.body, use_endptr ? &endp_b : nullptr, base,
		    loc);
		err_b = errno;

		check(st, ra == rb, "sweep[%u] rv", i);
		check(st, err_a == err_b, "sweep[%u] errno", i);
		if (use_endptr)
			check(st, str_off(pa, endp_a) == str_off(pb, endp_b),
			    "sweep[%u] endptr", i);
		check(st, strbuf_equal(pa, pb), "sweep[%u] buffer", i);
	}
}

static void
test_strtoull_sweep(str_fn port_fn, str_fn ref_fn, Stat &st)
{
	for (unsigned i = 0; i < SWEEP_ITERS; ++i) {
		StrBuf pa, pb;
		char *endp_a = nullptr;
		char *endp_b = nullptr;
		unsigned len = 1u + (unsigned)(rng_next() % (STRBUF - 2));
		int base = (int)(rng_next() % 40u) - 2;
		bool use_endptr = (rng_next() & 1u) != 0;
		int err_a, err_b;
		unsigned long long ra, rb;

		std::memset(&pa, GUARD, sizeof(pa));
		std::memset(&pb, GUARD, sizeof(pb));
		for (unsigned j = 0; j < len; ++j) {
			unsigned char c = (unsigned char)rng_byte();
			if (c == '\0')
				c = (unsigned char)('0' + (j % 10));
			pa.body[j] = (char)c;
			pb.body[j] = (char)c;
		}
		pa.body[len] = '\0';
		pb.body[len] = '\0';

		errno = 0;
		ra = port_fn(pa.body, use_endptr ? &endp_a : nullptr, base);
		err_a = errno;

		errno = 0;
		rb = ref_fn(pb.body, use_endptr ? &endp_b : nullptr, base);
		err_b = errno;

		check(st, ra == rb, "sweep[%u] rv", i);
		check(st, err_a == err_b, "sweep[%u] errno", i);
		if (use_endptr)
			check(st, str_off(pa, endp_a) == str_off(pb, endp_b),
			    "sweep[%u] endptr", i);
		check(st, strbuf_equal(pa, pb), "sweep[%u] buffer", i);
	}
}

struct GoCtx {
	char storage[512];
	char *argv[32];
	int argc;
	const char *optstring;
	int opterr;
};

static void
go_build(GoCtx &g, const char *optstring, int opterr,
    std::initializer_list<const char *> args)
{
	size_t off = 0;

	g.optstring = optstring;
	g.opterr = opterr;
	g.argc = 0;
	for (const char *a : args) {
		size_t n = std::strlen(a) + 1;

		if (off + n > sizeof(g.storage) || g.argc >= 31)
			std::abort();
		std::memcpy(g.storage + off, a, n);
		g.argv[g.argc++] = g.storage + off;
		off += n;
	}
	if (off + 1 > sizeof(g.storage))
		std::abort();
	g.storage[off] = '\0';
	g.argv[g.argc] = g.storage + off;
}

static void
go_reset_port(void)
{
	P::optreset = 1;
	P::optind = 1;
	P::optopt = 0;
	P::optarg = nullptr;
}

static void
go_reset_ref(void)
{
	ref_getopt_reset_state();
	ref_optind = 1;
	ref_optopt = 0;
	ref_optarg = nullptr;
}

static int
capture_stderr(int (*fn)(void *), void *ctx, char *buf, size_t bufsz)
{
	int pipefd[2];
	int saved;
	ssize_t n;
	char tmp[4096];

	if (pipe(pipefd) != 0)
		return (-1);
	saved = dup(STDERR_FILENO);
	dup2(pipefd[1], STDERR_FILENO);
	(void)fn(ctx);
	fflush(stderr);
	dup2(saved, STDERR_FILENO);
	close(saved);
	close(pipefd[1]);
	n = read(pipefd[0], tmp, sizeof(tmp) - 1);
	close(pipefd[0]);
	if (n < 0)
		return (-1);
	tmp[n] = '\0';
	std::strncpy(buf, tmp, bufsz - 1);
	buf[bufsz - 1] = '\0';
	return (0);
}

struct GoRun {
	GoCtx *g;
	int rv;
};

static int
run_port(void *vp)
{
	GoRun *r = static_cast<GoRun *>(vp);

	r->rv = P::getopt(r->g->argc, r->g->argv, r->g->optstring);
	return (0);
}

static int
run_ref(void *vp)
{
	GoRun *r = static_cast<GoRun *>(vp);

	r->rv = ref_getopt(r->g->argc, r->g->argv, r->g->optstring);
	return (0);
}

static bool
streq_ptr(const char *a, const char *b)
{
	if (a == nullptr && b == nullptr)
		return (true);
	if (a == nullptr || b == nullptr)
		return (false);
	return (std::strcmp(a, b) == 0);
}

static void
case_getopt_once(GoCtx &g, bool capture_err)
{
	char err_a[512], err_b[512];
	GoRun ra { &g, 0 }, rb { &g, 0 };

	if (capture_err && g.opterr) {
		(void)capture_stderr(run_port, &ra, err_a, sizeof(err_a));
		(void)capture_stderr(run_ref, &rb, err_b, sizeof(err_b));
	} else {
		ra.rv = P::getopt(g.argc, g.argv, g.optstring);
		rb.rv = ref_getopt(g.argc, g.argv, g.optstring);
		err_a[0] = err_b[0] = '\0';
	}

	check(st_getopt, ra.rv == rb.rv, "rv %d != %d opt=%s", ra.rv, rb.rv,
	    g.optstring);
	check(st_getopt, P::optind == ref_optind, "optind %d != %d", P::optind,
	    ref_optind);
	check(st_getopt, P::optopt == ref_optopt, "optopt %c != %c", P::optopt,
	    ref_optopt);
	check(st_getopt, streq_ptr(P::optarg, ref_optarg), "optarg mismatch");
	if (capture_err && g.opterr)
		check(st_getopt, std::strcmp(err_a, err_b) == 0,
		    "stderr '%s' != '%s'", err_a, err_b);
}

static void
case_getopt_seq(GoCtx &g, int steps, bool capture_err)
{
	go_reset_port();
	go_reset_ref();
	P::opterr = g.opterr;
	ref_opterr = g.opterr;

	for (int i = 0; i < steps; ++i)
		case_getopt_once(g, capture_err);
}

static void
test_getopt_edges(void)
{
	GoCtx g;

	go_build(g, "abc:", 0, { "prog", "file" });
	case_getopt_seq(g, 1, false);

	go_build(g, "abc:", 0, { "prog", "-a", "file" });
	case_getopt_seq(g, 1, false);

	go_build(g, "abc:", 0, { "prog", "-", "file" });
	case_getopt_seq(g, 1, false);

	go_build(g, "-abc:", 0, { "prog", "-", "file" });
	case_getopt_seq(g, 1, false);

	go_build(g, "abc:", 0, { "prog", "--", "file" });
	case_getopt_seq(g, 1, false);

	go_build(g, "abc:", 0, { "prog", "-abc", "tail" });
	case_getopt_seq(g, 4, false);

	go_build(g, "a:b:", 0, { "prog", "-a", "-b", "x" });
	case_getopt_seq(g, 3, false);

	go_build(g, "a::b:", 0, { "prog", "-a", "-b", "x" });
	case_getopt_seq(g, 3, false);

	go_build(g, "a::b:", 0, { "prog", "-a", "-b" });
	case_getopt_seq(g, 2, false);

	go_build(g, "a:b:", 0, { "prog", "-a" });
	case_getopt_seq(g, 2, false);

	go_build(g, ":a:b:", 0, { "prog", "-z" });
	case_getopt_seq(g, 1, true);

	go_build(g, "a:b:", 1, { "prog", "-z" });
	case_getopt_seq(g, 1, true);

	go_build(g, "a:b:", 1, { "prog", "-a" });
	case_getopt_seq(g, 1, true);

	go_build(g, "a:b:", 0, { "prog", "-a", "val", "rest" });
	case_getopt_seq(g, 2, false);

	go_build(g, "a:b:", 0, { "prog", "-aval", "rest" });
	case_getopt_seq(g, 2, false);

	go_build(g, "ab:", 0, { "prog", "-ba", "v" });
	case_getopt_seq(g, 3, false);

	go_build(g, "abc:", 0, { "prog" });
	case_getopt_seq(g, 1, false);

	go_build(g, "abc:", 0, { "prog", "" });
	case_getopt_seq(g, 1, false);

	go_build(g, ":", 0, { "prog", "-a" });
	case_getopt_seq(g, 1, false);

	go_build(g, "a:", 1, { "prog", "-a" });
	case_getopt_seq(g, 1, true);

	go_build(g, "a:", 0, { "prog", "-:", "x" });
	case_getopt_seq(g, 1, false);

	go_build(g, "a", 0, { "prog", "-a", "-a" });
	case_getopt_seq(g, 3, false);

	go_build(g, "a", 0, { "prog", "-a" });
	case_getopt_seq(g, 1, false);
	go_reset_port();
	go_reset_ref();
	case_getopt_seq(g, 1, false);
}

static void
test_getopt_sweep(void)
{
	static const char *letters =
	    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char optbuf[80];

	for (unsigned i = 0; i < SWEEP_ITERS; ++i) {
		GoCtx g;
		int argc = 2 + (int)(rng_next() % 7u);
		const char *arglist[32];
		char argstore[8][64];
		unsigned nopt = 1u + (unsigned)(rng_next() % 20u);
		unsigned opterr = 0;
		int steps;

		arglist[0] = "prog";
		for (int a = 1; a < argc && a < 8; ++a) {
			unsigned kind = (unsigned)(rng_next() % 6u);
			char *dst = argstore[a - 1];

			if (kind == 0) {
				std::snprintf(dst, sizeof(argstore[0]), "-%c",
				    letters[rng_next() % 52u]);
			} else if (kind == 1) {
				std::snprintf(dst, sizeof(argstore[0]),
				    "-%c%c", letters[rng_next() % 52u],
				    letters[rng_next() % 52u]);
			} else if (kind == 2) {
				std::strcpy(dst, "--");
			} else if (kind == 3) {
				std::strcpy(dst, "-");
			} else {
				unsigned len = 1u + (unsigned)(rng_next() % 12u);
				for (unsigned j = 0; j < len; ++j)
					dst[j] = (char)('0' + (rng_byte() % 10));
				dst[len] = '\0';
			}
			arglist[a] = dst;
		}

		optbuf[0] = '\0';
		for (unsigned j = 0; j < nopt && std::strlen(optbuf) + 4 < sizeof(optbuf);
		    ++j) {
			char spec[4];
			unsigned c = (unsigned)(rng_next() % 52u);
			unsigned mode = (unsigned)(rng_next() % 4u);

			spec[0] = letters[c];
			spec[1] = '\0';
			if (mode >= 1)
				spec[1] = ':', spec[2] = '\0';
			if (mode >= 2)
				spec[2] = ':', spec[3] = '\0';
			std::strcat(optbuf, spec);
		}
		if (optbuf[0] == '\0')
			std::strcpy(optbuf, "a");

		g.optstring = optbuf;
		g.opterr = (int)opterr;
		g.argc = argc;
		for (int a = 0; a < argc; ++a)
			g.argv[a] = const_cast<char *>(arglist[a]);
		g.argv[argc] = const_cast<char *>("");

		steps = 1 + (int)(rng_next() % 6u);
		case_getopt_seq(g, steps, false);
	}
}

enum { MAX_KEYS = 64, KEYLEN = 32 };

struct HTab {
	ref___hsearch ref_hs;
	P::__hsearch port_hs;
	ref_hsearch_data ref_htab;
	P::hsearch_data port_htab;
	char keys[MAX_KEYS][KEYLEN];
	void *datas[MAX_KEYS];
};

static void
htab_free(HTab &t)
{
	std::free(t.ref_hs.entries);
	std::free(t.port_hs.entries);
	t.ref_hs.entries = nullptr;
	t.port_hs.entries = nullptr;
}

static void
htab_init(HTab &t, size_t offset_basis)
{
	std::memset(&t, 0, sizeof(t));
	t.ref_hs.entries = static_cast<ref_ENTRY *>(
	    std::calloc(16, sizeof(ref_ENTRY)));
	t.port_hs.entries = static_cast<P::ENTRY *>(
	    std::calloc(16, sizeof(P::ENTRY)));
	t.ref_hs.offset_basis = offset_basis;
	t.port_hs.offset_basis = offset_basis;
	t.ref_hs.index_mask = 0xf;
	t.port_hs.index_mask = 0xf;
	t.ref_htab.__hsearch = &t.ref_hs;
	t.port_htab.__hsearch = &t.port_hs;
}

static long
hent_off(const ref_ENTRY *base, size_t count, const void *p)
{
	if (p == nullptr)
		return (-1);
	const unsigned char *q = reinterpret_cast<const unsigned char *>(p);
	const unsigned char *b = reinterpret_cast<const unsigned char *>(base);
	const unsigned char *end = b + count * sizeof(ref_ENTRY);
	if (q < b || q >= end)
		return (-2);
	return (q - b);
}

static bool
htab_equal(const HTab &t)
{
	if (t.ref_hs.offset_basis != t.port_hs.offset_basis)
		return (false);
	if (t.ref_hs.index_mask != t.port_hs.index_mask)
		return (false);
	if (t.ref_hs.entries_used != t.port_hs.entries_used)
		return (false);
	size_t n = t.ref_hs.index_mask + 1;
	for (size_t i = 0; i < n; ++i) {
		const ref_ENTRY *re = &t.ref_hs.entries[i];
		const P::ENTRY *pe = &t.port_hs.entries[i];
		if ((re->key == nullptr) != (pe->key == nullptr))
			return (false);
		if (re->key != nullptr &&
		    std::strcmp(re->key, pe->key) != 0)
			return (false);
		if (re->data != pe->data)
			return (false);
	}
	return (true);
}

static void
case_hsearch(HTab &t, const char *key, void *data, bool enter, unsigned idx)
{
	ref_ENTRY rit;
	P::ENTRY pit;
	ref_ENTRY *rret = nullptr;
	P::ENTRY *pret = nullptr;
	int rr, pr;
	int err_r, err_p;

	std::strncpy(t.keys[idx], key, KEYLEN - 1);
	t.keys[idx][KEYLEN - 1] = '\0';
	t.datas[idx] = data;

	rit.key = t.keys[idx];
	rit.data = data;
	pit.key = t.keys[idx];
	pit.data = data;

	errno = 0;
	pr = P::hsearch_r(pit, enter ? P::ENTER : P::FIND, &pret, &t.port_htab);
	err_p = errno;

	errno = 0;
	rr = ref_hsearch_r(rit, enter ? ref_ENTER : ref_FIND, &rret,
	    &t.ref_htab);
	err_r = errno;

	check(st_hsearch_r, pr == rr, "key=%s enter=%d rv %d != %d", key, enter,
	    pr, rr);
	check(st_hsearch_r, err_p == err_r, "key=%s errno %d != %d", key, err_p,
	    err_r);
	check(st_hsearch_r,
	    hent_off(t.ref_hs.entries, t.ref_hs.index_mask + 1, rret) ==
	    hent_off(reinterpret_cast<const ref_ENTRY *>(t.port_hs.entries),
	    t.port_hs.index_mask + 1, pret),
	    "key=%s retval off", key);
	check(st_hsearch_r, htab_equal(t), "key=%s table state", key);
}

static void
test_hsearch_edges(void)
{
	HTab t;

	htab_init(t, 0x811c9dc5u);
	case_hsearch(t, "alpha", (void *)0x1, true, 0);
	case_hsearch(t, "alpha", (void *)0x2, true, 0);
	case_hsearch(t, "alpha", nullptr, false, 0);
	case_hsearch(t, "beta", nullptr, false, 0);
	case_hsearch(t, "beta", (void *)0x3, true, 1);

	htab_free(t);

	htab_init(t, 0x01000193u);
	for (int i = 0; i < 20; ++i) {
		char key[16];

		std::snprintf(key, sizeof(key), "k%02d", i);
		case_hsearch(t, key, (void *)(intptr_t)(i + 1), true,
		    (unsigned)i);
	}
	htab_free(t);

	htab_init(t, 0xdeadbeefu);
	case_hsearch(t, "collision", (void *)0x10, true, 0);
	case_hsearch(t, "collision2", (void *)0x11, true, 1);
	case_hsearch(t, "collision", (void *)0x12, false, 0);
	htab_free(t);
}

static void
test_hsearch_sweep(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; ++i) {
		HTab stack_tab;
		HTab &t = stack_tab;
		unsigned nk = 1u + (unsigned)(rng_next() % 24u);
		unsigned ki = 0;

		htab_init(t, (size_t)rng_next());
		for (unsigned j = 0; j < nk; ++j) {
			char key[KEYLEN];
			unsigned len = 1u + (unsigned)(rng_next() % (KEYLEN - 2));
			bool enter = (rng_next() & 3u) != 0;

			for (unsigned c = 0; c < len; ++c)
				key[c] = (char)('a' + (rng_byte() % 26));
			key[len] = '\0';
		case_hsearch(t, key, (void *)(intptr_t)rng_next(),
		    enter, ki % MAX_KEYS);
			ki++;
		}
		htab_free(t);
	}
}

static void
print_stat(const Stat &s)
{
	std::printf("%-14s %8lu %8lu\n", s.name, s.cases, s.fails);
}

} // namespace

int
main(void)
{
	test_strtoull_l_edges(st_strtoull_l);
	test_strtoull_l_sweep(st_strtoull_l);

	test_strtoull_edges(P::strtoull, ref_strtoull, st_strtoull);
	test_strtoull_sweep(P::strtoull, ref_strtoull, st_strtoull);

	test_getopt_edges();
	test_getopt_sweep();

	test_hsearch_edges();
	test_hsearch_sweep();

	std::printf("\n%-14s %8s %8s\n", "function", "cases", "failures");
	print_stat(st_strtoull_l);
	print_stat(st_strtoull);
	print_stat(st_getopt);
	print_stat(st_hsearch_r);

	unsigned long total_fails = st_strtoull_l.fails + st_strtoull.fails +
	    st_getopt.fails + st_hsearch_r.fails;

	return (total_fails == 0 ? 0 : 1);
}
