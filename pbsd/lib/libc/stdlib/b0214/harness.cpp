/*
 * Differential test for batch b0214 (bsearch, ptsname*, strtoumax*, strtoul*).
 */

#include <cerrno>
#include <climits>
#include <cinttypes>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.stdlib.b0214;

namespace P = pbsd::lib_libc_stdlib::b0214;

extern "C" {
struct xlocale;
typedef struct xlocale *locale_t;

void *ref_bsearch(const void *, const void *, size_t, size_t,
    int (*)(const void *, const void *));
int ref_grantpt(int);
int ref_unlockpt(int);
int ref_ptsname_r(int, char *, size_t);
char *ref_ptsname(int);
uintmax_t ref_strtoumax_l(const char *, char **, int, locale_t);
uintmax_t ref_strtoumax(const char *, char **, int);
unsigned long ref_strtoul_l(const char *, char **, int, locale_t);
unsigned long ref_strtoul(const char *, char **, int);

extern int b0214_ioctl_fd;
extern int b0214_ioctl_ret;
extern int b0214_ioctl_errno;
extern int b0214_fdev_fail;
extern int b0214_fdev_errno;
extern char b0214_fdev_name[];
}

namespace {

enum Fn {
	F_BSEARCH,
	F_GRANTPT,
	F_UNLOCKPT,
	F_PTSNAME_R,
	F_PTSNAME,
	F_STRTOUMAX_L,
	F_STRTOUMAX,
	F_STRTOUL_L,
	F_STRTOUL,
	F_COUNT
};

static const char *const fn_name[F_COUNT] = {
	"bsearch",
	"grantpt",
	"unlockpt",
	"ptsname_r",
	"ptsname",
	"strtoumax_l",
	"strtoumax",
	"strtoul_l",
	"strtoul",
};

static unsigned long long n_cases[F_COUNT];
static unsigned long long n_fails[F_COUNT];
static unsigned long long reported[F_COUNT];

static const unsigned char GUARD = 0x7f;
static const size_t STRBUF = 256;
static const unsigned SWEEP_ITERS = 200000u;

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
rnd_sign(void)
{
	return (nextr() & 1u) ? 1 : -1;
}

static int
icmp(const void *a, const void *b)
{
	int x = *(const int *)a;
	int y = *(const int *)b;
	return (x > y) - (x < y);
}

static int
ucmp(const void *a, const void *b)
{
	unsigned x = *(const unsigned char *)a;
	unsigned y = *(const unsigned char *)b;
	return (x > y) - (x < y);
}

static void
reset_pty_mocks(int fd, int ioctl_ok, int ioctl_err, int fdev_fail,
    int fdev_err, const char *name)
{
	b0214_ioctl_fd = fd;
	b0214_ioctl_ret = ioctl_ok ? 0 : -1;
	b0214_ioctl_errno = ioctl_err;
	b0214_fdev_fail = fdev_fail;
	b0214_fdev_errno = fdev_err;
	std::strncpy(b0214_fdev_name, name, 63);
	b0214_fdev_name[63] = '\0';
}

/* ------------------------------------------------------------------ */
/* bsearch							      */
/* ------------------------------------------------------------------ */

static void
case_bsearch(const void *key, const void *base, size_t nmemb, size_t size,
    int (*compar)(const void *, const void *))
{
	void *rp = P::bsearch(key, base, nmemb, size, compar);
	void *rr = ref_bsearch(key, base, nmemb, size, compar);
	bool ok;

	if (rp == nullptr && rr == nullptr)
		ok = true;
	else if (rp == nullptr || rr == nullptr)
		ok = false;
	else
		ok = ((const char *)rp - (const char *)base) ==
		    ((const char *)rr - (const char *)base);

	record_case(F_BSEARCH, ok,
	    "nmemb=%zu size=%zu port_off=%td ref_off=%td",
	    nmemb, size,
	    rp ? (const char *)rp - (const char *)base : -1,
	    rr ? (const char *)rr - (const char *)base : -1);
}

static void
test_bsearch_edges(void)
{
	static const int vals[] = { 1, 3, 5, 7, 9, 11, 13, 15 };
	unsigned i, j;

	case_bsearch(&vals[0], vals, 0, sizeof(vals[0]), icmp);
	for (i = 0; i <= 8; i++) {
		int arr[8];
		for (j = 0; j < i; j++)
			arr[j] = (int)(j * 2);
		for (j = 0; j < i; j++) {
			int key = (int)(j * 2);
			case_bsearch(&key, arr, i, sizeof(arr[0]), icmp);
		}
		{
			int miss = (int)(i * 2 + 1);
			case_bsearch(&miss, arr, i, sizeof(arr[0]), icmp);
		}
	}
	{
		unsigned char bytes[] = { 0x00, 0x7f, 0x80, 0xff };
		unsigned char key = 0x80;
		case_bsearch(&key, bytes, 4, 1, ucmp);
	}
}

static void
test_bsearch_random(unsigned iters)
{
	unsigned i;

	for (i = 0; i < iters; i++) {
		int arr[32];
		size_t n = rnd_below(33);
		size_t j;

		for (j = 0; j < n; j++)
			arr[j] = (int)(j * 2 + (int)(nextr() & 1u));
		for (j = 1; j < n; j++)
			if (arr[j - 1] > arr[j]) {
				int t = arr[j - 1];
				arr[j - 1] = arr[j];
				arr[j] = t;
			}
		if (n == 0) {
			int key = (int)nextr();
			case_bsearch(&key, arr, n, sizeof(arr[0]), icmp);
			continue;
		}
		{
			size_t pick = rnd_below((unsigned)n);
			int key = arr[pick];
			case_bsearch(&key, arr, n, sizeof(arr[0]), icmp);
		}
		if ((i & 3u) == 0u) {
			int key = (int)nextr();
			case_bsearch(&key, arr, n, sizeof(arr[0]), icmp);
		}
	}
}

/* ------------------------------------------------------------------ */
/* ptsname*							      */
/* ------------------------------------------------------------------ */

static void
case_ptmaster(int fn, int (*port_fn)(int), int (*ref_fn)(int), int fd,
    int ioctl_ok, int ioctl_err)
{
	reset_pty_mocks(fd, ioctl_ok, ioctl_err, 0, 0, "pts/0");
	errno = 0;
	int rp = port_fn(fd);
	int re = errno;
	errno = 0;
	int rr = ref_fn(fd);
	int rerr = errno;
	record_case(fn, rp == rr && re == rerr,
	    "fd=%d port=%d/%d ref=%d/%d", fd, rp, re, rr, rerr);
}

static void
case_ptsname_r(size_t buflen, int fd, int ioctl_ok, int ioctl_err,
    int fdev_fail, int fdev_err, const char *devname)
{
	char pa[STRBUF], ra[STRBUF];
	int rp, rr, e1, e2;

	reset_pty_mocks(fd, ioctl_ok, ioctl_err, fdev_fail, fdev_err, devname);
	std::memset(pa, GUARD, sizeof(pa));
	std::memset(ra, GUARD, sizeof(ra));

	errno = 0;
	rp = P::ptsname_r(fd, pa, buflen);
	e1 = errno;
	errno = 0;
	rr = ref_ptsname_r(fd, ra, buflen);
	e2 = errno;

	bool ok = rp == rr && e1 == e2 &&
	    std::memcmp(pa, ra, sizeof(pa)) == 0;
	record_case(F_PTSNAME_R, ok,
	    "buflen=%zu fd=%d port=%d/%d ref=%d/%d", buflen, fd, rp, e1, rr, e2);
}

static void
case_ptsname(int fd, int ioctl_ok, int ioctl_err, int fdev_fail,
    int fdev_err, const char *devname)
{
	char ep[STRBUF], er[STRBUF];

	reset_pty_mocks(fd, ioctl_ok, ioctl_err, fdev_fail, fdev_err, devname);
	errno = 0;
	char *pp = P::ptsname(fd);
	int e1 = errno;
	if (pp != nullptr)
		std::snprintf(ep, sizeof(ep), "%s", pp);
	else
		ep[0] = '\0';

	errno = 0;
	char *pr = ref_ptsname(fd);
	int e2 = errno;
	if (pr != nullptr)
		std::snprintf(er, sizeof(er), "%s", pr);
	else
		er[0] = '\0';

	record_case(F_PTSNAME, (pp == nullptr) == (pr == nullptr) &&
	    e1 == e2 && std::strcmp(ep, er) == 0,
	    "fd=%d port=%s/%d ref=%s/%d", fd,
	    pp ? ep : "(null)", e1, pr ? er : "(null)", e2);
}

static void
test_pts_edges(void)
{
	case_ptmaster(F_GRANTPT, P::grantpt, ref_grantpt, 3, 1, 0);
	case_ptmaster(F_UNLOCKPT, P::unlockpt, ref_unlockpt, 3, 1, 0);
	case_ptmaster(F_GRANTPT, P::grantpt, ref_grantpt, 3, 0, EBADF);
	case_ptmaster(F_GRANTPT, P::grantpt, ref_grantpt, 3, 0, ENXIO);
	case_ptmaster(F_UNLOCKPT, P::unlockpt, ref_unlockpt, 9, 0, EINVAL);

	case_ptsname_r(0, 3, 1, 0, 0, 0, "pts/1");
	case_ptsname_r(1, 3, 1, 0, 0, 0, "pts/1");
	case_ptsname_r(5, 3, 1, 0, 0, 0, "pts/1");
	case_ptsname_r(6, 3, 1, 0, 0, 0, "pts/1");
	case_ptsname_r(32, 3, 1, 0, 0, 0, "pts/1");
	case_ptsname_r(64, 3, 0, EBADF, 0, 0, "pts/1");
	case_ptsname_r(64, 3, 1, 0, 1, EINVAL, "pts/1");
	case_ptsname_r(64, 3, 1, 0, 1, EIO, "pts/1");
	case_ptsname_r(128, 3, 1, 0, 0, 0, "");
	case_ptsname_r(128, 3, 1, 0, 0, 0, "pts/255");

	case_ptsname(3, 1, 0, 0, 0, "pts/0");
	case_ptsname(3, 0, EBADF, 0, 0, "pts/0");
	case_ptsname(3, 1, 0, 1, EINVAL, "pts/0");
}

static void
test_pts_random(unsigned iters)
{
	unsigned i;

	for (i = 0; i < iters; i++) {
		int fd = (int)(nextr() & 0xffu);
		int ioctl_ok = (int)(nextr() & 1u);
		int ioctl_err = (nextr() & 1u) ? EBADF : EINVAL;
		int fdev_fail = (int)(nextr() & 3u);
		int fdev_err = (int)((nextr() % 5u) + 1u);
		size_t buflen = (size_t)(nextr() % 200u);
		char name[16];

		std::snprintf(name, sizeof(name), "pts/%u", rnd_below(1000u));
		case_ptmaster(F_GRANTPT, P::grantpt, ref_grantpt, fd, ioctl_ok,
		    ioctl_err);
		case_ptmaster(F_UNLOCKPT, P::unlockpt, ref_unlockpt, fd,
		    ioctl_ok, ioctl_err);
		case_ptsname_r(buflen, fd, ioctl_ok, ioctl_err, fdev_fail,
		    fdev_err, name);
		case_ptsname(fd, ioctl_ok, ioctl_err, fdev_fail, fdev_err, name);
	}
}

/* ------------------------------------------------------------------ */
/* strtoul / strtoumax						      */
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
check_strto(unsigned fn_l, unsigned fn, bool use_l,
    const char *in, int base, bool use_endptr, P::locale_t loc)
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

	uintmax_t rmax_p = 0, rmax_r = 0;
	unsigned long rulp = 0, rulr = 0;
	int e1, e2;

	if (use_l) {
		if (fn_l == F_STRTOUMAX_L) {
			rmax_p = P::strtoumax_l(a, use_endptr ? &ea : nullptr,
			    base, loc);
			e1 = errno;
			errno = pre;
			rmax_r = ref_strtoumax_l(b, use_endptr ? &eb : nullptr,
			    base, loc);
			e2 = errno;
			bool ok = rmax_p == rmax_r && e1 == e2 &&
			    std::memcmp(a, b, sizeof(a)) == 0 &&
			    (!use_endptr ||
				(ea - a) == (eb - b));
			record_case(fn_l, ok,
			    "in=\"%s\" base=%d", in, base);
		} else {
			rulp = P::strtoul_l(a, use_endptr ? &ea : nullptr, base,
			    loc);
			e1 = errno;
			errno = pre;
			rulr = ref_strtoul_l(b, use_endptr ? &eb : nullptr, base,
			    loc);
			e2 = errno;
			bool ok = rulp == rulr && e1 == e2 &&
			    std::memcmp(a, b, sizeof(a)) == 0 &&
			    (!use_endptr ||
				(ea - a) == (eb - b));
			record_case(fn_l, ok,
			    "in=\"%s\" base=%d", in, base);
		}
	} else {
		if (fn == F_STRTOUMAX) {
			rmax_p = P::strtoumax(a, use_endptr ? &ea : nullptr,
			    base);
			e1 = errno;
			errno = pre;
			rmax_r = ref_strtoumax(b, use_endptr ? &eb : nullptr,
			    base);
			e2 = errno;
			bool ok = rmax_p == rmax_r && e1 == e2 &&
			    std::memcmp(a, b, sizeof(a)) == 0 &&
			    (!use_endptr ||
				(ea - a) == (eb - b));
			record_case(fn, ok,
			    "in=\"%s\" base=%d", in, base);
		} else {
			rulp = P::strtoul(a, use_endptr ? &ea : nullptr, base);
			e1 = errno;
			errno = pre;
			rulr = ref_strtoul(b, use_endptr ? &eb : nullptr, base);
			e2 = errno;
			bool ok = rulp == rulr && e1 == e2 &&
			    std::memcmp(a, b, sizeof(a)) == 0 &&
			    (!use_endptr ||
				(ea - a) == (eb - b));
			record_case(fn, ok,
			    "in=\"%s\" base=%d", in, base);
		}
	}
}

static void
check_both_strto(const char *in, int base, bool use_endptr, P::locale_t loc)
{
	check_strto(F_STRTOUMAX_L, F_STRTOUMAX, true, in, base, use_endptr, loc);
	check_strto(F_STRTOUL_L, F_STRTOUL, true, in, base, use_endptr, loc);
	check_strto(F_STRTOUMAX_L, F_STRTOUMAX, false, in, base, use_endptr, loc);
	check_strto(F_STRTOUL_L, F_STRTOUL, false, in, base, use_endptr, loc);
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
		"ffffffffffffffff",
		"18446744073709551615",
		"18446744073709551616",
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
		std::snprintf(buf, sizeof(buf), "%lu", ULONG_MAX);
		check_both_strto(buf, 10, true, loc);
		std::snprintf(buf, sizeof(buf), "%lu0", ULONG_MAX / 10);
		check_both_strto(buf, 10, true, loc);
		std::snprintf(buf, sizeof(buf), "%ju", (uintmax_t)UINTMAX_MAX);
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

} /* namespace */

int
main(void)
{
	unsigned fn;
	unsigned long long total_cases = 0, total_fails = 0;

	test_bsearch_edges();
	test_pts_edges();
	test_strto_edges();

	test_bsearch_random(SWEEP_ITERS);
	test_pts_random(SWEEP_ITERS);
	test_strto_random(SWEEP_ITERS);

	std::printf("\nbatch b0214 differential results\n");
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
