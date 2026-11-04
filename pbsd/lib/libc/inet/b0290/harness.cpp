/*
 * Batch b0290 differential test.
 */

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <string>
#include <vector>

import pbsd.lib.libc.inet.b0290;

namespace port = pbsd::lib_libc_inet::b0290;

extern "C" {
const char *ref_inet_ntop(int af, const void *src, char *dst, socklen_t size);
int ref_inet_pton(int af, const char *src, void *dst);
int ref_inet_cidr_pton(int af, const char *src, void *dst, int *bits);
char *ref_inet_cidr_ntop(int af, const void *src, int bits, char *dst,
    size_t size);
}

namespace {

inline constexpr unsigned char GUARD = 0x7f;
inline constexpr std::size_t PRE = 32;
inline constexpr std::size_t POST = 32;
inline constexpr std::size_t BIN_CAP = 64;
inline constexpr std::size_t STR_CAP = 256;
inline constexpr unsigned long long RANDOM_ITERS = 200000ULL;

struct stats {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
};

stats tbl[] = {
	{ "inet_ntop", 0, 0 },
	{ "inet_pton", 0, 0 },
	{ "inet_cidr_pton", 0, 0 },
	{ "inet_cidr_ntop", 0, 0 },
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
check_ntop(int af, const unsigned char *src, socklen_t size, const char *origin)
{
	tbl[0].cases++;

	str_arena ar;
	str_arena_init(ar, size + POST);
	str_arena_prepare(ar);

	char *rd = ref_str(ar);
	char *pd = port_str(ar);

	errno = 0;
	const char *rret = ref_inet_ntop(af, src, rd, size);
	int rerrno = errno;

	errno = 0;
	const char *pret = port::inet_ntop(af, src, pd, size);
	int perrno = errno;

	bool ok = true;
	if ((rret == nullptr) != (pret == nullptr))
		ok = false;
	if (rerrno != perrno)
		ok = false;
	if (rret != nullptr && pret != nullptr) {
		if (rret != rd || pret != pd)
			ok = false;
		if (static_cast<std::size_t>(rret - rd) !=
		    static_cast<std::size_t>(pret - pd))
			ok = false;
		if (std::strcmp(rret, pret) != 0)
			ok = false;
	}
	if (std::memcmp(ar.refbuf, ar.portbuf, ar.total) != 0)
		ok = false;

	if (!ok) {
		tbl[0].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL inet_ntop [%s] af=%d size=%u rerrno=%d "
			    "perrno=%d\n",
			    origin, af, static_cast<unsigned>(size), rerrno,
			    perrno);
		}
	}

	str_arena_free(ar);
	return ok;
}

bool
check_pton(int af, const char *src, const char *origin)
{
	tbl[1].cases++;

	bin_arena ar;
	bin_arena_init(ar);
	bin_arena_prepare(ar);

	unsigned char *rb = ref_bin(ar);
	unsigned char *pb = port_bin(ar);

	errno = 0;
	int rret = ref_inet_pton(af, src, rb);
	int rerrno = errno;

	errno = 0;
	int pret = port::inet_pton(af, src, pb);
	int perrno = errno;

	bool ok = true;
	if (rret != pret)
		ok = false;
	if (rret == -1 && rerrno != perrno)
		ok = false;
	if (rret == 1) {
		std::size_t len = (af == AF_INET) ? 4U : 16U;
		if (std::memcmp(rb, pb, len) != 0)
			ok = false;
	}
	if (std::memcmp(ar.refbuf, ar.portbuf, ar.total) != 0)
		ok = false;

	if (!ok) {
		tbl[1].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL inet_pton [%s] af=%d src=\"%s\" "
			    "rret=%d pret=%d rerrno=%d perrno=%d\n",
			    origin, af, src ? src : "(null)", rret, pret,
			    rerrno, perrno);
		}
	}

	bin_arena_free(ar);
	return ok;
}

bool
check_cidr_pton(int af, const char *src, const char *origin)
{
	tbl[2].cases++;

	bin_arena ar;
	bin_arena_init(ar);
	bin_arena_prepare(ar);

	unsigned char *rb = ref_bin(ar);
	unsigned char *pb = port_bin(ar);
	int rbits = -999;
	int pbits = -999;

	errno = 0;
	int rret = ref_inet_cidr_pton(af, src, rb, &rbits);
	int rerrno = errno;

	errno = 0;
	int pret = port::inet_cidr_pton(af, src, pb, &pbits);
	int perrno = errno;

	bool ok = true;
	if (rret != pret)
		ok = false;
	if (rret == -1 && rerrno != perrno)
		ok = false;
	if (rret == 0) {
		if (rbits != pbits)
			ok = false;
		std::size_t len = (af == AF_INET) ? 4U : 16U;
		if (std::memcmp(rb, pb, len) != 0)
			ok = false;
	}
	if (std::memcmp(ar.refbuf, ar.portbuf, ar.total) != 0)
		ok = false;

	if (!ok) {
		tbl[2].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL inet_cidr_pton [%s] af=%d src=\"%s\" "
			    "rret=%d pret=%d rbits=%d pbits=%d\n",
			    origin, af, src ? src : "(null)", rret, pret,
			    rbits, pbits);
		}
	}

	bin_arena_free(ar);
	return ok;
}

bool
check_cidr_ntop(int af, const unsigned char *src, int bits, size_t size,
    const char *origin)
{
	tbl[3].cases++;

	str_arena ar;
	str_arena_init(ar, size + POST);
	str_arena_prepare(ar);

	char *rd = ref_str(ar);
	char *pd = port_str(ar);

	errno = 0;
	char *rret = ref_inet_cidr_ntop(af, src, bits, rd, size);
	int rerrno = errno;

	errno = 0;
	char *pret = port::inet_cidr_ntop(af, src, bits, pd, size);
	int perrno = errno;

	bool ok = true;
	if ((rret == nullptr) != (pret == nullptr))
		ok = false;
	if (rerrno != perrno)
		ok = false;
	if (rret != nullptr && pret != nullptr) {
		if (rret != rd || pret != pd)
			ok = false;
		if (static_cast<std::size_t>(rret - rd) !=
		    static_cast<std::size_t>(pret - pd))
			ok = false;
		if (std::strcmp(rret, pret) != 0)
			ok = false;
	}
	if (std::memcmp(ar.refbuf, ar.portbuf, ar.total) != 0)
		ok = false;

	if (!ok) {
		tbl[3].failures++;
		if (reported < report_limit) {
			reported++;
			std::printf("FAIL inet_cidr_ntop [%s] af=%d bits=%d "
			    "size=%zu rerrno=%d perrno=%d\n",
			    origin, af, bits, size, rerrno, perrno);
		}
	}

	str_arena_free(ar);
	return ok;
}

void
hand_ntop_edges()
{
	unsigned char v4_zeros[4] = { 0, 0, 0, 0 };
	unsigned char v4_all[4] = { 255, 255, 255, 255 };
	unsigned char v4_mixed[4] = { 192, 5, 5, 240 };
	unsigned char v4_high[4] = { 0x80, 0xff, 0x01, 0xfe };

	unsigned char v6_zero[16] = {};
	unsigned char v6_loop[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	    0, 1 };
	unsigned char v6_full[16];
	for (int i = 0; i < 16; i++)
		v6_full[i] = static_cast<unsigned char>(0x80 | (i & 0x7f));

	unsigned char v6_v4map[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff,
		10, 20, 30, 40 };
	unsigned char v6_v4compat[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		2, 3, 4 };

	static const socklen_t sizes[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 15, 16, 17, 20, 24, 32,
		48, 64,
	};

	for (socklen_t sz : sizes) {
		check_ntop(AF_INET, v4_zeros, sz, "edge-v4-zero");
		check_ntop(AF_INET, v4_all, sz, "edge-v4-all");
		check_ntop(AF_INET, v4_mixed, sz, "edge-v4-mixed");
		check_ntop(AF_INET, v4_high, sz, "edge-v4-high");
		check_ntop(AF_INET6, v6_zero, sz, "edge-v6-zero");
		check_ntop(AF_INET6, v6_loop, sz, "edge-v6-loop");
		check_ntop(AF_INET6, v6_full, sz, "edge-v6-full");
		check_ntop(AF_INET6, v6_v4map, sz, "edge-v6-v4map");
		check_ntop(AF_INET6, v6_v4compat, sz, "edge-v6-v4compat");
	}

	check_ntop(99, v4_mixed, 64, "edge-bad-af");
	check_ntop(AF_INET, v4_mixed, 0, "edge-size0");
	check_ntop(AF_INET, v4_mixed, 7, "edge-size7");
	check_ntop(AF_INET, v4_mixed, 8, "edge-size8");
	check_ntop(AF_INET6, v6_loop, 3, "edge-v6-small");
	check_ntop(AF_INET6, v6_loop, 39, "edge-v6-39");
}

void
hand_pton_edges()
{
	static const char *v4_valid[] = {
		"0.0.0.0",
		"255.255.255.255",
		"127.0.0.1",
		"192.168.1.1",
		"10.20.30.40",
		"001.002.003.004",
		"0.0.0.1",
		"1.2.3.4",
	};
	static const char *v4_invalid[] = {
		"",
		"1",
		"1.2",
		"1.2.3",
		"1.2.3.4.5",
		"1.2.3.256",
		"1.2.3.4x",
		"1..2.3",
		".1.2.3",
		"1.2.3.",
		"01.02.03.04",
		"256.0.0.0",
		"x",
		"1.2.3.4.5.6",
		"300.1.1.1",
		"1.2.3.04",
	};

	static const char *v6_valid[] = {
		"::",
		"::1",
		"1::",
		"2001:db8::1",
		"2001:0db8:0000:0000:0000:0000:0000:0001",
		"::ffff:192.168.1.1",
		"::192.168.1.1",
		"ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff",
		"2001:db8:1:2:3:4:5:6",
		"::ffff:10.20.30.40",
		"1:2:3:4:5:6:7:8",
		"fe80::1",
	};
	static const char *v6_invalid[] = {
		"",
		":",
		":::",
		"1:",
		":1:",
		"1::2::3",
		"2001:db8::1::2",
		"ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff",
		"1.2.3.4.5",
		"::ffff:256.1.1.1",
		"::ffff:1.2.3.256",
		"::ffff:1.2.3",
		"1:2:3:4:5:6:7:",
		"1:2:3:4:5:6:7:8:9",
		"abcd",
		"10000::",
	};

	for (const char *s : v4_valid)
		check_pton(AF_INET, s, "edge-v4-valid");
	for (const char *s : v4_invalid)
		check_pton(AF_INET, s, "edge-v4-invalid");

	for (const char *s : v6_valid)
		check_pton(AF_INET6, s, "edge-v6-valid");
	for (const char *s : v6_invalid)
		check_pton(AF_INET6, s, "edge-v6-invalid");

	check_pton(99, "127.0.0.1", "edge-bad-af");
	check_pton(AF_INET, "", "edge-empty");
	check_pton(AF_INET6, "", "edge-empty-v6");

	/* high-bit bytes in string (invalid but must not touch dst) */
	check_pton(AF_INET, "1\x80.2.3.4", "edge-highbit");
	check_pton(AF_INET6, "::\x80", "edge-highbit-v6");
}

void
hand_cidr_pton_edges()
{
	static const char *v4_valid[] = {
		"0.0.0.0/0",
		"255.255.255.255/32",
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
	};
	static const char *v4_invalid[] = {
		"",
		"/32",
		"10/",
		"10/33",
		"10/0",
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

	for (const char *s : v4_valid)
		check_cidr_pton(AF_INET, s, "edge-v4-valid");
	for (const char *s : v4_invalid)
		check_cidr_pton(AF_INET, s, "edge-v4-invalid");

	for (const char *s : v6_valid)
		check_cidr_pton(AF_INET6, s, "edge-v6-valid");
	for (const char *s : v6_invalid)
		check_cidr_pton(AF_INET6, s, "edge-v6-invalid");

	check_cidr_pton(99, "10/8", "edge-bad-af");
	check_cidr_pton(AF_INET, "", "edge-empty");
	check_cidr_pton(AF_INET, "10\x80/8", "edge-highbit");
}

void
hand_cidr_ntop_edges()
{
	unsigned char v4[4] = { 192, 5, 5, 240 };
	unsigned char v4_partial[4] = { 10, 20, 30, 255 };
	unsigned char v4_high[4] = { 0x80, 0xff, 0x01, 0xfe };

	unsigned char v6[16] = {
		0x20, 0x01, 0x0d, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	};
	unsigned char v6_v4map[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff,
		10, 20, 30, 40 };
	unsigned char v6_zero[16] = {};

	static const int v4_bits[] = {
		-1, -2, 0, 1, 7, 8, 9, 15, 16, 17, 23, 24, 25, 31, 32, 33,
	};
	static const int v6_bits[] = { -1, -2, 0, 1, 64, 96, 112, 120, 127,
		128, 129 };
	static const size_t sizes[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 15, 16, 17, 20, 24,
		32, 48, 64,
	};

	for (int bits : v4_bits) {
		for (size_t sz : sizes) {
			check_cidr_ntop(AF_INET, v4, bits, sz, "edge-v4");
			check_cidr_ntop(AF_INET, v4_partial, bits, sz,
			    "edge-v4-partial");
			check_cidr_ntop(AF_INET, v4_high, bits, sz,
			    "edge-v4-high");
		}
	}

	for (int bits : v6_bits) {
		for (size_t sz : sizes) {
			check_cidr_ntop(AF_INET6, v6, bits, sz, "edge-v6");
			check_cidr_ntop(AF_INET6, v6_v4map, bits, sz,
			    "edge-v6-v4map");
			check_cidr_ntop(AF_INET6, v6_zero, bits, sz,
			    "edge-v6-zero");
		}
	}

	check_cidr_ntop(99, v4, 24, 64, "edge-bad-af");
	check_cidr_ntop(AF_INET, v4, 28, 64, "edge-v4-28");
	check_cidr_ntop(AF_INET, v4, 0, 8, "edge-v4-bits0");
	check_cidr_ntop(AF_INET, v4, 32, 64, "edge-v4-bits32");
	check_cidr_ntop(AF_INET6, v6, 128, 64, "edge-v6-128");

	for (int b = 1; b <= 31; b++)
		check_cidr_ntop(AF_INET, v4_partial, b, 64, "edge-partial");
}

std::string
random_v4_string()
{
	unsigned parts = 1 + (rand_u32() % 4);
	std::string s;
	for (unsigned i = 0; i < parts; i++) {
		if (i > 0)
			s.push_back('.');
		unsigned val = rand_u32() % 1000;
		s += std::to_string(val);
	}
	if (rand_u32() & 3) {
		s.push_back('/');
		s += std::to_string(rand_u32() % 40);
	}
	return s;
}

std::string
random_v6_string()
{
	static const char *hex = "0123456789abcdefABCDEF";
	unsigned groups = 1 + (rand_u32() % 8);
	std::string s;
	if (rand_u32() & 1)
		s = "::";
	for (unsigned i = 0; i < groups; i++) {
		if (!s.empty() && s.back() != ':')
			s.push_back(':');
		unsigned len = 1 + (rand_u32() % 4);
		for (unsigned j = 0; j < len; j++)
			s.push_back(hex[rand_u32() % 22]);
	}
	if (rand_u32() & 7 == 0) {
		s += ":ffff:";
		s += std::to_string(rand_u32() % 256);
		s.push_back('.');
		s += std::to_string(rand_u32() % 256);
		s.push_back('.');
		s += std::to_string(rand_u32() % 256);
		s.push_back('.');
		s += std::to_string(rand_u32() % 256);
	}
	if (rand_u32() & 3) {
		s.push_back('/');
		s += std::to_string(rand_u32() % 140);
	}
	return s;
}

std::string
random_garbage_string()
{
	std::string s;
	unsigned len = rand_u32() % 40;
	for (unsigned i = 0; i < len; i++)
		s.push_back(static_cast<char>(rand_u32() & 0xff));
	return s;
}

void
random_ntop_sweep()
{
	unsigned char v4[4];
	unsigned char v6[16];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		for (unsigned j = 0; j < 4; j++)
			v4[j] = static_cast<unsigned char>(rand_u32() & 0xff);
		for (unsigned j = 0; j < 16; j++)
			v6[j] = static_cast<unsigned char>(rand_u32() & 0xff);

		socklen_t sz = static_cast<socklen_t>(rand_u32() % 65);
		check_ntop(AF_INET, v4, sz, "random");
		check_ntop(AF_INET6, v6, sz, "random");

		if ((rand_u32() & 15) == 0)
			check_ntop(static_cast<int>(rand_u32() % 256), v4, sz,
			    "random-af");
	}
}

void
random_pton_sweep()
{
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		std::string s;
		unsigned kind = rand_u32() % 5;
		if (kind == 0)
			s = random_v4_string();
		else if (kind == 1)
			s = random_v6_string();
		else if (kind == 2)
			s = random_garbage_string();
		else if (kind == 3)
			s = "";
		else {
			s = std::to_string(rand_u32() % 1000);
			if (rand_u32() & 1)
				s.push_back(static_cast<char>(
				    0x80 | (rand_u32() & 0x7f)));
		}

		check_pton(AF_INET, s.c_str(), "random");
		check_pton(AF_INET6, s.c_str(), "random");

		if ((rand_u32() & 15) == 0)
			check_pton(static_cast<int>(rand_u32() % 256),
			    s.c_str(), "random-af");
	}
}

void
random_cidr_pton_sweep()
{
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		std::string s;
		unsigned kind = rand_u32() % 5;
		if (kind == 0)
			s = random_v4_string();
		else if (kind == 1)
			s = random_v6_string();
		else if (kind == 2)
			s = random_garbage_string();
		else if (kind == 3)
			s = "";
		else
			s = std::to_string(rand_u32() % 300);

		check_cidr_pton(AF_INET, s.c_str(), "random");
		check_cidr_pton(AF_INET6, s.c_str(), "random");

		if ((rand_u32() & 15) == 0)
			check_cidr_pton(static_cast<int>(rand_u32() % 256),
			    s.c_str(), "random-af");
	}
}

void
random_cidr_ntop_sweep()
{
	unsigned char v4[4];
	unsigned char v6[16];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		for (unsigned j = 0; j < 4; j++)
			v4[j] = static_cast<unsigned char>(rand_u32() & 0xff);
		for (unsigned j = 0; j < 16; j++)
			v6[j] = static_cast<unsigned char>(rand_u32() & 0xff);

		int bits4 = static_cast<int>(rand_u32() % 40) - 4;
		int bits6 = static_cast<int>(rand_u32() % 140) - 6;
		size_t sz = static_cast<size_t>(rand_u32() % 65);

		check_cidr_ntop(AF_INET, v4, bits4, sz, "random");
		check_cidr_ntop(AF_INET6, v6, bits6, sz, "random");

		if ((rand_u32() & 15) == 0)
			check_cidr_ntop(static_cast<int>(rand_u32() % 256),
			    v4, bits4, sz, "random-af");
	}
}

void
print_table()
{
	std::printf("\n=== b0290 differential test results ===\n");
	std::printf("%-16s %12s %12s\n", "function", "cases", "failures");
	for (const stats &s : tbl)
		std::printf("%-16s %12llu %12llu\n", s.name, s.cases,
		    s.failures);
}

} // namespace

int
main()
{
	hand_ntop_edges();
	hand_pton_edges();
	hand_cidr_pton_edges();
	hand_cidr_ntop_edges();
	random_ntop_sweep();
	random_pton_sweep();
	random_cidr_pton_sweep();
	random_cidr_ntop_sweep();
	print_table();

	unsigned long long total_fail = 0;
	for (const stats &s : tbl)
		total_fail += s.failures;
	return total_fail == 0 ? 0 : 1;
}
