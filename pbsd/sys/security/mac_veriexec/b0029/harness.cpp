/*
 * harness.cpp -- differential test for PBSD batch b0029.
 *
 * Batch b0029's only portable function is SHA1_Final() from
 * mac_veriexec_sha1.c.  The other three sources define no functions
 * (see skipped.txt).  SHA1_Final writes SHA1_RESULTLEN bytes through a
 * void* context that must already have been fed via sha1_init/sha1_loop;
 * both digest buffers and both context arenas are surrounded by 0x7f guard
 * bytes and compared in full after every call.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <new>
#include <random>
#include <vector>

import pbsd.sys.security.mac.veriexec.b0029;

namespace port = pbsd::sys_security_mac_veriexec::b0029;

extern "C" {
typedef char *caddr_t;

struct sha1_ctxt {
	union {
		std::uint8_t	b8[20];
		std::uint32_t	b32[5];
	} h;
	union {
		std::uint8_t	b8[8];
		std::uint64_t	b64[1];
	} c;
	union {
		std::uint8_t	b8[64];
		std::uint32_t	b32[16];
	} m;
	std::uint8_t	count;
};
typedef struct sha1_ctxt SHA1_CTX;

#define	SHA1_RESULTLEN	(160 / 8)

void sha1_init(struct sha1_ctxt *);
void sha1_loop(struct sha1_ctxt *, const std::uint8_t *, std::size_t);
void ref_SHA1_Final(unsigned char *buf, void *ctx);
}

/*
 * SHA1_Final casts its void* ctx to SHA1_CTX and hands it to sha1_result,
 * so the port's struct must be ABI-identical to the oracle's.  The port
 * never reads the fields itself, which means a wrong array bound is
 * invisible at runtime (padding hides most size changes).  Assert the
 * layout member by member instead, so any divergence is rejected here.
 */
namespace layout {

using P = port::SHA1_CTX;
using O = SHA1_CTX;

static_assert(sizeof(P) == sizeof(O), "SHA1_CTX size");
static_assert(alignof(P) == alignof(O), "SHA1_CTX alignment");

static_assert(offsetof(P, h) == offsetof(O, h), "SHA1_CTX::h offset");
static_assert(offsetof(P, c) == offsetof(O, c), "SHA1_CTX::c offset");
static_assert(offsetof(P, m) == offsetof(O, m), "SHA1_CTX::m offset");
static_assert(offsetof(P, count) == offsetof(O, count),
    "SHA1_CTX::count offset");

static_assert(sizeof(P::h) == sizeof(O::h), "SHA1_CTX::h size");
static_assert(sizeof(P::c) == sizeof(O::c), "SHA1_CTX::c size");
static_assert(sizeof(P::m) == sizeof(O::m), "SHA1_CTX::m size");
static_assert(sizeof(P::count) == sizeof(O::count), "SHA1_CTX::count size");

static_assert(sizeof(decltype(P::h)::b8) == sizeof(decltype(O::h)::b8),
    "SHA1_CTX::h.b8 bound");
static_assert(sizeof(decltype(P::h)::b32) == sizeof(decltype(O::h)::b32),
    "SHA1_CTX::h.b32 bound");
static_assert(sizeof(decltype(P::c)::b8) == sizeof(decltype(O::c)::b8),
    "SHA1_CTX::c.b8 bound");
static_assert(sizeof(decltype(P::c)::b64) == sizeof(decltype(O::c)::b64),
    "SHA1_CTX::c.b64 bound");
static_assert(sizeof(decltype(P::m)::b8) == sizeof(decltype(O::m)::b8),
    "SHA1_CTX::m.b8 bound");
static_assert(sizeof(decltype(P::m)::b32) == sizeof(decltype(O::m)::b32),
    "SHA1_CTX::m.b32 bound");

static_assert(sizeof(decltype(P::h)::b8[0]) == sizeof(decltype(O::h)::b8[0]),
    "SHA1_CTX::h.b8 element width");
static_assert(sizeof(decltype(P::h)::b32[0]) == sizeof(decltype(O::h)::b32[0]),
    "SHA1_CTX::h.b32 element width");
static_assert(sizeof(decltype(P::c)::b64[0]) == sizeof(decltype(O::c)::b64[0]),
    "SHA1_CTX::c.b64 element width");
static_assert(sizeof(decltype(P::m)::b32[0]) == sizeof(decltype(O::m)::b32[0]),
    "SHA1_CTX::m.b32 element width");

} /* namespace layout */

namespace {

constexpr std::size_t DIGEST = SHA1_RESULTLEN;
constexpr std::size_t GUARD = 16;
constexpr std::size_t CTX_ARENA = 128;
constexpr std::size_t BUF_ARENA = GUARD + DIGEST + GUARD;
constexpr unsigned char GUARD_BYTE = 0x7f;

struct Stats {
	const char	*name;
	unsigned long	 cases;
	unsigned long	 failures;
};

Stats g_sha1_final = { "SHA1_Final", 0, 0 };

unsigned long g_reported = 0;

void
report(const char *phase, std::size_t len, const char *what)
{
	if (g_reported >= 20) {
		if (g_reported == 20)
			std::printf("  ... further failures suppressed\n");
		g_reported++;
		return;
	}
	g_reported++;
	std::printf("  FAIL [%s] len=%zu: %s\n", phase, len, what);
}

void
run_case(const char *phase, const std::uint8_t *data, std::size_t len)
{
	alignas(16) unsigned char pbuf[BUF_ARENA];
	alignas(16) unsigned char obuf[BUF_ARENA];
	alignas(16) unsigned char pctxa[CTX_ARENA];
	alignas(16) unsigned char octxa[CTX_ARENA];

	std::memset(pbuf, GUARD_BYTE, BUF_ARENA);
	std::memset(obuf, GUARD_BYTE, BUF_ARENA);
	std::memset(pctxa, GUARD_BYTE, CTX_ARENA);
	std::memset(octxa, GUARD_BYTE, CTX_ARENA);

	SHA1_CTX *oc = new (octxa + GUARD) SHA1_CTX;
	port::SHA1_CTX *pc = new (pctxa + GUARD) port::SHA1_CTX;

	sha1_init(oc);
	sha1_init(reinterpret_cast<SHA1_CTX *>(pc));
	if (len > 0) {
		sha1_loop(oc, data, len);
		sha1_loop(reinterpret_cast<SHA1_CTX *>(pc), data, len);
	}

	unsigned char *pdigest = pbuf + GUARD;
	unsigned char *odigest = obuf + GUARD;

	port::SHA1_Final(pdigest, pc);
	ref_SHA1_Final(odigest, oc);

	g_sha1_final.cases++;
	bool bad = false;

	if (std::memcmp(pbuf, obuf, BUF_ARENA) != 0) {
		bad = true;
		report(phase, len, "digest arena bytes");
	}
	if (std::memcmp(pctxa, octxa, CTX_ARENA) != 0) {
		bad = true;
		report(phase, len, "context arena bytes");
	}

	if (bad)
		g_sha1_final.failures++;
}

void
run_bytes(const char *phase, const std::vector<std::uint8_t> &v)
{
	run_case(phase, v.data(), v.size());
}

std::vector<std::uint8_t>
make_pattern(std::size_t len, unsigned seed)
{
	std::vector<std::uint8_t> v(len);
	for (std::size_t i = 0; i < len; i++)
		v[i] = static_cast<std::uint8_t>((seed + i * 17u) & 0xffu);
	return v;
}

std::size_t
pick_len(std::mt19937_64 &rng)
{
	std::uint64_t r = rng();
	switch (r % 16u) {
	case 0: return 0;
	case 1: return 1;
	case 2: return 55;
	case 3: return 56;
	case 4: return 57;
	case 5: return 63;
	case 6: return 64;
	case 7: return 65;
	case 8: return 127;
	case 9: return 128;
	case 10: return 129;
	case 11: return 1023;
	case 12: return 1024;
	case 13: return 1025;
	case 14: return 4095;
	default: return static_cast<std::size_t>(r >> 8) % 8192u;
	}
}

std::uint8_t
pick_byte(std::mt19937_64 &rng)
{
	std::uint64_t r = rng();
	switch (r % 8u) {
	case 0: return 0x00;
	case 1: return 0x01;
	case 2: return 0x7f;
	case 3: return 0x80;
	case 4: return 0xfe;
	case 5: return 0xff;
	case 6: return static_cast<std::uint8_t>(r >> 8);
	default: return static_cast<std::uint8_t>(r >> 16);
	}
}

} /* anonymous namespace */

int
main(void)
{
	std::printf("PBSD batch b0029 differential test\n");
	std::printf("(oracle.c is the specification)\n\n");

	/* Hand-written edge cases. */
	run_bytes("empty", {});
	run_bytes("single-nul", {0x00});
	run_bytes("single-0x80", {0x80});
	run_bytes("single-0xff", {0xff});

	run_bytes("nul-heavy", std::vector<std::uint8_t>(64, 0x00));
	run_bytes("0x80-heavy", std::vector<std::uint8_t>(64, 0x80));
	run_bytes("0xff-heavy", std::vector<std::uint8_t>(64, 0xff));
	run_bytes("alt-nul-ff",
	    {0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff});

	/* SHA-1 block-size boundaries (64-byte blocks). */
	for (std::size_t len : {55u, 56u, 57u, 63u, 64u, 65u, 127u, 128u,
	    129u}) {
		run_bytes("boundary", make_pattern(len, 0xa5));
	}

	/* Known test vector fragments with high-bit bytes. */
	run_bytes("abc",
	    {'a', 'b', 'c'});
	run_bytes("high-bit-mix",
	    {0x80, 0x81, 0xfe, 0xff, 0x00, 0x7f, 0x01, 0x02});

	/* Escalating lengths with every byte value 0x00..0xff. */
	{
		std::vector<std::uint8_t> ramp(256);
		for (int i = 0; i < 256; i++)
			ramp[static_cast<std::size_t>(i)] =
			    static_cast<std::uint8_t>(i);
		for (std::size_t n : {1u, 16u, 32u, 64u, 128u, 256u}) {
			std::vector<std::uint8_t> slice(ramp.begin(),
			    ramp.begin() + static_cast<std::ptrdiff_t>(n));
			run_bytes("ramp", slice);
		}
	}

	/* Fixed-seed randomised sweep. */
	std::mt19937_64 rng(0x5042534462303039ull);
	for (long i = 0; i < 200000; i++) {
		std::size_t len = pick_len(rng);
		std::vector<std::uint8_t> v(len);
		for (std::size_t j = 0; j < len; j++)
			v[j] = pick_byte(rng);
		run_case("random", v.data(), v.size());
	}

	std::printf("%-32s %12s %12s %10s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");
	std::printf("%-32s %12lu %12lu %10s\n", g_sha1_final.name,
	    g_sha1_final.cases, g_sha1_final.failures,
	    g_sha1_final.failures == 0 ? "PASS" : "FAIL");
	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");
	std::printf("%-32s %12lu %12lu %10s\n", "TOTAL", g_sha1_final.cases,
	    g_sha1_final.failures,
	    g_sha1_final.failures == 0 ? "PASS" : "FAIL");

	std::printf("\n%lu case(s) run, %lu failure(s).\n",
	    g_sha1_final.cases, g_sha1_final.failures);

	return g_sha1_final.failures == 0 ? 0 : 1;
}
