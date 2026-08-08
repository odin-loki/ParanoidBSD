/*
 * Differential test for batch b0156s4 (lib/libc/locale/collate.c).
 *
 * Every ported function is run side by side with the ref_ oracle on the
 * very same collate table; return values, out-parameters, guard-filled
 * output buffers and mbstate_t are all compared byte for byte.
 */

#include <cstdarg>
#include <cerrno>
#include <climits>
#include <clocale>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>

#include <algorithm>
#include <memory>
#include <vector>

#undef COLL_WEIGHTS_MAX

import pbsd.lib.libc.locale.b0156s4;

namespace P = pbsd::lib_libc_locale::b0156s4;

static const int NWEIGHT = P::COLL_WEIGHTS_MAX;

extern "C" {
const int32_t *ref_substsearch(P::xlocale_collate *, wchar_t, int);
P::collate_chain_t *ref_chainsearch(P::xlocale_collate *, const wchar_t *,
    int *);
P::collate_large_t *ref_largesearch(P::xlocale_collate *, wchar_t);
void ref__collate_lookup(P::xlocale_collate *, const wchar_t *, int *, int *,
    int, const int **);
int ref_xfrm(P::xlocale_collate *, unsigned char *, int, int);
size_t ref__collate_wxfrm(P::xlocale_collate *, const wchar_t *, wchar_t *,
    size_t);
size_t ref__collate_sxfrm(P::xlocale_collate *, const wchar_t *, char *,
    size_t);
int ref__collate_equiv_value(P::_xlocale *, const wchar_t *, size_t);
size_t ref__collate_collating_symbol(wchar_t *, size_t, const char *, size_t,
    mbstate_t *);
int ref__collate_equiv_class(const char *, size_t, mbstate_t *);
size_t ref__collate_equiv_match(int, wchar_t *, size_t, wchar_t, const char *,
    size_t, mbstate_t *, size_t *);
void ref_set_collate(P::xlocale_collate *);
}

/* ------------------------------------------------------------------ */
/* bookkeeping							      */
/* ------------------------------------------------------------------ */

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
};

static Stat stats[] = {
	{ "substsearch", 0, 0 },
	{ "chainsearch", 0, 0 },
	{ "largesearch", 0, 0 },
	{ "xfrm", 0, 0 },
	{ "_collate_lookup", 0, 0 },
	{ "_collate_wxfrm", 0, 0 },
	{ "_collate_sxfrm", 0, 0 },
	{ "__collate_equiv_value", 0, 0 },
	{ "__collate_collating_symbol", 0, 0 },
	{ "__collate_equiv_class", 0, 0 },
	{ "__collate_equiv_match", 0, 0 },
};

enum {
	S_SUBST = 0, S_CHAIN, S_LARGE, S_XFRM, S_LOOKUP, S_WXFRM, S_SXFRM,
	S_EQVAL, S_CSYM, S_EQCLASS, S_EQMATCH, S_COUNT
};

static void
note(int which, bool ok, const char *fmt, ...)
{
	Stat &s = stats[which];

	s.cases++;
	if (ok)
		return;
	s.fails++;
	if (s.fails <= 8) {
		va_list ap;
		fprintf(stdout, "  MISMATCH %s: ", s.name);
		va_start(ap, fmt);
		vfprintf(stdout, fmt, ap);
		va_end(ap);
		fputc('\n', stdout);
		fflush(stdout);
	}
}

/* ------------------------------------------------------------------ */
/* deterministic PRNG						      */
/* ------------------------------------------------------------------ */

struct Rng {
	uint64_t s;

	explicit Rng(uint64_t seed) : s(seed ^ 0x9e3779b97f4a7c15ull) {}

	uint32_t u32()
	{
		s = s * 6364136223846793005ull + 1442695040888963407ull;
		uint64_t x = s;
		x ^= x >> 31;
		x *= 0xd6e8feb86659fd93ull;
		x ^= x >> 32;
		return (uint32_t)x;
	}
	uint32_t below(uint32_t n) { return n == 0 ? 0 : u32() % n; }
	int range(int lo, int hi) { return lo + (int)below((uint32_t)(hi - lo + 1)); }
	bool chance(int pct) { return (int)below(100) < pct; }
};

/* ------------------------------------------------------------------ */
/* collate table fixtures					      */
/* ------------------------------------------------------------------ */

struct Fixture {
	P::collate_info_t info;
	P::collate_char_t charpri[256];
	std::vector<P::collate_large_t> large;
	std::vector<P::collate_chain_t> chain;
	std::vector<P::collate_subst_t> subst[P::COLL_WEIGHTS_MAX];
	P::xlocale_collate table;
	P::_xlocale loc;
	std::vector<wchar_t> alpha;	/* interesting input characters */
	std::vector<int32_t> pris;	/* priorities that occur in the table */
	int ndir;
	const char *tag;

	Fixture()
	{
		memset(&info, 0, sizeof(info));
		memset(charpri, 0, sizeof(charpri));
		memset(&table, 0, sizeof(table));
		memset(&loc, 0, sizeof(loc));
		ndir = 1;
		tag = "";
	}

	void wire()
	{
		info.chain_count = (int32_t)chain.size();
		info.large_count = (int32_t)large.size();
		for (int z = 0; z < NWEIGHT; z++)
			info.subst_count[z] = (int32_t)subst[z].size();
		table.info = &info;
		table.char_pri_table = charpri;
		table.large_pri_table = large.empty() ? nullptr : large.data();
		table.chain_pri_table = chain.empty() ? nullptr : chain.data();
		for (int z = 0; z < NWEIGHT; z++)
			table.subst_table[z] =
			    subst[z].empty() ? nullptr : subst[z].data();
		table.map = nullptr;
		table.maplen = 0;
		loc.components[P::XLC_COLLATE] =
		    (P::xlocale_component *)&table;
	}

	void activate()
	{
		P::set_collate(&table);
		ref_set_collate(&table);
	}
};

static std::vector<std::unique_ptr<Fixture>> fixtures;

static int32_t
genpri(Rng &r, Fixture &f, int w)
{
	switch (r.below(20)) {
	case 0:
	case 1:
		return (0);
	case 2:
		return (-r.range(1, 60));
	case 3:
	case 4:
	case 5:
		if (!f.subst[w].empty())
			return (P::COLLATE_SUBST_PRIORITY |
			    (int32_t)r.below((uint32_t)f.subst[w].size()));
		return (r.range(1, 4000));
	case 6:
		return (P::COLLATE_MAX_PRIORITY);
	case 7:
		return (1);
	default:
		return (r.range(1, 4000));
	}
}

static void
buildSubst(Rng &r, Fixture &f, int flavor)
{
	for (int z = 0; z < NWEIGHT; z++) {
		int n;
		if (flavor == 0)
			n = 0;
		else if (flavor == 2)
			n = 4;
		else
			n = r.chance(70) ? r.range(1, 4) : 0;
		f.subst[z].resize((size_t)n);
		for (int i = 0; i < n; i++) {
			P::collate_subst_t &e = f.subst[z][(size_t)i];
			memset(&e, 0, sizeof(e));
			e.key = P::COLLATE_SUBST_PRIORITY | i;
			int k = r.range(1, 4);
			for (int j = 0; j < k; j++)
				e.pri[j] = r.range(1, 3000);
			if (r.chance(15))
				e.pri[0] = 0;
			else if (r.chance(10))
				e.pri[0] = -r.range(1, 40);
			else if (k > 1 && r.chance(15))
				e.pri[r.range(1, k - 1)] = -r.range(1, 40);
			/* e.pri[k] is still 0: the list terminator. */
		}
	}
}

static void
buildLarge(Rng &r, Fixture &f, int flavor)
{
	static const int32_t cand[] = {
		0x41, 0xfe, 0xff, 0x100, 0x101, 0x102, 0x1ff, 0x200, 0x2ff,
		0x300, 0x1000, 0x1234, 0x2000, 0x10000
	};
	int n = (flavor == 0) ? 0 : r.range(0, 9);
	std::vector<int32_t> vals;
	for (int i = 0; i < n; i++) {
		int32_t v = cand[r.below((uint32_t)(sizeof(cand) /
		    sizeof(cand[0])))];
		if (std::find(vals.begin(), vals.end(), v) == vals.end())
			vals.push_back(v);
	}
	std::sort(vals.begin(), vals.end());
	f.large.resize(vals.size());
	for (size_t i = 0; i < vals.size(); i++) {
		memset(&f.large[i], 0, sizeof(f.large[i]));
		f.large[i].val = vals[i];
		for (int w = 0; w < NWEIGHT; w++)
			f.large[i].pri.pri[w] = genpri(r, f, w);
	}
}

static void
buildChains(Rng &r, Fixture &f, int flavor)
{
	static const wchar_t pool[] = { L'a', L'b', L'c', L'd', L'e', 0x100,
	    0x101 };
	int n = (flavor == 0) ? 0 : r.range(0, 12);
	std::vector<std::vector<wchar_t>> strs;
	for (int i = 0; i < n; i++) {
		int l = r.range(1, r.chance(20) ? 5 : 3);
		std::vector<wchar_t> sv;
		for (int j = 0; j < l; j++)
			sv.push_back(pool[r.below((uint32_t)(sizeof(pool) /
			    sizeof(pool[0])))]);
		bool dup = false;
		for (size_t k = 0; k < strs.size(); k++)
			if (strs[k] == sv)
				dup = true;
		if (!dup)
			strs.push_back(sv);
	}
	std::sort(strs.begin(), strs.end());
	f.chain.resize(strs.size());
	for (size_t i = 0; i < strs.size(); i++) {
		memset(&f.chain[i], 0, sizeof(f.chain[i]));
		for (size_t j = 0; j < strs[i].size(); j++)
			f.chain[i].str[j] = strs[i][j];
		for (int w = 0; w < NWEIGHT; w++)
			f.chain[i].pri[w] = genpri(r, f, w);
	}
}

static std::unique_ptr<Fixture>
genFixture(Rng &r, int flavor)
{
	static const int32_t pcs[] = { 0, 1, 5, 63, 64, 65, 4095, 4096,
	    1000000, 0x7fffffff, -1 };
	auto fp = std::make_unique<Fixture>();
	Fixture &f = *fp;

	f.ndir = (flavor == 0) ? 1 : r.range(1, 4);
	f.info.directive_count = (uint8_t)f.ndir;
	for (int z = 0; z < NWEIGHT; z++) {
		int d;
		if (flavor == 0) {
			d = P::DIRECTIVE_FORWARD;
		} else {
			d = r.chance(30) ? P::DIRECTIVE_BACKWARD :
			    P::DIRECTIVE_FORWARD;
			if (r.chance(50))
				d |= P::DIRECTIVE_POSITION;
			if (r.chance(45))
				d |= P::DIRECTIVE_UNDEFINED;
		}
		f.info.directive[z] = (uint8_t)d;
		f.info.pri_count[z] = (flavor == 0) ? 64 :
		    pcs[r.below((uint32_t)(sizeof(pcs) / sizeof(pcs[0])))];
		switch (r.below(5)) {
		case 0:
			f.info.undef_pri[z] = 0;
			break;
		case 1:
			f.info.undef_pri[z] = -r.range(1, 50);
			break;
		default:
			f.info.undef_pri[z] = r.range(1, 3000);
			break;
		}
	}
	f.info.chain_max_len = (uint8_t)((flavor == 0) ? 1 : r.range(0, 6));
	f.info.flags = 0;

	buildSubst(r, f, flavor);
	buildLarge(r, f, flavor);
	buildChains(r, f, flavor);

	for (int c = 0; c < 256; c++)
		for (int w = 0; w < NWEIGHT; w++)
			f.charpri[c].pri[w] = genpri(r, f, w);

	f.table.__collate_load_error = (flavor == 1) ? 1 : 0;
	f.wire();

	/* Characters worth feeding to the collation routines. */
	static const wchar_t base[] = { 1, L'a', L'b', L'c', L'd', L'e', L'f',
	    L'A', L'0', 0x20, 0x7e, 0x7f, 0x80, 0x81, 0xfd, 0xfe, 0xff, 0x100,
	    0x101, 0x102, 0x2ff, 0x300, 0x1234, 0x9999, 0x10ffff };
	for (size_t i = 0; i < sizeof(base) / sizeof(base[0]); i++)
		f.alpha.push_back(base[i]);
	for (size_t i = 0; i < f.large.size(); i++)
		f.alpha.push_back((wchar_t)f.large[i].val);
	for (size_t i = 0; i < f.chain.size(); i++)
		f.alpha.push_back(f.chain[i].str[0]);

	/* Priorities that a caller might legitimately search for. */
	f.pris.push_back(0);
	f.pris.push_back(1);
	f.pris.push_back(-1);
	f.pris.push_back(P::COLLATE_MAX_PRIORITY);
	for (int c = 0; c < 256; c += 7)
		f.pris.push_back(f.charpri[c].pri[0]);
	for (size_t i = 0; i < f.chain.size(); i++) {
		f.pris.push_back(f.chain[i].pri[0]);
		f.pris.push_back(-f.chain[i].pri[0]);
	}
	for (size_t i = 0; i < f.large.size(); i++)
		f.pris.push_back(f.large[i].pri.pri[0]);
	return (fp);
}

static void
buildFixtures()
{
	Rng r(0xC0FFEEull);
	static const char *tags[] = { "empty", "load-error", "all-subst" };

	for (int i = 0; i < 3; i++) {
		auto f = genFixture(r, i);
		f->tag = tags[i];
		fixtures.push_back(std::move(f));
	}
	for (int i = 0; i < 29; i++) {
		auto f = genFixture(r, 3 + i);
		f->tag = "random";
		fixtures.push_back(std::move(f));
	}
}

static Fixture &
pickFixture(Rng &r, bool allowLoadError)
{
	for (;;) {
		Fixture &f = *fixtures[r.below((uint32_t)fixtures.size())];
		if (!allowLoadError && f.table.__collate_load_error)
			continue;
		return (f);
	}
}

/* random NUL-terminated wide string built from the fixture alphabet */
static void
genWide(Rng &r, Fixture &f, wchar_t *out, int maxlen)
{
	int l = r.range(0, maxlen);
	for (int i = 0; i < l; i++) {
		if (r.chance(25) && !f.chain.empty()) {
			/* splice in a whole chain so chainsearch hits */
			const wchar_t *cs =
			    f.chain[r.below((uint32_t)f.chain.size())].str;
			for (int j = 0; j < P::COLLATE_STR_LEN && cs[j] &&
			    i < maxlen; j++)
				out[i++] = cs[j];
			i--;
		} else {
			out[i] = f.alpha[r.below((uint32_t)f.alpha.size())];
		}
	}
	out[l < maxlen ? l : maxlen] = 0;
	out[maxlen] = 0;
}

static const int MBMAX = 48;

static void
genBytes(Rng &r, char *out, size_t *slen, int maxlen, bool utf8)
{
	static const unsigned char pool8[] = { 'a', 'b', 'c', 'd', 'e', 'A',
	    'z', '0', 0x00, 0x01, 0x7f, 0x80, 0xa9, 0xc3, 0xe2, 0xfe, 0xff };
	int l = r.range(0, maxlen);
	int i = 0;
	while (i < l) {
		if (utf8 && r.chance(20) && i + 2 <= l) {
			out[i++] = (char)0xc3;
			out[i++] = (char)0xa9;
		} else if (utf8 && r.chance(10) && i + 3 <= l) {
			out[i++] = (char)0xe2;
			out[i++] = (char)0x82;
			out[i++] = (char)0xac;
		} else {
			out[i++] = (char)pool8[r.below((uint32_t)(
			    sizeof(pool8) / sizeof(pool8[0])))];
		}
	}
	*slen = (size_t)l;
	for (int k = l; k < MBMAX; k++)
		out[k] = (char)0x7f;
}

/* ------------------------------------------------------------------ */
/* substsearch							      */
/* ------------------------------------------------------------------ */

static void
oneSubst(Fixture &f, wchar_t key, int pass)
{
	f.activate();
	const int32_t *a = P::substsearch(&f.table, key, pass);
	const int32_t *b = ref_substsearch(&f.table, key, pass);
	note(S_SUBST, a == b, "key=%#x pass=%d port=%p ref=%p", (unsigned)key,
	    pass, (const void *)a, (const void *)b);
}

static void
phaseSubst(unsigned long iters)
{
	/* hand written */
	for (size_t fi = 0; fi < fixtures.size(); fi++) {
		Fixture &f = *fixtures[fi];
		for (int pass = 0; pass < NWEIGHT; pass++) {
			int n = f.info.subst_count[pass];
			oneSubst(f, 0, pass);
			if (n > 0) {
				oneSubst(f, (wchar_t)(n - 1), pass);
				oneSubst(f, (wchar_t)(P::COLLATE_SUBST_PRIORITY |
				    0), pass);
				oneSubst(f, (wchar_t)(P::COLLATE_SUBST_PRIORITY |
				    (n - 1)), pass);
			} else {
				oneSubst(f, (wchar_t)P::COLLATE_SUBST_PRIORITY,
				    pass);
				oneSubst(f, 12345, pass);
			}
		}
	}

	Rng r(0x51B57ull);
	for (unsigned long it = 0; it < iters; it++) {
		Fixture &f = pickFixture(r, true);
		int pass = r.range(0, NWEIGHT - 1);
		int n = f.info.subst_count[pass];
		wchar_t key;
		if (n > 0)
			key = (wchar_t)((r.chance(60) ?
			    P::COLLATE_SUBST_PRIORITY : 0) |
			    (int)r.below((uint32_t)n));
		else
			key = (wchar_t)(r.chance(50) ?
			    P::COLLATE_SUBST_PRIORITY : 0);
		oneSubst(f, key, pass);
	}
}

/* ------------------------------------------------------------------ */
/* chainsearch							      */
/* ------------------------------------------------------------------ */

static void
oneChain(Fixture &f, const wchar_t *key)
{
	f.activate();
	int la = -12345, lb = -12345;
	P::collate_chain_t *a = P::chainsearch(&f.table, key, &la);
	P::collate_chain_t *b = ref_chainsearch(&f.table, key, &lb);
	long ia = a ? (a - f.chain.data()) : -1;
	long ib = b ? (b - f.chain.data()) : -1;
	note(S_CHAIN, a == b && la == lb, "first=%#x port=(%ld,%d) ref=(%ld,%d)",
	    (unsigned)key[0], ia, la, ib, lb);
}

static void
phaseChain(unsigned long iters)
{
	wchar_t buf[40];

	for (size_t fi = 0; fi < fixtures.size(); fi++) {
		Fixture &f = *fixtures[fi];
		buf[0] = 0;
		oneChain(f, buf);
		for (size_t i = 0; i < f.chain.size(); i++) {
			/* exact */
			oneChain(f, f.chain[i].str);
			/* exact plus a trailing character */
			size_t l = wcslen(f.chain[i].str);
			memcpy(buf, f.chain[i].str, l * sizeof(wchar_t));
			buf[l] = L'z';
			buf[l + 1] = 0;
			oneChain(f, buf);
			/* one character short */
			if (l > 1) {
				memcpy(buf, f.chain[i].str,
				    (l - 1) * sizeof(wchar_t));
				buf[l - 1] = 0;
				oneChain(f, buf);
			}
			/* first char bumped up and down */
			memcpy(buf, f.chain[i].str, (l + 1) * sizeof(wchar_t));
			buf[0] = (wchar_t)(f.chain[i].str[0] + 1);
			oneChain(f, buf);
			buf[0] = (wchar_t)(f.chain[i].str[0] - 1);
			oneChain(f, buf);
		}
		static const wchar_t misc[][3] = { { 1, 0, 0 }, { 0xff, 0, 0 },
		    { 0x100, 0, 0 }, { L'a', 0, 0 }, { L'z', L'z', 0 } };
		for (size_t i = 0; i < 5; i++)
			oneChain(f, misc[i]);
	}

	Rng r(0xC4A1Full);
	for (unsigned long it = 0; it < iters; it++) {
		Fixture &f = pickFixture(r, true);
		genWide(r, f, buf, 8);
		oneChain(f, buf);
	}
}

/* ------------------------------------------------------------------ */
/* largesearch							      */
/* ------------------------------------------------------------------ */

static void
oneLarge(Fixture &f, wchar_t key)
{
	f.activate();
	P::collate_large_t *a = P::largesearch(&f.table, key);
	P::collate_large_t *b = ref_largesearch(&f.table, key);
	note(S_LARGE, a == b, "key=%#x port=%p ref=%p", (unsigned)key,
	    (const void *)a, (const void *)b);
}

static void
phaseLarge(unsigned long iters)
{
	for (size_t fi = 0; fi < fixtures.size(); fi++) {
		Fixture &f = *fixtures[fi];
		oneLarge(f, 0);
		oneLarge(f, 1);
		oneLarge(f, 0xff);
		oneLarge(f, 0x7fffffff);
		for (size_t i = 0; i < f.large.size(); i++) {
			oneLarge(f, (wchar_t)f.large[i].val);
			oneLarge(f, (wchar_t)(f.large[i].val - 1));
			oneLarge(f, (wchar_t)(f.large[i].val + 1));
		}
	}

	Rng r(0x1A26Eull);
	for (unsigned long it = 0; it < iters; it++) {
		Fixture &f = pickFixture(r, true);
		wchar_t key;
		if (!f.large.empty() && r.chance(50))
			key = (wchar_t)(f.large[r.below((uint32_t)
			    f.large.size())].val + r.range(-1, 1));
		else
			key = f.alpha[r.below((uint32_t)f.alpha.size())];
		oneLarge(f, key);
	}
}

/* ------------------------------------------------------------------ */
/* xfrm								      */
/* ------------------------------------------------------------------ */

static void
oneXfrm(Fixture &f, int pri, int pass)
{
	unsigned char ba[64], bb[64];

	f.activate();
	memset(ba, 0x7f, sizeof(ba));
	memset(bb, 0x7f, sizeof(bb));
	int ra = P::xfrm(&f.table, ba + 8, pri, pass);
	int rb = ref_xfrm(&f.table, bb + 8, pri, pass);
	bool ok = (ra == rb) && memcmp(ba, bb, sizeof(ba)) == 0;
	note(S_XFRM, ok, "pri=%d pass=%d port=%d ref=%d buf%s", pri, pass, ra,
	    rb, memcmp(ba, bb, sizeof(ba)) ? " differs" : " equal");
}

static void
phaseXfrm(unsigned long iters)
{
	static const int pris[] = { 0, 1, 2, 63, 64, 65, -1, -64, 0x7fffffff,
	    (-0x7fffffff - 1), 12345, 0x3f, 0x40 };

	for (size_t fi = 0; fi < fixtures.size(); fi++) {
		Fixture &f = *fixtures[fi];
		for (int pass = 0; pass < NWEIGHT; pass++)
			for (size_t i = 0; i < sizeof(pris) / sizeof(pris[0]);
			    i++)
				oneXfrm(f, pris[i], pass);
	}

	Rng r(0x7F2A5ull);
	for (unsigned long it = 0; it < iters; it++) {
		Fixture &f = pickFixture(r, true);
		int pass = r.range(0, NWEIGHT - 1);
		int pri = r.chance(30) ?
		    pris[r.below((uint32_t)(sizeof(pris) / sizeof(pris[0])))] :
		    (int)r.u32();
		oneXfrm(f, pri, pass);
	}
}

/* ------------------------------------------------------------------ */
/* _collate_lookup						      */
/* ------------------------------------------------------------------ */

static void
oneLookup(Fixture &f, const wchar_t *t, int which, const int *state0)
{
	f.activate();
	int lena = -777, prja = -777;
	int lenb = -777, prjb = -777;
	const int *sa = state0;
	const int *sb = state0;
	P::_collate_lookup(&f.table, t, &lena, &prja, which, &sa);
	ref__collate_lookup(&f.table, t, &lenb, &prjb, which, &sb);
	note(S_LOOKUP, lena == lenb && prja == prjb && sa == sb,
	    "t=%#x which=%d st0=%p port=(%d,%d,%p) ref=(%d,%d,%p)",
	    (unsigned)t[0], which, (const void *)state0, lena, prja,
	    (const void *)sa, lenb, prjb, (const void *)sb);
}

static const int *
pickState(Rng &r, Fixture &f)
{
	if (r.chance(45))
		return (nullptr);
	std::vector<int> avail;
	for (int z = 0; z < NWEIGHT; z++)
		if (!f.subst[z].empty())
			avail.push_back(z);
	if (avail.empty())
		return (nullptr);
	int z = avail[r.below((uint32_t)avail.size())];
	size_t i = r.below((uint32_t)f.subst[z].size());
	int j = r.range(0, 5);
	return ((const int *)&f.subst[z][i].pri[j]);
}

static void
phaseLookup(unsigned long iters)
{
	wchar_t buf[40];

	for (size_t fi = 0; fi < fixtures.size(); fi++) {
		Fixture &f = *fixtures[fi];
		for (int which = 0; which <= f.ndir + 1; which++) {
			for (size_t i = 0; i < f.alpha.size(); i++) {
				buf[0] = f.alpha[i];
				buf[1] = L'a';
				buf[2] = 0;
				oneLookup(f, buf, which, nullptr);
			}
			for (size_t i = 0; i < f.chain.size(); i++)
				oneLookup(f, f.chain[i].str, which, nullptr);
			for (int z = 0; z < NWEIGHT; z++) {
				if (f.subst[z].empty())
					continue;
				buf[0] = L'a';
				buf[1] = 0;
				for (int j = 0; j < 5; j++)
					oneLookup(f, buf, which,
					    (const int *)&f.subst[z][0].pri[j]);
			}
		}
	}

	Rng r(0x10C0Aull);
	for (unsigned long it = 0; it < iters; it++) {
		Fixture &f = pickFixture(r, true);
		genWide(r, f, buf, 6);
		if (buf[0] == 0) {
			buf[0] = f.alpha[r.below((uint32_t)f.alpha.size())];
			buf[1] = 0;
		}
		int which = r.range(0, f.ndir + 1);
		oneLookup(f, buf, which, pickState(r, f));
	}
}

/* ------------------------------------------------------------------ */
/* _collate_wxfrm						      */
/* ------------------------------------------------------------------ */

static const int WBUF = 96;

static void
oneWxfrm(Fixture &f, const wchar_t *src, size_t room)
{
	wchar_t ba[WBUF], bb[WBUF];

	f.activate();
	memset(ba, 0x7f, sizeof(ba));
	memset(bb, 0x7f, sizeof(bb));
	errno = 0;
	size_t ra = P::_collate_wxfrm(&f.table, src, ba, room);
	int ea = errno;
	errno = 0;
	size_t rb = ref__collate_wxfrm(&f.table, src, bb, room);
	int eb = errno;
	bool bufok = memcmp(ba, bb, sizeof(ba)) == 0;
	note(S_WXFRM, ra == rb && ea == eb && bufok,
	    "src[0]=%#x room=%zu port=(%zu,e%d) ref=(%zu,e%d)%s",
	    (unsigned)src[0], room, ra, ea, rb, eb,
	    bufok ? "" : " BUFFER DIFFERS");
}

static void
phaseWxfrm(unsigned long iters)
{
	wchar_t src[40];

	for (size_t fi = 0; fi < fixtures.size(); fi++) {
		Fixture &f = *fixtures[fi];
		if (f.table.__collate_load_error)
			continue;
		static const size_t rooms[] = { 0, 1, 2, 3, 7, 32, WBUF };
		for (size_t ri = 0; ri < sizeof(rooms) / sizeof(rooms[0]);
		    ri++) {
			src[0] = 0;
			oneWxfrm(f, src, rooms[ri]);
			for (size_t i = 0; i < f.alpha.size(); i++) {
				src[0] = f.alpha[i];
				src[1] = 0;
				oneWxfrm(f, src, rooms[ri]);
				src[1] = f.alpha[(i + 1) % f.alpha.size()];
				src[2] = 0;
				oneWxfrm(f, src, rooms[ri]);
			}
			for (size_t i = 0; i < f.chain.size(); i++)
				oneWxfrm(f, f.chain[i].str, rooms[ri]);
		}
	}

	Rng r(0x3B71Aull);
	for (unsigned long it = 0; it < iters; it++) {
		Fixture &f = pickFixture(r, false);
		genWide(r, f, src, r.chance(30) ? 2 : 10);
		size_t room;
		switch (r.below(5)) {
		case 0:
			room = 0;
			break;
		case 1:
			room = (size_t)r.range(1, 4);
			break;
		case 2:
			room = WBUF;
			break;
		default:
			room = (size_t)r.range(0, WBUF);
			break;
		}
		oneWxfrm(f, src, room);
	}
}

/* ------------------------------------------------------------------ */
/* _collate_sxfrm						      */
/* ------------------------------------------------------------------ */

static const int CBUF = 320;

static void
oneSxfrm(Fixture &f, const wchar_t *src, size_t room)
{
	char ba[CBUF], bb[CBUF];

	f.activate();
	memset(ba, 0x7f, sizeof(ba));
	memset(bb, 0x7f, sizeof(bb));
	errno = 0;
	size_t ra = P::_collate_sxfrm(&f.table, src, ba, room);
	int ea = errno;
	errno = 0;
	size_t rb = ref__collate_sxfrm(&f.table, src, bb, room);
	int eb = errno;
	bool bufok = memcmp(ba, bb, sizeof(ba)) == 0;
	note(S_SXFRM, ra == rb && ea == eb && bufok,
	    "src[0]=%#x room=%zu port=(%zu,e%d) ref=(%zu,e%d)%s",
	    (unsigned)src[0], room, ra, ea, rb, eb,
	    bufok ? "" : " BUFFER DIFFERS");
}

static void
phaseSxfrm(unsigned long iters)
{
	wchar_t src[40];

	for (size_t fi = 0; fi < fixtures.size(); fi++) {
		Fixture &f = *fixtures[fi];
		if (f.table.__collate_load_error)
			continue;
		static const size_t rooms[] = { 0, 1, 2, 5, 6, 7, 64, CBUF };
		for (size_t ri = 0; ri < sizeof(rooms) / sizeof(rooms[0]);
		    ri++) {
			src[0] = 0;
			oneSxfrm(f, src, rooms[ri]);
			for (size_t i = 0; i < f.alpha.size(); i++) {
				src[0] = f.alpha[i];
				src[1] = 0;
				oneSxfrm(f, src, rooms[ri]);
				src[1] = f.alpha[(i + 1) % f.alpha.size()];
				src[2] = 0;
				oneSxfrm(f, src, rooms[ri]);
			}
			for (size_t i = 0; i < f.chain.size(); i++)
				oneSxfrm(f, f.chain[i].str, rooms[ri]);
		}
	}

	Rng r(0x5F71Aull);
	for (unsigned long it = 0; it < iters; it++) {
		Fixture &f = pickFixture(r, false);
		genWide(r, f, src, r.chance(30) ? 2 : 8);
		size_t room;
		switch (r.below(5)) {
		case 0:
			room = 0;
			break;
		case 1:
			room = (size_t)r.range(1, 8);
			break;
		case 2:
			room = CBUF;
			break;
		default:
			room = (size_t)r.range(0, CBUF);
			break;
		}
		oneSxfrm(f, src, room);
	}
}

/* ------------------------------------------------------------------ */
/* __collate_equiv_value					      */
/* ------------------------------------------------------------------ */

static void
oneEqval(Fixture &f, const wchar_t *str, size_t len, bool nullLocale)
{
	f.activate();
	P::_xlocale *lp = nullLocale ? nullptr : &f.loc;
	int a = P::__collate_equiv_value(lp, str, len);
	int b = ref__collate_equiv_value(lp, str, len);
	note(S_EQVAL, a == b, "str[0]=%#x len=%zu nl=%d port=%d ref=%d",
	    (unsigned)str[0], len, (int)nullLocale, a, b);
}

static void
phaseEqval(unsigned long iters)
{
	wchar_t src[64];

	for (size_t fi = 0; fi < fixtures.size(); fi++) {
		Fixture &f = *fixtures[fi];
		for (int i = 0; i < 40; i++)
			src[i] = L'a';
		src[40] = 0;
		static const size_t lens[] = { 0, 1, 2, 22, 23, 24, 25, 40 };
		for (size_t li = 0; li < sizeof(lens) / sizeof(lens[0]); li++) {
			oneEqval(f, src, lens[li], false);
			oneEqval(f, src, lens[li], true);
		}
		for (size_t i = 0; i < f.alpha.size(); i++) {
			src[0] = f.alpha[i];
			src[1] = 0;
			oneEqval(f, src, 1, false);
			oneEqval(f, src, 2, false);
		}
		for (size_t i = 0; i < f.chain.size(); i++) {
			size_t l = wcslen(f.chain[i].str);
			memcpy(src, f.chain[i].str, (l + 1) * sizeof(wchar_t));
			oneEqval(f, src, l, false);
			oneEqval(f, src, l + 1, false);
			if (l > 1)
				oneEqval(f, src, l - 1, false);
		}
	}

	Rng r(0x6E9A1ull);
	for (unsigned long it = 0; it < iters; it++) {
		Fixture &f = pickFixture(r, true);
		genWide(r, f, src, 30);
		/* make sure there are always >= 26 readable wchars */
		size_t have = wcslen(src);
		for (size_t i = have; i < 32; i++)
			src[i] = f.alpha[r.below((uint32_t)f.alpha.size())];
		src[32] = 0;
		if (r.chance(20))
			src[r.below(26)] = 0;
		size_t len;
		switch (r.below(6)) {
		case 0:
			len = 0;
			break;
		case 1:
			len = 1;
			break;
		case 2:
			len = 23;
			break;
		case 3:
			len = 24;
			break;
		case 4:
			len = 25;
			break;
		default:
			len = (size_t)r.range(1, 23);
			break;
		}
		oneEqval(f, src, len, r.chance(30));
	}
}

/* ------------------------------------------------------------------ */
/* __collate_collating_symbol					      */
/* ------------------------------------------------------------------ */

static const int DBUF = 24;

static void
oneCsym(Fixture &f, const char *src, size_t slen, size_t dlen, bool useps)
{
	wchar_t da[DBUF], db[DBUF];
	mbstate_t sa, sb;

	f.activate();
	memset(da, 0x7f, sizeof(da));
	memset(db, 0x7f, sizeof(db));
	memset(&sa, 0, sizeof(sa));
	memset(&sb, 0, sizeof(sb));
	size_t ra = P::__collate_collating_symbol(da, dlen, src, slen,
	    useps ? &sa : nullptr);
	size_t rb = ref__collate_collating_symbol(db, dlen, src, slen,
	    useps ? &sb : nullptr);
	bool bufok = memcmp(da, db, sizeof(da)) == 0;
	bool stok = memcmp(&sa, &sb, sizeof(sa)) == 0;
	note(S_CSYM, ra == rb && bufok && stok,
	    "slen=%zu dlen=%zu port=%zd ref=%zd%s%s", slen, dlen, (ssize_t)ra,
	    (ssize_t)rb, bufok ? "" : " BUFFER DIFFERS",
	    stok ? "" : " MBSTATE DIFFERS");
}

static void
phaseCsym(unsigned long iters, bool utf8)
{
	char src[MBMAX + 8];

	if (!utf8) {
		for (size_t fi = 0; fi < fixtures.size(); fi++) {
			Fixture &f = *fixtures[fi];
			static const char *lits[] = { "", "a", "ab", "abc",
			    "abcd", "\x80", "\xff", "a\x00" "b", "\x00" "ab",
			    "abcdefghijklmnopqrstuvwxyzabcdef" };
			static const size_t lens[] = { 0, 1, 2, 3, 4, 1, 1, 3,
			    3, 32 };
			for (size_t i = 0; i < 10; i++)
				for (size_t dlen = 0; dlen <= 6; dlen++) {
					oneCsym(f, lits[i], lens[i], dlen,
					    true);
					oneCsym(f, lits[i], lens[i], dlen,
					    false);
				}
			for (size_t i = 0; i < f.chain.size(); i++) {
				char b[32];
				size_t l = 0;
				bool ascii = true;
				for (size_t j = 0; f.chain[i].str[j]; j++) {
					if (f.chain[i].str[j] > 0x7f)
						ascii = false;
					b[l++] = (char)f.chain[i].str[j];
				}
				if (!ascii)
					continue;
				for (size_t dlen = 0; dlen <= 6; dlen++)
					oneCsym(f, b, l, dlen, true);
			}
		}
	}

	Rng r(utf8 ? 0xB0B0Bull : 0xA1A1Aull);
	for (unsigned long it = 0; it < iters; it++) {
		Fixture &f = pickFixture(r, true);
		size_t slen;
		genBytes(r, src, &slen, 12, utf8);
		size_t dlen = (size_t)r.range(0, 8);
		oneCsym(f, src, slen, dlen, utf8 ? true : r.chance(70));
	}
}

/* ------------------------------------------------------------------ */
/* __collate_equiv_class					      */
/* ------------------------------------------------------------------ */

static void
oneEqclass(Fixture &f, const char *src, size_t slen, bool useps)
{
	mbstate_t sa, sb;

	f.activate();
	memset(&sa, 0, sizeof(sa));
	memset(&sb, 0, sizeof(sb));
	int a = P::__collate_equiv_class(src, slen, useps ? &sa : nullptr);
	int b = ref__collate_equiv_class(src, slen, useps ? &sb : nullptr);
	bool stok = memcmp(&sa, &sb, sizeof(sa)) == 0;
	note(S_EQCLASS, a == b && stok, "slen=%zu port=%d ref=%d%s", slen, a, b,
	    stok ? "" : " MBSTATE DIFFERS");
}

static void
phaseEqclass(unsigned long iters, bool utf8)
{
	char src[MBMAX + 8];

	if (!utf8) {
		for (size_t fi = 0; fi < fixtures.size(); fi++) {
			Fixture &f = *fixtures[fi];
			static const char *lits[] = { "", "a", "ab", "abc",
			    "abcd", "\x80", "\xff", "a\x00" "b",
			    "abcdefghijklmnopqrstuvwxyzabcdef" };
			static const size_t lens[] = { 0, 1, 2, 3, 4, 1, 1, 3,
			    32 };
			for (size_t i = 0; i < 9; i++) {
				oneEqclass(f, lits[i], lens[i], true);
				oneEqclass(f, lits[i], lens[i], false);
			}
			for (size_t i = 0; i < f.chain.size(); i++) {
				char b[32];
				size_t l = 0;
				bool ascii = true;
				for (size_t j = 0; f.chain[i].str[j]; j++) {
					if (f.chain[i].str[j] > 0x7f)
						ascii = false;
					b[l++] = (char)f.chain[i].str[j];
				}
				if (ascii)
					oneEqclass(f, b, l, true);
			}
			for (int c = 0; c < 256; c++) {
				char b = (char)c;
				oneEqclass(f, &b, 1, true);
			}
		}
	}

	Rng r(utf8 ? 0xD0D0Dull : 0xC1C1Cull);
	for (unsigned long it = 0; it < iters; it++) {
		Fixture &f = pickFixture(r, true);
		size_t slen;
		genBytes(r, src, &slen, 12, utf8);
		oneEqclass(f, src, slen, utf8 ? true : r.chance(70));
	}
}

/* ------------------------------------------------------------------ */
/* __collate_equiv_match					      */
/* ------------------------------------------------------------------ */

static void
oneEqmatch(Fixture &f, int eq, size_t dlen, wchar_t start, const char *src,
    size_t slen, bool useps, bool usedst, bool userlen)
{
	wchar_t da[DBUF], db[DBUF];
	mbstate_t sa, sb;
	size_t rla = 0x5a5a5a, rlb = 0x5a5a5a;

	f.activate();
	memset(da, 0x7f, sizeof(da));
	memset(db, 0x7f, sizeof(db));
	memset(&sa, 0, sizeof(sa));
	memset(&sb, 0, sizeof(sb));
	size_t ra = P::__collate_equiv_match(eq, usedst ? da : nullptr, dlen,
	    start, src, slen, useps ? &sa : nullptr, userlen ? &rla : nullptr);
	size_t rb = ref__collate_equiv_match(eq, usedst ? db : nullptr, dlen,
	    start, src, slen, useps ? &sb : nullptr, userlen ? &rlb : nullptr);
	bool bufok = memcmp(da, db, sizeof(da)) == 0;
	bool stok = memcmp(&sa, &sb, sizeof(sa)) == 0;
	note(S_EQMATCH, ra == rb && rla == rlb && bufok && stok,
	    "eq=%d dlen=%zu start=%#x slen=%zu port=(%zd,%zu) ref=(%zd,%zu)%s%s",
	    eq, dlen, (unsigned)start, slen, (ssize_t)ra, rla, (ssize_t)rb, rlb,
	    bufok ? "" : " BUFFER DIFFERS", stok ? "" : " MBSTATE DIFFERS");
}

static void
phaseEqmatch(unsigned long iters, bool utf8)
{
	char src[MBMAX + 8];

	if (!utf8) {
		for (size_t fi = 0; fi < fixtures.size(); fi++) {
			Fixture &f = *fixtures[fi];
			static const char *lits[] = { "", "a", "ab", "abc",
			    "abcde", "\x80", "\xff", "a\x00" "b" };
			static const size_t lens[] = { 0, 1, 2, 3, 5, 1, 1, 3 };
			static const int eqs[] = { 0, 1, -1, 2 };
			for (size_t i = 0; i < 8; i++)
				for (size_t e = 0; e < 4; e++)
					for (size_t dlen = 0; dlen <= 6;
					    dlen += 2) {
						oneEqmatch(f, eqs[e], dlen, 0,
						    lits[i], lens[i], true,
						    true, true);
						oneEqmatch(f, eqs[e], dlen,
						    L'a', lits[i], lens[i],
						    true, true, true);
					}
			for (size_t i = 0; i < f.chain.size(); i++) {
				char b[32];
				size_t l = 0;
				bool ascii = true;
				for (size_t j = 0; f.chain[i].str[j]; j++) {
					if (f.chain[i].str[j] > 0x7f)
						ascii = false;
					b[l++] = (char)f.chain[i].str[j];
				}
				if (!ascii)
					continue;
				int e = f.chain[i].pri[0];
				if (e < 0)
					e = -e;
				for (size_t dlen = 0; dlen <= 8; dlen++) {
					oneEqmatch(f, e, dlen, 0, b, l, true,
					    true, true);
					oneEqmatch(f, e, dlen, b[0], b + 1,
					    l - 1, true, true, true);
				}
			}
		}
	}

	Rng r(utf8 ? 0xE0E0Eull : 0xF1F1Full);
	for (unsigned long it = 0; it < iters; it++) {
		Fixture &f = pickFixture(r, true);
		size_t slen;
		genBytes(r, src, &slen, 10, utf8);
		int eq;
		if (r.chance(60) && !f.pris.empty())
			eq = f.pris[r.below((uint32_t)f.pris.size())];
		else
			eq = r.range(-3, 3);
		wchar_t start = 0;
		if (r.chance(40))
			start = f.alpha[r.below((uint32_t)f.alpha.size())];
		oneEqmatch(f, eq, (size_t)r.range(0, 8), start, src, slen,
		    utf8 ? true : r.chance(70), r.chance(80), r.chance(80));
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	const unsigned long ITERS = 200000;

	setvbuf(stdout, nullptr, _IOLBF, 0);
	buildFixtures();

	printf("b0156s4: collate.c differential test\n");
	printf("fixtures: %zu\n", fixtures.size());

	printf("phase: substsearch\n");
	phaseSubst(ITERS);
	printf("phase: chainsearch\n");
	phaseChain(ITERS);
	printf("phase: largesearch\n");
	phaseLarge(ITERS);
	printf("phase: xfrm\n");
	phaseXfrm(ITERS);
	printf("phase: _collate_lookup\n");
	phaseLookup(ITERS);
	printf("phase: _collate_wxfrm\n");
	phaseWxfrm(ITERS);
	printf("phase: _collate_sxfrm\n");
	phaseSxfrm(ITERS);
	printf("phase: __collate_equiv_value\n");
	phaseEqval(ITERS);

	/* multibyte routines: single byte locale first */
	if (setlocale(LC_CTYPE, "C") == nullptr) {
		fprintf(stderr, "cannot select the C locale\n");
		return (1);
	}
	printf("phase: multibyte routines in the C locale\n");
	phaseCsym(ITERS / 2, false);
	phaseEqclass(ITERS / 2, false);
	phaseEqmatch(ITERS / 2, false);

	const char *utf8 = nullptr;
	static const char *cands[] = { "C.UTF-8", "en_US.UTF-8", "C.utf8" };
	for (size_t i = 0; i < 3 && utf8 == nullptr; i++)
		if (setlocale(LC_CTYPE, cands[i]) != nullptr)
			utf8 = cands[i];
	if (utf8 != nullptr) {
		printf("phase: multibyte routines in %s\n", utf8);
		phaseCsym(ITERS / 2, true);
		phaseEqclass(ITERS / 2, true);
		phaseEqmatch(ITERS / 2, true);
	} else {
		printf("phase: no UTF-8 locale available; "
		    "running the C locale sweep again\n");
		setlocale(LC_CTYPE, "C");
		phaseCsym(ITERS / 2, false);
		phaseEqclass(ITERS / 2, false);
		phaseEqmatch(ITERS / 2, false);
	}

	unsigned long long totc = 0, totf = 0;
	printf("\n%-30s %12s %12s\n", "function", "cases", "failures");
	printf("%-30s %12s %12s\n", "------------------------------",
	    "------------", "------------");
	for (int i = 0; i < S_COUNT; i++) {
		printf("%-30s %12llu %12llu\n", stats[i].name, stats[i].cases,
		    stats[i].fails);
		totc += stats[i].cases;
		totf += stats[i].fails;
	}
	printf("%-30s %12s %12s\n", "------------------------------",
	    "------------", "------------");
	printf("%-30s %12llu %12llu\n", "TOTAL", totc, totf);
	printf("\n%s\n", totf == 0 ? "PASS" : "FAIL");
	return (totf == 0 ? 0 : 1);
}
