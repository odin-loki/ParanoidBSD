/*
 * Batch b0274 differential test.
 */

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <vector>

import pbsd.lib.libc.inet.b0274;

namespace port = pbsd::lib_libc_inet::b0274;

extern "C" {
char *ref_inet_neta(in_addr_t src, char *dst, size_t size);
in_addr_t ref_inet_lnaof(struct in_addr in);
in_addr_t ref_inet_netof(struct in_addr in);
char *ref_inet_ntoa(struct in_addr in);
char *ref_inet_ntoa_r(struct in_addr in, char *buf, socklen_t size);
}

namespace {

inline constexpr unsigned char GUARD = 0x7f;
inline constexpr std::size_t PRE = 32;
inline constexpr std::size_t POST = 32;
inline constexpr std::size_t DST_CAP = 64;
inline constexpr unsigned long long RANDOM_ITERS = 200000ULL;

struct stats {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
};

stats tbl[] = {
	{ "inet_neta", 0, 0 },
	{ "inet_lnaof", 0, 0 },
	{ "inet_netof", 0, 0 },
	{ "inet_ntoa", 0, 0 },
	{ "inet_ntoa_r", 0, 0 },
};

unsigned long long reported = 0;
const unsigned long long report_limit = 40;

std::uint64_t rng_state = 0x243F6A8885A308D3ULL;

std::uint64_t
next_u64()
{
	std::uint64_t z = (rng_state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

unsigned
rand_u32()
{
	return static_cast<unsigned>(next_u64());
}

struct in_addr
make_in(std::uint32_t host)
{
	struct in_addr in;
	in.s_addr = htonl(host);
	return in;
}

void
fill_guard(unsigned char *p, std::size_t n)
{
	for (std::size_t i = 0; i < n; i++)
		p[i] = GUARD;
}

struct neta_arena {
	unsigned char *refbuf;
	unsigned char *portbuf;
	std::size_t total;
	std::size_t dst_off;
};

void
neta_arena_init(neta_arena &ar)
{
	ar.dst_off = PRE;
	ar.total = PRE + DST_CAP + POST;
	ar.refbuf = static_cast<unsigned char *>(std::malloc(ar.total));
	ar.portbuf = static_cast<unsigned char *>(std::malloc(ar.total));
	if (ar.refbuf == nullptr || ar.portbuf == nullptr) {
		std::fprintf(stderr, "out of memory\n");
		std::exit(2);
	}
}

void
neta_arena_free(neta_arena &ar)
{
	std::free(ar.refbuf);
	std::free(ar.portbuf);
}

void
neta_arena_prepare(neta_arena &ar)
{
	fill_guard(ar.refbuf, ar.total);
	fill_guard(ar.portbuf, ar.total);
}

char *
ref_dst(neta_arena &ar)
{
	return reinterpret_cast<char *>(ar.refbuf + ar.dst_off);
}

char *
port_dst(neta_arena &ar)
{
	return reinterpret_cast<char *>(ar.portbuf + ar.dst_off);
}

bool
check_neta(in_addr_t src, size_t size, const char *origin)
{
	tbl[0].cases++;

	neta_arena ar;
	neta_arena_init(ar);
	neta_arena_prepare(ar);

	char *rd = ref_dst(ar);
	char *pd = port_dst(ar);

	errno = 0;
	char *rret = ref_inet_neta(src, rd, size);
	int rerrno = errno;

	errno = 0;
	char *pret = port::inet_neta(src, pd, size);
	int perrno = errno;

	bool ok = true;
	if ((rret == nullptr) != (pret == nullptr))
		ok = false;
	if (rerrno != perrno)
		ok = false;
	if (rret != nullptr && pret != nullptr) {
		if (rret != rd || pret != pd)
			ok = false;
		if (std::memcmp(ar.refbuf, ar.portbuf, ar.total) != 0)
			ok = false;
	} else if (std::memcmp(ar.refbuf, ar.portbuf, ar.total) != 0)
		ok = false;

	if (!ok) {
		tbl[0].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL inet_neta [%s] src=0x%08x size=%zu "
			    "rret=%p pret=%p rerrno=%d perrno=%d\n",
			    origin, static_cast<unsigned>(src), size,
			    static_cast<void *>(rret),
			    static_cast<void *>(pret), rerrno, perrno);
		}
	}

	neta_arena_free(ar);
	return ok;
}

bool
check_lnaof(std::uint32_t host, const char *origin)
{
	tbl[1].cases++;

	struct in_addr in = make_in(host);
	in_addr_t r = ref_inet_lnaof(in);
	in_addr_t p = port::inet_lnaof(in);

	if (r != p) {
		tbl[1].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL inet_lnaof [%s] host=0x%08x r=0x%08x "
			    "p=0x%08x\n",
			    origin, host, static_cast<unsigned>(r),
			    static_cast<unsigned>(p));
		}
		return false;
	}
	return true;
}

bool
check_netof(std::uint32_t host, const char *origin)
{
	tbl[2].cases++;

	struct in_addr in = make_in(host);
	in_addr_t r = ref_inet_netof(in);
	in_addr_t p = port::inet_netof(in);

	if (r != p) {
		tbl[2].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL inet_netof [%s] host=0x%08x r=0x%08x "
			    "p=0x%08x\n",
			    origin, host, static_cast<unsigned>(r),
			    static_cast<unsigned>(p));
		}
		return false;
	}
	return true;
}

bool
check_ntoa(std::uint32_t host, const char *origin)
{
	tbl[3].cases++;

	struct in_addr in = make_in(host);
	char *r = ref_inet_ntoa(in);
	char *p = port::inet_ntoa(in);

	bool ok = true;
	if (r == nullptr || p == nullptr) {
		if (r != p)
			ok = false;
	} else {
		if (std::strcmp(r, p) != 0)
			ok = false;
	}

	if (!ok) {
		tbl[3].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL inet_ntoa [%s] host=0x%08x r=\"%s\" "
			    "p=\"%s\"\n",
			    origin, host, r ? r : "(null)", p ? p : "(null)");
		}
	}
	return ok;
}

struct ntoa_r_arena {
	unsigned char *refbuf;
	unsigned char *portbuf;
	std::size_t total;
	std::size_t buf_off;
};

void
ntoa_r_arena_init(ntoa_r_arena &ar, std::size_t cap)
{
	ar.buf_off = PRE;
	ar.total = PRE + cap + POST;
	ar.refbuf = static_cast<unsigned char *>(std::malloc(ar.total));
	ar.portbuf = static_cast<unsigned char *>(std::malloc(ar.total));
	if (ar.refbuf == nullptr || ar.portbuf == nullptr) {
		std::fprintf(stderr, "out of memory\n");
		std::exit(2);
	}
}

void
ntoa_r_arena_free(ntoa_r_arena &ar)
{
	std::free(ar.refbuf);
	std::free(ar.portbuf);
}

void
ntoa_r_arena_prepare(ntoa_r_arena &ar)
{
	fill_guard(ar.refbuf, ar.total);
	fill_guard(ar.portbuf, ar.total);
}

char *
ref_buf(ntoa_r_arena &ar)
{
	return reinterpret_cast<char *>(ar.refbuf + ar.buf_off);
}

char *
port_buf(ntoa_r_arena &ar)
{
	return reinterpret_cast<char *>(ar.portbuf + ar.buf_off);
}

bool
check_ntoa_r(std::uint32_t host, socklen_t size, std::size_t cap,
    const char *origin)
{
	tbl[4].cases++;

	ntoa_r_arena ar;
	ntoa_r_arena_init(ar, cap);
	ntoa_r_arena_prepare(ar);

	char *rb = ref_buf(ar);
	char *pb = port_buf(ar);

	char *rret = ref_inet_ntoa_r(make_in(host), rb, size);
	char *pret = port::inet_ntoa_r(make_in(host), pb, size);

	bool ok = true;
	if ((rret == nullptr) != (pret == nullptr))
		ok = false;
	if (rret != nullptr && pret != nullptr) {
		if (rret != rb || pret != pb)
			ok = false;
		if (std::memcmp(ar.refbuf, ar.portbuf, ar.total) != 0)
			ok = false;
	} else if (std::memcmp(ar.refbuf, ar.portbuf, ar.total) != 0)
		ok = false;

	if (!ok) {
		tbl[4].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL inet_ntoa_r [%s] host=0x%08x size=%u "
			    "cap=%zu\n",
			    origin, host, static_cast<unsigned>(size), cap);
		}
	}

	ntoa_r_arena_free(ar);
	return ok;
}

void
hand_neta_edges()
{
	static const in_addr_t srcs[] = {
		0u,
		1u,
		0x01000000u,
		0x00010000u,
		0x00000100u,
		0x00000001u,
		0x01020304u,
		0x7f000001u,
		0x80000001u,
		0xc0000001u,
		0xe0000001u,
		0xff000000u,
		0x00ff0000u,
		0x0000ff00u,
		0x000000ffu,
		0xffffffffu,
		0x80808080u,
		0x7f7f7f7fu,
		0x010080ffu,
	};
	static const size_t sizes[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 15, 16, 17, 31, 32,
		63, 64,
	};

	for (in_addr_t src : srcs) {
		for (size_t sz : sizes)
			check_neta(src, sz, "edge");
	}

	/* sizeof "255." boundary */
	check_neta(0xff000000u, 4, "edge-255-size4");
	check_neta(0xff000000u, 5, "edge-255-size5");

	/* sizeof "0.0.0.0" boundary */
	check_neta(0u, 7, "edge-zero-size7");
	check_neta(0u, 8, "edge-zero-size8");

	/* tight multi-segment capacity */
	check_neta(0x01020304u, 7, "edge-1234-size7");
	check_neta(0x01020304u, 8, "edge-1234-size8");
	check_neta(0x01020304u, 9, "edge-1234-size9");
	check_neta(0x01020304u, 10, "edge-1234-size10");
	check_neta(0x01020304u, 11, "edge-1234-size11");

	/* high-byte sweep 0x80-0xFF in each position */
	for (unsigned b = 0x80; b <= 0xff; b++) {
		check_neta(static_cast<in_addr_t>(b) << 24, 16, "edge-high-a");
		check_neta(static_cast<in_addr_t>(b) << 16, 16, "edge-high-b");
		check_neta(static_cast<in_addr_t>(b) << 8, 16, "edge-high-c");
		check_neta(static_cast<in_addr_t>(b), 16, "edge-high-d");
	}
}

void
hand_lnaof_edges()
{
	static const std::uint32_t hosts[] = {
		0x00000000u,
		0x00000001u,
		0x0000007fu,
		0x00000080u,
		0x000000ffu,
		0x007fffffu,
		0x7fffffffu,
		0x80000000u,
		0x80000001u,
		0x80ffffffu,
		0xbfffffffu,
		0xc0000000u,
		0xc0000001u,
		0xc0ffffffu,
		0xdfffffffu,
		0xe0000000u,
		0xe0000001u,
		0xffffffffu,
		0x01020304u,
		0x80808080u,
		0x7f7f7f7fu,
	};

	for (std::uint32_t h : hosts)
		check_lnaof(h, "edge");
}

void
hand_netof_edges()
{
	static const std::uint32_t hosts[] = {
		0x00000000u,
		0x00000001u,
		0x0000007fu,
		0x00000080u,
		0x000000ffu,
		0x007fffffu,
		0x7fffffffu,
		0x80000000u,
		0x80000001u,
		0x80ffffffu,
		0xbfffffffu,
		0xc0000000u,
		0xc0000001u,
		0xc0ffffffu,
		0xdfffffffu,
		0xe0000000u,
		0xe0000001u,
		0xffffffffu,
		0x01020304u,
		0x80808080u,
		0x7f7f7f7fu,
	};

	for (std::uint32_t h : hosts)
		check_netof(h, "edge");
}

void
hand_ntoa_edges()
{
	static const std::uint32_t hosts[] = {
		0x00000000u,
		0x00000001u,
		0x7fffffffu,
		0x80000000u,
		0xc0000000u,
		0xe0000000u,
		0xffffffffu,
		0x01020304u,
		0x80808080u,
		0x7f7f7f7fu,
		0x010080ffu,
	};

	for (std::uint32_t h : hosts)
		check_ntoa(h, "edge");
}

void
hand_ntoa_r_edges()
{
	static const std::uint32_t hosts[] = {
		0x00000000u,
		0x00000001u,
		0x7fffffffu,
		0x80000000u,
		0xc0000000u,
		0xe0000000u,
		0xffffffffu,
		0x01020304u,
		0x80808080u,
		0x7f7f7f7fu,
		0x010080ffu,
	};
	static const socklen_t sizes[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 15, 16, 17, 31, 32,
	};
	static const std::size_t caps[] = { 8, 16, 32, 64 };

	for (std::uint32_t h : hosts) {
		for (socklen_t sz : sizes) {
			for (std::size_t cap : caps)
				check_ntoa_r(h, sz, cap, "edge");
		}
	}

	check_ntoa_r(0x01020304u, 0, 64, "edge-size0");
	check_ntoa_r(0x01020304u, 1, 64, "edge-size1");
	check_ntoa_r(0x01020304u, 8, 64, "edge-size8");
	check_ntoa_r(0x01020304u, 16, 64, "edge-size16");
}

void
random_sweep()
{
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		in_addr_t src = static_cast<in_addr_t>(rand_u32());
		size_t size = static_cast<size_t>(rand_u32() % 65);
		check_neta(src, size, "random");

		std::uint32_t host = rand_u32();
		check_lnaof(host, "random");
		check_netof(host, "random");
		check_ntoa(host, "random");

		socklen_t sz = static_cast<socklen_t>(rand_u32() % 33);
		std::size_t cap = static_cast<std::size_t>(8 + (rand_u32() % 57));
		check_ntoa_r(host, sz, cap, "random");
	}
}

void
print_table()
{
	std::printf("\n=== b0274 differential test results ===\n");
	std::printf("%-16s %12s %12s\n", "function", "cases", "failures");
	for (const stats &s : tbl)
		std::printf("%-16s %12llu %12llu\n", s.name, s.cases,
		    s.failures);
}

} // namespace

int
main()
{
	hand_neta_edges();
	hand_lnaof_edges();
	hand_netof_edges();
	hand_ntoa_edges();
	hand_ntoa_r_edges();
	random_sweep();
	print_table();

	unsigned long long total_fail = 0;
	for (const stats &s : tbl)
		total_fail += s.failures;
	return total_fail == 0 ? 0 : 1;
}
