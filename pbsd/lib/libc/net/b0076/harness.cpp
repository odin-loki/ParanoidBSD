/*
 * Differential test for batch b0076: pbsd.lib.libc.net.b0076 (port) versus the
 * ref_ oracle built from the original HardenedBSD sources.
 */

#include <netinet/in.h>

#undef htonl
#undef htons
#undef ntohl
#undef ntohs

#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libc.net.b0076;

namespace port = pbsd::lib_libc_net::b0076;

extern "C" {
std::uint32_t ref_htonl(std::uint32_t hl);
std::uint16_t ref_htons(std::uint16_t hs);
std::uint32_t ref_ntohl(std::uint32_t nl);
std::uint16_t ref_ntohs(std::uint16_t ns);
extern const struct in6_addr ref_in6addr_any;
extern const struct in6_addr ref_in6addr_loopback;
extern const struct in6_addr ref_in6addr_nodelocal_allnodes;
extern const struct in6_addr ref_in6addr_linklocal_allnodes;
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr std::size_t BUFSZ = 64;
constexpr std::size_t STORE_OFF = 16;

struct Stat {
	const char *name;
	unsigned long cases;
	unsigned long failures;
};

Stat stats[] = {
	{ "htonl", 0, 0 },
	{ "htons", 0, 0 },
	{ "ntohl", 0, 0 },
	{ "ntohs", 0, 0 },
	{ "in6addr_any", 0, 0 },
	{ "in6addr_loopback", 0, 0 },
	{ "in6addr_nodelocal_allnodes", 0, 0 },
	{ "in6addr_linklocal_allnodes", 0, 0 },
};

enum {
	S_HTONL = 0,
	S_HTONS,
	S_NTOHL,
	S_NTOHS,
	S_ANY,
	S_LOOPBACK,
	S_NODELOCAL,
	S_LINKLOCAL,
};

unsigned long reported[8];

void
fail(int slot, const char *msg)
{

	stats[slot].failures++;
	if (reported[slot] < 10) {
		reported[slot]++;
		std::printf("FAIL %s: %s\n", stats[slot].name, msg);
	}
}

/*
 * Every scalar result is also stored into a guard-filled buffer so that the
 * whole buffer, including the bytes past the nominal write window, is compared
 * (a port whose result differs in width, not just in value, is caught too).
 */
template <typename T>
bool
buffers_agree(T pv, T rv)
{
	unsigned char pb[BUFSZ], rb[BUFSZ];

	std::memset(pb, GUARD, sizeof(pb));
	std::memset(rb, GUARD, sizeof(rb));
	std::memcpy(pb + STORE_OFF, &pv, sizeof(pv));
	std::memcpy(rb + STORE_OFF, &rv, sizeof(rv));
	return (std::memcmp(pb, rb, BUFSZ) == 0);
}

void
check32(int slot, std::uint32_t in, std::uint32_t pv, std::uint32_t rv)
{
	char msg[128];

	stats[slot].cases++;
	if (pv != rv) {
		std::snprintf(msg, sizeof(msg),
		    "in=0x%08x port=0x%08x ref=0x%08x", in, pv, rv);
		fail(slot, msg);
	} else if (!buffers_agree(pv, rv)) {
		std::snprintf(msg, sizeof(msg),
		    "in=0x%08x buffer mismatch (0x%08x)", in, pv);
		fail(slot, msg);
	}
}

void
check16(int slot, std::uint16_t in, std::uint16_t pv, std::uint16_t rv)
{
	char msg[128];

	stats[slot].cases++;
	if (pv != rv) {
		std::snprintf(msg, sizeof(msg),
		    "in=0x%04x port=0x%04x ref=0x%04x", in, pv, rv);
		fail(slot, msg);
	} else if (!buffers_agree(pv, rv)) {
		std::snprintf(msg, sizeof(msg),
		    "in=0x%04x buffer mismatch (0x%04x)", in, pv);
		fail(slot, msg);
	}
}

void
do_htonl(std::uint32_t v)
{

	check32(S_HTONL, v, port::htonl(v), ref_htonl(v));
}

void
do_ntohl(std::uint32_t v)
{

	check32(S_NTOHL, v, port::ntohl(v), ref_ntohl(v));
}

void
do_htons(std::uint16_t v)
{

	check16(S_HTONS, v, port::htons(v), ref_htons(v));
}

void
do_ntohs(std::uint16_t v)
{

	check16(S_NTOHS, v, port::ntohs(v), ref_ntohs(v));
}

void
do32(std::uint32_t v)
{

	do_htonl(v);
	do_ntohl(v);
}

void
do16(std::uint16_t v)
{

	do_htons(v);
	do_ntohs(v);
}

/* splitmix64, fixed seed: the randomised sweep must be reproducible. */
std::uint64_t rng_state = 0x0076b00760760076ULL;

std::uint64_t
rng_next()
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

void
test_byteorder()
{
	static const std::uint32_t edge32[] = {
		0x00000000u, 0x00000001u, 0x00000002u, 0x0000007fu,
		0x00000080u, 0x000000ffu, 0x00000100u, 0x00007fffu,
		0x00008000u, 0x0000fffeu, 0x0000ffffu, 0x00010000u,
		0x007fffffu, 0x00800000u, 0x00ff00ffu, 0x00ffff00u,
		0x01000000u, 0x01020304u, 0x04030201u, 0x7f7f7f7fu,
		0x7ffffffeu, 0x7fffffffu, 0x80000000u, 0x80000001u,
		0x80808080u, 0xdeadbeefu, 0xefbeaddeu, 0xff000000u,
		0xff00ff00u, 0xffff0000u, 0xfffffffeu, 0xffffffffu,
		0xfeedfaceu, 0x0080ff7fu, 0x7fff8000u, 0x8000ffffu,
	};
	static const std::uint16_t edge16[] = {
		0x0000u, 0x0001u, 0x0002u, 0x007fu, 0x0080u, 0x00feu,
		0x00ffu, 0x0100u, 0x0102u, 0x0201u, 0x7f00u, 0x7f7fu,
		0x7ffeu, 0x7fffu, 0x8000u, 0x8001u, 0x8080u, 0xfefeu,
		0xff00u, 0xff01u, 0xff7fu, 0xff80u, 0xfffeu, 0xffffu,
	};

	for (std::uint32_t v : edge32)
		do32(v);
	for (std::uint16_t v : edge16)
		do16(v);

	/* Single bit set / single bit clear: pins down every mask bit. */
	for (unsigned i = 0; i < 32; i++) {
		do32(std::uint32_t(1) << i);
		do32(~(std::uint32_t(1) << i));
	}
	for (unsigned i = 0; i < 16; i++) {
		do16(std::uint16_t(std::uint16_t(1) << i));
		do16(std::uint16_t(~(std::uint16_t(1) << i)));
	}

	/* Every byte value in every byte position. */
	for (unsigned pos = 0; pos < 4; pos++) {
		for (unsigned b = 0; b < 256; b++) {
			do32(std::uint32_t(b) << (8 * pos));
			do32(std::uint32_t(0xffffffffu ^
			    (std::uint32_t(b) << (8 * pos))));
		}
	}

	/* Exhaustive over the whole 16-bit domain. */
	for (std::uint32_t v = 0; v <= 0xffffu; v++)
		do16(std::uint16_t(v));

	/* Randomised sweep. */
	for (unsigned long i = 0; i < 250000ul; i++) {
		std::uint64_t r = rng_next();
		std::uint32_t v32 = std::uint32_t(r);
		std::uint16_t v16 = std::uint16_t(r >> 32);

		/*
		 * Bias part of the sweep towards high-bit and sparse byte
		 * patterns, which is where a swapped shift or a clipped mask
		 * shows up.
		 */
		switch (r % 4u) {
		case 0:
			break;
		case 1:
			v32 |= 0x80808080u;
			v16 = std::uint16_t(v16 | 0x8080u);
			break;
		case 2:
			v32 &= 0xff00ff00u;
			v16 = std::uint16_t(v16 & 0xff00u);
			break;
		default:
			v32 = std::uint32_t(r & 0xffu) <<
			    (8 * unsigned((r >> 8) % 4u));
			v16 = std::uint16_t(std::uint16_t((r >> 16) & 0xffu)
			    << (8 * unsigned((r >> 24) % 2u)));
			break;
		}
		do32(v32);
		do16(v16);
	}
}

void
check_addr(int slot, const struct in6_addr &pv, const struct in6_addr &rv)
{
	unsigned char pb[BUFSZ], rb[BUFSZ];
	char msg[128];

	/* Byte by byte, so a single planted digit is pinpointed. */
	for (std::size_t i = 0; i < sizeof(struct in6_addr); i++) {
		unsigned char a = reinterpret_cast<const unsigned char *>
		    (&pv)[i];
		unsigned char b = reinterpret_cast<const unsigned char *>
		    (&rv)[i];

		stats[slot].cases++;
		if (a != b) {
			std::snprintf(msg, sizeof(msg),
			    "byte %u: port=0x%02x ref=0x%02x", unsigned(i),
			    unsigned(a), unsigned(b));
			fail(slot, msg);
		}
	}

	std::memset(pb, GUARD, sizeof(pb));
	std::memset(rb, GUARD, sizeof(rb));
	std::memcpy(pb + STORE_OFF, &pv, sizeof(pv));
	std::memcpy(rb + STORE_OFF, &rv, sizeof(rv));
	stats[slot].cases++;
	if (std::memcmp(pb, rb, BUFSZ) != 0)
		fail(slot, "guard buffer mismatch");
}

void
test_vars()
{

	check_addr(S_ANY, port::in6addr_any, ref_in6addr_any);
	check_addr(S_LOOPBACK, port::in6addr_loopback, ref_in6addr_loopback);
	check_addr(S_NODELOCAL, port::in6addr_nodelocal_allnodes,
	    ref_in6addr_nodelocal_allnodes);
	check_addr(S_LINKLOCAL, port::in6addr_linklocal_allnodes,
	    ref_in6addr_linklocal_allnodes);
}

} /* namespace */

int
main()
{
	unsigned long total_cases = 0, total_failures = 0;

	test_byteorder();
	test_vars();

	std::printf("\n%-30s %10s %10s\n", "function", "cases", "failures");
	std::printf("%-30s %10s %10s\n", "------------------------------",
	    "----------", "----------");
	for (const Stat &s : stats) {
		std::printf("%-30s %10lu %10lu\n", s.name, s.cases,
		    s.failures);
		total_cases += s.cases;
		total_failures += s.failures;
	}
	std::printf("%-30s %10lu %10lu\n", "TOTAL", total_cases,
	    total_failures);

	if (total_failures != 0) {
		std::printf("\nRESULT: FAIL (%lu mismatches)\n",
		    total_failures);
		return (1);
	}
	std::printf("\nRESULT: PASS\n");
	return (0);
}
