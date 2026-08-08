// Differential test harness for PBSD batch b0224 (uuidgen.c).

import pbsd.bin.uuidgen.b0224;

#include <cerrno>
#include <cinttypes>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace P = pbsd::bin_uuidgen::b0224;

static const unsigned char GUARD = 0x7f;
static const long SWEEP = 200000L;

extern "C" {
void ref_uuid_to_compact_string(const ::uuid_t *u, char **s, std::uint32_t *status);
int ref_uuidgen_v4(::uuid *store, int count);
void b0224_arc4random_reset(std::uint64_t seed);
}

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
};

static Stat stats[] = {
	{ "uuid_to_compact_string", 0, 0 },
	{ "uuidgen_v4", 0, 0 },
};
static const int NSTAT = (int)(sizeof(stats) / sizeof(stats[0]));

static Stat &
S(const char *n)
{
	for (int i = 0; i < NSTAT; i++)
		if (std::strcmp(stats[i].name, n) == 0)
			return stats[i];
	std::fprintf(stderr, "bad stat %s\n", n);
	std::exit(2);
}

static void
fail(Stat &st, const char *msg)
{
	st.fails++;
	if (st.fails <= 8)
		std::printf("  FAIL %s: %s\n", st.name, msg);
}

static void
ok(Stat &st)
{
	st.cases++;
}

struct Rng {
	std::uint64_t s;

	explicit Rng(std::uint64_t seed) : s(seed) {}

	std::uint64_t next()
	{
		s += 0x9E3779B97F4A7C15ull;
		std::uint64_t z = s;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}

	std::uint32_t u32() { return (std::uint32_t)next(); }
	std::uint8_t u8() { return (std::uint8_t)next(); }
	int i32() { return (int)u32(); }
} rng(0x00b0224a5ULL);

struct StatusWrap {
	unsigned char pre;
	std::uint32_t st;
	unsigned char post;
};

static void
init_status(StatusWrap &w)
{
	w.pre = GUARD;
	w.st = 0x55555555u;
	w.post = GUARD;
}

static bool
status_guards_ok(const StatusWrap &w)
{
	return w.pre == GUARD && w.post == GUARD;
}

static void
fill_uuid(OracleUuid &u, Rng &r, bool hi_bytes)
{
	u.time_low = r.u32();
	u.time_mid = (std::uint16_t)r.u32();
	u.time_hi_and_version = (std::uint16_t)r.u32();
	if (hi_bytes) {
		u.clock_seq_hi_and_reserved = (std::uint8_t)(0x80u | (r.u8() & 0x7fu));
		u.clock_seq_low = r.u8();
		for (int i = 0; i < 6; i++)
			u.node[i] = (std::uint8_t)(0x80u | (r.u8() & 0x7fu));
	} else {
		u.clock_seq_hi_and_reserved = r.u8();
		u.clock_seq_low = r.u8();
		for (int i = 0; i < 6; i++)
			u.node[i] = r.u8();
	}
}

static void
cmp_compact_one(const OracleUuid *u, char **s_r, char **s_p, StatusWrap *sw_r,
    StatusWrap *sw_p, const char *tag)
{
	Stat &st = S("uuid_to_compact_string");
	std::uint32_t *status_r = sw_r ? &sw_r->st : nullptr;
	std::uint32_t *status_p = sw_p ? &sw_p->st : nullptr;

	ref_uuid_to_compact_string(u, s_r, status_r);
	P::uuid_to_compact_string(reinterpret_cast<const P::uuid *>(u), s_p,
	    status_p);

	if (sw_r != nullptr && !status_guards_ok(*sw_r)) {
		fail(st, std::string(tag + ": ref status guard").c_str());
	} else if (sw_p != nullptr && !status_guards_ok(*sw_p)) {
		fail(st, std::string(tag + ": port status guard").c_str());
	} else if (status_r != nullptr && status_p != nullptr &&
	    *status_r != *status_p) {
		fail(st, std::string(tag + ": status mismatch").c_str());
	} else if ((s_r == nullptr) != (s_p == nullptr)) {
		fail(st, std::string(tag + ": s null mismatch").c_str());
	} else if (s_r != nullptr) {
		if (*s_r == nullptr && *s_p == nullptr) {
			/* match */
		} else if (*s_r == nullptr || *s_p == nullptr) {
			fail(st, std::string(tag + ": *s null mismatch").c_str());
		} else if (std::strcmp(*s_r, *s_p) != 0) {
			fail(st, std::string(tag + ": string mismatch").c_str());
		}
	}
	ok(st);
}

static void
test_uuid_to_compact_string_hand(void)
{
	char *sr = nullptr;
	char *sp = nullptr;
	StatusWrap wr, wp;
	OracleUuid u;

	init_status(wr);
	init_status(wp);
	cmp_compact_one(nullptr, nullptr, nullptr, &wr, &wp, "nil_u_status");

	init_status(wr);
	cmp_compact_one(nullptr, nullptr, nullptr, &wr, nullptr, "nil_u_status_ref_only");

	cmp_compact_one(nullptr, nullptr, nullptr, nullptr, nullptr, "both_null");

	init_status(wr);
	init_status(wp);
	cmp_compact_one(nullptr, &sr, &sp, &wr, &wp, "nil_u_out");
	std::free(sr);
	std::free(sp);

	std::memset(&u, 0, sizeof(u));
	init_status(wr);
	init_status(wp);
	cmp_compact_one(&u, &sr, &sp, &wr, &wp, "zero_u");
	std::free(sr);
	std::free(sp);

	std::memset(&u, 0xff, sizeof(u));
	init_status(wr);
	init_status(wp);
	cmp_compact_one(&u, &sr, &sp, &wr, &wp, "ff_u");
	std::free(sr);
	std::free(sp);

	u.time_low = 0;
	u.time_mid = 0;
	u.time_hi_and_version = 0;
	u.clock_seq_hi_and_reserved = 0x80;
	u.clock_seq_low = 0xff;
	for (int i = 0; i < 6; i++)
		u.node[i] = (std::uint8_t)(0x80 + i);
	init_status(wr);
	init_status(wp);
	cmp_compact_one(&u, &sr, &sp, &wr, &wp, "hibyte_u");
	std::free(sr);
	std::free(sp);

	u.time_low = 1u;
	u.time_mid = 1u;
	u.time_hi_and_version = 1u;
	u.clock_seq_hi_and_reserved = 1u;
	u.clock_seq_low = 1u;
	for (int i = 0; i < 6; i++)
		u.node[i] = 1u;
	init_status(wr);
	init_status(wp);
	cmp_compact_one(&u, &sr, &sp, &wr, &wp, "one_u");
	std::free(sr);
	std::free(sp);

	u.time_low = 0xffffffffu;
	u.time_mid = 0xffffu;
	u.time_hi_and_version = 0xffffu;
	u.clock_seq_hi_and_reserved = 0xffu;
	u.clock_seq_low = 0xffu;
	for (int i = 0; i < 6; i++)
		u.node[i] = 0xffu;
	init_status(wr);
	init_status(wp);
	cmp_compact_one(&u, &sr, &sp, &wr, &wp, "max_u");
	std::free(sr);
	std::free(sp);

	init_status(wr);
	init_status(wp);
	cmp_compact_one(&u, nullptr, nullptr, &wr, &wp, "no_out_ptr");
}

static void
test_uuid_to_compact_string_sweep(void)
{
	Stat &st = S("uuid_to_compact_string");
	OracleUuid u;

	for (long i = 0; i < SWEEP; i++) {
		char *sr = nullptr;
		char *sp = nullptr;
		StatusWrap wr, wp;
		const ::uuid *up = nullptr;
		bool hi = (rng.u32() & 1u) != 0u;
		bool null_u = (rng.u32() & 3u) == 0u;
		bool null_s = (rng.u32() & 7u) == 0u;
		bool null_st = (rng.u32() & 7u) == 0u;

		if (!null_u) {
			fill_uuid(u, rng, hi);
			up = &u;
		}

		if (!null_st) {
			init_status(wr);
			init_status(wp);
		}

		ref_uuid_to_compact_string(up, null_s ? nullptr : &sr,
		    null_st ? nullptr : &wr.st);
		P::uuid_to_compact_string(up, null_s ? nullptr : &sp,
		    null_st ? nullptr : &wp.st);

		bool bad = false;
		if (!null_st) {
			if (!status_guards_ok(wr) || !status_guards_ok(wp))
				bad = true;
			else if (wr.st != wp.st)
				bad = true;
		}
		if (!null_s) {
			if ((sr == nullptr) != (sp == nullptr))
				bad = true;
			else if (sr != nullptr && sp != nullptr) {
				if ((*sr == nullptr) != (*sp == nullptr))
					bad = true;
				else if (*sr != nullptr && *sp != nullptr &&
				    std::strcmp(sr, sp) != 0)
					bad = true;
			}
		}
		if (bad)
			fail(st, "sweep");
		ok(st);
		std::free(sr);
		std::free(sp);
	}
}

static unsigned char *
alloc_uuid_store(size_t count, size_t &total)
{
	const size_t body = count * sizeof(::uuid);
	total = body + 2;
	unsigned char *raw = (unsigned char *)std::malloc(total);
	if (raw == nullptr) {
		std::fprintf(stderr, "malloc failed\n");
		std::exit(2);
	}
	std::memset(raw, GUARD, total);
	return raw;
}

static void
cmp_uuidgen_one(int count, std::uint64_t seed, const char *tag)
{
	Stat &st = S("uuidgen_v4");
	size_t total_r = 0, total_p = 0;
	unsigned char *raw_r = nullptr;
	unsigned char *raw_p = nullptr;
	::uuid *store_r = nullptr;
	::uuid *store_p = nullptr;
	int ret_r, ret_p;
	int errno_r, errno_p;

	if (count >= 1) {
		raw_r = alloc_uuid_store((size_t)count, total_r);
		raw_p = alloc_uuid_store((size_t)count, total_p);
		store_r = (::uuid *)(raw_r + 1);
		store_p = (::uuid *)(raw_p + 1);
	}

	errno = 0;
	b0224_arc4random_reset(seed);
	ret_r = ref_uuidgen_v4(store_r, count);
	errno_r = errno;

	errno = 0;
	b0224_arc4random_reset(seed);
	ret_p = P::uuidgen_v4(store_p, count);
	errno_p = errno;

	if (ret_r != ret_p)
		fail(st, std::string(tag + ": return").c_str());
	else if (ret_r < 0 && (errno_r != errno_p || errno_r != EINVAL))
		fail(st, std::string(tag + ": errno").c_str());
	else if (ret_r == 0) {
		if (std::memcmp(raw_r, raw_p, total_r) != 0)
			fail(st, std::string(tag + ": buffer").c_str());
	}
	ok(st);

	std::free(raw_r);
	std::free(raw_p);
}

static void
test_uuidgen_v4_hand(void)
{
	cmp_uuidgen_one(0, 1, "count0");
	cmp_uuidgen_one(-1, 2, "count_neg1");
	cmp_uuidgen_one(INT_MIN, 3, "count_min");
	cmp_uuidgen_one(1, 0x12345678ULL, "count1");
	cmp_uuidgen_one(2, 0xabcdef01ULL, "count2");
	cmp_uuidgen_one(16, 0xdeadbeefULL, "count16");
	cmp_uuidgen_one(1, 0xffffffffULL, "seed_ff");
	cmp_uuidgen_one(4, 0x1ULL, "seed_1");
}

static void
test_uuidgen_v4_sweep(void)
{
	Stat &st = S("uuidgen_v4");

	for (long i = 0; i < SWEEP; i++) {
		int count;
		std::uint32_t r = rng.u32();
		if ((r & 15u) == 0u)
			count = 0;
		else if ((r & 15u) == 1u)
			count = -(int)(rng.u32() & 0x7fffu);
		else
			count = 1 + (int)(rng.u32() % 32u);

		std::uint64_t seed = rng.next();
		size_t total_r = 0, total_p = 0;
		unsigned char *raw_r = nullptr;
		unsigned char *raw_p = nullptr;
		::uuid *store_r = nullptr;
		::uuid *store_p = nullptr;
		int ret_r, ret_p;
		int errno_r, errno_p;

		if (count >= 1) {
			raw_r = alloc_uuid_store((size_t)count, total_r);
			raw_p = alloc_uuid_store((size_t)count, total_p);
			store_r = (::uuid *)(raw_r + 1);
			store_p = (::uuid *)(raw_p + 1);
		}

		errno = 0;
		b0224_arc4random_reset(seed);
		ret_r = ref_uuidgen_v4(store_r, count);
		errno_r = errno;

		errno = 0;
		b0224_arc4random_reset(seed);
		ret_p = P::uuidgen_v4(store_p, count);
		errno_p = errno;

		bool bad = false;
		if (ret_r != ret_p)
			bad = true;
		else if (ret_r < 0) {
			if (errno_r != errno_p || errno_r != EINVAL)
				bad = true;
		} else if (std::memcmp(raw_r, raw_p, total_r) != 0)
			bad = true;

		if (bad)
			fail(st, "sweep");
		ok(st);

		std::free(raw_r);
		std::free(raw_p);
	}
}

int
main(void)
{
	test_uuid_to_compact_string_hand();
	test_uuid_to_compact_string_sweep();
	test_uuidgen_v4_hand();
	test_uuidgen_v4_sweep();

	std::printf("\n%-24s %12s %12s\n", "function", "cases", "failures");
	unsigned long long total_fail = 0;
	for (int i = 0; i < NSTAT; i++) {
		std::printf("%-24s %12llu %12llu\n", stats[i].name, stats[i].cases,
		    stats[i].fails);
		total_fail += stats[i].fails;
	}

	return total_fail == 0 ? 0 : 1;
}
