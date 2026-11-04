/*
 * Differential test for batch b0126 (abs, labs, strtonum).
 *
 * Every case is executed against BOTH the C++23 port and the ref_ oracle.
 * Return values, errno, errstr outputs, and the full guarded input regions are
 * compared bit for bit.
 */

#include <cerrno>
#include <climits>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.stdlib.b0126;

namespace P = pbsd::lib_libc_stdlib::b0126;

extern "C" {
int ref_abs(int j);
long ref_labs(long j);
long long ref_strtonum(const char *numstr, long long minval, long long maxval,
    const char **errstrp);
}

namespace {

enum {
	F_ABS,
	F_LABS,
	F_STRTONUM,
	F_COUNT
};

static const char *const fn_name[F_COUNT] = {
	"abs",
	"labs",
	"strtonum",
};

static unsigned long long n_cases[F_COUNT];
static unsigned long long n_fails[F_COUNT];
static unsigned long long reported[F_COUNT];

#define SWEEP_ITERS 200000u

/* ------------------------------------------------------------------ */
/* bookkeeping							      */
/* ------------------------------------------------------------------ */

static void
record_fail(int fn, const char *fmt, ...)
{
	n_fails[fn]++;
	if (reported[fn] < 10) {
		reported[fn]++;
		va_list ap;
		va_start(ap, fmt);
		std::fprintf(stderr, "FAIL %s: ", fn_name[fn]);
		std::vfprintf(stderr, fmt, ap);
		va_end(ap);
		std::fputc('\n', stderr);
	} else if (reported[fn] == 10) {
		reported[fn]++;
		std::fprintf(stderr, "FAIL %s: (further failures suppressed)\n",
		    fn_name[fn]);
	}
}

static void
record_case(int fn, bool ok, const char *fmt, ...)
{
	n_cases[fn]++;
	if (ok)
		return;
	n_fails[fn]++;
	if (reported[fn] < 10) {
		reported[fn]++;
		va_list ap;
		va_start(ap, fmt);
		std::fprintf(stderr, "FAIL %s: ", fn_name[fn]);
		std::vfprintf(stderr, fmt, ap);
		va_end(ap);
		std::fputc('\n', stderr);
	} else if (reported[fn] == 10) {
		reported[fn]++;
		std::fprintf(stderr, "FAIL %s: (further failures suppressed)\n",
		    fn_name[fn]);
	}
}

/* Fixed-seed xorshift64; identical stream on every run and every host. */
uint64_t rng_state = 0xc0ffeebaddecaf01ULL;

uint64_t
nextr(void)
{
	rng_state ^= rng_state << 13;
	rng_state ^= rng_state >> 7;
	rng_state ^= rng_state << 17;
	return rng_state;
}

static unsigned
rnd_below(unsigned n)
{
	return (unsigned)(nextr() % (uint64_t)n);
}

static int
rnd_sign(void)
{
	return (nextr() & 1u) ? 1 : -1;
}

static bool
errstr_equal(const char *a, const char *b)
{
	if (a == b)
		return true;
	if (a == nullptr || b == nullptr)
		return false;
	return std::strcmp(a, b) == 0;
}

/* ------------------------------------------------------------------ */
/* abs / labs							      */
/* ------------------------------------------------------------------ */

static void
case_abs(int j)
{
	int rp = P::abs(j);
	int rr = ref_abs(j);
	record_case(F_ABS, rp == rr, "j=%d port=%d ref=%d", j, rp, rr);
}

static void
case_labs(long j)
{
	long rp = P::labs(j);
	long rr = ref_labs(j);
	record_case(F_LABS, rp == rr, "j=%ld port=%ld ref=%ld", j, rp, rr);
}

static void
test_abs_edges(void)
{
	static const int vals[] = {
		0, 1, -1, 2, -2, 3, -3,
		INT_MAX, INT_MIN, INT_MAX - 1, INT_MIN + 1,
		INT_MAX - 2, INT_MIN + 2,
		0x7f, -0x7f, 0x80, -0x80,
		0x7fff, -0x7fff, 0x8000, -0x8000,
		0x7fffffff, (int)0x80000000,
	};
	unsigned i;

	for (i = 0; i < sizeof(vals) / sizeof(vals[0]); i++)
		case_abs(vals[i]);
}

static void
test_labs_edges(void)
{
	static const long vals[] = {
		0L, 1L, -1L, 2L, -2L,
		LONG_MAX, LONG_MIN, LONG_MAX - 1L, LONG_MIN + 1L,
		LONG_MAX - 2L, LONG_MIN + 2L,
		0x7fL, -0x7fL, 0x80L, -0x80L,
		0x7fffL, -0x7fffL, 0x8000L, -0x8000L,
		0x7fffffffL, -0x7fffffffL - 1L,
	};
	unsigned i;

	for (i = 0; i < sizeof(vals) / sizeof(vals[0]); i++)
		case_labs(vals[i]);
}

static void
test_abs_random(unsigned iters)
{
	unsigned i;

	for (i = 0; i < iters; i++) {
		int j;
		switch (i % 8u) {
		case 0:
			j = 0;
			break;
		case 1:
			j = (int)(nextr() & 0x7fffffffu);
			break;
		case 2:
			j = -(int)(nextr() & 0x7fffffffu);
			break;
		case 3:
			j = INT_MAX;
			break;
		case 4:
			j = INT_MIN;
			break;
		case 5:
			j = (int)(int32_t)nextr();
			break;
		case 6:
			j = rnd_sign() * (int)(1 + rnd_below(1000u));
			break;
		default:
			j = (int)(int16_t)nextr();
			break;
		}
		case_abs(j);
	}
}

static void
test_labs_random(unsigned iters)
{
	unsigned i;

	for (i = 0; i < iters; i++) {
		long j;
		switch (i % 8u) {
		case 0:
			j = 0L;
			break;
		case 1:
			j = (long)(nextr() & 0x7fffffffffffffffULL);
			break;
		case 2:
			j = -(long)(nextr() & 0x7fffffffffffffffULL);
			break;
		case 3:
			j = LONG_MAX;
			break;
		case 4:
			j = LONG_MIN;
			break;
		case 5:
			j = (long)(int64_t)nextr();
			break;
		case 6:
			j = (long)rnd_sign() * (long)(1 + rnd_below(100000u));
			break;
		default:
			j = (long)(int32_t)nextr();
			break;
		}
		case_labs(j);
	}
}

/* ------------------------------------------------------------------ */
/* strtonum							      */
/* ------------------------------------------------------------------ */

enum {
	B0126_MAX_INPUT = 96u,
	LEAD_GUARD = 16u,
	TRAIL_GUARD = 16u,
	BUF_SIZE = LEAD_GUARD + B0126_MAX_INPUT + 1u + TRAIL_GUARD,
	GUARD_BYTE = 0x7f,
	ERRSLOT_LEAD = 8u,
	ERRSLOT_SIZE = ERRSLOT_LEAD + sizeof(const char *) + 8u,
};

struct StrtonumObs {
	long long ret;
	int errno_val;
	const char *errstr;
	unsigned char errslot[ERRSLOT_SIZE];
	unsigned char buf[BUF_SIZE];
};

static void
prep_strtonum_buf(unsigned char *buf, const char *src, size_t len)
{
	std::memset(buf, GUARD_BYTE, BUF_SIZE);
	if (len > B0126_MAX_INPUT)
		len = B0126_MAX_INPUT;
	std::memcpy(buf + LEAD_GUARD, src, len);
	buf[LEAD_GUARD + len] = '\0';
}

static void
prep_errslot(unsigned char *slot)
{
	std::memset(slot, GUARD_BYTE, ERRSLOT_SIZE);
}

static const char **
errslot_ptr(unsigned char *slot)
{
	return reinterpret_cast<const char **>(slot + ERRSLOT_LEAD);
}

static void
run_strtonum_port(StrtonumObs *o, const char *numstr, long long minval,
    long long maxval, bool use_errstr)
{
	const char **epp = use_errstr ? errslot_ptr(o->errslot) : nullptr;

	prep_errslot(o->errslot);
	errno = 0;
	o->ret = P::strtonum(numstr, minval, maxval, epp);
	o->errno_val = errno;
	o->errstr = use_errstr ? *epp : nullptr;
}

static void
run_strtonum_ref(StrtonumObs *o, const char *numstr, long long minval,
    long long maxval, bool use_errstr)
{
	const char **epp = use_errstr ? errslot_ptr(o->errslot) : nullptr;

	prep_errslot(o->errslot);
	errno = 0;
	o->ret = ref_strtonum(numstr, minval, maxval, epp);
	o->errno_val = errno;
	o->errstr = use_errstr ? *epp : nullptr;
}

static bool
strtonum_obs_equal(const StrtonumObs *p, const StrtonumObs *r, bool use_errstr)
{
	if (p->ret != r->ret)
		return false;
	if (p->errno_val != r->errno_val)
		return false;
	if (!errstr_equal(p->errstr, r->errstr))
		return false;
	if (use_errstr &&
	    std::memcmp(p->errslot, r->errslot, ERRSLOT_SIZE) != 0)
		return false;
	if (std::memcmp(p->buf, r->buf, BUF_SIZE) != 0)
		return false;
	return true;
}

static void
case_strtonum(const char *src, size_t len, long long minval, long long maxval,
    int pre_errno, bool use_errstr)
{
	StrtonumObs po{}, ro{};
	const char *ps, *rs;

	prep_strtonum_buf(po.buf, src, len);
	prep_strtonum_buf(ro.buf, src, len);
	ps = reinterpret_cast<const char *>(po.buf + LEAD_GUARD);
	rs = reinterpret_cast<const char *>(ro.buf + LEAD_GUARD);

	errno = pre_errno;
	run_strtonum_port(&po, ps, minval, maxval, use_errstr);

	errno = pre_errno;
	run_strtonum_ref(&ro, rs, minval, maxval, use_errstr);

	record_case(F_STRTONUM,
	    strtonum_obs_equal(&po, &ro, use_errstr),
	    "numstr=\"%.*s\" min=%lld max=%lld pre_errno=%d errp=%d "
	    "ret port=%lld ref=%lld errno port=%d ref=%d err port=%s ref=%s",
	    (int)len, src, (long long)minval, (long long)maxval, pre_errno,
	    (int)use_errstr, (long long)po.ret, (long long)ro.ret,
	    po.errno_val, ro.errno_val,
	    po.errstr ? po.errstr : "(null)",
	    ro.errstr ? ro.errstr : "(null)");
}

static void
case_strtonum_cstr(const char *s, long long minval, long long maxval,
    int pre_errno, bool use_errstr)
{
	case_strtonum(s, std::strlen(s), minval, maxval, pre_errno, use_errstr);
}

static void
test_strtonum_edges(void)
{
	static const struct {
		const char *s;
		long long minv;
		long long maxv;
		int pre_errno;
		bool use_errstr;
	} fixed[] = {
		/* success paths and errno restoration */
		{ "", 0, 100, 0, true },
		{ "0", 0, 100, 0, true },
		{ "0", 0, 100, EIO, true },
		{ "42", 0, 100, EIO, true },
		{ "-5", -10, 10, 0, true },
		{ "100", 0, 100, 0, true },
		{ "-100", -100, 100, 0, true },
		{ "9223372036854775807", LLONG_MIN, LLONG_MAX, 0, true },
		{ "-9223372036854775808", LLONG_MIN, LLONG_MAX, 0, true },

		/* invalid: empty / no conversion / trailing junk */
		{ "", 0, 100, 0, true },
		{ "x", 0, 100, 0, true },
		{ "12x", 0, 100, 0, true },
		{ "12.5", 0, 100, 0, true },
		{ "+", 0, 100, 0, true },
		{ "-", 0, 100, 0, true },
		{ " 42", 0, 100, 0, true },
		{ "\t9", 0, 100, 0, true },
		{ "0x10", 0, 100, 0, true },

		/* minval > maxval */
		{ "5", 10, 5, 0, true },
		{ "5", 1, 0, 0, true },
		{ "", LLONG_MAX, LLONG_MIN, 0, true },

		/* too small */
		{ "5", 10, 100, 0, true },
		{ "-1", 0, 100, 0, true },
		{ "0", 1, 100, 0, true },
		{ "-9223372036854775809", LLONG_MIN, LLONG_MAX, 0, true },

		/* too large */
		{ "200", 0, 100, 0, true },
		{ "101", 0, 100, 0, true },
		{ "9223372036854775808", LLONG_MIN, LLONG_MAX, 0, true },

		/* exact boundaries for comparisons */
		{ "10", 10, 100, 0, true },
		{ "9", 10, 100, 0, true },
		{ "11", 10, 100, 0, true },
		{ "100", 0, 100, 0, true },
		{ "99", 0, 100, 0, true },
		{ "101", 0, 100, 0, true },

		/* errstrp == NULL */
		{ "42", 0, 100, 0, false },
		{ "x", 0, 100, 0, false },
		{ "5", 10, 5, 0, false },
		{ "200", 0, 100, 0, false },
	};
	unsigned i;

	for (i = 0; i < sizeof(fixed) / sizeof(fixed[0]); i++)
		case_strtonum_cstr(fixed[i].s, fixed[i].minv, fixed[i].maxv,
		    fixed[i].pre_errno, fixed[i].use_errstr);

	/* NUL-heavy and high-bit byte inputs */
	{
		static const char nul_heavy[] =
		    { '1', '\0', '2', '\0', '3', '\0', '\0', '4', '\0' };
		case_strtonum(nul_heavy, sizeof(nul_heavy), 0, 100, 0, true);

		unsigned char hi[8];
		hi[0] = (char)0x80;
		hi[1] = '1';
		hi[2] = (char)0xff;
		hi[3] = '2';
		hi[4] = '\0';
		case_strtonum(reinterpret_cast<const char *>(hi), 4, 0, 100,
		    0, true);
	}

	/* single-char and empty with varied ranges */
	case_strtonum_cstr("", LLONG_MIN, LLONG_MAX, EEXIST, true);
	case_strtonum_cstr("0", LLONG_MIN, LLONG_MAX, EEXIST, true);
	case_strtonum_cstr("1", LLONG_MIN, LLONG_MAX, EEXIST, true);
	case_strtonum_cstr("-", LLONG_MIN, LLONG_MAX, EEXIST, true);
	case_strtonum_cstr("+", LLONG_MIN, LLONG_MAX, EEXIST, true);
}

static size_t
gen_digits(char *buf, unsigned cap)
{
	unsigned n = 0;
	unsigned len = 1u + rnd_below(24u);
	unsigned k;
	bool neg;

	if (cap == 0)
		return 0;
	if (rnd_below(3u) == 0u)
		buf[n++] = (rnd_below(2u) != 0u) ? '-' : '+';
	neg = (n > 0 && buf[0] == '-');
	for (k = 0; k < len && n + 1u < cap; k++) {
		if (k == 0)
			buf[n++] = (char)('0' + (int)rnd_below(neg ? 10u : 9u));
		else
			buf[n++] = (char)('0' + (int)rnd_below(10u));
	}
	return n;
}

static size_t
gen_boundary_num(char *buf, unsigned cap)
{
	static const char *const patterns[] = {
		"0",
		"1",
		"-1",
		"9223372036854775807",
		"-9223372036854775808",
		"9223372036854775808",
		"-9223372036854775809",
		"099",
		"00042",
		"+42",
		"-0042",
		"42 ",
		" 42",
		"42\n",
		"42\0tail",
	};
	const char *pat = patterns[rnd_below(
	    (unsigned)(sizeof(patterns) / sizeof(patterns[0])))];
	size_t len = std::strlen(pat);

	if (len >= cap)
		len = cap - 1u;
	std::memcpy(buf, pat, len);
	return len;
}

static size_t
gen_soup(char *buf, unsigned cap)
{
	unsigned n = 0;
	unsigned len = rnd_below(cap > 0 ? cap : 1u);

	while (n < len && n + 1u < cap) {
		unsigned r = rnd_below(16u);
		if (r < 10u)
			buf[n++] = (char)('0' + (int)r);
		else if (r == 10u)
			buf[n++] = '-';
		else if (r == 11u)
			buf[n++] = '+';
		else if (r == 12u)
			buf[n++] = '\0';
		else
			buf[n++] = (char)(0x80u + rnd_below(0x80u));
	}
	return n;
}

static long long
gen_bound_ll(void)
{
	static const long long vals[] = {
		LLONG_MIN, LLONG_MIN + 1, LLONG_MIN + 2,
		-1, 0, 1,
		LLONG_MAX - 2, LLONG_MAX - 1, LLONG_MAX,
		-100, 100, -1000, 1000,
	};
	long long v = vals[rnd_below(
	    (unsigned)(sizeof(vals) / sizeof(vals[0])))];
	if (rnd_below(4u) == 0u)
		v += (long long)(int64_t)(nextr() & 0xffu) * rnd_sign();
	return v;
}

static void
test_strtonum_random(unsigned iters)
{
	char tmp[B0126_MAX_INPUT + 1];
	unsigned i;

	for (i = 0; i < iters; i++) {
		size_t n;
		long long minv, maxv;
		bool use_errstr;
		int pre_errno;

		switch (i % 5u) {
		case 0:
			n = gen_digits(tmp, B0126_MAX_INPUT);
			break;
		case 1:
			n = gen_boundary_num(tmp, B0126_MAX_INPUT);
			break;
		case 2:
			n = gen_soup(tmp, B0126_MAX_INPUT);
			break;
		case 3:
			n = 0;
			break;
		default:
			n = 1;
			tmp[0] = (char)(0x80u + rnd_below(0x80u));
			break;
		}

		switch (i % 7u) {
		case 0:
			minv = 0;
			maxv = 100;
			break;
		case 1:
			minv = LLONG_MIN;
			maxv = LLONG_MAX;
			break;
		case 2:
			minv = gen_bound_ll();
			maxv = gen_bound_ll();
			break;
		case 3:
			minv = 10;
			maxv = 5;
			break;
		case 4:
			minv = -1000;
			maxv = 1000;
			break;
		case 5:
			minv = LLONG_MAX - 10;
			maxv = LLONG_MAX;
			break;
		default:
			minv = LLONG_MIN;
			maxv = LLONG_MIN + 10;
			break;
		}

		use_errstr = (i % 3u) != 0u;
		pre_errno = (int)(nextr() & 0xffu);

		case_strtonum(tmp, n, minv, maxv, pre_errno, use_errstr);
	}
}

} /* namespace */

int
main(void)
{
	unsigned fn;
	unsigned long long total_cases = 0, total_fails = 0;

	test_abs_edges();
	test_labs_edges();
	test_strtonum_edges();

	test_abs_random(SWEEP_ITERS);
	test_labs_random(SWEEP_ITERS);
	test_strtonum_random(SWEEP_ITERS);

	std::printf("\nbatch b0126 differential results\n");
	std::printf("%-10s %12s %10s %s\n", "function", "cases", "failures",
	    "status");
	for (fn = 0; fn < F_COUNT; fn++) {
		std::printf("%-10s %12llu %10llu %s\n", fn_name[fn],
		    n_cases[fn], n_fails[fn],
		    n_fails[fn] == 0u ? "ok" : "FAIL");
		total_cases += n_cases[fn];
		total_fails += n_fails[fn];
	}
	std::printf("%-10s %12llu %10llu %s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0u ? "ok" : "FAIL");

	return total_fails == 0u ? 0 : 1;
}
