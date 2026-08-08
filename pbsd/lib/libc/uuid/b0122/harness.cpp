/*
 * PBSD batch b0122 -- differential test harness.
 *
 * Every function of the port is run side by side with the corresponding
 * ref_ function from oracle.c.  Both sides write into their own guard-filled
 * arena; the ENTIRE arena is compared afterwards, not just the sixteen bytes
 * a uuid_t nominally occupies.
 *
 * uuid_create() calls uuidgen(2), which is a kernel service rather than part
 * of the ported code.  This file supplies the one and only definition of the
 * uuidgen symbol that both the port and the oracle link against: a
 * deterministic, seedable test double that also records the arguments it was
 * handed.  Both sides are therefore driven from identical generator state,
 * and a port that passes the wrong count -- or the wrong pointer -- to
 * uuidgen is caught by comparing the recorded arguments and by comparing the
 * bytes the double wrote.
 */

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.libc.uuid.b0122;

namespace port = pbsd::lib_libc_uuid::b0122;

/* ------------------------------------------------------------------ */
/* Oracle interface                                                     */
/* ------------------------------------------------------------------ */

extern "C" {

struct ref_uuid_s {
	std::uint32_t	time_low;
	std::uint16_t	time_mid;
	std::uint16_t	time_hi_and_version;
	std::uint8_t	clock_seq_hi_and_reserved;
	std::uint8_t	clock_seq_low;
	std::uint8_t	node[6];
};

void ref_uuid_create(struct ref_uuid_s *u, std::uint32_t *status);
void ref_uuid_create_nil(struct ref_uuid_s *u, std::uint32_t *status);
std::uint16_t ref_uuid_hash(const struct ref_uuid_s *u, std::uint32_t *status);

}

static_assert(sizeof(port::uuid) == 16, "unexpected port uuid layout");
static_assert(sizeof(struct ref_uuid_s) == 16, "unexpected oracle uuid layout");
static_assert(offsetof(port::uuid, time_low) == 0);
static_assert(offsetof(port::uuid, time_mid) == 4);
static_assert(offsetof(port::uuid, time_hi_and_version) == 6);
static_assert(offsetof(port::uuid, clock_seq_hi_and_reserved) == 8);
static_assert(offsetof(port::uuid, clock_seq_low) == 9);
static_assert(offsetof(port::uuid, node) == 10);

/* ------------------------------------------------------------------ */
/* uuidgen(2) test double, shared by port and oracle                    */
/* ------------------------------------------------------------------ */

static std::uint64_t	gen_state;
static long		gen_calls;
static const void      *gen_last_store;
static int		gen_last_count;
static int		gen_last_ret;

/*
 * The real uuidgen(2) refuses count < 1 or count > 2048.  This double uses a
 * smaller upper bound purely so that the arenas below can be sized to hold
 * any accepted request; a rejected request writes nothing, which is exactly
 * what the real call does, so nothing is hidden by the tighter bound.
 */
#define GEN_MAX_COUNT	8

extern "C" int
uuidgen(void *store, int count)
{

	gen_calls++;
	gen_last_store = store;
	gen_last_count = count;

	if (store == nullptr || count < 1 || count > GEN_MAX_COUNT) {
		gen_last_ret = -1;
		return (-1);
	}

	unsigned char *p = static_cast<unsigned char *>(store);
	std::size_t n = static_cast<std::size_t>(count) * 16u;
	for (std::size_t i = 0; i < n; i++) {
		gen_state = gen_state * 6364136223846793005ULL +
		    1442695040888963407ULL;
		p[i] = static_cast<unsigned char>(gen_state >> 33);
	}
	gen_last_ret = 0;
	return (0);
}

static void
gen_reset(std::uint64_t seed)
{

	gen_state = seed;
	gen_calls = 0;
	gen_last_store = nullptr;
	gen_last_count = INT_MIN;
	gen_last_ret = INT_MIN;
}

/* ------------------------------------------------------------------ */
/* Arenas                                                               */
/* ------------------------------------------------------------------ */

#define GUARD	0x7f

/*
 * UOFF is where the uuid_t lives.  There are guard bytes both before it and
 * well past the end of the largest write the uuidgen double can perform
 * (GEN_MAX_COUNT * 16 == 128 bytes), so under-runs and over-runs both show up.
 */
#define ASZ	256
#define UOFF	32

#define SSZ	64
#define SOFF	16

alignas(16) static unsigned char A[ASZ];	/* port arena */
alignas(16) static unsigned char B[ASZ];	/* oracle arena */
alignas(16) static unsigned char SA[SSZ];	/* port status arena */
alignas(16) static unsigned char SB[SSZ];	/* oracle status arena */

static void
arenas_init(const unsigned char in[16], bool u_null)
{

	std::memset(A, GUARD, ASZ);
	std::memset(B, GUARD, ASZ);
	std::memset(SA, GUARD, SSZ);
	std::memset(SB, GUARD, SSZ);
	if (!u_null) {
		std::memcpy(A + UOFF, in, 16);
		std::memcpy(B + UOFF, in, 16);
	}
}

static long
store_offset(const void *p, const unsigned char *base)
{

	if (p == nullptr)
		return (-1);
	return (static_cast<const unsigned char *>(p) - base);
}

/* ------------------------------------------------------------------ */
/* Bookkeeping                                                          */
/* ------------------------------------------------------------------ */

struct Fn {
	const char     *name;
	long		cases;
	long		fails;
	long		reported;
};

static Fn fn_create	= { "uuid_create", 0, 0, 0 };
static Fn fn_create_nil	= { "uuid_create_nil", 0, 0, 0 };
static Fn fn_hash	= { "uuid_hash", 0, 0, 0 };

#define MAX_REPORTS	8

static void
hex16(char *out, const unsigned char *p)
{

	for (int i = 0; i < 16; i++)
		std::snprintf(out + i * 2, 3, "%02x", p[i]);
}

static void
fail(Fn &f, const unsigned char in[16], bool u_null, bool with_status,
    unsigned long long seed, const char *what)
{

	f.fails++;
	if (f.reported >= MAX_REPORTS)
		return;
	f.reported++;

	char hx[33];
	hex16(hx, in);
	std::printf("  FAIL %-16s in=%s u=%s status=%s seed=%llu : %s\n",
	    f.name, u_null ? "(null)" : hx, u_null ? "NULL" : "ptr",
	    with_status ? "ptr" : "NULL", seed, what);
}

static void
diff_arena(Fn &f, const unsigned char in[16], bool u_null, bool with_status,
    unsigned long long seed, const char *tag, const unsigned char *pa,
    const unsigned char *pb, std::size_t n)
{

	for (std::size_t i = 0; i < n; i++) {
		if (pa[i] != pb[i]) {
			char msg[160];
			std::snprintf(msg, sizeof(msg),
			    "%s differs at byte %zu: port=0x%02x ref=0x%02x",
			    tag, i, pa[i], pb[i]);
			fail(f, in, u_null, with_status, seed, msg);
			return;
		}
	}
}

/* ------------------------------------------------------------------ */
/* Per-function differential checks                                     */
/* ------------------------------------------------------------------ */

static void
check_create(const unsigned char in[16], bool u_null, bool with_status,
    std::uint64_t seed)
{
	Fn &f = fn_create;

	f.cases++;
	arenas_init(in, u_null);

	gen_reset(seed);
	port::uuid_create(u_null ? nullptr
	    : reinterpret_cast<port::uuid_t *>(A + UOFF),
	    with_status ? reinterpret_cast<std::uint32_t *>(SA + SOFF)
	    : nullptr);
	long p_calls = gen_calls;
	long p_off = store_offset(gen_last_store, A);
	int p_count = gen_last_count;
	int p_ret = gen_last_ret;

	gen_reset(seed);
	ref_uuid_create(u_null ? nullptr
	    : reinterpret_cast<struct ref_uuid_s *>(B + UOFF),
	    with_status ? reinterpret_cast<std::uint32_t *>(SB + SOFF)
	    : nullptr);
	long r_calls = gen_calls;
	long r_off = store_offset(gen_last_store, B);
	int r_count = gen_last_count;
	int r_ret = gen_last_ret;

	long before = f.fails;

	if (p_calls != r_calls) {
		char msg[128];
		std::snprintf(msg, sizeof(msg),
		    "uuidgen call count: port=%ld ref=%ld", p_calls, r_calls);
		fail(f, in, u_null, with_status, seed, msg);
	}
	if (p_off != r_off) {
		char msg[128];
		std::snprintf(msg, sizeof(msg),
		    "uuidgen store offset: port=%ld ref=%ld", p_off, r_off);
		fail(f, in, u_null, with_status, seed, msg);
	}
	if (p_count != r_count) {
		char msg[128];
		std::snprintf(msg, sizeof(msg),
		    "uuidgen count arg: port=%d ref=%d", p_count, r_count);
		fail(f, in, u_null, with_status, seed, msg);
	}
	if (p_ret != r_ret) {
		char msg[128];
		std::snprintf(msg, sizeof(msg),
		    "uuidgen return: port=%d ref=%d", p_ret, r_ret);
		fail(f, in, u_null, with_status, seed, msg);
	}
	if (f.fails == before)
		diff_arena(f, in, u_null, with_status, seed, "uuid arena",
		    A, B, ASZ);
	if (f.fails == before)
		diff_arena(f, in, u_null, with_status, seed, "status arena",
		    SA, SB, SSZ);
}

static void
check_create_nil(const unsigned char in[16], bool with_status)
{
	Fn &f = fn_create_nil;

	f.cases++;
	arenas_init(in, false);

	gen_reset(0);
	port::uuid_create_nil(reinterpret_cast<port::uuid_t *>(A + UOFF),
	    with_status ? reinterpret_cast<std::uint32_t *>(SA + SOFF)
	    : nullptr);
	long p_calls = gen_calls;

	gen_reset(0);
	ref_uuid_create_nil(reinterpret_cast<struct ref_uuid_s *>(B + UOFF),
	    with_status ? reinterpret_cast<std::uint32_t *>(SB + SOFF)
	    : nullptr);
	long r_calls = gen_calls;

	long before = f.fails;

	if (p_calls != r_calls) {
		char msg[128];
		std::snprintf(msg, sizeof(msg),
		    "uuidgen call count: port=%ld ref=%ld", p_calls, r_calls);
		fail(f, in, false, with_status, 0, msg);
	}
	if (f.fails == before)
		diff_arena(f, in, false, with_status, 0, "uuid arena",
		    A, B, ASZ);
	if (f.fails == before)
		diff_arena(f, in, false, with_status, 0, "status arena",
		    SA, SB, SSZ);
}

static void
check_hash(const unsigned char in[16], bool u_null, bool with_status)
{
	Fn &f = fn_hash;

	f.cases++;
	arenas_init(in, u_null);

	std::uint16_t p_ret = port::uuid_hash(u_null ? nullptr
	    : reinterpret_cast<const port::uuid_t *>(A + UOFF),
	    with_status ? reinterpret_cast<std::uint32_t *>(SA + SOFF)
	    : nullptr);

	std::uint16_t r_ret = ref_uuid_hash(u_null ? nullptr
	    : reinterpret_cast<const struct ref_uuid_s *>(B + UOFF),
	    with_status ? reinterpret_cast<std::uint32_t *>(SB + SOFF)
	    : nullptr);

	long before = f.fails;

	if (p_ret != r_ret) {
		char msg[128];
		std::snprintf(msg, sizeof(msg),
		    "return: port=0x%04x ref=0x%04x",
		    static_cast<unsigned>(p_ret), static_cast<unsigned>(r_ret));
		fail(f, in, u_null, with_status, 0, msg);
	}
	if (f.fails == before)
		diff_arena(f, in, u_null, with_status, 0, "uuid arena",
		    A, B, ASZ);
	if (f.fails == before)
		diff_arena(f, in, u_null, with_status, 0, "status arena",
		    SA, SB, SSZ);
}

static void
run_all(const unsigned char in[16], bool u_null, bool with_status,
    std::uint64_t seed)
{

	check_create(in, u_null, with_status, seed);
	/*
	 * uuid_create_nil() unconditionally bzero()s through u, so a null u
	 * is not a case the original code survives; it is not exercised.
	 */
	if (!u_null)
		check_create_nil(in, with_status);
	check_hash(in, u_null, with_status);
}

/* ------------------------------------------------------------------ */
/* Input patterns                                                       */
/* ------------------------------------------------------------------ */

#define MAX_PATTERNS	256

static unsigned char patterns[MAX_PATTERNS][16];
static int npatterns;

static void
add_pattern(const unsigned char p[16])
{

	if (npatterns >= MAX_PATTERNS)
		return;
	std::memcpy(patterns[npatterns], p, 16);
	npatterns++;
}

static void
add_fill(unsigned char v)
{
	unsigned char p[16];

	std::memset(p, v, 16);
	add_pattern(p);
}

static void
add_time_low(std::uint32_t tl, unsigned char rest)
{
	unsigned char p[16];

	std::memset(p, rest, 16);
	/* little-endian host: time_low occupies bytes 0..3 */
	p[0] = static_cast<unsigned char>(tl & 0xff);
	p[1] = static_cast<unsigned char>((tl >> 8) & 0xff);
	p[2] = static_cast<unsigned char>((tl >> 16) & 0xff);
	p[3] = static_cast<unsigned char>((tl >> 24) & 0xff);
	add_pattern(p);
}

static void
build_patterns(void)
{
	unsigned char p[16];
	int i;

	/* uniform fills, including all-NUL and all high-bit */
	add_fill(0x00);
	add_fill(0xff);
	add_fill(0x7f);
	add_fill(0x80);
	add_fill(0x01);
	add_fill(0xaa);
	add_fill(0x55);

	/* one hot byte in an otherwise NUL uuid, at every position */
	for (i = 0; i < 16; i++) {
		std::memset(p, 0x00, 16);
		p[i] = 0xff;
		add_pattern(p);
	}
	for (i = 0; i < 16; i++) {
		std::memset(p, 0x00, 16);
		p[i] = 0x80;
		add_pattern(p);
	}
	/* one cold byte in an otherwise saturated uuid, at every position */
	for (i = 0; i < 16; i++) {
		std::memset(p, 0xff, 16);
		p[i] = 0x00;
		add_pattern(p);
	}
	for (i = 0; i < 16; i++) {
		std::memset(p, 0xff, 16);
		p[i] = 0x01;
		add_pattern(p);
	}

	/* runs of high-bit bytes 0x80..0xff */
	for (i = 0; i < 16; i++)
		p[i] = static_cast<unsigned char>(0x80 + i);
	add_pattern(p);
	for (i = 0; i < 16; i++)
		p[i] = static_cast<unsigned char>(0xf0 + i);
	add_pattern(p);
	for (i = 0; i < 16; i++)
		p[i] = static_cast<unsigned char>(i);
	add_pattern(p);

	/*
	 * time_low values sitting on both sides of every boundary the
	 * 0xffff mask in uuid_hash() can be sensitive to.
	 */
	static const std::uint32_t tls[] = {
		0x00000000u, 0x00000001u, 0x00000002u, 0x000000ffu,
		0x00000100u, 0x00007fffu, 0x00008000u, 0x00008001u,
		0x0000fffeu, 0x0000ffffu, 0x00010000u, 0x00010001u,
		0x0000ff00u, 0x00ff0000u, 0x7fffffffu, 0x80000000u,
		0x80000001u, 0x80008000u, 0xffff0000u, 0xffff0001u,
		0xffff7fffu, 0xffff8000u, 0xfffffffeu, 0xffffffffu,
		0xdeadbeefu, 0x12345678u, 0xcafebabeu, 0xa5a5a5a5u,
	};
	for (std::size_t k = 0; k < sizeof(tls) / sizeof(tls[0]); k++) {
		add_time_low(tls[k], 0x00);
		add_time_low(tls[k], 0xff);
	}
}

/* ------------------------------------------------------------------ */
/* Randomised sweep                                                     */
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state;

static std::uint64_t
rng_next(void)
{
	std::uint64_t z;

	rng_state += 0x9e3779b97f4a7c15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

#define NRAND	200000

/* ------------------------------------------------------------------ */

int
main(void)
{
	unsigned char in[16];
	int i, k;

	build_patterns();

	static const std::uint64_t seeds[3] = {
		0ULL, 1ULL, 0x9e3779b97f4a7c15ULL,
	};

	/* hand-written edge cases */
	for (i = 0; i < npatterns; i++) {
		for (k = 0; k < 3; k++) {
			run_all(patterns[i], false, false, seeds[k]);
			run_all(patterns[i], false, true, seeds[k]);
			run_all(patterns[i], true, false, seeds[k]);
			run_all(patterns[i], true, true, seeds[k]);
		}
	}

	/* fixed-seed randomised sweep */
	rng_state = 0x0123456789abcdefULL;
	for (long it = 0; it < NRAND; it++) {
		std::uint64_t a = rng_next();
		std::uint64_t b = rng_next();
		std::uint64_t c = rng_next();

		std::memcpy(in, &a, 8);
		std::memcpy(in + 8, &b, 8);

		/*
		 * Bias a slice of the sweep towards uuids whose time_low is
		 * near a 16-bit boundary, so the mask keeps being exercised
		 * on both sides of it and not just on random noise.
		 */
		if ((c & 3) == 0) {
			std::uint32_t tl =
			    static_cast<std::uint32_t>(c >> 32) & 0xffffu;
			tl += ((c >> 8) & 1) ? 0x00010000u : 0u;
			tl -= ((c >> 9) & 1) ? 1u : 0u;
			in[0] = static_cast<unsigned char>(tl & 0xff);
			in[1] = static_cast<unsigned char>((tl >> 8) & 0xff);
			in[2] = static_cast<unsigned char>((tl >> 16) & 0xff);
			in[3] = static_cast<unsigned char>((tl >> 24) & 0xff);
		}
		if ((c & 7) == 1) {
			/* NUL-heavy input */
			std::memset(in, 0, 16);
			in[c % 16] = static_cast<unsigned char>(0x80 |
			    ((c >> 16) & 0x7f));
		}
		if ((c & 7) == 2) {
			/* every byte forced into the 0x80..0xff range */
			for (int j = 0; j < 16; j++)
				in[j] |= 0x80;
		}

		bool with_status = ((c >> 4) & 1) != 0;
		bool u_null = ((c >> 5) & 0x3f) == 0;
		std::uint64_t seed = rng_next();

		run_all(in, u_null, with_status, seed);
	}

	long total_fails = fn_create.fails + fn_create_nil.fails +
	    fn_hash.fails;

	std::printf("\n");
	std::printf("%-24s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-24s %12s %12s\n", "------------------------",
	    "------------", "------------");
	std::printf("%-24s %12ld %12ld\n", fn_create.name, fn_create.cases,
	    fn_create.fails);
	std::printf("%-24s %12ld %12ld\n", fn_create_nil.name,
	    fn_create_nil.cases, fn_create_nil.fails);
	std::printf("%-24s %12ld %12ld\n", fn_hash.name, fn_hash.cases,
	    fn_hash.fails);
	std::printf("%-24s %12s %12s\n", "------------------------",
	    "------------", "------------");
	std::printf("%-24s %12ld %12ld\n", "TOTAL",
	    fn_create.cases + fn_create_nil.cases + fn_hash.cases,
	    total_fails);
	std::printf("\n%s\n", total_fails == 0 ? "PASS" : "FAIL");

	return (total_fails == 0 ? 0 : 1);
}
