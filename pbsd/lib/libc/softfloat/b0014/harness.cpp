// harness.cpp -- differential test: PBSD port (b0014) vs. the C oracle.
//
// Every function in the port is driven against its ref_ counterpart in
// oracle.c.  For each case we compare:
//   * the return value, and
//   * the softfloat exception flag word after the call (the signaling-NaN
//     paths are only observable through it), and
//   * for the four batch entry points, the complete byte image of a
//     guard-filled (0x7f) block containing the argument, so that a port that
//     writes through or past its by-value arguments is caught.  These
//     functions take no buffers and return no pointers, so there is nothing
//     else with an address to compare.
//
// Cases are hand-written boundary values (zeros of both signs, subnormals,
// the smallest/largest normals, infinities, quiet and signaling NaNs,
// high-bit-set words, all-ones words, values differing in exactly one bit)
// crossed exhaustively, followed by fixed-seed randomised sweeps of >= 200000
// iterations each.

#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libc.softfloat.b0014;

namespace P = pbsd::lib_libc_softfloat::b0014;

// ---------------------------------------------------------------------
// oracle interface (layout-compatible mirrors of the oracle's types)
// ---------------------------------------------------------------------

struct RefF128 {
	std::uint64_t high, low;
};

struct RefX80 {
	std::uint16_t high;
	std::uint64_t low;
};

extern "C" {
extern int ref_float_exception_flags;

std::uint64_t ref_extractFloatx80Frac(RefX80);
int ref_extractFloatx80Exp(RefX80);
int ref_extractFloatx80Sign(RefX80);

std::uint64_t ref_extractFloat128Frac1(RefF128);
std::uint64_t ref_extractFloat128Frac0(RefF128);
int ref_extractFloat128Exp(RefF128);
int ref_extractFloat128Sign(RefF128);

int ref_lt128(std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t);

int ref_floatx80_is_signaling_nan(RefX80);
int ref_float128_is_signaling_nan(RefF128);

int ref_floatx80_eq(RefX80, RefX80);
int ref_float128_eq(RefF128, RefF128);
int ref_float128_lt(RefF128, RefF128);

int ref___nexf2(RefX80, RefX80);
int ref___gttf2(RefF128, RefF128);
int ref___netf2(RefF128, RefF128);
int ref___eqtf2(RefF128, RefF128);
}

// ---------------------------------------------------------------------
// bookkeeping
// ---------------------------------------------------------------------

struct Row {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
};

static Row rows[32];
static int nrows;

static Row &
mkrow(const char *name)
{
	Row &r = rows[nrows++];
	r.name = name;
	r.cases = 0;
	r.fails = 0;
	return r;
}

static void
report(Row &r, long long pv, int pf, long long rv, int rf,
    std::uint64_t o0, std::uint64_t o1, std::uint64_t o2, std::uint64_t o3,
    int nop, const char *tag)
{
	r.cases++;
	if (pv == rv && pf == rf)
		return;
	r.fails++;
	if (r.fails > 10)
		return;
	std::printf("  FAIL %-28s %-10s", r.name, tag);
	for (int i = 0; i < nop; i++) {
		const std::uint64_t o[4] = { o0, o1, o2, o3 };
		std::printf(" %016llx", (unsigned long long)o[i]);
	}
	std::printf("  port=(%lld,fl=%d) ref=(%lld,fl=%d)\n", pv, pf, rv, rf);
}

static void
report_bytes(Row &r, const char *what, const char *tag)
{
	r.cases++;
	r.fails++;
	if (r.fails <= 10)
		std::printf("  FAIL %-28s %-10s guard/argument image mismatch (%s)\n",
		    r.name, tag, what);
}

// Both implementations start every call from the same exception flag word.
// The word is rotated through non-zero values as well as zero so that a port
// which overwrites the flags instead of merging into them is visible.
static const int presets[] = { 0, 1, 2, 8, 0x0B, 4, 0x0F, 0x10 };
static unsigned preset_ix;

static void
begin()
{
	int p = presets[preset_ix++ % (sizeof presets / sizeof presets[0])];

	P::set_float_exception_flags(p);
	ref_float_exception_flags = p;
}

// ---------------------------------------------------------------------
// deterministic PRNG (splitmix64)
// ---------------------------------------------------------------------

static std::uint64_t rng_state;

static void
rng_seed(std::uint64_t s)
{
	rng_state = s;
}

static std::uint64_t
nx()
{
	rng_state += 0x9E3779B97F4A7C15ULL;
	std::uint64_t z = rng_state;
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

// ---------------------------------------------------------------------
// hand-written corpora
// ---------------------------------------------------------------------

struct V128 {
	std::uint64_t hi, lo;
};

static const V128 corpus128[] = {
	{ 0x0000000000000000ULL, 0x0000000000000000ULL }, // +0
	{ 0x8000000000000000ULL, 0x0000000000000000ULL }, // -0
	{ 0x0000000000000000ULL, 0x0000000000000001ULL }, // + tiniest subnormal
	{ 0x8000000000000000ULL, 0x0000000000000001ULL }, // - tiniest subnormal
	{ 0x0000000000000001ULL, 0x0000000000000000ULL }, // exp 0, frac0 bit 0
	{ 0x8000000000000001ULL, 0x0000000000000000ULL },
	{ 0x0000FFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL }, // largest subnormal
	{ 0x8000FFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL },
	{ 0x0001000000000000ULL, 0x0000000000000000ULL }, // smallest normal
	{ 0x8001000000000000ULL, 0x0000000000000000ULL },
	{ 0x0001000000000000ULL, 0x0000000000000001ULL },
	{ 0x0002000000000000ULL, 0x0000000000000000ULL },
	{ 0x3FFE000000000000ULL, 0x0000000000000000ULL }, // 0.5
	{ 0x3FFF000000000000ULL, 0x0000000000000000ULL }, // 1.0
	{ 0xBFFF000000000000ULL, 0x0000000000000000ULL }, // -1.0
	{ 0x3FFF000000000000ULL, 0x0000000000000001ULL }, // nextafter(1,inf)
	{ 0xBFFF000000000000ULL, 0x0000000000000001ULL },
	{ 0x4000000000000000ULL, 0x0000000000000000ULL }, // 2.0
	{ 0xC000000000000000ULL, 0x0000000000000000ULL }, // -2.0
	{ 0x4000000000000000ULL, 0xFFFFFFFFFFFFFFFFULL },
	{ 0x7FFE000000000000ULL, 0x0000000000000000ULL },
	{ 0xFFFE000000000000ULL, 0x0000000000000000ULL },
	{ 0x7FFEFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL }, // largest normal
	{ 0xFFFEFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL },
	{ 0x7FFF000000000000ULL, 0x0000000000000000ULL }, // +inf
	{ 0xFFFF000000000000ULL, 0x0000000000000000ULL }, // -inf
	{ 0x7FFF000000000000ULL, 0x0000000000000001ULL }, // +snan via frac1
	{ 0xFFFF000000000000ULL, 0x0000000000000001ULL }, // -snan via frac1
	{ 0x7FFF000000000001ULL, 0x0000000000000000ULL }, // +snan via frac0
	{ 0xFFFF000000000001ULL, 0x0000000000000000ULL }, // -snan via frac0
	{ 0x7FFF7FFFFFFFFFFFULL, 0x0000000000000000ULL }, // snan, quiet bit clear
	{ 0x7FFF800000000000ULL, 0x0000000000000000ULL }, // +qnan
	{ 0xFFFF800000000000ULL, 0x0000000000000000ULL }, // -qnan
	{ 0x7FFF800000000000ULL, 0x0000000000000001ULL }, // +qnan w/ frac1
	{ 0x7FFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL }, // qnan, all ones
	{ 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL },
	{ 0x0000000000000000ULL, 0x8000000000000000ULL }, // high-bit-only low
	{ 0x8000000000000000ULL, 0x8000000000000000ULL },
	{ 0x0000000000000000ULL, 0x0000000000000005ULL }, // low equal, high signs
	{ 0x8000000000000000ULL, 0x0000000000000005ULL },
	{ 0x0000000000000000ULL, 0xFFFFFFFFFFFFFFFFULL },
	{ 0x8000000000000000ULL, 0xFFFFFFFFFFFFFFFFULL },
};

struct VX80 {
	std::uint16_t hi;
	std::uint64_t lo;
};

static const VX80 corpusx80[] = {
	{ 0x0000, 0x0000000000000000ULL }, // +0
	{ 0x8000, 0x0000000000000000ULL }, // -0
	{ 0x0000, 0x0000000000000001ULL }, // + tiniest subnormal
	{ 0x8000, 0x0000000000000001ULL },
	{ 0x0000, 0x8000000000000000ULL }, // pseudo-denormal
	{ 0x8000, 0x8000000000000000ULL },
	{ 0x0000, 0xFFFFFFFFFFFFFFFFULL },
	{ 0x8000, 0xFFFFFFFFFFFFFFFFULL },
	{ 0x0000, 0x0000000000000005ULL },
	{ 0x8000, 0x0000000000000005ULL },
	{ 0x0001, 0x0000000000000000ULL }, // exp 1, frac 0
	{ 0x8001, 0x0000000000000000ULL },
	{ 0x0001, 0x8000000000000000ULL }, // smallest normal
	{ 0x8001, 0x8000000000000000ULL },
	{ 0x3FFE, 0x8000000000000000ULL }, // 0.5
	{ 0x3FFF, 0x8000000000000000ULL }, // 1.0
	{ 0xBFFF, 0x8000000000000000ULL }, // -1.0
	{ 0x3FFF, 0x8000000000000001ULL },
	{ 0x4000, 0x8000000000000000ULL }, // 2.0
	{ 0xC000, 0x8000000000000000ULL },
	{ 0x4000, 0x0000000000000000ULL },
	{ 0x7FFE, 0xFFFFFFFFFFFFFFFFULL }, // largest normal
	{ 0xFFFE, 0xFFFFFFFFFFFFFFFFULL },
	{ 0x7FFF, 0x8000000000000000ULL }, // +inf
	{ 0xFFFF, 0x8000000000000000ULL }, // -inf
	{ 0x7FFF, 0x8000000000000001ULL }, // +snan
	{ 0xFFFF, 0x8000000000000001ULL }, // -snan
	{ 0x7FFF, 0xC000000000000000ULL }, // +qnan
	{ 0xFFFF, 0xC000000000000000ULL }, // -qnan
	{ 0x7FFF, 0xFFFFFFFFFFFFFFFFULL }, // qnan, all ones
	{ 0x7FFF, 0x0000000000000000ULL }, // exp max, frac 0 (unsupported)
	{ 0xFFFF, 0x0000000000000000ULL },
	{ 0x7FFF, 0x0000000000000001ULL }, // exp max, frac 1 -> snan
	{ 0x7FFF, 0x4000000000000000ULL }, // exp max, quiet bit only
	{ 0xFFFF, 0x4000000000000000ULL },
};

static const std::uint64_t words64[] = {
	0x0000000000000000ULL,
	0x0000000000000001ULL,
	0x0000000000000002ULL,
	0x0000000000000005ULL,
	0x00000000FFFFFFFFULL,
	0x7FFFFFFFFFFFFFFFULL,
	0x8000000000000000ULL,
	0x8000000000000001ULL,
	0xFFFFFFFFFFFFFFFFULL,
};

// ---------------------------------------------------------------------
// guard-block helper
// ---------------------------------------------------------------------

template <class T>
struct Guarded {
	unsigned char pre[24];
	T v;
	unsigned char post[24];
};

// ---------------------------------------------------------------------
// rows
// ---------------------------------------------------------------------

static Row *r_nexf2, *r_gttf2, *r_netf2, *r_eqtf2;
static Row *r_f128eq, *r_f128lt, *r_x80eq;
static Row *r_f128snan, *r_x80snan;
static Row *r_f128exp, *r_f128frac0, *r_f128frac1, *r_f128sign;
static Row *r_x80exp, *r_x80frac, *r_x80sign;
static Row *r_lt128;

// ---------------------------------------------------------------------
// unary probes
// ---------------------------------------------------------------------

static void
probe128_unary(std::uint64_t hi, std::uint64_t lo, const char *tag)
{
	P::float128 pa{ hi, lo };
	RefF128 ra{ hi, lo };
	long long pv, rv;
	int pf, rf;

	begin();
	pv = P::extractFloat128Exp(pa);
	pf = P::get_float_exception_flags();
	rv = ref_extractFloat128Exp(ra);
	rf = ref_float_exception_flags;
	report(*r_f128exp, pv, pf, rv, rf, hi, lo, 0, 0, 2, tag);

	begin();
	pv = (long long)P::extractFloat128Frac0(pa);
	pf = P::get_float_exception_flags();
	rv = (long long)ref_extractFloat128Frac0(ra);
	rf = ref_float_exception_flags;
	report(*r_f128frac0, pv, pf, rv, rf, hi, lo, 0, 0, 2, tag);

	begin();
	pv = (long long)P::extractFloat128Frac1(pa);
	pf = P::get_float_exception_flags();
	rv = (long long)ref_extractFloat128Frac1(ra);
	rf = ref_float_exception_flags;
	report(*r_f128frac1, pv, pf, rv, rf, hi, lo, 0, 0, 2, tag);

	begin();
	pv = P::extractFloat128Sign(pa);
	pf = P::get_float_exception_flags();
	rv = ref_extractFloat128Sign(ra);
	rf = ref_float_exception_flags;
	report(*r_f128sign, pv, pf, rv, rf, hi, lo, 0, 0, 2, tag);

	begin();
	pv = P::float128_is_signaling_nan(pa);
	pf = P::get_float_exception_flags();
	rv = ref_float128_is_signaling_nan(ra);
	rf = ref_float_exception_flags;
	report(*r_f128snan, pv, pf, rv, rf, hi, lo, 0, 0, 2, tag);
}

static void
probex80_unary(std::uint16_t hi, std::uint64_t lo, const char *tag)
{
	P::floatx80 pa{ hi, lo };
	RefX80 ra{ hi, lo };
	long long pv, rv;
	int pf, rf;

	begin();
	pv = P::extractFloatx80Exp(pa);
	pf = P::get_float_exception_flags();
	rv = ref_extractFloatx80Exp(ra);
	rf = ref_float_exception_flags;
	report(*r_x80exp, pv, pf, rv, rf, hi, lo, 0, 0, 2, tag);

	begin();
	pv = (long long)P::extractFloatx80Frac(pa);
	pf = P::get_float_exception_flags();
	rv = (long long)ref_extractFloatx80Frac(ra);
	rf = ref_float_exception_flags;
	report(*r_x80frac, pv, pf, rv, rf, hi, lo, 0, 0, 2, tag);

	begin();
	pv = P::extractFloatx80Sign(pa);
	pf = P::get_float_exception_flags();
	rv = ref_extractFloatx80Sign(ra);
	rf = ref_float_exception_flags;
	report(*r_x80sign, pv, pf, rv, rf, hi, lo, 0, 0, 2, tag);

	begin();
	pv = P::floatx80_is_signaling_nan(pa);
	pf = P::get_float_exception_flags();
	rv = ref_floatx80_is_signaling_nan(ra);
	rf = ref_float_exception_flags;
	report(*r_x80snan, pv, pf, rv, rf, hi, lo, 0, 0, 2, tag);
}

// ---------------------------------------------------------------------
// binary probes
// ---------------------------------------------------------------------

static void
probe_lt128(std::uint64_t a0, std::uint64_t a1, std::uint64_t b0,
    std::uint64_t b1, const char *tag)
{
	begin();
	long long pv = P::lt128(a0, a1, b0, b1);
	int pf = P::get_float_exception_flags();
	long long rv = ref_lt128(a0, a1, b0, b1);
	int rf = ref_float_exception_flags;
	report(*r_lt128, pv, pf, rv, rf, a0, a1, b0, b1, 4, tag);
}

// The four batch entry points plus the two float128 primitives they use.
// The batch entry points are additionally called with their arguments sitting
// inside guard blocks whose entire byte image is compared afterwards.
static void
probe128_binary(std::uint64_t ah, std::uint64_t al, std::uint64_t bh,
    std::uint64_t bl, const char *tag, bool guards)
{
	P::float128 pa{ ah, al }, pb{ bh, bl };
	RefF128 ra{ ah, al }, rb{ bh, bl };
	long long pv, rv;
	int pf, rf;

	begin();
	pv = P::float128_eq(pa, pb);
	pf = P::get_float_exception_flags();
	rv = ref_float128_eq(ra, rb);
	rf = ref_float_exception_flags;
	report(*r_f128eq, pv, pf, rv, rf, ah, al, bh, bl, 4, tag);

	begin();
	pv = P::float128_lt(pa, pb);
	pf = P::get_float_exception_flags();
	rv = ref_float128_lt(ra, rb);
	rf = ref_float_exception_flags;
	report(*r_f128lt, pv, pf, rv, rf, ah, al, bh, bl, 4, tag);

	begin();
	pv = P::__eqtf2(pa, pb);
	pf = P::get_float_exception_flags();
	rv = ref___eqtf2(ra, rb);
	rf = ref_float_exception_flags;
	report(*r_eqtf2, pv, pf, rv, rf, ah, al, bh, bl, 4, tag);

	begin();
	pv = P::__netf2(pa, pb);
	pf = P::get_float_exception_flags();
	rv = ref___netf2(ra, rb);
	rf = ref_float_exception_flags;
	report(*r_netf2, pv, pf, rv, rf, ah, al, bh, bl, 4, tag);

	begin();
	pv = P::__gttf2(pa, pb);
	pf = P::get_float_exception_flags();
	rv = ref___gttf2(ra, rb);
	rf = ref_float_exception_flags;
	report(*r_gttf2, pv, pf, rv, rf, ah, al, bh, bl, 4, tag);

	if (!guards)
		return;

	typedef Guarded<P::float128> GP;
	typedef Guarded<RefF128> GR;
	static_assert(sizeof(GP) == sizeof(GR), "guard block layout");

	GP gpa, gpb;
	GR gra, grb;
	unsigned char snap_pa[sizeof(GP)], snap_pb[sizeof(GP)];
	unsigned char snap_ra[sizeof(GR)], snap_rb[sizeof(GR)];

	std::memset(&gpa, 0x7f, sizeof gpa);
	std::memset(&gpb, 0x7f, sizeof gpb);
	std::memset(&gra, 0x7f, sizeof gra);
	std::memset(&grb, 0x7f, sizeof grb);
	gpa.v.high = ah; gpa.v.low = al;
	gpb.v.high = bh; gpb.v.low = bl;
	gra.v.high = ah; gra.v.low = al;
	grb.v.high = bh; grb.v.low = bl;
	std::memcpy(snap_pa, &gpa, sizeof gpa);
	std::memcpy(snap_pb, &gpb, sizeof gpb);
	std::memcpy(snap_ra, &gra, sizeof gra);
	std::memcpy(snap_rb, &grb, sizeof grb);

	struct Case {
		Row *row;
		long long p;
		long long r;
	};
	Case cs[3];

	begin();
	cs[0].row = r_eqtf2;
	cs[0].p = P::__eqtf2(gpa.v, gpb.v);
	cs[0].r = ref___eqtf2(gra.v, grb.v);
	cs[1].row = r_netf2;
	cs[1].p = P::__netf2(gpa.v, gpb.v);
	cs[1].r = ref___netf2(gra.v, grb.v);
	cs[2].row = r_gttf2;
	cs[2].p = P::__gttf2(gpa.v, gpb.v);
	cs[2].r = ref___gttf2(gra.v, grb.v);

	for (int i = 0; i < 3; i++) {
		Row &row = *cs[i].row;
		if (std::memcmp(snap_pa, &gpa, sizeof gpa) != 0 ||
		    std::memcmp(snap_pb, &gpb, sizeof gpb) != 0)
			report_bytes(row, "port block modified", tag);
		else if (std::memcmp(snap_ra, &gra, sizeof gra) != 0 ||
		    std::memcmp(snap_rb, &grb, sizeof grb) != 0)
			report_bytes(row, "oracle block modified", tag);
		else if (std::memcmp(&gpa, &gra, sizeof gpa) != 0 ||
		    std::memcmp(&gpb, &grb, sizeof gpb) != 0)
			report_bytes(row, "port vs oracle block", tag);
		else if (cs[i].p != cs[i].r)
			report_bytes(row, "guarded return value", tag);
		else
			row.cases++;
	}
}

static void
probex80_binary(std::uint16_t ah, std::uint64_t al, std::uint16_t bh,
    std::uint64_t bl, const char *tag, bool guards)
{
	P::floatx80 pa{ ah, al }, pb{ bh, bl };
	RefX80 ra{ ah, al }, rb{ bh, bl };
	long long pv, rv;
	int pf, rf;

	begin();
	pv = P::floatx80_eq(pa, pb);
	pf = P::get_float_exception_flags();
	rv = ref_floatx80_eq(ra, rb);
	rf = ref_float_exception_flags;
	report(*r_x80eq, pv, pf, rv, rf, ah, al, bh, bl, 4, tag);

	begin();
	pv = P::__nexf2(pa, pb);
	pf = P::get_float_exception_flags();
	rv = ref___nexf2(ra, rb);
	rf = ref_float_exception_flags;
	report(*r_nexf2, pv, pf, rv, rf, ah, al, bh, bl, 4, tag);

	if (!guards)
		return;

	typedef Guarded<P::floatx80> GP;
	typedef Guarded<RefX80> GR;
	static_assert(sizeof(GP) == sizeof(GR), "guard block layout");

	GP gpa, gpb;
	GR gra, grb;
	unsigned char snap_pa[sizeof(GP)], snap_pb[sizeof(GP)];
	unsigned char snap_ra[sizeof(GR)], snap_rb[sizeof(GR)];

	std::memset(&gpa, 0x7f, sizeof gpa);
	std::memset(&gpb, 0x7f, sizeof gpb);
	std::memset(&gra, 0x7f, sizeof gra);
	std::memset(&grb, 0x7f, sizeof grb);
	gpa.v.high = ah; gpa.v.low = al;
	gpb.v.high = bh; gpb.v.low = bl;
	gra.v.high = ah; gra.v.low = al;
	grb.v.high = bh; grb.v.low = bl;
	std::memcpy(snap_pa, &gpa, sizeof gpa);
	std::memcpy(snap_pb, &gpb, sizeof gpb);
	std::memcpy(snap_ra, &gra, sizeof gra);
	std::memcpy(snap_rb, &grb, sizeof grb);

	begin();
	long long gp = P::__nexf2(gpa.v, gpb.v);
	long long gr = ref___nexf2(gra.v, grb.v);

	Row &row = *r_nexf2;
	if (std::memcmp(snap_pa, &gpa, sizeof gpa) != 0 ||
	    std::memcmp(snap_pb, &gpb, sizeof gpb) != 0)
		report_bytes(row, "port block modified", tag);
	else if (std::memcmp(snap_ra, &gra, sizeof gra) != 0 ||
	    std::memcmp(snap_rb, &grb, sizeof grb) != 0)
		report_bytes(row, "oracle block modified", tag);
	else if (std::memcmp(&gpa, &gra, sizeof gpa) != 0 ||
	    std::memcmp(&gpb, &grb, sizeof gpb) != 0)
		report_bytes(row, "port vs oracle block", tag);
	else if (gp != gr)
		report_bytes(row, "guarded return value", tag);
	else
		row.cases++;
}

// ---------------------------------------------------------------------
// randomised generators
// ---------------------------------------------------------------------

static void
gen128(std::uint64_t &hi, std::uint64_t &lo)
{
	std::uint64_t sign = nx() & 1;
	std::uint64_t exp;

	switch (nx() % 6) {
	case 0: exp = 0; break;
	case 1: exp = 1; break;
	case 2: exp = 0x7FFF; break;
	case 3: exp = 0x7FFE; break;
	case 4: exp = 0x3FFF; break;
	default: exp = nx() & 0x7FFF; break;
	}

	std::uint64_t frac0;
	switch (nx() % 6) {
	case 0: frac0 = 0; break;
	case 1: frac0 = 1; break;
	case 2: frac0 = 0x0000FFFFFFFFFFFFULL; break;
	case 3: frac0 = 0x0000800000000000ULL; break;
	case 4: frac0 = 0x00007FFFFFFFFFFFULL; break;
	default: frac0 = nx() & 0x0000FFFFFFFFFFFFULL; break;
	}

	hi = (sign << 63) | (exp << 48) | frac0;

	switch (nx() % 6) {
	case 0: lo = 0; break;
	case 1: lo = 1; break;
	case 2: lo = 0xFFFFFFFFFFFFFFFFULL; break;
	case 3: lo = 0x8000000000000000ULL; break;
	case 4: lo = nx() & 0xFFFF; break;
	default: lo = nx(); break;
	}
}

static void
genx80(std::uint16_t &hi, std::uint64_t &lo)
{
	std::uint32_t sign = (std::uint32_t)(nx() & 1);
	std::uint32_t exp;

	switch (nx() % 6) {
	case 0: exp = 0; break;
	case 1: exp = 1; break;
	case 2: exp = 0x7FFF; break;
	case 3: exp = 0x7FFE; break;
	case 4: exp = 0x3FFF; break;
	default: exp = (std::uint32_t)(nx() & 0x7FFF); break;
	}

	hi = (std::uint16_t)((sign << 15) | exp);

	switch (nx() % 8) {
	case 0: lo = 0; break;
	case 1: lo = 1; break;
	case 2: lo = 0xFFFFFFFFFFFFFFFFULL; break;
	case 3: lo = 0x8000000000000000ULL; break;
	case 4: lo = 0xC000000000000000ULL; break;
	case 5: lo = 0x8000000000000001ULL; break;
	case 6: lo = 0x4000000000000000ULL; break;
	default: lo = nx(); break;
	}
}

// Derive b from a so that the equality branches are exercised often.
static void
derive128(std::uint64_t ah, std::uint64_t al, std::uint64_t &bh,
    std::uint64_t &bl)
{
	switch (nx() % 6) {
	case 0:
		bh = ah; bl = al;
		break;
	case 1: {
		unsigned bit = (unsigned)(nx() % 128);
		bh = ah; bl = al;
		if (bit < 64)
			bl ^= 1ULL << bit;
		else
			bh ^= 1ULL << (bit - 64);
		break;
	}
	case 2:
		bh = ah ^ 0x8000000000000000ULL; bl = al;
		break;
	case 3:
		bh = ah; bl = al ^ 1ULL;
		break;
	case 4:
		gen128(bh, bl);
		bl = al;
		break;
	default:
		gen128(bh, bl);
		break;
	}
}

static void
derivex80(std::uint16_t ah, std::uint64_t al, std::uint16_t &bh,
    std::uint64_t &bl)
{
	switch (nx() % 6) {
	case 0:
		bh = ah; bl = al;
		break;
	case 1: {
		unsigned bit = (unsigned)(nx() % 80);
		bh = ah; bl = al;
		if (bit < 64)
			bl ^= 1ULL << bit;
		else
			bh = (std::uint16_t)(bh ^ (1u << (bit - 64)));
		break;
	}
	case 2:
		bh = (std::uint16_t)(ah ^ 0x8000u); bl = al;
		break;
	case 3:
		bh = ah; bl = al ^ 1ULL;
		break;
	case 4:
		genx80(bh, bl);
		bl = al;
		break;
	default:
		genx80(bh, bl);
		break;
	}
}

static std::uint64_t
gen64()
{
	switch (nx() % 6) {
	case 0: return 0;
	case 1: return 1;
	case 2: return 0xFFFFFFFFFFFFFFFFULL;
	case 3: return 0x8000000000000000ULL;
	case 4: return nx() & 3;
	default: return nx();
	}
}

// ---------------------------------------------------------------------

int
main()
{
	const int N128 = sizeof corpus128 / sizeof corpus128[0];
	const int NX80 = sizeof corpusx80 / sizeof corpusx80[0];
	const int NW = sizeof words64 / sizeof words64[0];
	const long SWEEP = 300000;

	r_nexf2 = &mkrow("__nexf2");
	r_gttf2 = &mkrow("__gttf2");
	r_netf2 = &mkrow("__netf2");
	r_eqtf2 = &mkrow("__eqtf2");
	r_f128eq = &mkrow("float128_eq");
	r_f128lt = &mkrow("float128_lt");
	r_x80eq = &mkrow("floatx80_eq");
	r_f128snan = &mkrow("float128_is_signaling_nan");
	r_x80snan = &mkrow("floatx80_is_signaling_nan");
	r_f128exp = &mkrow("extractFloat128Exp");
	r_f128frac0 = &mkrow("extractFloat128Frac0");
	r_f128frac1 = &mkrow("extractFloat128Frac1");
	r_f128sign = &mkrow("extractFloat128Sign");
	r_x80exp = &mkrow("extractFloatx80Exp");
	r_x80frac = &mkrow("extractFloatx80Frac");
	r_x80sign = &mkrow("extractFloatx80Sign");
	r_lt128 = &mkrow("lt128");

	/* Hand-written edge cases: every corpus value, and every ordered
	 * pair of corpus values. */
	for (int i = 0; i < N128; i++)
		probe128_unary(corpus128[i].hi, corpus128[i].lo, "edge");
	for (int i = 0; i < NX80; i++)
		probex80_unary(corpusx80[i].hi, corpusx80[i].lo, "edge");

	for (int i = 0; i < N128; i++)
		for (int j = 0; j < N128; j++)
			probe128_binary(corpus128[i].hi, corpus128[i].lo,
			    corpus128[j].hi, corpus128[j].lo, "edge", true);

	for (int i = 0; i < NX80; i++)
		for (int j = 0; j < NX80; j++)
			probex80_binary(corpusx80[i].hi, corpusx80[i].lo,
			    corpusx80[j].hi, corpusx80[j].lo, "edge", true);

	/* lt128: exhaustive over the boundary word set (equal high words,
	 * equal low words, both orders, all-ones and high-bit words). */
	for (int i = 0; i < NW; i++)
		for (int j = 0; j < NW; j++)
			for (int k = 0; k < NW; k++)
				for (int l = 0; l < NW; l++)
					probe_lt128(words64[i], words64[j],
					    words64[k], words64[l], "edge");

	/* Fixed-seed randomised sweeps. */
	rng_seed(0x0B0014C0FFEE1234ULL);
	for (long n = 0; n < SWEEP; n++) {
		std::uint64_t ah, al, bh, bl;
		gen128(ah, al);
		derive128(ah, al, bh, bl);
		probe128_unary(ah, al, "rand");
		probe128_unary(bh, bl, "rand");
		probe128_binary(ah, al, bh, bl, "rand", (n % 1024) == 0);
	}

	rng_seed(0x00B0014BEEF5678ULL);
	for (long n = 0; n < SWEEP; n++) {
		std::uint16_t ah, bh;
		std::uint64_t al, bl;
		genx80(ah, al);
		derivex80(ah, al, bh, bl);
		probex80_unary(ah, al, "rand");
		probex80_unary(bh, bl, "rand");
		probex80_binary(ah, al, bh, bl, "rand", (n % 1024) == 0);
	}

	rng_seed(0x00000B0014ABCDEFULL);
	for (long n = 0; n < SWEEP; n++) {
		std::uint64_t a0 = gen64();
		std::uint64_t a1 = gen64();
		std::uint64_t b0, b1;
		switch (nx() % 4) {
		case 0: b0 = a0; b1 = a1; break;
		case 1: b0 = a0; b1 = gen64(); break;
		case 2: b0 = gen64(); b1 = a1; break;
		default: b0 = gen64(); b1 = gen64(); break;
		}
		probe_lt128(a0, a1, b0, b1, "rand");
	}

	unsigned long long total_cases = 0, total_fails = 0;
	std::printf("\n%-28s %12s %10s\n", "function", "cases", "failures");
	std::printf("%-28s %12s %10s\n", "----------------------------",
	    "------------", "----------");
	for (int i = 0; i < nrows; i++) {
		std::printf("%-28s %12llu %10llu\n", rows[i].name,
		    rows[i].cases, rows[i].fails);
		total_cases += rows[i].cases;
		total_fails += rows[i].fails;
	}
	std::printf("%-28s %12llu %10llu\n", "TOTAL", total_cases, total_fails);

	if (total_fails != 0) {
		std::printf("\nRESULT: FAIL (%llu mismatches)\n", total_fails);
		return 1;
	}
	std::printf("\nRESULT: PASS\n");
	return 0;
}
