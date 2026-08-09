/*
 * harness.cpp -- differential test for PBSD batch b0294.
 */

#include <arpa/inet.h>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>

import pbsd.sbin.ipf.libipf.b0294;

namespace P = pbsd::sbin_ipf_libipf::b0294;

#define	OPT_VERBOSE	0x000040

extern "C" {
extern int opts;
void ref_printactiveaddress(int v, char *fmt, P::i6addr_t *addr, char *ifname);
void ref_verbose(int level, char *fmt, ...);
void ref_ipfkverbose(char *fmt, ...);
extern struct {
	int on_value;
	int on_bit;
	int on_siz;
	char *on_name;
} ref_tcpoptnames[];
void ref_printsbuf(char *buf);
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

static Stat st_printactiveaddress = { "printactiveaddress", 0, 0, 0 };
static Stat st_verbose = { "verbose", 0, 0, 0 };
static Stat st_ipfkverbose = { "ipfkverbose", 0, 0, 0 };
static Stat st_tcpoptnames = { "tcpoptnames", 0, 0, 0 };
static Stat st_printsbuf = { "printsbuf", 0, 0, 0 };

static std::uint64_t rng_state = 0xb0294feedfaceULL;

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
sync_opts(int v)
{
	P::opts = v;
	opts = v;
}

static void
fill_str(char *buf, std::size_t cap, int pattern, std::size_t len)
{
	static const unsigned char alpha[] = {
	    0x00, 0x01, 0x7e, 0x7f, 0x80, 0xfe, 0xff,
	    'a', 'z', 'A', 'Z', '0', '9', 's', 'd', 'x'
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
		default:
			buf[i] = (char)(rnd() & 0xff);
			break;
		}
	}
	buf[len] = '\0';
}

static void
pick_safe_fmt(char *fmt, std::size_t cap, std::uint64_t seed)
{
	static const char *safe[] = {
	    "",
	    "%s",
	    "[%s]",
	    "addr=%s",
	    "%s%s",
	    "%d",
	    "%%",
	    "x=%x d=%d",
	    "%c",
	    "plain",
	    "ipfk\n",
	    "%s:%x",
	};
	const std::size_t n = sizeof(safe) / sizeof(safe[0]);
	std::snprintf(fmt, cap, "%s", safe[seed % n]);
}

static std::string
capture_stdout_paa(void (*fn)(int, char *, P::i6addr_t *, char *), int v,
    char *fmt, P::i6addr_t *addr, char *ifname)
{
	std::fflush(stdout);
	int fds[2];
	if (pipe(fds) != 0)
		std::abort();
	int saved = dup(STDOUT_FILENO);
	dup2(fds[1], STDOUT_FILENO);
	close(fds[1]);
	fn(v, fmt, addr, ifname);
	std::fflush(stdout);
	dup2(saved, STDOUT_FILENO);
	close(saved);
	char out[8192];
	ssize_t n = read(fds[0], out, sizeof(out) - 1);
	close(fds[0]);
	if (n < 0)
		n = 0;
	out[n] = '\0';
	return std::string(out);
}

static void
call_port_printactiveaddress(int v, char *fmt, P::i6addr_t *addr, char *ifname)
{
	P::printactiveaddress(v, fmt, addr, ifname);
}

static void
call_ref_printactiveaddress(int v, char *fmt, P::i6addr_t *addr, char *ifname)
{
	ref_printactiveaddress(v, fmt, addr, ifname);
}

static void
init_i6addr(P::i6addr_t *a, int family, std::uint32_t w0, std::uint32_t w1,
    std::uint32_t w2, std::uint32_t w3)
{
	std::memset(a, 0, sizeof(*a));
	if (family == 4) {
		a->in4.s_addr = w0;
	} else {
		a->i6[0] = w0;
		a->i6[1] = w1;
		a->i6[2] = w2;
		a->i6[3] = w3;
	}
}

static void
printactiveaddress_case(const char *tag, int v, const char *fmt,
    int family, std::uint32_t w0, std::uint32_t w1, std::uint32_t w2,
    std::uint32_t w3, const char *ifname)
{
	unsigned char arena_p[sizeof(P::i6addr_t) + 256];
	unsigned char arena_r[sizeof(P::i6addr_t) + 256];
	char fmt_p[128], fmt_r[128];
	char if_p[64], if_r[64];

	std::memset(arena_p, GUARD, sizeof(arena_p));
	std::memset(arena_r, GUARD, sizeof(arena_r));
	std::memset(fmt_p, GUARD, sizeof(fmt_p));
	std::memset(fmt_r, GUARD, sizeof(fmt_r));
	std::memset(if_p, GUARD, sizeof(if_p));
	std::memset(if_r, GUARD, sizeof(if_r));

	P::i6addr_t *ap = (P::i6addr_t *)(arena_p + 32);
	P::i6addr_t *ar = (P::i6addr_t *)(arena_r + 32);

	init_i6addr(ap, family, w0, w1, w2, w3);
	init_i6addr(ar, family, w0, w1, w2, w3);

	if (fmt != nullptr)
		std::snprintf(fmt_p, sizeof(fmt_p), "%s", fmt);
	else
		fmt_p[0] = '\0';
	std::memcpy(fmt_r, fmt_p, sizeof(fmt_p));

	char *ifname_p = nullptr;
	char *ifname_r = nullptr;
	if (ifname != (const char *)1) {
		if (ifname != nullptr)
			std::snprintf(if_p, sizeof(if_p), "%s", ifname);
		else
			if_p[0] = '\0';
		std::memcpy(if_r, if_p, sizeof(if_p));
		ifname_p = if_p;
		ifname_r = if_r;
	}

	std::string got = capture_stdout_paa(call_port_printactiveaddress, v,
	    fmt_p, ap, ifname_p);
	std::string ref = capture_stdout_paa(call_ref_printactiveaddress, v,
	    fmt_r, ar, ifname_r);

	st_printactiveaddress.cases++;
	int bad_out = (got != ref);
	int bad_p = (std::memcmp(arena_p, arena_r, sizeof(arena_p)) != 0);
	if (bad_out || bad_p) {
		stat_fail(&st_printactiveaddress, tag, bad_out ? "stdout" : "buf");
		if (st_printactiveaddress.reported <= MAX_REPORT) {
			std::printf("      v=%d port=%.*s\n", v, (int)got.size(),
			    got.c_str());
			std::printf("           ref=%.*s\n", (int)ref.size(),
			    ref.c_str());
		}
	}
}

static void
test_printactiveaddress_edges(void)
{
	static const int vs[] = { 0, 3, 4, 5, 6, 7, -1, (int)0x80000000 };
	static const char *fmts[] = {
	    "",
	    "%s",
	    "[%s]",
	    "addr=%s\n",
	    "%s%s",
	    "%d",
	    "%%",
	    "\x80%s",
	};
	static const char *ifnames[] = {
	    "",
	    "lo0",
	    "eth0",
	    "\xff\xfe",
	    (const char *)1,
	};

	for (std::size_t i = 0; i < sizeof(vs) / sizeof(vs[0]); i++) {
		for (std::size_t j = 0; j < sizeof(fmts) / sizeof(fmts[0]);
		     j++) {
			for (std::size_t k = 0;
			     k < sizeof(ifnames) / sizeof(ifnames[0]); k++) {
				char tag[80];
				std::snprintf(tag, sizeof(tag), "edge v%d f%zu",
				    vs[i], j);
				printactiveaddress_case(tag, vs[i], fmts[j], 4,
				    htonl(0x7f000001), 0, 0, 0, ifnames[k]);
				printactiveaddress_case(tag, vs[i], fmts[j], 6,
				    0x20010db8, 0x00000000, 0x00000000,
				    0x00010001, ifnames[k]);
			}
		}
	}

	printactiveaddress_case("v4-zero", 4, "%s", 4, 0, 0, 0, 0, "");
	printactiveaddress_case("v4-max", 4, "%s", 4, 0xffffffff, 0, 0, 0,
	    "if");
	printactiveaddress_case("v6-allff", 6, "%s", 6, 0xffffffff, 0xffffffff,
	    0xffffffff, 0xffffffff, "tun0");
}

static void
test_printactiveaddress_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		int v;
		switch (rnd_mod(6)) {
		case 0:
			v = 4;
			break;
		case 1:
			v = 6;
			break;
		case 2:
			v = 0;
			break;
		case 3:
			v = (int)(rnd() & 0xff);
			break;
		case 4:
			v = (int)(rnd() | 0x80000000);
			break;
		default:
			v = (int)rnd();
			break;
		}
		char fmt[64];
		pick_safe_fmt(fmt, sizeof(fmt), rnd());
		char ifname[48];
		fill_str(ifname, sizeof(ifname), (int)(rnd() % 3), rnd_mod(24));
		int family = (v == 6) ? 6 : 4;
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		printactiveaddress_case(tag, v, fmt, family,
		    (std::uint32_t)rnd(), (std::uint32_t)rnd(),
		    (std::uint32_t)rnd(), (std::uint32_t)rnd(),
		    ((rnd() & 3) == 0) ? (const char *)1 : ifname);
	}
}

/* ------------------------------------------------------------------------ */
/* verbose / ipfkverbose                                                     */
/* ------------------------------------------------------------------------ */

static std::string
capture_stdout_verbose_args(int level, const char *fmt, int a0, int a1,
    const char *s0, const char *s1)
{
	std::fflush(stdout);
	int fds[2];
	if (pipe(fds) != 0)
		std::abort();
	int saved = dup(STDOUT_FILENO);
	dup2(fds[1], STDOUT_FILENO);
	close(fds[1]);
	P::verbose(level, (char *)fmt, a0, a1, s0, s1);
	std::fflush(stdout);
	dup2(saved, STDOUT_FILENO);
	close(saved);
	char out[8192];
	ssize_t n = read(fds[0], out, sizeof(out) - 1);
	close(fds[0]);
	if (n < 0)
		n = 0;
	out[n] = '\0';
	return std::string(out);
}

static std::string
capture_stdout_verbose_args_ref(int level, const char *fmt, int a0, int a1,
    const char *s0, const char *s1)
{
	std::fflush(stdout);
	int fds[2];
	if (pipe(fds) != 0)
		std::abort();
	int saved = dup(STDOUT_FILENO);
	dup2(fds[1], STDOUT_FILENO);
	close(fds[1]);
	ref_verbose(level, (char *)fmt, a0, a1, s0, s1);
	std::fflush(stdout);
	dup2(saved, STDOUT_FILENO);
	close(saved);
	char out[8192];
	ssize_t n = read(fds[0], out, sizeof(out) - 1);
	close(fds[0]);
	if (n < 0)
		n = 0;
	out[n] = '\0';
	return std::string(out);
}

static std::string
capture_stdout_ipfk(const char *fmt, int a0, int a1, const char *s0,
    const char *s1, int use_ref)
{
	std::fflush(stdout);
	int fds[2];
	if (pipe(fds) != 0)
		std::abort();
	int saved = dup(STDOUT_FILENO);
	dup2(fds[1], STDOUT_FILENO);
	close(fds[1]);
	if (use_ref)
		ref_ipfkverbose((char *)fmt, a0, a1, s0, s1);
	else
		P::ipfkverbose((char *)fmt, a0, a1, s0, s1);
	std::fflush(stdout);
	dup2(saved, STDOUT_FILENO);
	close(saved);
	char out[8192];
	ssize_t n = read(fds[0], out, sizeof(out) - 1);
	close(fds[0]);
	if (n < 0)
		n = 0;
	out[n] = '\0';
	return std::string(out);
}

static void
verbose_case(const char *tag, int opts_val, int level, const char *fmt,
    int a0, int a1, const char *s0, const char *s1)
{
	sync_opts(opts_val);

	std::string got = capture_stdout_verbose_args(level, fmt, a0, a1, s0, s1);
	std::string ref = capture_stdout_verbose_args_ref(level, fmt, a0, a1, s0,
	    s1);

	st_verbose.cases++;
	if (got != ref) {
		stat_fail(&st_verbose, tag, "stdout");
		if (st_verbose.reported <= MAX_REPORT) {
			std::printf("      opts=0x%x port=%.*s\n", opts_val,
			    (int)got.size(), got.c_str());
			std::printf("                 ref=%.*s\n", (int)ref.size(),
			    ref.c_str());
		}
	}
}

static void
ipfkverbose_case(const char *tag, int opts_val, const char *fmt, int a0,
    int a1, const char *s0, const char *s1)
{
	sync_opts(opts_val);

	std::string got = capture_stdout_ipfk(fmt, a0, a1, s0, s1, 0);
	std::string ref = capture_stdout_ipfk(fmt, a0, a1, s0, s1, 1);

	st_ipfkverbose.cases++;
	if (got != ref) {
		stat_fail(&st_ipfkverbose, tag, "stdout");
		if (st_ipfkverbose.reported <= MAX_REPORT) {
			std::printf("      opts=0x%x port=%.*s\n", opts_val,
			    (int)got.size(), got.c_str());
			std::printf("                 ref=%.*s\n", (int)ref.size(),
			    ref.c_str());
		}
	}
}

static void
test_verbose_edges(void)
{
	static const int optvals[] = { 0, OPT_VERBOSE, 0x40, 0x000041,
	    (int)0xffffffbf, 0x000080, (int)0xffffffff };
	static const char *fmts[] = {
	    "",
	    "plain\n",
	    "%d",
	    "%x %d",
	    "%s",
	    "%s %s",
	    "%%",
	    "%c",
	};
	static const char *s0s[] = { "", "abc", "\x80\xff", "long-string-here" };
	static const char *s1s[] = { "", "xyz", "\xfe\xfd", "tail" };

	for (std::size_t o = 0; o < sizeof(optvals) / sizeof(optvals[0]); o++) {
		for (std::size_t f = 0; f < sizeof(fmts) / sizeof(fmts[0]);
		     f++) {
			for (std::size_t a = 0; a < sizeof(s0s) / sizeof(s0s[0]);
			     a++) {
				for (std::size_t b = 0;
				     b < sizeof(s1s) / sizeof(s1s[0]); b++) {
					char tag[80];
					std::snprintf(tag, sizeof(tag),
					    "edge o%zu f%zu", o, f);
					verbose_case(tag, optvals[o], 0, fmts[f],
					    -1, 0x7f808182, s0s[a], s1s[b]);
					verbose_case(tag, optvals[o],
					    0x1fffffff, fmts[f], 0, 255, s0s[a],
					    s1s[b]);
				}
			}
		}
	}
}

static void
test_verbose_sweep(void)
{
	char fmt[96];
	char s0[64];
	char s1[64];

	for (long i = 0; i < SWEEP; i++) {
		int opts_val = (int)rnd();
		if ((rnd() & 3) == 0)
			opts_val &= ~OPT_VERBOSE;
		if ((rnd() & 3) == 0)
			opts_val |= OPT_VERBOSE;
		pick_safe_fmt(fmt, sizeof(fmt), rnd());
		fill_str(s0, sizeof(s0), (int)(rnd() % 3), rnd_mod(32));
		fill_str(s1, sizeof(s1), (int)(rnd() % 3), rnd_mod(32));
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		verbose_case(tag, opts_val, (int)rnd(), fmt, (int)rnd(),
		    (int)rnd(), s0, s1);
	}
}

static void
test_ipfkverbose_edges(void)
{
	static const int optvals[] = { 0, OPT_VERBOSE, 0x40, 0x000041,
	    (int)0xffffffbf, (int)0xffffffff };
	static const char *fmts[] = {
	    "",
	    "ipfk\n",
	    "%d",
	    "%s",
	    "%s:%x",
	    "%%",
	};

	for (std::size_t o = 0; o < sizeof(optvals) / sizeof(optvals[0]); o++) {
		for (std::size_t f = 0; f < sizeof(fmts) / sizeof(fmts[0]); f++) {
			char tag[64];
			std::snprintf(tag, sizeof(tag), "edge o%zu f%zu", o, f);
			ipfkverbose_case(tag, optvals[o], fmts[f], 42, -7,
			    "iface", "\x80\x81");
			ipfkverbose_case(tag, optvals[o], fmts[f], 0, 0xffff,
			    "", "end");
		}
	}
}

static void
test_ipfkverbose_sweep(void)
{
	char fmt[96];
	char s0[64];
	char s1[64];

	for (long i = 0; i < SWEEP; i++) {
		int opts_val = (int)rnd();
		if ((rnd() & 3) == 0)
			opts_val &= ~OPT_VERBOSE;
		if ((rnd() & 3) == 0)
			opts_val |= OPT_VERBOSE;
		pick_safe_fmt(fmt, sizeof(fmt), rnd());
		fill_str(s0, sizeof(s0), (int)(rnd() % 3), rnd_mod(32));
		fill_str(s1, sizeof(s1), (int)(rnd() % 3), rnd_mod(32));
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		ipfkverbose_case(tag, opts_val, fmt, (int)rnd(), (int)rnd(), s0,
		    s1);
	}
}

/* ------------------------------------------------------------------------ */
/* tcpoptnames                                                               */
/* ------------------------------------------------------------------------ */

static constexpr int TCPOPT_TABLE_LEN = 7;

static void
tcpoptnames_case(const char *tag, int full_table)
{
	st_tcpoptnames.cases++;

	if (full_table) {
		for (int i = 0; i < TCPOPT_TABLE_LEN; i++) {
			if (P::tcpoptnames[i].on_value !=
			    ref_tcpoptnames[i].on_value ||
			    P::tcpoptnames[i].on_bit !=
			    ref_tcpoptnames[i].on_bit ||
			    P::tcpoptnames[i].on_siz !=
			    ref_tcpoptnames[i].on_siz ||
			    std::strcmp(P::tcpoptnames[i].on_name,
				ref_tcpoptnames[i].on_name) != 0) {
				stat_fail(&st_tcpoptnames, tag, "entry");
				if (st_tcpoptnames.reported <= MAX_REPORT) {
					std::printf("      idx=%d port=%d/%d/%d/%s\n",
					    i, P::tcpoptnames[i].on_value,
					    P::tcpoptnames[i].on_bit,
					    P::tcpoptnames[i].on_siz,
					    P::tcpoptnames[i].on_name ?
						P::tcpoptnames[i].on_name : "(null)");
				}
				return;
			}
		}
		return;
	}

	int i = (int)rnd_mod(TCPOPT_TABLE_LEN);
	if (P::tcpoptnames[i].on_value != ref_tcpoptnames[i].on_value ||
	    P::tcpoptnames[i].on_bit != ref_tcpoptnames[i].on_bit ||
	    P::tcpoptnames[i].on_siz != ref_tcpoptnames[i].on_siz ||
	    std::strcmp(P::tcpoptnames[i].on_name,
		ref_tcpoptnames[i].on_name) != 0) {
		stat_fail(&st_tcpoptnames, tag, "entry");
	}
}

static void
test_tcpoptnames_edges(void)
{
	tcpoptnames_case("full", 1);
	for (int i = 0; i < TCPOPT_TABLE_LEN; i++) {
		char tag[32];
		std::snprintf(tag, sizeof(tag), "idx%d", i);
		tcpoptnames_case(tag, 1);
	}
}

static void
test_tcpoptnames_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		tcpoptnames_case(tag, (rnd() & 127) == 0);
	}
}

/* ------------------------------------------------------------------------ */
/* printsbuf                                                                 */
/* ------------------------------------------------------------------------ */

static void
printsbuf_case(const char *tag, const char *input, int pattern)
{
	unsigned char ba[256], bb[256];

	std::memset(ba, GUARD, sizeof(ba));
	std::memset(bb, GUARD, sizeof(bb));

	if (input != nullptr) {
		std::size_t n = std::strlen(input);
		if (n > sizeof(ba) - 32)
			n = sizeof(ba) - 32;
		std::memcpy(ba + 16, input, n + 1);
		std::memcpy(bb + 16, input, n + 1);
	} else {
		std::size_t len = rnd_mod(sizeof(ba) - 32);
		fill_str((char *)(ba + 16), sizeof(ba) - 16, pattern, len);
		std::memcpy(bb + 16, ba + 16, len + 1);
	}

	char *pa = (char *)(ba + 16);
	char *pb = (char *)(bb + 16);

	P::printsbuf(pa);
	ref_printsbuf(pb);

	st_printsbuf.cases++;
	int bad_buf = (std::memcmp(ba, bb, sizeof(ba)) != 0);
	if (bad_buf) {
		stat_fail(&st_printsbuf, tag, "buf");
	}
}

static void
test_printsbuf_edges(void)
{
	static const char *inputs[] = {
	    "",
	    "a",
	    "\x00",
	    "\x7f",
	    "\x80",
	    "\xff",
	    "printable",
	    "\x01\x02\x03",
	    "\x80\x81\xff",
	    "mix\x80ok",
	};

	for (std::size_t i = 0; i < sizeof(inputs) / sizeof(inputs[0]); i++) {
		char tag[32];
		std::snprintf(tag, sizeof(tag), "edge%zu", i);
		printsbuf_case(tag, inputs[i], 0);
	}

	char hi[32];
	for (int i = 0; i < 31; i++)
		hi[i] = (char)(0x80 + (i & 0x7f));
	hi[31] = '\0';
	printsbuf_case("hibyte-run", hi, 1);
}

static void
test_printsbuf_sweep(void)
{
	for (long i = 0; i < SWEEP; i++) {
		char tag[48];
		std::snprintf(tag, sizeof(tag), "rnd%ld", i);
		printsbuf_case(tag, nullptr, (int)(rnd() % 3));
	}
}

/* ------------------------------------------------------------------------ */

int
main(void)
{
	test_printactiveaddress_edges();
	test_printactiveaddress_sweep();
	test_verbose_edges();
	test_verbose_sweep();
	test_ipfkverbose_edges();
	test_ipfkverbose_sweep();
	test_tcpoptnames_edges();
	test_tcpoptnames_sweep();
	test_printsbuf_edges();
	test_printsbuf_sweep();

	std::printf("\n%-20s %8s %8s\n", "function", "cases", "fails");
	std::printf("%-20s %8ld %8ld\n", st_printactiveaddress.name,
	    st_printactiveaddress.cases, st_printactiveaddress.fails);
	std::printf("%-20s %8ld %8ld\n", st_verbose.name, st_verbose.cases,
	    st_verbose.fails);
	std::printf("%-20s %8ld %8ld\n", st_ipfkverbose.name,
	    st_ipfkverbose.cases, st_ipfkverbose.fails);
	std::printf("%-20s %8ld %8ld\n", st_tcpoptnames.name,
	    st_tcpoptnames.cases, st_tcpoptnames.fails);
	std::printf("%-20s %8ld %8ld\n", st_printsbuf.name, st_printsbuf.cases,
	    st_printsbuf.fails);

	long total_fails = st_printactiveaddress.fails + st_verbose.fails +
	    st_ipfkverbose.fails + st_tcpoptnames.fails + st_printsbuf.fails;
	return total_fails == 0 ? 0 : 1;
}
