/*
 * Differential harness for batch b0233 (ldexp, getprogname, setprogname,
 * freadlink).  Every case drives both the C++23 port and the C oracle;
 * return values, pointer offsets, and full guard buffers are compared.
 */

#include <cerrno>
#include <climits>
#include <cmath>
#include <limits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.gen.b0233;

namespace P = pbsd::lib_libc_gen::b0233;

extern "C" {
double ref_ldexp(double x, int n);
const char *ref__getprogname(void);
void ref_setprogname(const char *progname);
ssize_t ref_freadlink(int fd, char *buf, size_t bufsize);
extern const char *__progname;
}

/* ------------------------------------------------------------------ */
/* Statistics                                                         */
/* ------------------------------------------------------------------ */

enum { F_LDEXP, F_GETPROGNAME, F_SETPROGNAME, F_FREADLINK, NFUNC };

static const char *const fname[NFUNC] = {
	"ldexp", "_getprogname", "setprogname", "freadlink"
};

static unsigned long long ncases[NFUNC];
static unsigned long long nfails[NFUNC];

static constexpr int MAXREPORT = 12;
static int nreported;

static void
report(int f, const char *ctx, const char *detail)
{
	nfails[f]++;
	if (nreported < MAXREPORT) {
		nreported++;
		std::printf("FAIL %-12s %s: %s\n", fname[f], ctx, detail);
	} else if (nreported == MAXREPORT) {
		nreported++;
		std::printf("... further failures suppressed\n");
	}
}

/* ------------------------------------------------------------------ */
/* PRNG (fixed seed)                                                  */
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state = 0xc0ffeebaddecade1ULL;

static std::uint64_t
nextrand(void)
{
	std::uint64_t x = rng_state;

	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	rng_state = x;
	return x * 0x2545f4914f6cdd1dULL;
}

static int
randint(void)
{
	return (int)(nextrand() & 0xffffffffu);
}

static double
randdouble_bits(void)
{
	union {
		double f;
		std::uint64_t u;
	} v;

	v.u = nextrand();
	return v.f;
}

/* ------------------------------------------------------------------ */
/* ldexp helpers                                                      */
/* ------------------------------------------------------------------ */

static std::uint64_t
dblbits(double x)
{
	union {
		double f;
		std::uint64_t u;
	} v;

	v.f = x;
	return v.u;
}

static bool
ldexp_ok(double x, int n, const char *ctx)
{
	double a = P::ldexp(x, n);
	double b = ref_ldexp(x, n);

	if (dblbits(a) != dblbits(b)) {
		char msg[160];

		std::snprintf(msg, sizeof msg,
		    "x=%a n=%d port=%a (%016llx) ref=%a (%016llx)",
		    x, n, a, (unsigned long long)dblbits(a), b,
		    (unsigned long long)dblbits(b));
		report(F_LDEXP, ctx, msg);
		return false;
	}
	return true;
}

/* ------------------------------------------------------------------ */
/* progname helpers                                                   */
/* ------------------------------------------------------------------ */

static bool
offset_ok(const char *got, const char *base, const char *exp,
    const char *exp_base)
{
	if (exp == nullptr && got == nullptr)
		return true;
	if (exp == nullptr || got == nullptr || base == nullptr ||
	    exp_base == nullptr)
		return (exp == got);
	return (got - base) == (exp - exp_base);
}

static bool
progname_state_ok(const char *port_base, const char *ref_base,
    const char *ctx, int func)
{
	const char *port_ptr = P::__progname;
	const char *ref_ptr = __progname;

	if (!offset_ok(port_ptr, port_base, ref_ptr, ref_base)) {
		char msg[192];

		std::snprintf(msg, sizeof msg,
		    "port_off=%td ref_off=%td port_ptr=%p ref_ptr=%p",
		    port_ptr != nullptr && port_base != nullptr ?
			port_ptr - port_base : (ptrdiff_t)-1,
		    ref_ptr != nullptr && ref_base != nullptr ?
			ref_ptr - ref_base : (ptrdiff_t)-1,
		    (const void *)port_ptr, (const void *)ref_ptr);
		report(func, ctx, msg);
		return false;
	}
	return true;
}

static bool
getprogname_ok(const char *port_base, const char *ref_base,
    const char *ctx)
{
	const char *a = P::_getprogname();
	const char *b = ref__getprogname();

	if (!offset_ok(a, port_base, b, ref_base)) {
		char msg[192];

		std::snprintf(msg, sizeof msg,
		    "port_off=%td ref_off=%td",
		    a != nullptr && port_base != nullptr ? a - port_base :
			(ptrdiff_t)-1,
		    b != nullptr && ref_base != nullptr ? b - ref_base :
			(ptrdiff_t)-1);
		report(F_GETPROGNAME, ctx, msg);
		return false;
	}
	return true;
}

struct ProgBuf {
	static constexpr size_t PRE = 16;
	static constexpr size_t BODY = 192;
	static constexpr size_t POST = 16;
	unsigned char bytes[PRE + BODY + POST];

	void init(void)
	{
		std::memset(bytes, 0x7f, sizeof bytes);
	}

	char *data(void)
	{
		return (char *)(bytes + PRE);
	}

	size_t cap(void) const
	{
		return BODY;
	}

	void set_cstr(const char *s)
	{
		init();
		std::strncpy(data(), s, cap() - 1);
		data()[cap() - 1] = '\0';
	}

	void set_bytes(const unsigned char *s, size_t n, char term)
	{
		init();
		if (n > cap() - 1)
			n = cap() - 1;
		std::memcpy(data(), s, n);
		data()[n] = term;
	}

	bool identical(const ProgBuf &o) const
	{
		return std::memcmp(bytes, o.bytes, sizeof bytes) == 0;
	}
};

static bool
setprogname_ok(ProgBuf &port_buf, ProgBuf &ref_buf, const char *ctx)
{
	bool ok = true;

	P::setprogname(port_buf.data());
	ref_setprogname(ref_buf.data());

	if (!progname_state_ok(port_buf.data(), ref_buf.data(), ctx,
	    F_SETPROGNAME))
		ok = false;
	if (!port_buf.identical(ref_buf)) {
		report(F_SETPROGNAME, ctx, "input guard buffers diverged");
		ok = false;
	}
	return ok;
}

/* ------------------------------------------------------------------ */
/* freadlink mock                                                     */
/* ------------------------------------------------------------------ */

struct RlCase {
	ssize_t ret;
	int err;
	unsigned char payload[128];
	size_t paylen;
};

static RlCase rl_queue[8];
static int rl_qn;
static int rl_qi;

static void
rl_reset(const RlCase *cases, int n)
{
	rl_qn = n;
	rl_qi = 0;
	if (n > 0)
		std::memcpy(rl_queue, cases, (size_t)n * sizeof(RlCase));
}

extern "C" ssize_t
__wrap_readlinkat(int fd, const char *path, char *buf, size_t bufsize)
{
	(void)fd;
	if (path == nullptr || std::strcmp(path, "") != 0) {
		errno = EINVAL;
		return -1;
	}
	if (rl_qi >= rl_qn) {
		errno = EINVAL;
		return -1;
	}
	const RlCase &c = rl_queue[rl_qi++];
	if (c.ret < 0) {
		errno = c.err;
		return -1;
	}
	size_t n = c.paylen;
	if (n > bufsize)
		n = bufsize;
	if (n > 0)
		std::memcpy(buf, c.payload, n);
	return c.ret;
}

struct LinkBuf {
	static constexpr size_t PRE = 32;
	static constexpr size_t WIN = 256;
	static constexpr size_t POST = 32;
	unsigned char bytes[PRE + WIN + POST];

	void init(void)
	{
		std::memset(bytes, 0x7f, sizeof bytes);
	}

	char *win(void)
	{
		return (char *)(bytes + PRE);
	}

	size_t winsz(void) const
	{
		return WIN;
	}

	bool identical(const LinkBuf &o) const
	{
		return std::memcmp(bytes, o.bytes, sizeof bytes) == 0;
	}
};

static RlCase
make_rl(ssize_t ret, int err, const char *s)
{
	RlCase c{};

	c.ret = ret;
	c.err = err;
	if (s != nullptr) {
		c.paylen = std::strlen(s);
		std::memcpy(c.payload, s, c.paylen);
	} else {
		c.paylen = 0;
	}
	return c;
}

static RlCase
make_rl_bytes(ssize_t ret, int err, const unsigned char *p, size_t n)
{
	RlCase c{};

	c.ret = ret;
	c.err = err;
	c.paylen = n;
	if (n > sizeof c.payload)
		n = sizeof c.payload;
	if (n > 0)
		std::memcpy(c.payload, p, n);
	return c;
}

static bool
freadlink_ok(int fd, size_t bufsize, const RlCase *cases, int ncase,
    const char *ctx)
{
	LinkBuf port_buf, ref_buf;
	bool ok = true;

	port_buf.init();
	ref_buf.init();
	if (bufsize > port_buf.winsz())
		bufsize = port_buf.winsz();

	rl_reset(cases, ncase);
	ssize_t a = P::freadlink(fd, port_buf.win(), bufsize);

	rl_reset(cases, ncase);
	ssize_t b = ref_freadlink(fd, ref_buf.win(), bufsize);

	if (a != b) {
		char msg[128];

		std::snprintf(msg, sizeof msg, "ret port=%zd ref=%zd errno=%d",
		    a, b, errno);
		report(F_FREADLINK, ctx, msg);
		ok = false;
	}
	if (!port_buf.identical(ref_buf)) {
		report(F_FREADLINK, ctx, "full guard buffer mismatch");
		ok = false;
	}
	return ok;
}

/* ------------------------------------------------------------------ */
/* ldexp tests                                                        */
/* ------------------------------------------------------------------ */

static void
test_ldexp_edges(void)
{
	static const int ns[] = {
		0, 1, -1, 2, -2, 10, -10, 53, -53, 1022, 1023, 1024, 1025,
		2046, 2047, 2048, 3000, -1022, -1023, -1075, -2000, INT_MAX,
		INT_MIN
	};
	static const double xs[] = {
		0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 2.0, 1e-308, 1e308,
		1.0 / 3.0, std::numeric_limits<double>::infinity(),
		-std::numeric_limits<double>::infinity(),
		std::numeric_limits<double>::quiet_NaN()
	};

	for (double x : xs) {
		for (int n : ns) {
			ncases[F_LDEXP]++;
			ldexp_ok(x, n, "edge");
		}
	}

	ncases[F_LDEXP]++;
	ldexp_ok(1.0, 1024, "n>1023 once");
	ncases[F_LDEXP]++;
	ldexp_ok(1.0, 2047, "n>1023 twice");
	ncases[F_LDEXP]++;
	ldexp_ok(1.0, 4000, "n>1023 clamp");

	ncases[F_LDEXP]++;
	ldexp_ok(1.0, -1023, "n<-1022 once");
	ncases[F_LDEXP]++;
	ldexp_ok(1.0, -2047, "n<-1022 twice");
	ncases[F_LDEXP]++;
	ldexp_ok(1.0, -5000, "n<-1022 clamp");

	ncases[F_LDEXP]++;
	ldexp_ok(0x1p-600, 0, "subnormal");
	ncases[F_LDEXP]++;
	ldexp_ok(0x1p-600, 600, "subnormal up");
	ncases[F_LDEXP]++;
	ldexp_ok(0x1p-600, -600, "subnormal down");
}

static void
test_ldexp_random(void)
{
	for (int i = 0; i < 200000; i++) {
		double x = randdouble_bits();
		int n = randint();
		char ctx[48];

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		ncases[F_LDEXP]++;
		ldexp_ok(x, n, ctx);
	}
}

/* ------------------------------------------------------------------ */
/* progname tests                                                     */
/* ------------------------------------------------------------------ */

static void
test_progname_edges(void)
{
	static const char *const paths[] = {
		"",
		"a",
		"/",
		"prog",
		"/prog",
		"dir/prog",
		"/usr/bin/ls",
		"///a//b/c",
		"/foo/",
		"a/b/c/d/e",
		"\x80\xff/bin/\xfeprog",
		"/\x80\xff",
		"\xff",
		"/a\xff/b",
	};

	for (const char *s : paths) {
		ProgBuf pb, rb;
		char ctx[80];

		std::snprintf(ctx, sizeof ctx, "set edge \"%s\"", s);
		pb.set_cstr(s);
		rb.set_cstr(s);
		ncases[F_SETPROGNAME]++;
		setprogname_ok(pb, rb, ctx);

		ncases[F_GETPROGNAME]++;
		getprogname_ok(pb.data(), rb.data(), ctx);
	}

	{
		unsigned char raw[] = { 'a', '/', 'b', 0, '/', 'z' };
		ProgBuf pb, rb;
		const char *ctx = "set embedded NUL";

		pb.set_bytes(raw, sizeof raw, '\0');
		rb.set_bytes(raw, sizeof raw, '\0');
		ncases[F_SETPROGNAME]++;
		setprogname_ok(pb, rb, ctx);
		ncases[F_GETPROGNAME]++;
		getprogname_ok(pb.data(), rb.data(), ctx);
	}

	{
		unsigned char raw[] = { 0x80, 0xff, 0xfe, 0x00 };
		ProgBuf pb, rb;

		pb.set_bytes(raw, 3, '\0');
		rb.set_bytes(raw, 3, '\0');
		ncases[F_SETPROGNAME]++;
		setprogname_ok(pb, rb, "high-bit no slash");
		ncases[F_GETPROGNAME]++;
		getprogname_ok(pb.data(), rb.data(), "high-bit no slash");
	}
}

static void
fill_random_path(char *out, size_t cap, std::uint64_t r)
{
	size_t n = (size_t)(r % (cap - 1));
	if (n == 0)
		n = 1;
	for (size_t i = 0; i < n; i++) {
		unsigned v = (unsigned)((r >> (i % 56)) & 0xffu);
		if ((r & (1ULL << (i & 7))) != 0 && (i % 5) == 0)
			out[i] = '/';
		else
			out[i] = (char)(v == 0 ? 'a' : (unsigned char)v);
	}
	out[n] = '\0';
}

static void
test_progname_random(void)
{
	for (int i = 0; i < 200000; i++) {
		ProgBuf pb, rb;
		char ctx[48];

		fill_random_path(pb.data(), pb.cap(), nextrand());
		std::memcpy(rb.bytes, pb.bytes, sizeof pb.bytes);

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		ncases[F_SETPROGNAME]++;
		setprogname_ok(pb, rb, ctx);
		ncases[F_GETPROGNAME]++;
		getprogname_ok(pb.data(), rb.data(), ctx);
	}
}

/* ------------------------------------------------------------------ */
/* freadlink tests                                                    */
/* ------------------------------------------------------------------ */

static void
test_freadlink_edges(void)
{
	RlCase one = make_rl(3, 0, "abc");
	RlCase empty = make_rl(0, 0, "");
	RlCase err = { -1, ENOENT, {}, 0 };
	unsigned char hb[] = { 0x80, 0xff, 0xfe, 0x7f, 0x00 };
	RlCase high = make_rl_bytes(4, 0, hb, 4);
	RlCase trunc;

	trunc = make_rl(10, 0, "0123456789");

	ncases[F_FREADLINK]++;
	freadlink_ok(0, 64, &one, 1, "short link");
	ncases[F_FREADLINK]++;
	freadlink_ok(3, 64, &one, 1, "fd=3");
	ncases[F_FREADLINK]++;
	freadlink_ok(7, 0, &one, 1, "bufsize 0");
	ncases[F_FREADLINK]++;
	freadlink_ok(1, 1, &one, 1, "bufsize 1");
	ncases[F_FREADLINK]++;
	freadlink_ok(2, 2, &one, 1, "bufsize 2 exact");
	ncases[F_FREADLINK]++;
	freadlink_ok(4, 2, &trunc, 1, "truncate");
	ncases[F_FREADLINK]++;
	freadlink_ok(5, 64, &empty, 1, "empty link");
	ncases[F_FREADLINK]++;
	freadlink_ok(6, 64, &err, 1, "ENOENT");
	ncases[F_FREADLINK]++;
	freadlink_ok(9, 64, &high, 1, "high-bit payload");

	{
		RlCase seq[2];

		seq[0] = make_rl(2, 0, "xy");
		seq[1] = err;
		ncases[F_FREADLINK]++;
		freadlink_ok(10, 8, seq, 2, "two-step script");
	}
}

static void
test_freadlink_random(void)
{
	for (int i = 0; i < 200000; i++) {
		RlCase c = {};
		size_t paylen = (size_t)(nextrand() % 80u);
		size_t bufsize = (size_t)(nextrand() % 128u);
		char ctx[48];

		for (size_t j = 0; j < paylen; j++)
			c.payload[j] = (unsigned char)(nextrand() & 0xffu);
		c.paylen = paylen;
		if ((nextrand() & 7u) == 0u) {
			c.ret = -1;
			c.err = (int)(nextrand() % 40u) + 1;
		} else {
			c.ret = (ssize_t)(nextrand() % (paylen + 8u));
			c.err = 0;
		}

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		ncases[F_FREADLINK]++;
		freadlink_ok((int)(nextrand() % 50u), bufsize, &c, 1, ctx);
	}
}

/* ------------------------------------------------------------------ */
/* main                                                               */
/* ------------------------------------------------------------------ */

int
main(void)
{
	test_ldexp_edges();
	test_ldexp_random();
	test_progname_edges();
	test_progname_random();
	test_freadlink_edges();
	test_freadlink_random();

	std::printf("\n%-14s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < NFUNC; i++)
		std::printf("%-14s %12llu %12llu\n", fname[i],
		    ncases[i], nfails[i]);

	unsigned long long total_fail = 0;
	for (int i = 0; i < NFUNC; i++)
		total_fail += nfails[i];

	return total_fail == 0 ? 0 : 1;
}
