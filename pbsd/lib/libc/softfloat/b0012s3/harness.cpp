// PBSD batch b0012s3 -- differential test.
//
// Compares pbsd::lib_libc_softfloat::b0012s3::__negxf2() against the C oracle
// ref___negxf2() from oracle.c.
//
// negxf2.c is a single expression built out of two softfloat routines that
// belong to other batches (__mulxf3, __floatsixf).  This file provides the one
// and only definition of each; the port and the oracle both call into it, so
// every observable of the ported code is captured:
//
//   * the returned floatx80 (both fields, bit for bit),
//   * the exact sequence of calls made into __mulxf3/__floatsixf together
//     with every argument, so a mutated constant or a swapped operand shows
//     up even when the arithmetic would hide it,
//   * the softfloat exception flags raised,
//   * the guarded buffer the argument was read out of.

import pbsd.lib.libc.softfloat.b0012s3;

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <new>
#include <vector>

// The port's floatx80 has the same layout as the one oracle.c declares, and
// both __mulxf3/__floatsixf and ref___negxf2 have C language linkage, so one
// definition of each serves the port and the oracle alike.
using fx = pbsd::lib_libc_softfloat::b0012s3::floatx80;

extern "C" fx ref___negxf2(fx);

// ---------------------------------------------------------------------------
// Shared softfloat support: call recorder + reference floatx80 arithmetic.
// ---------------------------------------------------------------------------

namespace sf {

enum : int { FN_NONE = 0, FN_MULXF3 = 1, FN_FLOATSIXF = 2 };

enum : unsigned {
	flag_inexact   = 1u,
	flag_underflow = 2u,
	flag_overflow  = 4u,
	flag_invalid   = 16u
};

struct Call {
	int fn;
	std::int32_t iarg;
	std::uint16_t ah, bh;
	std::uint64_t al, bl;
};

constexpr int MAXCALL = 8;

Call log[MAXCALL];
int ncalls;
unsigned flags;

void
reset()
{
	ncalls = 0;
	flags = 0;
	std::memset(log, 0, sizeof log);
}

void
push(const Call &c)
{
	if (ncalls < MAXCALL)
		log[ncalls] = c;
	++ncalls;
}

inline int
clz64(std::uint64_t a)
{
	return a ? __builtin_clzll(a) : 64;
}

inline int
clz32(std::uint32_t a)
{
	return a ? __builtin_clz(a) : 32;
}

inline fx
pack(int zSign, std::int32_t zExp, std::uint64_t zSig)
{
	fx z;
	z.high = (std::uint16_t)(((unsigned)zSign << 15) +
	    ((unsigned)zExp & 0x7FFFu));
	z.low = zSig;
	return z;
}

inline std::uint64_t	extractFrac(fx a) { return a.low; }
inline std::int32_t	extractExp(fx a)  { return (std::int32_t)(a.high & 0x7FFF); }
inline int		extractSign(fx a) { return a.high >> 15; }

inline fx
defaultNaN()
{
	fx z;
	z.high = 0xFFFF;
	z.low = 0xC000000000000000ULL;
	return z;
}

inline void
normalizeSubnormal(std::uint64_t aSig, std::int32_t *zExpPtr,
    std::uint64_t *zSigPtr)
{
	int shiftCount = clz64(aSig);

	*zSigPtr = aSig << shiftCount;
	*zExpPtr = 1 - shiftCount;
}

inline void
shift64ExtraRightJamming(std::uint64_t a0, std::uint64_t a1, int count,
    std::uint64_t *z0Ptr, std::uint64_t *z1Ptr)
{
	std::uint64_t z0, z1;

	if (count == 0) {
		z1 = a1;
		z0 = a0;
	} else if (count < 64) {
		z1 = (a0 << (64 - count)) | (a1 != 0);
		z0 = a0 >> count;
	} else {
		if (count == 64)
			z1 = a0 | (a1 != 0);
		else
			z1 = ((a0 | a1) != 0);
		z0 = 0;
	}
	*z1Ptr = z1;
	*z0Ptr = z0;
}

inline fx
propagateNaN(fx a, fx b)
{
	bool aIsNaN = extractExp(a) == 0x7FFF && (std::uint64_t)(a.low << 1);
	bool bIsNaN = extractExp(b) == 0x7FFF && (std::uint64_t)(b.low << 1);
	bool aIsSignaling = aIsNaN && !(a.low & 0x4000000000000000ULL);
	bool bIsSignaling = bIsNaN && !(b.low & 0x4000000000000000ULL);

	a.low |= 0xC000000000000000ULL;
	b.low |= 0xC000000000000000ULL;
	if (aIsSignaling || bIsSignaling)
		flags |= flag_invalid;
	if (aIsNaN)
		return a;
	return b;
}

inline fx
roundAndPack(int zSign, std::int32_t zExp, std::uint64_t zSig0,
    std::uint64_t zSig1)
{
	if (0x7FFDu <= (std::uint32_t)(zExp - 1)) {
		if (0x7FFE < zExp) {
			flags |= flag_overflow | flag_inexact;
			return pack(zSign, 0x7FFF, 0x8000000000000000ULL);
		}
		if (zExp <= 0) {
			shift64ExtraRightJamming(zSig0, zSig1, 1 - zExp,
			    &zSig0, &zSig1);
			zExp = 0;
			if (zSig1 != 0)
				flags |= flag_underflow;
		}
	}
	if (zSig1 != 0)
		flags |= flag_inexact;
	if ((std::int64_t)zSig1 < 0) {
		++zSig0;
		if (zSig0 == 0) {
			zSig0 = 0x8000000000000000ULL;
			++zExp;
		} else if ((std::uint64_t)(zSig1 << 1) == 0) {
			zSig0 &= ~(std::uint64_t)1;
		}
	}
	if (zSig0 == 0)
		zExp = 0;
	if (zExp >= 0x7FFF) {
		flags |= flag_overflow | flag_inexact;
		return pack(zSign, 0x7FFF, 0x8000000000000000ULL);
	}
	return pack(zSign, zExp, zSig0);
}

inline fx
int32_to_floatx80(std::int32_t a)
{
	if (a == 0)
		return pack(0, 0, 0);

	int zSign = a < 0;
	std::uint32_t absA = zSign ? (std::uint32_t)(-(std::int64_t)a)
	    : (std::uint32_t)a;
	int shiftCount = clz32(absA) + 32;
	std::uint64_t zSig = absA;

	return pack(zSign, 0x403E - shiftCount, zSig << shiftCount);
}

inline fx
floatx80_mul(fx a, fx b)
{
	int aSign = extractSign(a);
	int bSign = extractSign(b);
	int zSign = aSign ^ bSign;
	std::int32_t aExp = extractExp(a);
	std::int32_t bExp = extractExp(b);
	std::uint64_t aSig = extractFrac(a);
	std::uint64_t bSig = extractFrac(b);

	if (aExp == 0x7FFF) {
		if ((std::uint64_t)(aSig << 1) ||
		    (bExp == 0x7FFF && (std::uint64_t)(bSig << 1)))
			return propagateNaN(a, b);
		if (((std::uint64_t)bExp | bSig) == 0) {
			flags |= flag_invalid;
			return defaultNaN();
		}
		return pack(zSign, 0x7FFF, 0x8000000000000000ULL);
	}
	if (bExp == 0x7FFF) {
		if ((std::uint64_t)(bSig << 1))
			return propagateNaN(a, b);
		if (((std::uint64_t)aExp | aSig) == 0) {
			flags |= flag_invalid;
			return defaultNaN();
		}
		return pack(zSign, 0x7FFF, 0x8000000000000000ULL);
	}
	if (aExp == 0) {
		if (aSig == 0)
			return pack(zSign, 0, 0);
		normalizeSubnormal(aSig, &aExp, &aSig);
	}
	if (bExp == 0) {
		if (bSig == 0)
			return pack(zSign, 0, 0);
		normalizeSubnormal(bSig, &bExp, &bSig);
	}

	std::int32_t zExp = aExp + bExp - 0x3FFE;
	unsigned __int128 prod = (unsigned __int128)aSig * bSig;
	std::uint64_t zSig0 = (std::uint64_t)(prod >> 64);
	std::uint64_t zSig1 = (std::uint64_t)prod;

	if ((std::int64_t)zSig0 > 0) {
		zSig0 = (zSig0 << 1) | (zSig1 >> 63);
		zSig1 <<= 1;
		--zExp;
	}
	return roundAndPack(zSign, zExp, zSig0, zSig1);
}

} // namespace sf

extern "C" fx
__floatsixf(std::int32_t a)
{
	sf::push(sf::Call{sf::FN_FLOATSIXF, a, 0, 0, 0, 0});
	return sf::int32_to_floatx80(a);
}

extern "C" fx
__mulxf3(fx a, fx b)
{
	sf::push(sf::Call{sf::FN_MULXF3, 0, a.high, b.high, a.low, b.low});
	return sf::floatx80_mul(a, b);
}

// ---------------------------------------------------------------------------
// Observation of one call.
// ---------------------------------------------------------------------------

namespace {

constexpr std::size_t BUFLEN = 80;
constexpr std::size_t VALOFF = 24;

struct Obs {
	std::uint16_t rhigh;
	std::uint64_t rlow;
	unsigned flags;
	int ncalls;
	sf::Call log[sf::MAXCALL];
	alignas(16) unsigned char buf[BUFLEN];
};

Obs
run(fx (*fn)(fx), fx v)
{
	Obs o;

	std::memset(&o, 0, sizeof o);
	std::memset(o.buf, 0x7f, sizeof o.buf);

	fx *slot = ::new (static_cast<void *>(o.buf + VALOFF)) fx;
	slot->high = v.high;
	slot->low = v.low;

	sf::reset();
	fx r = fn(*slot);

	o.rhigh = r.high;
	o.rlow = r.low;
	o.flags = sf::flags;
	o.ncalls = sf::ncalls;
	std::memcpy(o.log, sf::log, sizeof o.log);
	return o;
}

bool
same(const Obs &p, const Obs &q)
{
	if (p.rhigh != q.rhigh || p.rlow != q.rlow)
		return false;
	if (p.flags != q.flags || p.ncalls != q.ncalls)
		return false;
	if (std::memcmp(p.log, q.log, sizeof p.log) != 0)
		return false;
	if (std::memcmp(p.buf, q.buf, BUFLEN) != 0)
		return false;
	return true;
}

int failures;
long long cases;
int reported;

void
report(fx v, const Obs &p, const Obs &q)
{
	++failures;
	if (reported >= 10)
		return;
	++reported;
	std::printf("  FAIL __negxf2 in=%04x:%016llx\n", v.high,
	    (unsigned long long)v.low);
	std::printf("    port ret=%04x:%016llx flags=%u ncalls=%d\n",
	    p.rhigh, (unsigned long long)p.rlow, p.flags, p.ncalls);
	std::printf("    ref  ret=%04x:%016llx flags=%u ncalls=%d\n",
	    q.rhigh, (unsigned long long)q.rlow, q.flags, q.ncalls);
	for (int i = 0; i < sf::MAXCALL; i++) {
		if (std::memcmp(&p.log[i], &q.log[i], sizeof(sf::Call)) == 0)
			continue;
		std::printf("    call[%d] port fn=%d i=%d a=%04x:%016llx "
		    "b=%04x:%016llx\n", i, p.log[i].fn, p.log[i].iarg,
		    p.log[i].ah, (unsigned long long)p.log[i].al,
		    p.log[i].bh, (unsigned long long)p.log[i].bl);
		std::printf("    call[%d] ref  fn=%d i=%d a=%04x:%016llx "
		    "b=%04x:%016llx\n", i, q.log[i].fn, q.log[i].iarg,
		    q.log[i].ah, (unsigned long long)q.log[i].al,
		    q.log[i].bh, (unsigned long long)q.log[i].bl);
	}
	if (std::memcmp(p.buf, q.buf, BUFLEN) != 0)
		std::printf("    argument buffer diverged\n");
}

void
check(fx v)
{
	++cases;
	Obs p = run(pbsd::lib_libc_softfloat::b0012s3::__negxf2, v);
	Obs q = run(ref___negxf2, v);
	if (!same(p, q))
		report(v, p, q);
}

inline fx
mk(std::uint16_t high, std::uint64_t low)
{
	fx z;
	z.high = high;
	z.low = low;
	return z;
}

// ---------------------------------------------------------------------------
// Case generation.
// ---------------------------------------------------------------------------

const std::uint16_t kExps[] = {
	0x0000, 0x0001, 0x0002, 0x0003, 0x000F,
	0x3FFC, 0x3FFD, 0x3FFE, 0x3FFF, 0x4000, 0x4001,
	0x403D, 0x403E, 0x403F, 0x5555,
	0x7FFC, 0x7FFD, 0x7FFE, 0x7FFF
};

const std::uint64_t kSigs[] = {
	0x0000000000000000ULL, 0x0000000000000001ULL, 0x0000000000000002ULL,
	0x0000000000000003ULL, 0x000000000000FFFFULL, 0x00000000FFFFFFFFULL,
	0x0000000100000000ULL, 0x3FFFFFFFFFFFFFFFULL, 0x4000000000000000ULL,
	0x4000000000000001ULL, 0x7FFFFFFFFFFFFFFFULL, 0x8000000000000000ULL,
	0x8000000000000001ULL, 0xAAAAAAAAAAAAAAAAULL, 0xBFFFFFFFFFFFFFFFULL,
	0xC000000000000000ULL, 0xC000000000000001ULL, 0xFFFFFFFFFFFFFFFEULL,
	0xFFFFFFFFFFFFFFFFULL
};

void
edgeCases()
{
	for (int sign = 0; sign < 2; sign++) {
		for (std::uint16_t e : kExps) {
			std::uint16_t high =
			    (std::uint16_t)(e | (sign ? 0x8000u : 0u));
			for (std::uint64_t s : kSigs)
				check(mk(high, s));
		}
	}

	// Every byte value 0x00-0xFF smeared over all ten bytes of the value,
	// which covers the NUL-heavy and high-bit (0x80-0xFF) patterns.
	for (int b = 0; b < 256; b++) {
		std::uint64_t low = 0x0101010101010101ULL * (std::uint64_t)b;
		std::uint16_t high = (std::uint16_t)(b | (b << 8));
		check(mk(high, low));
		check(mk(high, 0));
		check(mk(0, low));
		check(mk((std::uint16_t)(0x7F00u | b), low));
		check(mk((std::uint16_t)(0xFF00u | b), low));
	}

	// Single set bit walked through the whole 80-bit value.
	for (int i = 0; i < 64; i++)
		check(mk(0x0000, 1ULL << i));
	for (int i = 0; i < 16; i++) {
		check(mk((std::uint16_t)(1u << i), 0));
		check(mk((std::uint16_t)(1u << i), 0x8000000000000000ULL));
		check(mk((std::uint16_t)~(1u << i), 0xFFFFFFFFFFFFFFFFULL));
	}
}

std::uint64_t rngState;

std::uint64_t
next64()
{
	std::uint64_t z = (rngState += 0x9E3779B97F4A7C15ULL);

	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

void
randomSweep(long long iters)
{
	rngState = 0xB0012533C0FFEEULL;

	for (long long i = 0; i < iters; i++) {
		std::uint64_t r = next64();
		std::uint64_t low;
		std::uint16_t exp;

		switch (r & 7) {
		case 0:
			exp = (std::uint16_t)(next64() & 0x7FFF);
			break;
		case 1:
			exp = 0x0000;
			break;
		case 2:
			exp = (std::uint16_t)(next64() & 0x3);
			break;
		case 3:
			exp = 0x7FFF;
			break;
		case 4:
			exp = (std::uint16_t)(0x7FFF - (next64() & 0x3));
			break;
		case 5:
			exp = (std::uint16_t)(0x3FFF +
			    (std::int64_t)(next64() & 0x7) - 4);
			break;
		case 6:
			exp = (std::uint16_t)(next64() & 0x00FF);
			break;
		default:
			exp = (std::uint16_t)(next64() & 0x7FFF);
			break;
		}

		switch ((r >> 3) & 7) {
		case 0:
			low = next64();
			break;
		case 1:
			low = 0;
			break;
		case 2:
			low = next64() | 0x8000000000000000ULL;
			break;
		case 3:
			low = next64() & 0x7FFFFFFFFFFFFFFFULL;
			break;
		case 4:
			low = 1ULL << (next64() & 63);
			break;
		case 5:
			low = ~(1ULL << (next64() & 63));
			break;
		case 6:
			low = next64() >> (next64() & 63);
			break;
		default:
			low = 0x8000000000000000ULL | (next64() & 0xFF);
			break;
		}

		std::uint16_t high =
		    (std::uint16_t)(exp | (((r >> 6) & 1) ? 0x8000u : 0u));
		check(mk(high, low));
	}
}

} // namespace

int
main()
{
	edgeCases();
	randomSweep(250000);

	std::printf("\n%-16s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-16s %12lld %12d\n", "__negxf2", cases, failures);
	std::printf("%-16s %12lld %12d\n", "TOTAL", cases, failures);

	if (failures != 0) {
		std::printf("\nRESULT: FAIL\n");
		return 1;
	}
	std::printf("\nRESULT: PASS\n");
	return 0;
}
