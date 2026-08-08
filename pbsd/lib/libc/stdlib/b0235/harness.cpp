/*
 * Differential test for batch b0235 (memalignment, qsort_r, qsort_s,
 * __qsort_r_compat, qsort_b).
 */

#include <cerrno>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifndef RSIZE_MAX
#define RSIZE_MAX (SIZE_MAX >> 1)
#endif

import pbsd.lib.libc.stdlib.b0235;

namespace P = pbsd::lib_libc_stdlib::b0235;

extern "C" {
size_t ref_memalignment(const void *p);
void ref_qsort_r(void *a, size_t n, size_t es,
    int (*cmp)(const void *, const void *, void *), void *thunk);
errno_t ref_qsort_s(void *a, size_t n, size_t es,
    int (*cmp)(const void *, const void *, void *), void *thunk);
void ref___qsort_r_compat(void *a, size_t n, size_t es, void *thunk,
    int (*cmp)(void *, const void *, const void *));
struct qsort_block {
	void *isa;
	int flags;
	int reserved;
	int (*invoke)(void *, ...);
};
void ref_qsort_b(void *base, size_t nel, size_t width, struct qsort_block *compar);
}

namespace {

enum Fn {
	F_MEMALIGNMENT,
	F_QSORT_R,
	F_QSORT_S,
	F_QSORT_R_COMPAT,
	F_QSORT_B,
	F_COUNT
};

static const char *const fn_name[F_COUNT] = {
	"memalignment",
	"qsort_r",
	"qsort_s",
	"__qsort_r_compat",
	"qsort_b",
};

static unsigned long long n_cases[F_COUNT];
static unsigned long long n_fails[F_COUNT];
static unsigned long long reported[F_COUNT];

static const unsigned char GUARD = 0x7f;
static const unsigned SWEEP_ITERS = 200000u;

static void
record_fail(int fn, const char *fmt, ...)
{
	n_fails[fn]++;
	if (reported[fn] < 10) {
		reported[fn]++;
		va_list ap;
		va_start(ap, fmt);
		std::fprintf(stderr, "FAIL %s: ", fn_name[fn]);
		std::vfprintf(stderr, fmt, ap);
		va_end(ap);
		std::fputc('\n', stderr);
	} else if (reported[fn] == 10) {
		reported[fn]++;
		std::fprintf(stderr, "FAIL %s: (further failures suppressed)\n",
		    fn_name[fn]);
	}
}

static void
record_case(int fn, bool ok)
{
	n_cases[fn]++;
	if (!ok)
		n_fails[fn]++;
}

static uint64_t rng_state = 0xc0ffeebaddecaf01ULL;

static uint64_t
nextr(void)
{
	rng_state ^= rng_state << 13;
	rng_state ^= rng_state >> 7;
	rng_state ^= rng_state << 17;
	return rng_state;
}

static unsigned
rnd_below(unsigned n)
{
	return (unsigned)(nextr() % n);
}

static int
cmp_int_r(const void *a, const void *b, void *thunk)
{
	(void)thunk;
	int ia = *(const int *)a;
	int ib = *(const int *)b;
	return (ia > ib) - (ia < ib);
}

static int
cmp_byte_r(const void *a, const void *b, void *thunk)
{
	size_t es = (size_t)(uintptr_t)thunk;
	const unsigned char *pa = (const unsigned char *)a;
	const unsigned char *pb = (const unsigned char *)b;
	for (size_t i = 0; i < es; i++) {
		if (pa[i] != pb[i])
			return (pa[i] > pb[i]) - (pa[i] < pb[i]);
	}
	return 0;
}

static int
cmp_byte_compat(void *thunk, const void *a, const void *b)
{
	return cmp_byte_r(a, b, thunk);
}

static int
block_cmp_int(void *self, ...)
{
	(void)self;
	const void *a;
	const void *b;
	__builtin_va_list ap;
	__builtin_va_start(ap, self);
	a = __builtin_va_arg(ap, const void *);
	b = __builtin_va_arg(ap, const void *);
	__builtin_va_end(ap);
	return cmp_int_r(a, b, nullptr);
}

static int
block_cmp_byte(void *self, ...)
{
	(void)self;
	const void *a;
	const void *b;
	__builtin_va_list ap;
	__builtin_va_start(ap, self);
	a = __builtin_va_arg(ap, const void *);
	b = __builtin_va_arg(ap, const void *);
	__builtin_va_end(ap);
	size_t es = (size_t)(uintptr_t)self;
	return cmp_byte_r(a, b, (void *)es);
}

static void
check_memalignment(const void *p, const char *tag)
{
	size_t got = P::memalignment(p);
	size_t ref = ref_memalignment(p);
	bool ok = (got == ref);
	record_case(F_MEMALIGNMENT, ok);
	if (!ok)
		record_fail(F_MEMALIGNMENT, "%s p=%p got=%zu ref=%zu", tag, p,
		    got, ref);
}

static void
test_memalignment_hand(void)
{
	check_memalignment(nullptr, "null");
	char c;
	check_memalignment(&c, "char");
	short s;
	check_memalignment(&s, "short");
	int i;
	check_memalignment(&i, "int");
	long long ll;
	check_memalignment(&ll, "long long");
	double d;
	check_memalignment(&d, "double");
	void *heap = std::malloc(256);
	if (heap != nullptr) {
		check_memalignment(heap, "malloc");
		check_memalignment((char *)heap + 1, "malloc+1");
		check_memalignment((char *)heap + 7, "malloc+7");
		std::free(heap);
	}
	alignas(64) char big[128];
	check_memalignment(big, "alignas64");
	check_memalignment(big + 63, "alignas64+63");
	for (uintptr_t v = 1; v != 0; v <<= 1)
		check_memalignment((const void *)v, "pow2");
	check_memalignment((const void *)(uintptr_t)0x1001, "0x1001");
	check_memalignment((const void *)(uintptr_t)0x8080, "0x8080");
}

static void
test_memalignment_sweep(void)
{
	for (unsigned t = 0; t < SWEEP_ITERS / 5; t++) {
		uintptr_t v = (uintptr_t)nextr();
		if ((t & 7u) == 0)
			v &= ~(uintptr_t)15;
		if ((t & 7u) == 1)
			v |= 1;
		if ((t & 7u) == 2)
			v |= 0x80;
		check_memalignment((const void *)v, "sweep");
	}
	char pool[4096];
	for (unsigned t = 0; t < SWEEP_ITERS / 5; t++) {
		unsigned off = rnd_below((unsigned)sizeof(pool));
		check_memalignment(pool + off, "pool");
	}
}

struct SortBuf {
	unsigned char *a;
	unsigned char *b;
	size_t cap;
	size_t es;
	size_t n;
};

static SortBuf
make_sortbuf(size_t n, size_t es)
{
	size_t data = n * es;
	size_t cap = data + 64;
	SortBuf sb{};
	sb.cap = cap;
	sb.es = es;
	sb.n = n;
	sb.a = (unsigned char *)std::malloc(cap);
	sb.b = (unsigned char *)std::malloc(cap);
	if (sb.a == nullptr || sb.b == nullptr)
		std::abort();
	std::memset(sb.a, GUARD, cap);
	std::memset(sb.b, GUARD, cap);
	std::memset(sb.a + 32, 0, data);
	std::memset(sb.b + 32, 0, data);
	return sb;
}

static void
free_sortbuf(SortBuf &sb)
{
	std::free(sb.a);
	std::free(sb.b);
}

static void
fill_sortbuf(SortBuf &sb, unsigned seed)
{
	size_t data = sb.n * sb.es;
	for (size_t i = 0; i < data; i++)
		sb.a[32 + i] = sb.b[32 + i] =
		    (unsigned char)((seed + i * 17u + 3u) & 0xffu);
}

static bool
bufs_equal(const SortBuf &sb)
{
	return std::memcmp(sb.a, sb.b, sb.cap) == 0;
}

static void
check_qsort_r_bytes(size_t n, size_t es, unsigned seed, const char *tag)
{
	SortBuf sb = make_sortbuf(n, es);
	fill_sortbuf(sb, seed);
	void *thunk = (void *)(uintptr_t)es;

	P::qsort_r(sb.a + 32, n, es, cmp_byte_r, thunk);
	ref_qsort_r(sb.b + 32, n, es, cmp_byte_r, thunk);

	bool ok = bufs_equal(sb);
	record_case(F_QSORT_R, ok);
	if (!ok)
		record_fail(F_QSORT_R, "%s n=%zu es=%zu seed=%u", tag, n, es, seed);
	free_sortbuf(sb);
}

static void
check_qsort_compat_case(size_t n, size_t es, unsigned seed, const char *tag)
{
	SortBuf sb = make_sortbuf(n, es);
	fill_sortbuf(sb, seed);
	void *thunk = (void *)(uintptr_t)es;

	P::__qsort_r_compat(sb.a + 32, n, es, thunk, cmp_byte_compat);
	ref___qsort_r_compat(sb.b + 32, n, es, thunk, cmp_byte_compat);

	bool ok = bufs_equal(sb);
	record_case(F_QSORT_R_COMPAT, ok);
	if (!ok)
		record_fail(F_QSORT_R_COMPAT, "%s n=%zu es=%zu seed=%u", tag, n, es,
		    seed);
	free_sortbuf(sb);
}

static void
check_qsort_b_case(size_t n, size_t es, unsigned seed, const char *tag)
{
	SortBuf sb = make_sortbuf(n, es);
	fill_sortbuf(sb, seed);

	struct qsort_block blk{};
	blk.invoke = (es == sizeof(int)) ? block_cmp_int : block_cmp_byte;
	blk.isa = (void *)(uintptr_t)es;

	P::qsort_b(sb.a + 32, n, es, (decltype(blk) *)&blk);
	ref_qsort_b(sb.b + 32, n, es, &blk);

	bool ok = bufs_equal(sb);
	record_case(F_QSORT_B, ok);
	if (!ok)
		record_fail(F_QSORT_B, "%s n=%zu es=%zu seed=%u", tag, n, es, seed);
	free_sortbuf(sb);
}

static void
check_qsort_s_case(size_t n, size_t es, void *base_a, void *base_b,
    int (*cmp)(const void *, const void *, void *), void *thunk,
    const char *tag)
{
	errno_t got = P::qsort_s(base_a, n, es, cmp, thunk);
	errno_t ref = ref_qsort_s(base_b, n, es, cmp, thunk);
	bool ok = (got == ref);
	record_case(F_QSORT_S, ok);
	if (!ok)
		record_fail(F_QSORT_S, "%s n=%zu es=%zu got=%d ref=%d", tag,
		    (size_t)n, (size_t)es, (int)got, (int)ref);
}

static void
check_qsort_s_sort(size_t n, size_t es, unsigned seed, const char *tag)
{
	SortBuf sb = make_sortbuf(n, es);
	fill_sortbuf(sb, seed);
	void *thunk = (void *)(uintptr_t)es;

	errno_t got = P::qsort_s(sb.a + 32, n, es, cmp_byte_r, thunk);
	errno_t ref = ref_qsort_s(sb.b + 32, n, es, cmp_byte_r, thunk);
	bool ok = (got == ref) && (got == 0) && bufs_equal(sb);
	record_case(F_QSORT_S, ok);
	if (!ok)
		record_fail(F_QSORT_S, "%s n=%zu es=%zu seed=%u ret=%d/%d", tag, n,
		    es, seed, (int)got, (int)ref);
	free_sortbuf(sb);
}

static void
test_qsort_hand(void)
{
	static const size_t ns[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 15, 16, 39, 40,
	    41, 42, 63, 64, 100, 200, 500};
	static const size_t ess[] = {1, 2, 4, 8, 16};

	for (size_t n : ns) {
		for (size_t es : ess) {
			check_qsort_r_bytes(n, es, 0x80, "hand-byte");
			check_qsort_compat_case(n, es, 0xff, "hand-compat");
			check_qsort_b_case(n, es, 0x55, "hand-block");
			check_qsort_s_sort(n, es, 0x33, "hand-sort");
		}
	}

	for (size_t n : {2u, 7u, 8u, 40u, 41u, 100u}) {
		SortBuf sb = make_sortbuf(n, sizeof(int));
		for (size_t i = 0; i < n; i++) {
			int v = (int)((i % 3) - 1);
			std::memcpy(sb.a + 32 + i * sizeof(int), &v, sizeof(int));
			std::memcpy(sb.b + 32 + i * sizeof(int), &v, sizeof(int));
		}
		P::qsort_r(sb.a + 32, n, sizeof(int), cmp_int_r, nullptr);
		ref_qsort_r(sb.b + 32, n, sizeof(int), cmp_int_r, nullptr);
		bool ok = bufs_equal(sb);
		record_case(F_QSORT_R, ok);
		if (!ok)
			record_fail(F_QSORT_R, "dupes n=%zu", n);
		free_sortbuf(sb);
	}

	SortBuf sb0 = make_sortbuf(4, 4);
	check_qsort_s_case(RSIZE_MAX + 1, 4, sb0.a + 32, sb0.b + 32, cmp_byte_r,
	    (void *)(uintptr_t)4, "n>RSIZE_MAX");
	check_qsort_s_case(4, RSIZE_MAX + 1, sb0.a + 32, sb0.b + 32, cmp_byte_r,
	    (void *)(uintptr_t)4, "es>RSIZE_MAX");
	check_qsort_s_case(4, 4, nullptr, nullptr, cmp_byte_r,
	    (void *)(uintptr_t)4, "null-base");
	check_qsort_s_case(4, 4, sb0.a + 32, sb0.b + 32, nullptr,
	    (void *)(uintptr_t)4, "null-cmp");
	check_qsort_s_case(4, 0, sb0.a + 32, sb0.b + 32, cmp_byte_r,
	    (void *)(uintptr_t)4, "es=0");
	check_qsort_s_case(4, (size_t)-1, sb0.a + 32, sb0.b + 32, cmp_byte_r,
	    (void *)(uintptr_t)4, "es<0");
	check_qsort_s_case(0, 4, nullptr, nullptr, nullptr, nullptr, "n=0-nulls");
	free_sortbuf(sb0);
}

static void
test_qsort_sweep(void)
{
	const unsigned per_fn = SWEEP_ITERS / 4;
	for (unsigned t = 0; t < per_fn; t++) {
		size_t n = (size_t)(nextr() % 256u);
		size_t es = 1u << (nextr() % 5u);
		unsigned seed = (unsigned)nextr();
		check_qsort_r_bytes(n, es, seed, "sweep");
	}
	for (unsigned t = 0; t < per_fn; t++) {
		size_t n = (size_t)(nextr() % 256u);
		size_t es = 1u << (nextr() % 5u);
		unsigned seed = (unsigned)nextr();
		check_qsort_compat_case(n, es, seed, "sweep");
	}
	for (unsigned t = 0; t < per_fn; t++) {
		size_t n = (size_t)(nextr() % 256u);
		size_t es = (size_t)(1u << (nextr() % 2u ? 2u : 0u));
		unsigned seed = (unsigned)nextr();
		check_qsort_b_case(n, es, seed, "sweep");
	}
	for (unsigned t = 0; t < per_fn; t++) {
		size_t n = (size_t)(nextr() % 128u);
		size_t es = 1u << (nextr() % 4u);
		unsigned seed = (unsigned)nextr();
		check_qsort_s_sort(n, es, seed, "sweep");
	}
}

} // namespace

int
main(void)
{
	test_memalignment_hand();
	test_memalignment_sweep();
	test_qsort_hand();
	test_qsort_sweep();

	std::printf("function          cases     fails\n");
	std::printf("----------------  --------  ------\n");
	unsigned long long total_cases = 0;
	unsigned long long total_fails = 0;
	for (int i = 0; i < F_COUNT; i++) {
		std::printf("%-16s  %8llu  %6llu\n", fn_name[i], n_cases[i],
		    n_fails[i]);
		total_cases += n_cases[i];
		total_fails += n_fails[i];
	}
	std::printf("%-16s  %8llu  %6llu\n", "TOTAL", total_cases, total_fails);
	return total_fails == 0 ? 0 : 1;
}
