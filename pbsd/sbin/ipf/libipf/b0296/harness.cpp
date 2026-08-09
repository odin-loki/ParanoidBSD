/*
 * harness.cpp -- differential test for PBSD batch b0296.
 */

#include <arpa/inet.h>
#include <climits>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>

#include <fcntl.h>
#include <sys/ioctl.h>

import pbsd.sbin.ipf.libipf.b0296;

namespace P = pbsd::sbin_ipf_libipf::b0296;

#define OPT_NORESOLVE 0x8000000
#define OPT_DONTOPEN 0x10000000

struct ipfunc_resolve_ioctl {
	char ipfu_name[32];
	void *ipfu_addr;
	void *ipfu_init;
	void *ipfu_fini;
};

#ifndef SIOCFUNCL
#define SIOCFUNCL _IOWR('r', 86, struct ipfunc_resolve_ioctl)
#endif

extern "C" {
extern int opts;
extern int use_inet6;
int ref_count4bits(unsigned int ip);
void ref_printportcmp(int pr, P::frpcmp_t *frp);
int ref_ntomask(int family, int nbits, P::u_32_t *ap);
P::ipfunc_t ref_nametokva(char *name, P::ioctlfunc_t iocfunc);
}

static constexpr unsigned char GUARD = 0x7f;
static constexpr int MAX_REPORT = 8;
static constexpr long SWEEP = 200000;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

static Stat st_count4bits = { "count4bits", 0, 0, 0 };
static Stat st_printportcmp = { "printportcmp", 0, 0, 0 };
static Stat st_ntomask = { "ntomask", 0, 0, 0 };
static Stat st_nametokva = { "nametokva", 0, 0, 0 };

static std::uint64_t rng_state = 0xb0296feedfaceULL;

static int g_open_fail = 0;

extern "C" int __real_open(const char *, int, ...);
extern "C" int __real_close(int);

extern "C" int
__wrap_open(const char *path, int flags, ...)
{
	(void)path;
	(void)flags;
	if (g_open_fail)
		return -1;
	return 3;
}

extern "C" int
__wrap_close(int fd)
{
	return __real_close(fd);
}

static inline std::uint64_t
rnd(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static inline std::size_t
rnd_mod(std::size_t m)
{
	if (m == 0)
		return 0;
	return (std::size_t)(rnd() % (std::uint64_t)m);
}

static void
stat_fail(Stat *st, const char *tag, const char *detail)
{
	st->fails++;
	if (st->reported < MAX_REPORT) {
		st->reported++;
		std::printf("  FAIL %s [%s] %s\n", st->name, tag, detail);
	}
}

static void
sync_globals(int optv, int inet6v)
{
	P::opts = optv;
	opts = optv;
	P::use_inet6 = inet6v;
	use_inet6 = inet6v;
}

static void
fill_str(char *buf, std::size_t cap, int pattern, std::size_t len)
{
	static const unsigned char alpha[] = {
	    0x00, 0x01, 0x7e, 0x7f, 0x80, 0xfe, 0xff,
	    'a', 'z', 'A', 'Z', '0', '9', '/', ':', '-'
	};
	const std::size_t na = sizeof(alpha);

	if (cap == 0)
		return;
	if (len >= cap)
		len = cap - 1;
	for (std::size_t i = 0; i < len; i++) {
		switch (pattern) {
		case 0:
			buf[i] = (char)alpha[i % na];
			break;
		case 1:
			buf[i] = (char)(0x80 + (i & 0x7f));
			break;
		case 2:
			buf[i] = (char)((i & 1) ? '\0' : 'x');
			break;
		default:
			buf[i] = (char)(rnd() & 0xff);
			break;
		}
	}
	buf[len] = '\0';
}

/* ------------------------------------------------------------------------ */
/* count4bits                                                                */
/* ------------------------------------------------------------------------ */

static void
count4bits_case(const char *tag, unsigned int ip)
{
	int got = P::count4bits(ip);
	int ref = ref_count4bits(ip);

	st_count4bits.cases++;
	if (got != ref) {
		stat_fail(&st_count4bits, tag, "return");
		if (st_count4bits.reported <= MAX_REPORT) {
			std::printf("      ip=0x%08x port=%d ref=%d\n", ip, got,
			    ref);
		}
	}
}

static void
test_count4bits_edges(void)
{
	static const unsigned int host_masks[] = {
	    0x00000000U,
	    0x80000000U,
	    0xc0000000U,
	    0xe0000000U,
	    0xf0000000U,
	    0xf8000000U,
	    0xfc000000U,
	    0xfe000000U,
	    0xff000000U,
	    0xfff00000U,
	    0xffff0000U,
	    0xffffff00U,
	    0xfffffffeU,
	    0xffffffffU,
	};
	static const unsigned int invalid[] = {
	    0x00000001U,
	    0x00000080U,
	    0x7fffffffU,
	    0x80000001U,
	    0xc0000001U,
	    0xf00000ffU,
	    0x80808080U,
	    0x0000ffffU,
	    0xffff0001U,
	    0x01234567U,
	};

	for (std::size_t i = 0; i < sizeof(host_masks) / sizeof(host_masks[0]);
	     i++) {
		char tag[48];
		std::snprintf(tag, sizeof(tag), "valid%zu", i);
		count4bits_case(tag, htonl(host_masks[i]));
	}

	for (std::size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); i++) {
		char tag[48];
		std::snprintf(tag, sizeof(tag), "bad%zu", i);
		count4bits_case(tag, htonl(invalid[i]));
	}

	for (int b = 0; b <= 32; b++) {
		unsigned int m = (b == 0) ? 0U :
		    (0xffffffffU << (32 - b));
		char tag[48];
		std::snprintf(tag, sizeof(tag), "bits%d", b);
		count4bits_case(tag, htonl(m));
		if (b > 0 && b < 32) {
			std::snprintf(tag, sizeof(tag), "bits%d+1", b);
			count4bits_case(tag, htonl(m | 1U));
		}
	}

	count4bits_case("raw0", 0U);
	count4bits_case("rawff", 0xffffffffU);
	count4bits_case("raw80", 0x00000080U);
	count4bits_case("raw0080", 0x00008000U);
}

static void
test_count4bits_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		unsigned int ip;
		if ((rnd() & 7) == 0) {
			int b = (int)(rnd() % 33);
			ip = htonl((b == 0) ? 0U :
			    (0xffffffffU << (32 - b)));
			if ((rnd() & 3) == 0 && b > 0 && b < 32)
				ip = htonl(ntohl(ip) | (unsigned int)rnd());
		} else {
			ip = (unsigned int)rnd();
		}
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		count4bits_case(tag, ip);
	}
}

/* ------------------------------------------------------------------------ */
/* printportcmp                                                              */
/* ------------------------------------------------------------------------ */

static std::string
capture_stdout_pcmp(void (*fn)(int, P::frpcmp_t *), int pr, P::frpcmp_t *frp)
{
	std::fflush(stdout);
	int fds[2];
	if (pipe(fds) != 0)
		std::abort();
	int saved = dup(STDOUT_FILENO);
	dup2(fds[1], STDOUT_FILENO);
	close(fds[1]);
	fn(pr, frp);
	std::fflush(stdout);
	dup2(saved, STDOUT_FILENO);
	close(saved);
	char out[512];
	ssize_t n = read(fds[0], out, sizeof(out) - 1);
	close(fds[0]);
	if (n < 0)
		n = 0;
	out[n] = '\0';
	return std::string(out);
}

static void
call_port_printportcmp(int pr, P::frpcmp_t *frp)
{
	P::printportcmp(pr, frp);
}

static void
call_ref_printportcmp(int pr, P::frpcmp_t *frp)
{
	ref_printportcmp(pr, frp);
}

static void
printportcmp_case(const char *tag, int pr, P::fr_ctypes_t cmp,
    P::u_32_t port, P::u_32_t top)
{
	unsigned char arena_p[sizeof(P::frpcmp_t) + 64];
	unsigned char arena_r[sizeof(P::frpcmp_t) + 64];

	std::memset(arena_p, GUARD, sizeof(arena_p));
	std::memset(arena_r, GUARD, sizeof(arena_r));

	P::frpcmp_t *fp = (P::frpcmp_t *)(arena_p + 32);
	P::frpcmp_t *fr = (P::frpcmp_t *)(arena_r + 32);

	fp->frp_cmp = cmp;
	fp->frp_port = port;
	fp->frp_top = top;
	fr->frp_cmp = cmp;
	fr->frp_port = port;
	fr->frp_top = top;

	sync_globals(OPT_NORESOLVE, 0);

	std::string got = capture_stdout_pcmp(call_port_printportcmp, pr, fp);
	std::string ref = capture_stdout_pcmp(call_ref_printportcmp, pr, fr);

	st_printportcmp.cases++;
	int bad_out = (got != ref);
	int bad_buf = (std::memcmp(arena_p, arena_r, sizeof(arena_p)) != 0);
	if (bad_out || bad_buf) {
		stat_fail(&st_printportcmp, tag, bad_out ? "stdout" : "buf");
		if (st_printportcmp.reported <= MAX_REPORT) {
			std::printf("      port=%s\n", got.c_str());
			std::printf("      ref =%s\n", ref.c_str());
		}
	}
}

static void
test_printportcmp_edges(void)
{
	static const int prs[] = { -2, -1, 0, 6, 17, 255, INT_MAX, INT_MIN };
	static const P::fr_ctypes_t cmps[] = {
	    P::FR_NONE,
	    P::FR_EQUAL,
	    P::FR_NEQUAL,
	    P::FR_LESST,
	    P::FR_GREATERT,
	    P::FR_LESSTE,
	    P::FR_GREATERTE,
	    P::FR_OUTRANGE,
	    P::FR_INRANGE,
	    P::FR_INCRANGE,
	};

	for (std::size_t i = 0; i < sizeof(cmps) / sizeof(cmps[0]); i++) {
		for (std::size_t j = 0; j < sizeof(prs) / sizeof(prs[0]); j++) {
			char tag[64];
			std::snprintf(tag, sizeof(tag), "cmp%zu pr%zu", i, j);
			printportcmp_case(tag, prs[j], cmps[i], 0, 0);
			printportcmp_case(tag, prs[j], cmps[i], 1, 65535);
			printportcmp_case(tag, prs[j], cmps[i], 0x8080, 0x7f7f);
		}
	}

	printportcmp_case("range0", 6, P::FR_INRANGE, 1000, 2000);
	printportcmp_case("range1", 6, P::FR_OUTRANGE, 80, 443);
	printportcmp_case("incr", 17, P::FR_INCRANGE, 1024, 65535);
}

static void
test_printportcmp_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		int pr = (int)rnd();
		if ((rnd() & 7) == 0)
			pr = (int)(rnd_mod(20)) - 3;
		P::fr_ctypes_t cmp =
		    (P::fr_ctypes_t)(rnd_mod(P::FR_INCRANGE + 1));
		P::u_32_t port = (P::u_32_t)rnd();
		P::u_32_t top = (P::u_32_t)rnd();
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		printportcmp_case(tag, pr, cmp, port, top);
	}
}

/* ------------------------------------------------------------------------ */
/* ntomask                                                                   */
/* ------------------------------------------------------------------------ */

static void
ntomask_case(const char *tag, int family, int nbits, int inet6v)
{
	unsigned char buf_p[64];
	unsigned char buf_r[64];

	std::memset(buf_p, GUARD, sizeof(buf_p));
	std::memset(buf_r, GUARD, sizeof(buf_r));

	P::u_32_t *ap = (P::u_32_t *)(buf_p + 16);
	P::u_32_t *ar = (P::u_32_t *)(buf_r + 16);

	sync_globals(OPT_NORESOLVE, inet6v);

	int got = P::ntomask(family, nbits, ap);
	int ref = ref_ntomask(family, nbits, ar);

	st_ntomask.cases++;
	int bad_ret = (got != ref);
	int bad_buf = (std::memcmp(buf_p, buf_r, sizeof(buf_p)) != 0);
	if (bad_ret || bad_buf) {
		stat_fail(&st_ntomask, tag, bad_ret ? "return" : "buf");
		if (st_ntomask.reported <= MAX_REPORT) {
			std::printf("      fam=%d nbits=%d inet6=%d got=%d ref=%d\n",
			    family, nbits, inet6v, got, ref);
		}
	}
}

static void
test_ntomask_edges(void)
{
	static const int families[] = { 0, 2, 28, -1, 1, 99, INT_MAX };
	static const int nbits_vals[] = {
	    -1, 0, 1, 31, 32, 33, 64, 96, 97, 127, 128, 129
	};
	static const int inet6_vals[] = { -1, 0, 1 };

	for (std::size_t f = 0; f < sizeof(families) / sizeof(families[0]); f++) {
		for (std::size_t n = 0; n < sizeof(nbits_vals) / sizeof(nbits_vals[0]);
		     n++) {
			for (std::size_t u = 0;
			     u < sizeof(inet6_vals) / sizeof(inet6_vals[0]); u++) {
				char tag[80];
				std::snprintf(tag, sizeof(tag), "f%zu n%zu u%zu",
				    f, n, u);
				ntomask_case(tag, families[f], nbits_vals[n],
				    inet6_vals[u]);
			}
		}
	}

	for (int b = 0; b <= 128; b++) {
		char tag[48];
		std::snprintf(tag, sizeof(tag), "v6-%d", b);
		ntomask_case(tag, 28, b, 0);
		ntomask_case(tag, 0, b, 0);
	}

	for (int b = 0; b <= 32; b++) {
		char tag[48];
		std::snprintf(tag, sizeof(tag), "v4-%d", b);
		ntomask_case(tag, 2, b, 0);
		ntomask_case(tag, 2, b, 1);
	}
}

static void
test_ntomask_sweep(void)
{
	static const int families[] = { 0, 2, 28, 3, 10, 99 };

	for (long i = 0; i < SWEEP; i++) {
		int family = families[rnd_mod(sizeof(families) / sizeof(families[0]))];
		int nbits;
		if ((rnd() & 3) == 0)
			nbits = (int)(rnd() % 140) - 5;
		else if ((rnd() & 3) == 1)
			nbits = (int)(rnd() % 33);
		else
			nbits = (int)(rnd() % 129);
		int inet6v = (int)(rnd() % 3) - 1;
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		ntomask_case(tag, family, nbits, inet6v);
	}
}

/* ------------------------------------------------------------------------ */
/* nametokva                                                                 */
/* ------------------------------------------------------------------------ */

static int
mock_ntk_ioctl_addr(int fd, unsigned long cmd, ...)
{
	va_list ap;
	void *arg;

	va_start(ap, cmd);
	arg = va_arg(ap, void *);
	va_end(ap);
	(void)fd;
	if (cmd != (unsigned long)SIOCFUNCL || arg == nullptr)
		return -1;

	P::ipfunc_resolve_t *res = (P::ipfunc_resolve_t *)arg;
	std::uintptr_t h = 0;
	for (std::size_t i = 0; res->ipfu_name[i] != '\0'; i++)
		h = h * 31 + (unsigned char)res->ipfu_name[i];
	res->ipfu_addr = (P::ipfunc_t)(h ? (h | 0x1000) : 0);
	return 0;
}

static int
mock_ntk_ioctl_null(int fd, unsigned long cmd, ...)
{
	va_list ap;
	void *arg;

	va_start(ap, cmd);
	arg = va_arg(ap, void *);
	va_end(ap);
	(void)fd;
	if (cmd != (unsigned long)SIOCFUNCL || arg == nullptr)
		return -1;

	P::ipfunc_resolve_t *res = (P::ipfunc_resolve_t *)arg;
	res->ipfu_addr = nullptr;
	return 0;
}

static int
mock_ntk_ioctl_set(int fd, unsigned long cmd, ...)
{
	va_list ap;
	void *arg;

	va_start(ap, cmd);
	arg = va_arg(ap, void *);
	va_end(ap);
	(void)fd;
	if (cmd != (unsigned long)SIOCFUNCL || arg == nullptr)
		return -1;

	P::ipfunc_resolve_t *res = (P::ipfunc_resolve_t *)arg;
	res->ipfu_addr = (P::ipfunc_t)0xdeadbeef;
	return 0;
}

static void
nametokva_case(const char *tag, int optv, int open_fail, const char *name_in,
    P::ioctlfunc_t ioc)
{
	unsigned char name_p[128];
	unsigned char name_r[128];

	std::memset(name_p, GUARD, sizeof(name_p));
	std::memset(name_r, GUARD, sizeof(name_r));

	if (name_in != nullptr) {
		std::snprintf((char *)name_p + 16, sizeof(name_p) - 16, "%s",
		    name_in);
	} else {
		fill_str((char *)name_p + 16, sizeof(name_p) - 16,
		    (int)(rnd() % 4), rnd_mod(64));
	}
	std::memcpy(name_r + 16, name_p + 16, sizeof(name_r) - 16);

	sync_globals(optv, 0);
	g_open_fail = open_fail;

	P::ipfunc_t got = P::nametokva((char *)name_p + 16, ioc);
	P::ipfunc_t ref = ref_nametokva((char *)name_r + 16, ioc);

	st_nametokva.cases++;
	int got_null = (got == nullptr);
	int ref_null = (ref == nullptr);
	int bad_null = (got_null != ref_null);
	int bad_ptr = 0;

	if (!got_null && !ref_null)
		bad_ptr = (got != ref);
	else if (got_null != ref_null)
		bad_ptr = 1;

	int bad_buf = (std::memcmp(name_p, name_r, sizeof(name_p)) != 0);
	if (bad_null || bad_ptr || bad_buf) {
		stat_fail(&st_nametokva, tag,
		    bad_null ? "null" : (bad_ptr ? "ptr" : "buf"));
		if (st_nametokva.reported <= MAX_REPORT) {
			std::printf("      port=%p ref=%p name=%.*s\n",
			    (void *)got, (void *)ref, 48,
			    (char *)name_p + 16);
		}
	}
}

static void
test_nametokva_edges(void)
{
	static const char *names[] = {
	    "",
	    "a",
	    "lookup",
	    "\x80\xff",
	    "a\xff" "b",
	    "0123456789abcdef0123456789abcdef",
	    "long-name-that-exceeds-thirty-two-characters-ok",
	    "\x00hidden",
	    "nul\x00tail",
	};

	g_open_fail = 0;
	for (std::size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
		char tag[48];
		std::snprintf(tag, sizeof(tag), "name%zu", i);
		nametokva_case(tag, 0, 0, names[i], mock_ntk_ioctl_addr);
		nametokva_case(tag, OPT_DONTOPEN, 0, names[i],
		    mock_ntk_ioctl_addr);
	}

	nametokva_case("null-addr", OPT_DONTOPEN, 0, "x", mock_ntk_ioctl_null);
	nametokva_case("set-addr", OPT_DONTOPEN, 0, "y", mock_ntk_ioctl_set);
	nametokva_case("open-fail", 0, 1, "z", mock_ntk_ioctl_addr);
	nametokva_case("dontopen-fail", OPT_DONTOPEN, 1, "w",
	    mock_ntk_ioctl_addr);

	char hi[40];
	for (int i = 0; i < 35; i++)
		hi[i] = (char)(0x80 + (i & 0x7f));
	hi[35] = '\0';
	nametokva_case("hibyte", OPT_DONTOPEN, 0, hi, mock_ntk_ioctl_addr);
}

static void
test_nametokva_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		int optv = ((rnd() & 3) == 0) ? OPT_DONTOPEN : 0;
		int ofail = ((rnd() & 7) == 0 && optv == 0) ? 1 : 0;
		P::ioctlfunc_t ioc = mock_ntk_ioctl_addr;
		if ((rnd() & 15) == 0)
			ioc = mock_ntk_ioctl_null;
		else if ((rnd() & 31) == 0)
			ioc = mock_ntk_ioctl_set;
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		nametokva_case(tag, optv, ofail, nullptr, ioc);
	}
}

/* ------------------------------------------------------------------------ */

int
main(void)
{
	test_count4bits_edges();
	test_count4bits_sweep();
	test_printportcmp_edges();
	test_printportcmp_sweep();
	test_ntomask_edges();
	test_ntomask_sweep();
	test_nametokva_edges();
	test_nametokva_sweep();

	std::printf("\n%-14s %8s %8s\n", "function", "cases", "fails");
	std::printf("%-14s %8ld %8ld\n", st_count4bits.name,
	    st_count4bits.cases, st_count4bits.fails);
	std::printf("%-14s %8ld %8ld\n", st_printportcmp.name,
	    st_printportcmp.cases, st_printportcmp.fails);
	std::printf("%-14s %8ld %8ld\n", st_ntomask.name, st_ntomask.cases,
	    st_ntomask.fails);
	std::printf("%-14s %8ld %8ld\n", st_nametokva.name, st_nametokva.cases,
	    st_nametokva.fails);

	long total_fails = st_count4bits.fails + st_printportcmp.fails +
	    st_ntomask.fails + st_nametokva.fails;
	return total_fails == 0 ? 0 : 1;
}
