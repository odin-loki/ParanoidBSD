// Differential test for PBSD batch b0138.

import pbsd.sys.kern.b0138;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>

namespace port = pbsd::sys_kern::b0138;

#define GUARD     0x7f
#define PAD       32u
#define SWEEP     200000L
#define MAX_PRINT 12

#define M_WAITOK 0x0002
#define M_ZERO   0x0100
#define M_NOWAIT 0x0001

struct stat_row {
	const char *name;
	long cases;
	long failures;
	long printed;
};

static stat_row rows[] = {
	{ "cpu_fill_vdso_timehands",   0, 0, 0 },
	{ "cpu_fill_vdso_timehands32", 0, 0, 0 },
	{ "memset_early",              0, 0, 0 },
	{ "memcpy_early",              0, 0, 0 },
	{ "memmove_early",             0, 0, 0 },
	{ "buf_ring_alloc",            0, 0, 0 },
	{ "buf_ring_free",             0, 0, 0 },
	{ "sdt_probe_stub",            0, 0, 0 },
	{ "sdt_probe",                 0, 0, 0 },
	{ "sdt_probe6",                0, 0, 0 },
};

enum {
	R_VDSO = 0,
	R_VDSO32,
	R_MEMSET,
	R_MEMCPY,
	R_MEMMOVE,
	R_BR_ALLOC,
	R_BR_FREE,
	R_STUB,
	R_PROBE,
	R_PROBE6,
};

static uint64_t rng_state = 0x00b0138faceULL;

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

static void
fail_row(int row, const char *label, const char *detail)
{
	stat_row &r = rows[row];

	r.failures++;
	if (r.printed < MAX_PRINT) {
		r.printed++;
		std::printf("  FAIL %-22s %-24s %s\n", r.name, label, detail);
	}
}

/* ----------------------------------------------------------- mocked kernel I/O */

static int g_printf_calls;
static int g_backtrace_calls;

extern "C" int
printf(const char *fmt, ...)
{
	(void)fmt;
	g_printf_calls++;
	return (0);
}

extern "C" void
kdb_backtrace(void)
{
	g_backtrace_calls++;
}

static void
io_reset(void)
{
	g_printf_calls = 0;
	g_backtrace_calls = 0;
}

/* ----------------------------------------------------------- oracle declarations */

struct vdso_timehands {
	uint64_t pad[4];
};

struct vdso_timehands32 {
	uint32_t pad[4];
};

struct timecounter {
	void *tc_priv;
};

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64
#endif

struct buf_ring {
	uint32_t br_prod_head;
	uint32_t br_prod_tail;
	int br_prod_size;
	int br_prod_mask;
	uint64_t br_drops;
	uint32_t br_cons_head __attribute__((aligned(CACHE_LINE_SIZE)));
	uint32_t br_cons_tail;
	int br_cons_size;
	int br_cons_mask;
	void *br_ring[0] __attribute__((aligned(CACHE_LINE_SIZE)));
};

struct malloc_type {
	const char *ks_shortdesc;
};

struct mtx {
	int dummy;
};

extern "C" {
typedef void (*sdt_probe_func_t)(uint32_t, uintptr_t, uintptr_t,
    uintptr_t, uintptr_t, uintptr_t, uintptr_t);

uint32_t ref_cpu_fill_vdso_timehands(struct vdso_timehands *, struct timecounter *);
uint32_t ref_cpu_fill_vdso_timehands32(struct vdso_timehands32 *,
    struct timecounter *);
void *ref_memset_early(void *, int, size_t);
void *ref_memcpy_early(void *, const void *, size_t);
void *ref_memmove_early(void *, const void *, size_t);
struct buf_ring *ref_buf_ring_alloc(int, struct malloc_type *, int, struct mtx *);
void ref_buf_ring_free(struct buf_ring *, struct malloc_type *);
void ref_sdt_probe_stub(uint32_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t,
    uintptr_t, uintptr_t);
void ref_sdt_probe(uint32_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t,
    uintptr_t);
void ref_sdt_probe6(uint32_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t,
    uintptr_t, uintptr_t);

extern sdt_probe_func_t sdt_probe_func;

void oracle_malloc_reset(void);
void oracle_malloc_fail_at(int);
}

/* ----------------------------------------------------------- probe recorder */

struct probe_log {
	int calls;
	uint32_t id;
	uintptr_t args[7];
};

static probe_log port_probe_log;
static probe_log ref_probe_log;

static void
probe_log_reset(probe_log *log)
{
	std::memset(log, 0, sizeof(*log));
}

static void
port_record_probe(uint32_t id, uintptr_t a0, uintptr_t a1, uintptr_t a2,
    uintptr_t a3, uintptr_t a4, uintptr_t a5)
{
	port_probe_log.calls++;
	port_probe_log.id = id;
	port_probe_log.args[0] = a0;
	port_probe_log.args[1] = a1;
	port_probe_log.args[2] = a2;
	port_probe_log.args[3] = a3;
	port_probe_log.args[4] = a4;
	port_probe_log.args[5] = a5;
}

extern "C" void
ref_record_probe(uint32_t id, uintptr_t a0, uintptr_t a1, uintptr_t a2,
    uintptr_t a3, uintptr_t a4, uintptr_t a5)
{
	ref_probe_log.calls++;
	ref_probe_log.id = id;
	ref_probe_log.args[0] = a0;
	ref_probe_log.args[1] = a1;
	ref_probe_log.args[2] = a2;
	ref_probe_log.args[3] = a3;
	ref_probe_log.args[4] = a4;
	ref_probe_log.args[5] = a5;
}

static bool
probe_logs_match(const probe_log &a, const probe_log &b)
{
	if (a.calls != b.calls || a.id != b.id)
		return (false);
	for (int i = 0; i < 7; i++) {
		if (a.args[i] != b.args[i])
			return (false);
	}
	return (true);
}

/* ----------------------------------------------------------- buffer helpers */

struct guarded_buf {
	unsigned char pre[PAD];
	unsigned char data[4096];
	unsigned char post[PAD];
};

static void
guard_init(guarded_buf *g)
{
	std::memset(g->pre, GUARD, PAD);
	std::memset(g->data, GUARD, sizeof(g->data));
	std::memset(g->post, GUARD, PAD);
}

static bool
guards_ok(const guarded_buf *g)
{
	for (size_t i = 0; i < PAD; i++) {
		if (g->pre[i] != GUARD || g->post[i] != GUARD)
			return (false);
	}
	return (true);
}

static void
fill_pattern(unsigned char *p, size_t len, uint32_t seed)
{
	for (size_t i = 0; i < len; i++)
		p[i] = (unsigned char)((seed + (uint32_t)i * 17u) & 0xffu);
}

static void
fill_nulheavy(unsigned char *p, size_t len, uint32_t seed)
{
	for (size_t i = 0; i < len; i++) {
		uint32_t v = seed + (uint32_t)i * 31u;

		p[i] = (unsigned char)((v % 5u == 0u) ? 0 : (v & 0xffu));
	}
}

static bool
bufs_match(const guarded_buf &a, const guarded_buf &b)
{
	return (std::memcmp(&a, &b, sizeof(a)) == 0);
}

static ptrdiff_t
ptr_off(const void *base, const void *p)
{
	return ((const unsigned char *)p - (const unsigned char *)base);
}

/* ----------------------------------------------------------- vdso tests */

static void
test_vdso_one(int row, uint32_t seed)
{
	port::vdso_timehands pth;
	port::timecounter ptc;
	struct vdso_timehands rth;
	struct timecounter rtc;

	std::memset(&pth, (int)(seed & 0xffu), sizeof(pth));
	std::memset(&ptc, (int)((seed >> 8) & 0xffu), sizeof(ptc));
	std::memcpy(&rth, &pth, sizeof(rth));
	std::memcpy(&rtc, &ptc, sizeof(ptc));

	rows[row].cases++;
	uint32_t pr = (row == R_VDSO)
	    ? port::cpu_fill_vdso_timehands(&pth, &ptc)
	    : port::cpu_fill_vdso_timehands32(
	          reinterpret_cast<port::vdso_timehands32 *>(&pth), &ptc);
	uint32_t rr = (row == R_VDSO)
	    ? ref_cpu_fill_vdso_timehands(&rth, &rtc)
	    : ref_cpu_fill_vdso_timehands32(
	          reinterpret_cast<struct vdso_timehands32 *>(&rth), &rtc);

	if (pr != rr)
		fail_row(row, "return", "mismatch");
	if (std::memcmp(&pth, &rth, sizeof(pth)) != 0)
		fail_row(row, "struct", "mutated");
	if (std::memcmp(&ptc, &rtc, sizeof(ptc)) != 0)
		fail_row(row, "tc", "mutated");
}

static void
test_vdso(void)
{
	test_vdso_one(R_VDSO, 0);
	test_vdso_one(R_VDSO32, 0);

	for (uint32_t s = 0; s < 256u; s++) {
		test_vdso_one(R_VDSO, s | 0x80000000u);
		test_vdso_one(R_VDSO32, s | 0x80808080u);
	}

	for (long i = 0; i < SWEEP; i++)
		test_vdso_one(R_VDSO, rnd32());
	for (long i = 0; i < SWEEP; i++)
		test_vdso_one(R_VDSO32, rnd32());
}

/* ----------------------------------------------------------- early memory */

static void
test_memset_one(size_t off, size_t len, int fill, uint32_t seed)
{
	guarded_buf pg;
	guarded_buf rg;

	guard_init(&pg);
	guard_init(&rg);
	if (off + len > sizeof(pg.data))
		len = sizeof(pg.data) - off;
	fill_pattern(pg.data + off, len, seed);
	fill_pattern(rg.data + off, len, seed);

	rows[R_MEMSET].cases++;
	void *pp = port::memset_early(pg.data + off, fill, len);
	void *rp = ref_memset_early(rg.data + off, fill, len);

	if (ptr_off(pg.data + off, pp) != ptr_off(rg.data + off, rp))
		fail_row(R_MEMSET, "ret-off", "mismatch");
	if (!bufs_match(pg, rg))
		fail_row(R_MEMSET, "buffer", "mismatch");
	if (!guards_ok(&pg) || !guards_ok(&rg))
		fail_row(R_MEMSET, "guard", "clobbered");
}

static void
test_memcpy_one(size_t dst_off, size_t src_off, size_t len, uint32_t seed)
{
	guarded_buf pd;
	guarded_buf ps;
	guarded_buf rd;
	guarded_buf rs;

	guard_init(&pd);
	guard_init(&ps);
	guard_init(&rd);
	guard_init(&rs);

	if (dst_off + len > sizeof(pd.data))
		len = sizeof(pd.data) - dst_off;
	if (src_off + len > sizeof(ps.data))
		len = sizeof(ps.data) - src_off;

	fill_pattern(pd.data, sizeof(pd.data), seed);
	fill_pattern(ps.data, sizeof(ps.data), seed ^ 0xabcdefu);
	std::memcpy(rd.data, pd.data, sizeof(rd.data));
	std::memcpy(rs.data, ps.data, sizeof(rs.data));

	rows[R_MEMCPY].cases++;
	void *pp = port::memcpy_early(pd.data + dst_off, ps.data + src_off, len);
	void *rp = ref_memcpy_early(rd.data + dst_off, rs.data + src_off, len);

	if (ptr_off(pd.data + dst_off, pp) != ptr_off(rd.data + dst_off, rp))
		fail_row(R_MEMCPY, "ret-off", "mismatch");
	if (!bufs_match(pd, rd))
		fail_row(R_MEMCPY, "dst", "mismatch");
	if (!guards_ok(&pd) || !guards_ok(&ps) || !guards_ok(&rd) || !guards_ok(&rs))
		fail_row(R_MEMCPY, "guard", "clobbered");
}

static void
test_memmove_one(size_t dst_off, size_t src_off, size_t len, uint32_t seed)
{
	guarded_buf pd;
	guarded_buf rd;

	guard_init(&pd);
	guard_init(&rd);
	if (dst_off + len > sizeof(pd.data))
		len = sizeof(pd.data) - dst_off;
	if (src_off + len > sizeof(pd.data))
		len = sizeof(pd.data) - src_off;

	fill_nulheavy(pd.data, sizeof(pd.data), seed);
	std::memcpy(rd.data, pd.data, sizeof(rd.data));

	rows[R_MEMMOVE].cases++;
	void *pp = port::memmove_early(pd.data + dst_off, pd.data + src_off, len);
	void *rp = ref_memmove_early(rd.data + dst_off, rd.data + src_off, len);

	if (ptr_off(pd.data + dst_off, pp) != ptr_off(rd.data + dst_off, rp))
		fail_row(R_MEMMOVE, "ret-off", "mismatch");
	if (!bufs_match(pd, rd))
		fail_row(R_MEMMOVE, "buffer", "mismatch");
	if (!guards_ok(&pd) || !guards_ok(&rd))
		fail_row(R_MEMMOVE, "guard", "clobbered");
}

static void
test_early_memory(void)
{
	static const int fills[] = {
		0, 1, -1, 127, -128, 128, 255, 256, 0x7f, 0x80, 0xff,
		0x100, 0x101, 0x7fff, -32768, 0x7fffffff, (int)0x80000000u,
	};
	static const size_t lens[] = { 0, 1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63,
	    64, 127, 128, 255, 256, 511, 512, 1023, 1024, 2048, 4096 };
	static const size_t offs[] = { 0, 1, 2, 7, 8, 15, 16, 31, 32, 63, 64,
	    127, 128, 255, 256, 512, 1024, 2048, 3000 };

	for (int fill : fills) {
		for (size_t len : lens)
			test_memset_one(0, len, fill, (uint32_t)fill ^ (uint32_t)len);
		for (size_t off : offs)
			test_memset_one(off, 64, fill, (uint32_t)off ^ (uint32_t)fill);
	}

	for (size_t len : lens) {
		test_memcpy_one(0, 0, len, (uint32_t)len);
		test_memcpy_one(16, 0, len, (uint32_t)len ^ 1u);
		test_memcpy_one(0, 16, len, (uint32_t)len ^ 2u);
	}

	for (size_t len : lens) {
		test_memmove_one(0, 0, len, (uint32_t)len);
		test_memmove_one(32, 0, len, (uint32_t)len ^ 3u);
		test_memmove_one(0, 32, len, (uint32_t)len ^ 4u);
		test_memmove_one(1, 0, len, (uint32_t)len ^ 5u);
		test_memmove_one(64, 32, len, (uint32_t)len ^ 6u);
	}

	for (long i = 0; i < SWEEP; i++) {
		uint32_t s = rnd32();
		size_t len = (size_t)(s & 0xfffu);
		size_t off = (size_t)((s >> 12) & 0xfffu);
		int fill = (int)(s ^ (s >> 16));

		test_memset_one(off % 2048, len, fill, s);
	}
	for (long i = 0; i < SWEEP; i++) {
		uint32_t s = rnd32();
		size_t len = (size_t)(s & 0xfffu);
		size_t d = (size_t)((s >> 10) & 0xfffu);
		size_t so = (size_t)((s >> 20) & 0xfffu);

		test_memcpy_one(d % 2048, so % 2048, len, s);
	}
	for (long i = 0; i < SWEEP; i++) {
		uint32_t s = rnd32();
		size_t len = (size_t)(s & 0xfffu);
		size_t d = (size_t)((s >> 10) & 0xfffu);
		size_t so = (size_t)((s >> 20) & 0xfffu);

		test_memmove_one(d % 2048, so % 2048, len, s);
	}
}

/* ----------------------------------------------------------- buf_ring */

static size_t
br_bytes(int count)
{
	return (sizeof(port::buf_ring) + (size_t)count * sizeof(char *));
}

static void
compare_br_alloc(int count, int flags, int fail_at)
{
	port::malloc_type ptype = { "port" };
	struct malloc_type rtype = { "ref" };
	port::mtx plock = { 1 };
	struct mtx rlock = { 2 };

	port::malloc_reset();
	oracle_malloc_reset();
	port::malloc_fail_at(fail_at);
	oracle_malloc_fail_at(fail_at);

	rows[R_BR_ALLOC].cases++;
	port::buf_ring *pbr = port::buf_ring_alloc(count, &ptype, flags, &plock);
	struct buf_ring *rbr = ref_buf_ring_alloc(count, &rtype, flags, &rlock);

	bool pnull = (pbr == nullptr);
	bool rnull = (rbr == nullptr);
	if (pnull != rnull)
		fail_row(R_BR_ALLOC, "null", "mismatch");

	if (!pnull && !rnull) {
		size_t n = br_bytes(count);
		if (std::memcmp(pbr, rbr, n) != 0)
			fail_row(R_BR_ALLOC, "bytes", "mismatch");
	}

	rows[R_BR_FREE].cases++;
	port::buf_ring_free(pbr, &ptype);
	ref_buf_ring_free(rbr, &rtype);
}

static void
test_buf_ring(void)
{
	static const int counts[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512,
	    1024, 2048 };
	static const int flags[] = { 0, M_ZERO, M_WAITOK, M_NOWAIT,
	    M_WAITOK | M_ZERO, M_NOWAIT | M_ZERO, M_NOWAIT | M_WAITOK,
	    M_NOWAIT | M_WAITOK | M_ZERO };

	for (int count : counts) {
		for (int fl : flags)
			compare_br_alloc(count, fl, 0);
	}

	/* non-power-of-two counts still run with KASSERT disabled */
	for (int count : { 0, 3, 5, 6, 7, 9, 10, 15, 17, 31, 33, 63, 65, 100,
	    127, 129, 255, 257, 511, 513, 1000, 1023, 1025 }) {
		compare_br_alloc(count, M_WAITOK | M_ZERO, 0);
	}

	for (int n = 1; n <= 8; n++)
		compare_br_alloc(16, M_WAITOK | M_ZERO, n);

	for (long i = 0; i < SWEEP; i++) {
		uint32_t s = rnd32();
		int count = (int)((s & 0x3ffu) + 1u);
		if ((s & 0x8000u) != 0) {
			/* force power of two */
			if (count > 1024)
				count = 1024;
			int p = 1;
			while (p < count)
				p <<= 1;
			count = p;
		}
		int fl = (int)(s >> 16) & (M_WAITOK | M_NOWAIT | M_ZERO);
		int fail = ((s & 0x10000u) != 0) ? (int)(s & 0xfu) + 1 : 0;
		compare_br_alloc(count, fl, fail);
	}
}

/* ----------------------------------------------------------- sdt */

static void
test_sdt_stub_once(uint32_t seed)
{
	(void)seed;
	io_reset();

	rows[R_STUB].cases++;
	port::sdt_probe_func = port::sdt_probe_stub;
	sdt_probe_func = ref_sdt_probe_stub;

	port::sdt_probe_stub(seed, seed ^ 1u, seed ^ 2u, seed ^ 3u, seed ^ 4u,
	    seed ^ 5u, seed ^ 6u);
	ref_sdt_probe_stub(seed, seed ^ 1u, seed ^ 2u, seed ^ 3u, seed ^ 4u,
	    seed ^ 5u, seed ^ 6u);

	if (g_printf_calls != 2)
		fail_row(R_STUB, "printf", "count");
	if (g_backtrace_calls != 2)
		fail_row(R_STUB, "backtrace", "count");
}

static void
test_sdt_probe_once(uint32_t id, uintptr_t a0, uintptr_t a1, uintptr_t a2,
    uintptr_t a3, uintptr_t a4, bool six, uintptr_t a5)
{
	probe_log_reset(&port_probe_log);
	probe_log_reset(&ref_probe_log);
	port::sdt_probe_func = port_record_probe;
	sdt_probe_func = ref_record_probe;

	rows[six ? R_PROBE6 : R_PROBE].cases++;
	if (six)
		port::sdt_probe6(id, a0, a1, a2, a3, a4, a5);
	else
		port::sdt_probe(id, a0, a1, a2, a3, a4);

	if (six)
		ref_sdt_probe6(id, a0, a1, a2, a3, a4, a5);
	else
		ref_sdt_probe(id, a0, a1, a2, a3, a4);

	if (!probe_logs_match(port_probe_log, ref_probe_log))
		fail_row(six ? R_PROBE6 : R_PROBE, "args", "mismatch");
	if (!six && port_probe_log.args[6] != 0)
		fail_row(R_PROBE, "arg5", "not zero");
}

static void
test_sdt(void)
{
	for (uint32_t s = 0; s < 256u; s++)
		test_sdt_stub_once(s);

	for (long i = 0; i < SWEEP; i++) {
		uint32_t s = rnd32();
		test_sdt_stub_once(s);
	}

	static const uintptr_t edges[] = { 0, 1, 2, 0x7f, 0x80, 0xff, 0x100,
	    0x7fff, 0x8000, 0xffff, 0x10000, 0x7fffffff, 0x80000000,
	    0xffffffff, 0x100000000ULL, 0xffffffffffffffffULL };

	for (uintptr_t a0 : edges) {
		for (uintptr_t a1 : edges) {
			test_sdt_probe_once((uint32_t)a0, a0, a1, a0 ^ a1,
			    a0 + a1, a0 - a1, false, 0);
			test_sdt_probe_once((uint32_t)a1, a0, a1, a0 ^ a1,
			    a0 + a1, a0 - a1, true, a0 ^ a1 ^ a0);
		}
	}

	for (long i = 0; i < SWEEP; i++) {
		uint32_t s = rnd32();
		uintptr_t a0 = (uintptr_t)rnd64();
		uintptr_t a1 = (uintptr_t)rnd64();
		uintptr_t a2 = (uintptr_t)rnd64();
		uintptr_t a3 = (uintptr_t)rnd64();
		uintptr_t a4 = (uintptr_t)rnd64();
		uintptr_t a5 = (uintptr_t)rnd64();
		bool six = (s & 1u) != 0;

		test_sdt_probe_once(s, a0, a1, a2, a3, a4, six, a5);
	}
}

/* ------------------------------------------------------------------ main */

int
main(void)
{
	test_vdso();
	test_early_memory();
	test_buf_ring();
	test_sdt();

	long total_cases = 0;
	long total_fail = 0;

	std::printf("\n%-22s %10s %10s\n", "function", "cases", "failures");
	std::printf("%-22s %10s %10s\n", "--------", "-----", "--------");
	for (const stat_row &r : rows) {
		std::printf("%-22s %10ld %10ld\n", r.name, r.cases, r.failures);
		total_cases += r.cases;
		total_fail += r.failures;
	}
	std::printf("%-22s %10ld %10ld\n", "TOTAL", total_cases, total_fail);

	return (total_fail == 0 ? 0 : 1);
}
