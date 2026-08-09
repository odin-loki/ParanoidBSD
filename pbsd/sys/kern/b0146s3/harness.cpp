/*
 * PBSD batch b0146s3 -- differential test: port.cppm vs oracle.c.
 *
 * Both sides get their own private arena, their own malloc()/free() log and
 * their own panic hook, so that every observable effect of the kernel
 * routines can be compared:
 *
 *	- the return value, as an OFFSET into the caller's own arena
 *	  (never a raw address);
 *	- the requested allocation size, the M_* flags handed to malloc(),
 *	  the malloc_type pointer, and the free() calls;
 *	- whether the routine panicked (KASSERT is live, as in an INVARIANTS
 *	  kernel);
 *	- the *whole* arena, guard bytes included: both arenas are painted
 *	  with 0x7f before every call, so a table entry that is initialised
 *	  one element too far, or one element too few, shows up as a byte
 *	  difference past the nominal write window;
 *	- the u_long output parameter, which lives in the middle of a
 *	  three-word 0x7f-painted buffer so that a stray neighbouring write
 *	  is caught as well.
 */

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csetjmp>

import pbsd.sys.kern.b0146s3;

namespace port = pbsd::sys_kern::b0146s3;

/* ---------------------------------------------------------------- oracle */

extern "C" {

struct malloc_type {
	const char *ks_shortdesc;
};

void *ref_hashinit_flags(int elements, struct malloc_type *type,
    unsigned long *hashmask, int flags);
void *ref_hashinit(int elements, struct malloc_type *type,
    unsigned long *hashmask);
void ref_hashdestroy(void *vhashtbl, struct malloc_type *type,
    unsigned long hashmask);
void *ref_phashinit_flags(int elements, struct malloc_type *type,
    unsigned long *nentries, int flags);
void *ref_phashinit(int elements, struct malloc_type *type,
    unsigned long *nentries);
int ref_hash_mflags_probe(int flags);

}

/* ----------------------------------------------------------- environment */

static constexpr std::size_t PRE = 64;		/* guard before the table */
static constexpr std::size_t POST = 256;	/* guard after the table */
static constexpr std::size_t CAP = 320UL * 1024; /* largest servable size */
static constexpr std::size_t TOTAL = PRE + CAP + POST;
static constexpr unsigned char GUARD = 0x7f;

struct AllocEv {
	unsigned long size;
	int flags;
	int type_ok;
	long off;
};

struct FreeEv {
	long off;
	int type_ok;
};

struct Env {
	unsigned char *arena;
	void *type;
	int fail_alloc;
	int nalloc;
	int nfree;
	AllocEv al[4];
	FreeEv fr[4];
	volatile int panicked;
	std::jmp_buf jb;
	char pmsg[256];
};

alignas(16) static unsigned char arena_port[TOTAL];
alignas(16) static unsigned char arena_ref[TOTAL];

static Env g_port;
static Env g_ref;

static port::malloc_type mt_port = { "pbsdtest" };
static struct malloc_type mt_ref = { "pbsdtest" };

static void *
env_alloc(Env &e, unsigned long size, void *type, int flags)
{
	void *ret;

	if (e.fail_alloc != 0 || size > CAP)
		ret = nullptr;
	else
		ret = e.arena + PRE;
	if (e.nalloc < 4) {
		e.al[e.nalloc].size = size;
		e.al[e.nalloc].flags = flags;
		e.al[e.nalloc].type_ok = (type == e.type);
		e.al[e.nalloc].off = (ret == nullptr) ? -1 :
		    static_cast<long>(PRE);
	}
	e.nalloc++;
	return ret;
}

static void
env_free(Env &e, void *addr, void *type)
{

	if (e.nfree < 4) {
		e.fr[e.nfree].off = (addr == nullptr) ? -1 :
		    static_cast<long>(static_cast<unsigned char *>(addr) -
		    e.arena);
		e.fr[e.nfree].type_ok = (type == e.type);
	}
	e.nfree++;
}

[[noreturn]] static void
env_panic(Env &e, const char *fmt, va_list ap)
{

	std::vsnprintf(e.pmsg, sizeof(e.pmsg), fmt, ap);
	e.panicked = 1;
	std::longjmp(e.jb, 1);
}

extern "C" void *
pbsd_env_malloc(unsigned long size, void *type, int flags)
{

	return env_alloc(g_port, size, type, flags);
}

extern "C" void
pbsd_env_free(void *addr, void *type)
{

	env_free(g_port, addr, type);
}

extern "C" [[noreturn]] void
pbsd_env_panic(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	env_panic(g_port, fmt, ap);
}

extern "C" void *
ref_env_malloc(unsigned long size, struct malloc_type *type, int flags)
{

	return env_alloc(g_ref, size, static_cast<void *>(type), flags);
}

extern "C" void
ref_env_free(void *addr, struct malloc_type *type)
{

	env_free(g_ref, addr, static_cast<void *>(type));
}

extern "C" [[noreturn]] void
ref_env_panic(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	env_panic(g_ref, fmt, ap);
}

/* ------------------------------------------------------------- bookkeeping */

enum {
	F_MFLAGS,
	F_HASHINIT_FLAGS,
	F_HASHINIT,
	F_HASHDESTROY,
	F_PHASHINIT_FLAGS,
	F_PHASHINIT,
	F_NUM
};

static const char *const fnames[F_NUM] = {
	"hash_mflags",
	"hashinit_flags",
	"hashinit",
	"hashdestroy",
	"phashinit_flags",
	"phashinit"
};

static long g_cases[F_NUM];
static long g_fails[F_NUM];
static int g_shown[F_NUM];

static char g_reason[512];

static const char *
reasonf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	std::vsnprintf(g_reason, sizeof(g_reason), fmt, ap);
	va_end(ap);
	return g_reason;
}

static void
record(int fid, const char *why, const char *ctx)
{

	g_cases[fid]++;
	if (why == nullptr)
		return;
	g_fails[fid]++;
	if (g_shown[fid] < 5) {
		g_shown[fid]++;
		std::printf("FAIL %-16s %s   {%s}\n", fnames[fid], why, ctx);
	}
}

/* --------------------------------------------------------------- plumbing */

static std::size_t
clamp_need(unsigned long long b)
{
	std::size_t n;

	if (b > CAP)
		b = CAP;
	n = PRE + static_cast<std::size_t>(b) + POST;
	if (n > TOTAL)
		n = TOTAL;
	return n;
}

/*
 * Upper bound on the bytes hashinit_flags() can touch: hashsize is the
 * largest power of two <= elements, so 8 * elements + 8 always covers it.
 * Above 4 * CAP the allocation is certain to be refused (hashsize > CAP/8),
 * so nothing is written at all.
 */
static std::size_t
need_for_hash(long long elements)
{
	unsigned long long b;

	if (elements <= 0)
		return PRE + 64 + POST;
	b = static_cast<unsigned long long>(elements) * 8ULL + 8ULL;
	if (b > 4ULL * static_cast<unsigned long long>(CAP))
		return PRE + 64 + POST;
	return clamp_need(b);
}

/* phashinit_flags() never picks a hashsize above primes[NPRIMES - 1]. */
static std::size_t
need_for_phash(long long elements)
{
	unsigned long long b;
	const unsigned long long lim = 32749ULL * 8ULL + 8ULL;

	if (elements <= 0)
		return PRE + 64 + POST;
	b = static_cast<unsigned long long>(elements) * 8ULL + 8ULL;
	if (b > lim)
		b = lim;
	return clamp_need(b);
}

static void
reset_env(Env &e, int fail)
{

	e.fail_alloc = fail;
	e.nalloc = 0;
	e.nfree = 0;
	e.panicked = 0;
	std::memset(e.al, 0, sizeof(e.al));
	std::memset(e.fr, 0, sizeof(e.fr));
	e.pmsg[0] = '\0';
}

static void
prep(std::size_t need, int fail)
{

	std::memset(arena_port, GUARD, need);
	std::memset(arena_ref, GUARD, need);
	reset_env(g_port, fail);
	reset_env(g_ref, fail);
}

static long
off_of(const Env &e, void *p)
{

	if (p == nullptr)
		return -1;
	return static_cast<long>(static_cast<unsigned char *>(p) - e.arena);
}

/*
 * Compare every observable: panic, logs, the output word buffer and the
 * whole painted arena region.  Returns nullptr when the two sides agree.
 */
static const char *
cmp_all(std::size_t need, const void *outp, const void *outr,
    std::size_t outsz)
{
	int k;

	if (g_port.panicked != g_ref.panicked)
		return reasonf("panic mismatch: port=%d ref=%d (port msg "
		    "\"%s\", ref msg \"%s\")", g_port.panicked,
		    g_ref.panicked, g_port.pmsg, g_ref.pmsg);
	if (g_port.nalloc != g_ref.nalloc)
		return reasonf("malloc count: port=%d ref=%d", g_port.nalloc,
		    g_ref.nalloc);
	if (g_port.nfree != g_ref.nfree)
		return reasonf("free count: port=%d ref=%d", g_port.nfree,
		    g_ref.nfree);
	for (k = 0; k < g_port.nalloc && k < 4; k++) {
		if (g_port.al[k].size != g_ref.al[k].size)
			return reasonf("malloc[%d] size: port=%lu ref=%lu", k,
			    g_port.al[k].size, g_ref.al[k].size);
		if (g_port.al[k].flags != g_ref.al[k].flags)
			return reasonf("malloc[%d] flags: port=0x%x ref=0x%x",
			    k, g_port.al[k].flags, g_ref.al[k].flags);
		if (g_port.al[k].type_ok != g_ref.al[k].type_ok)
			return reasonf("malloc[%d] type: port=%d ref=%d", k,
			    g_port.al[k].type_ok, g_ref.al[k].type_ok);
		if (g_port.al[k].off != g_ref.al[k].off)
			return reasonf("malloc[%d] result: port=%ld ref=%ld",
			    k, g_port.al[k].off, g_ref.al[k].off);
	}
	for (k = 0; k < g_port.nfree && k < 4; k++) {
		if (g_port.fr[k].off != g_ref.fr[k].off)
			return reasonf("free[%d] offset: port=%ld ref=%ld", k,
			    g_port.fr[k].off, g_ref.fr[k].off);
		if (g_port.fr[k].type_ok != g_ref.fr[k].type_ok)
			return reasonf("free[%d] type: port=%d ref=%d", k,
			    g_port.fr[k].type_ok, g_ref.fr[k].type_ok);
	}
	if (outsz != 0 && std::memcmp(outp, outr, outsz) != 0) {
		const unsigned long *a = static_cast<const unsigned long *>(outp);
		const unsigned long *b = static_cast<const unsigned long *>(outr);
		std::size_t n = outsz / sizeof(unsigned long);

		for (std::size_t j = 0; j < n; j++)
			if (a[j] != b[j])
				return reasonf("out word %zu: port=0x%lx "
				    "ref=0x%lx", j, a[j], b[j]);
		return reasonf("out buffer differs");
	}
	if (std::memcmp(arena_port, arena_ref, need) != 0) {
		std::size_t j;

		for (j = 0; j < need; j++)
			if (arena_port[j] != arena_ref[j])
				break;
		return reasonf("arena byte %zu of %zu (table starts at %zu): "
		    "port=0x%02x ref=0x%02x", j, need, PRE,
		    arena_port[j], arena_ref[j]);
	}
	return nullptr;
}

/* ------------------------------------------------------------------ cases */

static void
case_mflags(int flags)
{
	int a, b;
	char ctx[64];

	a = port::hash_mflags(flags);
	b = ref_hash_mflags_probe(flags);
	std::snprintf(ctx, sizeof(ctx), "flags=0x%x", flags);
	record(F_MFLAGS, a == b ? nullptr :
	    reasonf("return: port=0x%x ref=0x%x", a, b), ctx);
}

/* wrapper != 0 drives hashinit()/phashinit(), which force HASH_WAITOK. */
static void
case_hash(int elements, int flags, int fail, int wrapper)
{
	const int fid = wrapper ? F_HASHINIT : F_HASHINIT_FLAGS;
	std::size_t need = need_for_hash(elements);
	unsigned long outp[3], outr[3];
	void *volatile rp = nullptr;
	void *volatile rr = nullptr;
	const char *why;
	char ctx[128];

	prep(need, fail);
	std::memset(outp, GUARD, sizeof(outp));
	std::memset(outr, GUARD, sizeof(outr));

	if (setjmp(g_port.jb) == 0) {
		if (wrapper)
			rp = port::hashinit(elements, &mt_port, &outp[1]);
		else
			rp = port::hashinit_flags(elements, &mt_port, &outp[1],
			    flags);
	}
	if (setjmp(g_ref.jb) == 0) {
		if (wrapper)
			rr = ref_hashinit(elements, &mt_ref, &outr[1]);
		else
			rr = ref_hashinit_flags(elements, &mt_ref, &outr[1],
			    flags);
	}

	std::snprintf(ctx, sizeof(ctx), "elements=%d flags=0x%x failalloc=%d",
	    elements, wrapper ? port::HASH_WAITOK : flags, fail);

	why = nullptr;
	if (g_port.panicked == 0 && g_ref.panicked == 0) {
		long op = off_of(g_port, rp);
		long orf = off_of(g_ref, rr);

		if (op != orf)
			why = reasonf("return offset: port=%ld ref=%ld", op,
			    orf);
	}
	if (why == nullptr)
		why = cmp_all(need, outp, outr, sizeof(outp));
	record(fid, why, ctx);
}

static void
case_phash(int elements, int flags, int fail, int wrapper)
{
	const int fid = wrapper ? F_PHASHINIT : F_PHASHINIT_FLAGS;
	std::size_t need = need_for_phash(elements);
	unsigned long outp[3], outr[3];
	void *volatile rp = nullptr;
	void *volatile rr = nullptr;
	const char *why;
	char ctx[128];

	prep(need, fail);
	std::memset(outp, GUARD, sizeof(outp));
	std::memset(outr, GUARD, sizeof(outr));

	if (setjmp(g_port.jb) == 0) {
		if (wrapper)
			rp = port::phashinit(elements, &mt_port, &outp[1]);
		else
			rp = port::phashinit_flags(elements, &mt_port,
			    &outp[1], flags);
	}
	if (setjmp(g_ref.jb) == 0) {
		if (wrapper)
			rr = ref_phashinit(elements, &mt_ref, &outr[1]);
		else
			rr = ref_phashinit_flags(elements, &mt_ref, &outr[1],
			    flags);
	}

	std::snprintf(ctx, sizeof(ctx), "elements=%d flags=0x%x failalloc=%d",
	    elements, wrapper ? port::HASH_WAITOK : flags, fail);

	why = nullptr;
	if (g_port.panicked == 0 && g_ref.panicked == 0) {
		long op = off_of(g_port, rp);
		long orf = off_of(g_ref, rr);

		if (op != orf)
			why = reasonf("return offset: port=%ld ref=%ld", op,
			    orf);
	}
	if (why == nullptr)
		why = cmp_all(need, outp, outr, sizeof(outp));
	record(fid, why, ctx);
}

/*
 * hashdestroy(): build a real table with hashinit_flags(), optionally poison
 * one head so that LIST_EMPTY() fails there, then destroy it.  maskkind
 * walks the mask across the end of the table, so both the last in-range head
 * and the first out-of-range head (which the 0x7f paint makes "not empty")
 * are visited.
 */
static void
case_destroy(int elements, int flags, int plantkind, unsigned plantrnd,
    int maskkind)
{
	std::size_t need = need_for_hash(elements);
	unsigned long outp[3], outr[3];
	void *volatile tp = nullptr;
	void *volatile tr = nullptr;
	unsigned long hs, mask;
	long plant;
	const char *why;
	char ctx[160];

	prep(need, 0);
	std::memset(outp, GUARD, sizeof(outp));
	std::memset(outr, GUARD, sizeof(outr));

	if (setjmp(g_port.jb) == 0)
		tp = port::hashinit_flags(elements, &mt_port, &outp[1], flags);
	if (setjmp(g_ref.jb) == 0)
		tr = ref_hashinit_flags(elements, &mt_ref, &outr[1], flags);
	if (g_port.panicked != 0 || g_ref.panicked != 0 || tp == nullptr ||
	    tr == nullptr)
		return;			/* not a usable hashdestroy() case */
	if (g_port.al[0].size != g_ref.al[0].size) {
		record(F_HASHDESTROY, reasonf("setup malloc size: port=%lu "
		    "ref=%lu", g_port.al[0].size, g_ref.al[0].size), "setup");
		return;
	}

	hs = g_port.al[0].size / sizeof(void *);
	switch (maskkind) {
	case 0:
		mask = 0;
		break;
	case 1:
		mask = hs / 2;
		break;
	case 2:
		mask = hs - 1;
		break;
	case 3:
		mask = hs;
		break;
	default:
		mask = hs + 2;
		break;
	}

	switch (plantkind) {
	case 0:
		plant = -1;
		break;
	case 1:
		plant = 0;
		break;
	case 2:
		plant = static_cast<long>(hs / 2);
		break;
	case 3:
		plant = static_cast<long>(hs - 1);
		break;
	case 4:
		plant = static_cast<long>(hs);
		break;
	default:
		plant = static_cast<long>(plantrnd % (hs + 1));
		break;
	}
	if (plant >= 0 &&
	    PRE + static_cast<std::size_t>(plant) * sizeof(void *) +
	    sizeof(void *) <= need) {
		unsigned long fake = 0xdeadbeefUL;
		std::size_t at = PRE +
		    static_cast<std::size_t>(plant) * sizeof(void *);

		std::memcpy(arena_port + at, &fake, sizeof(fake));
		std::memcpy(arena_ref + at, &fake, sizeof(fake));
	} else {
		plant = -1;
	}

	if (setjmp(g_port.jb) == 0)
		port::hashdestroy(tp, &mt_port, mask);
	if (setjmp(g_ref.jb) == 0)
		ref_hashdestroy(tr, &mt_ref, mask);

	std::snprintf(ctx, sizeof(ctx), "elements=%d flags=0x%x hashsize=%lu "
	    "hashmask=%lu plant=%ld", elements, flags, hs, mask, plant);
	why = cmp_all(need, outp, outr, sizeof(outp));
	record(F_HASHDESTROY, why, ctx);
}

/* --------------------------------------------------------------- fixtures */

static const int elem_cases[] = {
	-2147483647 - 1, -1000000, -65537, -4096, -3, -2, -1, 0,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
	30, 31, 32, 33, 60, 61, 62, 63, 64, 65,
	126, 127, 128, 129, 250, 251, 252, 255, 256, 257,
	508, 509, 510, 511, 512, 513, 760, 761, 762,
	1020, 1021, 1022, 1023, 1024, 1025, 1530, 1531, 1532,
	2038, 2039, 2040, 2047, 2048, 2049, 2556, 2557, 2558,
	3066, 3067, 3068, 3582, 3583, 3584, 4092, 4093, 4094, 4095, 4096, 4097,
	4602, 4603, 4604, 5118, 5119, 5120, 5622, 5623, 5624,
	6142, 6143, 6144, 6652, 6653, 6654, 7158, 7159, 7160,
	7672, 7673, 7674, 8190, 8191, 8192, 8193,
	12280, 12281, 12282, 16380, 16381, 16382, 16383, 16384, 16385,
	24570, 24571, 24572, 32747, 32748, 32749, 32750, 32767, 32768, 32769,
	40959, 40960, 40961, 65535, 65536, 65537, 131072, 1000000,
	2147483646, 2147483647
};

static const int flag_cases[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	0x10, 0x11, 0x12, 0x13, 0x100, 0x101, 0x102, 0x103,
	-1, -2, -3, -4, 0x7ffffffe, 0x7fffffff, -2147483647 - 1
};

static const int primes_tab[] = { 1, 13, 31, 61, 127, 251, 509, 761, 1021,
	1531, 2039, 2557, 3067, 3583, 4093, 4603, 5119, 5623, 6143, 6653,
	7159, 7673, 8191, 12281, 16381, 24571, 32749 };

/* ------------------------------------------------------------------- rng */

static std::uint64_t rng_state = 0x0146533146505342ULL;

static std::uint64_t
rnd(void)
{
	std::uint64_t z;

	z = (rng_state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

static int
rnd_elements(void)
{
	std::uint64_t sel = rnd() % 64;
	int k;

	if (sel < 12)
		return 1 + static_cast<int>(rnd() % 8);
	if (sel < 24)
		return 1 + static_cast<int>(rnd() % 64);
	if (sel < 34)
		return 1 + static_cast<int>(rnd() % 512);
	if (sel < 40)
		return 1 + static_cast<int>(rnd() % 4096);
	if (sel < 48) {
		k = static_cast<int>(rnd() % 13);
		return (1 << k) + static_cast<int>(rnd() % 5) - 2;
	}
	if (sel < 56) {
		k = static_cast<int>(rnd() % 23);
		return primes_tab[k] + static_cast<int>(rnd() % 5) - 2;
	}
	if (sel < 58) {
		k = 23 + static_cast<int>(rnd() % 4);
		return primes_tab[k] + static_cast<int>(rnd() % 5) - 2;
	}
	if (sel < 60)
		return 1 + static_cast<int>(rnd() % 40000);
	if (sel < 62)
		return -static_cast<int>(rnd() % 8);
	return static_cast<int>(static_cast<std::uint32_t>(rnd()));
}

static int
rnd_elements_small(void)
{
	std::uint64_t sel = rnd() % 32;
	int k;

	if (sel < 10)
		return 1 + static_cast<int>(rnd() % 4);
	if (sel < 20)
		return 1 + static_cast<int>(rnd() % 40);
	if (sel < 26)
		return 1 + static_cast<int>(rnd() % 300);
	if (sel < 30) {
		k = static_cast<int>(rnd() % 11);
		return (1 << k) + static_cast<int>(rnd() % 3);
	}
	return 1 + static_cast<int>(rnd() % 2000);
}

static int
rnd_flags(void)
{
	std::uint64_t sel = rnd() % 8;

	if (sel < 4)
		return flag_cases[rnd() % (sizeof(flag_cases) /
		    sizeof(flag_cases[0]))];
	if (sel < 6)
		return static_cast<int>(rnd() % 16);
	return static_cast<int>(static_cast<std::uint32_t>(rnd()));
}

/* Flag combinations that pass the KASSERT, so hashdestroy() gets a table. */
static const int good_flags[] = { 1, 2, 3, 5, 6, 7, 0x11, 0x12, 0x13 };

/* ------------------------------------------------------------------- main */

static const long SWEEP = 200000;

int
main(void)
{
	const int nelem = static_cast<int>(sizeof(elem_cases) /
	    sizeof(elem_cases[0]));
	const int nflag = static_cast<int>(sizeof(flag_cases) /
	    sizeof(flag_cases[0]));
	const int ngood = static_cast<int>(sizeof(good_flags) /
	    sizeof(good_flags[0]));
	long i;
	int a, b, f;
	long total_cases = 0, total_fails = 0;

	g_port.arena = arena_port;
	g_ref.arena = arena_ref;
	g_port.type = &mt_port;
	g_ref.type = &mt_ref;

	/* ---- hash_mflags: every low bit pattern, every single bit, edges. */
	for (a = 0; a < 256; a++)
		case_mflags(a);
	for (a = 0; a < 32; a++) {
		case_mflags(1 << a);
		case_mflags(~(1 << a));
		case_mflags((1 << a) | 1);
		case_mflags((1 << a) & ~1);
	}
	case_mflags(0);
	case_mflags(-1);
	case_mflags(2147483647);
	case_mflags(-2147483647 - 1);
	for (i = 0; i < SWEEP; i++)
		case_mflags(static_cast<int>(
		    static_cast<std::uint32_t>(rnd())));

	/* ---- hashinit_flags / hashinit: full edge grid. */
	for (a = 0; a < nelem; a++)
		for (b = 0; b < nflag; b++) {
			case_hash(elem_cases[a], flag_cases[b], 0, 0);
			case_hash(elem_cases[a], flag_cases[b], 1, 0);
		}
	for (a = 0; a < nelem; a++) {
		case_hash(elem_cases[a], 0, 0, 1);
		case_hash(elem_cases[a], 0, 1, 1);
	}

	/* ---- phashinit_flags / phashinit: full edge grid. */
	for (a = 0; a < nelem; a++)
		for (b = 0; b < nflag; b++) {
			case_phash(elem_cases[a], flag_cases[b], 0, 0);
			case_phash(elem_cases[a], flag_cases[b], 1, 0);
		}
	for (a = 0; a < nelem; a++) {
		case_phash(elem_cases[a], 0, 0, 1);
		case_phash(elem_cases[a], 0, 1, 1);
	}

	/* ---- hashdestroy: every mask/plant combination on many sizes. */
	for (a = 0; a < nelem; a++)
		for (b = 0; b < 5; b++)
			for (f = 0; f < 5; f++)
				case_destroy(elem_cases[a],
				    good_flags[(a + b + f) % ngood], f, 0, b);

	/* ---- randomised sweeps. */
	for (i = 0; i < SWEEP; i++)
		case_hash(rnd_elements(), rnd_flags(),
		    (rnd() % 8) == 0 ? 1 : 0, 0);
	for (i = 0; i < SWEEP; i++)
		case_hash(rnd_elements(), 0, (rnd() % 8) == 0 ? 1 : 0, 1);
	for (i = 0; i < SWEEP; i++)
		case_phash(rnd_elements(), rnd_flags(),
		    (rnd() % 8) == 0 ? 1 : 0, 0);
	for (i = 0; i < SWEEP; i++)
		case_phash(rnd_elements(), 0, (rnd() % 8) == 0 ? 1 : 0, 1);
	for (i = 0; i < SWEEP; i++)
		case_destroy(rnd_elements_small(),
		    good_flags[rnd() % ngood],
		    static_cast<int>(rnd() % 6),
		    static_cast<unsigned>(rnd()),
		    static_cast<int>(rnd() % 5));

	std::printf("\n%-18s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-18s %12s %12s\n", "------------------",
	    "------------", "------------");
	for (a = 0; a < F_NUM; a++) {
		std::printf("%-18s %12ld %12ld\n", fnames[a], g_cases[a],
		    g_fails[a]);
		total_cases += g_cases[a];
		total_fails += g_fails[a];
	}
	std::printf("%-18s %12s %12s\n", "------------------",
	    "------------", "------------");
	std::printf("%-18s %12ld %12ld\n", "TOTAL", total_cases, total_fails);
	std::printf("\n%s\n", total_fails == 0 ? "PASS" : "FAIL");
	return total_fails == 0 ? 0 : 1;
}
