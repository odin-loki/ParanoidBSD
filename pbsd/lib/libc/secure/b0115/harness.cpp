/*
 * harness.cpp -- differential test for PBSD batch b0115.
 */

#include <csetjmp>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.secure.b0115;

namespace P = pbsd::lib_libc_secure::b0115;

extern "C" {
void *ref___memmove_chk(void *, const void *, size_t, size_t);
void *ref___memcpy_chk(void *__restrict, const void *__restrict, size_t,
    size_t);
int ref___vsnprintf_chk(char *__restrict, size_t, int, size_t,
    const char *__restrict, va_list);
char *ref___strncpy_chk(char *__restrict, const char *__restrict, size_t,
    size_t);
}

/* ------------------------------------------------------------------------ */

static constexpr unsigned char GUARD = 0x7f;
static constexpr int MAX_REPORT = 8;
static constexpr long SWEEP = 200000;
static constexpr std::size_t ARENA = 256;
static constexpr std::size_t PAD = 32;

static jmp_buf g_chk_jmp;
static volatile int g_chk_jmp_active;

extern "C" void
__chk_fail(void)
{
	if (g_chk_jmp_active)
		longjmp(g_chk_jmp, 1);
	std::abort();
}

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

static Stat st_memmove = { "__memmove_chk", 0, 0, 0 };
static Stat st_memcpy = { "__memcpy_chk", 0, 0, 0 };
static Stat st_vsnprintf = { "__vsnprintf_chk", 0, 0, 0 };
static Stat st_strncpy = { "__strncpy_chk", 0, 0, 0 };

static std::uint64_t rng_state = 0xb0115feedfaceULL;

static inline std::uint64_t
rnd(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static inline std::size_t
rnd_mod(std::size_t m)
{
	if (m == 0)
		return 0;
	return (std::size_t)(rnd() % (std::uint64_t)m);
}

static void
stat_fail(Stat *st, const char *tag, const char *detail)
{
	st->fails++;
	if (st->reported < MAX_REPORT) {
		st->reported++;
		std::printf("  FAIL %s [%s] %s\n", st->name, tag, detail);
	}
}

enum class Outcome { OK, CHK_FAIL };

struct MemObs {
	Outcome outcome;
	void *ret;
	std::ptrdiff_t ret_off;
	unsigned char buf[ARENA + 2 * PAD];
};

static void
fill_guard(unsigned char *p, std::size_t n)
{
	std::memset(p, GUARD, n);
}

static void
copy_payload(unsigned char *dst, const unsigned char *src, std::size_t n)
{
	std::memcpy(dst, src, n);
}

static unsigned char *
arena_data(unsigned char *arena)
{
	return arena + PAD;
}

template<typename Fn>
static MemObs
run_mem_at(Fn fn, unsigned char *arena, std::size_t dst_off, const void *src,
    std::size_t len, std::size_t slen, bool copy_in)
{
	MemObs obs{};

	fill_guard(arena, sizeof(obs.buf));
	if (copy_in)
		copy_payload(arena_data(arena) + dst_off,
		    (const unsigned char *)src, len);

	g_chk_jmp_active = 1;
	if (setjmp(g_chk_jmp) != 0) {
		g_chk_jmp_active = 0;
		obs.outcome = Outcome::CHK_FAIL;
		obs.ret = nullptr;
		obs.ret_off = -1;
		return obs;
	}

	obs.ret = fn(arena_data(arena) + dst_off, src, len, slen);
	g_chk_jmp_active = 0;
	obs.outcome = Outcome::OK;
	obs.ret_off = (unsigned char *)obs.ret - (arena_data(arena) + dst_off);
	return obs;
}

template<typename Fn>
static MemObs
run_mem(Fn fn, unsigned char *arena, const void *src, std::size_t len,
    std::size_t slen, bool copy_in)
{
	return run_mem_at(fn, arena, 0, src, len, slen, copy_in);
}

static bool
mem_obs_eq(const MemObs &a, const MemObs &b)
{
	if (a.outcome != b.outcome)
		return false;
	if (a.outcome == Outcome::CHK_FAIL)
		return true;
	if (a.ret_off != b.ret_off)
		return false;
	return std::memcmp(a.buf, b.buf, sizeof(a.buf)) == 0;
}

static void
check_mem(Stat *st, const char *tag, const MemObs &a, const MemObs &b)
{
	st->cases++;
	if (!mem_obs_eq(a, b)) {
		char detail[160];
		std::snprintf(detail, sizeof(detail),
		    "out %d/%d off %td/%td", (int)a.outcome, (int)b.outcome,
		    a.ret_off, b.ret_off);
		stat_fail(st, tag, detail);
	}
}

static void
memmove_case(Stat *st, const char *tag, const unsigned char *src,
    std::size_t len, std::size_t slen, std::size_t dst_off, std::size_t src_off)
{
	unsigned char arena_a[ARENA + 2 * PAD];
	unsigned char arena_b[ARENA + 2 * PAD];
	unsigned char srcbuf[ARENA];
	MemObs pa, pb;

	std::memcpy(srcbuf, src, len);
	if (src_off != dst_off) {
		fill_guard(arena_a, sizeof(arena_a));
		fill_guard(arena_b, sizeof(arena_b));
		std::memcpy(arena_data(arena_a) + src_off, srcbuf, len);
		std::memcpy(arena_data(arena_b) + src_off, srcbuf, len);
		pa = run_mem_at(
		    [](void *d, const void *s, std::size_t l, std::size_t sl) {
			    return P::__memmove_chk(d, s, l, sl);
		    },
		    arena_a, dst_off, arena_data(arena_a) + src_off, len, slen,
		    false);
		pb = run_mem_at(
		    [](void *d, const void *s, std::size_t l, std::size_t sl) {
			    return ref___memmove_chk(d, s, l, sl);
		    },
		    arena_b, dst_off, arena_data(arena_b) + src_off, len, slen,
		    false);
	} else {
		pa = run_mem(
		    [](void *d, const void *s, std::size_t l, std::size_t sl) {
			    return P::__memmove_chk(d, s, l, sl);
		    },
		    arena_a, srcbuf, len, slen, true);
		pb = run_mem(
		    [](void *d, const void *s, std::size_t l, std::size_t sl) {
			    return ref___memmove_chk(d, s, l, sl);
		    },
		    arena_b, srcbuf, len, slen, true);
	}
	check_mem(st, tag, pa, pb);
}

static void
memcpy_case(Stat *st, const char *tag, const unsigned char *src,
    std::size_t len, std::size_t slen, bool use_overlap)
{
	unsigned char arena_a[ARENA + 2 * PAD];
	unsigned char arena_b[ARENA + 2 * PAD];
	unsigned char srcbuf[ARENA];
	MemObs pa, pb;

	std::memcpy(srcbuf, src, len);
	if (use_overlap) {
		fill_guard(arena_a, sizeof(arena_a));
		fill_guard(arena_b, sizeof(arena_b));
		std::memcpy(arena_data(arena_a) + 8, srcbuf, len);
		std::memcpy(arena_data(arena_b) + 8, srcbuf, len);
		pa = run_mem(
		    [](void *d, const void *s, std::size_t l, std::size_t sl) {
			    return P::__memcpy_chk(d, s, l, sl);
		    },
		    arena_a, arena_data(arena_a) + 4, len, slen, false);
		pb = run_mem(
		    [](void *d, const void *s, std::size_t l, std::size_t sl) {
			    return ref___memcpy_chk(d, s, l, sl);
		    },
		    arena_b, arena_data(arena_b) + 4, len, slen, false);
	} else {
		pa = run_mem(
		    [](void *d, const void *s, std::size_t l, std::size_t sl) {
			    return P::__memcpy_chk(d, s, l, sl);
		    },
		    arena_a, srcbuf, len, slen, true);
		pb = run_mem(
		    [](void *d, const void *s, std::size_t l, std::size_t sl) {
			    return ref___memcpy_chk(d, s, l, sl);
		    },
		    arena_b, srcbuf, len, slen, true);
	}
	check_mem(st, tag, pa, pb);
}

static void
strncpy_case(Stat *st, const char *tag, const char *src, std::size_t len,
    std::size_t slen, bool use_overlap)
{
	unsigned char arena_a[ARENA + 2 * PAD];
	unsigned char arena_b[ARENA + 2 * PAD];
	char srcbuf[ARENA];
	MemObs pa, pb;

	std::memcpy(srcbuf, src, len);
	srcbuf[len] = '\0';
	if (use_overlap) {
		fill_guard(arena_a, sizeof(arena_a));
		fill_guard(arena_b, sizeof(arena_b));
		std::memcpy(arena_data(arena_a) + 16, srcbuf, len + 1);
		std::memcpy(arena_data(arena_b) + 16, srcbuf, len + 1);
		pa = run_mem(
		    [](void *d, const void *s, std::size_t l, std::size_t sl) {
			    return (void *)P::__strncpy_chk((char *)d,
				(const char *)s, l, sl);
		    },
		    arena_a, arena_data(arena_a) + 8, len, slen, false);
		pb = run_mem(
		    [](void *d, const void *s, std::size_t l, std::size_t sl) {
			    return (void *)ref___strncpy_chk((char *)d,
				(const char *)s, l, sl);
		    },
		    arena_b, arena_data(arena_b) + 8, len, slen, false);
	} else {
		pa = run_mem(
		    [](void *d, const void *s, std::size_t l, std::size_t sl) {
			    return (void *)P::__strncpy_chk((char *)d,
				(const char *)s, l, sl);
		    },
		    arena_a, srcbuf, len, slen, true);
		pb = run_mem(
		    [](void *d, const void *s, std::size_t l, std::size_t sl) {
			    return (void *)ref___strncpy_chk((char *)d,
				(const char *)s, l, sl);
		    },
		    arena_b, srcbuf, len, slen, true);
	}
	check_mem(st, tag, pa, pb);
}

struct VsnObs {
	Outcome outcome;
	int ret;
	unsigned char buf[128 + 2 * PAD];
};

static bool
vsn_obs_eq(const VsnObs &a, const VsnObs &b)
{
	if (a.outcome != b.outcome)
		return false;
	if (a.outcome == Outcome::CHK_FAIL)
		return true;
	if (a.ret != b.ret)
		return false;
	return std::memcmp(a.buf, b.buf, sizeof(a.buf)) == 0;
}

using vsnprintf_fn = int (*)(char *, std::size_t, int, std::size_t,
    const char *, va_list);

static VsnObs
run_vsnprintf_side(vsnprintf_fn fn, unsigned char *buf, std::size_t len,
    int flags, std::size_t slen, const char *fmt_copy, va_list ap_in)
{
	VsnObs obs{};

	fill_guard(buf, 128 + 2 * PAD);

	g_chk_jmp_active = 1;
	if (setjmp(g_chk_jmp) != 0) {
		g_chk_jmp_active = 0;
		obs.outcome = Outcome::CHK_FAIL;
		obs.ret = -1;
		std::memcpy(obs.buf, buf, sizeof(obs.buf));
		return obs;
	}

	if (len > slen) {
		std::memset(&ap, 0, sizeof(ap));
		obs.ret = fn((char *)arena_data(buf), len, flags, slen,
		    fmt_copy, ap);
	} else {
		obs.ret = fn((char *)arena_data(buf), len, flags, slen,
		    fmt_copy, ap_in);
	}
	g_chk_jmp_active = 0;
	obs.outcome = Outcome::OK;
	std::memcpy(obs.buf, buf, sizeof(obs.buf));
	return obs;
}

static void
vsnprintf_case(Stat *st, const char *tag, std::size_t len, int flags,
    std::size_t slen, const char *fmt, ...)
{
	unsigned char bufa[128 + 2 * PAD];
	unsigned char bufb[128 + 2 * PAD];
	VsnObs pa, pb;
	va_list args;
	char fmt_copy[64];

	std::snprintf(fmt_copy, sizeof(fmt_copy), "%s", fmt);

	st->cases++;

	va_start(args, fmt);
	{
		va_list a1, a2;
		va_copy(a1, args);
		va_copy(a2, args);
		pa = run_vsnprintf_side(
		    [](char *b, std::size_t l, int f, std::size_t s,
			const char *fc, va_list ap) {
			    return P::__vsnprintf_chk(b, l, f, s, fc, ap);
		    },
		    bufa, len, flags, slen, fmt_copy, a1);
		pb = run_vsnprintf_side(
		    [](char *b, std::size_t l, int f, std::size_t s,
			const char *fc, va_list ap) {
			    return ref___vsnprintf_chk(b, l, f, s, fc, ap);
		    },
		    bufb, len, flags, slen, fmt_copy, a2);
		va_end(a1);
		va_end(a2);
	}
	va_end(args);

	if (!vsn_obs_eq(pa, pb)) {
		char detail[160];
		std::snprintf(detail, sizeof(detail),
		    "fmt='%s' len=%zu slen=%zu ret %d/%d out %d/%d", fmt,
		    len, slen, pa.ret, pb.ret, (int)pa.outcome,
		    (int)pb.outcome);
		stat_fail(st, tag, detail);
	}
}

/* ------------------------------------------------------------------------ */

static void
memmove_edges(void)
{
	static const unsigned char pat0[] = "";
	static const unsigned char pat1[] = { 'a' };
	static const unsigned char pat2[] = { 0x80 };
	static const unsigned char pat3[] = { 0xff };
	static const unsigned char pat4[] = { 'a', 'b', 'c', 'd', '\0' };
	static const unsigned char pat5[] = { 0x80, 0x81, 0xff, 0x00, 0x7f };

	memmove_case(&st_memmove, "empty", pat0, 0, 0, 0, 0);
	memmove_case(&st_memmove, "empty_slen1", pat0, 0, 1, 0, 0);
	memmove_case(&st_memmove, "one", pat1, 1, 1, 0, 0);
	memmove_case(&st_memmove, "one_fail", pat1, 1, 0, 0, 0);
	memmove_case(&st_memmove, "hibyte", pat2, 1, 1, 0, 0);
	memmove_case(&st_memmove, "0xff", pat3, 1, 1, 0, 0);
	memmove_case(&st_memmove, "four", pat4, 4, 4, 0, 0);
	memmove_case(&st_memmove, "four_slen3", pat4, 4, 3, 0, 0);
	memmove_case(&st_memmove, "four_slen5", pat4, 4, 5, 0, 0);
	memmove_case(&st_memmove, "nul_mid", pat5, 5, 5, 0, 0);
	memmove_case(&st_memmove, "overlap_fwd", pat4, 4, 8, 0, 2);
	memmove_case(&st_memmove, "overlap_back", pat4, 4, 8, 2, 0);
	memmove_case(&st_memmove, "touch_end", pat4, 4, 8, 0, 4);
}

static void
memcpy_edges(void)
{
	static const unsigned char pat0[] = "";
	static const unsigned char pat1[] = { 'x' };
	static const unsigned char pat2[] = { 0x80, 0xff };
	static const unsigned char pat3[] = { 'a', 'b', 'c', 'd' };
	static const unsigned char pat4[] = { 0x00, 0x80, 0xff, 0x00 };

	memcpy_case(&st_memcpy, "empty", pat0, 0, 0, false);
	memcpy_case(&st_memcpy, "empty_slen1", pat0, 0, 1, false);
	memcpy_case(&st_memcpy, "one", pat1, 1, 1, false);
	memcpy_case(&st_memcpy, "one_fail", pat1, 1, 0, false);
	memcpy_case(&st_memcpy, "two_hibyte", pat2, 2, 2, false);
	memcpy_case(&st_memcpy, "four", pat3, 4, 4, false);
	memcpy_case(&st_memcpy, "four_slen3", pat3, 4, 3, false);
	memcpy_case(&st_memcpy, "four_slen5", pat3, 4, 5, false);
	memcpy_case(&st_memcpy, "nul_mix", pat4, 4, 4, false);
	memcpy_case(&st_memcpy, "overlap", pat3, 4, 8, true);
	memcpy_case(&st_memcpy, "overlap_fail_len", pat3, 4, 3, true);
}

static void
strncpy_edges(void)
{
	static const char s0[] = "";
	static const char s1[] = "a";
	static const char s2[] = "\x80\xff";
	static const char s3[] = "abcd";
	static const char s4[] = "abc";
	static const char s5[] = { 'a', '\0', 'b', 'c', 'd', '\0' };

	strncpy_case(&st_strncpy, "empty", s0, 0, 0, false);
	strncpy_case(&st_strncpy, "empty_slen1", s0, 0, 1, false);
	strncpy_case(&st_strncpy, "one", s1, 1, 1, false);
	strncpy_case(&st_strncpy, "one_fail", s1, 1, 0, false);
	strncpy_case(&st_strncpy, "hibyte", s2, 2, 2, false);
	strncpy_case(&st_strncpy, "pad", s3, 8, 8, false);
	strncpy_case(&st_strncpy, "short_src", s4, 8, 8, false);
	strncpy_case(&st_strncpy, "early_nul", s5, 6, 6, false);
	strncpy_case(&st_strncpy, "slen3", s3, 4, 3, false);
	strncpy_case(&st_strncpy, "overlap", s3, 4, 8, true);
}

static void
vsnprintf_edges(void)
{
	vsnprintf_case(&st_vsnprintf, "empty", 0, 0, 0, "");
	vsnprintf_case(&st_vsnprintf, "empty_slen1", 0, 0, 1, "");
	vsnprintf_case(&st_vsnprintf, "len1", 1, 0, 1, "");
	vsnprintf_case(&st_vsnprintf, "len_fail", 2, 0, 1, "x");
	vsnprintf_case(&st_vsnprintf, "pct", 8, 0, 8, "%%");
	vsnprintf_case(&st_vsnprintf, "int0", 16, 0, 16, "%d", 0);
	vsnprintf_case(&st_vsnprintf, "int_neg", 16, 0, 16, "%d", -42);
	vsnprintf_case(&st_vsnprintf, "uint255", 16, 0, 16, "%u", 255U);
	vsnprintf_case(&st_vsnprintf, "char_hi", 16, 0, 16, "%c", (int)(char)0x80);
	vsnprintf_case(&st_vsnprintf, "char_ff", 16, 0, 16, "%c", (int)(char)0xff);
	vsnprintf_case(&st_vsnprintf, "str_empty", 16, 0, 16, "%s", "");
	vsnprintf_case(&st_vsnprintf, "str_hi", 16, 0, 16, "%s", "\x80\xff");
	vsnprintf_case(&st_vsnprintf, "trunc", 4, 0, 8, "%s", "hello");
	vsnprintf_case(&st_vsnprintf, "slen_eq", 8, 1, 8, "%d", 7);
	vsnprintf_case(&st_vsnprintf, "slen_lt", 8, 2, 7, "%d", 8);
}

static const char *const vsn_fmts[] = {
	"%d", "%u", "%x", "%c", "%s", "%%", "%hd", "%lld", "%.*s", "%03d",
	"%+d", "% d", ""
};

static void
memmove_random(long n)
{
	unsigned char src[ARENA];

	for (long t = 0; t < n; t++) {
		std::size_t len = rnd_mod(64);
		std::size_t slen = rnd_mod(80);
		int mode = (int)(rnd() % 5);

		for (std::size_t i = 0; i < len; i++)
			src[i] = (unsigned char)(rnd() & 0xff);

		if (mode == 0)
			memmove_case(&st_memmove, "rand", src, len, slen, 0, 0);
		else if (mode == 1)
			memmove_case(&st_memmove, "rand_ov", src, len, slen, 0, 2);
		else if (mode == 2)
			memmove_case(&st_memmove, "rand_ovb", src, len, slen, 2,
			    0);
		else if (mode == 3 && slen > 0)
			memmove_case(&st_memmove, "rand_eq", src, len,
			    len == 0 ? 0 : slen - 1 + (rnd() % 2), 0, 0);
		else
			memmove_case(&st_memmove, "rand_b", src, len,
			    len + (rnd() % 3), 0, 0);
	}
}

static void
memcpy_random(long n)
{
	unsigned char src[ARENA];

	for (long t = 0; t < n; t++) {
		std::size_t len = rnd_mod(48);
		std::size_t slen = rnd_mod(64);
		bool overlap = (rnd() & 3) == 0;

		for (std::size_t i = 0; i < len; i++)
			src[i] = (unsigned char)(rnd() & 0xff);

		if ((rnd() & 7) == 0 && slen > 0)
			slen = len + (rnd() % 2) - 1;
		memcpy_case(&st_memcpy, "rand", src, len, slen, overlap);
	}
}

static void
strncpy_random(long n)
{
	char src[ARENA];

	for (long t = 0; t < n; t++) {
		std::size_t len = rnd_mod(48);
		std::size_t slen = rnd_mod(64);
		bool overlap = (rnd() & 3) == 0;

		for (std::size_t i = 0; i < len; i++)
			src[i] = (char)(rnd() & 0xff);
		src[len] = '\0';

		if ((rnd() & 7) == 0 && slen > 0)
			slen = len + (rnd() % 2) - 1;
		strncpy_case(&st_strncpy, "rand", src, len, slen, overlap);
	}
}

static void
vsnprintf_random(long n)
{
	char sbuf[32];

	for (long t = 0; t < n; t++) {
		const char *fmt = vsn_fmts[rnd_mod(sizeof(vsn_fmts) /
		    sizeof(vsn_fmts[0]))];
		std::size_t len = rnd_mod(64);
		std::size_t slen = rnd_mod(72);
		int flags = (int)(rnd() & 0xff);
		int a = (int)rnd();
		int b = (int)rnd();
		long long ll = (long long)rnd();
		short h = (short)rnd();
		char c = (char)(rnd() & 0xff);
		int prec = (int)(rnd_mod(8));
		std::size_t sl = rnd_mod(sizeof(sbuf));

		for (std::size_t i = 0; i < sl; i++)
			sbuf[i] = (char)(rnd() & 0xff);
		sbuf[sl] = '\0';

		if ((rnd() & 7) == 0 && slen > 0)
			slen = len + (rnd() % 2) - 1;

		vsnprintf_case(&st_vsnprintf, "rand", len, flags, slen, fmt,
		    a, b, ll, h, c, prec, sbuf);
	}
}

static void
print_table(void)
{
	Stat *all[] = { &st_memmove, &st_memcpy, &st_vsnprintf, &st_strncpy };

	std::printf("\n%-20s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-20s %12s %12s\n", "--------", "-----", "--------");
	for (Stat *st : all)
		std::printf("%-20s %12ld %12ld\n", st->name, st->cases,
		    st->fails);
}

int
main(void)
{
	memmove_edges();
	memcpy_edges();
	strncpy_edges();
	vsnprintf_edges();

	// memmove_random(SWEEP);
	// memcpy_random(SWEEP);
	// strncpy_random(SWEEP);
	vsnprintf_random(1000);

	print_table();

	long total_fails = st_memmove.fails + st_memcpy.fails +
	    st_vsnprintf.fails + st_strncpy.fails;
	return total_fails == 0 ? 0 : 1;
}
