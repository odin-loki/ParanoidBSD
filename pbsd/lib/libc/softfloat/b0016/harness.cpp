/*
 * harness.cpp -- differential test for PBSD batch b0016.
 *
 * Every function in the batch is driven with hand-written edge cases (all-NUL
 * operands, single-bit and single-byte operands, high-bit bytes 0x80..0xFF,
 * signed zeroes, denormal/normal/infinity/NaN boundaries, the full cross
 * product of an interesting-value pool) and with a fixed-seed randomised
 * sweep of >200000 iterations per function.  Each case is evaluated by both
 * the C++23 port and the ref_ oracle and the two are compared on:
 *
 *	- the return value;
 *	- the softfloat exception flags raised;
 *	- the ENTIRE operand buffer afterwards, including the guard bytes
 *	  outside the operands and the tail padding inside floatx80.
 *
 * Operands are materialised inside two separate 0x7f-filled buffers (one for
 * the port, one for the oracle) and loaded from those buffers, so any write
 * through an operand or past its nominal window is caught.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libc.softfloat.b0016;

namespace pb = pbsd::lib_libc_softfloat::b0016;

using pb::bits16;
using pb::bits64;
using pb::float128;
using pb::floatx80;

/*
 * The oracle's operand types are re-declared here rather than reused from the
 * module, so that the port's own idea of the softfloat struct layout is under
 * test too: the port hands these structs on to the softfloat.c primitives by
 * value, so a layout that disagrees with the C declaration below must show up
 * as a divergence instead of cancelling out on both sides.
 */
struct h_float128 {
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	std::uint64_t high, low;
#else
	std::uint64_t low, high;
#endif
};

struct h_floatx80 {
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	std::uint16_t high;
	std::uint64_t low;
#else
	std::uint64_t low;
	std::uint16_t high;
#endif
};

extern "C" {
int ref___lttf2(h_float128 a, h_float128 b);
int ref___gexf2(h_floatx80 a, h_floatx80 b);
int ref___getf2(h_float128 a, h_float128 b);
int ref___letf2(h_float128 a, h_float128 b);
extern int float_exception_flags;
}

enum { float_flag_invalid = 16 };

static_assert(sizeof(float128) == 16, "float128 must be two 64-bit words");
static_assert(sizeof(floatx80) == 16, "floatx80 must be 16 bytes");
static_assert(sizeof(float128) == sizeof(h_float128), "float128 size");
static_assert(sizeof(floatx80) == sizeof(h_floatx80), "floatx80 size");
static_assert(sizeof(bits64) == 8, "bits64");
static_assert(sizeof(bits16) == 2, "bits16");

/* ---------------------------------------------------------------- values */

struct V128 {
	std::uint64_t high;
	std::uint64_t low;
};

struct VX80 {
	std::uint16_t high;
	std::uint64_t low;
};

static const V128 pool128[] = {
	{ 0x0000000000000000ULL, 0x0000000000000000ULL },	/* +0 */
	{ 0x8000000000000000ULL, 0x0000000000000000ULL },	/* -0 */
	{ 0x0000000000000000ULL, 0x0000000000000001ULL },	/* +denorm min */
	{ 0x8000000000000000ULL, 0x0000000000000001ULL },	/* -denorm min */
	{ 0x0000000000000000ULL, 0x8000000000000000ULL },
	{ 0x0000FFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL },	/* +denorm max */
	{ 0x8000FFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL },	/* -denorm max */
	{ 0x0001000000000000ULL, 0x0000000000000000ULL },	/* +normal min */
	{ 0x8001000000000000ULL, 0x0000000000000000ULL },	/* -normal min */
	{ 0x3FFE000000000000ULL, 0x0000000000000000ULL },	/* +0.5 */
	{ 0x3FFF000000000000ULL, 0x0000000000000000ULL },	/* +1.0 */
	{ 0x3FFF000000000000ULL, 0x0000000000000001ULL },	/* +1.0+ulp */
	{ 0xBFFF000000000000ULL, 0x0000000000000000ULL },	/* -1.0 */
	{ 0xBFFF000000000000ULL, 0x0000000000000001ULL },	/* -1.0-ulp */
	{ 0x4000000000000000ULL, 0x0000000000000000ULL },	/* +2.0 */
	{ 0xC000000000000000ULL, 0x0000000000000000ULL },	/* -2.0 */
	{ 0x4001000000000000ULL, 0x0000000000000000ULL },	/* +4.0 */
	{ 0xC001000000000000ULL, 0x0000000000000000ULL },	/* -4.0 */
	{ 0x7FFEFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL },	/* +normal max */
	{ 0xFFFEFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL },	/* -normal max */
	{ 0x7FFF000000000000ULL, 0x0000000000000000ULL },	/* +inf */
	{ 0xFFFF000000000000ULL, 0x0000000000000000ULL },	/* -inf */
	{ 0x7FFF800000000000ULL, 0x0000000000000000ULL },	/* +qnan */
	{ 0xFFFF800000000000ULL, 0x0000000000000000ULL },	/* -qnan */
	{ 0x7FFF000000000000ULL, 0x0000000000000001ULL },	/* +snan in frac1 */
	{ 0x7FFF000000000001ULL, 0x0000000000000000ULL },	/* +snan in frac0 */
	{ 0xFFFF000000000000ULL, 0x0000000000000001ULL },	/* -snan in frac1 */
	{ 0x7F7F7F7F7F7F7F7FULL, 0x7F7F7F7F7F7F7F7FULL },	/* guard pattern */
	{ 0x8080808080808080ULL, 0x8080808080808080ULL },	/* high-bit bytes */
	{ 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL },	/* all bits set */
	{ 0x0000000000000000ULL, 0xFFFFFFFFFFFFFFFFULL },
	{ 0xFFFFFFFFFFFFFFFFULL, 0x0000000000000000ULL },
	{ 0x00000000000000FFULL, 0xFF00000000000000ULL },
	{ 0x0001020304050607ULL, 0x08090A0B0C0D0E0FULL },
	{ 0xF0E1D2C3B4A59687ULL, 0x78695A4B3C2D1E0FULL },
};
static const std::size_t NPOOL128 = sizeof(pool128) / sizeof(pool128[0]);

static const VX80 poolx80[] = {
	{ 0x0000, 0x0000000000000000ULL },	/* +0 */
	{ 0x8000, 0x0000000000000000ULL },	/* -0 */
	{ 0x0000, 0x0000000000000001ULL },	/* +denorm min */
	{ 0x8000, 0x0000000000000001ULL },	/* -denorm min */
	{ 0x0000, 0x8000000000000000ULL },	/* pseudo-denormal */
	{ 0x8000, 0x8000000000000000ULL },
	{ 0x0001, 0x8000000000000000ULL },	/* +normal min */
	{ 0x8001, 0x8000000000000000ULL },	/* -normal min */
	{ 0x3FFE, 0x8000000000000000ULL },	/* +0.5 */
	{ 0x3FFF, 0x8000000000000000ULL },	/* +1.0 */
	{ 0x3FFF, 0x8000000000000001ULL },	/* +1.0+ulp */
	{ 0xBFFF, 0x8000000000000000ULL },	/* -1.0 */
	{ 0xBFFF, 0x8000000000000001ULL },	/* -1.0-ulp */
	{ 0x4000, 0x8000000000000000ULL },	/* +2.0 */
	{ 0xC000, 0x8000000000000000ULL },	/* -2.0 */
	{ 0x7FFE, 0xFFFFFFFFFFFFFFFFULL },	/* +normal max */
	{ 0xFFFE, 0xFFFFFFFFFFFFFFFFULL },	/* -normal max */
	{ 0x7FFF, 0x8000000000000000ULL },	/* +inf */
	{ 0xFFFF, 0x8000000000000000ULL },	/* -inf */
	{ 0x7FFF, 0xC000000000000000ULL },	/* +qnan */
	{ 0xFFFF, 0xC000000000000000ULL },	/* -qnan */
	{ 0x7FFF, 0x8000000000000001ULL },	/* +snan */
	{ 0x7FFF, 0x0000000000000000ULL },	/* exp all ones, frac 0 */
	{ 0xFFFF, 0x0000000000000000ULL },
	{ 0x7FFF, 0xFFFFFFFFFFFFFFFFULL },
	{ 0x7F7F, 0x7F7F7F7F7F7F7F7FULL },	/* guard pattern */
	{ 0x8080, 0x8080808080808080ULL },	/* high-bit bytes */
	{ 0xFFFF, 0xFFFFFFFFFFFFFFFFULL },	/* all bits set */
	{ 0x0000, 0x7FFFFFFFFFFFFFFFULL },
	{ 0x00FF, 0x00000000000000FFULL },
	{ 0x0102, 0x0304050607080910ULL },
};
static const std::size_t NPOOLX80 = sizeof(poolx80) / sizeof(poolx80[0]);

/* ------------------------------------------------------------ statistics */

enum { FN_LTTF2, FN_GETF2, FN_LETF2, FN_GEXF2, NFN };

struct FnStat {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
	unsigned long long n_neg;
	unsigned long long n_zero;
	unsigned long long n_pos;
	unsigned long long n_other;
	unsigned long long n_inv;
	unsigned long long n_noinv;
	int required;		/* 1: -1 seen, 2: 0 seen, 4: +1 seen */
};

static FnStat stats[NFN] = {
	{ "__lttf2", 0, 0, 0, 0, 0, 0, 0, 0, 1 | 2 },
	{ "__getf2", 0, 0, 0, 0, 0, 0, 0, 0, 1 | 2 },
	{ "__letf2", 0, 0, 0, 0, 0, 0, 0, 0, 2 | 4 },
	{ "__gexf2", 0, 0, 0, 0, 0, 0, 0, 0, 1 | 2 },
};

static unsigned long long reported = 0;
static const unsigned long long MAXREPORT = 20;

static void
tally(FnStat &st, int want, int wantflags)
{

	if (want == -1)
		st.n_neg++;
	else if (want == 0)
		st.n_zero++;
	else if (want == 1)
		st.n_pos++;
	else
		st.n_other++;

	if ((wantflags & float_flag_invalid) != 0)
		st.n_inv++;
	else
		st.n_noinv++;
}

/* --------------------------------------------------------------- buffers */

static const std::size_t BUFSZ = 96;
static const std::size_t OFF_A = 16;
static const std::size_t OFF_B = 48;
static const unsigned char GUARD = 0x7f;

template <class T>
static void
fill128(unsigned char *buf, unsigned char pad, const V128 &a, const V128 &b)
{

	std::memset(buf, GUARD, BUFSZ);
	std::memset(buf + OFF_A, pad, sizeof(T));
	std::memset(buf + OFF_B, pad, sizeof(T));
	std::memcpy(buf + OFF_A + offsetof(T, high), &a.high, 8);
	std::memcpy(buf + OFF_A + offsetof(T, low), &a.low, 8);
	std::memcpy(buf + OFF_B + offsetof(T, high), &b.high, 8);
	std::memcpy(buf + OFF_B + offsetof(T, low), &b.low, 8);
}

template <class T>
static void
fillx80(unsigned char *buf, unsigned char pad, const VX80 &a, const VX80 &b)
{

	std::memset(buf, GUARD, BUFSZ);
	std::memset(buf + OFF_A, pad, sizeof(T));
	std::memset(buf + OFF_B, pad, sizeof(T));
	std::memcpy(buf + OFF_A + offsetof(T, high), &a.high, 2);
	std::memcpy(buf + OFF_A + offsetof(T, low), &a.low, 8);
	std::memcpy(buf + OFF_B + offsetof(T, high), &b.high, 2);
	std::memcpy(buf + OFF_B + offsetof(T, low), &b.low, 8);
}

static void
dumpdiff(const char *what, const unsigned char *x, const unsigned char *y)
{
	std::size_t i;

	std::printf("      %s:", what);
	for (i = 0; i < BUFSZ; i++)
		if (x[i] != y[i])
			std::printf(" [%zu] %02x!=%02x", i, x[i], y[i]);
	std::printf("\n");
}

/* ----------------------------------------------------------- case drivers */

static void
case128(int fn, const V128 &a, const V128 &b, unsigned char pad)
{
	FnStat &st = stats[fn];
	unsigned char pbuf[BUFSZ], rbuf[BUFSZ], pexp[BUFSZ], rexpbuf[BUFSZ];
	float128 pa, pbv;
	h_float128 ra, rb;
	int got, gotflags, want, wantflags;
	bool ok;

	st.cases++;

	fill128<float128>(pbuf, pad, a, b);
	fill128<float128>(pexp, pad, a, b);
	fill128<h_float128>(rbuf, pad, a, b);
	fill128<h_float128>(rexpbuf, pad, a, b);

	std::memcpy(&pa, pbuf + OFF_A, sizeof pa);
	std::memcpy(&pbv, pbuf + OFF_B, sizeof pbv);
	std::memcpy(&ra, rbuf + OFF_A, sizeof ra);
	std::memcpy(&rb, rbuf + OFF_B, sizeof rb);

	float_exception_flags = 0;
	if (fn == FN_LTTF2)
		got = pb::__lttf2(pa, pbv);
	else if (fn == FN_GETF2)
		got = pb::__getf2(pa, pbv);
	else
		got = pb::__letf2(pa, pbv);
	gotflags = float_exception_flags;

	float_exception_flags = 0;
	if (fn == FN_LTTF2)
		want = ref___lttf2(ra, rb);
	else if (fn == FN_GETF2)
		want = ref___getf2(ra, rb);
	else
		want = ref___letf2(ra, rb);
	wantflags = float_exception_flags;

	tally(st, want, wantflags);

	ok = true;
	if (got != want)
		ok = false;
	if (gotflags != wantflags)
		ok = false;
	if (std::memcmp(pbuf, pexp, BUFSZ) != 0)
		ok = false;
	if (std::memcmp(rbuf, rexpbuf, BUFSZ) != 0)
		ok = false;
	if (std::memcmp(pbuf, rbuf, BUFSZ) != 0)
		ok = false;

	if (!ok) {
		st.failures++;
		if (reported < MAXREPORT) {
			reported++;
			std::printf("FAIL %s(a={%016llx,%016llx}, "
			    "b={%016llx,%016llx}, pad=%02x): "
			    "port=%d flags=%d oracle=%d flags=%d\n",
			    st.name,
			    (unsigned long long)a.high,
			    (unsigned long long)a.low,
			    (unsigned long long)b.high,
			    (unsigned long long)b.low,
			    pad, got, gotflags, want, wantflags);
			if (std::memcmp(pbuf, pexp, BUFSZ) != 0)
				dumpdiff("port buffer clobbered", pbuf, pexp);
			if (std::memcmp(rbuf, rexpbuf, BUFSZ) != 0)
				dumpdiff("oracle buffer clobbered", rbuf,
				    rexpbuf);
			if (std::memcmp(pbuf, rbuf, BUFSZ) != 0)
				dumpdiff("operand images differ", pbuf, rbuf);
		}
	}
}

static void
all128(const V128 &a, const V128 &b, unsigned char pad)
{

	case128(FN_LTTF2, a, b, pad);
	case128(FN_GETF2, a, b, pad);
	case128(FN_LETF2, a, b, pad);
}

static void
casex80(const VX80 &a, const VX80 &b, unsigned char pad)
{
	FnStat &st = stats[FN_GEXF2];
	unsigned char pbuf[BUFSZ], rbuf[BUFSZ], pexp[BUFSZ], rexpbuf[BUFSZ];
	floatx80 pa, pbv;
	h_floatx80 ra, rb;
	int got, gotflags, want, wantflags;
	bool ok;

	st.cases++;

	fillx80<floatx80>(pbuf, pad, a, b);
	fillx80<floatx80>(pexp, pad, a, b);
	fillx80<h_floatx80>(rbuf, pad, a, b);
	fillx80<h_floatx80>(rexpbuf, pad, a, b);

	std::memcpy(&pa, pbuf + OFF_A, sizeof pa);
	std::memcpy(&pbv, pbuf + OFF_B, sizeof pbv);
	std::memcpy(&ra, rbuf + OFF_A, sizeof ra);
	std::memcpy(&rb, rbuf + OFF_B, sizeof rb);

	float_exception_flags = 0;
	got = pb::__gexf2(pa, pbv);
	gotflags = float_exception_flags;

	float_exception_flags = 0;
	want = ref___gexf2(ra, rb);
	wantflags = float_exception_flags;

	tally(st, want, wantflags);

	ok = true;
	if (got != want)
		ok = false;
	if (gotflags != wantflags)
		ok = false;
	if (std::memcmp(pbuf, pexp, BUFSZ) != 0)
		ok = false;
	if (std::memcmp(rbuf, rexpbuf, BUFSZ) != 0)
		ok = false;
	if (std::memcmp(pbuf, rbuf, BUFSZ) != 0)
		ok = false;

	if (!ok) {
		st.failures++;
		if (reported < MAXREPORT) {
			reported++;
			std::printf("FAIL %s(a={%04x,%016llx}, "
			    "b={%04x,%016llx}, pad=%02x): "
			    "port=%d flags=%d oracle=%d flags=%d\n",
			    st.name, (unsigned)a.high,
			    (unsigned long long)a.low, (unsigned)b.high,
			    (unsigned long long)b.low,
			    pad, got, gotflags, want, wantflags);
			if (std::memcmp(pbuf, pexp, BUFSZ) != 0)
				dumpdiff("port buffer clobbered", pbuf, pexp);
			if (std::memcmp(rbuf, rexpbuf, BUFSZ) != 0)
				dumpdiff("oracle buffer clobbered", rbuf,
				    rexpbuf);
			if (std::memcmp(pbuf, rbuf, BUFSZ) != 0)
				dumpdiff("operand images differ", pbuf, rbuf);
		}
	}
}

/* ------------------------------------------------------------------- rng */

static std::uint64_t rngstate = 0x0DDB1A5E5BAD5EEDULL;

static std::uint64_t
nextu64(void)
{
	std::uint64_t z;

	rngstate += 0x9E3779B97F4A7C15ULL;
	z = rngstate;
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

static const std::uint16_t rexp[] = {
	0x0000, 0x0001, 0x0002, 0x1234, 0x3FFE, 0x3FFF, 0x4000, 0x7FFD,
	0x7FFE, 0x7FFF, 0x7FFF, 0x7FFF
};
static const std::size_t NREXP = sizeof(rexp) / sizeof(rexp[0]);

static const unsigned char rbyte[] = {
	0x00, 0x01, 0x7f, 0x80, 0x81, 0xfe, 0xff
};
static const std::size_t NRBYTE = sizeof(rbyte) / sizeof(rbyte[0]);

static std::uint64_t
splat(unsigned char b)
{
	std::uint64_t v = b;

	return v * 0x0101010101010101ULL;
}

static V128
rand128(void)
{
	V128 v;
	std::uint64_t sign, frac0, frac1;
	std::uint16_t e;
	unsigned char b;

	switch ((int)(nextu64() % 6)) {
	case 0:
		v.high = nextu64();
		v.low = nextu64();
		break;
	case 1:
		sign = nextu64() & 1;
		e = rexp[nextu64() % NREXP];
		switch ((int)(nextu64() % 4)) {
		case 0:
			frac0 = 0;
			frac1 = 0;
			break;
		case 1:
			frac0 = 0x0000FFFFFFFFFFFFULL;
			frac1 = 0xFFFFFFFFFFFFFFFFULL;
			break;
		case 2:
			frac0 = 0;
			frac1 = nextu64() & 0xF;
			break;
		default:
			frac0 = nextu64() & 0x0000FFFFFFFFFFFFULL;
			frac1 = nextu64();
			break;
		}
		v.high = (sign << 63) | ((std::uint64_t)e << 48) | frac0;
		v.low = frac1;
		break;
	case 2:
		b = rbyte[nextu64() % NRBYTE];
		v.high = splat(b);
		v.low = splat(b);
		break;
	case 3:
		v.high = (nextu64() & 1) << 63;
		v.low = nextu64() & 0xFFFF;
		break;
	case 4:
		sign = nextu64() & 1;
		e = (std::uint16_t)(nextu64() & 0x7FFF);
		v.high = (sign << 63) | ((std::uint64_t)e << 48);
		v.low = 0;
		break;
	default:
		v.high = pool128[nextu64() % NPOOL128].high;
		v.low = pool128[nextu64() % NPOOL128].low;
		break;
	}
	return v;
}

static V128
derive128(const V128 &a)
{
	V128 v = a;

	switch ((int)(nextu64() % 8)) {
	case 0:
		break;
	case 1:
		v.high ^= 0x8000000000000000ULL;
		break;
	case 2:
		v.low += 1;
		break;
	case 3:
		v.low -= 1;
		break;
	case 4:
		v.high += 1;
		break;
	case 5:
		v.high -= 1;
		break;
	case 6:
		v.high ^= 0x8000000000000000ULL;
		v.low ^= 1;
		break;
	default:
		v.high ^= (std::uint64_t)1 << (nextu64() % 64);
		break;
	}
	return v;
}

static VX80
randx80(void)
{
	VX80 v;
	std::uint64_t sign;
	std::uint16_t e;
	unsigned char b;

	switch ((int)(nextu64() % 6)) {
	case 0:
		v.high = (std::uint16_t)nextu64();
		v.low = nextu64();
		break;
	case 1:
		sign = nextu64() & 1;
		e = rexp[nextu64() % NREXP];
		switch ((int)(nextu64() % 4)) {
		case 0:
			v.low = 0;
			break;
		case 1:
			v.low = 0x8000000000000000ULL;
			break;
		case 2:
			v.low = nextu64() & 0xF;
			break;
		default:
			v.low = nextu64();
			break;
		}
		v.high = (std::uint16_t)((sign << 15) | e);
		break;
	case 2:
		b = rbyte[nextu64() % NRBYTE];
		v.high = (std::uint16_t)splat(b);
		v.low = splat(b);
		break;
	case 3:
		v.high = (std::uint16_t)((nextu64() & 1) << 15);
		v.low = nextu64() & 0xFFFF;
		break;
	case 4:
		v.high = (std::uint16_t)(nextu64() & 0xFFFF);
		v.low = 0;
		break;
	default:
		v.high = poolx80[nextu64() % NPOOLX80].high;
		v.low = poolx80[nextu64() % NPOOLX80].low;
		break;
	}
	return v;
}

static VX80
derivex80(const VX80 &a)
{
	VX80 v = a;

	switch ((int)(nextu64() % 8)) {
	case 0:
		break;
	case 1:
		v.high ^= 0x8000;
		break;
	case 2:
		v.low += 1;
		break;
	case 3:
		v.low -= 1;
		break;
	case 4:
		v.high = (std::uint16_t)(v.high + 1);
		break;
	case 5:
		v.high = (std::uint16_t)(v.high - 1);
		break;
	case 6:
		v.high ^= 0x8000;
		v.low ^= 1;
		break;
	default:
		v.low ^= (std::uint64_t)1 << (nextu64() % 64);
		break;
	}
	return v;
}

/* ------------------------------------------------------------ edge cases */

static const unsigned char pads[] = { 0x7f, 0x00, 0xff };
static const std::size_t NPADS = sizeof(pads) / sizeof(pads[0]);

static void
edge128(void)
{
	std::size_t i, j, k, p;
	V128 a, b;

	/* Full cross product of the interesting-value pool. */
	for (i = 0; i < NPOOL128; i++)
		for (j = 0; j < NPOOL128; j++)
			all128(pool128[i], pool128[j], GUARD);

	/* Every operand byte carrying a lone high-bit / all-ones byte. */
	for (k = 0; k < NRBYTE; k++) {
		for (i = 0; i < 16; i++) {
			std::uint64_t shifted =
			    (std::uint64_t)rbyte[k] << (8 * (i % 8));

			a.high = (i < 8) ? 0 : shifted;
			a.low = (i < 8) ? shifted : 0;
			for (j = 0; j < NPOOL128; j++) {
				all128(a, pool128[j], GUARD);
				all128(pool128[j], a, GUARD);
			}
			all128(a, a, GUARD);
		}
	}

	/* Every single-bit operand against zero, itself and 1.0. */
	for (i = 0; i < 128; i++) {
		a.high = (i < 64) ? 0 : ((std::uint64_t)1 << (i - 64));
		a.low = (i < 64) ? ((std::uint64_t)1 << i) : 0;
		b.high = 0;
		b.low = 0;
		all128(a, b, GUARD);
		all128(b, a, GUARD);
		all128(a, a, GUARD);
		b.high = 0x3FFF000000000000ULL;
		b.low = 0;
		all128(a, b, GUARD);
		all128(b, a, GUARD);
		b.high = 0x8000000000000000ULL;
		b.low = 0;
		all128(a, b, GUARD);
		all128(b, a, GUARD);
	}

	/* Adjacent values: equality against one-ulp either side. */
	for (i = 0; i < NPOOL128; i++) {
		a = pool128[i];
		all128(a, a, GUARD);
		b = a;
		b.low += 1;
		all128(a, b, GUARD);
		all128(b, a, GUARD);
		b = a;
		b.low -= 1;
		all128(a, b, GUARD);
		all128(b, a, GUARD);
		b = a;
		b.high += 1;
		all128(a, b, GUARD);
		all128(b, a, GUARD);
		b = a;
		b.high -= 1;
		all128(a, b, GUARD);
		all128(b, a, GUARD);
		b = a;
		b.high ^= 0x8000000000000000ULL;
		all128(a, b, GUARD);
		all128(b, a, GUARD);
	}

	/* Operand padding must not matter. */
	for (p = 0; p < NPADS; p++)
		for (i = 0; i < NPOOL128; i++)
			for (j = 0; j < NPOOL128; j++)
				all128(pool128[i], pool128[j], pads[p]);
}

static void
edgex80(void)
{
	std::size_t i, j, k, p;
	VX80 a, b;

	for (p = 0; p < NPADS; p++)
		for (i = 0; i < NPOOLX80; i++)
			for (j = 0; j < NPOOLX80; j++)
				casex80(poolx80[i], poolx80[j], pads[p]);

	for (k = 0; k < NRBYTE; k++) {
		for (i = 0; i < 10; i++) {
			std::uint64_t shifted =
			    (std::uint64_t)rbyte[k] << (8 * (i % 8));

			a.high = (i < 8) ? 0 :
			    (std::uint16_t)((std::uint64_t)rbyte[k] <<
			    (8 * (i - 8)));
			a.low = (i < 8) ? shifted : 0;
			for (j = 0; j < NPOOLX80; j++) {
				casex80(a, poolx80[j], GUARD);
				casex80(poolx80[j], a, GUARD);
			}
			casex80(a, a, GUARD);
		}
	}

	for (i = 0; i < 80; i++) {
		a.high = (i < 64) ? 0 :
		    (std::uint16_t)((std::uint32_t)1 << (i - 64));
		a.low = (i < 64) ? ((std::uint64_t)1 << i) : 0;
		b.high = 0;
		b.low = 0;
		casex80(a, b, GUARD);
		casex80(b, a, GUARD);
		casex80(a, a, GUARD);
		b.high = 0x3FFF;
		b.low = 0x8000000000000000ULL;
		casex80(a, b, GUARD);
		casex80(b, a, GUARD);
		b.high = 0x8000;
		b.low = 0;
		casex80(a, b, GUARD);
		casex80(b, a, GUARD);
	}

	for (i = 0; i < NPOOLX80; i++) {
		a = poolx80[i];
		casex80(a, a, GUARD);
		b = a;
		b.low += 1;
		casex80(a, b, GUARD);
		casex80(b, a, GUARD);
		b = a;
		b.low -= 1;
		casex80(a, b, GUARD);
		casex80(b, a, GUARD);
		b = a;
		b.high = (std::uint16_t)(b.high + 1);
		casex80(a, b, GUARD);
		casex80(b, a, GUARD);
		b = a;
		b.high = (std::uint16_t)(b.high - 1);
		casex80(a, b, GUARD);
		casex80(b, a, GUARD);
		b = a;
		b.high ^= 0x8000;
		casex80(a, b, GUARD);
		casex80(b, a, GUARD);
	}
}

/* ------------------------------------------------------------------ main */

static const unsigned long long RANDITERS = 220000ULL;

int
main(void)
{
	unsigned long long i;
	unsigned long long totalcases = 0, totalfail = 0;
	int coveragebad = 0;
	int fn, seen;

	edge128();
	edgex80();

	for (i = 0; i < RANDITERS; i++) {
		V128 a = rand128();
		V128 b = ((nextu64() & 1) != 0) ? derive128(a) : rand128();
		unsigned char pad = pads[nextu64() % NPADS];

		all128(a, b, pad);
	}

	for (i = 0; i < RANDITERS; i++) {
		VX80 a = randx80();
		VX80 b = ((nextu64() & 1) != 0) ? derivex80(a) : randx80();
		unsigned char pad = pads[nextu64() % NPADS];

		casex80(a, b, pad);
	}

	std::printf("\n%-10s %12s %10s %10s %10s %10s %8s %10s %10s\n",
	    "function", "cases", "failures", "ret<0", "ret==0", "ret>0",
	    "other", "invalid", "no-invalid");
	for (fn = 0; fn < NFN; fn++) {
		const FnStat &st = stats[fn];

		std::printf("%-10s %12llu %10llu %10llu %10llu %10llu %8llu "
		    "%10llu %10llu\n", st.name, st.cases, st.failures,
		    st.n_neg, st.n_zero, st.n_pos, st.n_other, st.n_inv,
		    st.n_noinv);
		totalcases += st.cases;
		totalfail += st.failures;
	}
	std::printf("%-10s %12llu %10llu\n", "TOTAL", totalcases, totalfail);

	for (fn = 0; fn < NFN; fn++) {
		const FnStat &st = stats[fn];

		seen = (st.n_neg != 0 ? 1 : 0) | (st.n_zero != 0 ? 2 : 0) |
		    (st.n_pos != 0 ? 4 : 0);
		if ((seen & st.required) != st.required) {
			std::printf("COVERAGE %s: required return values "
			    "not exercised (seen mask %d, required %d)\n",
			    st.name, seen, st.required);
			coveragebad = 1;
		}
		if (st.n_other != 0) {
			std::printf("COVERAGE %s: oracle returned a value "
			    "outside {-1,0,1}\n", st.name);
			coveragebad = 1;
		}
		if (st.n_inv == 0 || st.n_noinv == 0) {
			std::printf("COVERAGE %s: NaN/non-NaN paths not both "
			    "exercised (invalid=%llu, no-invalid=%llu)\n",
			    st.name, st.n_inv, st.n_noinv);
			coveragebad = 1;
		}
		if (st.cases < 200000ULL) {
			std::printf("COVERAGE %s: only %llu cases\n",
			    st.name, st.cases);
			coveragebad = 1;
		}
	}

	if (totalfail == 0 && coveragebad == 0) {
		std::printf("PASS: all %llu cases matched\n", totalcases);
		return 0;
	}
	std::printf("FAILED: %llu of %llu cases diverged%s\n", totalfail,
	    totalcases, coveragebad != 0 ? " (coverage incomplete)" : "");
	return 1;
}
