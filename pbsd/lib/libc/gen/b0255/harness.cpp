/*
 * Differential harness for batch b0255 (basename).
 * Every case drives both the C++23 port and the C oracle; return pointer
 * offsets (or string content for NULL) and full guard buffers are compared.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.gen.b0255;

namespace P = pbsd::lib_libc_gen::b0255;

extern "C" {
char *ref_basename(char *path);
}

/* ------------------------------------------------------------------ */
/* Statistics                                                         */
/* ------------------------------------------------------------------ */

enum { F_BASENAME, NFUNC };

static const char *const fname[NFUNC] = {
	"basename"
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
		std::printf("FAIL %-20s %s: %s\n", fname[f], ctx, detail);
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

static std::uint32_t
randu32(void)
{
	return (std::uint32_t)(nextrand() & 0xffffffffu);
}

/* ------------------------------------------------------------------ */
/* Guard buffers                                                      */
/* ------------------------------------------------------------------ */

struct GuardBuf {
	static constexpr size_t PRE = 32;
	static constexpr size_t WIN = 256;
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
	copy_path(const char *s)
	{
		size_t n = std::strlen(s);

		if (n >= WIN)
			n = WIN - 1;
		std::memcpy(win(), s, n);
		win()[n] = '\0';
	}

	void
	copy_path_len(const unsigned char *data, size_t n)
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
/* basename                                                           */
/* ------------------------------------------------------------------ */

static bool
basename_null_ok(const char *ctx)
{
	char *a = P::basename(nullptr);
	char *b = ref_basename(nullptr);
	bool ok = true;

	if (a == nullptr || b == nullptr) {
		report(F_BASENAME, ctx, "null return pointer");
		return false;
	}
	if (std::strcmp(a, b) != 0) {
		char msg[96];

		std::snprintf(msg, sizeof msg, "port=\"%s\" ref=\"%s\"", a, b);
		report(F_BASENAME, ctx, msg);
		ok = false;
	}
	return ok;
}

static bool
basename_buf_ok(GuardBuf &gp, GuardBuf &gr, const char *ctx)
{
	char *path_p = gp.win();
	char *path_r = gr.win();
	char *a = P::basename(path_p);
	char *b = ref_basename(path_r);
	bool ok = true;

	if (*path_p == '\0') {
		if (a == nullptr || b == nullptr) {
			report(F_BASENAME, ctx, "null return pointer");
			ok = false;
		} else if (std::strcmp(a, b) != 0) {
			char msg[96];

			std::snprintf(msg, sizeof msg, "port=\"%s\" ref=\"%s\"", a, b);
			report(F_BASENAME, ctx, msg);
			ok = false;
		}
	} else if (!ptr_offset_ok(a, path_p, b, path_r)) {
		char msg[160];

		std::snprintf(msg, sizeof msg,
		    "offset port=%td ref=%td",
		    a != nullptr ? a - path_p : (ptrdiff_t)-1,
		    b != nullptr ? b - path_r : (ptrdiff_t)-1);
		report(F_BASENAME, ctx, msg);
		ok = false;
	}
	if (!gp.identical(gr)) {
		report(F_BASENAME, ctx, "full guard buffer mismatch");
		ok = false;
	}
	return ok;
}

static void
test_basename_edge(const char *path, const char *label)
{
	GuardBuf gp, gr;
	char ctx[96];

	std::snprintf(ctx, sizeof ctx, "%s", label);

	if (path == nullptr) {
		ncases[F_BASENAME]++;
		basename_null_ok(ctx);
		return;
	}

	gp.init();
	gr.init();
	gp.copy_path(path);
	gr.copy_path(path);
	ncases[F_BASENAME]++;
	basename_buf_ok(gp, gr, ctx);
}

static void
test_basename_edge_len(const unsigned char *data, size_t n, const char *label)
{
	GuardBuf gp, gr;
	char ctx[96];

	std::snprintf(ctx, sizeof ctx, "%s", label);
	gp.init();
	gr.init();
	gp.copy_path_len(data, n);
	gr.copy_path_len(data, n);
	ncases[F_BASENAME]++;
	basename_buf_ok(gp, gr, ctx);
}

static void
test_basename_edges(void)
{
	static const char *paths[] = {
		"",
		"/",
		"//",
		"///",
		"////",
		"a",
		"/a",
		"a/",
		"/a/",
		"foo",
		"foo/",
		"/foo",
		"/foo/",
		"foo/bar",
		"/foo/bar",
		"/foo/bar/",
		"///foo///bar///",
		"//a//b//",
		".",
		"..",
		"/.",
		"/..",
		"foo/.",
		"foo/..",
		"a/b/c/d/e",
		"/a/b/c/d/e/",
		"no/slash/at/end",
		"slash/at/end/",
		"x/y",
		"/x/y",
		"/x/y/",
	};

	ncases[F_BASENAME]++;
	basename_null_ok("null path");

	for (size_t i = 0; i < sizeof paths / sizeof paths[0]; i++) {
		char label[64];

		std::snprintf(label, sizeof label, "edge \"%s\"", paths[i]);
		test_basename_edge(paths[i], label);
	}

	{
		static const unsigned char hb1[] = { 0x80 };
		static const unsigned char hb2[] = { '/', 0xff, '/' };
		static const unsigned char hb3[] = {
			0xfe, '/', 0x80, 'b', 'a', 'r', '/', 0xff
		};
		static const unsigned char hb4[] = {
			0x80, 0x81, 0x82, '/', 0xfe, 0xff
		};
		static const unsigned char hb5[] = {
			'/', 0x80, 0x81, 0x82, '/'
		};
		static const unsigned char hb6[] = {
			0xff, 0xff, '/', 0x80
		};

		test_basename_edge_len(hb1, sizeof hb1, "high-bit single 0x80");
		test_basename_edge_len(hb2, sizeof hb2, "high-bit /\\xff/");
		test_basename_edge_len(hb3, sizeof hb3, "high-bit mixed path");
		test_basename_edge_len(hb4, sizeof hb4, "high-bit component");
		test_basename_edge_len(hb5, sizeof hb5, "high-bit trailing slashes");
		test_basename_edge_len(hb6, sizeof hb6, "high-bit only slashes");
	}

	{
		GuardBuf gp, gr;

		gp.init();
		gr.init();
		gp.win()[0] = '\0';
		gr.win()[0] = '\0';
		ncases[F_BASENAME]++;
		basename_buf_ok(gp, gr, "empty first byte only");
	}

	{
		GuardBuf gp, gr;
		size_t i;

		gp.init();
		gr.init();
		for (i = 0; i + 1 < gp.winsz(); i++)
			gp.win()[i] = '/';
		gp.win()[i] = '\0';
		gr.copy_path_len((const unsigned char *)gp.win(), i);
		ncases[F_BASENAME]++;
		basename_buf_ok(gp, gr, "all slashes fill window");
	}
}

static void
fill_random_path(unsigned char *dst, size_t maxn)
{
	size_t n;
	size_t i;
	unsigned char alphabet[] = {
		'/', 'a', 'b', 'c', 'd', 'e', 'f', 'g', '.', '\0', 0x80, 0xfe, 0xff
	};

	if (maxn == 0)
		return;
	n = (size_t)(randu32() % (unsigned)(maxn + 1));
	for (i = 0; i < n; i++)
		dst[i] = alphabet[randu32() % (sizeof alphabet / sizeof alphabet[0])];
	dst[n] = '\0';
}

static void
test_basename_random(void)
{
	for (int i = 0; i < 200000; i++) {
		GuardBuf gp, gr;
		char ctx[48];

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		gp.init();
		gr.init();
		fill_random_path((unsigned char *)gp.win(), gp.winsz() - 1);
		std::memcpy(gr.win(), gp.win(), gr.winsz());
		ncases[F_BASENAME]++;
		basename_buf_ok(gp, gr, ctx);
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

	test_basename_edges();
	test_basename_random();

	std::printf("\n%-20s %10s %10s\n", "Function", "Cases", "Failures");
	for (int f = 0; f < NFUNC; f++) {
		std::printf("%-20s %10llu %10llu\n",
		    fname[f], ncases[f], nfails[f]);
		total_cases += ncases[f];
		total_fails += nfails[f];
	}
	std::printf("%-20s %10llu %10llu\n", "TOTAL", total_cases, total_fails);

	if (total_fails != 0)
		exit_code = 1;
	return exit_code;
}
