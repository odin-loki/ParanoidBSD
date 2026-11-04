/*
 * harness.cpp -- differential test for batch b0057.
 *
 * Every function in the batch is called with identical bits through the port
 * and through the ref_ oracle, and the results are compared BIT FOR BIT (not
 * with ==).  Bit comparison is mandatory here: conj() only differs from a
 * broken conj() by the sign of the imaginary part, and for a zero imaginary
 * part -0.0 == +0.0 is true, so a value comparison would happily accept a
 * port that dropped the negation.
 *
 * Arguments and results both travel through 64-byte buffers pre-filled with
 * the guard byte 0x7f.  The whole buffer is compared afterwards, including
 * the bytes outside the nominal value, so a write past the value is caught.
 * The only bytes excluded from the comparison are the 6 padding bytes of the
 * 16-byte x86 long double, whose content is genuinely indeterminate.
 */

#include <cfloat>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.src.b0057;

namespace port = pbsd::lib_msun_src::b0057;

extern "C" {
float ref_crealf(float _Complex);
double ref_creal(double _Complex);
long double ref_creall(long double _Complex);
double _Complex ref_conj(double _Complex);
}

namespace {

/* ------------------------------------------------------------------ */
/* guarded buffers						      */
/* ------------------------------------------------------------------ */

constexpr std::size_t BUFSZ = 64;
constexpr std::size_t OFF = 16;
constexpr unsigned char GUARD = 0x7f;

struct Layout {
	std::size_t size; /* bytes of the whole object		*/
	std::size_t elem; /* bytes of one scalar component	*/
	std::size_t sig;  /* leading significant bytes per elem	*/
};

#if LDBL_MANT_DIG == 64
constexpr std::size_t LD_SIG = 10; /* x87 80-bit value in 16 bytes	*/
#else
constexpr std::size_t LD_SIG = sizeof(long double);
#endif

constexpr Layout L_float = { sizeof(float), sizeof(float), sizeof(float) };
constexpr Layout L_double = { sizeof(double), sizeof(double), sizeof(double) };
constexpr Layout L_ldouble = { sizeof(long double), sizeof(long double), LD_SIG };
constexpr Layout L_fcomplex = { sizeof(float _Complex), sizeof(float), sizeof(float) };
constexpr Layout L_dcomplex = { sizeof(double _Complex), sizeof(double), sizeof(double) };
constexpr Layout L_ldcomplex = { sizeof(long double _Complex), sizeof(long double), LD_SIG };

struct Buf {
	alignas(16) unsigned char b[BUFSZ];

	void fill() { std::memset(b, GUARD, BUFSZ); }
	unsigned char *slot() { return b + OFF; }
	const unsigned char *slot() const { return b + OFF; }
};

bool
is_padding(std::size_t i, Layout L)
{
	if (i < OFF || i >= OFF + L.size)
		return false;
	return ((i - OFF) % L.elem) >= L.sig;
}

/* Whole-buffer comparison, skipping only indeterminate padding. */
bool
buf_equal(const Buf &x, const Buf &y, Layout L)
{
	for (std::size_t i = 0; i < BUFSZ; i++) {
		if (is_padding(i, L))
			continue;
		if (x.b[i] != y.b[i])
			return false;
	}
	return true;
}

/* Every byte outside the value window must still be the guard byte. */
bool
guards_intact(const Buf &x, Layout L)
{
	for (std::size_t i = 0; i < BUFSZ; i++) {
		if (i >= OFF && i < OFF + L.size)
			continue;
		if (x.b[i] != GUARD)
			return false;
	}
	return true;
}

void
dump(const char *tag, const Buf &x, Layout L)
{
	std::printf("      %-6s", tag);
	for (std::size_t i = OFF; i < OFF + L.size; i++)
		std::printf(" %02x", x.b[i]);
	std::printf("   guards:");
	for (std::size_t i = 0; i < BUFSZ; i++) {
		if (i >= OFF && i < OFF + L.size)
			continue;
		if (x.b[i] != GUARD) {
			std::printf(" [%zu]=%02x", i, x.b[i]);
		}
	}
	std::printf("\n");
}

/* ------------------------------------------------------------------ */
/* bookkeeping							      */
/* ------------------------------------------------------------------ */

struct Stats {
	const char *name;
	unsigned long cases;
	unsigned long failures;
	unsigned long reported;
};

Stats st_crealf = { "crealf", 0, 0, 0 };
Stats st_creal = { "creal", 0, 0, 0 };
Stats st_creall = { "creall", 0, 0, 0 };
Stats st_conj = { "conj", 0, 0, 0 };

/* ------------------------------------------------------------------ */
/* deterministic PRNG (splitmix64, fixed seed)			      */
/* ------------------------------------------------------------------ */

std::uint64_t rng_state = 0x0057b0057b0057ULL;

std::uint64_t
next64()
{
	std::uint64_t z = (rng_state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

/* ------------------------------------------------------------------ */
/* edge-case bit patterns					      */
/* ------------------------------------------------------------------ */

const std::uint32_t f_edge[] = {
	0x00000000u, /* +0			*/
	0x80000000u, /* -0			*/
	0x00000001u, /* smallest subnormal	*/
	0x80000001u,
	0x007fffffu, /* largest subnormal	*/
	0x807fffffu,
	0x00800000u, /* smallest normal	*/
	0x80800000u,
	0x3f800000u, /* 1			*/
	0xbf800000u,
	0x7f7fffffu, /* FLT_MAX		*/
	0xff7fffffu,
	0x7f800000u, /* +inf			*/
	0xff800000u, /* -inf			*/
	0x7fc00000u, /* quiet NaN		*/
	0xffc00000u,
	0x7fc00001u, /* quiet NaN, payload	*/
	0x7f800001u, /* signalling NaN	*/
	0xff800001u,
	0x40490fdbu, /* pi			*/
	0xc0490fdbu,
	0x7f7f7f7fu, /* guard-byte fill	*/
	0x80808080u, /* high bit in every byte */
	0xffffffffu,
	0xfefefefeu,
	0x00808080u,
};
constexpr std::size_t F_EDGE_N = sizeof(f_edge) / sizeof(f_edge[0]);

const std::uint64_t d_edge[] = {
	0x0000000000000000ULL, /* +0			*/
	0x8000000000000000ULL, /* -0			*/
	0x0000000000000001ULL, /* smallest subnormal	*/
	0x8000000000000001ULL,
	0x000fffffffffffffULL, /* largest subnormal	*/
	0x800fffffffffffffULL,
	0x0010000000000000ULL, /* smallest normal	*/
	0x8010000000000000ULL,
	0x3ff0000000000000ULL, /* 1			*/
	0xbff0000000000000ULL,
	0x7fefffffffffffffULL, /* DBL_MAX		*/
	0xffefffffffffffffULL,
	0x7ff0000000000000ULL, /* +inf			*/
	0xfff0000000000000ULL, /* -inf			*/
	0x7ff8000000000000ULL, /* quiet NaN		*/
	0xfff8000000000000ULL,
	0x7ff8000000000001ULL, /* quiet NaN, payload	*/
	0x7ff0000000000001ULL, /* signalling NaN	*/
	0xfff0000000000001ULL,
	0x400921fb54442d18ULL, /* pi			*/
	0xc00921fb54442d18ULL,
	0x7f7f7f7f7f7f7f7fULL, /* guard-byte fill	*/
	0x8080808080808080ULL, /* high bit in every byte */
	0xffffffffffffffffULL,
	0xfefefefefefefefeULL,
	0x0080808080808080ULL,
};
constexpr std::size_t D_EDGE_N = sizeof(d_edge) / sizeof(d_edge[0]);

struct LdBits {
	std::uint64_t frac; /* low 64 bits (significand)	*/
	std::uint16_t se;   /* sign bit + 15-bit exponent	*/
};

const LdBits ld_edge[] = {
	{ 0x0000000000000000ULL, 0x0000 }, /* +0			  */
	{ 0x0000000000000000ULL, 0x8000 }, /* -0			  */
	{ 0x0000000000000001ULL, 0x0000 }, /* smallest subnormal  */
	{ 0x0000000000000001ULL, 0x8000 },
	{ 0x7fffffffffffffffULL, 0x0000 }, /* largest subnormal	  */
	{ 0x8000000000000000ULL, 0x0001 }, /* smallest normal	  */
	{ 0x8000000000000000ULL, 0x8001 },
	{ 0x8000000000000000ULL, 0x3fff }, /* 1			  */
	{ 0x8000000000000000ULL, 0xbfff },
	{ 0xffffffffffffffffULL, 0x7ffe }, /* LDBL_MAX		  */
	{ 0xffffffffffffffffULL, 0xfffe },
	{ 0x8000000000000000ULL, 0x7fff }, /* +inf		  */
	{ 0x8000000000000000ULL, 0xffff }, /* -inf		  */
	{ 0xc000000000000000ULL, 0x7fff }, /* quiet NaN		  */
	{ 0xc000000000000000ULL, 0xffff },
	{ 0xc000000000000001ULL, 0x7fff }, /* quiet NaN, payload	  */
	{ 0xa000000000000000ULL, 0x7fff }, /* signalling NaN	  */
	{ 0xa000000000000000ULL, 0xffff },
	{ 0xc90fdaa22168c235ULL, 0x4000 }, /* pi			  */
	{ 0xc90fdaa22168c235ULL, 0xc000 },
	{ 0x7f7f7f7f7f7f7f7fULL, 0x7f7f }, /* guard-byte fill	  */
	{ 0x8080808080808080ULL, 0x8080 }, /* high bit everywhere  */
	{ 0xffffffffffffffffULL, 0xffff },
	{ 0xfefefefefefefefeULL, 0xfefe },
};
constexpr std::size_t LD_EDGE_N = sizeof(ld_edge) / sizeof(ld_edge[0]);

unsigned char
rand_byte_fill()
{
	static const unsigned char bs[] = { 0x00, 0x01, 0x7e, 0x7f, 0x80,
		0x81, 0xfe, 0xff, 0x88, 0x55 };
	return bs[next64() % (sizeof(bs) / sizeof(bs[0]))];
}

std::uint32_t
rand_fbits()
{
	switch (next64() & 3u) {
	case 0:
		return (std::uint32_t)next64();
	case 1:
		return f_edge[next64() % F_EDGE_N];
	case 2: {
		static const std::uint32_t ex[] = { 0, 1, 2, 0x7e, 0x7f, 0x80,
			0xfd, 0xfe, 0xff };
		std::uint32_t s = (std::uint32_t)(next64() & 1u);
		std::uint32_t e = ex[next64() % (sizeof(ex) / sizeof(ex[0]))];
		std::uint32_t m = (std::uint32_t)next64() & 0x7fffffu;
		return (s << 31) | (e << 23) | m;
	}
	default: {
		unsigned char b = rand_byte_fill();
		std::uint32_t v = 0;
		for (int i = 0; i < 4; i++)
			v = (v << 8) | b;
		return v;
	}
	}
}

std::uint64_t
rand_dbits()
{
	switch (next64() & 3u) {
	case 0:
		return next64();
	case 1:
		return d_edge[next64() % D_EDGE_N];
	case 2: {
		static const std::uint64_t ex[] = { 0, 1, 2, 0x3fe, 0x3ff,
			0x400, 0x7fd, 0x7fe, 0x7ff };
		std::uint64_t s = next64() & 1u;
		std::uint64_t e = ex[next64() % (sizeof(ex) / sizeof(ex[0]))];
		std::uint64_t m = next64() & 0x000fffffffffffffULL;
		return (s << 63) | (e << 52) | m;
	}
	default: {
		unsigned char b = rand_byte_fill();
		std::uint64_t v = 0;
		for (int i = 0; i < 8; i++)
			v = (v << 8) | b;
		return v;
	}
	}
}

LdBits
rand_ldbits()
{
	LdBits r;

	switch (next64() & 3u) {
	case 0:
		r.frac = next64();
		r.se = (std::uint16_t)next64();
		return r;
	case 1:
		return ld_edge[next64() % LD_EDGE_N];
	case 2: {
		static const std::uint16_t ex[] = { 0, 1, 2, 0x3ffe, 0x3fff,
			0x4000, 0x7ffd, 0x7ffe, 0x7fff };
		std::uint16_t s = (std::uint16_t)(next64() & 1u);
		std::uint16_t e = ex[next64() % (sizeof(ex) / sizeof(ex[0]))];
		r.frac = next64();
		if (next64() & 1u)
			r.frac |= 0x8000000000000000ULL; /* normalised	*/
		else
			r.frac &= 0x7fffffffffffffffULL; /* unnormal	*/
		r.se = (std::uint16_t)((s << 15) | e);
		return r;
	}
	default: {
		unsigned char b = rand_byte_fill();
		r.frac = 0;
		for (int i = 0; i < 8; i++)
			r.frac = (r.frac << 8) | b;
		r.se = (std::uint16_t)((b << 8) | b);
		return r;
	}
	}
}

/* ------------------------------------------------------------------ */
/* per-function differential checks				      */
/* ------------------------------------------------------------------ */

/*
 * Each check builds the argument as an explicit byte image (padding zeroed so
 * that the argument buffers are byte-for-byte comparable), copies it into two
 * guarded buffers, loads one argument out of each buffer, calls port and
 * oracle, and compares result buffers and argument buffers in full.
 */

void
fail_report(Stats &s, const Buf &pa, const Buf &pb, Layout rl, const char *what)
{
	s.failures++;
	if (s.reported < 10) {
		s.reported++;
		std::printf("  FAIL %s case %lu (%s)\n", s.name, s.cases, what);
		dump("port", pa, rl);
		dump("ref", pb, rl);
	}
}

void
check_crealf(std::uint32_t rb, std::uint32_t ib)
{
	Stats &s = st_crealf;
	unsigned char img[sizeof(float _Complex)];
	Buf inA, inB, inRef, outA, outB;

	s.cases++;

	std::memset(img, 0, sizeof(img));
	std::memcpy(img + 0 * sizeof(float), &rb, sizeof(rb));
	std::memcpy(img + 1 * sizeof(float), &ib, sizeof(ib));

	inA.fill();
	inB.fill();
	inRef.fill();
	std::memcpy(inA.slot(), img, sizeof(img));
	std::memcpy(inB.slot(), img, sizeof(img));
	std::memcpy(inRef.slot(), img, sizeof(img));

	float _Complex za, zb;
	std::memcpy(&za, inA.slot(), sizeof(za));
	std::memcpy(&zb, inB.slot(), sizeof(zb));

	outA.fill();
	outB.fill();

	float ra = port::crealf(za);
	float rr = ref_crealf(zb);

	std::memcpy(outA.slot(), &ra, sizeof(ra));
	std::memcpy(outB.slot(), &rr, sizeof(rr));

	if (!buf_equal(outA, outB, L_float) || !guards_intact(outA, L_float) ||
	    !guards_intact(outB, L_float)) {
		fail_report(s, outA, outB, L_float, "result");
		return;
	}
	if (std::memcmp(inA.b, inRef.b, BUFSZ) != 0 ||
	    std::memcmp(inB.b, inRef.b, BUFSZ) != 0) {
		fail_report(s, inA, inB, L_fcomplex, "argument buffer");
		return;
	}
}

void
check_creal(std::uint64_t rb, std::uint64_t ib)
{
	Stats &s = st_creal;
	unsigned char img[sizeof(double _Complex)];
	Buf inA, inB, inRef, outA, outB;

	s.cases++;

	std::memset(img, 0, sizeof(img));
	std::memcpy(img + 0 * sizeof(double), &rb, sizeof(rb));
	std::memcpy(img + 1 * sizeof(double), &ib, sizeof(ib));

	inA.fill();
	inB.fill();
	inRef.fill();
	std::memcpy(inA.slot(), img, sizeof(img));
	std::memcpy(inB.slot(), img, sizeof(img));
	std::memcpy(inRef.slot(), img, sizeof(img));

	double _Complex za, zb;
	std::memcpy(&za, inA.slot(), sizeof(za));
	std::memcpy(&zb, inB.slot(), sizeof(zb));

	outA.fill();
	outB.fill();

	double ra = port::creal(za);
	double rr = ref_creal(zb);

	std::memcpy(outA.slot(), &ra, sizeof(ra));
	std::memcpy(outB.slot(), &rr, sizeof(rr));

	if (!buf_equal(outA, outB, L_double) || !guards_intact(outA, L_double) ||
	    !guards_intact(outB, L_double)) {
		fail_report(s, outA, outB, L_double, "result");
		return;
	}
	if (std::memcmp(inA.b, inRef.b, BUFSZ) != 0 ||
	    std::memcmp(inB.b, inRef.b, BUFSZ) != 0) {
		fail_report(s, inA, inB, L_dcomplex, "argument buffer");
		return;
	}
}

void
check_creall(LdBits rb, LdBits ib)
{
	Stats &s = st_creall;
	unsigned char img[sizeof(long double _Complex)];
	Buf inA, inB, inRef, outA, outB;

	s.cases++;

	std::memset(img, 0, sizeof(img));
	std::memcpy(img + 0 * sizeof(long double) + 0, &rb.frac, 8);
	std::memcpy(img + 0 * sizeof(long double) + 8, &rb.se, 2);
	std::memcpy(img + 1 * sizeof(long double) + 0, &ib.frac, 8);
	std::memcpy(img + 1 * sizeof(long double) + 8, &ib.se, 2);

	inA.fill();
	inB.fill();
	inRef.fill();
	std::memcpy(inA.slot(), img, sizeof(img));
	std::memcpy(inB.slot(), img, sizeof(img));
	std::memcpy(inRef.slot(), img, sizeof(img));

	long double _Complex za, zb;
	std::memcpy(&za, inA.slot(), sizeof(za));
	std::memcpy(&zb, inB.slot(), sizeof(zb));

	outA.fill();
	outB.fill();

	long double ra = port::creall(za);
	long double rr = ref_creall(zb);

	std::memcpy(outA.slot(), &ra, sizeof(ra));
	std::memcpy(outB.slot(), &rr, sizeof(rr));

	if (!buf_equal(outA, outB, L_ldouble) ||
	    !guards_intact(outA, L_ldouble) ||
	    !guards_intact(outB, L_ldouble)) {
		fail_report(s, outA, outB, L_ldouble, "result");
		return;
	}
	if (std::memcmp(inA.b, inRef.b, BUFSZ) != 0 ||
	    std::memcmp(inB.b, inRef.b, BUFSZ) != 0) {
		fail_report(s, inA, inB, L_ldcomplex, "argument buffer");
		return;
	}
}

void
check_conj(std::uint64_t rb, std::uint64_t ib)
{
	Stats &s = st_conj;
	unsigned char img[sizeof(double _Complex)];
	Buf inA, inB, inRef, outA, outB;

	s.cases++;

	std::memset(img, 0, sizeof(img));
	std::memcpy(img + 0 * sizeof(double), &rb, sizeof(rb));
	std::memcpy(img + 1 * sizeof(double), &ib, sizeof(ib));

	inA.fill();
	inB.fill();
	inRef.fill();
	std::memcpy(inA.slot(), img, sizeof(img));
	std::memcpy(inB.slot(), img, sizeof(img));
	std::memcpy(inRef.slot(), img, sizeof(img));

	double _Complex za, zb;
	std::memcpy(&za, inA.slot(), sizeof(za));
	std::memcpy(&zb, inB.slot(), sizeof(zb));

	outA.fill();
	outB.fill();

	double _Complex ra = port::conj(za);
	double _Complex rr = ref_conj(zb);

	std::memcpy(outA.slot(), &ra, sizeof(ra));
	std::memcpy(outB.slot(), &rr, sizeof(rr));

	if (!buf_equal(outA, outB, L_dcomplex) ||
	    !guards_intact(outA, L_dcomplex) ||
	    !guards_intact(outB, L_dcomplex)) {
		fail_report(s, outA, outB, L_dcomplex, "result");
		return;
	}
	if (std::memcmp(inA.b, inRef.b, BUFSZ) != 0 ||
	    std::memcmp(inB.b, inRef.b, BUFSZ) != 0) {
		fail_report(s, inA, inB, L_dcomplex, "argument buffer");
		return;
	}
}

/* ------------------------------------------------------------------ */

constexpr unsigned long RANDOM_ITERS = 250000;

void
edge_sweep()
{
	/*
	 * Full cross product of the edge tables: every special real part is
	 * combined with every special imaginary part, so the two components
	 * always differ somewhere and a port that returned the wrong one, or
	 * negated the wrong one, cannot hide.
	 */
	for (std::size_t i = 0; i < F_EDGE_N; i++)
		for (std::size_t j = 0; j < F_EDGE_N; j++)
			check_crealf(f_edge[i], f_edge[j]);

	for (std::size_t i = 0; i < D_EDGE_N; i++)
		for (std::size_t j = 0; j < D_EDGE_N; j++) {
			check_creal(d_edge[i], d_edge[j]);
			check_conj(d_edge[i], d_edge[j]);
		}

	for (std::size_t i = 0; i < LD_EDGE_N; i++)
		for (std::size_t j = 0; j < LD_EDGE_N; j++)
			check_creall(ld_edge[i], ld_edge[j]);

	/*
	 * conj() is the only function in the batch with an operator in it, so
	 * both sides of the sign of the imaginary part get their own cases:
	 * +0 must come back as -0 and -0 as +0, which only a bit comparison
	 * can see, and every exponent class is walked with the mantissa bit
	 * set and clear.
	 */
	static const std::uint64_t sgn = 0x8000000000000000ULL;
	for (std::size_t i = 0; i < D_EDGE_N; i++) {
		check_conj(d_edge[i], 0x0000000000000000ULL);
		check_conj(d_edge[i], 0x8000000000000000ULL);
		check_conj(d_edge[i], d_edge[i]);
		check_conj(d_edge[i], d_edge[i] ^ sgn);
		check_conj(d_edge[i] ^ sgn, d_edge[i]);
	}
	for (std::uint64_t e = 0; e <= 0x7ff; e++) {
		for (int mant = 0; mant < 2; mant++) {
			std::uint64_t m = mant ? 0x0008000000000000ULL : 0;
			std::uint64_t v = (e << 52) | m;
			check_conj(v, v);
			check_conj(v, v | sgn);
			check_conj(v | sgn, v);
			check_conj(v | sgn, v | sgn);
			check_creal(v, v | sgn);
			check_creal(v | sgn, v);
		}
	}
}

void
random_sweep()
{
	for (unsigned long n = 0; n < RANDOM_ITERS; n++) {
		check_crealf(rand_fbits(), rand_fbits());
		check_creal(rand_dbits(), rand_dbits());
		check_creall(rand_ldbits(), rand_ldbits());
		check_conj(rand_dbits(), rand_dbits());
	}
}

void
row(const Stats &s)
{
	std::printf("  %-10s %12lu %12lu   %s\n", s.name, s.cases, s.failures,
	    s.failures == 0 ? "ok" : "FAILED");
}

} /* namespace */

int
main()
{
	std::printf("b0057 differential test: port vs ref_ oracle\n");
	std::printf("  sizeof(long double)=%zu significant bytes=%zu\n",
	    sizeof(long double), LD_SIG);
	std::printf("  guard byte 0x%02x, buffer %zu bytes, value at +%zu\n\n",
	    GUARD, BUFSZ, OFF);

	edge_sweep();
	random_sweep();

	std::printf("\n  %-10s %12s %12s\n", "function", "cases", "failures");
	std::printf("  ---------- ------------ ------------\n");
	row(st_crealf);
	row(st_creal);
	row(st_creall);
	row(st_conj);

	unsigned long total = st_crealf.failures + st_creal.failures +
	    st_creall.failures + st_conj.failures;
	unsigned long cases = st_crealf.cases + st_creal.cases +
	    st_creall.cases + st_conj.cases;

	std::printf("  ---------- ------------ ------------\n");
	std::printf("  %-10s %12lu %12lu   %s\n", "TOTAL", cases, total,
	    total == 0 ? "ok" : "FAILED");

	return total == 0 ? 0 : 1;
}
