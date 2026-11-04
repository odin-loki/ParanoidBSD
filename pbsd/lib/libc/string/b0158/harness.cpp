// Differential test harness for PBSD batch b0158.
//
// Every ported function is exercised against the unmodified reference
// implementation in oracle.c.  Pointer results are reduced to an offset from
// the arena base before comparison.  wcstok is driven to exhaustion on every
// case, comparing token offset, the output state pointer, and the buffer after
// EVERY iteration.

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>

import pbsd.lib.libc.string.b0158;

namespace P = pbsd::lib_libc_string::b0158;

extern "C" {
wchar_t *ref_wcstok(wchar_t *s, const wchar_t *delim, wchar_t **last);
}

/* ------------------------------------------------------------------ */
/* bookkeeping                                                        */
/* ------------------------------------------------------------------ */

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	int shown;
};

static Stat st_wcstok = { "wcstok", 0, 0, 0 };

static const int MAX_SHOW = 8;

static bool
begin_fail(Stat &st, const char *what)
{
	st.fails++;
	if (st.shown >= MAX_SHOW)
		return false;
	st.shown++;
	std::printf("FAIL %s: %s\n", st.name, what);
	return true;
}

static void
dump_wide(const char *label, const wchar_t *p, size_t n)
{
	std::printf("    %s[%zu] =", label, n);
	for (size_t i = 0; i < n; i++)
		std::printf(" %08lx", (unsigned long)(std::uint32_t)p[i]);
	std::printf("\n");
}

static long
first_diff(const void *a, const void *b, size_t n)
{
	const unsigned char *pa = (const unsigned char *)a;
	const unsigned char *pb = (const unsigned char *)b;

	for (size_t i = 0; i < n; i++)
		if (pa[i] != pb[i])
			return (long)i;
	return -1;
}

/* ------------------------------------------------------------------ */
/* deterministic RNG (splitmix64, fixed seed)                         */
/* ------------------------------------------------------------------ */

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

	std::uint32_t below(std::uint32_t n) { return (std::uint32_t)(next() % n); }
	bool pct(std::uint32_t p) { return below(100) < p; }
};

/* ------------------------------------------------------------------ */
/* wchar buffers                                                      */
/* ------------------------------------------------------------------ */

static const unsigned char GUARD = 0x7f;

static const size_t WBUF = 128;
static const size_t MAXW = 48;

static void
fill_wstr(wchar_t *buf, size_t cap, const wchar_t *body, size_t len)
{
	std::memset(buf, GUARD, cap * sizeof(wchar_t));
	for (size_t i = 0; i < len; i++)
		buf[i] = body[i];
	buf[len] = L'\0';
}

static wchar_t
gen_wchar(Rng &r, int mode)
{
	switch (mode) {
	case 0:
		return (wchar_t)(1 + r.below(255));
	case 1:
		return (wchar_t)(0x80 + r.below(0x80));
	case 2:
		return (wchar_t)(0x8000 + r.below(0x8000));
	case 3:
		return (wchar_t)(0x7f7f7f7f);
	default:
		return (wchar_t)(1 + r.below(0x10ffff));
	}
}

/* ------------------------------------------------------------------ */
/* wcstok                                                             */
/* ------------------------------------------------------------------ */

static void
t_wcstok(const wchar_t *body, size_t blen, const wchar_t *dl, size_t dlen)
{
	wchar_t a[WBUF], b[WBUF], da[WBUF], db[WBUF];

	fill_wstr(a, WBUF, body, blen);
	fill_wstr(b, WBUF, body, blen);
	fill_wstr(da, WBUF, dl, dlen);
	fill_wstr(db, WBUF, dl, dlen);

	wchar_t *la = nullptr;
	wchar_t *lb = nullptr;
	size_t limit = blen + 8;
	bool done = false;

	for (size_t it = 0; it <= limit; it++) {
		wchar_t *ta = (it == 0)
		    ? P::wcstok(a, da, &la)
		    : P::wcstok(nullptr, da, &la);
		wchar_t *tb = (it == 0)
		    ? ref_wcstok(b, db, &lb)
		    : ref_wcstok(nullptr, db, &lb);

		long toka = (ta == nullptr) ? -1 : (long)(ta - a);
		long tokb = (tb == nullptr) ? -1 : (long)(tb - b);
		long sta = (la == nullptr) ? -1 : (long)(la - a);
		long stb = (lb == nullptr) ? -1 : (long)(lb - b);

		st_wcstok.cases++;
		if (toka != tokb || sta != stb ||
		    std::memcmp(a, b, sizeof a) != 0 ||
		    std::memcmp(da, db, sizeof da) != 0) {
			if (begin_fail(st_wcstok, "mismatch")) {
				dump_wide("s", body, blen);
				dump_wide("delim", dl, dlen);
				std::printf("    iter=%zu tok port=%ld ref=%ld "
				    "state port=%ld ref=%ld buf_diff@%ld "
				    "delim_diff@%ld\n", it, toka, tokb, sta, stb,
				    first_diff(a, b, sizeof a),
				    first_diff(da, db, sizeof da));
			}
			return;
		}
		if (toka < 0) {
			done = true;
			break;
		}
	}

	if (!done) {
		st_wcstok.cases++;
		if (begin_fail(st_wcstok, "iteration limit exceeded")) {
			dump_wide("s", body, blen);
			dump_wide("delim", dl, dlen);
		}
	}
}

static void
t_wcstok_null(const wchar_t *dl, size_t dlen)
{
	wchar_t da[WBUF], db[WBUF];

	fill_wstr(da, WBUF, dl, dlen);
	fill_wstr(db, WBUF, dl, dlen);

	wchar_t *la = nullptr;
	wchar_t *lb = nullptr;
	wchar_t *ta = P::wcstok(nullptr, da, &la);
	wchar_t *tb = ref_wcstok(nullptr, db, &lb);

	st_wcstok.cases++;
	if (ta != nullptr || tb != nullptr || la != nullptr || lb != nullptr ||
	    std::memcmp(da, db, sizeof da) != 0) {
		if (begin_fail(st_wcstok, "NULL s and NULL *last")) {
			dump_wide("delim", dl, dlen);
			std::printf("    tok_null port=%d ref=%d state_null "
			    "port=%d ref=%d delim_diff@%ld\n", ta == nullptr,
			    tb == nullptr, la == nullptr, lb == nullptr,
			    first_diff(da, db, sizeof da));
		}
	}
}

static void
t_wcstok_resume_null_last(const wchar_t *body, size_t blen,
    const wchar_t *dl, size_t dlen)
{
	wchar_t a[WBUF], b[WBUF], da[WBUF], db[WBUF];

	fill_wstr(a, WBUF, body, blen);
	fill_wstr(b, WBUF, body, blen);
	fill_wstr(da, WBUF, dl, dlen);
	fill_wstr(db, WBUF, dl, dlen);

	wchar_t *la = a;
	wchar_t *lb = b;
	*la = L'\0';
	*lb = L'\0';

	wchar_t *ta = P::wcstok(nullptr, da, &la);
	wchar_t *tb = ref_wcstok(nullptr, db, &lb);

	st_wcstok.cases++;
	if (ta != nullptr || tb != nullptr || la != nullptr || lb != nullptr ||
	    std::memcmp(a, b, sizeof a) != 0 ||
	    std::memcmp(da, db, sizeof da) != 0) {
		if (begin_fail(st_wcstok, "resume with *last at empty string")) {
			dump_wide("s", body, blen);
			dump_wide("delim", dl, dlen);
		}
	}
}

#define WCS(...) (const wchar_t[]){ __VA_ARGS__, L'\0' }

static const wchar_t W_EMPTY[] = { L'\0' };

static void
hand_wcstok(void)
{
	t_wcstok_null(W_EMPTY, 0);
	t_wcstok_null(WCS(L','), 1);
	t_wcstok_null(WCS(L'a', L'b'), 2);

	t_wcstok_resume_null_last(WCS(L'x'), 1, WCS(L','), 1);

	t_wcstok(W_EMPTY, 0, W_EMPTY, 0);
	t_wcstok(W_EMPTY, 0, WCS(L','), 1);
	t_wcstok(WCS(L'a'), 1, W_EMPTY, 0);
	t_wcstok(WCS(L'a'), 1, WCS(L','), 1);

	t_wcstok(WCS(L'a', L'b'), 2, WCS(L','), 1);
	t_wcstok(WCS(L',', L'a'), 2, WCS(L','), 1);
	t_wcstok(WCS(L'a', L','), 2, WCS(L','), 1);
	t_wcstok(WCS(L','), 1, WCS(L','), 1);
	t_wcstok(WCS(L',', L','), 2, WCS(L','), 1);
	t_wcstok(WCS(L',', L',', L','), 3, WCS(L','), 1);
	t_wcstok(WCS(L'a', L',', L'b', L',', L'c', L',', L'd'), 7, WCS(L','), 1);
	t_wcstok(WCS(L'a', L',', L',', L'b'), 4, WCS(L','), 1);

	t_wcstok(WCS(L'a', L'b', L'c'), 3, WCS(L','), 1);
	t_wcstok(WCS(L'a', L'b', L'c'), 3, WCS(L'x', L'y', L'z'), 3);
	t_wcstok(WCS(L'a', L'b', L'c'), 3, WCS(L'\x7f'), 1);
	t_wcstok(WCS(L'a', L',', L'b'), 3, WCS(L',', L'\x7f'), 2);

	t_wcstok(WCS(L'a', L';', L'b'), 3, WCS(L',', L';'), 2);
	t_wcstok(WCS(L'a', L';', L'b'), 3, WCS(L'x', L'y', L';'), 3);
	t_wcstok(WCS(L'a', L';', L'b', L',', L'c'), 5, WCS(L',', L';'), 2);
	t_wcstok(WCS(L'a', L';', L'b', L',', L'c'), 5, WCS(L';', L','), 2);
	t_wcstok(WCS(L'a', L'.', L'b', L':', L'c', L';', L'd', L',', L'e'), 9,
	    WCS(L',', L';', L':', L'.'), 4);
	t_wcstok(WCS(L'a', L'b', L'c', L'd', L'e', L'f', L'g'), 7,
	    WCS(L'u', L'v', L'w', L'x', L'y', L'z'), 6);

	t_wcstok(WCS(L'a', (wchar_t)0x80, L'b'), 3, WCS((wchar_t)0x80), 1);
	t_wcstok(WCS(L'a', (wchar_t)0xff, L'b'), 3, WCS((wchar_t)0xff), 1);
	t_wcstok(WCS(L'a', (wchar_t)0xff, L'b'), 3, WCS(L'\x7f', (wchar_t)0xff), 2);
	t_wcstok(WCS((wchar_t)0x80, (wchar_t)0xff, (wchar_t)0xc0), 3,
	    WCS((wchar_t)0x80, (wchar_t)0xc0), 2);
	t_wcstok(WCS((wchar_t)0xff, (wchar_t)0xff, (wchar_t)0xff), 3,
	    WCS((wchar_t)0xff), 1);
	t_wcstok(WCS(L'a', (wchar_t)0xc0, L'b', (wchar_t)0xbf, L'c'), 5,
	    WCS((wchar_t)0xc0, (wchar_t)0xbf), 2);
	t_wcstok(WCS(L'a', L'b', L'c'), 3,
	    WCS((wchar_t)0x80, (wchar_t)0x81, (wchar_t)0x82), 3);

	t_wcstok(WCS(L'a', L'a', L'a', L'a', L'a', L'a', L'a', L'a'), 8,
	    WCS(L'a'), 1);
	t_wcstok(WCS(L'a', L'a', L'a', L'a', L'a', L'a', L'a', L'b'), 8,
	    WCS(L'a'), 1);
	t_wcstok(WCS(L'b', L'a', L'a', L'a', L'a', L'a', L'a', L'a'), 8,
	    WCS(L'a'), 1);
	t_wcstok(WCS(L'a', L',', L'b', L';', L'c', L',', L'd', L';', L'e', L',',
	    L'f', L';', L'g', L',', L'h'), 15, WCS(L',', L';'), 2);

	t_wcstok(WCS(L'a', L'b'), 2, WCS(L'b'), 1);
	t_wcstok(WCS(L'a', L'b', L'c', L'd', L'e'), 5, WCS(L'c'), 1);
	t_wcstok(WCS(L',', L'a', L',', L'b', L','), 5, WCS(L','), 1);
	t_wcstok(WCS(L'a'), 1, WCS(L'a', L'b', L'c'), 3);
	t_wcstok(WCS(L'x', L'y', L'z'), 3, WCS(L'x', L'y'), 2);

	t_wcstok(WCS((wchar_t)0x80000000), 1, WCS((wchar_t)0x80000000), 1);
	t_wcstok(WCS((wchar_t)0xffffffff), 1, WCS((wchar_t)0xffffffff), 1);
	t_wcstok(WCS(L'a', (wchar_t)0xffffffff, L'b'), 3,
	    WCS((wchar_t)0xffffffff), 1);
}

static void
sweep_wcstok(void)
{
	Rng r(0x574353544F4B31ull);
	wchar_t s[MAXW + 1], dl[MAXW + 1];

	for (int iter = 0; iter < 200000; iter++) {
		int mode = (iter % 6 == 0) ? 0 : (int)(iter % 5);
		size_t dlen = r.below(6);
		for (size_t i = 0; i < dlen; i++)
			dl[i] = gen_wchar(r, mode);

		size_t slen = r.below(25);
		for (size_t i = 0; i < slen; i++) {
			if (dlen != 0 && r.pct(40))
				s[i] = dl[r.below((std::uint32_t)dlen)];
			else
				s[i] = gen_wchar(r, mode);
		}

		t_wcstok(s, slen, dl, dlen);
		if ((iter & 0x3ff) == 0)
			t_wcstok_null(dl, dlen);
		if ((iter & 0x7ff) == 0)
			t_wcstok_resume_null_last(s, slen, dl, dlen);
	}
}

/* ------------------------------------------------------------------ */
/* main                                                               */
/* ------------------------------------------------------------------ */

int
main()
{
	hand_wcstok();
	sweep_wcstok();

	const Stat *all[] = { &st_wcstok };
	unsigned long long total_cases = 0;
	unsigned long long total_fails = 0;
	int any_fail = 0;

	std::printf("\n%-12s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-12s %12s %12s\n", "--------", "-----", "--------");
	for (const Stat *st : all) {
		std::printf("%-12s %12llu %12llu\n", st->name, st->cases,
		    st->fails);
		total_cases += st->cases;
		total_fails += st->fails;
		if (st->fails != 0)
			any_fail = 1;
	}
	std::printf("%-12s %12llu %12llu\n", "TOTAL", total_cases,
	    total_fails);

	return any_fail ? 1 : 0;
}
