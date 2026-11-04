/*
 * Differential test for batch b0216 (strtoll*, tsearch, tdelete, realpath).
 */

#define _DEFAULT_SOURCE

#include <cerrno>
#include <climits>
#include <cinttypes>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

import pbsd.lib.libc.stdlib.b0216;

namespace P = pbsd::lib_libc_stdlib::b0216;

extern "C" {
struct xlocale;
long long ref_strtoll_l(const char *, char **, int, struct xlocale *);
long long ref_strtoll(const char *, char **, int);
P::posix_tnode *ref_tsearch(const void *, P::posix_tnode **,
    int (*)(const void *, const void *));
void *ref_tdelete(const void *, P::posix_tnode **,
    int (*)(const void *, const void *));
char *ref_realpath(const char *, char *);
}

namespace {

enum Fn {
	F_STRTOLL_L,
	F_STRTOLL,
	F_TSEARCH,
	F_TDELETE,
	F_REALPATH,
	F_COUNT
};

static const char *const fn_name[F_COUNT] = {
	"strtoll_l",
	"strtoll",
	"tsearch",
	"tdelete",
	"realpath",
};

static unsigned long long n_cases[F_COUNT];
static unsigned long long n_fails[F_COUNT];
static unsigned long long reported[F_COUNT];

static const unsigned char GUARD = 0x7f;
static const size_t STRBUF = 256;
static const unsigned SWEEP_ITERS = 200000u;

static char g_tmpdir[PATH_MAX];
static char g_tmpbase[PATH_MAX];
static char g_subdir[PATH_MAX];
static char g_file[PATH_MAX];
static char g_symlink[PATH_MAX];

static void
record_fail(int fn, const char *fmt, ...)
{
	n_fails[fn]++;
	if (reported[fn] < 10) {
		reported[fn]++;
		va_list ap;
		va_start(ap, fmt);
		std::fprintf(stderr, "FAIL %s: ", fn_name[fn]);
		std::vfprintf(stderr, fmt, ap);
		va_end(ap);
		std::fputc('\n', stderr);
	} else if (reported[fn] == 10) {
		reported[fn]++;
		std::fprintf(stderr, "FAIL %s: (further failures suppressed)\n",
		    fn_name[fn]);
	}
}

static void
record_case(int fn, bool ok, const char *fmt, ...)
{
	n_cases[fn]++;
	if (ok)
		return;
	n_fails[fn]++;
	if (reported[fn] < 10) {
		reported[fn]++;
		va_list ap;
		va_start(ap, fmt);
		std::fprintf(stderr, "FAIL %s: ", fn_name[fn]);
		std::vfprintf(stderr, fmt, ap);
		va_end(ap);
		std::fputc('\n', stderr);
	} else if (reported[fn] == 10) {
		reported[fn]++;
		std::fprintf(stderr, "FAIL %s: (further failures suppressed)\n",
		    fn_name[fn]);
	}
}

static uint64_t rng_state = 0xc0ffeebaddecaf01ULL;

static uint64_t
nextr(void)
{
	rng_state ^= rng_state << 13;
	rng_state ^= rng_state >> 7;
	rng_state ^= rng_state << 17;
	return rng_state;
}

static unsigned
rnd_below(unsigned n)
{
	return (unsigned)(nextr() % (uint64_t)n);
}

static int
icmp(const void *a, const void *b)
{
	int x = *(const int *)a;
	int y = *(const int *)b;
	return (x > y) - (x < y);
}

static unsigned
tree_height(const P::posix_tnode *n)
{
	unsigned hl, hr;

	if (n == nullptr)
		return 0;
	hl = tree_height(n->llink);
	hr = tree_height(n->rlink);
	return (hl > hr ? hl : hr) + 1;
}

static void
tree_collect(const P::posix_tnode *n, int *out, unsigned *count)
{
	if (n == nullptr)
		return;
	tree_collect(n->llink, out, count);
	out[(*count)++] = *(const int *)n->key;
	tree_collect(n->rlink, out, count);
}

static bool
trees_equal(const P::posix_tnode *a, const P::posix_tnode *b)
{
	int keys_a[4096], keys_b[4096];
	unsigned na = 0, nb = 0;

	tree_collect(a, keys_a, &na);
	tree_collect(b, keys_b, &nb);
	if (na != nb)
		return false;
	for (unsigned i = 0; i < na; i++)
		if (keys_a[i] != keys_b[i])
			return false;
	return true;
}

static void
free_tree(P::posix_tnode *n)
{
	if (n == nullptr)
		return;
	free_tree(n->llink);
	free_tree(n->rlink);
	free(n);
}

/* ------------------------------------------------------------------ */
/* strtoll							      */
/* ------------------------------------------------------------------ */

static const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";

static void
fill_str(char *buf, size_t bufsz, size_t *outlen)
{
	size_t n = 1 + rnd_below((unsigned)(bufsz - 2));
	size_t i;

	for (i = 0; i < n; i++) {
		unsigned pick = rnd_below(256u);
		if ((i % 11u) == 0u)
			buf[i] = (char)(0x80 + rnd_below(0x80u));
		else if (pick < 10u)
			buf[i] = (char)('0' + pick);
		else if (pick < 36u)
			buf[i] = (char)('a' + pick - 10u);
		else if (pick < 46u)
			buf[i] = (char)('A' + pick - 36u);
		else
			buf[i] = (char)pick;
	}
	buf[n] = '\0';
	*outlen = n;
}

static void
check_strto(unsigned fn_l, unsigned fn, bool use_l, const char *in, int base,
    bool use_endptr, P::locale_t loc)
{
	char a[STRBUF], b[STRBUF];
	char *ea, *eb;
	int pre;

	std::memset(a, GUARD, sizeof(a));
	std::memset(b, GUARD, sizeof(b));
	std::strncpy(a, in, sizeof(a) - 1);
	std::strncpy(b, in, sizeof(b) - 1);
	a[sizeof(a) - 1] = '\0';
	b[sizeof(b) - 1] = '\0';

	ea = a + 17;
	eb = b + 17;
	pre = (int)(nextr() & 0xffu);
	errno = pre;

	long long rllp = 0, rllr = 0;
	int e1, e2;

	if (use_l) {
		rllp = P::strtoll_l(a, use_endptr ? &ea : nullptr, base, loc);
		e1 = errno;
		errno = pre;
		rllr = ref_strtoll_l(b, use_endptr ? &eb : nullptr, base,
		    (struct xlocale *)loc);
		e2 = errno;
		bool ok = rllp == rllr && e1 == e2 &&
		    std::memcmp(a, b, sizeof(a)) == 0 &&
		    (!use_endptr || (ea - a) == (eb - b));
		record_case(fn_l, ok, "in=\"%s\" base=%d", in, base);
	} else {
		rllp = P::strtoll(a, use_endptr ? &ea : nullptr, base);
		e1 = errno;
		errno = pre;
		rllr = ref_strtoll(b, use_endptr ? &eb : nullptr, base);
		e2 = errno;
		bool ok = rllp == rllr && e1 == e2 &&
		    std::memcmp(a, b, sizeof(a)) == 0 &&
		    (!use_endptr || (ea - a) == (eb - b));
		record_case(fn, ok, "in=\"%s\" base=%d", in, base);
	}
}

static void
check_both_strto(const char *in, int base, bool use_endptr, P::locale_t loc)
{
	check_strto(F_STRTOLL_L, F_STRTOLL, true, in, base, use_endptr, loc);
	check_strto(F_STRTOLL_L, F_STRTOLL, false, in, base, use_endptr, loc);
}

static void
test_strto_edges(void)
{
	static const char *const fixed[] = {
		"",
		" ",
		"\t\n\r",
		"0",
		"00",
		"01",
		"07",
		"08",
		"09",
		"+0",
		"-0",
		"-1",
		"-42",
		"42",
		"0x",
		"0X",
		"0x0",
		"0xG",
		"0xg",
		"0x10",
		"0Xff",
		"0b",
		"0B",
		"0b0",
		"0b10",
		"0b2",
		"+0x10",
		"-0x10",
		"z",
		"123abc",
		"7fffffffffffffff",
		"9223372036854775807",
		"9223372036854775808",
		"999999999999999999999",
		"\x80",
		"\xff",
		" \x80""42",
		" 0x10 ",
		"0x",
		"0b",
		"0",
	};
	P::locale_t loc = P::__get_locale();
	P::locale_t nul = nullptr;
	unsigned i;
	int bases[] = { 0, 2, 8, 10, 16, 1, 37, -1, 36 };

	for (i = 0; i < sizeof(fixed) / sizeof(fixed[0]); i++) {
		for (int base : bases) {
			check_both_strto(fixed[i], base, true, loc);
			check_both_strto(fixed[i], base, false, loc);
			check_both_strto(fixed[i], base, true, nul);
		}
	}
	{
		char buf[64];
		std::snprintf(buf, sizeof(buf), "%lld", LLONG_MAX);
		check_both_strto(buf, 10, true, loc);
		std::snprintf(buf, sizeof(buf), "%lld0", LLONG_MAX / 10);
		check_both_strto(buf, 10, true, loc);
		std::snprintf(buf, sizeof(buf), "%lld", LLONG_MIN);
		check_both_strto(buf, 10, true, loc);
	}
}

static void
test_strto_random(unsigned iters)
{
	char tmp[STRBUF];
	P::locale_t loc = P::__get_locale();
	unsigned i;

	for (i = 0; i < iters; i++) {
		size_t n;
		int base;
		bool use_endptr = (i % 3u) != 0u;

		fill_str(tmp, sizeof(tmp), &n);
		switch (i % 9u) {
		case 0: base = 0; break;
		case 1: base = 2; break;
		case 2: base = 8; break;
		case 3: base = 10; break;
		case 4: base = 16; break;
		case 5: base = 36; break;
		case 6: base = 1; break;
		case 7: base = 37; break;
		default: base = (int)(nextr() % 40u) - 2; break;
		}
		if ((i % 17u) == 0u) {
			unsigned b = 2 + rnd_below(35u);
			unsigned len = 4 + rnd_below(40u);
			tmp[0] = (i % 2u) ? '+' : '-';
			size_t pos = 1;
			if (b == 16 && (i % 4u) == 0u) {
				tmp[pos++] = '0';
				tmp[pos++] = (i % 8u) ? 'x' : 'X';
			} else if (b == 2 && (i % 5u) == 0u) {
				tmp[pos++] = '0';
				tmp[pos++] = (i % 8u) ? 'b' : 'B';
			}
			for (; pos < len && pos < sizeof(tmp) - 1; pos++)
				tmp[pos] = digits[rnd_below(b)];
			tmp[pos] = '\0';
			base = (int)((i % 3u) == 0u ? 0 : b);
		}
		check_both_strto(tmp, base, use_endptr, loc);
	}
}

/* ------------------------------------------------------------------ */
/* tsearch / tdelete						      */
/* ------------------------------------------------------------------ */

static void
case_tsearch_null_rootp(void)
{
	P::posix_tnode *rp = P::tsearch(&g_tmpbase[0], nullptr, icmp);
	P::posix_tnode *rr = ref_tsearch(&g_tmpbase[0], nullptr, icmp);
	record_case(F_TSEARCH, rp == nullptr && rr == nullptr, "rootp=NULL");
}

static void
case_tree_op(int op, int key, P::posix_tnode **proot, P::posix_tnode **rroot,
    int *pkeys, int *rkeys, unsigned nkeys, bool *ppresent, bool *rpresent)
{
	unsigned pi, ri;
	bool pnull, rnull;

	(void)pi;
	(void)ri;

	if (op == 0) {
		P::posix_tnode *tp = P::tsearch(&pkeys[key], proot, icmp);
		P::posix_tnode *tr = ref_tsearch(&rkeys[key], rroot, icmp);
		pnull = tp == nullptr;
		rnull = tr == nullptr;
		bool ok = pnull == rnull;
		if (!pnull && !rnull) {
			ok = ok && *(const int *)tp->key == pkeys[key] &&
			    *(const int *)tr->key == rkeys[key];
			ppresent[key] = true;
			rpresent[key] = true;
		}
		record_case(F_TSEARCH, ok,
		    "insert key=%d port_null=%d ref_null=%d", key, pnull, rnull);
	} else {
		void *dp = P::tdelete(&pkeys[key], proot, icmp);
		void *dr = ref_tdelete(&rkeys[key], rroot, icmp);
		pnull = dp == nullptr;
		rnull = dr == nullptr;
		bool ok = pnull == rnull;
		if (!pnull && !rnull) {
			ok = ok && trees_equal(*proot, *rroot);
		}
		if (!pnull)
			ppresent[key] = false;
		if (!rnull)
			rpresent[key] = false;
		record_case(F_TDELETE, ok,
		    "delete key=%d port_null=%d ref_null=%d", key, pnull, rnull);
	}

	if (*proot != nullptr && *rroot != nullptr)
		record_case(F_TSEARCH, trees_equal(*proot, *rroot),
		    "tree state key=%d op=%d", key, op);

	(void)nkeys;
}

static void
test_tree_edges(void)
{
#define NKEYS 64
	int pkeys[NKEYS], rkeys[NKEYS];
	bool ppresent[NKEYS], rpresent[NKEYS];
	P::posix_tnode *proot = nullptr, *rroot = nullptr;
	unsigned i;

	for (i = 0; i < NKEYS; i++) {
		pkeys[i] = (int)i;
		rkeys[i] = (int)i;
		ppresent[i] = false;
		rpresent[i] = false;
	}

	case_tsearch_null_rootp();

	for (i = 0; i < NKEYS; i++)
		case_tree_op(0, (int)i, &proot, &rroot, pkeys, rkeys, NKEYS,
		    ppresent, rpresent);
	for (i = 0; i < NKEYS; i++)
		case_tree_op(1, (int)i, &proot, &rroot, pkeys, rkeys, NKEYS,
		    ppresent, rpresent);

	case_tree_op(1, 99, &proot, &rroot, pkeys, rkeys, NKEYS, ppresent,
	    rpresent);

	free_tree(proot);
	free_tree(rroot);
#undef NKEYS
}

static void
test_tree_random(unsigned iters)
{
#define NKEYS 256
	int pkeys[NKEYS], rkeys[NKEYS];
	bool ppresent[NKEYS], rpresent[NKEYS];
	P::posix_tnode *proot = nullptr, *rroot = nullptr;
	unsigned i;

	for (i = 0; i < NKEYS; i++) {
		pkeys[i] = (int)(nextr() & 0x7fffffffu);
		rkeys[i] = pkeys[i];
		ppresent[i] = false;
		rpresent[i] = false;
	}

	for (i = 0; i < iters; i++) {
		unsigned key = rnd_below(NKEYS);
		int op = (int)(nextr() % 3u);
		if (op == 2)
			op = ppresent[key] ? 1 : 0;
		case_tree_op(op, (int)key, &proot, &rroot, pkeys, rkeys, NKEYS,
		    ppresent, rpresent);
	}

	free_tree(proot);
	free_tree(rroot);
#undef NKEYS
}

/* ------------------------------------------------------------------ */
/* realpath							      */
/* ------------------------------------------------------------------ */

static void
case_realpath(const char *path, char *resolved, bool use_buf)
{
	char pa[PATH_MAX], ra[PATH_MAX];
	char *pp, *pr;
	int e1, e2;
	bool ok;

	if (use_buf) {
		std::memset(pa, GUARD, sizeof(pa));
		std::memset(ra, GUARD, sizeof(ra));
		errno = 0;
		pp = P::realpath(path, pa);
		e1 = errno;
		errno = 0;
		pr = ref_realpath(path, ra);
		e2 = errno;
		ok = (pp == nullptr) == (pr == nullptr) && e1 == e2;
		if (pp != nullptr && pr != nullptr)
			ok = ok && std::strcmp(pa, ra) == 0;
		else if (pp == nullptr && pr == nullptr && path != nullptr &&
		    path[0] != '\0')
			ok = ok && std::strcmp(pa, ra) == 0;
		record_case(F_REALPATH, ok,
		    "path=\"%s\" buf port=%s/%d ref=%s/%d",
		    path ? path : "(null)",
		    pp ? pa : "(null)", e1, pr ? ra : "(null)", e2);
	} else {
		errno = 0;
		pp = P::realpath(path, resolved);
		e1 = errno;
		errno = 0;
		pr = ref_realpath(path, resolved);
		e2 = errno;
		ok = (pp == nullptr) == (pr == nullptr) && e1 == e2;
		if (pp != nullptr && pr != nullptr)
			ok = ok && std::strcmp(pp, pr) == 0;
		record_case(F_REALPATH, ok,
		    "path=\"%s\" malloc port=%s/%d ref=%s/%d",
		    path ? path : "(null)",
		    pp ? pp : "(null)", e1, pr ? pr : "(null)", e2);
		free(pp);
		free(pr);
	}
}

static bool
setup_fs(void)
{
	char tmpl[PATH_MAX];

	std::snprintf(tmpl, sizeof(tmpl), "/tmp/b0216XXXXXX");
	if (mkdtemp(tmpl) == nullptr)
		return false;
	if (realpath(tmpl, g_tmpdir) == nullptr)
		std::strcpy(g_tmpdir, tmpl);
	std::snprintf(g_tmpbase, sizeof(g_tmpbase), "%s/base", g_tmpdir);
	std::snprintf(g_subdir, sizeof(g_subdir), "%s/sub", g_tmpdir);
	std::snprintf(g_file, sizeof(g_file), "%s/file", g_tmpdir);
	std::snprintf(g_symlink, sizeof(g_symlink), "%s/link", g_tmpdir);

	if (mkdir(g_tmpbase, 0755) != 0)
		return false;
	if (mkdir(g_subdir, 0755) != 0)
		return false;
	{
		int fd = open(g_file, O_CREAT | O_WRONLY, 0644);
		if (fd < 0)
			return false;
		close(fd);
	}
	if (symlink(g_file, g_symlink) != 0)
		return false;
	return true;
}

static void
test_realpath_edges(void)
{
	char rel[PATH_MAX];
	char dotpath[PATH_MAX];
	char dotdotpath[PATH_MAX];
	char linkpath[PATH_MAX];
	char bad[PATH_MAX];

	if (!setup_fs())
		return;

	std::snprintf(rel, sizeof(rel), "file");
	std::snprintf(dotpath, sizeof(dotpath), "%s/.", g_tmpdir);
	std::snprintf(dotdotpath, sizeof(dotdotpath), "%s/sub/../file", g_tmpdir);
	std::snprintf(linkpath, sizeof(linkpath), "%s/link", g_tmpdir);
	std::snprintf(bad, sizeof(bad), "%s/nonexistent", g_tmpdir);

	case_realpath(nullptr, nullptr, true);
	case_realpath("", nullptr, true);
	case_realpath("/", nullptr, true);
	case_realpath(g_tmpdir, nullptr, true);
	case_realpath(g_file, nullptr, true);
	case_realpath(rel, nullptr, true);
	case_realpath(dotpath, nullptr, true);
	case_realpath(dotdotpath, nullptr, true);
	case_realpath(linkpath, nullptr, true);
	case_realpath(bad, nullptr, true);
	case_realpath(g_tmpdir, nullptr, false);
	case_realpath(g_file, nullptr, false);

	{
		char longpath[PATH_MAX * 2];
		std::memset(longpath, 'a', PATH_MAX + 100);
		longpath[PATH_MAX + 100] = '\0';
		case_realpath(longpath, nullptr, true);
	}
}

static void
test_realpath_random(unsigned iters)
{
	char path[PATH_MAX];
	unsigned i;

	if (g_tmpdir[0] == '\0' && !setup_fs())
		return;

	for (i = 0; i < iters; i++) {
		unsigned pick = rnd_below(12u);
		bool use_buf = (i % 4u) != 0u;

		switch (pick) {
		case 0:
			std::snprintf(path, sizeof(path), "%s", g_tmpdir);
			break;
		case 1:
			std::snprintf(path, sizeof(path), "%s/file", g_tmpdir);
			break;
		case 2:
			std::snprintf(path, sizeof(path), "%s/sub", g_tmpdir);
			break;
		case 3:
			std::snprintf(path, sizeof(path), "%s/link", g_tmpdir);
			break;
		case 4:
			std::snprintf(path, sizeof(path), "%s/.", g_tmpdir);
			break;
		case 5:
			std::snprintf(path, sizeof(path), "%s/sub/../file",
			    g_tmpdir);
			break;
		case 6:
			std::snprintf(path, sizeof(path), "/");
			break;
		case 7:
			std::snprintf(path, sizeof(path), "file");
			break;
		case 8:
			std::snprintf(path, sizeof(path), "%s/missing%u",
			    g_tmpdir, rnd_below(10000u));
			break;
		case 9:
			path[0] = '\0';
			break;
		case 10:
			std::snprintf(path, sizeof(path), "%s/%u",
			    g_tmpdir, rnd_below(1000u));
			break;
		default:
			std::snprintf(path, sizeof(path), "%s/base/%u",
			    g_tmpdir, rnd_below(1000u));
			break;
		}
		case_realpath(path, nullptr, use_buf);
	}
}

} /* namespace */

int
main(void)
{
	unsigned fn;
	unsigned long long total_cases = 0, total_fails = 0;

	test_strto_edges();
	test_tree_edges();
	test_realpath_edges();

	test_strto_random(SWEEP_ITERS);
	test_tree_random(SWEEP_ITERS);
	test_realpath_random(SWEEP_ITERS);

	std::printf("\nbatch b0216 differential results\n");
	std::printf("%-14s %12s %10s %s\n", "function", "cases", "failures",
	    "status");
	for (fn = 0; fn < F_COUNT; fn++) {
		std::printf("%-14s %12llu %10llu %s\n", fn_name[fn],
		    n_cases[fn], n_fails[fn],
		    n_fails[fn] == 0u ? "ok" : "FAIL");
		total_cases += n_cases[fn];
		total_fails += n_fails[fn];
	}
	std::printf("%-14s %12llu %10llu %s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0u ? "ok" : "FAIL");

	return total_fails == 0u ? 0 : 1;
}
