// Differential test for PBSD batch b0146s3 (subr_hash.c).

import pbsd.sys.kern.b0146s3;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace port = pbsd::sys_kern::b0146s3;

#define GUARD     0x7f
#define PAD       64u
#define SWEEP     200000L
#define MAX_PRINT 12
#define ARENA_CAP 262144u

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

/* ------------------------------------------------------------------ */
/* Shared kernel malloc environment                                   */
/* ------------------------------------------------------------------ */

struct kenv_state {
	int		alloc_calls;
	std::size_t	alloc_size;
	int		alloc_flags;
	int		alloc_failed;
	int		free_calls;
	unsigned char	arena[ARENA_CAP];
	unsigned char	arena_snap[ARENA_CAP];
};

static kenv_state g_env;
static int g_fail_at;

extern "C" void
pbsd_kern_env_reset(void)
{

	g_env.alloc_calls = 0;
	g_env.alloc_size = 0;
	g_env.alloc_flags = 0;
	g_env.alloc_failed = 0;
	g_env.free_calls = 0;
	std::memset(g_env.arena, GUARD, ARENA_CAP);
	g_fail_at = 0;
}

extern "C" void
pbsd_kern_fail_at(int n)
{

	g_fail_at = n;
}

extern "C" void *
pbsd_kern_malloc(std::size_t size, port::malloc_type *type, int flags)
{
	std::size_t need;
	void *ret;

	(void)type;

	g_env.alloc_calls++;
	g_env.alloc_size = size;
	g_env.alloc_flags = flags;
	need = (size + 15u) & ~15u;
	if (g_fail_at != 0 && g_env.alloc_calls >= g_fail_at) {
		g_env.alloc_failed = 1;
		return (nullptr);
	}
	if (need > ARENA_CAP) {
		g_env.alloc_failed = 1;
		return (nullptr);
	}
	ret = g_env.arena;
	std::memset(g_env.arena, GUARD, ARENA_CAP);
	g_env.alloc_failed = 0;
	return (ret);
}

extern "C" void
pbsd_kern_free(void *addr, port::malloc_type *type)
{

	(void)addr;
	(void)type;
	g_env.free_calls++;
}

static void
kenv_snapshot(unsigned char snap[ARENA_CAP])
{
	std::memcpy(snap, g_env.arena, ARENA_CAP);
}

static void
kenv_restore(const unsigned char snap[ARENA_CAP])
{
	std::memcpy(g_env.arena, snap, ARENA_CAP);
}

/* ------------------------------------------------------------------ */
/* Oracle declarations                                                */
/* ------------------------------------------------------------------ */

extern "C" {
struct malloc_type;
void *ref_hashinit_flags(int, struct malloc_type *, unsigned long *, int);
void *ref_hashinit(int, struct malloc_type *, unsigned long *);
void ref_hashdestroy(void *, struct malloc_type *, unsigned long);
void *ref_phashinit_flags(int, struct malloc_type *, unsigned long *, int);
void *ref_phashinit(int, struct malloc_type *, unsigned long *);
void oracle_malloc_reset(void);
void oracle_malloc_fail_at(int);
}

/* ------------------------------------------------------------------ */
/* Harness helpers                                                    */
/* ------------------------------------------------------------------ */

static struct malloc_type g_mtype = { "test" };

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

static bool
arena_unchanged_outside(const unsigned char before[ARENA_CAP],
    std::size_t alloc_size)
{
	std::size_t need = (alloc_size + 15u) & ~15u;

	if (need > ARENA_CAP)
		need = ARENA_CAP;
	for (std::size_t i = need; i < ARENA_CAP; i++) {
		if (g_env.arena[i] != before[i])
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
	int alloc_failed;
	int free_calls;
	unsigned char arena[ARENA_CAP];
};

static run_obs
run_port_hashinit_flags(int elements, int flags, int fail_at)
{
	run_obs obs = {};

	unsigned char pre[ARENA_CAP];
	port::malloc_reset();
	port::malloc_fail_at(fail_at);
	kenv_snapshot(pre);

	obs.mask_or_n = 0xdeadbeefUL;
	obs.tbl = port::hashinit_flags(elements, &g_mtype, &obs.mask_or_n, flags);

	obs.alloc_calls = g_env.alloc_calls;
	obs.alloc_size = g_env.alloc_size;
	obs.alloc_flags = g_env.alloc_flags;
	obs.alloc_failed = g_env.alloc_failed;
	obs.free_calls = g_env.free_calls;
	kenv_snapshot(obs.arena);

	if (obs.tbl != nullptr) {
		if (!arena_unchanged_outside(pre, obs.alloc_size))
			obs.alloc_failed = -1;
	}
	return (obs);
}

static run_obs
run_ref_hashinit_flags(int elements, int flags, int fail_at)
{
	run_obs obs = {};

	unsigned char pre[ARENA_CAP];
	oracle_malloc_reset();
	oracle_malloc_fail_at(fail_at);
	kenv_snapshot(pre);

	obs.mask_or_n = 0xdeadbeefUL;
	obs.tbl = ref_hashinit_flags(elements,
	    reinterpret_cast<malloc_type *>(&g_mtype), &obs.mask_or_n, flags);

	obs.alloc_calls = g_env.alloc_calls;
	obs.alloc_size = g_env.alloc_size;
	obs.alloc_flags = g_env.alloc_flags;
	obs.alloc_failed = g_env.alloc_failed;
	obs.free_calls = g_env.free_calls;
	kenv_snapshot(obs.arena);

	if (obs.tbl != nullptr) {
		if (!arena_unchanged_outside(pre, obs.alloc_size))
			obs.alloc_failed = -1;
	}
	return (obs);
}

static void
compare_hashinit_flags(int elements, int flags, int fail_at)
{
	case_row(R_HASH_FLAGS);

	run_obs p = run_port_hashinit_flags(elements, flags, fail_at);
	run_obs r = run_ref_hashinit_flags(elements, flags, fail_at);

	if ((p.tbl == nullptr) != (r.tbl == nullptr)) {
		fail_row(R_HASH_FLAGS, "alloc", "null mismatch");
		goto cleanup;
	}
	if (p.alloc_calls != r.alloc_calls)
		fail_row(R_HASH_FLAGS, "malloc-calls", "count mismatch");
	if (p.alloc_size != r.alloc_size)
		fail_row(R_HASH_FLAGS, "malloc-size", "size mismatch");
	if (p.alloc_flags != r.alloc_flags)
		fail_row(R_HASH_FLAGS, "malloc-flags", "flags mismatch");
	if (p.alloc_failed != r.alloc_failed)
		fail_row(R_HASH_FLAGS, "malloc-fail", "fail state mismatch");
	if (p.tbl != nullptr) {
		if (p.mask_or_n != r.mask_or_n)
			fail_row(R_HASH_FLAGS, "hashmask", "mask mismatch");
		if (!check_bucket_inits(p.tbl, p.mask_or_n))
			fail_row(R_HASH_FLAGS, "port-buckets", "not empty");
		if (!check_bucket_inits(r.tbl, r.mask_or_n))
			fail_row(R_HASH_FLAGS, "ref-buckets", "not empty");
		if (std::memcmp(p.arena, r.arena, ARENA_CAP) != 0)
			fail_row(R_HASH_FLAGS, "arena", "buffer mismatch");
	}

cleanup:
	if (p.tbl != nullptr)
		port::hashdestroy(p.tbl, &g_mtype, p.mask_or_n);
	if (r.tbl != nullptr)
		ref_hashdestroy(r.tbl, reinterpret_cast<malloc_type *>(&g_mtype),
		    r.mask_or_n);
}

static run_obs
run_port_phashinit_flags(int elements, int flags, int fail_at)
{
	run_obs obs = {};

	unsigned char pre[ARENA_CAP];
	port::malloc_reset();
	port::malloc_fail_at(fail_at);
	kenv_snapshot(pre);

	obs.mask_or_n = 0xdeadbeefUL;
	obs.tbl = port::phashinit_flags(elements, &g_mtype, &obs.mask_or_n, flags);

	obs.alloc_calls = g_env.alloc_calls;
	obs.alloc_size = g_env.alloc_size;
	obs.alloc_flags = g_env.alloc_flags;
	obs.alloc_failed = g_env.alloc_failed;
	obs.free_calls = g_env.free_calls;
	kenv_snapshot(obs.arena);

	if (obs.tbl != nullptr) {
		if (!arena_unchanged_outside(pre, obs.alloc_size))
			obs.alloc_failed = -1;
	}
	return (obs);
}

static run_obs
run_ref_phashinit_flags(int elements, int flags, int fail_at)
{
	run_obs obs = {};

	unsigned char pre[ARENA_CAP];
	oracle_malloc_reset();
	oracle_malloc_fail_at(fail_at);
	kenv_snapshot(pre);

	obs.mask_or_n = 0xdeadbeefUL;
	obs.tbl = ref_phashinit_flags(elements, &g_mtype, &obs.mask_or_n, flags);

	obs.alloc_calls = g_env.alloc_calls;
	obs.alloc_size = g_env.alloc_size;
	obs.alloc_flags = g_env.alloc_flags;
	obs.alloc_failed = g_env.alloc_failed;
	obs.free_calls = g_env.free_calls;
	kenv_snapshot(obs.arena);

	if (obs.tbl != nullptr) {
		if (!arena_unchanged_outside(pre, obs.alloc_size))
			obs.alloc_failed = -1;
	}
	return (obs);
}

static void
compare_phashinit_flags(int elements, int flags, int fail_at)
{
	case_row(R_PHASH_FLAGS);

	run_obs p = run_port_phashinit_flags(elements, flags, fail_at);
	run_obs r = run_ref_phashinit_flags(elements, flags, fail_at);

	if ((p.tbl == nullptr) != (r.tbl == nullptr)) {
		fail_row(R_PHASH_FLAGS, "alloc", "null mismatch");
		goto cleanup;
	}
	if (p.alloc_calls != r.alloc_calls)
		fail_row(R_PHASH_FLAGS, "malloc-calls", "count mismatch");
	if (p.alloc_size != r.alloc_size)
		fail_row(R_PHASH_FLAGS, "malloc-size", "size mismatch");
	if (p.alloc_flags != r.alloc_flags)
		fail_row(R_PHASH_FLAGS, "malloc-flags", "flags mismatch");
	if (p.alloc_failed != r.alloc_failed)
		fail_row(R_PHASH_FLAGS, "malloc-fail", "fail state mismatch");
	if (p.tbl != nullptr) {
		if (p.mask_or_n != r.mask_or_n)
			fail_row(R_PHASH_FLAGS, "nentries", "count mismatch");
		if (!check_bucket_inits(p.tbl, p.mask_or_n - 1))
			fail_row(R_PHASH_FLAGS, "port-buckets", "not empty");
		if (!check_bucket_inits(r.tbl, r.mask_or_n - 1))
			fail_row(R_PHASH_FLAGS, "ref-buckets", "not empty");
		if (std::memcmp(p.arena, r.arena, ARENA_CAP) != 0)
			fail_row(R_PHASH_FLAGS, "arena", "buffer mismatch");
	}

cleanup:
	if (p.tbl != nullptr)
		port::hashdestroy(p.tbl, &g_mtype, p.mask_or_n - 1);
	if (r.tbl != nullptr)
		ref_hashdestroy(r.tbl, &g_mtype, r.mask_or_n - 1);
}

static void
compare_hashinit(int elements)
{
	case_row(R_HASH);

	unsigned long pm = 0, rm = 0;
	void *pt = nullptr;
	void *rt = nullptr;

	port::malloc_reset();
	port::malloc_fail_at(0);
	pt = port::hashinit(elements, &g_mtype, &pm);
	run_obs pobs = {};
	pobs.tbl = pt;
	pobs.mask_or_n = pm;
	pobs.alloc_calls = g_env.alloc_calls;
	pobs.alloc_size = g_env.alloc_size;
	pobs.alloc_flags = g_env.alloc_flags;
	kenv_snapshot(pobs.arena);

	oracle_malloc_reset();
	oracle_malloc_fail_at(0);
	rt = ref_hashinit(elements, &g_mtype, &rm);
	run_obs robs = {};
	robs.tbl = rt;
	robs.mask_or_n = rm;
	robs.alloc_calls = g_env.alloc_calls;
	robs.alloc_size = g_env.alloc_size;
	robs.alloc_flags = g_env.alloc_flags;
	kenv_snapshot(robs.arena);

	if ((pt == nullptr) != (rt == nullptr))
		fail_row(R_HASH, "alloc", "null mismatch");
	else if (pt != nullptr) {
		if (pm != rm)
			fail_row(R_HASH, "hashmask", "mask mismatch");
		if (pobs.alloc_flags != robs.alloc_flags)
			fail_row(R_HASH, "malloc-flags", "flags mismatch");
		if (!check_bucket_inits(pt, pm))
			fail_row(R_HASH, "port-buckets", "not empty");
		if (!check_bucket_inits(rt, rm))
			fail_row(R_HASH, "ref-buckets", "not empty");
		if (std::memcmp(pobs.arena, robs.arena, ARENA_CAP) != 0)
			fail_row(R_HASH, "arena", "buffer mismatch");
	}

	if (pt != nullptr)
		port::hashdestroy(pt, &g_mtype, pm);
	if (rt != nullptr)
		ref_hashdestroy(rt, &g_mtype, rm);
}

static void
compare_phashinit(int elements)
{
	case_row(R_PHASH);

	unsigned long pn = 0, rn = 0;
	void *pt = nullptr;
	void *rt = nullptr;

	port::malloc_reset();
	port::malloc_fail_at(0);
	pt = port::phashinit(elements, &g_mtype, &pn);
	run_obs pobs = {};
	pobs.tbl = pt;
	pobs.mask_or_n = pn;
	pobs.alloc_calls = g_env.alloc_calls;
	pobs.alloc_size = g_env.alloc_size;
	pobs.alloc_flags = g_env.alloc_flags;
	kenv_snapshot(pobs.arena);

	oracle_malloc_reset();
	oracle_malloc_fail_at(0);
	rt = ref_phashinit(elements, &g_mtype, &rn);
	run_obs robs = {};
	robs.tbl = rt;
	robs.mask_or_n = rn;
	robs.alloc_calls = g_env.alloc_calls;
	robs.alloc_size = g_env.alloc_size;
	robs.alloc_flags = g_env.alloc_flags;
	kenv_snapshot(robs.arena);

	if ((pt == nullptr) != (rt == nullptr))
		fail_row(R_PHASH, "alloc", "null mismatch");
	else if (pt != nullptr) {
		if (pn != rn)
			fail_row(R_PHASH, "nentries", "count mismatch");
		if (pobs.alloc_flags != robs.alloc_flags)
			fail_row(R_PHASH, "malloc-flags", "flags mismatch");
		if (!check_bucket_inits(pt, pn - 1))
			fail_row(R_PHASH, "port-buckets", "not empty");
		if (!check_bucket_inits(rt, rn - 1))
			fail_row(R_PHASH, "ref-buckets", "not empty");
		if (std::memcmp(pobs.arena, robs.arena, ARENA_CAP) != 0)
			fail_row(R_PHASH, "arena", "buffer mismatch");
	}

	if (pt != nullptr)
		port::hashdestroy(pt, &g_mtype, pn - 1);
	if (rt != nullptr)
		ref_hashdestroy(rt, &g_mtype, rn - 1);
}

static void
compare_hashdestroy(void)
{
	case_row(R_HASH_DESTROY);

	unsigned long pm = 0, rm = 0;
	int pfree = 0, rfree = 0;

	port::malloc_reset();
	void *pt = port::hashinit(8, &g_mtype, &pm);
	pfree = g_env.free_calls;
	port::hashdestroy(pt, &g_mtype, pm);
	pfree = g_env.free_calls - pfree;

	oracle_malloc_reset();
	void *rt = ref_hashinit(8, &g_mtype, &rm);
	rfree = g_env.free_calls;
	ref_hashdestroy(rt, &g_mtype, rm);
	rfree = g_env.free_calls - rfree;

	if (pfree != rfree)
		fail_row(R_HASH_DESTROY, "free-calls", "count mismatch");
}

/* ------------------------------------------------------------------ */
/* Hand-written edge cases                                            */
/* ------------------------------------------------------------------ */

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

/* ------------------------------------------------------------------ */
/* Randomised sweep                                                   */
/* ------------------------------------------------------------------ */

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

/* ------------------------------------------------------------------ */

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
