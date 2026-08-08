/*
 * Differential harness for batch b0153s3.
 */

import pbsd.lib.libc.locale.b0153s3;

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace P = pbsd::lib_libc_locale::b0153s3;

extern "C" {
typedef struct {
	int			open_fail;
	int			open_errno;
	int			fstat_fail;
	int			fstat_errno;
	off_t			file_size;
	const char		*file_content;
	int			read_fail;
	int			read_errno;
	int			malloc_fail;
	int			close_count;
} pbsd_ldpart_hook_t;

void			pbsd_reset_hooks(void);
pbsd_ldpart_hook_t	*pbsd_get_ldpart_hook(void);
int			ref___part_load_locale(const char *, int *, char **,
			    const char *, int, int, const char **);
int			ref_split_lines(char *, const char *);
}

enum {
	F_SPLIT_LINES,
	F_PART_LOAD,
	F_COUNT
};

enum {
	LDP_LOADED = 0,
	LDP_ERROR = -1,
	LDP_CACHE = 1,
};

static const char *const fname[F_COUNT] = {
	"split_lines",
	"__part_load_locale",
};

static long long ncase[F_COUNT];
static long long nfail[F_COUNT];
static int nprinted[F_COUNT];

static constexpr unsigned char GUARD = 0x7f;
static constexpr long long SWEEP = 200000;
static constexpr size_t BUF_OFF = 16;

static pbsd_ldpart_hook_t &
ldhook()
{
	return (*pbsd_get_ldpart_hook());
}

static uint64_t rng = 0xB0153A301ULL;

static uint64_t
rnd(void)
{
	rng ^= rng << 13;
	rng ^= rng >> 7;
	rng ^= rng << 17;
	return (rng);
}

static uint32_t
u32(uint32_t m)
{
	return ((uint32_t)(rnd() % m));
}

static void
report(int f, const char *why)
{
	nfail[f]++;
	if (nprinted[f]++ < 8)
		std::printf("  FAIL %-22s : %s\n", fname[f], why);
}

static bool
chk_int(int f, int pv, int rv)
{
	ncase[f]++;
	if (pv != rv) {
		report(f, "return");
		return (false);
	}
	return (true);
}

static void
fill_guard(unsigned char *b, size_t n)
{
	std::memset(b, GUARD, n);
}

static bool
run_split_lines(const unsigned char *content, size_t len)
{
	unsigned char buf_p[256], buf_r[256];
	char *p_p, *p_r;
	const char *plim_p, *plim_r;
	int pr, rr;
	int f = F_SPLIT_LINES;

	fill_guard(buf_p, sizeof(buf_p));
	fill_guard(buf_r, sizeof(buf_r));
	std::memcpy(buf_p + BUF_OFF, content, len);
	std::memcpy(buf_r + BUF_OFF, content, len);
	p_p = (char *)(buf_p + BUF_OFF);
	p_r = (char *)(buf_r + BUF_OFF);
	plim_p = p_p + len;
	plim_r = p_r + len;

	pr = P::split_lines(p_p, plim_p);
	rr = ref_split_lines(p_r, plim_r);
	if (!chk_int(f, pr, rr))
		return (false);
	if (std::memcmp(buf_p, buf_r, sizeof(buf_p)) != 0) {
		report(f, "buf");
		return (false);
	}
	return (true);
}

static void
edge_split_lines()
{
	static const unsigned char empty[] = "";
	static const unsigned char one_nl[] = "\n";
	static const unsigned char one_line[] = "a\n";
	static const unsigned char two_lines[] = "a\nb\n";
	static const unsigned char no_nl[] = "abc";
	static const unsigned char nul_heavy[] = "\0\n\0\n";
	static const unsigned char highbit[] = "\x80\n\xff\n";
	static const unsigned char consec[] = "\n\n\n";
	static const unsigned char mixed[] = "x\x80\ny\xff\nz\n";

	run_split_lines(empty, 0);
	run_split_lines(one_nl, 1);
	run_split_lines(one_line, 2);
	run_split_lines(two_lines, 4);
	run_split_lines(no_nl, 3);
	run_split_lines(nul_heavy, 4);
	run_split_lines(highbit, 4);
	run_split_lines(consec, 3);
	run_split_lines(mixed, 8);
}

static void
sweep_split_lines()
{
	unsigned char content[64];

	for (long long i = 0; i < SWEEP; i++) {
		size_t len = u32(48);
		for (size_t j = 0; j < len; j++) {
			switch (u32(5)) {
			case 0: content[j] = '\n'; break;
			case 1: content[j] = '\0'; break;
			case 2: content[j] = (unsigned char)(0x80 + u32(128));
				break;
			default: content[j] = (unsigned char)u32(256); break;
			}
		}
		run_split_lines(content, len);
	}
}

static bool
compare_dst(int f, int maxl, const char *dst_p[], const char *dst_r[],
    const char *base_p, const char *base_r)
{
	for (int i = 0; i < maxl; i++) {
		if ((dst_p[i] == nullptr) != (dst_r[i] == nullptr)) {
			report(f, "dst-null");
			return (false);
		}
		if (dst_p[i] == nullptr)
			continue;
		if ((dst_p[i] - base_p) != (dst_r[i] - base_r)) {
			report(f, "dst-off");
			return (false);
		}
		if (std::strcmp(dst_p[i], dst_r[i]) != 0) {
			report(f, "dst-str");
			return (false);
		}
	}
	return (true);
}

static bool
run_part_load(const char *name, const char *category, int maxl, int minl,
    int expect, char **reuse_p, char **reuse_r)
{
	const char *dst_p[16]{};
	const char *dst_r[16]{};
	char *buf_p = reuse_p ? *reuse_p : nullptr;
	char *buf_r = reuse_r ? *reuse_r : nullptr;
	int up = -1, ur = -1;
	int pr, rr, pe, re;
	int f = F_PART_LOAD;

	errno = 0;
	pr = P::__part_load_locale(name, &up, &buf_p, category, maxl, minl,
	    dst_p);
	pe = errno;
	errno = 0;
	rr = ref___part_load_locale(name, &ur, &buf_r, category, maxl, minl,
	    dst_r);
	re = errno;

	if (!chk_int(f, pr, rr))
		return (false);
	if (pr != expect) {
		report(f, "code");
		return (false);
	}
	if (pe != re) {
		report(f, "errno");
		return (false);
	}
	if (up != ur) {
		report(f, "using");
		return (false);
	}
	if (pr == LDP_LOADED) {
		if (buf_p == nullptr || buf_r == nullptr) {
			report(f, "lbuf-null");
			return (false);
		}
		if (std::strcmp(buf_p, buf_r) != 0) {
			report(f, "lbuf-name");
			return (false);
		}
		size_t ns = std::strlen(name) + 1;
		size_t fs = (size_t)ldhook().file_size;
		if (std::memcmp(buf_p + ns, buf_r + ns, fs) != 0) {
			report(f, "lbuf-file");
			return (false);
		}
		if (!compare_dst(f, maxl, dst_p, dst_r, buf_p, buf_r))
			return (false);
	}
	if (reuse_p)
		*reuse_p = buf_p;
	if (reuse_r)
		*reuse_r = buf_r;
	else {
		std::free(buf_p);
		std::free(buf_r);
	}
	return (true);
}

static void
edge_part_load()
{
	static const char good[] = "line1\nline2\nline3\n";
	static const char one_line[] = "only\n";
	static const char bad_nl[] = "noeol";
	static const char many_lines[] = "a\nb\nc\nd\ne\n";
	char *cache_p = nullptr;
	char *cache_r = nullptr;

	ldhook().file_content = good;
	ldhook().file_size = (off_t)(sizeof(good) - 1);

	run_part_load("C", "LC_FAKE", 8, 2, LDP_CACHE, nullptr, nullptr);
	run_part_load("POSIX", "LC_FAKE", 8, 2, LDP_CACHE, nullptr, nullptr);
	run_part_load("C.UTF-8", "LC_FAKE", 8, 2, LDP_CACHE, nullptr,
	    nullptr);
	run_part_load("en_US", "LC_FAKE", 8, 2, LDP_LOADED, nullptr,
	    nullptr);

	ldhook().open_fail = 1;
	ldhook().open_errno = ENOENT;
	run_part_load("xx", "LC_FAKE", 8, 2, LDP_ERROR, nullptr, nullptr);
	ldhook().open_fail = 0;

	ldhook().fstat_fail = 1;
	ldhook().fstat_errno = EIO;
	run_part_load("xx", "LC_FAKE", 8, 2, LDP_ERROR, nullptr, nullptr);
	ldhook().fstat_fail = 0;

	ldhook().file_size = 0;
	run_part_load("xx", "LC_FAKE", 8, 2, LDP_ERROR, nullptr, nullptr);
	ldhook().file_size = (off_t)(sizeof(good) - 1);

	ldhook().malloc_fail = 1;
	run_part_load("xx", "LC_FAKE", 8, 2, LDP_ERROR, nullptr, nullptr);
	ldhook().malloc_fail = 0;

	ldhook().file_content = bad_nl;
	ldhook().file_size = (off_t)(sizeof(bad_nl) - 1);
	run_part_load("xx", "LC_FAKE", 8, 2, LDP_ERROR, nullptr, nullptr);

	ldhook().file_content = one_line;
	ldhook().file_size = (off_t)(sizeof(one_line) - 1);
	run_part_load("xx", "LC_FAKE", 8, 3, LDP_ERROR, nullptr, nullptr);

	ldhook().read_fail = 1;
	ldhook().read_errno = EIO;
	ldhook().file_content = good;
	ldhook().file_size = (off_t)(sizeof(good) - 1);
	run_part_load("xx", "LC_FAKE", 8, 2, LDP_ERROR, nullptr, nullptr);
	ldhook().read_fail = 0;

	run_part_load("cached", "LC_FAKE", 8, 2, LDP_LOADED, &cache_p,
	    &cache_r);
	run_part_load("cached", "LC_FAKE", 8, 2, LDP_CACHE, &cache_p,
	    &cache_r);

	ldhook().file_content = many_lines;
	ldhook().file_size = (off_t)(sizeof(many_lines) - 1);
	run_part_load("clamp", "LC_FAKE", 3, 2, LDP_LOADED, nullptr,
	    nullptr);
	run_part_load("minok", "LC_FAKE", 8, 2, LDP_LOADED, nullptr,
	    nullptr);

	std::free(cache_p);
	std::free(cache_r);
}

static void
sweep_part_load()
{
	static const char *names[] = {
		"C", "POSIX", "C.foo", "en", "xx", "cached", "z"
	};
	char content[96];
	char *cache_p = nullptr;
	char *cache_r = nullptr;

	for (long long i = 0; i < SWEEP; i++) {
		int lines = 1 + (int)u32(8);
		size_t pos = 0;
		for (int l = 0; l < lines; l++) {
			switch (u32(4)) {
			case 0: content[pos++] = '\n'; break;
			case 1: content[pos++] = '\0'; break;
			case 2: content[pos++] = (char)(0x80 + u32(128));
				break;
			default: content[pos++] = (char)('a' + (l % 26));
				break;
			}
			if (u32(3) != 0)
				content[pos++] = '\n';
		}
		if (pos == 0 || content[pos - 1] != '\n')
			content[pos++] = '\n';
		ldhook().file_content = content;
		ldhook().file_size = (off_t)pos;
		ldhook().open_fail = (u32(20) == 0);
		ldhook().open_errno = ENOENT;
		ldhook().fstat_fail = (u32(25) == 0);
		ldhook().fstat_errno = EIO;
		ldhook().malloc_fail = (u32(30) == 0);
		ldhook().read_fail = (u32(35) == 0);
		ldhook().read_errno = EIO;

		const char *name = names[u32(7)];
		int maxl = 2 + (int)u32(7);
		int minl = 1 + (int)u32(maxl);
		int expect = LDP_ERROR;

		if (!ldhook().open_fail && !ldhook().fstat_fail &&
		    !ldhook().malloc_fail && !ldhook().read_fail &&
		    ldhook().file_size > 0 && content[pos - 1] == '\n') {
			if (std::strcmp(name, "C") == 0 ||
			    std::strcmp(name, "POSIX") == 0 ||
			    std::strncmp(name, "C.", 2) == 0)
				expect = LDP_CACHE;
			else {
				int nl = 0;
				for (size_t j = 0; j < (size_t)pos; j++)
					if (content[j] == '\n')
						nl++;
				if (nl >= minl)
					expect = LDP_LOADED;
			}
		}

		if (std::strcmp(name, "cached") == 0 && expect == LDP_LOADED &&
		    cache_p != nullptr) {
			expect = LDP_CACHE;
			run_part_load(name, "LC_FAKE", maxl, minl, expect,
			    &cache_p, &cache_r);
			continue;
		}

		run_part_load(name, "LC_CAT", maxl, minl, expect,
		    (std::strcmp(name, "cached") == 0) ? &cache_p : nullptr,
		    (std::strcmp(name, "cached") == 0) ? &cache_r : nullptr);
	}

	std::free(cache_p);
	std::free(cache_r);
}

int
main()
{
	pbsd_reset_hooks();
	edge_split_lines();
	sweep_split_lines();
	edge_part_load();
	sweep_part_load();

	std::printf("\n%-22s %12s %12s\n", "function", "cases", "failures");
	long long total_fail = 0;
	for (int i = 0; i < F_COUNT; i++) {
		std::printf("%-22s %12lld %12lld\n", fname[i], ncase[i],
		    nfail[i]);
		total_fail += nfail[i];
	}
	return (total_fail == 0 ? 0 : 1);
}
