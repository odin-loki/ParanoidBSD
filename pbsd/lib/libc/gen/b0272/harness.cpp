/*
 * Differential harness for batch b0272 (__isinf family, __freebsd11_basename*).
 */

#include <cerrno>
#include <cfloat>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/param.h>

#ifndef MAXPATHLEN
#define MAXPATHLEN PATH_MAX
#endif

import pbsd.lib.libc.gen.b0272;

namespace P = pbsd::lib_libc_gen::b0272;

extern "C" {
int ref___isinf(double d);
int ref___isinff(float f);
int ref___isinfl(long double e);
char *ref___freebsd11_basename_r(const char *path, char *bname);
char *ref___freebsd11_basename(char *path);
}

enum {
	F_ISINF,
	F_ISINFF,
	F_ISINFL,
	F_BASENAME_R,
	F_BASENAME,
	NFUNC
};

static const char *const fname[NFUNC] = {
	"__isinf",
	"__isinff",
	"__isinfl",
	"__freebsd11_basename_r",
	"__freebsd11_basename",
};

static unsigned long long ncases[NFUNC];
static unsigned long long nfails[NFUNC];

static constexpr int MAXREPORT = 16;
static int nreported;

static void
report(int f, const char *ctx, const char *detail)
{
	nfails[f]++;
	if (nreported < MAXREPORT) {
		nreported++;
		std::printf("FAIL %-28s %s: %s\n", fname[f], ctx, detail);
	} else if (nreported == MAXREPORT) {
		nreported++;
		std::printf("... further failures suppressed\n");
	}
}

/* ------------------------------------------------------------------ */
/* PRNG (fixed seed)                                                  */
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state = 0xb0272decade0ddecULL;

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

static std::uint32_t
randu32(void)
{
	return (std::uint32_t)(nextrand() & 0xffffffffu);
}

/* ------------------------------------------------------------------ */
/* Bit helpers                                                        */
/* ------------------------------------------------------------------ */

static std::uint64_t
dbits(double d)
{
	std::uint64_t u;

	std::memcpy(&u, &d, sizeof(u));
	return u;
}

static double
fromdbits(std::uint64_t u)
{
	double d;

	std::memcpy(&d, &u, sizeof(d));
	return d;
}

static std::uint32_t
fbits(float f)
{
	std::uint32_t u;

	std::memcpy(&u, &f, sizeof(u));
	return u;
}

static float
fromfbits(std::uint32_t u)
{
	float f;

	std::memcpy(&f, &u, sizeof(u));
	return f;
}

#if LDBL_MANT_DIG == 64
static const std::size_t LD_SIG = 10;
#else
static const std::size_t LD_SIG = sizeof(long double);
#endif

struct ldrep {
	unsigned char b[sizeof(long double)];
};

static ldrep
ldbits(long double x)
{
	ldrep r;

	std::memset(r.b, 0, sizeof(r.b));
	std::memcpy(r.b, &x, LD_SIG);
	return r;
}

static long double
mkld(std::uint16_t se, std::uint64_t m)
{
	unsigned char b[sizeof(long double)];
	long double x;

	std::memset(b, 0, sizeof(b));
	std::memcpy(b, &m, sizeof(m));
	std::memcpy(b + 8, &se, sizeof(se));
	std::memcpy(&x, b, sizeof(x));
	return x;
}

/* ------------------------------------------------------------------ */
/* Guard buffers                                                      */
/* ------------------------------------------------------------------ */

struct GuardBuf {
	static constexpr size_t PRE = 32;
	static constexpr size_t WIN = 8192;
	static constexpr size_t POST = 32;
	unsigned char bytes[PRE + WIN + POST];

	void
	init(void)
	{
		std::memset(bytes, 0x7f, sizeof bytes);
	}

	char *
	win(void)
	{
		return (char *)(bytes + PRE);
	}

	size_t
	winsz(void) const
	{
		return WIN;
	}

	void
	copy_cstr(const char *s)
	{
		size_t n = s != nullptr ? std::strlen(s) : 0;

		if (n >= WIN)
			n = WIN - 1;
		std::memcpy(win(), s, n);
		win()[n] = '\0';
	}

	void
	copy_bytes(const unsigned char *data, size_t n)
	{
		if (n >= WIN)
			n = WIN - 1;
		std::memcpy(win(), data, n);
		win()[n] = '\0';
	}

	bool
	identical(const GuardBuf &o) const
	{
		return std::memcmp(bytes, o.bytes, sizeof bytes) == 0;
	}
};

static bool
ptr_offset_ok(const char *got, const char *base, const char *exp,
    const char *exp_base)
{
	if (exp == nullptr && got == nullptr)
		return true;
	if (exp == nullptr || got == nullptr)
		return false;
	return (got - base) == (exp - exp_base);
}

/* ------------------------------------------------------------------ */
/* __isinf                                                            */
/* ------------------------------------------------------------------ */

static void
check_isinf(double x, const char *ctx)
{
	int p, r;

	ncases[F_ISINF]++;
	p = P::__isinf(x);
	r = ref___isinf(x);
	if (p != r) {
		char msg[96];

		std::snprintf(msg, sizeof msg, "port=%d ref=%d bits=%#018llx",
		    p, r, (unsigned long long)dbits(x));
		report(F_ISINF, ctx, msg);
	}
}

static void
test_isinf_edges(void)
{
	static const std::uint64_t vec[] = {
		0x0000000000000000ull,
		0x8000000000000000ull,
		0x0000000000000001ull,
		0x7ff0000000000000ull,
		0xfff0000000000000ull,
		0x7ff0000000000001ull,
		0xfff0000000000001ull,
		0x7ff8000000000000ull,
		0xfff8000000000000ull,
		0x7fefffffffffffffull,
		0xffefffffffffffffull,
		0x7ff0000000000002ull,
		0x7ff0000000100000ull,
		0x7ff0000000000000ull,
		0x0010000000000000ull,
		0x3ff0000000000000ull,
		0xbff0000000000000ull,
		0x4058ff0000000000ull,
		0x7fef123456789abcull,
		0x000fffffffffffffull,
	};

	for (size_t i = 0; i < sizeof vec / sizeof vec[0]; i++) {
		char ctx[48];

		std::snprintf(ctx, sizeof ctx, "edge %#llx",
		    (unsigned long long)vec[i]);
		check_isinf(fromdbits(vec[i]), ctx);
	}

	check_isinf(1.0 / 0.0, "div +inf");
	check_isinf(-1.0 / 0.0, "div -inf");
	check_isinf(0.0 / 0.0, "div nan");
	check_isinf(1.0, "one");
	check_isinf(-1.0, "neg one");
	check_isinf(0.0, "zero");
	check_isinf(-0.0, "neg zero");
}

static void
test_isinf_random(void)
{
	for (int i = 0; i < 200000; i++) {
		char ctx[48];
		std::uint64_t u;

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		if ((randu32() % 8u) == 0u) {
			u = 0x7ff0000000000000ull;
			u ^= ((std::uint64_t)randu32() << 32) | randu32();
			if ((randu32() & 1u) != 0u)
				u |= 0x8000000000000000ull;
		} else if ((randu32() % 16u) == 1u) {
			u = 0x7fe0000000000000ull |
			    ((std::uint64_t)randu32() << 20) | (randu32() & 0xfffff);
		} else {
			u = ((std::uint64_t)randu32() << 32) | randu32();
		}
		check_isinf(fromdbits(u), ctx);
	}
}

/* ------------------------------------------------------------------ */
/* __isinff                                                           */
/* ------------------------------------------------------------------ */

static void
check_isinff(float x, const char *ctx)
{
	int p, r;

	ncases[F_ISINFF]++;
	p = P::__isinff(x);
	r = ref___isinff(x);
	if (p != r) {
		char msg[80];

		std::snprintf(msg, sizeof msg, "port=%d ref=%d bits=%#010x", p, r,
		    (unsigned)fbits(x));
		report(F_ISINFF, ctx, msg);
	}
}

static void
test_isinff_edges(void)
{
	static const std::uint32_t vec[] = {
		0x00000000u,
		0x80000000u,
		0x7f800000u,
		0xff800000u,
		0x7f800001u,
		0xff800001u,
		0x7fc00000u,
		0x7f7fffffu,
		0xff7fffffu,
		0x3f800000u,
		0xbf800000u,
		0x7f800002u,
		0x7f801000u,
		0x00000001u,
		0x80800000u,
	};

	for (size_t i = 0; i < sizeof vec / sizeof vec[0]; i++) {
		char ctx[48];

		std::snprintf(ctx, sizeof ctx, "edge %#x", vec[i]);
		check_isinff(fromfbits(vec[i]), ctx);
	}

	check_isinff(1.0f / 0.0f, "div +inf");
	check_isinff(-1.0f / 0.0f, "div -inf");
	check_isinff(0.0f / 0.0f, "div nan");
}

static void
test_isinff_random(void)
{
	for (int i = 0; i < 200000; i++) {
		char ctx[48];
		std::uint32_t u;

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		if ((randu32() % 8u) == 0u) {
			u = 0x7f800000u | (randu32() & 0x007fffffu);
			if ((randu32() & 1u) != 0u)
				u |= 0x80000000u;
		} else if ((randu32() % 16u) == 1u) {
			u = 0x7f000000u | (randu32() & 0x00ffffffu);
		} else {
			u = randu32();
		}
		check_isinff(fromfbits(u), ctx);
	}
}

/* ------------------------------------------------------------------ */
/* __isinfl                                                           */
/* ------------------------------------------------------------------ */

static void
check_isinfl(long double x, const char *ctx)
{
	int p, r;

	ncases[F_ISINFL]++;
	p = P::__isinfl(x);
	r = ref___isinfl(x);
	if (p != r) {
		char msg[128];
		ldrep bits = ldbits(x);

		std::snprintf(msg, sizeof msg, "port=%d ref=%d ld=", p, r);
		report(F_ISINFL, ctx, msg);
		if (nreported <= MAXREPORT) {
			for (std::size_t j = LD_SIG; j-- > 0;)
				std::printf("%02x", bits.b[j]);
			std::printf("\n");
		}
	}
}

static void
test_isinfl_edges(void)
{
	static const struct {
		std::uint16_t se;
		std::uint64_t m;
	} vec[] = {
		{ 0x0000u, 0x0000000000000000ull },
		{ 0x8000u, 0x0000000000000000ull },
		{ 0x3fffu, 0x8000000000000000ull },
		{ 0xbfffu, 0x8000000000000000ull },
		{ 0x7fffu, 0x8000000000000000ull },
		{ 0xffffu, 0x8000000000000000ull },
		{ 0x7fffu, 0x8000000000000001ull },
		{ 0xffffu, 0x8000000000000001ull },
		{ 0x7fffu, 0xc000000000000000ull },
		{ 0x7ffeu, 0xffffffffffffffffull },
		{ 0x7fffu, 0x0000000080000000ull },
		{ 0x7fffu, 0xffffffffffffffffull },
		{ 0x4000u, 0x8000000000000000ull },
		{ 0x0001u, 0x8000000000000001ull },
	};

	for (size_t i = 0; i < sizeof vec / sizeof vec[0]; i++) {
		char ctx[48];

		std::snprintf(ctx, sizeof ctx, "edge se=%#06x m=%#llx", vec[i].se,
		    (unsigned long long)vec[i].m);
		check_isinfl(mkld(vec[i].se, vec[i].m), ctx);
	}

	check_isinfl(1.0L / 0.0L, "div +inf");
	check_isinfl(-1.0L / 0.0L, "div -inf");
	check_isinfl(0.0L / 0.0L, "div nan");
	check_isinfl(1.0L, "one");
	check_isinfl(0.0L, "zero");
}

static void
test_isinfl_random(void)
{
	for (int i = 0; i < 200000; i++) {
		char ctx[48];
		std::uint16_t se;
		std::uint64_t m;

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		if ((randu32() % 8u) == 0u) {
			se = (randu32() & 1u) != 0u ? 0xffffu : 0x7fffu;
			m = ((std::uint64_t)randu32() << 32) | randu32();
			if ((randu32() % 4u) == 0u)
				m |= 0x8000000000000000ull;
		} else if ((randu32() % 16u) == 1u) {
			se = (std::uint16_t)(0x7ffeu - (randu32() % 4u));
			m = ((std::uint64_t)randu32() << 32) | randu32();
		} else {
			se = (std::uint16_t)(randu32() & 0xffffu);
			m = ((std::uint64_t)randu32() << 32) | randu32();
		}
		check_isinfl(mkld(se, m), ctx);
	}
}

/* ------------------------------------------------------------------ */
/* __freebsd11_basename_r                                             */
/* ------------------------------------------------------------------ */

static bool
basename_r_case(const char *path, GuardBuf &path_p, GuardBuf &path_r,
    GuardBuf &out_p, GuardBuf &out_r, const char *ctx)
{
	char *bp = out_p.win();
	char *br = out_r.win();
	int e0 = errno;
	int perrno, rerrno;
	char *a;
	char *b;
	bool ok = true;

	errno = 0;
	a = P::__freebsd11_basename_r(path, bp);
	perrno = errno;
	errno = 0;
	b = ref___freebsd11_basename_r(path, br);
	rerrno = errno;

	if (!ptr_offset_ok(a, bp, b, br)) {
		char msg[160];

		std::snprintf(msg, sizeof msg,
		    "offset port=%td ref=%td",
		    a != nullptr ? a - bp : (ptrdiff_t)-1,
		    b != nullptr ? b - br : (ptrdiff_t)-1);
		report(F_BASENAME_R, ctx, msg);
		ok = false;
	}
	if (perrno != rerrno) {
		char msg[80];

		std::snprintf(msg, sizeof msg, "errno port=%d ref=%d", perrno,
		    rerrno);
		report(F_BASENAME_R, ctx, msg);
		ok = false;
	}
	if (!out_p.identical(out_r)) {
		report(F_BASENAME_R, ctx, "output guard buffer mismatch");
		ok = false;
	}
	if (!path_p.identical(path_r)) {
		report(F_BASENAME_R, ctx, "path guard buffer mismatch");
		ok = false;
	}
	errno = e0;
	return ok;
}

static void
test_basename_r_edge(const char *path, const char *label)
{
	GuardBuf path_p, path_r, out_p, out_r;
	char ctx[96];

	std::snprintf(ctx, sizeof ctx, "%s", label);
	path_p.init();
	path_r.init();
	out_p.init();
	out_r.init();
	if (path != nullptr) {
		path_p.copy_cstr(path);
		path_r.copy_cstr(path);
	}
	ncases[F_BASENAME_R]++;
	basename_r_case(path, path_p, path_r, out_p, out_r, ctx);
}

static void
test_basename_r_edge_bytes(const unsigned char *data, size_t n,
    const char *label)
{
	GuardBuf path_p, path_r, out_p, out_r;
	char ctx[96];

	std::snprintf(ctx, sizeof ctx, "%s", label);
	path_p.init();
	path_r.init();
	out_p.init();
	out_r.init();
	path_p.copy_bytes(data, n);
	path_r.copy_bytes(data, n);
	ncases[F_BASENAME_R]++;
	basename_r_case(path_p.win(), path_p, path_r, out_p, out_r, ctx);
}

static void
test_basename_r_edges(void)
{
	static const char *paths[] = {
		"",
		"/",
		"//",
		"///",
		"a",
		"/a",
		"a/",
		"/a/",
		"foo",
		"foo/",
		"/foo",
		"/foo/",
		"foo/bar",
		"/foo/bar/",
		"///foo///bar///",
		".",
		"..",
		"/.",
		"a/b/c/d/e",
		"/a/b/c/d/e/",
		"x/y",
		"/x/y/",
	};

	ncases[F_BASENAME_R]++;
	test_basename_r_edge(nullptr, "null path");

	for (size_t i = 0; i < sizeof paths / sizeof paths[0]; i++) {
		char label[64];

		std::snprintf(label, sizeof label, "edge \"%s\"", paths[i]);
		test_basename_r_edge(paths[i], label);
	}

	{
		static const unsigned char hb[] = {
			0xfe, '/', 0x80, 'b', 'a', 'r', '/', 0xff
		};

		test_basename_r_edge_bytes(hb, sizeof hb, "high-bit path");
	}

	{
		GuardBuf path_p, path_r, out_p, out_r;
		size_t n = (size_t)MAXPATHLEN;

		path_p.init();
		path_r.init();
		out_p.init();
		out_r.init();
		std::memset(path_p.win(), 'a', n);
		path_p.win()[n] = '\0';
		path_r.copy_cstr(path_p.win());
		ncases[F_BASENAME_R]++;
		basename_r_case(path_p.win(), path_p, path_r, out_p, out_r,
		    "len==MAXPATHLEN fail");
	}

	{
		GuardBuf path_p, path_r, out_p, out_r;
		size_t n = (size_t)MAXPATHLEN - 1;

		path_p.init();
		path_r.init();
		out_p.init();
		out_r.init();
		std::memset(path_p.win(), 'b', n);
		path_p.win()[n] = '\0';
		path_r.copy_cstr(path_p.win());
		ncases[F_BASENAME_R]++;
		basename_r_case(path_p.win(), path_p, path_r, out_p, out_r,
		    "len==MAXPATHLEN-1 ok");
	}

	{
		GuardBuf path_p, path_r, out_p, out_r;

		path_p.init();
		path_r.init();
		out_p.init();
		out_r.init();
		std::snprintf(path_p.win(), path_p.winsz(), "/%.*s",
		    MAXPATHLEN - 2,
		    "ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc");
		path_r.copy_cstr(path_p.win());
		ncases[F_BASENAME_R]++;
		basename_r_case(path_p.win(), path_p, path_r, out_p, out_r,
		    "slash+long component fail");
	}
}

static void
fill_random_path(unsigned char *dst, size_t maxn)
{
	size_t n;
	size_t i;
	unsigned char alphabet[] = {
		'/', 'a', 'b', 'c', 'd', 'e', 'f', '.', 0x80, 0xfe, 0xff
	};

	if (maxn == 0)
		return;
	n = (size_t)(randu32() % (unsigned)(maxn + 1));
	for (i = 0; i < n; i++)
		dst[i] = alphabet[randu32() % (sizeof alphabet / sizeof alphabet[0])];
	dst[n] = '\0';
}

static void
test_basename_r_random(void)
{
	for (int i = 0; i < 200000; i++) {
		GuardBuf path_p, path_r, out_p, out_r;
		char ctx[48];
		const char *path;

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		path_p.init();
		path_r.init();
		out_p.init();
		out_r.init();
		if ((randu32() % 64u) == 0u)
			path = nullptr;
		else
			path = path_p.win();
		if (path != nullptr) {
			fill_random_path((unsigned char *)path_p.win(),
			    path_p.winsz() - 1);
			std::memcpy(path_r.win(), path_p.win(), path_p.winsz());
		}
		ncases[F_BASENAME_R]++;
		basename_r_case(path, path_p, path_r, out_p, out_r, ctx);
	}
}

/* ------------------------------------------------------------------ */
/* __freebsd11_basename                                               */
/* ------------------------------------------------------------------ */

static bool
basename_case(char *path_p, char *path_r, const char *ctx)
{
	char *a;
	char *b;
	char *a0;
	char *b0;
	bool ok = true;

	a = P::__freebsd11_basename(path_p);
	b = ref___freebsd11_basename(path_r);

	if ((a == nullptr) != (b == nullptr)) {
		report(F_BASENAME, ctx, "null mismatch");
		ok = false;
	}
	if (a != nullptr && b != nullptr && std::strcmp(a, b) != 0) {
		char msg[128];

		std::snprintf(msg, sizeof msg, "port=\"%s\" ref=\"%s\"", a, b);
		report(F_BASENAME, ctx, msg);
		ok = false;
	}

	a0 = P::__freebsd11_basename(path_p);
	b0 = ref___freebsd11_basename(path_r);
	if (a != nullptr && a0 != nullptr && a != a0) {
		report(F_BASENAME, ctx, "port static pointer unstable");
		ok = false;
	}
	if (b != nullptr && b0 != nullptr && b != b0) {
		report(F_BASENAME, ctx, "ref static pointer unstable");
		ok = false;
	}
	if (a != nullptr && a0 != nullptr && b != nullptr && b0 != nullptr) {
		ptrdiff_t po = a - a0;
		ptrdiff_t ro = b - b0;

		if (po != ro) {
			char msg[80];

			std::snprintf(msg, sizeof msg,
			    "static offset port=%td ref=%td", po, ro);
			report(F_BASENAME, ctx, msg);
			ok = false;
		}
	}
	return ok;
}

static void
test_basename_edge(const char *path, const char *label)
{
	GuardBuf gp, gr;
	char ctx[96];

	std::snprintf(ctx, sizeof ctx, "%s", label);
	gp.init();
	gr.init();
	if (path != nullptr) {
		gp.copy_cstr(path);
		gr.copy_cstr(path);
	}
	ncases[F_BASENAME]++;
	basename_case(path != nullptr ? gp.win() : nullptr,
	    path != nullptr ? gr.win() : nullptr, ctx);
	if (!gp.identical(gr))
		report(F_BASENAME, ctx, "path guard buffer mismatch");
}

static void
test_basename_edges(void)
{
	static const char *paths[] = {
		"",
		"/",
		"//",
		"foo",
		"/foo/",
		"foo/bar",
		"/foo/bar/",
		"///a///b///",
	};

	ncases[F_BASENAME]++;
	test_basename_edge(nullptr, "null path");

	for (size_t i = 0; i < sizeof paths / sizeof paths[0]; i++) {
		char label[64];

		std::snprintf(label, sizeof label, "edge \"%s\"", paths[i]);
		test_basename_edge(paths[i], label);
	}

	{
		GuardBuf gp, gr;
		static const unsigned char hb[] = { 0x80, '/', 0xff };

		gp.init();
		gr.init();
		gp.copy_bytes(hb, sizeof hb);
		gr.copy_bytes(hb, sizeof hb);
		ncases[F_BASENAME]++;
		basename_case(gp.win(), gr.win(), "high-bit path");
		if (!gp.identical(gr))
			report(F_BASENAME, "high-bit path", "path guard mismatch");
	}
}

static void
test_basename_random(void)
{
	for (int i = 0; i < 200000; i++) {
		GuardBuf gp, gr;
		char ctx[48];
		char *path;

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		gp.init();
		gr.init();
		if ((randu32() % 64u) == 0u)
			path = nullptr;
		else {
			fill_random_path((unsigned char *)gp.win(), gp.winsz() - 1);
			std::memcpy(gr.win(), gp.win(), gp.winsz());
			path = gp.win();
		}
		ncases[F_BASENAME]++;
		basename_case(path, path != nullptr ? gr.win() : nullptr, ctx);
		if (!gp.identical(gr))
			report(F_BASENAME, ctx, "path guard buffer mismatch");
	}
}

/* ------------------------------------------------------------------ */
/* main                                                               */
/* ------------------------------------------------------------------ */

int
main(void)
{
	unsigned long long total_cases = 0;
	unsigned long long total_fails = 0;
	int exit_code = 0;

	test_isinf_edges();
	test_isinf_random();
	test_isinff_edges();
	test_isinff_random();
	test_isinfl_edges();
	test_isinfl_random();
	test_basename_r_edges();
	test_basename_r_random();
	test_basename_edges();
	test_basename_random();

	std::printf("\n%-28s %10s %10s\n", "Function", "Cases", "Failures");
	for (int f = 0; f < NFUNC; f++) {
		std::printf("%-28s %10llu %10llu\n",
		    fname[f], ncases[f], nfails[f]);
		total_cases += ncases[f];
		total_fails += nfails[f];
	}
	std::printf("%-28s %10llu %10llu\n", "TOTAL", total_cases, total_fails);

	if (total_fails != 0)
		exit_code = 1;
	return exit_code;
}
