/*
 * Differential harness for batch b0156s4 (collate.c).
 */

import pbsd.lib.libc.locale.b0156s4;

#include <cerrno>
#include <climits>
#include <clocale>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <sys/stat.h>
#include <unistd.h>

namespace P = pbsd::lib_libc_locale::b0156s4;

extern "C" {
typedef struct collate_info {
	uint8_t directive_count;
	uint8_t directive[10];
	uint8_t chain_max_len;
	int32_t pri_count[10];
	int32_t flags;
	int32_t chain_count;
	int32_t large_count;
	int32_t subst_count[10];
	int32_t undef_pri[10];
} collate_info_t;

typedef struct collate_char {
	int32_t pri[10];
} collate_char_t;

typedef struct collate_chain {
	wchar_t str[24];
	int32_t pri[10];
} collate_chain_t;

typedef struct collate_large {
	int32_t val;
	collate_char_t pri;
} collate_large_t;

typedef struct collate_subst {
	int32_t key;
	int32_t pri[24];
} collate_subst_t;

struct xlocale_refcounted {
	long retain_count;
	void (*destructor)(void *);
};

struct xlocale_component {
	struct xlocale_refcounted header;
	char locale[32];
	char version[12];
};

struct xlocale_collate {
	struct xlocale_component header;
	int __collate_load_error;
	char *map;
	size_t maplen;
	collate_info_t *info;
	collate_char_t *char_pri_table;
	collate_large_t *large_pri_table;
	collate_chain_t *chain_pri_table;
	collate_subst_t *subst_table[10];
};

struct _xlocale {
	struct xlocale_refcounted header;
	struct xlocale_component *components[6];
	int monetary_locale_changed;
	int using_monetary_locale;
	int numeric_locale_changed;
	int using_numeric_locale;
	int using_time_locale;
	int using_messages_locale;
	struct lconv lconv;
	char *csym;
};

typedef struct _xlocale *pbsd_locale_t;

extern char *_PathLocale;
extern struct _xlocale ref_global_locale;
extern struct _xlocale ref_C_locale;
extern struct xlocale_collate ref___xlocale_global_collate;
extern struct xlocale_collate ref___xlocale_C_collate;
extern struct xlocale_collate ref___xlocale_POSIX_collate;
extern struct xlocale_collate ref___xlocale_CUTF8_collate;

void *ref___collate_load(const char *, pbsd_locale_t);
int ref___collate_load_tables(const char *);
void ref__collate_lookup(struct xlocale_collate *, const wchar_t *, int *, int *,
    int, const int **);
size_t ref__collate_wxfrm(struct xlocale_collate *, const wchar_t *, wchar_t *,
    size_t);
size_t ref__collate_sxfrm(struct xlocale_collate *, const wchar_t *, char *,
    size_t);
int ref___collate_equiv_value(pbsd_locale_t, const wchar_t *, size_t);
size_t ref___collate_collating_symbol(wchar_t *, size_t, const char *, size_t,
    mbstate_t *);
int ref___collate_equiv_class(const char *, size_t, mbstate_t *);
size_t ref___collate_equiv_match(int, wchar_t *, size_t, wchar_t, const char *,
    size_t, mbstate_t *, size_t *);
}

enum {
	F_LOAD, F_LOAD_TABLES, F_LOOKUP, F_WXFRM, F_SXFRM,
	F_EQUIV_VALUE, F_COLL_SYM, F_EQUIV_CLASS, F_EQUIV_MATCH, F_COUNT
};

static const char *const fnames[F_COUNT] = {
	"__collate_load", "__collate_load_tables", "_collate_lookup",
	"_collate_wxfrm", "_collate_sxfrm", "__collate_equiv_value",
	"__collate_collating_symbol", "__collate_equiv_class", "__collate_equiv_match"
};

static long long ncase[F_COUNT];
static long long nfail[F_COUNT];
static int nprint[F_COUNT];

static constexpr unsigned char GUARD = 0x7f;
static constexpr wchar_t WGUARD = (wchar_t)0x7f7f;
static constexpr long long SWEEP = 200000;
static constexpr int PBSD_COLL_WEIGHTS_MAX = 10;
static constexpr int COLLATE_STR_LEN = 24;
static constexpr int COLLATE_FMT_VERSION_LEN = 12;
static constexpr int XLOCALE_DEF_VERSION_LEN = 12;
static constexpr int XLC_COLLATE = 0;

static constexpr int DIRECTIVE_FORWARD = 0x01;
static constexpr int DIRECTIVE_BACKWARD = 0x02;
static constexpr int DIRECTIVE_POSITION = 0x04;
static constexpr int DIRECTIVE_UNDEFINED = 0x08;
static constexpr int COLLATE_MAX_PRIORITY = 0x7fffffff;
static constexpr int COLLATE_SUBST_PRIORITY = 0x40000000;
static constexpr int IGNORE_EQUIV_CLASS = 1;
static constexpr int _LDP_LOADED = 0;
static constexpr int _LDP_ERROR = -1;
static constexpr int _LDP_CACHE = 1;

static uint64_t rng = 0xB01564001ULL;

static uint64_t rnd()
{
	rng ^= rng << 13;
	rng ^= rng >> 7;
	rng ^= rng << 17;
	return (rng);
}

static uint32_t ru32(uint32_t m)
{
	return ((uint32_t)(rnd() % (m ? m : 1)));
}

static void report(int f, const char *why)
{
	nfail[f]++;
	if (nprint[f]++ < 8)
		std::printf("  FAIL %-28s : %s\n", fnames[f], why);
}

static void bump(int f)
{
	ncase[f]++;
}

static void fail(int f, const char *why)
{
	bump(f);
	report(f, why);
}

static void ok(int f)
{
	bump(f);
}

struct PortBacking {
	P::collate_info_t info{};
	P::collate_char_t chars[256]{};
	P::collate_chain_t chains[8]{};
	P::collate_large_t larges[8]{};
	P::collate_subst_t subst0[4]{};
	P::collate_subst_t subst1[2]{};
};

struct RefBacking {
	collate_info_t info{};
	collate_char_t chars[256]{};
	collate_chain_t chains[8]{};
	collate_large_t larges[8]{};
	collate_subst_t subst0[4]{};
	collate_subst_t subst1[2]{};
};

struct Fixture {
	PortBacking pback;
	P::xlocale_collate ptab{};
	RefBacking rback;
	xlocale_collate rtab{};
};

static char tmp_root[256];

static void wset(wchar_t *d, const wchar_t *s)
{
	std::wcsncpy(d, s, COLLATE_STR_LEN - 1);
	d[COLLATE_STR_LEN - 1] = 0;
}

static void init_loaded(Fixture &fx, unsigned variant)
{
	std::memset(&fx, 0, sizeof(fx));
	fx.ptab.__collate_load_error = 0;
	fx.rtab.__collate_load_error = 0;
	fx.ptab.info = &fx.pback.info;
	fx.rtab.info = &fx.rback.info;
	fx.ptab.char_pri_table = fx.pback.chars;
	fx.rtab.char_pri_table = fx.rback.chars;
	fx.ptab.chain_pri_table = fx.pback.chains;
	fx.rtab.chain_pri_table = fx.rback.chains;
	fx.ptab.large_pri_table = fx.pback.larges;
	fx.rtab.large_pri_table = fx.rback.larges;
	fx.ptab.subst_table[0] = fx.pback.subst0;
	fx.rtab.subst_table[0] = fx.rback.subst0;
	fx.ptab.subst_table[1] = fx.pback.subst1;
	fx.rtab.subst_table[1] = fx.rback.subst1;

	auto &pi = fx.pback.info;
	auto &ri = fx.rback.info;
	pi.directive_count = 3;
	ri.directive_count = 3;
	pi.directive[0] = DIRECTIVE_FORWARD;
	ri.directive[0] = DIRECTIVE_FORWARD;
	pi.directive[1] = (variant & 1) ? (DIRECTIVE_BACKWARD | DIRECTIVE_POSITION)
	    : (DIRECTIVE_FORWARD | DIRECTIVE_POSITION);
	ri.directive[1] = pi.directive[1];
	pi.directive[2] = DIRECTIVE_FORWARD;
	ri.directive[2] = DIRECTIVE_FORWARD;
	pi.pri_count[0] = 64;
	pi.pri_count[1] = 64;
	pi.pri_count[2] = 8;
	ri.pri_count[0] = pi.pri_count[0];
	ri.pri_count[1] = pi.pri_count[1];
	ri.pri_count[2] = pi.pri_count[2];
	pi.chain_count = 3;
	ri.chain_count = 3;
	pi.large_count = 2;
	ri.large_count = 2;
	pi.subst_count[0] = 1;
	pi.subst_count[1] = 1;
	ri.subst_count[0] = 1;
	ri.subst_count[1] = 1;
	pi.chain_max_len = 4;
	ri.chain_max_len = 4;
	pi.undef_pri[0] = 99;
	pi.undef_pri[1] = 88;
	pi.undef_pri[2] = 77;
	ri.undef_pri[0] = 99;
	ri.undef_pri[1] = 88;
	ri.undef_pri[2] = 77;

	for (int i = 0; i < 256; i++) {
		for (int p = 0; p < PBSD_COLL_WEIGHTS_MAX; p++) {
			int v = (i * 3 + p * 7 + (int)variant + 1) & 0x3fff;
			if (i == 5 && p == 1)
				v = -1;
			if (i == 7 && p == 0)
				v = 0;
			fx.pback.chars[i].pri[p] = v;
		}
	}
	fx.pback.chars['a'].pri[0] = (50 | COLLATE_SUBST_PRIORITY);
	fx.pback.subst0[0].key = fx.pback.chars['a'].pri[0];
	fx.pback.subst0[0].pri[0] = 10;
	fx.pback.subst0[0].pri[1] = 20;
	fx.pback.subst0[0].pri[2] = 0;

	wset(fx.pback.chains[0].str, L"ab");
	wset(fx.pback.chains[1].str, L"cd");
	wset(fx.pback.chains[2].str, L"xy");
	for (int c = 0; c < 3; c++) {
		for (int p = 0; p < PBSD_COLL_WEIGHTS_MAX; p++) {
			int v = 100 + c * 10 + p;
			fx.pback.chains[c].pri[p] = v;
		}
	}
	fx.pback.chains[1].pri[0] = -5;

	fx.pback.larges[0].val = 300;
	fx.pback.larges[1].val = 500;
	for (int p = 0; p < PBSD_COLL_WEIGHTS_MAX; p++) {
		fx.pback.larges[0].pri.pri[p] = 400 + p;
		fx.pback.larges[1].pri.pri[p] = 600 + p;
	}

	fx.pback.subst1[0].key = (60 | COLLATE_SUBST_PRIORITY);
	fx.pback.chars['b'].pri[1] = fx.pback.subst1[0].key;
	fx.pback.subst1[0].pri[0] = 30;
	fx.pback.subst1[0].pri[1] = 0;

	std::memcpy(&fx.rback.info, &fx.pback.info, sizeof(fx.rback.info));
	std::memcpy(fx.rback.chars, fx.pback.chars, sizeof(fx.rback.chars));
	std::memcpy(fx.rback.chains, fx.pback.chains, sizeof(fx.rback.chains));
	std::memcpy(fx.rback.larges, fx.pback.larges, sizeof(fx.rback.larges));
	std::memcpy(fx.rback.subst0, fx.pback.subst0, sizeof(fx.rback.subst0));
	std::memcpy(fx.rback.subst1, fx.pback.subst1, sizeof(fx.rback.subst1));
}

static void bind_locales(Fixture &fx)
{
	P::global_locale.components[XLC_COLLATE] =
	    (P::xlocale_component *)&fx.ptab;
	ref_global_locale.components[XLC_COLLATE] =
	    (xlocale_component *)&fx.rtab;
}

static void bind_posix()
{
	P::__xlocale_global_collate.__collate_load_error = 1;
	ref_xlocale_global_collate_ptr()->__collate_load_error = 1;
	P::global_locale.components[XLC_COLLATE] =
	    (P::xlocale_component *)&P::__xlocale_global_collate;
	ref_global_locale.components[XLC_COLLATE] =
	    (xlocale_component *)ref_xlocale_global_collate_ptr();
}

static bool write_collate_file(const char *locname, const RefBacking &b)
{
	char path[512];
	std::snprintf(path, sizeof(path), "%s/%s", tmp_root, locname);
	if (mkdir(path, 0755) != 0 && errno != EEXIST)
		return (false);
	std::snprintf(path, sizeof(path), "%s/%s/LC_COLLATE", tmp_root, locname);

	int chains = b.info.chain_count;
	int subsz = b.info.subst_count[0] + b.info.subst_count[1];
	size_t payload = sizeof(collate_info_t) +
	    sizeof(collate_char_t) * 256 +
	    sizeof(collate_subst_t) * subsz +
	    sizeof(collate_chain_t) * chains +
	    sizeof(collate_large_t) * b.info.large_count;
	size_t total = COLLATE_FMT_VERSION_LEN + XLOCALE_DEF_VERSION_LEN + payload;

	FILE *fp = std::fopen(path, "wb");
	if (!fp)
		return (false);
	std::fwrite("BSD 1.0\n", 1, COLLATE_FMT_VERSION_LEN, fp);
	char ver[XLOCALE_DEF_VERSION_LEN]{};
	std::fwrite(ver, 1, XLOCALE_DEF_VERSION_LEN, fp);
	std::fwrite(&b.info, 1, sizeof(b.info), fp);
	std::fwrite(b.chars, 1, sizeof(b.chars), fp);
	if (b.info.subst_count[0])
		std::fwrite(b.subst0, 1,
		    sizeof(collate_subst_t) * b.info.subst_count[0], fp);
	if (b.info.subst_count[1])
		std::fwrite(b.subst1, 1,
		    sizeof(collate_subst_t) * b.info.subst_count[1], fp);
	if (chains)
		std::fwrite(b.chains, 1, sizeof(collate_chain_t) * chains, fp);
	if (b.info.large_count)
		std::fwrite(b.larges, 1,
		    sizeof(collate_large_t) * b.info.large_count, fp);
	std::fclose(fp);
	(void)total;
	return (true);
}

static void setup_paths()
{
	std::strcpy(tmp_root, "/tmp/pbsdcltXXXXXX");
	if (!mkdtemp(tmp_root))
		std::strcpy(tmp_root, "/tmp");
	std::strncpy(P::_PathLocale, tmp_root, 255);
	P::_PathLocale[255] = 0;
	std::strncpy(_PathLocale, tmp_root, 255);
	_PathLocale[255] = 0;
}

static void test_load_hand()
{
	struct {
		const char *enc;
		bool stat;
	} cases[] = {
		{"C", true}, {"POSIX", true}, {"C.UTF-8", true},
		{"C.", true}, {"bogus-no-such-locale-xyz", false},
	};
	for (auto &c : cases) {
		void *pv = P::__collate_load(c.enc, (P::pbsd_locale_t)0);
		void *rv = ref___collate_load(c.enc, (pbsd_locale_t)0);
		bump(F_LOAD);
		if (c.stat) {
			if (!pv || !rv) {
				report(F_LOAD, "static null");
				continue;
			}
		} else if (pv != nullptr || rv != nullptr) {
			report(F_LOAD, "expected null");
			continue;
		}
	}
	RefBacking b{};
	b.info.directive_count = 1;
	b.info.directive[0] = DIRECTIVE_FORWARD;
	b.info.pri_count[0] = 8;
	b.info.chain_count = 0;
	b.info.large_count = 0;
	for (int i = 0; i < 256; i++)
		b.chars[i].pri[0] = i + 1;
	if (write_collate_file("tstLC1", b)) {
		void *pv = P::__collate_load("tstLC1", (P::pbsd_locale_t)0);
		void *rv = ref___collate_load("tstLC1", (pbsd_locale_t)0);
		bump(F_LOAD);
		if (!pv || !rv)
			report(F_LOAD, "file load null");
		else {
			auto *pt = (P::xlocale_collate *)pv;
			auto *rt = (xlocale_collate *)rv;
			if (pt->__collate_load_error != rt->__collate_load_error)
				report(F_LOAD, "load_error");
		}
		if (pv)
			P::xlocale_release(pv);
		if (rv) {
			struct xlocale_refcounted *obj = (struct xlocale_refcounted *)rv;
			long count = __sync_sub_and_fetch(&(obj->retain_count), 1);
			if (count < 0 && obj->destructor != NULL)
				obj->destructor(obj);
		}
	}
}

static void test_load_tables_hand()
{
	const char *encs[] = {"C", "POSIX", "C.foo", "nope-locale"};
	for (auto enc : encs) {
		int pv = P::__collate_load_tables(enc);
		int rv = ref___collate_load_tables(enc);
		bump(F_LOAD_TABLES);
		if (pv != rv)
			report(F_LOAD_TABLES, "cache enc");
	}
	RefBacking b{};
	b.info.directive_count = 1;
	b.info.directive[0] = DIRECTIVE_FORWARD;
	b.info.pri_count[0] = 8;
	b.info.chain_count = 0;
	b.info.large_count = 0;
	for (int i = 0; i < 256; i++)
		b.chars[i].pri[0] = i + 1;
	if (write_collate_file("tstLC2", b)) {
		P::__xlocale_global_collate.map = nullptr;
		ref_xlocale_global_collate_ptr()->map = nullptr;
		int pv = P::__collate_load_tables("tstLC2");
		int rv = ref___collate_load_tables("tstLC2");
		bump(F_LOAD_TABLES);
		if (pv != rv || pv != _LDP_LOADED)
			report(F_LOAD_TABLES, "loaded");
	}
}

static void test_lookup_one(Fixture &fx, const wchar_t *ws, int which, int f)
{
	const int *pstate = nullptr;
	const int *rstate = nullptr;
	int plen = -1, rlen = -1, ppri = -1, rpri = -1;
	const wchar_t *pt = ws;
	const wchar_t *rt = ws;
	for (int step = 0; step < 32 && (pt[0] || pstate); step++) {
		plen = rlen = -1;
		ppri = rpri = -1;
		P::_collate_lookup(&fx.ptab, pt, &plen, &ppri, which, &pstate);
		ref__collate_lookup(&fx.rtab, rt, &rlen, &rpri, which, &rstate);
		bump(f);
		if (plen != rlen || ppri != rpri)
			report(f, "lookup out");
		pt += plen > 0 ? plen : 0;
		rt += rlen > 0 ? rlen : 0;
		if (pstate == nullptr && rstate == nullptr && !pt[0])
			break;
	}
}

static void test_lookup_hand()
{
	Fixture fx;
	for (unsigned v = 0; v < 4; v++) {
		init_loaded(fx, v);
		const wchar_t *ss[] = {L"", L"a", L"ab", L"cd", L"zz",
		    L"\x500", L"a\xb", L"xyq", L"\xff"};
		for (auto ws : ss)
			test_lookup_one(fx, ws, (int)(v % 4), F_LOOKUP);
		for (int w = 0; w <= 4; w++)
			test_lookup_one(fx, L"abc", w, F_LOOKUP);
	}
}

static bool buf_same_w(const wchar_t *p, const wchar_t *r, size_t cap)
{
	for (size_t i = 0; i < cap; i++)
		if (p[i] != r[i])
			return (false);
	return (true);
}

static bool buf_same_c(const unsigned char *p, const unsigned char *r, size_t cap)
{
	for (size_t i = 0; i < cap; i++)
		if (p[i] != r[i])
			return (false);
	return (true);
}

static void test_wxfrm_hand()
{
	Fixture fx;
	init_loaded(fx, 1);
	const wchar_t *ss[] = {L"", L"a", L"ab", L"bac", L"\xff", L"a\xbcd"};
	for (auto ws : ss) {
	for (auto room : {size_t(0), size_t(1), size_t(8), size_t(256)}) {
			wchar_t pbuf[512], rbuf[512];
			for (size_t i = 0; i < 512; i++) {
				pbuf[i] = WGUARD;
				rbuf[i] = WGUARD;
			}
			errno = 0;
			size_t pv = P::_collate_wxfrm(&fx.ptab, ws, pbuf, room);
			int pe = errno;
			errno = 0;
			size_t rv = ref__collate_wxfrm(&fx.rtab, ws, rbuf, room);
			int re = errno;
			bump(F_WXFRM);
			if (pv != rv || pe != re || !buf_same_w(pbuf, rbuf, 512))
				report(F_WXFRM, "wxfrm");
		}
	}
}

static void test_sxfrm_hand()
{
	Fixture fx;
	init_loaded(fx, 2);
	const wchar_t *ss[] = {L"", L"z", L"cd", L"bac", L"\x300"};
	for (auto ws : ss) {
	for (auto room : {size_t(0), size_t(2), size_t(16), size_t(512)}) {
			unsigned char pbuf[1024], rbuf[1024];
			std::memset(pbuf, GUARD, sizeof(pbuf));
			std::memset(rbuf, GUARD, sizeof(rbuf));
			errno = 0;
			size_t pv = P::_collate_sxfrm(&fx.ptab, ws,
			    (char *)pbuf, room);
			int pe = errno;
			errno = 0;
			size_t rv = ref__collate_sxfrm(&fx.rtab, ws,
			    (char *)rbuf, room);
			int re = errno;
			bump(F_SXFRM);
			if (pv != rv || pe != re || !buf_same_c(pbuf, rbuf, 1024))
				report(F_SXFRM, "sxfrm");
		}
	}
}

static void test_equiv_value_hand()
{
	Fixture fx;
	init_loaded(fx, 0);
	bind_locales(fx);
	const wchar_t one[] = {L'a', L'b', L'\x500', 0};
	for (size_t len = 0; len <= 30; len++) {
		int pv = P::__collate_equiv_value((P::pbsd_locale_t)0, one, len);
		int rv = ref___collate_equiv_value((pbsd_locale_t)0, one, len);
		bump(F_EQUIV_VALUE);
		if (pv != rv)
			report(F_EQUIV_VALUE, "value");
	}
	bind_posix();
	for (wchar_t ch = 0; ch < 512; ch += 17) {
		wchar_t w[2] = {ch, 0};
		int pv = P::__collate_equiv_value((P::pbsd_locale_t)-1, w, 1);
		int rv = ref___collate_equiv_value((pbsd_locale_t)-1, w, 1);
		bump(F_EQUIV_VALUE);
		if (pv != rv)
			report(F_EQUIV_VALUE, "posix");
	}
}

static void test_coll_sym_hand()
{
	Fixture fx;
	init_loaded(fx, 0);
	bind_locales(fx);
	const char *ss[] = {"", "a", "ab", "cd", "\x80", "\xff", "xy"};
	for (auto s : ss) {
		for (size_t dlen = 0; dlen < 8; dlen++) {
			wchar_t pd[16], rd[16];
			for (int i = 0; i < 16; i++) {
				pd[i] = WGUARD;
				rd[i] = WGUARD;
			}
			mbstate_t ps{}, rs{};
			size_t pv = P::__collate_collating_symbol(pd, dlen, s,
			    std::strlen(s), &ps);
			size_t rv = ref___collate_collating_symbol(rd, dlen, s,
			    std::strlen(s), &rs);
			bump(F_COLL_SYM);
			if (pv != rv || !buf_same_w(pd, rd, 16))
				report(F_COLL_SYM, "sym");
		}
	}
	bind_posix();
}

static void test_equiv_class_hand()
{
	Fixture fx;
	init_loaded(fx, 0);
	bind_locales(fx);
	const char *ss[] = {"a", "ab", "cd", "\xc3\xa9", ""};
	for (auto s : ss) {
		mbstate_t ps{}, rs{};
		int pv = P::__collate_equiv_class(s, std::strlen(s), &ps);
		int rv = ref___collate_equiv_class(s, std::strlen(s), &rs);
		bump(F_EQUIV_CLASS);
		if (pv != rv)
			report(F_EQUIV_CLASS, "class");
	}
}

static void test_equiv_match_hand()
{
	Fixture fx;
	init_loaded(fx, 0);
	bind_locales(fx);
	const char *ss[] = {"a", "ab", "cd", "zz", "abx"};
	for (int eq = -2; eq < 120; eq += 7) {
		for (auto s : ss) {
			for (wchar_t st = 0; st < 3; st++) {
				for (size_t dlen = 0; dlen < 6; dlen++) {
					wchar_t pd[16], rd[16];
					mbstate_t ps{}, rs{};
					size_t prl = 99, rrl = 99;
					size_t pv = P::__collate_equiv_match(eq, pd,
					    dlen, st, s, std::strlen(s), &ps, &prl);
					size_t rv = ref___collate_equiv_match(eq, rd,
					    dlen, st, s, std::strlen(s), &rs, &rrl);
					bump(F_EQUIV_MATCH);
					if (pv != rv || prl != rrl ||
					    !buf_same_w(pd, rd, 16))
						report(F_EQUIV_MATCH, "match");
				}
			}
		}
	}
}

static void rand_wcs(wchar_t *buf, int n)
{
	int len = (int)(ru32((uint32_t)n + 1));
	for (int i = 0; i < len; i++)
		buf[i] = (wchar_t)(ru32(0x600) + (ru32(3) == 0 ? 0x80 : 0));
	buf[len] = 0;
}

static void rand_mbs(char *buf, int n)
{
	int len = (int)(ru32((uint32_t)n + 1));
	for (int i = 0; i < len; i++)
		buf[i] = (char)(ru32(256));
	buf[len] = 0;
}

static void sweep_load()
{
	const char *encs[] = {"C", "POSIX", "C.UTF-8"};
	for (long long i = 0; i < SWEEP; i++) {
		const char *enc = encs[ru32(3)];
		void *pv = P::__collate_load(enc, (P::pbsd_locale_t)(ru32(2) ? -1 : 0));
		void *rv = ref___collate_load(enc, (pbsd_locale_t)(ru32(2) ? -1 : 0));
		bump(F_LOAD);
		if ((pv == nullptr) != (rv == nullptr))
			report(F_LOAD, "sweep ptr");
	}
}

static void sweep_load_tables()
{
	const char *encs[] = {"C", "POSIX", "C.foo"};
	for (long long i = 0; i < SWEEP; i++) {
		int pv = P::__collate_load_tables(encs[ru32(3)]);
		int rv = ref___collate_load_tables(encs[ru32(3)]);
		bump(F_LOAD_TABLES);
		if (pv != rv)
			report(F_LOAD_TABLES, "sweep");
	}
}

static void sweep_lookup()
{
	Fixture fx;
	wchar_t ws[32];
	for (long long i = 0; i < SWEEP; i++) {
		init_loaded(fx, (unsigned)ru32(8));
		rand_wcs(ws, 12);
		test_lookup_one(fx, ws, (int)ru32(5), F_LOOKUP);
	}
}

static void sweep_wxfrm()
{
	Fixture fx;
	wchar_t ws[32];
	for (long long i = 0; i < SWEEP; i++) {
		init_loaded(fx, (unsigned)ru32(4));
		rand_wcs(ws, 16);
		size_t room = ru32(300);
		wchar_t pbuf[512], rbuf[512];
		for (size_t j = 0; j < 512; j++) {
			pbuf[j] = WGUARD;
			rbuf[j] = WGUARD;
		}
		errno = 0;
		size_t pv = P::_collate_wxfrm(&fx.ptab, ws, pbuf, room);
		int pe = errno;
		errno = 0;
		size_t rv = ref__collate_wxfrm(&fx.rtab, ws, rbuf, room);
		int re = errno;
		bump(F_WXFRM);
		if (pv != rv || pe != re || !buf_same_w(pbuf, rbuf, 512))
			report(F_WXFRM, "sweep");
	}
}

static void sweep_sxfrm()
{
	Fixture fx;
	wchar_t ws[32];
	for (long long i = 0; i < SWEEP; i++) {
		init_loaded(fx, (unsigned)ru32(4));
		rand_wcs(ws, 16);
		size_t room = ru32(600);
		unsigned char pbuf[1024], rbuf[1024];
		std::memset(pbuf, GUARD, sizeof(pbuf));
		std::memset(rbuf, GUARD, sizeof(rbuf));
		errno = 0;
		size_t pv = P::_collate_sxfrm(&fx.ptab, ws, (char *)pbuf, room);
		int pe = errno;
		errno = 0;
		size_t rv = ref__collate_sxfrm(&fx.rtab, ws, (char *)rbuf, room);
		int re = errno;
		bump(F_SXFRM);
		if (pv != rv || pe != re || !buf_same_c(pbuf, rbuf, 1024))
			report(F_SXFRM, "sweep");
	}
}

static void sweep_equiv_value()
{
	Fixture fx;
	wchar_t ws[32];
	for (long long i = 0; i < SWEEP; i++) {
		if (ru32(4) == 0)
			bind_posix();
		else {
			init_loaded(fx, (unsigned)ru32(4));
			bind_locales(fx);
		}
		rand_wcs(ws, 10);
		size_t len = ru32(30);
		int pv = P::__collate_equiv_value((P::pbsd_locale_t)(intptr_t)(ru32(3) - 1),
		    ws, len);
		int rv = ref___collate_equiv_value((pbsd_locale_t)(intptr_t)(ru32(3) - 1),
		    ws, len);
		bump(F_EQUIV_VALUE);
		if (pv != rv)
			report(F_EQUIV_VALUE, "sweep");
	}
}

static void sweep_coll_sym()
{
	Fixture fx;
	char mb[32];
	for (long long i = 0; i < SWEEP; i++) {
		if (ru32(4) == 0)
			bind_posix();
		else {
			init_loaded(fx, (unsigned)ru32(4));
			bind_locales(fx);
		}
		rand_mbs(mb, 12);
		size_t sl = ru32(20);
		size_t dlen = ru32(10);
		wchar_t pd[16], rd[16];
		mbstate_t ps{}, rs{};
		size_t pv = P::__collate_collating_symbol(pd, dlen, mb, sl, &ps);
		size_t rv = ref___collate_collating_symbol(rd, dlen, mb, sl, &rs);
		bump(F_COLL_SYM);
		if (pv != rv || !buf_same_w(pd, rd, 16))
			report(F_COLL_SYM, "sweep");
	}
}

static void sweep_equiv_class()
{
	Fixture fx;
	char mb[32];
	for (long long i = 0; i < SWEEP; i++) {
		if (ru32(4) == 0)
			bind_posix();
		else {
			init_loaded(fx, (unsigned)ru32(4));
			bind_locales(fx);
		}
		rand_mbs(mb, 12);
		mbstate_t ps{}, rs{};
		int pv = P::__collate_equiv_class(mb, ru32(20), &ps);
		int rv = ref___collate_equiv_class(mb, ru32(20), &rs);
		bump(F_EQUIV_CLASS);
		if (pv != rv)
			report(F_EQUIV_CLASS, "sweep");
	}
}

static void sweep_equiv_match()
{
	Fixture fx;
	char mb[32];
	for (long long i = 0; i < SWEEP; i++) {
		init_loaded(fx, (unsigned)ru32(4));
		bind_locales(fx);
		rand_mbs(mb, 12);
		wchar_t pd[16], rd[16];
		mbstate_t ps{}, rs{};
		size_t prl = 0, rrl = 0;
		int eq = (int)ru32(200) - 5;
		size_t pv = P::__collate_equiv_match(eq, pd, ru32(8),
		    (wchar_t)ru32(256), mb, ru32(20), &ps, &prl);
		size_t rv = ref___collate_equiv_match(eq, rd, ru32(8),
		    (wchar_t)ru32(256), mb, ru32(20), &rs, &rrl);
		bump(F_EQUIV_MATCH);
		if (pv != rv || prl != rrl || !buf_same_w(pd, rd, 16))
			report(F_EQUIV_MATCH, "sweep");
	}
}

int main()
{
	setup_paths();
	test_load_hand();
	test_load_tables_hand();
	test_lookup_hand();
	test_wxfrm_hand();
	test_sxfrm_hand();
	test_equiv_value_hand();
	test_coll_sym_hand();
	test_equiv_class_hand();
	test_equiv_match_hand();
	sweep_load();
	sweep_load_tables();
	sweep_lookup();
	sweep_wxfrm();
	sweep_sxfrm();
	sweep_equiv_value();
	sweep_coll_sym();
	sweep_equiv_class();
	sweep_equiv_match();

	std::printf("\n%-32s %12s %12s\n", "function", "cases", "failures");
	long long tc = 0, tf = 0;
	for (int i = 0; i < F_COUNT; i++) {
		std::printf("%-32s %12lld %12lld\n", fnames[i], ncase[i], nfail[i]);
		tc += ncase[i];
		tf += nfail[i];
	}
	std::printf("%-32s %12lld %12lld\n", "TOTAL", tc, tf);
	return (tf ? 1 : 0);
}

