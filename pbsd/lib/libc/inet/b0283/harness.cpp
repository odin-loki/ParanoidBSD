/*
 * Batch b0283 differential test.
 */

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <string>
#include <vector>

import pbsd.lib.libc.inet.b0283;

namespace port = pbsd::lib_libc_inet::b0283;

extern "C" {
struct in_addr ref_inet_makeaddr(in_addr_t net, in_addr_t host);
u_int ref_inet_nsap_addr(const char *ascii, unsigned char *binary, int maxlen);
char *ref_inet_nsap_ntoa(int binlen, const unsigned char *binary, char *ascii);
in_addr_t ref_inet_network(const char *cp);
char *ref_inet_net_ntop(int af, const void *src, int bits, char *dst,
    size_t size);
extern char inet_nsap_ntoa_tmpbuf[];
}

namespace {

inline constexpr unsigned char GUARD = 0x7f;
inline constexpr std::size_t PRE = 32;
inline constexpr std::size_t POST = 32;
inline constexpr std::size_t BIN_CAP = 512;
inline constexpr std::size_t STR_CAP = 1024;
inline constexpr unsigned long long RANDOM_ITERS = 200000ULL;

struct stats {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
};

stats tbl[] = {
	{ "inet_makeaddr", 0, 0 },
	{ "inet_nsap_addr", 0, 0 },
	{ "inet_nsap_ntoa", 0, 0 },
	{ "inet_network", 0, 0 },
	{ "inet_net_ntop", 0, 0 },
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

void
fill_guard(unsigned char *p, std::size_t n)
{
	for (std::size_t i = 0; i < n; i++)
		p[i] = GUARD;
}

struct bin_arena {
	unsigned char *refbuf;
	unsigned char *portbuf;
	std::size_t total;
	std::size_t data_off;
};

void
bin_arena_init(bin_arena &ar)
{
	ar.data_off = PRE;
	ar.total = PRE + BIN_CAP + POST;
	ar.refbuf = static_cast<unsigned char *>(std::malloc(ar.total));
	ar.portbuf = static_cast<unsigned char *>(std::malloc(ar.total));
	if (ar.refbuf == nullptr || ar.portbuf == nullptr) {
		std::fprintf(stderr, "out of memory\n");
		std::exit(2);
	}
}

void
bin_arena_free(bin_arena &ar)
{
	std::free(ar.refbuf);
	std::free(ar.portbuf);
}

void
bin_arena_prepare(bin_arena &ar)
{
	fill_guard(ar.refbuf, ar.total);
	fill_guard(ar.portbuf, ar.total);
}

unsigned char *
ref_bin(bin_arena &ar)
{
	return ar.refbuf + ar.data_off;
}

unsigned char *
port_bin(bin_arena &ar)
{
	return ar.portbuf + ar.data_off;
}

struct str_arena {
	unsigned char *refbuf;
	unsigned char *portbuf;
	std::size_t total;
	std::size_t data_off;
};

void
str_arena_init(str_arena &ar, std::size_t cap)
{
	ar.data_off = PRE;
	ar.total = PRE + cap + POST;
	ar.refbuf = static_cast<unsigned char *>(std::malloc(ar.total));
	ar.portbuf = static_cast<unsigned char *>(std::malloc(ar.total));
	if (ar.refbuf == nullptr || ar.portbuf == nullptr) {
		std::fprintf(stderr, "out of memory\n");
		std::exit(2);
	}
}

void
str_arena_free(str_arena &ar)
{
	std::free(ar.refbuf);
	std::free(ar.portbuf);
}

void
str_arena_prepare(str_arena &ar)
{
	fill_guard(ar.refbuf, ar.total);
	fill_guard(ar.portbuf, ar.total);
}

char *
ref_str(str_arena &ar)
{
	return reinterpret_cast<char *>(ar.refbuf + ar.data_off);
}

char *
port_str(str_arena &ar)
{
	return reinterpret_cast<char *>(ar.portbuf + ar.data_off);
}

bool
check_makeaddr(in_addr_t net, in_addr_t host, const char *origin)
{
	tbl[0].cases++;

	struct in_addr r = ref_inet_makeaddr(net, host);
	struct in_addr p = port::inet_makeaddr(net, host);

	if (r.s_addr != p.s_addr) {
		tbl[0].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL inet_makeaddr [%s] net=0x%08x host=0x%08x "
			    "r=0x%08x p=0x%08x\n",
			    origin, static_cast<unsigned>(net),
			    static_cast<unsigned>(host),
			    static_cast<unsigned>(r.s_addr),
			    static_cast<unsigned>(p.s_addr));
		}
		return false;
	}
	return true;
}

bool
check_nsap_addr(const char *ascii, int maxlen, const char *origin)
{
	tbl[1].cases++;

	bin_arena ar;
	bin_arena_init(ar);
	bin_arena_prepare(ar);

	unsigned char *rb = ref_bin(ar);
	unsigned char *pb = port_bin(ar);

	u_int rlen = ref_inet_nsap_addr(ascii, rb, maxlen);
	u_int plen = port::inet_nsap_addr(ascii, pb, maxlen);

	bool ok = true;
	if (rlen != plen)
		ok = false;
	if (rlen > 0 && std::memcmp(rb, pb, rlen) != 0)
		ok = false;
	if (std::memcmp(ar.refbuf, ar.portbuf, ar.total) != 0)
		ok = false;

	if (!ok) {
		tbl[1].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL inet_nsap_addr [%s] ascii=\"%s\" "
			    "maxlen=%d rlen=%u plen=%u\n",
			    origin, ascii ? ascii : "(null)", maxlen, rlen,
			    plen);
		}
	}

	bin_arena_free(ar);
	return ok;
}

bool
check_nsap_ntoa_tmp(int binlen, const unsigned char *binary,
    const char *origin)
{
	tbl[2].cases++;

	char *rret = ref_inet_nsap_ntoa(binlen, binary, nullptr);
	char *pret = port::inet_nsap_ntoa(binlen, binary, nullptr);

	bool ok = true;
	if ((rret == nullptr) != (pret == nullptr))
		ok = false;
	if (rret != nullptr && pret != nullptr) {
		if (rret != inet_nsap_ntoa_tmpbuf)
			ok = false;
		if (pret != port::inet_nsap_ntoa_tmpbuf)
			ok = false;
		if (std::strcmp(rret, pret) != 0)
			ok = false;
	}

	if (!ok) {
		tbl[2].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL inet_nsap_ntoa [%s] binlen=%d tmpbuf\n",
			    origin, binlen);
		}
	}
	return ok;
}

bool
check_nsap_ntoa_buf(int binlen, const unsigned char *binary,
    const char *origin)
{
	str_arena ar;
	str_arena_init(ar, STR_CAP);
	str_arena_prepare(ar);

	tbl[2].cases++;

	char *rb = ref_str(ar);
	char *pb = port_str(ar);

	char *rret = ref_inet_nsap_ntoa(binlen, binary, rb);
	char *pret = port::inet_nsap_ntoa(binlen, binary, pb);

	bool ok = true;
	if ((rret == nullptr) != (pret == nullptr))
		ok = false;
	if (rret != nullptr && pret != nullptr) {
		if (rret != rb || pret != pb)
			ok = false;
		if (std::strcmp(rret, pret) != 0)
			ok = false;
	}
	if (std::memcmp(ar.refbuf, ar.portbuf, ar.total) != 0)
		ok = false;

	if (!ok) {
		tbl[2].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL inet_nsap_ntoa [%s] binlen=%d\n",
			    origin, binlen);
		}
	}

	str_arena_free(ar);
	return ok;
}

bool
check_network(const char *cp, const char *origin)
{
	tbl[3].cases++;

	in_addr_t r = ref_inet_network(cp);
	in_addr_t p = port::inet_network(cp);

	if (r != p) {
		tbl[3].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL inet_network [%s] cp=\"%s\" "
			    "r=0x%08x p=0x%08x\n",
			    origin, cp ? cp : "(null)",
			    static_cast<unsigned>(r),
			    static_cast<unsigned>(p));
		}
		return false;
	}
	return true;
}

bool
check_net_ntop(int af, const unsigned char *src, int bits, size_t size,
    const char *origin)
{
	tbl[4].cases++;

	str_arena ar;
	str_arena_init(ar, size + POST);
	str_arena_prepare(ar);

	char *rd = ref_str(ar);
	char *pd = port_str(ar);

	errno = 0;
	char *rret = ref_inet_net_ntop(af, src, bits, rd, size);
	int rerrno = errno;

	errno = 0;
	char *pret = port::inet_net_ntop(af, src, bits, pd, size);
	int perrno = errno;

	bool ok = true;
	if ((rret == nullptr) != (pret == nullptr))
		ok = false;
	if (rerrno != perrno)
		ok = false;
	if (rret != nullptr && pret != nullptr) {
		if (rret != rd || pret != pd)
			ok = false;
		if (std::strcmp(rret, pret) != 0)
			ok = false;
	}
	if (std::memcmp(ar.refbuf, ar.portbuf, ar.total) != 0)
		ok = false;

	if (!ok) {
		tbl[4].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL inet_net_ntop [%s] af=%d bits=%d "
			    "size=%zu rerrno=%d perrno=%d\n",
			    origin, af, bits, size, rerrno, perrno);
		}
	}

	str_arena_free(ar);
	return ok;
}

void
hand_makeaddr_edges()
{
	static const in_addr_t nets[] = {
		0u, 1u, 127u, 128u, 129u, 255u, 256u, 65535u, 65536u,
		16777215u, 16777216u, 0xffffffffu,
	};
	static const in_addr_t hosts[] = {
		0u, 1u, 0x7fu, 0x80u, 0xffu, 0xffffu, 0xffffffu, 0xffffffffu,
		0x01020304u, 0x80808080u,
	};

	for (in_addr_t net : nets) {
		for (in_addr_t host : hosts)
			check_makeaddr(net, host, "edge");
	}

	check_makeaddr(127u, 0x00ffffffu, "edge-classa-maxhost");
	check_makeaddr(128u, 0x0000ffffu, "edge-classb-maxhost");
	check_makeaddr(65536u, 0x000000ffu, "edge-classc-maxhost");
	check_makeaddr(16777216u, 0xffffffffu, "edge-else");
}

void
hand_nsap_addr_edges()
{
	static const char *valid[] = {
		"0x",
		"0X",
		"0x00",
		"0xAB",
		"0xab",
		"0xABCD",
		"0xAB.CD",
		"0xAB+CD",
		"0xAB/CD",
		"0xAB.CD.EF",
		"0x0123456789ABCDEF",
		"0xdeadbeef",
		"0xDEAD.BEEF",
	};
	static const char *invalid[] = {
		"",
		"0",
		"1x00",
		"0y00",
		"0xG0",
		"0xA",
		"0xABG",
		"0xAB\x80",
		"0xAB\xFF",
		"xx",
		"0x.",
		"0x+",
		"0x/",
	};
	static const int maxlens[] = { 0, 1, 2, 3, 4, 8, 16, 32, 64, 256 };

	for (const char *s : valid) {
		for (int ml : maxlens)
			check_nsap_addr(s, ml, "edge-valid");
	}
	for (const char *s : invalid) {
		for (int ml : maxlens)
			check_nsap_addr(s, ml, "edge-invalid");
	}

	/* maxlen boundary: long string */
	char longhex[600];
	std::memcpy(longhex, "0x", 2);
	for (int i = 2; i < 598; i++)
		longhex[i] = "0123456789ABCDEF"[i & 15];
	longhex[598] = '\0';
	check_nsap_addr(longhex, 10, "edge-long");
	check_nsap_addr(longhex, 100, "edge-long100");
}

void
hand_nsap_ntoa_edges()
{
	unsigned char bin[300];
	for (int i = 0; i < 300; i++)
		bin[i] = static_cast<unsigned char>(0x80 | (i & 0x7f));

	static const int lens[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 16, 32, 64, 128, 255, 256, 300,
	};

	for (int len : lens) {
		check_nsap_ntoa_buf(len, bin, "edge-buf");
		check_nsap_ntoa_tmp(len, bin, "edge-tmp");
	}

	unsigned char nibbles[] = { 0x00, 0x01, 0x09, 0x0a, 0x0f, 0x10, 0x99,
	    0xaa, 0xff };
	for (unsigned char b : nibbles) {
		unsigned char one[] = { b };
		check_nsap_ntoa_buf(1, one, "edge-nibble");
		check_nsap_ntoa_tmp(1, one, "edge-nibble-tmp");
	}
}

void
hand_network_edges()
{
	static const char *valid[] = {
		"0",
		"1",
		"255",
		"256",
		"0.0.0.0",
		"127.0.0.1",
		"192.168.1.1",
		"255.255.255.255",
		"010",
		"077",
		"0x0",
		"0xFF",
		"0xff",
		"0x10.20",
		"10.20.30.40",
		"0x1.0x2",
		"128.1",
		"  10  ",
	};
	static const char *invalid[] = {
		"",
		"08",
		"019",
		"256",
		"300",
		"1.2.3.4.5",
		"1.2.3.256",
		"1.2.3.4x",
		".",
		"1.",
		".1",
		"x",
		"0x",
		"0xg",
		"1..2",
	};

	for (const char *s : valid)
		check_network(s, "edge-valid");
	for (const char *s : invalid)
		check_network(s, "edge-invalid");

	/* octal 8/9 rejection */
	check_network("08", "edge-oct8");
	check_network("09", "edge-oct9");
	check_network("018", "edge-oct18");
	check_network("00", "edge-oct00");
	check_network("010", "edge-oct10");

	/* high-bit in char path via isspace(*cp&0xff) */
	check_network("10\x80", "edge-highbit");
	check_network("10 ", "edge-space");
	check_network("10\t", "edge-tab");
}

void
hand_net_ntop_edges()
{
	unsigned char v4[4] = { 192, 5, 5, 240 };
	unsigned char v6[16] = {
		0x20, 0x01, 0x0d, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	};

	static const int v4_bits[] = {
		-1, 0, 1, 7, 8, 9, 15, 16, 17, 23, 24, 25, 31, 32, 33,
	};
	static const size_t sizes[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 15, 16, 17, 20, 24,
		32, 48, 64,
	};

	for (int bits : v4_bits) {
		for (size_t sz : sizes)
			check_net_ntop(AF_INET, v4, bits, sz, "edge-v4");
	}

	static const int v6_bits[] = { -1, 0, 1, 64, 127, 128, 129 };
	for (int bits : v6_bits) {
		for (size_t sz : sizes)
			check_net_ntop(AF_INET6, v6, bits, sz, "edge-v6");
	}

	check_net_ntop(99, v4, 24, 64, "edge-bad-af");
	check_net_ntop(AF_INET, v4, 28, 64, "edge-v4-28");
	check_net_ntop(AF_INET, v4, 0, 8, "edge-v4-bits0");
	check_net_ntop(AF_INET, v4, 32, 64, "edge-v4-bits32");

	unsigned char partial[4] = { 10, 20, 30, 255 };
	for (int b = 1; b <= 31; b++)
		check_net_ntop(AF_INET, partial, b, 64, "edge-partial");
}

std::string
random_hex_string(unsigned len)
{
	std::string s = (rand_u32() & 1) ? "0x" : "0X";
	static const char *hex = "0123456789ABCDEF";
	static const char *sep = ".+/";
	for (unsigned i = 0; i < len; i++) {
		unsigned r = rand_u32();
		if ((r & 7) == 0)
			s.push_back(sep[r & 3]);
		else
			s.push_back(hex[r & 15]);
	}
	return s;
}

std::string
random_network_string()
{
	std::string s;
	unsigned kind = rand_u32() % 8;
	if (kind == 0) {
		s = "0";
		if (rand_u32() & 1)
			s += (rand_u32() & 1) ? "x" : "X";
	}
	unsigned parts = 1 + (rand_u32() % 5);
	for (unsigned i = 0; i < parts; i++) {
		if (i > 0)
			s.push_back('.');
		unsigned base = rand_u32() % 3;
		unsigned val = rand_u32() & 0xff;
		if (base == 1)
			s += std::to_string(val);
		else if (base == 2) {
			char buf[16];
			std::snprintf(buf, sizeof(buf), "%u", val);
			s += buf;
		} else {
			char buf[16];
			std::snprintf(buf, sizeof(buf), "0x%x", val);
			s += buf;
		}
	}
	if (rand_u32() & 1)
		s.push_back(static_cast<char>(0x80 | (rand_u32() & 0x7f)));
	if (rand_u32() & 3)
		s.push_back(' ');
	return s;
}

void
random_sweep()
{
	unsigned char bin[300];
	unsigned char v4[4];
	unsigned char v6[16];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		in_addr_t net = static_cast<in_addr_t>(rand_u32());
		in_addr_t host = static_cast<in_addr_t>(rand_u32());
		check_makeaddr(net, host, "random");

		std::string hex = random_hex_string(rand_u32() % 80);
		int maxlen = static_cast<int>(rand_u32() % 129);
		check_nsap_addr(hex.c_str(), maxlen, "random");

		unsigned blen = rand_u32() % 301;
		for (unsigned j = 0; j < blen; j++)
			bin[j] = static_cast<unsigned char>(rand_u32() & 0xff);
		check_nsap_ntoa_buf(static_cast<int>(blen), bin, "random");
		if ((rand_u32() & 3) == 0)
			check_nsap_ntoa_tmp(static_cast<int>(blen), bin,
			    "random");

		std::string netstr = random_network_string();
		check_network(netstr.c_str(), "random");

		for (unsigned j = 0; j < 4; j++)
			v4[j] = static_cast<unsigned char>(rand_u32() & 0xff);
		for (unsigned j = 0; j < 16; j++)
			v6[j] = static_cast<unsigned char>(rand_u32() & 0xff);

		int bits4 = static_cast<int>(rand_u32() % 40) - 4;
		int bits6 = static_cast<int>(rand_u32() % 140) - 6;
		size_t sz = static_cast<size_t>(rand_u32() % 65);

		check_net_ntop(AF_INET, v4, bits4, sz, "random");
		check_net_ntop(AF_INET6, v6, bits6, sz, "random");
		if ((rand_u32() & 7) == 0)
			check_net_ntop(static_cast<int>(rand_u32() % 256),
			    v4, bits4, sz, "random-af");
	}
}

void
print_table()
{
	std::printf("\n=== b0283 differential test results ===\n");
	std::printf("%-16s %12s %12s\n", "function", "cases", "failures");
	for (const stats &s : tbl)
		std::printf("%-16s %12llu %12llu\n", s.name, s.cases,
		    s.failures);
}

} // namespace

int
main()
{
	hand_makeaddr_edges();
	hand_nsap_addr_edges();
	hand_nsap_ntoa_edges();
	hand_network_edges();
	hand_net_ntop_edges();
	random_sweep();
	print_table();

	unsigned long long total_fail = 0;
	for (const stats &s : tbl)
		total_fail += s.failures;
	return total_fail == 0 ? 0 : 1;
}
