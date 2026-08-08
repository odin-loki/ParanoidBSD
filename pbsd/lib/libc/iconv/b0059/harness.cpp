/*
 * harness.cpp -- differential test for PBSD batch b0059.
 *
 * Compares the C++23 module port against the unmodified C oracle for:
 *	bcs_strtol, bcs_strtoul, db_hash_std, map_file, unmap_file
 */

#include <sys/stat.h>
#include <sys/types.h>

#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>

#include <fcntl.h>
#include <unistd.h>

import pbsd.lib.libc.iconv.b0059;

namespace P = pbsd::lib_libc_iconv::b0059;

extern "C" {
long ref__bcs_strtol(const char *nptr, char **endptr, int base);
unsigned long ref__bcs_strtoul(const char *nptr, char **endptr, int base);
uint32_t ref__citrus_db_hash_std(P::citrus_region *r);
int ref__citrus_map_file(P::citrus_region * __restrict r,
    const char * __restrict path);
void ref__citrus_unmap_file(P::citrus_region *r);
}

/* ------------------------------------------------------------------ stats */

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	unsigned long long printed;
};

static Stat st_strtol = { "bcs_strtol", 0, 0, 0 };
static Stat st_strtoul = { "bcs_strtoul", 0, 0, 0 };
static Stat st_hash = { "db_hash_std", 0, 0, 0 };
static Stat st_map = { "map_file", 0, 0, 0 };
static Stat st_unmap = { "unmap_file", 0, 0, 0 };

static const unsigned long long MAX_PRINT = 8;
static const unsigned char GUARD = 0x7f;
static const size_t BUFSZ = 128;

static void
fail(Stat &s, const char *detail)
{
	s.fails++;
	if (s.printed < MAX_PRINT) {
		s.printed++;
		std::fprintf(stderr, "FAIL %s: %s\n", s.name, detail);
	}
}

static std::string
escape(const char *p, size_t n)
{
	std::string out;
	char tmp[8];
	for (size_t i = 0; i < n; i++) {
		unsigned char c = (unsigned char)p[i];
		if (c >= 0x20 && c < 0x7f && c != '\\' && c != '"') {
			out.push_back((char)c);
		} else {
			std::snprintf(tmp, sizeof(tmp), "\\x%02x", c);
			out += tmp;
		}
	}
	return out;
}

/* --------------------------------------------------------------- rng ----- */

static uint64_t rng_state;

static void
rng_seed(uint64_t s)
{
	rng_state = s;
}

static uint64_t
rng_next(void)
{
	uint64_t z = (rng_state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

static uint32_t
rng_below(uint32_t n)
{
	return (uint32_t)(rng_next() % (uint64_t)n);
}

/* ------------------------------------------------------- strto* checking - */

static void
check_strtol(const std::string &in, int base, bool use_endptr)
{
	char a[BUFSZ], b[BUFSZ];
	char detail[512];

	st_strtol.cases++;

	std::memset(a, GUARD, BUFSZ);
	std::memset(b, GUARD, BUFSZ);

	size_t n = in.size();
	if (n > BUFSZ - 2)
		n = BUFSZ - 2;
	std::memcpy(a, in.data(), n);
	std::memcpy(b, in.data(), n);
	a[n] = '\0';
	b[n] = '\0';

	char *ea = (char *)((uintptr_t)a + BUFSZ + 13);
	char *eb = (char *)((uintptr_t)b + BUFSZ + 13);

	errno = 0;
	long ra = P::bcs_strtol(a, use_endptr ? &ea : (char **)0, base);
	int erra = errno;

	errno = 0;
	long rb = ref__bcs_strtol(b, use_endptr ? &eb : (char **)0, base);
	int errb = errno;

	if (ra != rb) {
		std::snprintf(detail, sizeof(detail),
		    "input=\"%s\" base=%d ret port=%ld ref=%ld",
		    escape(in.data(), in.size()).c_str(), base,
		    (long)ra, (long)rb);
		fail(st_strtol, detail);
		return;
	}
	if (erra != errb) {
		std::snprintf(detail, sizeof(detail),
		    "input=\"%s\" base=%d errno port=%d ref=%d",
		    escape(in.data(), in.size()).c_str(), base, erra, errb);
		fail(st_strtol, detail);
		return;
	}
	if (use_endptr) {
		long oa = (long)((uintptr_t)ea - (uintptr_t)a);
		long ob = (long)((uintptr_t)eb - (uintptr_t)b);
		if (oa != ob) {
			std::snprintf(detail, sizeof(detail),
			    "input=\"%s\" base=%d endptr off port=%ld ref=%ld",
			    escape(in.data(), in.size()).c_str(), base, oa, ob);
			fail(st_strtol, detail);
			return;
		}
	}
	if (std::memcmp(a, b, BUFSZ) != 0) {
		std::snprintf(detail, sizeof(detail),
		    "input=\"%s\" base=%d buffer diverged",
		    escape(in.data(), in.size()).c_str(), base);
		fail(st_strtol, detail);
	}
}

static void
check_strtoul(const std::string &in, int base, bool use_endptr)
{
	char a[BUFSZ], b[BUFSZ];
	char detail[512];

	st_strtoul.cases++;

	std::memset(a, GUARD, BUFSZ);
	std::memset(b, GUARD, BUFSZ);

	size_t n = in.size();
	if (n > BUFSZ - 2)
		n = BUFSZ - 2;
	std::memcpy(a, in.data(), n);
	std::memcpy(b, in.data(), n);
	a[n] = '\0';
	b[n] = '\0';

	char *ea = (char *)((uintptr_t)a + BUFSZ + 13);
	char *eb = (char *)((uintptr_t)b + BUFSZ + 13);

	errno = 0;
	unsigned long ra = P::bcs_strtoul(a, use_endptr ? &ea : (char **)0, base);
	int erra = errno;

	errno = 0;
	unsigned long rb = ref__bcs_strtoul(b, use_endptr ? &eb : (char **)0, base);
	int errb = errno;

	if (ra != rb) {
		std::snprintf(detail, sizeof(detail),
		    "input=\"%s\" base=%d ret port=%lu ref=%lu",
		    escape(in.data(), in.size()).c_str(), base,
		    (unsigned long)ra, (unsigned long)rb);
		fail(st_strtoul, detail);
		return;
	}
	if (erra != errb) {
		std::snprintf(detail, sizeof(detail),
		    "input=\"%s\" base=%d errno port=%d ref=%d",
		    escape(in.data(), in.size()).c_str(), base, erra, errb);
		fail(st_strtoul, detail);
		return;
	}
	if (use_endptr) {
		long oa = (long)((uintptr_t)ea - (uintptr_t)a);
		long ob = (long)((uintptr_t)eb - (uintptr_t)b);
		if (oa != ob) {
			std::snprintf(detail, sizeof(detail),
			    "input=\"%s\" base=%d endptr off port=%ld ref=%ld",
			    escape(in.data(), in.size()).c_str(), base, oa, ob);
			fail(st_strtoul, detail);
			return;
		}
	}
	if (std::memcmp(a, b, BUFSZ) != 0) {
		std::snprintf(detail, sizeof(detail),
		    "input=\"%s\" base=%d buffer diverged",
		    escape(in.data(), in.size()).c_str(), base);
		fail(st_strtoul, detail);
	}
}

static void
check_both_strto(const std::string &in, int base)
{
	check_strtol(in, base, true);
	check_strtol(in, base, false);
	check_strtoul(in, base, true);
	check_strtoul(in, base, false);
}

/* ------------------------------------------------------- db_hash checking */

struct RegionWrap {
	unsigned char	pre[16];
	P::citrus_region	r;
	unsigned char	post[16];
};

static void
check_db_hash(const unsigned char *data, size_t len)
{
	char detail[512];
	RegionWrap wp, wr;
	unsigned char bufp[256], bufr[256];

	st_hash.cases++;

	if (len > sizeof(bufp))
		len = sizeof(bufp);

	std::memset(&wp, GUARD, sizeof(wp));
	std::memset(&wr, GUARD, sizeof(wr));
	std::memset(bufp, GUARD, sizeof(bufp));
	std::memset(bufr, GUARD, sizeof(bufr));
	std::memcpy(bufp, data, len);
	std::memcpy(bufr, data, len);

	wp.r.r_head = bufp;
	wp.r.r_size = len;
	wr.r.r_head = bufr;
	wr.r.r_size = len;

	uint32_t hp = P::db_hash_std(&wp.r);
	uint32_t hr = ref__citrus_db_hash_std(&wr.r);

	if (hp != hr) {
		std::snprintf(detail, sizeof(detail),
		    "len=%zu hash port=0x%08x ref=0x%08x data=\"%s\"",
		    len, hp, hr, escape((const char *)data, len).c_str());
		fail(st_hash, detail);
		return;
	}
	if (wp.r.r_size != wr.r.r_size) {
		fail(st_hash, "region size diverged");
		return;
	}
	if (std::memcmp(bufp, bufr, sizeof(bufp)) != 0) {
		fail(st_hash, "source buffer diverged");
	}
}

/* ------------------------------------------------------- mmap checking --- */

static void
check_unmap_pair(P::citrus_region *rp, P::citrus_region *rr,
    RegionWrap *wp, RegionWrap *wr)
{
	char detail[256];

	st_unmap.cases++;

	P::unmap_file(rp);
	ref__citrus_unmap_file(rr);

	if ((rp->r_head != nullptr) != (rr->r_head != nullptr) ||
	    rp->r_size != rr->r_size) {
		std::snprintf(detail, sizeof(detail),
		    "region port={head=%p,size=%zu} ref={head=%p,size=%zu}",
		    rp->r_head, rp->r_size, rr->r_head, rr->r_size);
		fail(st_unmap, detail);
		return;
	}
}

static int
write_temp_file(const unsigned char *data, size_t len, char *path, size_t pathsz)
{
	std::snprintf(path, pathsz, "/tmp/pbsd_b0059_XXXXXX");
	int fd = mkstemp(path);
	if (fd < 0)
		return (-1);
	if (len > 0 && write(fd, data, len) != (ssize_t)len) {
		close(fd);
		unlink(path);
		return (-1);
	}
	close(fd);
	return (0);
}

static void
check_map_file(const char *path)
{
	char detail[512];
	RegionWrap wp, wr;

	st_map.cases++;

	std::memset(&wp, GUARD, sizeof(wp));
	std::memset(&wr, GUARD, sizeof(wr));

	int rp = P::map_file(&wp.r, path);
	int rr = ref__citrus_map_file(&wr.r, path);

	if (rp != rr) {
		std::snprintf(detail, sizeof(detail),
		    "path=\"%s\" ret port=%d ref=%d", path, rp, rr);
		fail(st_map, detail);
		goto unmap;
	}

	if (wp.r.r_size != wr.r.r_size) {
		std::snprintf(detail, sizeof(detail),
		    "path=\"%s\" size port=%zu ref=%zu",
		    path, wp.r.r_size, wr.r.r_size);
		fail(st_map, detail);
		goto unmap;
	}

	if (rp == 0 && wp.r.r_size > 0) {
		if (std::memcmp(wp.r.r_head, wr.r.r_head, wp.r.r_size) != 0) {
			std::snprintf(detail, sizeof(detail),
			    "path=\"%s\" mapped content diverged", path);
			fail(st_map, detail);
		}
	}

unmap:
	check_unmap_pair(&wp.r, &wr.r, &wp, &wr);
}

/* ------------------------------------------------------ input generation - */

static std::string
fmt_base(unsigned long long v, int base, bool upper)
{
	const char *lo = "0123456789abcdefghijklmnopqrstuvwxyz";
	const char *up = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	const char *tab = upper ? up : lo;
	std::string s;

	if (base < 2 || base > 36)
		base = 10;
	if (v == 0)
		return std::string("0");
	while (v != 0) {
		s.push_back(tab[v % (unsigned long long)base]);
		v /= (unsigned long long)base;
	}
	for (size_t i = 0, j = s.size() - 1; i < j; i++, j--) {
		char t = s[i];
		s[i] = s[j];
		s[j] = t;
	}
	return s;
}

static const char *edge_strings[] = {
	"", " ", "\t", "\n", "\v", "\f", "\r", " \t\n\v\f\r ",
	"+", "-", "++1", "--1", "+-1", "-+1",
	"0", "1", "9", "a", "z", "A", "Z", "g", "G",
	"0b", "0B", "0x", "0X", "0x0", "0x1", "0xg", "0xf",
	"-0x1", "+0x7f", "  0x10", "   -12", "\t+42",
	"12abc", "abc", " abc", "1.5", "-1",
	"7fffffffffffffff", "8000000000000000",
	"ffffffffffffffff", "10000000000000000",
	"0x7fffffffffffffff", "0x8000000000000000",
	"9223372036854775807", "9223372036854775808",
	"-9223372036854775808", "-9223372036854775809",
	"18446744073709551615", "18446744073709551616",
	"-18446744073709551615",
	"\x80", "\xff", "\x7f", "\x81\x82", "\xa0",
	"1\x80" "2", "\x80" "1", " \xff 1",
	"0b1", "0B10", "0b", "0B",
	"1234567890123456789012345678901234567890",
};

static const int edge_bases[] = {
	0, 1, 2, 3, 8, 10, 16, 36, 37, -1, INT_MAX, INT_MIN,
};

static void
edge_cases_strto(void)
{
	size_t ns = sizeof(edge_strings) / sizeof(edge_strings[0]);
	size_t nb = sizeof(edge_bases) / sizeof(edge_bases[0]);

	for (size_t i = 0; i < ns; i++) {
		std::string s(edge_strings[i]);
		for (size_t j = 0; j < nb; j++)
			check_both_strto(s, edge_bases[j]);
	}

	for (int c = 0; c <= 0xff; c++) {
		std::string one(1, (char)(unsigned char)c);
		std::string two = std::string("1") + one;
		check_both_strto(one, 0);
		check_both_strto(one, 10);
		check_both_strto(one, 16);
		check_both_strto(two, 0);
		check_both_strto(two, 10);
	}

	for (size_t len = 0; len <= 40; len++) {
		std::string s(len, '9');
		check_both_strto(s, 10);
		check_both_strto(std::string("-") + s, 10);
		std::string f(len, 'f');
		check_both_strto(std::string("0x") + f, 0);
		std::string z(len, 'z');
		check_both_strto(z, 36);
	}

	for (int base = 2; base <= 36; base++) {
		unsigned long long marks[] = {
			0ULL, 1ULL,
			(unsigned long long)LONG_MAX - 1,
			(unsigned long long)LONG_MAX,
			(unsigned long long)LONG_MAX + 1,
			ULONG_MAX - 1, ULONG_MAX,
		};
		for (size_t k = 0; k < sizeof(marks) / sizeof(marks[0]); k++) {
			std::string s = fmt_base(marks[k], base, false);
			check_both_strto(s, base);
			check_both_strto(std::string("-") + s, base);
			check_both_strto(s + "0", base);
		}
	}
}

static void
edge_cases_hash(void)
{
	static const unsigned char empty[] = "";
	static const unsigned char one_a[] = "a";
	static const unsigned char one_A[] = "A";
	static const unsigned char hi[] = "\x80\xff\xfe";
	static const unsigned char mix[] = "AbC\x80\xFF";

	check_db_hash(empty, 0);
	check_db_hash(one_a, 1);
	check_db_hash(one_A, 1);
	check_db_hash(hi, sizeof(hi) - 1);
	check_db_hash(mix, sizeof(mix) - 1);

	for (int c = 0; c <= 0xff; c++) {
		unsigned char b[1] = { (unsigned char)c };
		check_db_hash(b, 1);
	}

	for (size_t len = 0; len <= 64; len++) {
		std::vector<unsigned char> v(len);
		for (size_t i = 0; i < len; i++)
			v[i] = (unsigned char)(0x41 + (i % 26));
		check_db_hash(v.data(), len);
	}

	{
		std::vector<unsigned char> v(32);
		for (size_t i = 0; i < v.size(); i++)
			v[i] = (unsigned char)(0x80 + (i * 17) % 128);
		check_db_hash(v.data(), v.size());
	}
}

static void
edge_cases_mmap(void)
{
	char path[64];
	unsigned char data[] = "hello mmap batch b0059";

	if (write_temp_file(data, sizeof(data) - 1, path, sizeof(path)) == 0) {
		check_map_file(path);
		unlink(path);
	}

	if (write_temp_file(nullptr, 0, path, sizeof(path)) == 0) {
		check_map_file(path);
		unlink(path);
	}

	check_map_file("/nonexistent/pbsd_b0059_path");
	check_map_file("/tmp");
	check_map_file("");

	{
		RegionWrap wp, wr;
		std::memset(&wp, GUARD, sizeof(wp));
		std::memset(&wr, GUARD, sizeof(wr));
		check_unmap_pair(&wp.r, &wr.r, &wp, &wr);
	}
}

/* ---------------------------------------------------------- random sweeps */

static std::string
random_string(void)
{
	static const char alpha[] =
	    "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
	    "  \t\n\v\f\r++--xXbB";
	std::string s;
	uint32_t mode = rng_below(5);

	switch (mode) {
	case 0: {
		uint32_t len = rng_below(24);
		for (uint32_t i = 0; i < len; i++)
			s.push_back((char)(unsigned char)rng_below(256));
		break;
	}
	case 1: {
		uint32_t len = rng_below(24);
		for (uint32_t i = 0; i < len; i++)
			s.push_back(alpha[rng_below((uint32_t)(sizeof(alpha) - 1))]);
		break;
	}
	case 2: {
		unsigned long long v = rng_next();
		int base = 2 + (int)rng_below(35);
		if (rng_below(3) == 1)
			s.push_back('-');
		else if (rng_below(3) == 2)
			s.push_back('+');
		if (base == 16 && rng_below(2))
			s += "0x";
		if (base == 2 && rng_below(2))
			s += "0b";
		s += fmt_base(v >> (rng_below(32)), base, rng_below(2) != 0);
		break;
	}
	default: {
		uint32_t len = rng_below(4);
		for (uint32_t i = 0; i < len; i++) {
			if (rng_below(2))
				s.push_back((char)(unsigned char)(0x80 + rng_below(128)));
			else
				s.push_back(alpha[rng_below((uint32_t)(sizeof(alpha) - 1))]);
		}
		break;
	}
	}
	return s;
}

static int
random_base(void)
{
	switch (rng_below(8)) {
	case 0: return 0;
	case 1: return 10;
	case 2: return 16;
	case 3: return 8;
	case 4: return 2;
	case 5: return 36;
	case 6: return (int)rng_below(40) - 2;
	default: return (int)rng_below(37);
	}
}

static const unsigned long long RANDOM_ITERS = 200000;

static void
random_sweep_strto(void)
{
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		std::string s = random_string();
		int base = random_base();
		bool ep = (rng_below(4) != 0);
		check_strtol(s, base, ep);
		check_strtoul(s, base, ep);
	}
}

static void
random_sweep_hash(void)
{
	unsigned char buf[256];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		uint32_t len = rng_below((uint32_t)sizeof(buf));
		for (uint32_t j = 0; j < len; j++)
			buf[j] = (unsigned char)rng_below(256);
		check_db_hash(buf, len);
	}
}

static void
random_sweep_mmap(void)
{
	char path[64];
	unsigned char buf[512];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		uint32_t len = rng_below((uint32_t)sizeof(buf));
		for (uint32_t j = 0; j < len; j++)
			buf[j] = (unsigned char)rng_below(256);

		if (rng_below(16) == 0) {
			check_map_file("/nonexistent/pbsd_b0059");
			continue;
		}
		if (rng_below(32) == 0) {
			check_map_file("/tmp");
			continue;
		}

		if (write_temp_file(buf, len, path, sizeof(path)) != 0)
			continue;
		check_map_file(path);
		unlink(path);
	}
}

/* --------------------------------------------------------------- reporting */

static void
print_row(const Stat &s)
{
	std::printf("  %-14s %14llu %14llu  %s\n", s.name, s.cases, s.fails,
	    s.fails == 0 ? "ok" : "FAILED");
}

int
main(void)
{
	rng_seed(0xB0059C0FFEEULL);

	edge_cases_strto();
	edge_cases_hash();
	edge_cases_mmap();

	random_sweep_strto();
	random_sweep_hash();
	random_sweep_mmap();

	std::printf("\n  %-14s %14s %14s  %s\n", "function", "cases",
	    "failures", "status");
	std::printf("  -------------------------------------------------------\n");
	print_row(st_strtol);
	print_row(st_strtoul);
	print_row(st_hash);
	print_row(st_map);
	print_row(st_unmap);

	unsigned long long total_cases = st_strtol.cases + st_strtoul.cases +
	    st_hash.cases + st_map.cases + st_unmap.cases;
	unsigned long long total_fails = st_strtol.fails + st_strtoul.fails +
	    st_hash.fails + st_map.fails + st_unmap.fails;

	std::printf("  -------------------------------------------------------\n");
	std::printf("  %-14s %14llu %14llu  %s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0 ? "ok" : "FAILED");

	return (total_fails == 0 ? 0 : 1);
}
