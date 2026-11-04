/*
 * Batch b0324 differential test.
 */

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <string>
#include <vector>

import pbsd.lib.libc.inet.b0324;

namespace port = pbsd::lib_libc_inet::b0324;

extern "C" {
in_addr_t ref_inet_addr(const char *cp);
int ref_inet_aton(const char *cp, struct in_addr *addr);
int ref_inet_net_pton(int af, const char *src, void *dst, size_t size);
}

namespace {

inline constexpr unsigned char GUARD = 0x7f;
inline constexpr std::size_t PRE = 32;
inline constexpr std::size_t POST = 32;
inline constexpr std::size_t BIN_CAP = 64;
inline constexpr unsigned long long RANDOM_ITERS = 50000ULL;

struct stats {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
};

stats tbl[] = {
	{ "inet_addr", 0, 0 },
	{ "inet_aton", 0, 0 },
	{ "inet_net_pton", 0, 0 },
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

struct addr_arena {
	unsigned char *refbuf;
	unsigned char *portbuf;
	std::size_t total;
	std::size_t data_off;
};

void
addr_arena_init(addr_arena &ar)
{
	ar.data_off = PRE;
	ar.total = PRE + sizeof(struct in_addr) + POST;
	ar.refbuf = static_cast<unsigned char *>(std::malloc(ar.total));
	ar.portbuf = static_cast<unsigned char *>(std::malloc(ar.total));
	if (ar.refbuf == nullptr || ar.portbuf == nullptr) {
		std::fprintf(stderr, "out of memory\n");
		std::exit(2);
	}
}

void
addr_arena_free(addr_arena &ar)
{
	std::free(ar.refbuf);
	std::free(ar.portbuf);
}

void
addr_arena_prepare(addr_arena &ar)
{
	fill_guard(ar.refbuf, ar.total);
	fill_guard(ar.portbuf, ar.total);
}

struct in_addr *
ref_addr(addr_arena &ar)
{
	return reinterpret_cast<struct in_addr *>(ar.refbuf + ar.data_off);
}

struct in_addr *
port_addr(addr_arena &ar)
{
	return reinterpret_cast<struct in_addr *>(ar.portbuf + ar.data_off);
}

bool
check_addr(const char *src, const char *origin)
{
	tbl[0].cases++;

	in_addr_t rret = ref_inet_addr(src);
	in_addr_t pret = port::inet_addr(src);

	bool ok = (rret == pret);
	if (!ok) {
		tbl[0].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL inet_addr [%s] src=\"%s\" rret=0x%x "
			    "pret=0x%x\n",
			    origin, src ? src : "(null)",
			    static_cast<unsigned>(rret),
			    static_cast<unsigned>(pret));
		}
	}
	return ok;
}

bool
check_aton(const char *src, bool use_null, const char *origin)
{
	tbl[1].cases++;

	addr_arena ar;
	addr_arena_init(ar);
	addr_arena_prepare(ar);

	struct in_addr *ra = ref_addr(ar);
	struct in_addr *pa = port_addr(ar);

	int rret = ref_inet_aton(src, use_null ? nullptr : ra);
	int pret = port::inet_aton(src, use_null ? nullptr : pa);

	bool ok = true;
	if (rret != pret)
		ok = false;
	if (!use_null) {
		if (ra->s_addr != pa->s_addr)
			ok = false;
		if (std::memcmp(ar.refbuf, ar.portbuf, ar.total) != 0)
			ok = false;
	}

	if (!ok) {
		tbl[1].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL inet_aton [%s] src=\"%s\" null=%d "
			    "rret=%d pret=%d\n",
			    origin, src ? src : "(null)", use_null ? 1 : 0,
			    rret, pret);
		}
	}

	addr_arena_free(ar);
	return ok;
}

bool
check_net_pton(int af, const char *src, size_t size, const char *origin)
{
	tbl[2].cases++;

	bin_arena ar;
	bin_arena_init(ar);
	bin_arena_prepare(ar);

	unsigned char *rb = ref_bin(ar);
	unsigned char *pb = port_bin(ar);

	errno = 0;
	int rret = ref_inet_net_pton(af, src, rb, size);
	int rerrno = errno;

	errno = 0;
	int pret = port::inet_net_pton(af, src, pb, size);
	int perrno = errno;

	bool ok = true;
	if (rret != pret)
		ok = false;
	if (rret == -1 && rerrno != perrno)
		ok = false;
	if (std::memcmp(ar.refbuf, ar.portbuf, ar.total) != 0)
		ok = false;

	if (!ok) {
		tbl[2].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL inet_net_pton [%s] af=%d size=%zu "
			    "src=\"%s\" rret=%d pret=%d rerrno=%d perrno=%d\n",
			    origin, af, size, src ? src : "(null)", rret, pret,
			    rerrno, perrno);
		}
	}

	bin_arena_free(ar);
	return ok;
}

void
hand_addr_edges()
{
	static const char *valid[] = {
		"0.0.0.0",
		"255.255.255.255",
		"127.0.0.1",
		"192.168.1.1",
		"10.20.30.40",
		"1.2.3.4",
		"0.0.0.1",
		"001.002.003.004",
		"127.1",
		"127.1.2",
		"127.1.2.3",
		"0x7f000001",
		"0177.0.0.1",
		"2130706433",
		"255.255.255.255",
		"1",
		"1.2",
		"1.2.3",
	};
	static const char *invalid[] = {
		"",
		"x",
		"1.2.3.4.5",
		"1.2.3.256",
		"1.2.3.4x",
		"1..2.3",
		".1.2.3",
		"1.2.3.",
		"256.0.0.0",
		"300.1.1.1",
		"01.02.03.04",
		"1.2.3.08",
		"0x",
		"0xg",
		"08.0.0.1",
		"09.0.0.1",
	};

	for (const char *s : valid)
		check_addr(s, "edge-valid");
	for (const char *s : invalid)
		check_addr(s, "edge-invalid");

	check_addr("127.0.0.1 ", "edge-trailing-space");
	check_addr(" 127.0.0.1", "edge-leading-space");
	check_addr("127.0.0.1\n", "edge-trailing-nl");
	check_addr("\t1.2.3.4", "edge-leading-tab");

	/* high-bit byte in string */
	check_addr("1\x80.2.3.4", "edge-highbit");
}

void
hand_aton_edges()
{
	static const char *valid[] = {
		"0.0.0.0",
		"255.255.255.255",
		"127.0.0.1",
		"192.168.1.1",
		"10.20.30.40",
		"1.2.3.4",
		"127.1",
		"127.1.2",
		"127.1.2.3",
		"0x7f000001",
		"0177.0.0.1",
		"2130706433",
		"0",
		"0xffffffff",
		"255.255",
		"255.255.255",
	};
	static const char *invalid[] = {
		"",
		"x",
		"1.2.3.4.5",
		"1.2.3.256",
		"1.2.3.4x",
		"1..2.3",
		".1.2.3",
		"1.2.3.",
		"256.0.0.0",
		"300.1.1.1",
		"08.0.0.1",
		"09.0.0.1",
		"0x",
		"0xg",
		"1.2.3.08",
	};

	for (const char *s : valid) {
		check_aton(s, false, "edge-valid");
		check_aton(s, true, "edge-valid-null");
	}
	for (const char *s : invalid) {
		check_aton(s, false, "edge-invalid");
		check_aton(s, true, "edge-invalid-null");
	}

	check_aton("127.0.0.1 ", false, "edge-trailing-space");
	check_aton(" 127.0.0.1", false, "edge-leading-space");
	check_aton("127.0.0.1\n", false, "edge-trailing-nl");
	check_aton("\t1.2.3.4", false, "edge-leading-tab");
	check_aton("1\x80.2.3.4", false, "edge-highbit");

	/* octal boundary: 8 and 9 invalid in octal */
	check_aton("08", false, "edge-octal-8");
	check_aton("09", false, "edge-octal-9");
	check_aton("010", false, "edge-octal-10");
	check_aton("07", false, "edge-octal-07");

	/* hex digits */
	check_aton("0xabcdef", false, "edge-hex-lower");
	check_aton("0xABCDEF", false, "edge-hex-upper");
	check_aton("0xAbCdEf01", false, "edge-hex-mixed");

	/* 2/3/4 part boundary values */
	check_aton("1.16777215", false, "edge-2part-max");
	check_aton("1.16777216", false, "edge-2part-over");
	check_aton("1.2.65535", false, "edge-3part-max");
	check_aton("1.2.65536", false, "edge-3part-over");
	check_aton("1.2.3.255", false, "edge-4part-max");
	check_aton("1.2.3.256", false, "edge-4part-over");
}

void
hand_net_pton_edges()
{
	static const char *v4_valid[] = {
		"0.0.0.0",
		"255.255.255.255",
		"192.5.5.240/28",
		"10/8",
		"10.0/16",
		"10.0.0/24",
		"10.0.0.0/32",
		"10.0.0.0",
		"192.168.1.1/24",
		"0/0",
		"255/8",
		"1.2.3.4/1",
		"1.2.3.4/31",
		"1.2.3.4/32",
		"127.0.0.1",
		"224.0.0.1",
		"240.0.0.1",
		"128.0.0.1",
		"192.0.0.1",
		"0x7f000001",
		"0x7f",
		"0x7f00",
		"0x7f0000",
		"0x7f000001",
		"0xabcdef01",
		"0xabc",
		"0xab",
		"0xa",
		"1",
		"10",
		"10.20",
		"10.20.30",
		"10.20.30.40",
	};
	static const char *v4_invalid[] = {
		"",
		"/32",
		"10/",
		"10/33",
		"10.0.0.0/33",
		"256/8",
		"1.2.3.256/8",
		"1.2.3.4/256",
		"1.2.3.4/",
		"1.2.3.4//",
		"1.2.3.4/01",
		"10.0.0.0/24/8",
		".1/8",
		"1./8",
		"1..2/8",
		"10.0.0.1/40",
		"1/40",
		"0xg",
		"0x",
		"x",
		"1.2.3.4.5",
	};

	static const char *v6_valid[] = {
		"::/0",
		"::1/128",
		"2001:db8::/32",
		"2001:db8::1/128",
		"::ffff:192.168.1.1/96",
		"::ffff:10.20.30.40/120",
		"fe80::/10",
		"2001:db8:1::/48",
		"::1",
		"::",
		"2001:db8::1",
	};
	static const char *v6_invalid[] = {
		"",
		":",
		"::/",
		"::/129",
		"::1/129",
		"2001:db8::1::2/64",
		"::ffff:256.1.1.1/96",
		"1:2:3:4:5:6:7:8/129",
		"::/01",
	};

	static const size_t sizes[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 32, 64,
	};

	for (const char *s : v4_valid) {
		for (size_t sz : sizes)
			check_net_pton(AF_INET, s, sz, "edge-v4-valid");
	}
	for (const char *s : v4_invalid) {
		for (size_t sz : sizes)
			check_net_pton(AF_INET, s, sz, "edge-v4-invalid");
	}

	for (const char *s : v6_valid) {
		for (size_t sz : sizes)
			check_net_pton(AF_INET6, s, sz, "edge-v6-valid");
	}
	for (const char *s : v6_invalid) {
		for (size_t sz : sizes)
			check_net_pton(AF_INET6, s, sz, "edge-v6-invalid");
	}

	check_net_pton(99, "10/8", 16, "edge-bad-af");
	check_net_pton(AF_INET, "", 4, "edge-empty");
	check_net_pton(AF_INET6, "", 16, "edge-empty-v6");
	check_net_pton(AF_INET, "10\x80/8", 16, "edge-highbit");

	/* class boundary octets */
	check_net_pton(AF_INET, "127.0.0.1", 4, "edge-class-a");
	check_net_pton(AF_INET, "128.0.0.1", 4, "edge-class-b");
	check_net_pton(AF_INET, "191.255.255.255", 4, "edge-class-b-max");
	check_net_pton(AF_INET, "192.0.0.1", 4, "edge-class-c");
	check_net_pton(AF_INET, "223.255.255.255", 4, "edge-class-c-max");
	check_net_pton(AF_INET, "224.0.0.1", 4, "edge-class-d");
	check_net_pton(AF_INET, "239.255.255.255", 4, "edge-class-d-max");
	check_net_pton(AF_INET, "240.0.0.1", 4, "edge-class-e");
	check_net_pton(AF_INET, "255.255.255.255", 4, "edge-class-e-max");

	/* CIDR boundary bits */
	for (int b = 0; b <= 33; b++) {
		char buf[64];
		std::snprintf(buf, sizeof(buf), "10.0.0.0/%d", b);
		check_net_pton(AF_INET, buf, 8, "edge-cidr-bits");
	}

	/* hex path with odd nybbles */
	check_net_pton(AF_INET, "0x1", 4, "edge-hex-odd");
	check_net_pton(AF_INET, "0x12", 4, "edge-hex-even");
	check_net_pton(AF_INET, "0x123", 4, "edge-hex-odd3");
}

std::string
random_v4_aton_string()
{
	unsigned kind = rand_u32() % 8;
	if (kind == 0) {
		unsigned parts = 1 + (rand_u32() % 4);
		std::string s;
		for (unsigned i = 0; i < parts; i++) {
			if (i > 0)
				s.push_back('.');
			s += std::to_string(rand_u32() % 1000);
		}
		return s;
	}
	if (kind == 1) {
		std::string s = "0x";
		unsigned len = 1 + (rand_u32() % 8);
		static const char *hex = "0123456789abcdefABCDEF";
		for (unsigned i = 0; i < len; i++)
			s.push_back(hex[rand_u32() % 22]);
		return s;
	}
	if (kind == 2) {
		std::string s = "0";
		unsigned len = 1 + (rand_u32() % 12);
		for (unsigned i = 0; i < len; i++)
			s.push_back('0' + (rand_u32() % 10));
		return s;
	}
	if (kind == 3)
		return std::to_string(rand_u32() % 0x100000000ULL);
	if (kind == 4) {
		std::string s;
		unsigned len = rand_u32() % 20;
		for (unsigned i = 0; i < len; i++)
			s.push_back(static_cast<char>(rand_u32() & 0xff));
		return s;
	}
	if (kind == 5)
		return "";
	if (kind == 6) {
		return std::to_string(rand_u32() % 256) + "." +
		    std::to_string(rand_u32() % 256);
	}
	return std::to_string(rand_u32() % 256) + "." +
	    std::to_string(rand_u32() % 256) + "." +
	    std::to_string(rand_u32() % 65536);
}

std::string
random_v4_net_string()
{
	unsigned kind = rand_u32() % 7;
	if (kind == 0) {
		unsigned parts = 1 + (rand_u32() % 4);
		std::string s;
		for (unsigned i = 0; i < parts; i++) {
			if (i > 0)
				s.push_back('.');
			s += std::to_string(rand_u32() % 1000);
		}
		if (rand_u32() & 3) {
			s.push_back('/');
			s += std::to_string(rand_u32() % 40);
		}
		return s;
	}
	if (kind == 1) {
		std::string s = "0x";
		unsigned len = 1 + (rand_u32() % 10);
		static const char *hex = "0123456789abcdefABCDEF";
		for (unsigned i = 0; i < len; i++)
			s.push_back(hex[rand_u32() % 22]);
		if (rand_u32() & 7) {
			s.push_back('/');
			s += std::to_string(rand_u32() % 40);
		}
		return s;
	}
	if (kind == 2) {
		std::string s;
		unsigned len = rand_u32() % 30;
		for (unsigned i = 0; i < len; i++)
			s.push_back(static_cast<char>(rand_u32() & 0xff));
		return s;
	}
	if (kind == 3)
		return "";
	if (kind == 4)
		return std::to_string(rand_u32() % 300);
	if (kind == 5) {
		return std::to_string(rand_u32() % 256) + "/" +
		    std::to_string(rand_u32() % 40);
	}
	return "/" + std::to_string(rand_u32() % 40);
}

std::string
random_v6_net_string()
{
	static const char *hex = "0123456789abcdefABCDEF";
	unsigned kind = rand_u32() % 6;
	if (kind == 0) {
		std::string s = "::";
		unsigned groups = 1 + (rand_u32() % 8);
		for (unsigned i = 0; i < groups; i++) {
			if (!s.empty() && s.back() != ':')
				s.push_back(':');
			unsigned len = 1 + (rand_u32() % 4);
			for (unsigned j = 0; j < len; j++)
				s.push_back(hex[rand_u32() % 22]);
		}
		if (rand_u32() & 3) {
			s.push_back('/');
			s += std::to_string(rand_u32() % 140);
		}
		return s;
	}
	if (kind == 1) {
		std::string s = "::ffff:";
		s += std::to_string(rand_u32() % 256);
		s.push_back('.');
		s += std::to_string(rand_u32() % 256);
		s.push_back('.');
		s += std::to_string(rand_u32() % 256);
		s.push_back('.');
		s += std::to_string(rand_u32() % 256);
		if (rand_u32() & 3) {
			s.push_back('/');
			s += std::to_string(rand_u32() % 140);
		}
		return s;
	}
	if (kind == 2) {
		std::string s;
		unsigned len = rand_u32() % 40;
		for (unsigned i = 0; i < len; i++)
			s.push_back(static_cast<char>(rand_u32() & 0xff));
		return s;
	}
	if (kind == 3)
		return "";
	if (kind == 4)
		return "::/" + std::to_string(rand_u32() % 140);
	return "2001:db8::" + std::to_string(rand_u32() % 1000) + "/" +
	    std::to_string(rand_u32() % 140);
}

void
random_addr_sweep()
{
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		std::string s = random_v4_aton_string();
		check_addr(s.c_str(), "random");
	}
}

void
random_aton_sweep()
{
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		std::string s = random_v4_aton_string();
		check_aton(s.c_str(), (rand_u32() & 3) == 0, "random");
	}
}

void
random_net_pton_sweep()
{
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		std::string s;
		int af;
		unsigned kind = rand_u32() % 3;
		if (kind == 0) {
			s = random_v4_net_string();
			af = AF_INET;
		} else if (kind == 1) {
			s = random_v6_net_string();
			af = AF_INET6;
		} else {
			s = (rand_u32() & 1) ? random_v4_net_string()
					     : random_v6_net_string();
			af = (rand_u32() & 1) ? AF_INET : AF_INET6;
		}
		size_t sz = static_cast<size_t>(rand_u32() % 65);
		check_net_pton(af, s.c_str(), sz, "random");

		if ((rand_u32() & 15) == 0)
			check_net_pton(static_cast<int>(rand_u32() % 256),
			    s.c_str(), sz, "random-af");
	}
}

void
print_table()
{
	std::printf("\n=== b0324 differential test results ===\n");
	std::printf("%-16s %12s %12s\n", "function", "cases", "failures");
	for (const stats &s : tbl)
		std::printf("%-16s %12llu %12llu\n", s.name, s.cases,
		    s.failures);
}

} // namespace

int
main()
{
	hand_addr_edges();
	hand_aton_edges();
	hand_net_pton_edges();
	random_addr_sweep();
	random_aton_sweep();
	random_net_pton_sweep();
	print_table();

	unsigned long long total_fail = 0;
	for (const stats &s : tbl)
		total_fail += s.failures;
	return total_fail == 0 ? 0 : 1;
}
