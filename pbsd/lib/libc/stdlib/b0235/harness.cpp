/*
 * harness.cpp -- differential test for PBSD batch b0235.
 *
 * Every case is run twice: once through the C++23 port and once through the
 * unmodified C oracle.  For the qsort family we compare
 *   - the entire destination buffer, guard bytes included,
 *   - the full trace of comparator invocations (element OFFSETS from the
 *     buffer base, never raw addresses, plus the value each call returned),
 *     which pins down pivot choice and partition order, not just the final
 *     permutation,
 *   - the thunk pointer seen by the comparator,
 *   - the return value and the constraint-handler diagnostic (qsort_s).
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <cstdarg>

import pbsd.lib.libc.stdlib.b0235;

namespace P = pbsd::lib_libc_stdlib::b0235;

extern "C" {
size_t ref_memalignment(const void *p);
void ref_qsort_r(void *a, size_t n, size_t es,
    int (*cmp)(const void *, const void *, void *), void *thunk);
void ref___qsort_r_compat(void *a, size_t n, size_t es, void *thunk,
    int (*cmp)(void *, const void *, const void *));
int ref_qsort_s(void *a, size_t n, size_t es,
    int (*cmp)(const void *, const void *, void *), void *thunk);
extern char ref_ch_msg[256];
extern int ref_ch_error;
extern unsigned long ref_ch_count;
}

/* ------------------------------------------------------------------ */
/* bookkeeping                                                        */
/* ------------------------------------------------------------------ */

struct Stat {
	const char *name;
	unsigned long cases;
	unsigned long fails;
};

static Stat st_memalignment = { "memalignment", 0, 0 };
static Stat st_qsort_r = { "qsort_r", 0, 0 };
static Stat st_qsort_r_compat = { "__qsort_r_compat", 0, 0 };
static Stat st_qsort_s = { "qsort_s", 0, 0 };

static unsigned long reported;

static void
fail(Stat &s, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

static void
fail(Stat &s, const char *fmt, ...)
{
	va_list ap;

	s.fails++;
	if (reported < 25) {
		reported++;
		std::fprintf(stderr, "FAIL [%s] ", s.name);
		va_start(ap, fmt);
		std::vfprintf(stderr, fmt, ap);
		va_end(ap);
		std::fputc('\n', stderr);
	}
}

/* ------------------------------------------------------------------ */
/* deterministic PRNG (fixed seed, no libstdc++ dependency)           */
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state;

static void
rng_seed(std::uint64_t s)
{
	rng_state = s ? s : 0x9e3779b97f4a7c15ull;
}

static std::uint64_t
rng_next(void)
{
	std::uint64_t x = rng_state;

	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rng_state = x;
	return (x * 0x2545f4914f6cdd1dull);
}

static std::uint32_t
rng_below(std::uint32_t bound)
{
	return (bound == 0 ? 0 : (std::uint32_t)(rng_next() % bound));
}

/* ------------------------------------------------------------------ */
/* memalignment                                                       */
/* ------------------------------------------------------------------ */

static void
check_memalignment(std::uintptr_t v)
{
	const void *p = (const void *)v;
	size_t got, want;

	st_memalignment.cases++;
	got = P::memalignment(p);
	want = ref_memalignment(p);
	if (got != want)
		fail(st_memalignment, "p=0x%llx port=%llu ref=%llu",
		    (unsigned long long)v, (unsigned long long)got,
		    (unsigned long long)want);
}

static void
test_memalignment(void)
{
	static const std::uintptr_t fixed[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 16, 17, 24, 31, 32,
		33, 48, 63, 64, 96, 127, 128, 129, 192, 255, 256, 257, 384,
		511, 512, 1023, 1024, 4095, 4096, 4097, 8192, 65535, 65536,
		0x7f, 0x80, 0x81, 0xfe, 0xff, 0x100, 0x7fff, 0x8000, 0x8001,
		0xffff, 0x7fffffffu, 0x80000000u, 0x80000001u, 0xffffffffu,
		0x100000000ull, 0xfffffffffffffffeull, 0xffffffffffffffffull,
		0x8000000000000000ull, 0x8000000000000001ull,
		0x4000000000000000ull, 0xdeadbeefull, 0xdeadbee0ull,
		0xcafebabe00000000ull, (std::uintptr_t)SIZE_MAX,
		(std::uintptr_t)SIZE_MAX - 1, (std::uintptr_t)SIZE_MAX / 2,
		((std::uintptr_t)SIZE_MAX >> 1) + 1,
	};
	unsigned i;

	for (i = 0; i < sizeof(fixed) / sizeof(fixed[0]); i++)
		check_memalignment(fixed[i]);

	/* every single-bit value, and that value with the low bit set */
	for (i = 0; i < 8 * sizeof(std::uintptr_t); i++) {
		std::uintptr_t bit = (std::uintptr_t)1 << i;

		check_memalignment(bit);
		check_memalignment(bit | 1);
		check_memalignment(bit - 1);
		check_memalignment(~bit);
	}

	/* real object addresses */
	{
		char stackbuf[64];
		void *heap = std::malloc(64);

		for (i = 0; i < sizeof(stackbuf); i++)
			check_memalignment((std::uintptr_t)&stackbuf[i]);
		if (heap != nullptr) {
			for (i = 0; i < 64; i++)
				check_memalignment(
				    (std::uintptr_t)((char *)heap + i));
			std::free(heap);
		}
		check_memalignment((std::uintptr_t)(void *)&st_memalignment);
		check_memalignment((std::uintptr_t)(void *)ref_memalignment);
	}

	rng_seed(0xb0235a11ull);
	for (unsigned long it = 0; it < 250000; it++) {
		std::uintptr_t v = (std::uintptr_t)rng_next();

		switch (it % 4) {
		case 0:
			break;
		case 1:
			/* force a known number of trailing zero bits */
			v <<= (rng_next() % (8 * sizeof(std::uintptr_t)));
			break;
		case 2:
			v &= 0xffff;
			break;
		case 3:
			v |= (std::uintptr_t)1
			    << (8 * sizeof(std::uintptr_t) - 1);
			break;
		}
		check_memalignment(v);
	}
}

/* ------------------------------------------------------------------ */
/* comparator plumbing shared by the qsort variants                   */
/* ------------------------------------------------------------------ */

struct Trace {
	static const size_t CAP = 1u << 18;
	size_t n;
	bool overflow;
	bool thunk_bad;
	std::int32_t ax[CAP];
	std::int32_t bx[CAP];
	std::int16_t res[CAP];
};

static Trace tr_port;
static Trace tr_ref;

static const unsigned CTX_MAGIC = 0x5ab3c0deu;

struct Ctx {
	unsigned magic;
	const unsigned char *base;
	size_t es;
	int mode;
	Trace *tr;
};

static void
trace_reset(Trace *t)
{
	t->n = 0;
	t->overflow = false;
	t->thunk_bad = false;
}

static int
cmp_core(const unsigned char *x, const unsigned char *y, size_t es, int mode)
{
	int r;

	switch (mode) {
	case 0:
		r = (es != 0 ? std::memcmp(x, y, es) : 0);
		return (r < 0 ? -1 : (r > 0 ? 1 : 0));
	case 1:
		/* signed first byte: exercises high-bit bytes 0x80-0xff */
		return (es != 0 ? (int)(signed char)x[0] - (int)(signed char)y[0]
		    : 0);
	case 2:
		r = (es != 0 ? std::memcmp(x, y, es) : 0);
		return (r < 0 ? 1 : (r > 0 ? -1 : 0));
	case 3:
		/* everything compares equal */
		return (0);
	case 4:
		/* very coarse: forces long runs of ties */
		return (es != 0 ? (int)(x[0] & 3u) - (int)(y[0] & 3u) : 0);
	case 5:
		return (es != 0 ? (int)x[0] - (int)y[0] : 0);
	case 6:
		return (es != 0 ? (int)x[es - 1] - (int)y[es - 1] : 0);
	default:
		return (0);
	}
}

static void
trace_push(Trace *t, std::ptrdiff_t a, std::ptrdiff_t b, int r)
{
	if (t->n >= Trace::CAP) {
		t->overflow = true;
		return;
	}
	t->ax[t->n] = (std::int32_t)a;
	t->bx[t->n] = (std::int32_t)b;
	t->res[t->n] = (std::int16_t)r;
	t->n++;
}

extern "C" int
cmp_xy_thunk(const void *x, const void *y, void *th)
{
	Ctx *c = (Ctx *)th;
	int r;

	if (c == nullptr || c->magic != CTX_MAGIC) {
		if (c != nullptr && c->tr != nullptr)
			c->tr->thunk_bad = true;
		return (0);
	}
	r = cmp_core((const unsigned char *)x, (const unsigned char *)y, c->es,
	    c->mode);
	trace_push(c->tr, (const unsigned char *)x - c->base,
	    (const unsigned char *)y - c->base, r);
	return (r);
}

extern "C" int
cmp_thunk_xy(void *th, const void *x, const void *y)
{
	return (cmp_xy_thunk(x, y, th));
}

static bool
traces_equal(Stat &s, const char *tag)
{
	if (tr_port.n != tr_ref.n) {
		fail(s, "%s: comparator call count port=%zu ref=%zu", tag,
		    tr_port.n, tr_ref.n);
		return (false);
	}
	if (tr_port.overflow != tr_ref.overflow ||
	    tr_port.thunk_bad != tr_ref.thunk_bad) {
		fail(s, "%s: trace flags port=%d/%d ref=%d/%d", tag,
		    (int)tr_port.overflow, (int)tr_port.thunk_bad,
		    (int)tr_ref.overflow, (int)tr_ref.thunk_bad);
		return (false);
	}
	if (tr_port.thunk_bad) {
		fail(s, "%s: comparator received a bad thunk", tag);
		return (false);
	}
	for (size_t i = 0; i < tr_port.n; i++) {
		if (tr_port.ax[i] != tr_ref.ax[i] ||
		    tr_port.bx[i] != tr_ref.bx[i] ||
		    tr_port.res[i] != tr_ref.res[i]) {
			fail(s,
			    "%s: comparison #%zu port=(%d,%d)->%d "
			    "ref=(%d,%d)->%d", tag, i, (int)tr_port.ax[i],
			    (int)tr_port.bx[i], (int)tr_port.res[i],
			    (int)tr_ref.ax[i], (int)tr_ref.bx[i],
			    (int)tr_ref.res[i]);
			return (false);
		}
	}
	return (true);
}

static const size_t PAD = 32;
static const unsigned char GUARD = 0x7f;

static bool
buffers_equal(Stat &s, const char *tag, const unsigned char *pb,
    const unsigned char *rb, size_t total)
{
	for (size_t i = 0; i < total; i++) {
		if (pb[i] != rb[i]) {
			fail(s, "%s: byte %zu of %zu port=0x%02x ref=0x%02x",
			    tag, i, total, pb[i], rb[i]);
			return (false);
		}
	}
	return (true);
}

/* ------------------------------------------------------------------ */
/* qsort_r / __qsort_r_compat / qsort_s sort cases                    */
/* ------------------------------------------------------------------ */

enum Variant { V_R = 0, V_RC = 1, V_S = 2 };

static Stat &
variant_stat(int v)
{
	return (v == V_R ? st_qsort_r
	    : (v == V_RC ? st_qsort_r_compat : st_qsort_s));
}

static void
run_sort_case(int variant, const unsigned char *input, size_t n, size_t es,
    int mode, const char *tag)
{
	Stat &s = variant_stat(variant);
	size_t body = n * es;
	size_t total = body + PAD;
	unsigned char *pbuf = (unsigned char *)std::malloc(total);
	unsigned char *rbuf = (unsigned char *)std::malloc(total);
	Ctx pctx, rctx;
	int prc = 0, rrc = 0;

	s.cases++;
	if (pbuf == nullptr || rbuf == nullptr) {
		fail(s, "%s: out of memory", tag);
		std::free(pbuf);
		std::free(rbuf);
		return;
	}
	std::memset(pbuf, GUARD, total);
	std::memset(rbuf, GUARD, total);
	if (body != 0) {
		std::memcpy(pbuf, input, body);
		std::memcpy(rbuf, input, body);
	}

	trace_reset(&tr_port);
	trace_reset(&tr_ref);

	pctx.magic = CTX_MAGIC;
	pctx.base = pbuf;
	pctx.es = es;
	pctx.mode = mode;
	pctx.tr = &tr_port;
	rctx = pctx;
	rctx.base = rbuf;
	rctx.tr = &tr_ref;

	P::constraint_handler_reset();
	ref_ch_count = 0;
	ref_ch_error = 0;
	ref_ch_msg[0] = '\0';

	switch (variant) {
	case V_R:
		P::qsort_r(pbuf, n, es, cmp_xy_thunk, &pctx);
		ref_qsort_r(rbuf, n, es, cmp_xy_thunk, &rctx);
		break;
	case V_RC:
		P::__qsort_r_compat(pbuf, n, es, &pctx, cmp_thunk_xy);
		ref___qsort_r_compat(rbuf, n, es, &rctx, cmp_thunk_xy);
		break;
	default:
		prc = P::qsort_s(pbuf, n, es, cmp_xy_thunk, &pctx);
		rrc = ref_qsort_s(rbuf, n, es, cmp_xy_thunk, &rctx);
		break;
	}

	if (prc != rrc) {
		fail(s, "%s: return port=%d ref=%d", tag, prc, rrc);
		goto out;
	}
	if (variant == V_S) {
		if (P::constraint_handler_count() != ref_ch_count) {
			fail(s, "%s: handler count port=%lu ref=%lu", tag,
			    P::constraint_handler_count(), ref_ch_count);
			goto out;
		}
		if (P::constraint_handler_error() != ref_ch_error) {
			fail(s, "%s: handler errno port=%d ref=%d", tag,
			    P::constraint_handler_error(), ref_ch_error);
			goto out;
		}
		if (std::strcmp(P::constraint_handler_msg(), ref_ch_msg) != 0) {
			fail(s, "%s: handler msg port=\"%s\" ref=\"%s\"", tag,
			    P::constraint_handler_msg(), ref_ch_msg);
			goto out;
		}
	}
	if (!buffers_equal(s, tag, pbuf, rbuf, total))
		goto out;
	(void)traces_equal(s, tag);
out:
	std::free(pbuf);
	std::free(rbuf);
}

static void
run_sort_case_all(const unsigned char *input, size_t n, size_t es, int mode,
    const char *tag)
{
	run_sort_case(V_R, input, n, es, mode, tag);
	run_sort_case(V_RC, input, n, es, mode, tag);
	run_sort_case(V_S, input, n, es, mode, tag);
}

/* ------------------------------------------------------------------ */
/* hand written sort cases                                            */
/* ------------------------------------------------------------------ */

static const size_t MAXN = 400;
static const size_t MAXES = 16;
static unsigned char scratch[MAXN * MAXES];

static void
fill_pattern(unsigned char *buf, size_t body, int pattern, size_t es)
{
	size_t i;

	switch (pattern) {
	case 0:				/* ascending */
		for (i = 0; i < body; i++)
			buf[i] = (unsigned char)(i / (es ? es : 1));
		break;
	case 1:				/* descending */
		for (i = 0; i < body; i++)
			buf[i] = (unsigned char)(255 - i / (es ? es : 1));
		break;
	case 2:				/* all NUL */
		std::memset(buf, 0, body);
		break;
	case 3:				/* all 0xff */
		std::memset(buf, 0xff, body);
		break;
	case 4:				/* alternating 0x00 / 0x80 */
		for (i = 0; i < body; i++)
			buf[i] = (unsigned char)((i / (es ? es : 1)) & 1 ? 0x80
			    : 0x00);
		break;
	case 5:				/* high-bit bytes only */
		for (i = 0; i < body; i++)
			buf[i] = (unsigned char)(0x80 + (i & 0x7f));
		break;
	case 6:				/* NUL heavy with occasional 0xff */
		std::memset(buf, 0, body);
		for (i = 0; i < body; i += 5)
			buf[i] = 0xff;
		break;
	case 7:				/* two distinct values */
		for (i = 0; i < body; i++)
			buf[i] = (unsigned char)((i / (es ? es : 1)) % 2 ? 0xfe
			    : 0x01);
		break;
	case 8:				/* every byte value cycling */
		for (i = 0; i < body; i++)
			buf[i] = (unsigned char)(i * 37u + 11u);
		break;
	default:			/* organ pipe */
		for (i = 0; i < body; i++) {
			size_t k = i / (es ? es : 1);

			buf[i] = (unsigned char)(k < 128 ? k : 255 - k);
		}
		break;
	}
}

static void
test_sorts_fixed(void)
{
	static const size_t interesting_n[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 13, 14, 15, 16, 17,
		39, 40, 41, 42, 43, 63, 64, 65, 80, 81, 127, 128, 200, 333,
	};
	static const size_t interesting_es[] = { 1, 2, 3, 4, 5, 7, 8, 16 };
	char tag[96];

	for (unsigned ni = 0; ni < sizeof(interesting_n) / sizeof(size_t);
	    ni++) {
		size_t n = interesting_n[ni];

		for (unsigned ei = 0; ei < sizeof(interesting_es) / sizeof(size_t);
		    ei++) {
			size_t es = interesting_es[ei];

			if (n * es > sizeof(scratch))
				continue;
			for (int pat = 0; pat < 10; pat++) {
				for (int mode = 0; mode < 7; mode++) {
					fill_pattern(scratch, n * es, pat, es);
					std::snprintf(tag, sizeof(tag),
					    "n=%zu es=%zu pat=%d mode=%d", n,
					    es, pat, mode);
					run_sort_case_all(scratch, n, es, mode,
					    tag);
				}
			}
		}
	}

	/*
	 * es == 0 is only exercised with n < 7.  With n >= 7 the original
	 * algorithm reaches swapfunc(a, pm, 0), whose do/while underflows
	 * size_t and runs away; that is a property of the C source, not of
	 * the port, and both sides would hang identically.
	 */
	for (size_t n = 0; n < 7; n++)
		for (int mode = 0; mode < 7; mode++) {
			std::snprintf(tag, sizeof(tag), "n=%zu es=0 mode=%d", n,
			    mode);
			run_sort_case_all(scratch, n, 0, mode, tag);
		}

	/* a == NULL with n == 0: legal for every variant */
	for (int v = 0; v < 3; v++) {
		Stat &s = variant_stat(v);
		Ctx pctx = { CTX_MAGIC, nullptr, 1, 0, &tr_port };
		Ctx rctx = { CTX_MAGIC, nullptr, 1, 0, &tr_ref };
		int prc = 0, rrc = 0;

		s.cases++;
		trace_reset(&tr_port);
		trace_reset(&tr_ref);
		P::constraint_handler_reset();
		ref_ch_count = 0;
		ref_ch_msg[0] = '\0';
		ref_ch_error = 0;
		if (v == V_R) {
			P::qsort_r(nullptr, 0, 1, cmp_xy_thunk, &pctx);
			ref_qsort_r(nullptr, 0, 1, cmp_xy_thunk, &rctx);
		} else if (v == V_RC) {
			P::__qsort_r_compat(nullptr, 0, 1, &pctx, cmp_thunk_xy);
			ref___qsort_r_compat(nullptr, 0, 1, &rctx,
			    cmp_thunk_xy);
		} else {
			prc = P::qsort_s(nullptr, 0, 1, cmp_xy_thunk, &pctx);
			rrc = ref_qsort_s(nullptr, 0, 1, cmp_xy_thunk, &rctx);
		}
		if (prc != rrc)
			fail(s, "a=NULL n=0: return port=%d ref=%d", prc, rrc);
		else if (P::constraint_handler_count() != ref_ch_count)
			fail(s, "a=NULL n=0: handler count port=%lu ref=%lu",
			    P::constraint_handler_count(), ref_ch_count);
		else
			(void)traces_equal(s, "a=NULL n=0");
	}
}

/* ------------------------------------------------------------------ */
/* qsort_s constraint checks                                          */
/* ------------------------------------------------------------------ */

static void
check_qsort_s_params(void *pa, void *ra, size_t n, size_t es, bool with_cmp,
    const char *tag)
{
	Stat &s = st_qsort_s;
	Ctx pctx = { CTX_MAGIC, (const unsigned char *)pa, es, 0, &tr_port };
	Ctx rctx = { CTX_MAGIC, (const unsigned char *)ra, es, 0, &tr_ref };
	int prc, rrc;

	s.cases++;
	trace_reset(&tr_port);
	trace_reset(&tr_ref);
	P::constraint_handler_reset();
	ref_ch_count = 0;
	ref_ch_error = 0;
	ref_ch_msg[0] = '\0';

	prc = P::qsort_s(pa, n, es, with_cmp ? cmp_xy_thunk : nullptr, &pctx);
	rrc = ref_qsort_s(ra, n, es, with_cmp ? cmp_xy_thunk : nullptr, &rctx);

	if (prc != rrc) {
		fail(s, "%s: return port=%d ref=%d", tag, prc, rrc);
		return;
	}
	if (P::constraint_handler_count() != ref_ch_count) {
		fail(s, "%s: handler count port=%lu ref=%lu", tag,
		    P::constraint_handler_count(), ref_ch_count);
		return;
	}
	if (P::constraint_handler_error() != ref_ch_error) {
		fail(s, "%s: handler errno port=%d ref=%d", tag,
		    P::constraint_handler_error(), ref_ch_error);
		return;
	}
	if (std::strcmp(P::constraint_handler_msg(), ref_ch_msg) != 0) {
		fail(s, "%s: handler msg port=\"%s\" ref=\"%s\"", tag,
		    P::constraint_handler_msg(), ref_ch_msg);
		return;
	}
	(void)traces_equal(s, tag);
}

static void
test_qsort_s_constraints(void)
{
	const size_t RMAX = SIZE_MAX >> 1;
	unsigned char pbuf[64], rbuf[64];
	char tag[96];

	std::memset(pbuf, GUARD, sizeof(pbuf));
	std::memset(rbuf, GUARD, sizeof(rbuf));

	/* n boundary: RSIZE_MAX passes, RSIZE_MAX + 1 does not.  Both end in
	 * EINVAL, so the diagnostic string is what separates them. */
	check_qsort_s_params(nullptr, nullptr, RMAX, 1, true, "n=RMAX a=NULL");
	check_qsort_s_params(nullptr, nullptr, RMAX + 1, 1, true,
	    "n=RMAX+1 a=NULL");
	check_qsort_s_params(pbuf, rbuf, RMAX + 1, 1, true, "n=RMAX+1 a=buf");
	check_qsort_s_params(pbuf, rbuf, SIZE_MAX, 1, true, "n=SIZE_MAX");
	check_qsort_s_params(nullptr, nullptr, RMAX - 1, 1, true,
	    "n=RMAX-1 a=NULL");

	/* es boundary: es == RSIZE_MAX passes and, with n == 1, sorts a
	 * single element, i.e. returns 0 without touching memory. */
	check_qsort_s_params(pbuf, rbuf, 1, RMAX, true, "n=1 es=RMAX");
	check_qsort_s_params(pbuf, rbuf, 1, RMAX + 1, true, "n=1 es=RMAX+1");
	check_qsort_s_params(pbuf, rbuf, 1, SIZE_MAX, true, "n=1 es=SIZE_MAX");
	check_qsort_s_params(pbuf, rbuf, 0, RMAX + 1, true, "n=0 es=RMAX+1");
	check_qsort_s_params(pbuf, rbuf, 0, RMAX, true, "n=0 es=RMAX");

	/* the n != 0 gate */
	check_qsort_s_params(nullptr, nullptr, 0, 0, false, "n=0 all-null");
	check_qsort_s_params(nullptr, nullptr, 0, 4, false, "n=0 a=NULL");
	check_qsort_s_params(nullptr, nullptr, 1, 4, true, "n=1 a=NULL");
	check_qsort_s_params(nullptr, nullptr, 2, 4, true, "n=2 a=NULL");

	/* cmp == NULL, checked only after a != NULL */
	check_qsort_s_params(pbuf, rbuf, 1, 4, false, "n=1 cmp=NULL");
	check_qsort_s_params(pbuf, rbuf, 8, 4, false, "n=8 cmp=NULL");
	check_qsort_s_params(nullptr, nullptr, 8, 4, false,
	    "n=8 a=NULL cmp=NULL");

	/* es <= 0, checked only after cmp != NULL */
	check_qsort_s_params(pbuf, rbuf, 1, 0, true, "n=1 es=0");
	check_qsort_s_params(pbuf, rbuf, 5, 0, true, "n=5 es=0");
	check_qsort_s_params(pbuf, rbuf, 1, 1, true, "n=1 es=1");
	check_qsort_s_params(pbuf, rbuf, 6, 0, false, "n=6 es=0 cmp=NULL");
	check_qsort_s_params(nullptr, nullptr, 6, 0, true, "n=6 es=0 a=NULL");

	for (size_t n = 0; n <= 3; n++)
		for (size_t es = 0; es <= 2; es++)
			for (int wc = 0; wc < 2; wc++)
				for (int an = 0; an < 2; an++) {
					std::snprintf(tag, sizeof(tag),
					    "grid n=%zu es=%zu cmp=%d a=%d", n,
					    es, wc, an);
					check_qsort_s_params(
					    an ? nullptr : pbuf,
					    an ? nullptr : rbuf, n, es,
					    wc != 0, tag);
				}
}

/* ------------------------------------------------------------------ */
/* randomised sweep                                                   */
/* ------------------------------------------------------------------ */

static void
test_sorts_random(int variant, unsigned long iters, std::uint64_t seed)
{
	char tag[64];

	rng_seed(seed);
	for (unsigned long it = 0; it < iters; it++) {
		size_t n, es;
		int mode;
		std::uint32_t bucket = rng_below(100);

		if (bucket < 60)
			n = rng_below(13);
		else if (bucket < 85)
			n = rng_below(49);
		else if (bucket < 98)
			n = rng_below(97);
		else
			n = rng_below(MAXN + 1);

		es = 1 + rng_below(8);
		if (rng_below(16) == 0)
			es = MAXES;
		if (rng_below(32) == 0) {
			es = 0;
			n = rng_below(7);
		}
		while (n * es > sizeof(scratch))
			n /= 2;

		mode = (int)rng_below(7);

		{
			size_t body = n * es;
			std::uint32_t style = rng_below(8);

			for (size_t i = 0; i < body; i++) {
				switch (style) {
				case 0:
					scratch[i] = (unsigned char)rng_next();
					break;
				case 1:	/* only high-bit bytes */
					scratch[i] = (unsigned char)(0x80 |
					    (rng_next() & 0x7f));
					break;
				case 2:	/* NUL heavy */
					scratch[i] = (unsigned char)
					    (rng_below(4) == 0 ?
					    (rng_next() & 0xff) : 0);
					break;
				case 3:	/* tiny alphabet -> many ties */
					scratch[i] =
					    (unsigned char)rng_below(3);
					break;
				case 4:	/* extremes only */
					scratch[i] = (unsigned char)
					    (rng_below(2) ? 0xff : 0x00);
					break;
				case 5:	/* nearly sorted */
					scratch[i] = (unsigned char)
					    ((i / (es ? es : 1)) +
					    (rng_below(8) == 0 ?
					    rng_below(16) : 0));
					break;
				case 6:	/* nearly reverse sorted */
					scratch[i] = (unsigned char)
					    (255 - (i / (es ? es : 1)));
					break;
				default:
					scratch[i] = (unsigned char)
					    (rng_next() & 0x0f);
					break;
				}
			}
			if (style == 4 && body != 0 && rng_below(2) == 0)
				std::memset(scratch, 0x00, body);
		}

		std::snprintf(tag, sizeof(tag), "rnd#%lu n=%zu es=%zu mode=%d",
		    it, n, es, mode);
		run_sort_case(variant, scratch, n, es, mode, tag);
	}
}

/* ------------------------------------------------------------------ */

static void
row(const Stat &s)
{
	std::printf("  %-18s %10lu %10lu   %s\n", s.name, s.cases, s.fails,
	    s.fails == 0 ? "ok" : "FAIL");
}

int
main(void)
{
	unsigned long total_fail;

	test_memalignment();
	test_sorts_fixed();
	test_qsort_s_constraints();
	test_sorts_random(V_R, 200000, 0x1234567089abcdefull);
	test_sorts_random(V_RC, 200000, 0x0fedcba987654321ull);
	test_sorts_random(V_S, 200000, 0x00c0ffee0badf00dull);

	std::printf("\n  %-18s %10s %10s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("  %-18s %10s %10s   %s\n", "------------------",
	    "----------", "----------", "------");
	row(st_memalignment);
	row(st_qsort_r);
	row(st_qsort_r_compat);
	row(st_qsort_s);

	total_fail = st_memalignment.fails + st_qsort_r.fails +
	    st_qsort_r_compat.fails + st_qsort_s.fails;
	std::printf("\n  b0235: %s (%lu failure%s)\n\n",
	    total_fail == 0 ? "PASS" : "FAIL", total_fail,
	    total_fail == 1 ? "" : "s");
	return (total_fail == 0 ? 0 : 1);
}
