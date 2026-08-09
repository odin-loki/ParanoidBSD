/*
 * Differential harness for PBSD batch b0315.
 *
 * invtrig.c contributes only const LONGDOUBLE data (no functions).
 * fenv.c is absent; see skipped.txt.
 */

import pbsd.lib.msun.i387.b0315;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace port_ns = pbsd::lib_msun_i387::b0315;

extern "C" {

struct ref_LONGDOUBLE {
	std::uint64_t mant;
	std::uint16_t expsign;
};

extern const ref_LONGDOUBLE ref_pS0, ref_pS1, ref_pS2, ref_pS3, ref_pS4,
    ref_pS5, ref_pS6, ref_qS1, ref_qS2, ref_qS3, ref_qS4, ref_qS5;
extern const ref_LONGDOUBLE ref_atanhi[], ref_atanlo[], ref_aT[];
extern const ref_LONGDOUBLE ref_pi_lo;

extern const std::size_t ref_sizeof_LONGDOUBLE;
extern const std::size_t ref_alignof_LONGDOUBLE;
extern const std::size_t ref_offsetof_mant;
extern const std::size_t ref_offsetof_expsign;
extern const std::size_t ref_atanhi_n;
extern const std::size_t ref_atanlo_n;
extern const std::size_t ref_aT_n;

} /* extern "C" */

using LD = port_ns::LONGDOUBLE;
using ref_LD = ref_LONGDOUBLE;

static constexpr unsigned k_guard = 0x7fu;
static constexpr int k_random_iters = 50000;
static constexpr unsigned k_seed = 0xb0315u;

struct Stats {
	const char *name;
	unsigned cases;
	unsigned failures;
};

static Stats g_stats[32];
static int g_nstats;

static Stats &stat(const char *name)
{
	for (int i = 0; i < g_nstats; ++i) {
		if (std::strcmp(g_stats[i].name, name) == 0)
			return g_stats[i];
	}
	g_stats[g_nstats].name = name;
	g_stats[g_nstats].cases = 0;
	g_stats[g_nstats].failures = 0;
	return g_stats[g_nstats++];
}

static int fail(Stats &s, const char *detail)
{
	++s.failures;
	if (s.failures <= 3)
		std::fprintf(stderr, "  FAIL [%s]: %s\n", s.name, detail);
	return 0;
}

static int eq_ld(const LD &port, const ref_LD &ref, Stats &s, const char *tag)
{
	++s.cases;
	if (port.mant != ref.mant) {
		std::fprintf(stderr,
		    "  FAIL [%s] %s mant: port=0x%016llx ref=0x%016llx\n",
		    s.name, tag, static_cast<unsigned long long>(port.mant),
		    static_cast<unsigned long long>(ref.mant));
		++s.failures;
		return 0;
	}
	if (port.expsign != ref.expsign) {
		std::fprintf(stderr,
		    "  FAIL [%s] %s expsign: port=0x%04x ref=0x%04x\n", s.name,
		    tag, static_cast<unsigned>(port.expsign),
		    static_cast<unsigned>(ref.expsign));
		++s.failures;
		return 0;
	}
	return 1;
}

static int eq_ld_bytes(const LD &port, const ref_LD &ref, Stats &s,
    const char *tag)
{
	++s.cases;
	const auto *pb = reinterpret_cast<const unsigned char *>(&port);
	const auto *rb = reinterpret_cast<const unsigned char *>(&ref);
	for (std::size_t i = 0; i < sizeof(LD); ++i) {
		if (pb[i] != rb[i]) {
			std::fprintf(stderr,
			    "  FAIL [%s] %s byte[%zu]: port=0x%02x ref=0x%02x\n",
			    s.name, tag, i, pb[i], rb[i]);
			++s.failures;
			return 0;
		}
	}
	return 1;
}

static void test_scalar(const char *name, const LD &port, const ref_LD &ref)
{
	Stats &s = stat(name);
	eq_ld(port, ref, s, "value");
	eq_ld_bytes(port, ref, s, "bytes");
}

static void test_array(const char *name, const LD *port, const ref_LD *ref,
    std::size_t n)
{
	Stats &s = stat(name);
	for (std::size_t i = 0; i < n; ++i) {
		char tag[64];
		std::snprintf(tag, sizeof(tag), "idx[%zu]", i);
		eq_ld(port[i], ref[i], s, tag);
		eq_ld_bytes(port[i], ref[i], s, tag);
	}
}

static void test_layout(void)
{
	Stats &s = stat("layout");

	++s.cases;
	if (sizeof(LD) != ref_sizeof_LONGDOUBLE)
		fail(s, "sizeof(LONGDOUBLE) mismatch");

	++s.cases;
	if (alignof(LD) != ref_alignof_LONGDOUBLE)
		fail(s, "alignof(LONGDOUBLE) mismatch");

	++s.cases;
	if (offsetof(LD, mant) != ref_offsetof_mant)
		fail(s, "offsetof(mant) mismatch");

	++s.cases;
	if (offsetof(LD, expsign) != ref_offsetof_expsign)
		fail(s, "offsetof(expsign) mismatch");
}

struct Blob {
	const char *name;
	const LD *port;
	const ref_LD *ref;
	std::size_t count;
};

static const Blob k_blobs[] = {
    {"pS0", &port_ns::pS0, &ref_pS0, 1},
    {"pS1", &port_ns::pS1, &ref_pS1, 1},
    {"pS2", &port_ns::pS2, &ref_pS2, 1},
    {"pS3", &port_ns::pS3, &ref_pS3, 1},
    {"pS4", &port_ns::pS4, &ref_pS4, 1},
    {"pS5", &port_ns::pS5, &ref_pS5, 1},
    {"pS6", &port_ns::pS6, &ref_pS6, 1},
    {"qS1", &port_ns::qS1, &ref_qS1, 1},
    {"qS2", &port_ns::qS2, &ref_qS2, 1},
    {"qS3", &port_ns::qS3, &ref_qS3, 1},
    {"qS4", &port_ns::qS4, &ref_qS4, 1},
    {"qS5", &port_ns::qS5, &ref_qS5, 1},
    {"atanhi", port_ns::atanhi, ref_atanhi, ref_atanhi_n},
    {"atanlo", port_ns::atanlo, ref_atanlo, ref_atanlo_n},
    {"aT", port_ns::aT, ref_aT, ref_aT_n},
    {"pi_lo", &port_ns::pi_lo, &ref_pi_lo, 1},
};

static constexpr std::size_t k_blob_count =
    sizeof(k_blobs) / sizeof(k_blobs[0]);

static void test_edge_cases(void)
{
	test_scalar("pS0", port_ns::pS0, ref_pS0);
	test_scalar("pS1", port_ns::pS1, ref_pS1);
	test_scalar("pS2", port_ns::pS2, ref_pS2);
	test_scalar("pS3", port_ns::pS3, ref_pS3);
	test_scalar("pS4", port_ns::pS4, ref_pS4);
	test_scalar("pS5", port_ns::pS5, ref_pS5);
	test_scalar("pS6", port_ns::pS6, ref_pS6);
	test_scalar("qS1", port_ns::qS1, ref_qS1);
	test_scalar("qS2", port_ns::qS2, ref_qS2);
	test_scalar("qS3", port_ns::qS3, ref_qS3);
	test_scalar("qS4", port_ns::qS4, ref_qS4);
	test_scalar("qS5", port_ns::qS5, ref_qS5);
	test_array("atanhi", port_ns::atanhi, ref_atanhi, ref_atanhi_n);
	test_array("atanlo", port_ns::atanlo, ref_atanlo, ref_atanlo_n);
	test_array("aT", port_ns::aT, ref_aT, ref_aT_n);
	test_scalar("pi_lo", port_ns::pi_lo, ref_pi_lo);
	test_layout();

	/* Boundary indices: first, last, and both fields at extremes. */
	{
		Stats &s = stat("edge_boundaries");
		const std::size_t last_at = ref_aT_n - 1;
		++s.cases;
		if (port_ns::aT[0].mant != ref_aT[0].mant ||
		    port_ns::aT[0].expsign != ref_aT[0].expsign)
			fail(s, "aT[0]");
		++s.cases;
		if (port_ns::aT[last_at].mant != ref_aT[last_at].mant ||
		    port_ns::aT[last_at].expsign != ref_aT[last_at].expsign)
			fail(s, "aT[last]");
		++s.cases;
		if (port_ns::atanhi[ref_atanhi_n - 1].expsign != 0x3fffu)
			fail(s, "atanhi[last].expsign != 0x3fff");
		++s.cases;
		if ((port_ns::pS1.mant >> 63) != 1u)
			fail(s, "pS1 high mant bit");
		++s.cases;
		if ((port_ns::qS1.expsign & 0x8000u) == 0)
			fail(s, "qS1 negative expsign");
	}

	/* Guarded memcpy window: identical padding must stay untouched. */
	{
		Stats &s = stat("guard_window");
		const std::size_t span = sizeof(LD) + 8;
		unsigned char port_buf[sizeof(LD) + 16];
		unsigned char ref_buf[sizeof(LD) + 16];
		std::memset(port_buf, k_guard, sizeof(port_buf));
		std::memset(ref_buf, k_guard, sizeof(ref_buf));
		std::memcpy(port_buf + 4, &port_ns::pi_lo, sizeof(LD));
		std::memcpy(ref_buf + 4, &ref_pi_lo, sizeof(LD));
		++s.cases;
		if (std::memcmp(port_buf, ref_buf, span) != 0)
			fail(s, "guarded window mismatch");
		++s.cases;
		if (port_buf[0] != k_guard || port_buf[span + 3] != k_guard)
			fail(s, "guard bytes clobbered");
	}
}

static void test_random_sweep(void)
{
	Stats &s = stat("random_sweep");
	std::uint32_t state = k_seed;

	for (int iter = 0; iter < k_random_iters; ++iter) {
		state = state * 1664525u + 1013904223u;
		const std::size_t blob_idx = state % k_blob_count;
		const Blob &b = k_blobs[blob_idx];
		const std::size_t elem = (state >> 8) % b.count;
		const std::size_t byte_off = (state >> 16) % sizeof(LD);

		const LD &port = b.port[elem];
		const ref_LD &ref = b.ref[elem];
		const auto *pb =
		    reinterpret_cast<const unsigned char *>(&port);
		const auto *rb =
		    reinterpret_cast<const unsigned char *>(&ref);

		++s.cases;
		if (pb[byte_off] != rb[byte_off]) {
			std::fprintf(stderr,
			    "  FAIL [random_sweep] iter=%d %s[%zu] byte[%zu]: "
			    "port=0x%02x ref=0x%02x\n",
			    iter, b.name, elem, byte_off, pb[byte_off],
			    rb[byte_off]);
			++s.failures;
			if (s.failures > 3)
				return;
		}

		++s.cases;
		if (port.mant != ref.mant || port.expsign != ref.expsign) {
			std::fprintf(stderr,
			    "  FAIL [random_sweep] iter=%d %s[%zu] fields\n",
			    iter, b.name, elem);
			++s.failures;
			if (s.failures > 3)
				return;
		}
	}
}

int main(void)
{
	test_edge_cases();
	test_random_sweep();

	unsigned total_cases = 0;
	unsigned total_failures = 0;

	std::printf("b0315 differential harness\n");
	std::printf("%-20s %10s %10s\n", "function", "cases", "failures");
	for (int i = 0; i < g_nstats; ++i) {
		std::printf("%-20s %10u %10u\n", g_stats[i].name,
		    g_stats[i].cases, g_stats[i].failures);
		total_cases += g_stats[i].cases;
		total_failures += g_stats[i].failures;
	}
	std::printf("%-20s %10u %10u\n", "TOTAL", total_cases,
	    total_failures);

	return total_failures == 0 ? 0 : 1;
}
