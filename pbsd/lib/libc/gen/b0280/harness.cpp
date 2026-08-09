/*
 * Differential harness for batch b0280 (stringlist).
 */

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

import pbsd.lib.libc.gen.b0280;

namespace P = pbsd::lib_libc_gen::b0280;

typedef struct _stringlist {
	char	**sl_str;
	size_t	  sl_max;
	size_t	  sl_cur;
} StringList;

static inline P::StringList *
to_port(StringList *sl)
{
	return (reinterpret_cast<P::StringList *>(sl));
}

static inline StringList *
from_port(P::StringList *sl)
{
	return (reinterpret_cast<StringList *>(sl));
}

static StringList *
port_sl_init(void)
{
	return (from_port(P::sl_init()));
}

static void
port_sl_free(StringList *sl, int all)
{
	P::sl_free(to_port(sl), all);
}

extern "C" {
StringList *ref_sl_init(void);
int ref_sl_add(StringList *, char *);
void ref_sl_free(StringList *, int);
char *ref_sl_find(StringList *, const char *);
}

extern "C" void * __real_malloc(size_t);
extern "C" void * __real_realloc(void *, size_t);

enum {
	F_SL_INIT,
	F_SL_ADD,
	F_SL_FREE,
	F_SL_FIND,
	NFUNC
};

static const char *const fname[NFUNC] = {
	"sl_init",
	"sl_add",
	"sl_free",
	"sl_find",
};

static unsigned long long ncases[NFUNC];
static unsigned long long nfails[NFUNC];

static constexpr int MAXREPORT = 16;
static int nreported;

static constexpr unsigned char GUARD = 0x7fu;

static void
report(int f, const char *ctx, const char *detail)
{
	nfails[f]++;
	if (nreported < MAXREPORT) {
		nreported++;
		std::printf("FAIL %-24s %s: %s\n", fname[f], ctx, detail);
	} else if (nreported == MAXREPORT) {
		nreported++;
		std::printf("... further failures suppressed\n");
	}
}

/* ------------------------------------------------------------------ */
/* PRNG (fixed seed)                                                  */
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state = 0xb0280feedfaceULL;

static std::uint64_t
nextrand(void)
{
	std::uint64_t x = rng_state;

	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	rng_state = x;
	return x * 0x2545f4914f6cdd1dULL;
}

static std::uint32_t
randu32(void)
{
	return (std::uint32_t)(nextrand() & 0xffffffffu);
}

static int
randint(int lo, int hi)
{
	if (hi <= lo)
		return lo;
	return lo + (int)(randu32() % (std::uint32_t)(hi - lo + 1));
}

/* ------------------------------------------------------------------ */
/* malloc / reallocf wrap hooks                                       */
/* ------------------------------------------------------------------ */

static int malloc_fail_nth;
static int malloc_call_count;
static int realloc_fail;

static void
alloc_hook_reset(void)
{
	malloc_fail_nth = 0;
	malloc_call_count = 0;
	realloc_fail = 0;
}

extern "C" void *
__wrap_malloc(size_t sz)
{
	malloc_call_count++;
	if (malloc_fail_nth != 0 && malloc_call_count == malloc_fail_nth)
		return (NULL);
	return __real_malloc(sz);
}

extern "C" void * __real_realloc(void *, size_t);

extern "C" void *
__wrap_realloc(void *ptr, size_t sz)
{
	if (realloc_fail != 0)
		return (NULL);
	return __real_realloc(ptr, sz);
}

/* ------------------------------------------------------------------ */
/* String pool helpers                                                */
/* ------------------------------------------------------------------ */

struct StrPool {
	static constexpr std::size_t CAP = 65536;
	char bytes[CAP];
	std::size_t used;

	void
	reset(void)
	{
		used = 0;
	}

	char *
	dup(const char *s)
	{
		std::size_t n = std::strlen(s) + 1;

		if (used + n > CAP)
			return (NULL);
		char *p = bytes + used;

		std::memcpy(p, s, n);
		used += n;
		return (p);
	}

	char *
	dup_bytes(const unsigned char *s, std::size_t n)
	{
		if (used + n + 1 > CAP)
			return (NULL);
		char *p = bytes + used;

		std::memcpy(p, s, n);
		p[n] = '\0';
		used += n + 1;
		return (p);
	}

	long
	offset(const char *p) const
	{
		if (p == NULL)
			return (-1);
		return (long)(p - bytes);
	}
};

static long
ptr_off(const char *p, const char *base)
{
	if (p == NULL)
		return (-1);
	return (long)(p - base);
}

static void
fill_hibyte_str(char *buf, std::size_t bufsz, std::uint32_t pat, int len)
{
	std::size_t i;

	if (len < 0)
		len = 0;
	if ((std::size_t)len >= bufsz)
		len = (int)bufsz - 1;
	for (i = 0; i < (std::size_t)len; i++)
		buf[i] = (char)(0x80u | ((pat + (std::uint32_t)i) & 0x7fu));
	buf[len] = '\0';
}

/* ------------------------------------------------------------------ */
/* StringList snapshot comparison                                     */
/* ------------------------------------------------------------------ */

struct SlSnap {
	size_t sl_cur;
	size_t sl_max;
	char **sl_str;
	long *entry_offs;
};

static void
snap_free(SlSnap *s)
{
	std::free(s->entry_offs);
	std::memset(s, 0, sizeof(*s));
}

static int
snap_capture(SlSnap *s, StringList *sl, const char *pool_base)
{
	std::size_t i;

	s->sl_cur = sl->sl_cur;
	s->sl_max = sl->sl_max;
	s->sl_str = sl->sl_str;
	s->entry_offs = (long *)std::calloc(sl->sl_cur, sizeof(long));
	if (s->entry_offs == NULL)
		return (-1);
	for (i = 0; i < sl->sl_cur; i++)
		s->entry_offs[i] = ptr_off(sl->sl_str[i], pool_base);
	return (0);
}

static int
snap_equal(const SlSnap *a, const SlSnap *b)
{
	std::size_t i;

	if (a->sl_cur != b->sl_cur || a->sl_max != b->sl_max)
		return (0);
	if ((a->sl_str == NULL) != (b->sl_str == NULL))
		return (0);
	for (i = 0; i < a->sl_cur; i++) {
		if (a->entry_offs[i] != b->entry_offs[i])
			return (0);
	}
	return (1);
}

/* ------------------------------------------------------------------ */
/* sl_init                                                            */
/* ------------------------------------------------------------------ */

static void
check_sl_init(int f, const char *ctx, StringList *(*init_fn)(void))
{
	StringList *sl;

	ncases[f]++;
	sl = init_fn();
	if (sl == NULL) {
		report(f, ctx, "init returned NULL");
		return;
	}
	if (sl->sl_cur != 0 || sl->sl_max != 20 || sl->sl_str == NULL) {
		report(f, ctx, "unexpected initial fields");
		ref_sl_free(sl, 0);
		return;
	}
	ref_sl_free(sl, 0);
}

static void
test_sl_init_err_exit(const char *ctx, int fail_nth,
    StringList *(*init_fn)(void))
{
	pid_t pid;
	int st;

	alloc_hook_reset();
	malloc_fail_nth = fail_nth;
	pid = fork();
	if (pid < 0) {
		report(F_SL_INIT, ctx, "fork failed");
		return;
	}
	if (pid == 0) {
		StringList *sl = init_fn();

		(void)sl;
		_exit(99);
	}
	if (waitpid(pid, &st, 0) < 0) {
		report(F_SL_INIT, ctx, "waitpid failed");
		return;
	}
	ncases[F_SL_INIT]++;
	if (!WIFEXITED(st) || WEXITSTATUS(st) != 1)
		report(F_SL_INIT, ctx, "expected exit status 1 from _err");
	alloc_hook_reset();
}

static void
test_sl_init_edges(void)
{
	check_sl_init(F_SL_INIT, "basic", ref_sl_init);
	check_sl_init(F_SL_INIT, "basic port", port_sl_init);
	test_sl_init_err_exit("malloc fail 1 ref", 1, ref_sl_init);
	test_sl_init_err_exit("malloc fail 2 ref", 2, ref_sl_init);
	test_sl_init_err_exit("malloc fail 1 port", 1, port_sl_init);
	test_sl_init_err_exit("malloc fail 2 port", 2, port_sl_init);
}

/* ------------------------------------------------------------------ */
/* sl_add                                                             */
/* ------------------------------------------------------------------ */

static void
check_sl_add_pair(int f, const char *ctx, StringList *sl_r, StringList *sl_p,
    char *name_r, char *name_p, const StrPool *pool_r, const StrPool *pool_p,
    int expect_rv, const SlSnap *pre_r, const SlSnap *pre_p)
{
	SlSnap got_r, got_p;
	int rv_r, rv_p;

	std::memset(&got_r, 0, sizeof got_r);
	std::memset(&got_p, 0, sizeof got_p);

	ncases[f]++;
	rv_r = ref_sl_add(sl_r, name_r);
	rv_p = P::sl_add(to_port(sl_p), name_p);
	if (rv_r != expect_rv || rv_p != expect_rv) {
		report(f, ctx, "unexpected return value");
		goto out;
	}
	if (rv_r != rv_p) {
		report(f, ctx, "return value mismatch");
		goto out;
	}
	if (snap_capture(&got_r, sl_r, pool_r->bytes) != 0 ||
	    snap_capture(&got_p, sl_p, pool_p->bytes) != 0) {
		report(f, ctx, "snap capture failed");
		goto out;
	}
	if (!snap_equal(&got_r, &got_p)) {
		report(f, ctx, "post-add list mismatch");
		goto out;
	}
	if (rv_r == 0 && pre_r != NULL && pre_p != NULL) {
		long want_r = pool_r->offset(name_r);
		long want_p = pool_p->offset(name_p);

		if (got_r.sl_cur != pre_r->sl_cur + 1 ||
		    got_p.sl_cur != pre_p->sl_cur + 1) {
			report(f, ctx, "sl_cur not incremented");
			goto out;
		}
		if (got_r.entry_offs[got_r.sl_cur - 1] != want_r ||
		    got_p.entry_offs[got_p.sl_cur - 1] != want_p) {
			report(f, ctx, "stored pointer offset wrong");
			goto out;
		}
	}
out:
	snap_free(&got_r);
	snap_free(&got_p);
}

static void
run_add_sequence(int f, const char *ctx, int nadd, int trigger_realloc_fail)
{
	StringList *sl_r, *sl_p;
	StrPool pool_r, pool_p;
	char label[128];
	int i;

	sl_r = ref_sl_init();
	sl_p = port_sl_init();
	pool_r.reset();
	pool_p.reset();

	for (i = 0; i < nadd; i++) {
		SlSnap pre_r, pre_p;
		char item[64];
		char *name_r, *name_p;
		int expect = 0;

		std::snprintf(item, sizeof item, "item%d", i);
		name_r = pool_r.dup(item);
		name_p = pool_p.dup(item);
		std::memset(&pre_r, 0, sizeof pre_r);
		std::memset(&pre_p, 0, sizeof pre_p);
		(void)snap_capture(&pre_r, sl_r, pool_r.bytes);
		(void)snap_capture(&pre_p, sl_p, pool_p.bytes);

		if (trigger_realloc_fail != 0 && i == nadd - 1) {
			alloc_hook_reset();
			realloc_fail = 1;
			expect = -1;
		}

		std::snprintf(label, sizeof label, "%s i=%d", ctx, i);
		check_sl_add_pair(f, label, sl_r, sl_p, name_r, name_p, &pool_r,
		    &pool_p, expect, &pre_r, &pre_p);
		snap_free(&pre_r);
		snap_free(&pre_p);
		alloc_hook_reset();

		if (expect == -1)
			break;
	}

	ref_sl_free(sl_r, 1);
	P::sl_free(to_port(sl_p), 1);
}

static void
test_sl_add_edges(void)
{
	run_add_sequence(F_SL_ADD, "zero adds", 0, 0);
	run_add_sequence(F_SL_ADD, "single", 1, 0);
	run_add_sequence(F_SL_ADD, "boundary 18", 18, 0);
	run_add_sequence(F_SL_ADD, "boundary 19 no grow", 19, 0);
	run_add_sequence(F_SL_ADD, "boundary 20 grow", 20, 0);
	run_add_sequence(F_SL_ADD, "boundary 21 grow", 21, 0);
	run_add_sequence(F_SL_ADD, "boundary 39", 39, 0);
	run_add_sequence(F_SL_ADD, "boundary 40 grow2", 40, 0);
	run_add_sequence(F_SL_ADD, "reallocf fail at 19", 19, 1);

	{
		StringList *sl_r, *sl_p;
		StrPool pool_r, pool_p;
		unsigned char hb[] = { 0x80, 'a', 0xff, 0x00 };
		char *s_r, *s_p;

		sl_r = ref_sl_init();
		sl_p = port_sl_init();
		pool_r.reset();
		pool_p.reset();
		s_r = pool_r.dup_bytes(hb, 3);
		s_p = pool_p.dup_bytes(hb, 3);
		check_sl_add_pair(F_SL_ADD, "high-bit bytes", sl_r, sl_p, s_r, s_p,
		    &pool_r, &pool_p, 0, NULL, NULL);
		ref_sl_free(sl_r, 1);
		P::sl_free(to_port(sl_p), 1);
	}
}

static void
test_sl_add_random(void)
{
	for (int i = 0; i < 200000; i++) {
		StringList *sl_r, *sl_p;
		StrPool pool_r, pool_p;
		char ctx[48];
		int n = randint(0, 80);

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		sl_r = ref_sl_init();
		sl_p = port_sl_init();
		pool_r.reset();
		pool_p.reset();

		for (int j = 0; j < n; j++) {
			char buf[128];
			char label[80];
			SlSnap pre_r, pre_p;

			if ((randu32() % 5u) == 0u)
				fill_hibyte_str(buf, sizeof buf, randu32(),
				    randint(0, 60));
			else
				std::snprintf(buf, sizeof buf, "%08x_%d",
				    randu32(), j);
			char *name_r = pool_r.dup(buf);
			char *name_p = pool_p.dup(buf);

			std::memset(&pre_r, 0, sizeof pre_r);
			std::memset(&pre_p, 0, sizeof pre_p);
			(void)snap_capture(&pre_r, sl_r, pool_r.bytes);
			(void)snap_capture(&pre_p, sl_p, pool_p.bytes);
			std::snprintf(label, sizeof label, "%s j=%d", ctx, j);
			check_sl_add_pair(F_SL_ADD, label, sl_r, sl_p, name_r,
			    name_p, &pool_r, &pool_p, 0, &pre_r, &pre_p);
			snap_free(&pre_r);
			snap_free(&pre_p);
		}

		ref_sl_free(sl_r, 1);
		P::sl_free(to_port(sl_p), 1);
	}
}

/* ------------------------------------------------------------------ */
/* sl_free                                                            */
/* ------------------------------------------------------------------ */

static void
check_sl_free_null(int f, const char *ctx, void (*free_fn)(StringList *, int))
{
	ncases[f]++;
	free_fn(NULL, 0);
	free_fn(NULL, 1);
}

static void
check_sl_free_pair(int f, const char *ctx, int all, int nitems)
{
	StringList *sl_r, *sl_p;
	StrPool pool_r, pool_p;
	int i;

	sl_r = ref_sl_init();
	sl_p = port_sl_init();
	pool_r.reset();
	pool_p.reset();
	for (i = 0; i < nitems; i++) {
		char buf[32];

		char *s_r, *s_p;

		std::snprintf(buf, sizeof buf, "x%d", i);
		s_r = pool_r.dup(buf);
		s_p = pool_p.dup(buf);
		(void)ref_sl_add(sl_r, s_r);
		(void)P::sl_add(to_port(sl_p), s_p);
	}

	ncases[f]++;
	ref_sl_free(sl_r, all);
	port_sl_free(sl_p, all);
}

static void
test_sl_free_edges(void)
{
	check_sl_free_null(F_SL_FREE, "null all=0 ref", ref_sl_free);
	check_sl_free_null(F_SL_FREE, "null all=1 ref", ref_sl_free);
	check_sl_free_null(F_SL_FREE, "null all=0 port", port_sl_free);
	check_sl_free_null(F_SL_FREE, "null all=1 port", port_sl_free);
	check_sl_free_pair(F_SL_FREE, "empty all=0", 0, 0);
	check_sl_free_pair(F_SL_FREE, "empty all=1", 1, 0);
	check_sl_free_pair(F_SL_FREE, "items all=0", 0, 5);
	check_sl_free_pair(F_SL_FREE, "items all=1", 1, 5);
	check_sl_free_pair(F_SL_FREE, "items all=1 grow", 1, 25);
}

static void
test_sl_free_random(void)
{
	for (int i = 0; i < 200000; i++) {
		StringList *sl_r, *sl_p;
		StrPool pool_r, pool_p;
		char ctx[48];
		int n = randint(0, 50);
		int all = (int)(randu32() & 1u);

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		sl_r = ref_sl_init();
		sl_p = port_sl_init();
		pool_r.reset();
		pool_p.reset();
		for (int j = 0; j < n; j++) {
			char buf[64];

			char *s_r, *s_p;

			std::snprintf(buf, sizeof buf, "%x", randu32());
			s_r = pool_r.dup(buf);
			s_p = pool_p.dup(buf);
			(void)ref_sl_add(sl_r, s_r);
			(void)P::sl_add(to_port(sl_p), s_p);
		}
		ncases[F_SL_FREE]++;
		ref_sl_free(sl_r, all);
		port_sl_free(sl_p, all);
	}
}

/* ------------------------------------------------------------------ */
/* sl_find                                                            */
/* ------------------------------------------------------------------ */

static void
check_sl_find_pair(int f, const char *ctx, StringList *sl_r, StringList *sl_p,
    const char *needle_r, const char *needle_p, const StrPool *pool_r,
    const StrPool *pool_p, int expect_found)
{
	char *got_r, *got_p;
	long off_r, off_p;

	ncases[f]++;
	got_r = ref_sl_find(sl_r, needle_r);
	got_p = P::sl_find(to_port(sl_p), needle_p);
	off_r = pool_r->offset(got_r);
	off_p = pool_p->offset(got_p);

	if ((got_r == NULL) != (got_p == NULL)) {
		report(f, ctx, "NULL mismatch");
		return;
	}
	if (expect_found) {
		if (got_r == NULL) {
			report(f, ctx, "expected found");
			return;
		}
		if (off_r != off_p) {
			report(f, ctx, "offset mismatch");
			return;
		}
	} else if (got_r != NULL) {
		report(f, ctx, "expected not found");
	}
}

static void
build_list(StringList **sl_r, StringList **sl_p, StrPool *pool_r,
    StrPool *pool_p, const char *const *items, int n)
{
	int i;

	*sl_r = ref_sl_init();
	*sl_p = port_sl_init();
	pool_r->reset();
	pool_p->reset();
	for (i = 0; i < n; i++) {
		char *s_r = pool_r->dup(items[i]);
		char *s_p = pool_p->dup(items[i]);

		(void)ref_sl_add(*sl_r, s_r);
		(void)P::sl_add(to_port(*sl_p), s_p);
	}
}

static void
test_sl_find_edges(void)
{
	StringList *sl_r, *sl_p;
	StrPool pool_r, pool_p;
	unsigned char hb_needle[] = { 0xfe, 0x80, 0x00 };
	unsigned char hb_item[] = { 0xfe, 0x80, 0x00 };
	char hb_buf[8];

	sl_r = ref_sl_init();
	sl_p = port_sl_init();
	pool_r.reset();
	pool_p.reset();
	check_sl_find_pair(F_SL_FIND, "empty list", sl_r, sl_p, "x", "x",
	    &pool_r, &pool_p, 0);
	ref_sl_free(sl_r, 0);
	P::sl_free(to_port(sl_p), 0);

	build_list(&sl_r, &sl_p, &pool_r, &pool_p,
	    (const char *const[]){ "alpha", "beta", "gamma" }, 3);
	check_sl_find_pair(F_SL_FIND, "first", sl_r, sl_p, "alpha", "alpha",
	    &pool_r, &pool_p, 1);
	check_sl_find_pair(F_SL_FIND, "middle", sl_r, sl_p, "beta", "beta",
	    &pool_r, &pool_p, 1);
	check_sl_find_pair(F_SL_FIND, "last", sl_r, sl_p, "gamma", "gamma",
	    &pool_r, &pool_p, 1);
	check_sl_find_pair(F_SL_FIND, "missing", sl_r, sl_p, "delta", "delta",
	    &pool_r, &pool_p, 0);
	check_sl_find_pair(F_SL_FIND, "empty needle", sl_r, sl_p, "", "",
	    &pool_r, &pool_p, 0);
	ref_sl_free(sl_r, 1);
	P::sl_free(to_port(sl_p), 1);

	build_list(&sl_r, &sl_p, &pool_r, &pool_p,
	    (const char *const[]){ "dup", "dup", "other" }, 3);
	check_sl_find_pair(F_SL_FIND, "duplicate first", sl_r, sl_p, "dup",
	    "dup", &pool_r, &pool_p, 1);
	ref_sl_free(sl_r, 1);
	P::sl_free(to_port(sl_p), 1);

	{
		const char *hb_items[] = { (const char *)hb_item };

		build_list(&sl_r, &sl_p, &pool_r, &pool_p, hb_items, 1);
		std::memcpy(hb_buf, hb_needle, sizeof hb_needle);
		check_sl_find_pair(F_SL_FIND, "high-bit", sl_r, sl_p, hb_buf, hb_buf,
		    &pool_r, &pool_p, 1);
		ref_sl_free(sl_r, 1);
		port_sl_free(sl_p, 1);
	}

	build_list(&sl_r, &sl_p, &pool_r, &pool_p,
	    (const char *const[]){ "a" }, 1);
	check_sl_find_pair(F_SL_FIND, "prefix miss", sl_r, sl_p, "ab", "ab",
	    &pool_r, &pool_p, 0);
	ref_sl_free(sl_r, 1);
	P::sl_free(to_port(sl_p), 1);
}

static void
test_sl_find_random(void)
{
	for (int i = 0; i < 200000; i++) {
		StringList *sl_r, *sl_p;
		StrPool pool_r, pool_p;
		char ctx[48];
		char items[32][64];
		int n = randint(0, 20);
		int q;

		std::snprintf(ctx, sizeof ctx, "rand %d", i);
		sl_r = ref_sl_init();
		sl_p = port_sl_init();
		pool_r.reset();
		pool_p.reset();

		for (int j = 0; j < n; j++) {
			if ((randu32() % 4u) == 0u)
				fill_hibyte_str(items[j], sizeof items[j],
				    randu32(), randint(1, 40));
			else
				std::snprintf(items[j], sizeof items[j],
				    "%08x", randu32());
			char *s_r = pool_r.dup(items[j]);
			char *s_p = pool_p.dup(items[j]);

			(void)ref_sl_add(sl_r, s_r);
			(void)P::sl_add(to_port(sl_p), s_p);
		}

		q = randint(0, n + 3);
		if (q < n) {
			check_sl_find_pair(F_SL_FIND, ctx, sl_r, sl_p, items[q],
			    items[q], &pool_r, &pool_p, 1);
		} else {
			char miss[64];

			std::snprintf(miss, sizeof miss, "missing_%08x",
			    randu32());
			check_sl_find_pair(F_SL_FIND, ctx, sl_r, sl_p, miss,
			    miss, &pool_r, &pool_p, 0);
		}

		ref_sl_free(sl_r, 1);
		P::sl_free(to_port(sl_p), 1);
	}
}

/* ------------------------------------------------------------------ */
/* main                                                               */
/* ------------------------------------------------------------------ */

int
main(void)
{
	unsigned long long total_cases = 0;
	unsigned long long total_fails = 0;
	int exit_code = 0;

	test_sl_init_edges();
	test_sl_add_edges();
	test_sl_add_random();
	test_sl_free_edges();
	test_sl_free_random();
	test_sl_find_edges();
	test_sl_find_random();

	std::printf("\n%-24s %10s %10s\n", "Function", "Cases", "Failures");
	for (int f = 0; f < NFUNC; f++) {
		std::printf("%-24s %10llu %10llu\n",
		    fname[f], ncases[f], nfails[f]);
		total_cases += ncases[f];
		total_fails += nfails[f];
	}
	std::printf("%-24s %10llu %10llu\n", "TOTAL", total_cases, total_fails);

	if (total_fails != 0)
		exit_code = 1;
	return exit_code;
}
