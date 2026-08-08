/*
 * harness.cpp -- differential test for PBSD batch b0160.
 */

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.sbin.ipf.libipf.b0160;

namespace P = pbsd::sbin_ipf_libipf::b0160;

extern "C" {
extern char *ref_icmpcodes[];
P::mb_t *ref_dupmbt(P::mb_t *orig);
char *ref_icmptypename(int family, int type);
P::u_32_t ref_getoptbyname(char *optname);
P::u_32_t ref_getoptbyvalue(int optval);
}

static constexpr std::size_t MB_BUF_WORDS = 2048;
static constexpr unsigned char GUARD = 0x7f;
static constexpr int MAX_REPORT = 8;
static constexpr long SWEEP = 200000;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

static Stat st_icmpcodes = { "icmpcodes", 0, 0, 0 };
static Stat st_dupmbt = { "dupmbt", 0, 0, 0 };
static Stat st_icmptypename = { "icmptypename", 0, 0, 0 };
static Stat st_getoptbyname = { "getoptbyname", 0, 0, 0 };
static Stat st_getoptbyvalue = { "getoptbyvalue", 0, 0, 0 };

static std::uint64_t rng_state = 0xb0160feedfaceULL;

static inline std::uint64_t
rnd(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static inline std::size_t
rnd_mod(std::size_t m)
{
	if (m == 0)
		return 0;
	return (std::size_t)(rnd() % (std::uint64_t)m);
}

static void
stat_fail(Stat *st, const char *tag, const char *detail)
{
	st->fails++;
	if (st->reported < MAX_REPORT) {
		st->reported++;
		std::printf("  FAIL %s [%s] %s\n", st->name, tag, detail);
	}
}

static const char *
str_or_null(const char *s)
{
	return s != nullptr ? s : "(null)";
}

static int
streq_or_both_null(const char *a, const char *b)
{
	if (a == nullptr && b == nullptr)
		return 1;
	if (a == nullptr || b == nullptr)
		return 0;
	return std::strcmp(a, b) == 0;
}

/* ------------------------------------------------------------------------ */
/* icmpcodes (data from icmpcode.c)                                          */
/* ------------------------------------------------------------------------ */

static void
icmpcodes_case(const char *tag, int idx)
{
	const char *got = (idx >= 0 && idx <= P::MAX_ICMPCODE) ?
	    P::icmpcodes[idx] :
	    nullptr;
	const char *ref = (idx >= 0 && idx <= P::MAX_ICMPCODE) ?
	    ref_icmpcodes[idx] :
	    nullptr;

	st_icmpcodes.cases++;
	if (!streq_or_both_null(got, ref)) {
		stat_fail(&st_icmpcodes, tag, "string");
		if (st_icmpcodes.reported <= MAX_REPORT) {
			std::printf("      idx=%d port=%s ref=%s\n", idx,
			    str_or_null(got), str_or_null(ref));
		}
	}
}

static void
test_icmpcodes_edges(void)
{
	icmpcodes_case("null-term", P::MAX_ICMPCODE);
	for (int i = 0; i < P::MAX_ICMPCODE; i++)
		icmpcodes_case("idx", i);
	icmpcodes_case("first", 0);
	icmpcodes_case("last", P::MAX_ICMPCODE - 1);
}

static void
test_icmpcodes_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		int idx = (int)(rnd_mod((std::size_t)P::MAX_ICMPCODE + 1));
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		icmpcodes_case(tag, idx);
	}
}

/* ------------------------------------------------------------------------ */
/* dupmbt                                                                    */
/* ------------------------------------------------------------------------ */

static void
init_mb(P::mb_t *m, int len, std::ptrdiff_t data_off, const unsigned char *pat,
    std::size_t patlen)
{
	std::memset(m, GUARD, sizeof(*m));
	m->mb_len = len;
	m->mb_next = (P::mb_t *)(uintptr_t)0xdeadbeefUL;
	m->mb_data = (char *)m->mb_buf + data_off;
	for (std::size_t i = 0; i < sizeof(m->mb_buf); i++)
		m->mb_buf[i] = GUARD ^ (unsigned long)(i & 0xff);
	if (len > 0 && pat != nullptr) {
		std::size_t n = (std::size_t)len;
		if (patlen < n)
			n = patlen;
		std::memcpy(m->mb_data, pat, n);
	}
}

static void
dupmbt_case(const char *tag, int len, std::ptrdiff_t data_off,
    const unsigned char *pat, std::size_t patlen)
{
	P::mb_t orig_p, orig_r;
	unsigned char guard_p[sizeof(P::mb_t)];
	unsigned char guard_r[sizeof(P::mb_t)];

	init_mb(&orig_p, len, data_off, pat, patlen);
	init_mb(&orig_r, len, data_off, pat, patlen);
	std::memcpy(guard_p, &orig_p, sizeof(orig_p));
	std::memcpy(guard_r, &orig_r, sizeof(orig_r));

	P::mb_t *got = P::dupmbt(&orig_p);
	P::mb_t *ref = ref_dupmbt(&orig_r);

	st_dupmbt.cases++;

	int bad = 0;
	if ((got == nullptr) != (ref == nullptr))
		bad = 1;
	if (std::memcmp(&orig_p, guard_p, sizeof(orig_p)) != 0)
		bad = 1;
	if (std::memcmp(&orig_r, guard_r, sizeof(orig_r)) != 0)
		bad = 1;

	if (!bad && got != nullptr && ref != nullptr) {
		std::ptrdiff_t off_got = got->mb_data - (char *)got->mb_buf;
		std::ptrdiff_t off_ref = ref->mb_data - (char *)ref->mb_buf;
		if (got->mb_len != ref->mb_len || got->mb_next != nullptr ||
		    ref->mb_next != nullptr || off_got != off_ref)
			bad = 1;
		if (std::memcmp(got->mb_buf, ref->mb_buf, sizeof(got->mb_buf)) != 0)
			bad = 1;
		if (len > 0 &&
		    std::memcmp(got->mb_data, ref->mb_data, (std::size_t)len) != 0)
			bad = 1;
	}

	if (bad) {
		stat_fail(&st_dupmbt, tag, "dup");
		if (st_dupmbt.reported <= MAX_REPORT) {
			std::printf(
			    "      len=%d off=%td got=%p ref=%p\n", len,
			    (ptrdiff_t)data_off, (void *)got, (void *)ref);
		}
	}

	std::free(got);
	std::free(ref);
}

static void
test_dupmbt_edges(void)
{
	static const unsigned char empty[] = { 0 };
	static const unsigned char one[] = { 0x00 };
	static const unsigned char hi[] = { 0x80, 0xff, 0xfe, 0x81 };
	static const unsigned char nulheavy[] = { 0x00, 0x00, 0x7f, 0x00, 0xff };

	dupmbt_case("zero-len", 0, 0, empty, 0);
	dupmbt_case("one-byte", 1, 0, one, 1);
	dupmbt_case("hi-off", 4, 128, hi, sizeof(hi));
	dupmbt_case("mid-off", 3, 512, hi, sizeof(hi));
	dupmbt_case("end-off", 2, (std::ptrdiff_t)(MB_BUF_WORDS - 4), hi,
	    sizeof(hi));
	dupmbt_case("neg-off", 5, -3, nulheavy, sizeof(nulheavy));
	dupmbt_case("large-len", 1024, 0, hi, sizeof(hi));
	dupmbt_case("len-2048", 2048, 0, hi, sizeof(hi));
	dupmbt_case("neg-len", -1, 0, hi, sizeof(hi));
}

static void
test_dupmbt_sweep(void)
{
	unsigned char pat[256];

	for (long i = 0; i < SWEEP; i++) {
		for (std::size_t j = 0; j < sizeof(pat); j++)
			pat[j] = (unsigned char)(rnd() & 0xff);
		int len = (int)(std::int32_t)rnd();
		std::ptrdiff_t off =
		    (std::ptrdiff_t)(rnd_mod(MB_BUF_WORDS * 2)) - 64;
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		dupmbt_case(tag, len, off, pat, sizeof(pat));
	}
}

/* ------------------------------------------------------------------------ */
/* icmptypename                                                              */
/* ------------------------------------------------------------------------ */

static void
icmptypename_case(const char *tag, int family, int type)
{
	char *got = P::icmptypename(family, type);
	char *ref = ref_icmptypename(family, type);

	st_icmptypename.cases++;
	if (!streq_or_both_null(got, ref)) {
		stat_fail(&st_icmptypename, tag, "name");
		if (st_icmptypename.reported <= MAX_REPORT) {
			std::printf(
			    "      fam=%d type=%d port=%s ref=%s\n", family,
			    type, str_or_null(got), str_or_null(ref));
		}
	}
}

static void
test_icmptypename_edges(void)
{
	static const int families[] = { 0, 2, 28, -1, 255 };
	static const int types[] = { -1, 0, 1, 3, 4, 5, 8, 9, 10, 11, 12, 13,
	    14, 15, 16, 17, 18, 42, 127, 128, 255, 256, 300, -128 };

	for (std::size_t i = 0; i < sizeof(families) / sizeof(families[0]); i++) {
		for (std::size_t j = 0; j < sizeof(types) / sizeof(types[0]);
		     j++) {
			char tag[64];
			std::snprintf(tag, sizeof(tag), "f%zu t%zu", i, j);
			icmptypename_case(tag, families[i], types[j]);
		}
	}
}

static void
test_icmptypename_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		int family = (int)(std::int32_t)rnd();
		int type = (int)(rnd() & 0x1ff) - 64;
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		icmptypename_case(tag, family, type);
	}
}

/* ------------------------------------------------------------------------ */
/* getoptbyname                                                              */
/* ------------------------------------------------------------------------ */

static void
fill_name(char *buf, std::size_t cap, int pattern, std::size_t len)
{
	static const unsigned char alpha[] = {
	    0x00, 'n', 'o', 'p', 'r', 'r', 's', 'e', 'c', '-', 'c', 'l', 'a',
	    's', 's', 'z', 's', 'u', 0x80, 0xff
	};
	const std::size_t na = sizeof(alpha);

	if (cap == 0)
		return;
	if (len >= cap)
		len = cap - 1;
	for (std::size_t i = 0; i < len; i++) {
		switch (pattern) {
		case 0:
			buf[i] = (char)alpha[i % na];
			break;
		case 1:
			buf[i] = (char)(0x80 + (i & 0x7f));
			break;
		case 2:
			buf[i] = (char)((i & 1) ? 'A' : 'a');
			break;
		default:
			buf[i] = (char)(rnd() & 0xff);
			break;
		}
	}
	buf[len] = '\0';
}

static void
getoptbyname_case(const char *tag, const char *name, int pattern)
{
	char pa[128], pb[128];

	std::memset(pa, GUARD, sizeof(pa));
	std::memset(pb, GUARD, sizeof(pb));
	std::memcpy(pa + 16, name, std::strlen(name) + 1);
	std::memcpy(pb + 16, name, std::strlen(name) + 1);
	(void)pattern;

	char *pname = pa + 16;
	char *rname = pb + 16;

	P::u_32_t got = P::getoptbyname(pname);
	P::u_32_t ref = ref_getoptbyname(rname);

	st_getoptbyname.cases++;
	int bad = (got != ref);
	if (std::memcmp(pa, pb, sizeof(pa)) != 0)
		bad = 1;
	if (bad) {
		stat_fail(&st_getoptbyname, tag, bad ? "ret/buf" : "buf");
		if (st_getoptbyname.reported <= MAX_REPORT) {
			std::printf("      name=%s port=%u ref=%u\n", name,
			    (unsigned)got, (unsigned)ref);
		}
	}
}

static void
test_getoptbyname_edges(void)
{
	static const char *names[] = {
	    "", "nop", "NOP", "NoP", "rr", "RR", "zsu", "mtup", "mtur",
	    "encode", "ts", "tr", "sec", "sec-class", "SEC-CLASS", "lsrr",
	    "e-sec", "cipso", "satid", "ssrr", "addext", "visa", "imitd",
	    "eip", "finn", "dps", "sdb", "nsapa", "rtralrt", "ump", "ah",
	    "AH", "bogus", "n", "se", "secclass", "\xff", "\x80nop"
	};

	for (std::size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++)
		getoptbyname_case("edge", names[i], 0);
}

static void
test_getoptbyname_sweep(void)
{
	char name[96];

	for (long i = 0; i < SWEEP; i++) {
		int pat = (int)(rnd() % 4);
		std::size_t nlen = rnd_mod(sizeof(name) - 1);
		fill_name(name, sizeof(name), pat, nlen);
		if ((rnd() & 15) == 0) {
			static const char *known[] = { "nop", "rr", "sec-class",
			    "ah", "bogus" };
			std::snprintf(name, sizeof(name), "%s",
			    known[rnd_mod(sizeof(known) / sizeof(known[0]))]);
		}
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		getoptbyname_case(tag, name, pat);
	}
}

/* ------------------------------------------------------------------------ */
/* getoptbyvalue                                                             */
/* ------------------------------------------------------------------------ */

static void
getoptbyvalue_case(const char *tag, int optval)
{
	P::u_32_t got = P::getoptbyvalue(optval);
	P::u_32_t ref = ref_getoptbyvalue(optval);

	st_getoptbyvalue.cases++;
	if (got != ref) {
		stat_fail(&st_getoptbyvalue, tag, "ret");
		if (st_getoptbyvalue.reported <= MAX_REPORT) {
			std::printf("      val=%d port=%u ref=%u\n", optval,
			    (unsigned)got, (unsigned)ref);
		}
	}
}

static void
test_getoptbyvalue_edges(void)
{
	static const int vals[] = {
	    0, 1, 7, 10, 11, 12, 15, 68, 82, 130, 131, 133, 134, 136, 137,
	    142, 144, 145, 147, 148, 149, 151, 152, 205, 256, 307, -1, 999,
	    0x7fffffff, -0x7fffffff, 0x80, 0xff
	};

	for (std::size_t i = 0; i < sizeof(vals) / sizeof(vals[0]); i++) {
		char tag[32];
		std::snprintf(tag, sizeof(tag), "v%zu", i);
		getoptbyvalue_case(tag, vals[i]);
	}
}

static void
test_getoptbyvalue_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		int val = (int)(std::int32_t)rnd();
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		getoptbyvalue_case(tag, val);
	}
}

/* ------------------------------------------------------------------------ */

int
main(void)
{
	test_icmpcodes_edges();
	test_icmpcodes_sweep();
	test_dupmbt_edges();
	test_dupmbt_sweep();
	test_icmptypename_edges();
	test_icmptypename_sweep();
	test_getoptbyname_edges();
	test_getoptbyname_sweep();
	test_getoptbyvalue_edges();
	test_getoptbyvalue_sweep();

	std::printf("\n%-16s %8s %8s\n", "function", "cases", "fails");
	std::printf("%-16s %8ld %8ld\n", st_icmpcodes.name, st_icmpcodes.cases,
	    st_icmpcodes.fails);
	std::printf("%-16s %8ld %8ld\n", st_dupmbt.name, st_dupmbt.cases,
	    st_dupmbt.fails);
	std::printf("%-16s %8ld %8ld\n", st_icmptypename.name,
	    st_icmptypename.cases, st_icmptypename.fails);
	std::printf("%-16s %8ld %8ld\n", st_getoptbyname.name,
	    st_getoptbyname.cases, st_getoptbyname.fails);
	std::printf("%-16s %8ld %8ld\n", st_getoptbyvalue.name,
	    st_getoptbyvalue.cases, st_getoptbyvalue.fails);

	long total_fails = st_icmpcodes.fails + st_dupmbt.fails +
	    st_icmptypename.fails + st_getoptbyname.fails +
	    st_getoptbyvalue.fails;
	return total_fails == 0 ? 0 : 1;
}
