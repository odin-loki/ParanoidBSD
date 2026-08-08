/*
 * PBSD batch b0138 -- differential test: C++23 port vs. the ref_ oracle.
 *
 * Every ported function is driven with hand-written edge cases and with a
 * fixed-seed randomised sweep.  Buffer writers are checked over their WHOLE
 * destination buffer (guard byte 0x7f past the nominal write window
 * included), pointer results are compared as offsets from the buffer base,
 * and the kernel-service side effects (malloc size/flags/type, free, printf
 * text, kdb_backtrace count, probe function arguments) are compared too.
 */

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <climits>

import pbsd.sys.kern.b0138;

namespace k = pbsd::sys_kern::b0138;

/* ------------------------------------------------------------------ */
/* Oracle declarations                                                */
/* ------------------------------------------------------------------ */

extern "C" {

struct malloc_type;
struct mtx;
struct vdso_timehands;
struct vdso_timehands32;
struct timecounter;
struct buf_ring;

typedef void (*ref_probe_func_t)(std::uint32_t, std::uintptr_t,
    std::uintptr_t, std::uintptr_t, std::uintptr_t, std::uintptr_t,
    std::uintptr_t);

std::uint32_t ref_cpu_fill_vdso_timehands(struct vdso_timehands *,
    struct timecounter *);
std::uint32_t ref_cpu_fill_vdso_timehands32(struct vdso_timehands32 *,
    struct timecounter *);

void *ref_memset_early(void *, int, std::size_t);
void *ref_memcpy_early(void *, const void *, std::size_t);
void *ref_memmove_early(void *, const void *, std::size_t);

struct buf_ring *ref_buf_ring_alloc(int, struct malloc_type *, int,
    struct mtx *);
void ref_buf_ring_free(struct buf_ring *, struct malloc_type *);

void ref_sdt_probe_stub(std::uint32_t, std::uintptr_t, std::uintptr_t,
    std::uintptr_t, std::uintptr_t, std::uintptr_t, std::uintptr_t);
void ref_sdt_probe(std::uint32_t, std::uintptr_t, std::uintptr_t,
    std::uintptr_t, std::uintptr_t, std::uintptr_t);
void ref_sdt_probe6(std::uint32_t, std::uintptr_t, std::uintptr_t,
    std::uintptr_t, std::uintptr_t, std::uintptr_t, std::uintptr_t);

extern ref_probe_func_t sdt_probe_func;		/* oracle global */
extern volatile bool sdt_probes_enabled;	/* oracle global */

} /* extern "C" */

/* ------------------------------------------------------------------ */
/* Kernel services (one shared environment, used by both sides)       */
/* ------------------------------------------------------------------ */

enum : int { M_NOWAIT = 0x0001, M_WAITOK = 0x0002, M_ZERO = 0x0100 };

/* Simulated kernel heap: a single arena, so both sides are handed the same
 * base address; requests larger than the arena fail the way M_NOWAIT would. */
enum : std::size_t { ARENA_CAP = 8192 };
alignas(64) static unsigned char g_arena[ARENA_CAP];

static int g_force_alloc_fail;

struct kenv_state {
	int		alloc_calls;
	std::size_t	alloc_size;
	const void	*alloc_type;
	int		alloc_flags;
	int		alloc_failed;
	int		free_calls;
	const void	*free_ptr;
	const void	*free_type;
	int		printf_calls;
	int		backtrace_calls;
	char		text[512];
	std::size_t	text_len;
};

static kenv_state g_env;

/*
 * The port declares these hooks with its own (module-attached) malloc_type,
 * and clang makes that declaration visible to importers, so the definitions
 * here must use the same type.  Both are opaque; only the pointer value is
 * ever inspected.
 */
extern "C" void *
pbsd_kern_malloc(std::size_t size, k::malloc_type *type, int flags)
{

	g_env.alloc_calls++;
	g_env.alloc_size = size;
	g_env.alloc_type = static_cast<const void *>(type);
	g_env.alloc_flags = flags;
	std::memset(g_arena, 0x7f, ARENA_CAP);
	if (g_force_alloc_fail != 0 || size > ARENA_CAP) {
		g_env.alloc_failed = 1;
		return (nullptr);
	}
	if ((flags & M_ZERO) != 0)
		std::memset(g_arena, 0, size);
	g_env.alloc_failed = 0;
	return (g_arena);
}

extern "C" void
pbsd_kern_free(void *addr, struct malloc_type *type)
{

	g_env.free_calls++;
	g_env.free_ptr = addr;
	g_env.free_type = static_cast<const void *>(type);
}

extern "C" int
pbsd_kern_printf(const char *fmt, ...)
{
	std::va_list ap;
	int n;

	g_env.printf_calls++;
	va_start(ap, fmt);
	n = std::vsnprintf(g_env.text + g_env.text_len,
	    sizeof(g_env.text) - g_env.text_len, fmt, ap);
	va_end(ap);
	if (n > 0) {
		g_env.text_len += static_cast<std::size_t>(n);
		if (g_env.text_len >= sizeof(g_env.text))
			g_env.text_len = sizeof(g_env.text) - 1;
	}
	return (n);
}

extern "C" void
kdb_backtrace(void)
{

	g_env.backtrace_calls++;
}

/*
 * Only the recorded side effects need clearing: pbsd_kern_malloc() repaints
 * the entire arena with the guard byte on every call, failed calls included,
 * so the arena state after an allocation attempt is fully determined by that
 * call.
 */
static void
kenv_reset(void)
{

	std::memset(&g_env, 0, sizeof(g_env));
}

static bool
kenv_equal(const kenv_state &a, const kenv_state &b)
{

	return (a.alloc_calls == b.alloc_calls &&
	    a.alloc_size == b.alloc_size &&
	    a.alloc_type == b.alloc_type &&
	    a.alloc_flags == b.alloc_flags &&
	    a.alloc_failed == b.alloc_failed &&
	    a.free_calls == b.free_calls &&
	    a.free_ptr == b.free_ptr &&
	    a.free_type == b.free_type &&
	    a.printf_calls == b.printf_calls &&
	    a.backtrace_calls == b.backtrace_calls &&
	    a.text_len == b.text_len &&
	    std::memcmp(a.text, b.text, sizeof(a.text)) == 0);
}

/* ------------------------------------------------------------------ */
/* Probe recorder (installed into both sdt_probe_func variables)       */
/* ------------------------------------------------------------------ */

struct probe_rec {
	int		calls;
	std::uint32_t	id;
	std::uintptr_t	arg[6];
};

static probe_rec g_rec;

extern "C" void
probe_recorder(std::uint32_t id, std::uintptr_t a0, std::uintptr_t a1,
    std::uintptr_t a2, std::uintptr_t a3, std::uintptr_t a4,
    std::uintptr_t a5)
{

	g_rec.calls++;
	g_rec.id = id;
	g_rec.arg[0] = a0;
	g_rec.arg[1] = a1;
	g_rec.arg[2] = a2;
	g_rec.arg[3] = a3;
	g_rec.arg[4] = a4;
	g_rec.arg[5] = a5;
}

static bool
probe_equal(const probe_rec &a, const probe_rec &b)
{

	if (a.calls != b.calls || a.id != b.id)
		return (false);
	for (int i = 0; i < 6; i++)
		if (a.arg[i] != b.arg[i])
			return (false);
	return (true);
}

/* ------------------------------------------------------------------ */
/* Scoreboard                                                         */
/* ------------------------------------------------------------------ */

enum {
	F_VDSO,
	F_VDSO32,
	F_MEMSET,
	F_MEMCPY,
	F_MEMMOVE,
	F_BR_ALLOC,
	F_BR_FREE,
	F_STUB,
	F_PROBE,
	F_PROBE6,
	F_COUNT
};

static const char *const f_name[F_COUNT] = {
	"cpu_fill_vdso_timehands",
	"cpu_fill_vdso_timehands32",
	"memset_early",
	"memcpy_early",
	"memmove_early",
	"buf_ring_alloc",
	"buf_ring_free",
	"sdt_probe_stub",
	"sdt_probe",
	"sdt_probe6",
};

static long f_cases[F_COUNT];
static long f_fails[F_COUNT];
static int f_reported[F_COUNT];

static void
record(int fn, bool ok, const char *what, const char *detail)
{

	f_cases[fn]++;
	if (ok)
		return;
	f_fails[fn]++;
	if (f_reported[fn] < 5) {
		f_reported[fn]++;
		std::printf("MISMATCH %s: %s [%s]\n", f_name[fn], what, detail);
	}
}

/* ------------------------------------------------------------------ */
/* Fixed-seed PRNG (splitmix64)                                       */
/* ------------------------------------------------------------------ */

static std::uint64_t g_seed = 0x9e3779b97f4a7c15ULL;

static std::uint64_t
rnd(void)
{
	std::uint64_t z;

	g_seed += 0x9e3779b97f4a7c15ULL;
	z = g_seed;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static std::uint32_t
rnd_below(std::uint32_t n)
{

	return (n == 0 ? 0 : static_cast<std::uint32_t>(rnd() % n));
}

/* ------------------------------------------------------------------ */
/* subr_dummy_vdso_tc.c                                               */
/* ------------------------------------------------------------------ */

static void
test_vdso(void)
{
	static unsigned char scratch[64];
	void *ptrs[4];
	char buf[128];

	ptrs[0] = nullptr;
	ptrs[1] = scratch;
	ptrs[2] = scratch + 32;
	ptrs[3] = reinterpret_cast<void *>(static_cast<std::uintptr_t>(~0ULL));

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			std::uint32_t p = k::cpu_fill_vdso_timehands(
			    static_cast<k::vdso_timehands *>(ptrs[i]),
			    static_cast<k::timecounter *>(ptrs[j]));
			std::uint32_t r = ref_cpu_fill_vdso_timehands(
			    static_cast<struct vdso_timehands *>(ptrs[i]),
			    static_cast<struct timecounter *>(ptrs[j]));
			std::snprintf(buf, sizeof(buf),
			    "i=%d j=%d port=%u ref=%u", i, j, p, r);
			record(F_VDSO, p == r && p == 0, "return", buf);

			std::uint32_t p32 = k::cpu_fill_vdso_timehands32(
			    static_cast<k::vdso_timehands32 *>(ptrs[i]),
			    static_cast<k::timecounter *>(ptrs[j]));
			std::uint32_t r32 = ref_cpu_fill_vdso_timehands32(
			    static_cast<struct vdso_timehands32 *>(ptrs[i]),
			    static_cast<struct timecounter *>(ptrs[j]));
			std::snprintf(buf, sizeof(buf),
			    "i=%d j=%d port=%u ref=%u", i, j, p32, r32);
			record(F_VDSO32, p32 == r32 && p32 == 0, "return",
			    buf);
		}
	}

	for (long it = 0; it < 200000; it++) {
		std::uintptr_t a = static_cast<std::uintptr_t>(rnd());
		std::uintptr_t b = static_cast<std::uintptr_t>(rnd());
		std::uint32_t p = k::cpu_fill_vdso_timehands(
		    reinterpret_cast<k::vdso_timehands *>(a),
		    reinterpret_cast<k::timecounter *>(b));
		std::uint32_t r = ref_cpu_fill_vdso_timehands(
		    reinterpret_cast<struct vdso_timehands *>(a),
		    reinterpret_cast<struct timecounter *>(b));
		std::snprintf(buf, sizeof(buf), "it=%ld port=%u ref=%u",
		    it, p, r);
		record(F_VDSO, p == r && p == 0, "rnd return", buf);

		std::uint32_t p32 = k::cpu_fill_vdso_timehands32(
		    reinterpret_cast<k::vdso_timehands32 *>(a),
		    reinterpret_cast<k::timecounter *>(b));
		std::uint32_t r32 = ref_cpu_fill_vdso_timehands32(
		    reinterpret_cast<struct vdso_timehands32 *>(a),
		    reinterpret_cast<struct timecounter *>(b));
		std::snprintf(buf, sizeof(buf), "it=%ld port=%u ref=%u",
		    it, p32, r32);
		record(F_VDSO32, p32 == r32 && p32 == 0, "rnd return", buf);
	}
}

/* ------------------------------------------------------------------ */
/* subr_early.c                                                       */
/* ------------------------------------------------------------------ */

enum : std::size_t { BUFSZ = 512, SLOT = 128 };

static unsigned char pbuf[BUFSZ];
static unsigned char rbuf[BUFSZ];
static unsigned char srcpat[BUFSZ];

/*
 * One memset_early case: write at pbuf/rbuf + off for len bytes, guard byte
 * 0x7f everywhere else, compare the ENTIRE buffer plus the returned offset.
 */
static void
case_memset(std::size_t off, std::size_t len, int c)
{
	char buf[160];
	void *pr, *rr;
	long poff, roff;

	std::memset(pbuf, 0x7f, BUFSZ);
	std::memset(rbuf, 0x7f, BUFSZ);
	pr = k::memset_early(pbuf + off, c, len);
	rr = ref_memset_early(rbuf + off, c, len);
	poff = static_cast<unsigned char *>(pr) - pbuf;
	roff = static_cast<unsigned char *>(rr) - rbuf;
	std::snprintf(buf, sizeof(buf),
	    "off=%zu len=%zu c=%d poff=%ld roff=%ld", off, len, c, poff, roff);
	record(F_MEMSET, poff == roff &&
	    std::memcmp(pbuf, rbuf, BUFSZ) == 0, "ret+buffer", buf);
}

static void
case_memcpy(std::size_t doff, std::size_t soff, std::size_t len)
{
	char buf[160];
	void *pr, *rr;
	long poff, roff;
	unsigned char psrc[BUFSZ], rsrc[BUFSZ];

	std::memset(pbuf, 0x7f, BUFSZ);
	std::memset(rbuf, 0x7f, BUFSZ);
	std::memcpy(psrc, srcpat, BUFSZ);
	std::memcpy(rsrc, srcpat, BUFSZ);
	pr = k::memcpy_early(pbuf + doff, psrc + soff, len);
	rr = ref_memcpy_early(rbuf + doff, rsrc + soff, len);
	poff = static_cast<unsigned char *>(pr) - pbuf;
	roff = static_cast<unsigned char *>(rr) - rbuf;
	std::snprintf(buf, sizeof(buf),
	    "doff=%zu soff=%zu len=%zu poff=%ld roff=%ld",
	    doff, soff, len, poff, roff);
	record(F_MEMCPY, poff == roff &&
	    std::memcmp(pbuf, rbuf, BUFSZ) == 0 &&
	    std::memcmp(psrc, rsrc, BUFSZ) == 0, "ret+buffer", buf);
}

/* memmove_early, source and destination in the same buffer so that overlap
 * in either direction is exercised. */
static void
case_memmove(std::size_t doff, std::size_t soff, std::size_t len)
{
	char buf[160];
	void *pr, *rr;
	long poff, roff;

	std::memset(pbuf, 0x7f, BUFSZ);
	std::memset(rbuf, 0x7f, BUFSZ);
	std::memcpy(pbuf + SLOT, srcpat, BUFSZ - SLOT);
	std::memcpy(rbuf + SLOT, srcpat, BUFSZ - SLOT);
	pr = k::memmove_early(pbuf + doff, pbuf + soff, len);
	rr = ref_memmove_early(rbuf + doff, rbuf + soff, len);
	poff = static_cast<unsigned char *>(pr) - pbuf;
	roff = static_cast<unsigned char *>(rr) - rbuf;
	std::snprintf(buf, sizeof(buf),
	    "doff=%zu soff=%zu len=%zu poff=%ld roff=%ld",
	    doff, soff, len, poff, roff);
	record(F_MEMMOVE, poff == roff &&
	    std::memcmp(pbuf, rbuf, BUFSZ) == 0, "ret+buffer", buf);
}

static void
test_early(void)
{
	static const std::size_t lens[] = { 0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16,
	    17, 31, 32, 33, 63, 64, 65, 127, 128, 129, 191, 255, 256 };
	static const std::size_t offs[] = { 0, 1, 2, 3, 7, 8, 15, 16, 63, 64,
	    100, 128 };
	static const int cs[] = { 0, 1, 2, 0x7e, 0x7f, 0x80, 0x81, 0xfe, 0xff,
	    0x100, 0x101, 0x1ff, -1, -2, -128, -129, -255, -256, INT_MIN,
	    INT_MAX };
	std::size_t nl = sizeof(lens) / sizeof(lens[0]);
	std::size_t no = sizeof(offs) / sizeof(offs[0]);
	std::size_t nc = sizeof(cs) / sizeof(cs[0]);

	for (std::size_t i = 0; i < BUFSZ; i++)
		srcpat[i] = static_cast<unsigned char>((i * 7) ^ 0x80 ^
		    (i >> 3));
	/* NUL-heavy and high-bit runs inside the pattern. */
	std::memset(srcpat + 32, 0x00, 32);
	std::memset(srcpat + 96, 0xff, 16);
	std::memset(srcpat + 200, 0x80, 24);

	for (std::size_t o = 0; o < no; o++) {
		for (std::size_t l = 0; l < nl; l++) {
			if (offs[o] + lens[l] > BUFSZ)
				continue;
			for (std::size_t c = 0; c < nc; c++)
				case_memset(offs[o], lens[l], cs[c]);
			case_memcpy(offs[o], 0, lens[l]);
			case_memcpy(0, offs[o], lens[l]);
			case_memcpy(offs[o], offs[o], lens[l]);
		}
	}

	/* memmove: identical, one-byte overlap either way, adjacent, half
	 * overlapping and fully disjoint -- both sides of "src < dst". */
	for (std::size_t l = 0; l < nl; l++) {
		std::size_t len = lens[l];

		if (len <= BUFSZ - 256) {
			case_memmove(0, 256, len);
			case_memmove(256, 0, len);
		}
		if (len > 200)
			continue;
		case_memmove(64, 64, len);
		case_memmove(64, 65, len);
		case_memmove(65, 64, len);
		case_memmove(64, 64 + len, len);
		case_memmove(64 + len, 64, len);
		case_memmove(64, 64 + len / 2, len);
		case_memmove(64 + len / 2, 64, len);
	}

	for (long it = 0; it < 200000; it++) {
		std::size_t len = rnd_below(257);
		std::size_t off = rnd_below(static_cast<std::uint32_t>(BUFSZ -
		    len));
		int c;

		switch (rnd_below(4)) {
		case 0:
			c = static_cast<int>(rnd_below(256));
			break;
		case 1:
			c = -static_cast<int>(rnd_below(512));
			break;
		case 2:
			c = static_cast<int>(rnd());
			break;
		default:
			c = static_cast<int>(rnd_below(65536)) - 32768;
			break;
		}
		case_memset(off, len, c);

		std::size_t dlen = rnd_below(257);
		std::size_t doff = rnd_below(static_cast<std::uint32_t>(BUFSZ -
		    dlen));
		std::size_t soff = rnd_below(static_cast<std::uint32_t>(BUFSZ -
		    dlen));
		case_memcpy(doff, soff, dlen);
		case_memmove(doff, soff, dlen);
	}
}

/* ------------------------------------------------------------------ */
/* subr_bufring.c                                                     */
/* ------------------------------------------------------------------ */

static void
case_buf_ring(int count, int flags, unsigned typesel, unsigned locksel,
    int forcefail)
{
	char buf[240];
	static unsigned char snapshot[ARENA_CAP];
	kenv_state penv, renv;
	void *pbr, *rbr;
	long pofs, rofs;
	struct malloc_type *rtype;
	k::malloc_type *ptype;
	struct mtx *rlock;
	k::mtx *plock;
	std::uintptr_t tv, lv;
	bool ok;

	tv = typesel == 0 ? 0 : static_cast<std::uintptr_t>(0x1000 * typesel);
	lv = locksel == 0 ? 0 : static_cast<std::uintptr_t>(0x2000 * locksel);
	ptype = reinterpret_cast<k::malloc_type *>(tv);
	rtype = reinterpret_cast<struct malloc_type *>(tv);
	plock = reinterpret_cast<k::mtx *>(lv);
	rlock = reinterpret_cast<struct mtx *>(lv);

	g_force_alloc_fail = forcefail;

	kenv_reset();
	pbr = k::buf_ring_alloc(count, ptype, flags, plock);
	penv = g_env;
	std::memcpy(snapshot, g_arena, ARENA_CAP);
	pofs = pbr == nullptr ? -1 :
	    static_cast<long>(static_cast<unsigned char *>(pbr) - g_arena);

	kenv_reset();
	rbr = static_cast<void *>(ref_buf_ring_alloc(count, rtype, flags,
	    rlock));
	renv = g_env;
	rofs = rbr == nullptr ? -1 :
	    static_cast<long>(static_cast<unsigned char *>(rbr) - g_arena);

	ok = pofs == rofs && kenv_equal(penv, renv) &&
	    std::memcmp(snapshot, g_arena, ARENA_CAP) == 0;
	std::snprintf(buf, sizeof(buf),
	    "count=%d flags=0x%x type=%p lock=%p fail=%d pofs=%ld rofs=%ld "
	    "psz=%zu rsz=%zu pfl=0x%x rfl=0x%x",
	    count, flags, static_cast<void *>(ptype),
	    static_cast<void *>(plock), forcefail, pofs, rofs,
	    penv.alloc_size, renv.alloc_size, penv.alloc_flags,
	    renv.alloc_flags);
	record(F_BR_ALLOC, ok, "ret+arena+malloc args", buf);

	/* Free the (identical) rings and compare the free() side effects. */
	kenv_reset();
	k::buf_ring_free(static_cast<k::buf_ring *>(pbr), ptype);
	penv = g_env;
	kenv_reset();
	ref_buf_ring_free(static_cast<struct buf_ring *>(rbr), rtype);
	renv = g_env;
	std::snprintf(buf, sizeof(buf),
	    "count=%d type=%p pcalls=%d rcalls=%d pptr=%p rptr=%p",
	    count, static_cast<void *>(ptype), penv.free_calls,
	    renv.free_calls, const_cast<void *>(penv.free_ptr),
	    const_cast<void *>(renv.free_ptr));
	record(F_BR_FREE, kenv_equal(penv, renv), "free side effects", buf);

	g_force_alloc_fail = 0;
}

static void
test_bufring(void)
{
	static const int counts[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 15, 16, 17,
	    31, 32, 33, 63, 64, 65, 127, 128, 129, 255, 256, 257, 511, 512,
	    1023, 1024, 4095, 4096, 8192, 65536, -1, -2, -3, -4, -8, -1024,
	    INT_MIN, INT_MAX, INT_MAX - 1, INT_MIN + 1 };
	static const int flagset[] = { 0, M_NOWAIT, M_WAITOK, M_ZERO,
	    M_NOWAIT | M_ZERO, M_NOWAIT | M_WAITOK, ~0, 0x4000 };
	std::size_t nc = sizeof(counts) / sizeof(counts[0]);
	std::size_t nf = sizeof(flagset) / sizeof(flagset[0]);

	for (std::size_t i = 0; i < nc; i++) {
		for (std::size_t j = 0; j < nf; j++) {
			case_buf_ring(counts[i], flagset[j],
			    static_cast<unsigned>(j % 3),
			    static_cast<unsigned>(i % 3), 0);
			case_buf_ring(counts[i], flagset[j],
			    static_cast<unsigned>(j % 3),
			    static_cast<unsigned>(i % 3), 1);
		}
	}

	for (long it = 0; it < 200000; it++) {
		int count;
		int flags;

		switch (rnd_below(6)) {
		case 0:
			count = 1 << rnd_below(20);		/* 2^n */
			break;
		case 1:
			count = (1 << rnd_below(20)) - 1;	/* 2^n - 1 */
			break;
		case 2:
			count = (1 << rnd_below(20)) + 1;	/* 2^n + 1 */
			break;
		case 3:
			count = static_cast<int>(rnd_below(1024));
			break;
		case 4:
			count = -static_cast<int>(rnd_below(1024));
			break;
		default:
			count = static_cast<int>(rnd());
			break;
		}
		flags = static_cast<int>(rnd());
		case_buf_ring(count, flags, rnd_below(3), rnd_below(3),
		    static_cast<int>(rnd_below(8) == 0));
	}
}

/* ------------------------------------------------------------------ */
/* kern_sdt.c                                                         */
/* ------------------------------------------------------------------ */

static void
case_probe(std::uint32_t id, const std::uintptr_t *a, bool with_recorder)
{
	char buf[240];
	kenv_state penv, renv;
	probe_rec prec, rrec;
	bool ok;

	if (with_recorder) {
		k::sdt_probe_func = &probe_recorder;
		sdt_probe_func = &probe_recorder;
	} else {
		k::sdt_probe_func = &k::sdt_probe_stub;
		sdt_probe_func = &ref_sdt_probe_stub;
	}

	/* sdt_probe: five arguments plus a literal 0 for arg5. */
	kenv_reset();
	std::memset(&g_rec, 0, sizeof(g_rec));
	k::sdt_probe(id, a[0], a[1], a[2], a[3], a[4]);
	penv = g_env;
	prec = g_rec;

	kenv_reset();
	std::memset(&g_rec, 0, sizeof(g_rec));
	ref_sdt_probe(id, a[0], a[1], a[2], a[3], a[4]);
	renv = g_env;
	rrec = g_rec;

	ok = kenv_equal(penv, renv) && probe_equal(prec, rrec);
	std::snprintf(buf, sizeof(buf),
	    "id=%u rec=%d pcalls=%d rcalls=%d pa5=%llu ra5=%llu ptext=%zu "
	    "rtext=%zu", id, static_cast<int>(with_recorder), prec.calls,
	    rrec.calls, static_cast<unsigned long long>(prec.arg[5]),
	    static_cast<unsigned long long>(rrec.arg[5]), penv.text_len,
	    renv.text_len);
	record(F_PROBE, ok, "probe dispatch", buf);

	/* sdt_probe6: all six arguments forwarded. */
	kenv_reset();
	std::memset(&g_rec, 0, sizeof(g_rec));
	k::sdt_probe6(id, a[0], a[1], a[2], a[3], a[4], a[5]);
	penv = g_env;
	prec = g_rec;

	kenv_reset();
	std::memset(&g_rec, 0, sizeof(g_rec));
	ref_sdt_probe6(id, a[0], a[1], a[2], a[3], a[4], a[5]);
	renv = g_env;
	rrec = g_rec;

	ok = kenv_equal(penv, renv) && probe_equal(prec, rrec);
	std::snprintf(buf, sizeof(buf),
	    "id=%u rec=%d pcalls=%d rcalls=%d pa0=%llu ra0=%llu pa5=%llu "
	    "ra5=%llu", id, static_cast<int>(with_recorder), prec.calls,
	    rrec.calls, static_cast<unsigned long long>(prec.arg[0]),
	    static_cast<unsigned long long>(rrec.arg[0]),
	    static_cast<unsigned long long>(prec.arg[5]),
	    static_cast<unsigned long long>(rrec.arg[5]));
	record(F_PROBE6, ok, "probe6 dispatch", buf);

	/* sdt_probe_stub called directly. */
	kenv_reset();
	k::sdt_probe_stub(id, a[0], a[1], a[2], a[3], a[4], a[5]);
	penv = g_env;
	kenv_reset();
	ref_sdt_probe_stub(id, a[0], a[1], a[2], a[3], a[4], a[5]);
	renv = g_env;
	std::snprintf(buf, sizeof(buf),
	    "id=%u pprintf=%d rprintf=%d pbt=%d rbt=%d ptext=\"%.60s\" "
	    "rtext=\"%.60s\"", id, penv.printf_calls, renv.printf_calls,
	    penv.backtrace_calls, renv.backtrace_calls, penv.text, renv.text);
	record(F_STUB, kenv_equal(penv, renv) && penv.printf_calls == 1 &&
	    penv.backtrace_calls == 1, "stub side effects", buf);
}

static void
test_sdt(void)
{
	static const std::uintptr_t interesting[] = {
		0, 1, 2, 3, 0x7f, 0x80, 0xff, 0x100, 0x7fffffffu, 0x80000000u,
		0xffffffffu, static_cast<std::uintptr_t>(-1),
		static_cast<std::uintptr_t>(-2),
		static_cast<std::uintptr_t>(0x8000000000000000ULL)
	};
	std::size_t ni = sizeof(interesting) / sizeof(interesting[0]);
	std::uintptr_t a[6];
	char buf[160];

	/* The default hook must be the stub, and probes must start off. */
	std::snprintf(buf, sizeof(buf), "port=%p ref=%p",
	    reinterpret_cast<void *>(k::sdt_probe_func),
	    reinterpret_cast<void *>(sdt_probe_func));
	record(F_STUB, reinterpret_cast<void *>(k::sdt_probe_func) ==
	    reinterpret_cast<void *>(&k::sdt_probe_stub) &&
	    reinterpret_cast<void *>(sdt_probe_func) ==
	    reinterpret_cast<void *>(&ref_sdt_probe_stub),
	    "default hook is stub", buf);
	std::snprintf(buf, sizeof(buf), "port=%d ref=%d",
	    static_cast<int>(k::sdt_probes_enabled),
	    static_cast<int>(sdt_probes_enabled));
	record(F_STUB, k::sdt_probes_enabled == sdt_probes_enabled &&
	    !k::sdt_probes_enabled, "probes_enabled initial", buf);

	/* Distinct value in every slot: any swap or substitution shows up. */
	for (std::size_t i = 0; i < 6; i++)
		a[i] = static_cast<std::uintptr_t>(0x1111111111111111ULL *
		    (i + 1));
	case_probe(0xdeadbeefu, a, true);
	case_probe(0xdeadbeefu, a, false);

	for (std::size_t i = 0; i < 6; i++)
		a[i] = 0;
	case_probe(0, a, true);
	for (std::size_t i = 0; i < 6; i++)
		a[i] = static_cast<std::uintptr_t>(-1);
	case_probe(0xffffffffu, a, true);
	for (std::size_t i = 0; i < 6; i++)
		a[i] = 1;
	case_probe(1, a, true);

	/* One interesting value at a time in each slot. */
	for (std::size_t slot = 0; slot < 6; slot++) {
		for (std::size_t v = 0; v < ni; v++) {
			for (std::size_t i = 0; i < 6; i++)
				a[i] = static_cast<std::uintptr_t>(
				    0x0101010101010101ULL * (i + 1));
			a[slot] = interesting[v];
			case_probe(static_cast<std::uint32_t>(v), a,
			    (v & 1) == 0);
		}
	}

	for (long it = 0; it < 200000; it++) {
		std::uint32_t id = static_cast<std::uint32_t>(rnd());

		for (std::size_t i = 0; i < 6; i++) {
			switch (rnd_below(3)) {
			case 0:
				a[i] = 0;
				break;
			case 1:
				a[i] = static_cast<std::uintptr_t>(
				    rnd_below(4));
				break;
			default:
				a[i] = static_cast<std::uintptr_t>(rnd());
				break;
			}
		}
		case_probe(id, a, rnd_below(8) != 0);
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	long total_cases = 0;
	long total_fails = 0;

	test_vdso();
	test_early();
	test_bufring();
	test_sdt();

	std::printf("\n");
	std::printf("%-28s %12s %10s %8s\n", "function", "cases", "failures",
	    "result");
	std::printf("%-28s %12s %10s %8s\n", "----------------------------",
	    "------------", "----------", "--------");
	for (int i = 0; i < F_COUNT; i++) {
		std::printf("%-28s %12ld %10ld %8s\n", f_name[i], f_cases[i],
		    f_fails[i], f_fails[i] == 0 ? "PASS" : "FAIL");
		total_cases += f_cases[i];
		total_fails += f_fails[i];
	}
	std::printf("%-28s %12s %10s %8s\n", "----------------------------",
	    "------------", "----------", "--------");
	std::printf("%-28s %12ld %10ld %8s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0 ? "PASS" : "FAIL");

	return (total_fails == 0 ? 0 : 1);
}
