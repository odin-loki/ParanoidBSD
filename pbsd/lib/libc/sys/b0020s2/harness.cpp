/*
 * Differential test harness for PBSD batch b0020s2.
 *
 * Compares pbsd::lib_libc_sys::b0020s2::closefrom() against the unmodified
 * oracle ref_closefrom() from oracle.c.
 *
 * closefrom() returns void; its entire observable behaviour is the argument
 * vector it hands to the raw close_range(2) stub.  This harness therefore
 * supplies __sys_close_range() itself and has it record into a guarded byte
 * region: two regions are filled with the guard byte 0x7f, the port writes
 * into one and the oracle into the other, and the ENTIRE region (including the
 * bytes before and after the nominal record window) is compared afterwards.
 */

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <climits>
#include <cstddef>

import pbsd.lib.libc.sys.b0020s2;

extern "C" void ref_closefrom(int lowfd);

namespace {

constexpr std::size_t kRegionSize = 64;
constexpr unsigned char kGuard = 0x7f;

/* Record window inside the region; everything outside must stay kGuard. */
constexpr std::size_t kOffLowfd = 4;
constexpr std::size_t kOffHighfd = 8;
constexpr std::size_t kOffFlags = 12;
constexpr std::size_t kOffCalls = 16;
constexpr std::size_t kWindowEnd = 20;

unsigned char g_port_region[kRegionSize];
unsigned char g_ref_region[kRegionSize];
unsigned char *g_active = nullptr;

void
region_reset(unsigned char *r)
{
	std::memset(r, kGuard, kRegionSize);
	std::uint32_t zero = 0;
	std::memcpy(r + kOffCalls, &zero, sizeof(zero));
}

std::uint32_t
region_calls(const unsigned char *r)
{
	std::uint32_t v;
	std::memcpy(&v, r + kOffCalls, sizeof(v));
	return v;
}

std::uint32_t
region_u32(const unsigned char *r, std::size_t off)
{
	std::uint32_t v;
	std::memcpy(&v, r + off, sizeof(v));
	return v;
}

std::int32_t
region_i32(const unsigned char *r, std::size_t off)
{
	std::int32_t v;
	std::memcpy(&v, r + off, sizeof(v));
	return v;
}

bool
region_guards_intact(const unsigned char *r)
{
	for (std::size_t i = 0; i < kOffLowfd; i++) {
		if (r[i] != kGuard)
			return false;
	}
	for (std::size_t i = kWindowEnd; i < kRegionSize; i++) {
		if (r[i] != kGuard)
			return false;
	}
	return true;
}

void
region_dump(const char *tag, const unsigned char *r)
{
	std::printf("    %s:", tag);
	for (std::size_t i = 0; i < kRegionSize; i++)
		std::printf(" %02x", r[i]);
	std::printf("\n");
	std::printf("      calls=%u lowfd=%u (0x%08x) highfd=%u (0x%08x) "
	    "flags=%d\n", region_calls(r), region_u32(r, kOffLowfd),
	    region_u32(r, kOffLowfd), region_u32(r, kOffHighfd),
	    region_u32(r, kOffHighfd), region_i32(r, kOffFlags));
}

struct Stats {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
};

Stats g_closefrom = { "closefrom", 0, 0 };

unsigned long long g_reported = 0;
constexpr unsigned long long kMaxReports = 20;

/* splitmix64: deterministic, fixed seed. */
class Rng {
public:
	explicit Rng(std::uint64_t seed) : state_(seed) {}

	std::uint64_t
	next()
	{
		state_ += 0x9e3779b97f4a7c15ULL;
		std::uint64_t z = state_;
		z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
		z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
		return z ^ (z >> 31);
	}

private:
	std::uint64_t state_;
};

bool
run_case(int lowfd, const char *origin)
{
	region_reset(g_port_region);
	g_active = g_port_region;
	pbsd::lib_libc_sys::b0020s2::closefrom(lowfd);

	region_reset(g_ref_region);
	g_active = g_ref_region;
	ref_closefrom(lowfd);

	g_active = nullptr;
	g_closefrom.cases++;

	bool ok = std::memcmp(g_port_region, g_ref_region, kRegionSize) == 0;

	/*
	 * Non-vacuity: the oracle must actually have reached the stub, and it
	 * must not have scribbled outside the record window.
	 */
	if (region_calls(g_ref_region) != 1)
		ok = false;
	if (!region_guards_intact(g_ref_region))
		ok = false;
	if (!region_guards_intact(g_port_region))
		ok = false;

	if (!ok) {
		g_closefrom.failures++;
		if (g_reported < kMaxReports) {
			g_reported++;
			std::printf("FAIL closefrom [%s] lowfd=%d (0x%08x)\n",
			    origin, lowfd,
			    static_cast<unsigned int>(lowfd));
			region_dump("port", g_port_region);
			region_dump("ref ", g_ref_region);
		}
	}
	return ok;
}

} // namespace

extern "C" int
__sys_close_range(unsigned int lowfd, unsigned int highfd, int flags)
{
	if (g_active == nullptr) {
		std::printf("FATAL: __sys_close_range called with no active "
		    "region\n");
		return -1;
	}
	std::uint32_t lo = lowfd;
	std::uint32_t hi = highfd;
	std::int32_t fl = flags;
	std::memcpy(g_active + kOffLowfd, &lo, sizeof(lo));
	std::memcpy(g_active + kOffHighfd, &hi, sizeof(hi));
	std::memcpy(g_active + kOffFlags, &fl, sizeof(fl));
	std::uint32_t calls = region_calls(g_active);
	calls++;
	std::memcpy(g_active + kOffCalls, &calls, sizeof(calls));
	return 0;
}

int
main()
{
	/*
	 * Hand-written edge cases.  Both sides of the MAX(0, lowfd) boundary
	 * are covered (negative, zero, positive), plus the extremes of int,
	 * plus values whose low byte covers 0x00..0xff including the high-bit
	 * range 0x80..0xff and their negations.
	 */
	static const int edge[] = {
		INT_MIN, INT_MIN + 1, INT_MIN + 2,
		-2147483647, -1073741824, -65537, -65536, -65535,
		-32769, -32768, -32767, -1024, -1023, -256, -255, -129,
		-128, -127, -3, -2, -1,
		0, 1, 2, 3, 4, 7, 8,
		126, 127, 128, 129, 130, 254, 255, 256, 257,
		1023, 1024, 1025, 32766, 32767, 32768, 32769,
		65534, 65535, 65536, 65537,
		1073741823, 1073741824,
		INT_MAX - 2, INT_MAX - 1, INT_MAX,
	};

	for (std::size_t i = 0; i < sizeof(edge) / sizeof(edge[0]); i++)
		run_case(edge[i], "edge");

	/* Every single byte value, positive and negated. */
	for (int b = 0; b <= 255; b++) {
		run_case(b, "byte");
		run_case(-b, "byte-neg");
		run_case(static_cast<int>(static_cast<unsigned int>(b) << 24),
		    "byte-hi");
	}

	/* Dense sweep across the boundary in both directions. */
	for (int v = -4096; v <= 4096; v++)
		run_case(v, "dense");

	/* Fixed-seed randomised sweep. */
	Rng rng(0x0020520000205200ULL);
	const unsigned long long kIters = 250000;
	for (unsigned long long i = 0; i < kIters; i++) {
		std::uint64_t r = rng.next();
		int lowfd;
		switch (static_cast<unsigned>(r & 7u)) {
		case 0:
			/* full 32-bit range */
			lowfd = static_cast<int>(
			    static_cast<std::uint32_t>(r >> 32));
			break;
		case 1:
			/* tight around zero */
			lowfd = static_cast<int>(
			    static_cast<std::uint32_t>(r >> 32) % 9u) - 4;
			break;
		case 2:
			/* small negatives */
			lowfd = -static_cast<int>(
			    static_cast<std::uint32_t>(r >> 32) % 1024u);
			break;
		case 3:
			/* small positives */
			lowfd = static_cast<int>(
			    static_cast<std::uint32_t>(r >> 32) % 1024u);
			break;
		case 4:
			/* near INT_MAX */
			lowfd = INT_MAX - static_cast<int>(
			    static_cast<std::uint32_t>(r >> 32) % 64u);
			break;
		case 5:
			/* near INT_MIN */
			lowfd = INT_MIN + static_cast<int>(
			    static_cast<std::uint32_t>(r >> 32) % 64u);
			break;
		case 6:
			/* single low byte, includes 0x80..0xff */
			lowfd = static_cast<int>(
			    static_cast<std::uint32_t>(r >> 32) & 0xffu);
			break;
		default:
			/* sign-flipped full range */
			lowfd = -static_cast<int>(
			    static_cast<std::uint32_t>(r >> 32) >> 1);
			break;
		}
		run_case(lowfd, "random");
	}

	std::printf("\n%-24s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-24s %12llu %12llu\n", g_closefrom.name,
	    g_closefrom.cases, g_closefrom.failures);

	unsigned long long total_cases = g_closefrom.cases;
	unsigned long long total_failures = g_closefrom.failures;
	std::printf("%-24s %12llu %12llu\n", "TOTAL", total_cases,
	    total_failures);

	if (total_failures != 0) {
		std::printf("\nRESULT: FAIL\n");
		return 1;
	}
	std::printf("\nRESULT: PASS\n");
	return 0;
}
