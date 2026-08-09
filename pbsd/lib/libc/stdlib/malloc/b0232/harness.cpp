/*
 * Differential harness for batch b0232 (__free_sized, __free_aligned_sized).
 */

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.stdlib.malloc.b0232;

namespace P = pbsd::lib_libc_stdlib_malloc::b0232;

extern "C" {
void ref___free_sized(void *ptr, size_t size);
void ref___free_aligned_sized(void *ptr, size_t alignment, size_t size);
void pbsd_b0232_mock_reset(void);
void *pbsd_b0232_mock_last_ptr(void);
unsigned pbsd_b0232_mock_call_count(void);
}

namespace {

enum Fn {
	F_FREE_SIZED,
	F_FREE_ALIGNED_SIZED,
	F_COUNT
};

static const char *const fn_name[F_COUNT] = {
	"__free_sized",
	"__free_aligned_sized",
};

static unsigned long long n_cases[F_COUNT];
static unsigned long long n_fails[F_COUNT];
static unsigned long long reported[F_COUNT];

static constexpr unsigned SWEEP_ITERS = 200000u;

static void
record_case(int fn, bool ok, const char *fmt, ...)
{
	n_cases[fn]++;
	if (ok)
		return;
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

static uint64_t rng_state = 0xb0232c0ffeeULL;

static uint64_t
nextr(void)
{
	rng_state ^= rng_state << 13;
	rng_state ^= rng_state >> 7;
	rng_state ^= rng_state << 17;
	return (rng_state);
}

static size_t
rnd_size(void)
{
	uint64_t v = nextr();
	if ((v & 7u) == 0u)
		return (0);
	if ((v & 7u) == 1u)
		return (1);
	if ((v & 7u) == 2u)
		return (SIZE_MAX);
	if ((v & 7u) == 3u)
		return (SIZE_MAX - 1);
	if ((v & 7u) == 4u)
		return ((size_t)1 << ((unsigned)(v >> 3) % (sizeof(size_t) * 8)));
	return ((size_t)(v ^ (v >> 32)));
}

static size_t
rnd_align(void)
{
	static const size_t tab[] = {
		0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 4096, 8192,
		SIZE_MAX / 2, SIZE_MAX - 1, SIZE_MAX
	};
	return (tab[nextr() % (sizeof(tab) / sizeof(tab[0]))]);
}

static void *
alloc_block(size_t sz)
{
	if (sz == 0)
		sz = 1;
	void *p = std::malloc(sz);
	if (p == NULL)
		std::abort();
	std::memset(p, 0xa5, sz);
	return (p);
}

static bool
ptrs_match(void *expected, void *actual)
{
	if (expected == actual)
		return (true);
	if (expected == NULL || actual == NULL)
		return (false);
	return (false);
}

static void
check_free_sized_pair(void *port_arg, void *ref_arg, size_t size,
    const char *tag)
{
	pbsd_b0232_mock_reset();
	P::__free_sized(port_arg, size);
	void *port_ptr = pbsd_b0232_mock_last_ptr();
	unsigned port_calls = pbsd_b0232_mock_call_count();

	pbsd_b0232_mock_reset();
	ref___free_sized(ref_arg, size);
	void *ref_ptr = pbsd_b0232_mock_last_ptr();
	unsigned ref_calls = pbsd_b0232_mock_call_count();

	bool ok = port_calls == ref_calls && port_calls == 1u &&
	    ptrs_match(port_arg, port_ptr) && ptrs_match(ref_arg, ref_ptr) &&
	    ((port_arg == NULL && ref_arg == NULL) || (port_arg != NULL && ref_arg != NULL));
	record_case(F_FREE_SIZED, ok,
	    "%s port_arg=%p ref_arg=%p size=%zu port_calls=%u ref_calls=%u port_last=%p ref_last=%p",
	    tag, port_arg, ref_arg, size, port_calls, ref_calls, port_ptr, ref_ptr);
}

static void
check_free_aligned_sized_pair(void *port_arg, void *ref_arg, size_t alignment,
    size_t size, const char *tag)
{
	pbsd_b0232_mock_reset();
	P::__free_aligned_sized(port_arg, alignment, size);
	void *port_ptr = pbsd_b0232_mock_last_ptr();
	unsigned port_calls = pbsd_b0232_mock_call_count();

	pbsd_b0232_mock_reset();
	ref___free_aligned_sized(ref_arg, alignment, size);
	void *ref_ptr = pbsd_b0232_mock_last_ptr();
	unsigned ref_calls = pbsd_b0232_mock_call_count();

	bool ok = port_calls == ref_calls && port_calls == 1u &&
	    ptrs_match(port_arg, port_ptr) && ptrs_match(ref_arg, ref_ptr) &&
	    ((port_arg == NULL && ref_arg == NULL) || (port_arg != NULL && ref_arg != NULL));
	record_case(F_FREE_ALIGNED_SIZED, ok,
	    "%s port_arg=%p ref_arg=%p align=%zu size=%zu port_calls=%u ref_calls=%u port_last=%p ref_last=%p",
	    tag, port_arg, ref_arg, alignment, size, port_calls, ref_calls,
	    port_ptr, ref_ptr);
}

static void
check_free_sized(size_t alloc_sz, size_t size, const char *tag)
{
	if (alloc_sz == 0) {
		check_free_sized_pair(NULL, NULL, size, tag);
		return;
	}
	void *port_arg = alloc_block(alloc_sz);
	void *ref_arg = alloc_block(alloc_sz);
	check_free_sized_pair(port_arg, ref_arg, size, tag);
}

static void
check_free_aligned_sized(size_t alloc_sz, size_t alignment, size_t size,
    const char *tag)
{
	if (alloc_sz == 0) {
		check_free_aligned_sized_pair(NULL, NULL, alignment, size, tag);
		return;
	}
	void *port_arg = alloc_block(alloc_sz);
	void *ref_arg = alloc_block(alloc_sz);
	check_free_aligned_sized_pair(port_arg, ref_arg, alignment, size, tag);
}

static void
edge_cases(void)
{
	check_free_sized(0, 0, "null-size0");
	check_free_sized(0, 1, "null-size1");
	check_free_sized(0, SIZE_MAX, "null-sizemax");

	check_free_sized(1, 0, "one-size0");
	check_free_sized(1, 1, "one-size1");
	check_free_sized(1, SIZE_MAX, "one-sizemax");

	check_free_sized(16, 16, "small-match");
	check_free_sized(256, 1, "large-size1");

	check_free_aligned_sized(0, 0, 0, "null-0-0");
	check_free_aligned_sized(0, 1, 0, "null-1-0");
	check_free_aligned_sized(0, 0, 1, "null-0-1");
	check_free_aligned_sized(0, 8, SIZE_MAX, "null-8-max");
	check_free_aligned_sized(0, SIZE_MAX, SIZE_MAX, "null-max-max");

	check_free_aligned_sized(64, 0, 0, "blk-0-0");
	check_free_aligned_sized(64, 8, 64, "blk-8-64");
	check_free_aligned_sized(128, 16, 1, "blk-16-1");
	check_free_aligned_sized(4096, 4096, 4096, "blk-4096-4096");
	check_free_aligned_sized(32, SIZE_MAX, SIZE_MAX, "blk-max-max");
}

static void
random_sweep(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++) {
		size_t size = rnd_size();
		size_t align = rnd_align();
		bool use_null = (nextr() & 15u) == 0u;
		size_t alloc_sz = use_null ? 0 : (size % 4096u) + 1u;

		check_free_sized(alloc_sz, size, "sweep");
		alloc_sz = use_null ? 0 : (align % 2048u) + 1u;
		check_free_aligned_sized(alloc_sz, align, size, "sweep");
	}
}

} /* namespace */

int
main(void)
{
	edge_cases();
	random_sweep();

	unsigned long long total_cases = 0;
	unsigned long long total_fails = 0;

	std::printf("function                 cases     fails\n");
	std::printf("----------------------------------------\n");
	for (int i = 0; i < F_COUNT; i++) {
		std::printf("%-24s %7llu %7llu\n", fn_name[i], n_cases[i],
		    n_fails[i]);
		total_cases += n_cases[i];
		total_fails += n_fails[i];
	}
	std::printf("----------------------------------------\n");
	std::printf("%-24s %7llu %7llu\n", "TOTAL", total_cases, total_fails);

	return (total_fails == 0 ? 0 : 1);
}
