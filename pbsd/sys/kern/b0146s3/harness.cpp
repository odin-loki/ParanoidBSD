// Differential test for PBSD batch b0146s3 (subr_hash.c).

import pbsd.sys.kern.b0146s3;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace port = pbsd::sys_kern::b0146s3;

#define SWEEP     200000L
#define MAX_PRINT 12

#define HASH_WAITOK 0x00000001
#define HASH_NOWAIT 0x00000002

struct stat_row {
	const char *name;
	long cases;
	long failures;
	long printed;
};

static stat_row rows[] = {
	{ "hashinit_flags",  0, 0, 0 },
	{ "hashinit",        0, 0, 0 },
	{ "hashdestroy",     0, 0, 0 },
	{ "phashinit_flags", 0, 0, 0 },
	{ "phashinit",       0, 0, 0 },
};

enum {
	R_HASH_FLAGS = 0,
	R_HASH,
	R_HASH_DESTROY,
	R_PHASH_FLAGS,
	R_PHASH,
};

static uint64_t rng_state = 0x00b0146b3faceULL;

static inline uint64_t
rnd64(void)
{
	uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static inline uint32_t
rnd32(void)
{
	return ((uint32_t)(rnd64() >> 32));
}

extern "C" {
struct malloc_type {
	const char *ks_shortdesc;
};
void *ref_hashinit_flags(int, malloc_type *, unsigned long *, int);
void *ref_hashinit(int, malloc_type *, unsigned long *);
void ref_hashdestroy(void *, malloc_type *, unsigned long);
void *ref_phashinit_flags(int, malloc_type *, unsigned long *, int);
void *ref_phashinit(int, malloc_type *, unsigned long *);
void oracle_malloc_reset(void);
void oracle_malloc_fail_at(int);
int oracle_malloc_calls_count(void);
std::size_t oracle_malloc_last_size(void);
int oracle_malloc_last_flags(void);
}

static port::malloc_type g_mtype = { "test" };

static malloc_type *
c_mtype(void)
{
	return (reinterpret_cast<malloc_type *>(&g_mtype));
}

static void
fail_row(int row, const char *label, const char *detail)
{
	stat_row &r = rows[row];

	r.failures++;
	if (r.printed < MAX_PRINT) {
		r.printed++;
		std::printf("  FAIL %-24s %-24s %s\n", r.name, label, detail);
	}
}

static void
case_row(int row)
{
	rows[row].cases++;
}

struct generic_head {
	void *lh_first;
};

static bool
check_bucket_inits(void *tbl, unsigned long mask)
{
	auto *hp = static_cast<generic_head *>(tbl);

	for (unsigned long i = 0; i <= mask; i++) {
		if (hp[i].lh_first != nullptr)
			return (false);
	}
	return (true);
}

struct run_obs {
	void *tbl;
	unsigned long mask_or_n;
	int alloc_calls;
	std::size_t alloc_size;
	int alloc_flags;
	bool alloc_null;
};

static run_obs
run_port_hashinit_flags(int elements, int flags, int fail_at)
{
	run_obs obs = {};

	port::malloc_reset();
	port::malloc_fail_at(fail_at);

	obs.mask_or_n = 0xdeadbeefUL;
	obs.tbl = port::hashinit_flags(elements, &g_mtype, &obs.mask_or_n, flags);

	obs.alloc_calls = port::malloc_calls();
	obs.alloc_size = port::malloc_last_size();
	obs.alloc_flags = port::malloc_last_flags();
	obs.alloc_null = (obs.tbl == nullptr);
	return (obs);
}

static run_obs
run_ref_hashinit_flags(int elements, int flags, int fail_at)
{
	run_obs obs = {};

	oracle_malloc_reset();
	oracle_malloc_fail_at(fail_at);

	obs.mask_or_n = 0xdeadbeefUL;
	obs.tbl = ref_hashinit_flags(elements, c_mtype(), &obs.mask_or_n, flags);

	obs.alloc_calls = oracle_malloc_calls_count();
	obs.alloc_size = oracle_malloc_last_size();
	obs.alloc_flags = oracle_malloc_last_flags();
	obs.alloc_null = (obs.tbl == nullptr);
	return (obs);
}

static void
compare_obs(int row, const run_obs &p, const run_obs &r, bool prime)
{
	if (p.alloc_null != r.alloc_null) {
		fail_row(row, "alloc", "null mismatch");
		return;
	}
	if (p.alloc_calls != r.alloc_calls)
		fail_row(row, "malloc-calls", "count mismatch");
	if (p.alloc_size != r.alloc_size)
		fail_row(row, "malloc-size", "size mismatch");
	if (p.alloc_flags != r.alloc_flags)
		fail_row(row, "malloc-flags", "flags mismatch");
	if (p.tbl == nullptr)
		return;
	if (p.mask_or_n != r.mask_or_n) {
		fail_row(row, prime ? "nentries" : "hashmask", "value mismatch");
		return;
	}
	unsigned long mask = prime ? (p.mask_or_n - 1) : p.mask_or_n;
	if (!check_bucket_inits(p.tbl, mask))
		fail_row(row, "port-buckets", "not empty");
	if (!check_bucket_inits(r.tbl, mask))
		fail_row(row, "ref-buckets", "not empty");
}

static void
compare_hashinit_flags(int elements, int flags, int fail_at)
{
	case_row(R_HASH_FLAGS);

	run_obs p = run_port_hashinit_flags(elements, flags, fail_at);
	run_obs r = run_ref_hashinit_flags(elements, flags, fail_at);

	compare_obs(R_HASH_FLAGS, p, r, false);

	if (p.tbl != nullptr)
		port::hashdestroy(p.tbl, &g_mtype, p.mask_or_n);
	if (r.tbl != nullptr)
		ref_hashdestroy(r.tbl, c_mtype(), r.mask_or_n);
}

static run_obs
run_port_phashinit_flags(int elements, int flags, int fail_at)
{
	run_obs obs = {};

	port::malloc_reset();
	port::malloc_fail_at(fail_at);

	obs.mask_or_n = 0xdeadbeefUL;
	obs.tbl = port::phashinit_flags(elements, &g_mtype, &obs.mask_or_n, flags);

	obs.alloc_calls = port::malloc_calls();
	obs.alloc_size = port::malloc_last_size();
	obs.alloc_flags = port::malloc_last_flags();
	obs.alloc_null = (obs.tbl == nullptr);
	return (obs);
}

static run_obs
run_ref_phashinit_flags(int elements, int flags, int fail_at)
{
	run_obs obs = {};

	oracle_malloc_reset();
	oracle_malloc_fail_at(fail_at);

	obs.mask_or_n = 0xdeadbeefUL;
	obs.tbl = ref_phashinit_flags(elements, c_mtype(), &obs.mask_or_n, flags);

	obs.alloc_calls = oracle_malloc_calls_count();
	obs.alloc_size = oracle_malloc_last_size();
	obs.alloc_flags = oracle_malloc_last_flags();
	obs.alloc_null = (obs.tbl == nullptr);
	return (obs);
}

static void
compare_phashinit_flags(int elements, int flags, int fail_at)
{
	case_row(R_PHASH_FLAGS);

	run_obs p = run_port_phashinit_flags(elements, flags, fail_at);
	run_obs r = run_ref_phashinit_flags(elements, flags, fail_at);

	compare_obs(R_PHASH_FLAGS, p, r, true);

	if (p.tbl != nullptr)
		port::hashdestroy(p.tbl, &g_mtype, p.mask_or_n - 1);
	if (r.tbl != nullptr)
		ref_hashdestroy(r.tbl, c_mtype(), r.mask_or_n - 1);
}

static void
compare_hashinit(int elements)
{
	case_row(R_HASH);

	unsigned long pm = 0, rm = 0;

	port::malloc_reset();
	port::malloc_fail_at(0);
	void *pt = port::hashinit(elements, &g_mtype, &pm);
	run_obs p = {};
	p.tbl = pt;
	p.mask_or_n = pm;
	p.alloc_calls = port::malloc_calls();
	p.alloc_size = port::malloc_last_size();
	p.alloc_flags = port::malloc_last_flags();
	p.alloc_null = (pt == nullptr);

	oracle_malloc_reset();
	oracle_malloc_fail_at(0);
	void *rt = ref_hashinit(elements, c_mtype(), &rm);
	run_obs r = {};
	r.tbl = rt;
	r.mask_or_n = rm;
	r.alloc_calls = oracle_malloc_calls_count();
	r.alloc_size = oracle_malloc_last_size();
	r.alloc_flags = oracle_malloc_last_flags();
	r.alloc_null = (rt == nullptr);

	compare_obs(R_HASH, p, r, false);

	if (pt != nullptr)
		port::hashdestroy(pt, &g_mtype, pm);
	if (rt != nullptr)
		ref_hashdestroy(rt, c_mtype(), rm);
}

static void
compare_phashinit(int elements)
{
	case_row(R_PHASH);

	unsigned long pn = 0, rn = 0;

	port::malloc_reset();
	port::malloc_fail_at(0);
	void *pt = port::phashinit(elements, &g_mtype, &pn);
	run_obs p = {};
	p.tbl = pt;
	p.mask_or_n = pn;
	p.alloc_calls = port::malloc_calls();
	p.alloc_size = port::malloc_last_size();
	p.alloc_flags = port::malloc_last_flags();
	p.alloc_null = (pt == nullptr);

	oracle_malloc_reset();
	oracle_malloc_fail_at(0);
	void *rt = ref_phashinit(elements, c_mtype(), &rn);
	run_obs r = {};
	r.tbl = rt;
	r.mask_or_n = rn;
	r.alloc_calls = oracle_malloc_calls_count();
	r.alloc_size = oracle_malloc_last_size();
	r.alloc_flags = oracle_malloc_last_flags();
	r.alloc_null = (rt == nullptr);

	compare_obs(R_PHASH, p, r, true);

	if (pt != nullptr)
		port::hashdestroy(pt, &g_mtype, pn - 1);
	if (rt != nullptr)
		ref_hashdestroy(rt, c_mtype(), rn - 1);
}

static void
compare_hashdestroy(void)
{
	case_row(R_HASH_DESTROY);

	unsigned long pm = 0, rm = 0;
	int pcalls_before, pcalls_after, rcalls_before, rcalls_after;

	port::malloc_reset();
	void *pt = port::hashinit(8, &g_mtype, &pm);
	pcalls_before = port::malloc_calls();
	port::hashdestroy(pt, &g_mtype, pm);
	pcalls_after = port::malloc_calls();

	oracle_malloc_reset();
	void *rt = ref_hashinit(8, c_mtype(), &rm);
	rcalls_before = oracle_malloc_calls_count();
	ref_hashdestroy(rt, c_mtype(), rm);
	rcalls_after = oracle_malloc_calls_count();

	if ((pcalls_after - pcalls_before) != (rcalls_after - rcalls_before))
		fail_row(R_HASH_DESTROY, "malloc-calls", "destroy side mismatch");
}

static void
test_hand(void)
{
	const int flags[] = { HASH_WAITOK, HASH_NOWAIT };
	const int hash_elems[] = { 1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64,
	    127, 128, 255, 256, 1023, 1024, 4095, 4096, 8191, 16384, 32768,
	    65536 };
	const int prime_elems[] = { 1, 2, 12, 13, 14, 30, 31, 32, 60, 61, 62,
	    126, 127, 128, 250, 251, 508, 509, 760, 761, 1020, 1021, 1530,
	    1531, 2038, 2039, 2556, 2557, 3066, 3067, 3582, 3583, 4092, 4093,
	    4602, 4603, 5118, 5119, 5622, 5623, 6142, 6143, 6652, 6653, 7158,
	    7159, 7672, 7673, 8190, 8191, 12280, 12281, 16380, 16381, 24570,
	    24571, 32748, 32749, 32750, 50000, 100000 };

	for (int e : hash_elems) {
		for (int f : flags)
			compare_hashinit_flags(e, f, 0);
	}
	compare_hashinit_flags(16, HASH_WAITOK, 1);
	compare_hashinit_flags(1, HASH_NOWAIT, 1);

	for (int e : prime_elems) {
		for (int f : flags)
			compare_phashinit_flags(e, f, 0);
	}
	compare_phashinit_flags(31, HASH_WAITOK, 1);
	compare_phashinit_flags(1, HASH_NOWAIT, 1);

	for (int e : hash_elems)
		compare_hashinit(e);
	for (int e : prime_elems)
		compare_phashinit(e);

	compare_hashdestroy();
}

static void
test_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		int elements = (int)(rnd32() % 50000) + 1;
		int flags = (rnd32() & 1) ? HASH_WAITOK : HASH_NOWAIT;
		int fail_at = ((rnd32() % 25) == 0) ? 1 : 0;

		compare_hashinit_flags(elements, flags, fail_at);
		compare_phashinit_flags(elements, flags, fail_at);

		if ((rnd32() % 50) == 0)
			compare_hashinit(elements);
		if ((rnd32() % 50) == 0)
			compare_phashinit(elements);
		if ((rnd32() % 10000) == 0)
			compare_hashdestroy();
	}
}

int
main(void)
{
	test_hand();
	test_sweep();

	long total_cases = 0;
	long total_fail = 0;

	std::printf("\n%-24s %12s %12s\n", "function", "cases", "failures");
	for (const auto &r : rows) {
		std::printf("%-24s %12ld %12ld\n", r.name, r.cases, r.failures);
		total_cases += r.cases;
		total_fail += r.failures;
	}
	std::printf("%-24s %12ld %12ld\n", "TOTAL", total_cases, total_fail);

	return (total_fail == 0 ? 0 : 1);
}
