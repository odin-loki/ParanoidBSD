// Differential test harness for PBSD batch b0220 (dd args.c + parity/swapbytes).

import pbsd.bin.dd.b0220;

#include <cinttypes>
#include <climits>
#include <csetjmp>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace P = pbsd::bin_dd::b0220;

static const unsigned char GUARD = 0x7f;
static const long SWEEP = 200000L;

extern "C" {
void b0220_reset_state(void);
extern char *oper;
extern jmp_buf b0220_jmp;
extern int b0220_err_exit;

int ref_parity(unsigned char c);
void ref_swapbytes(void *v, size_t len);
intmax_t ref_postfix_to_mult(char expr);
int ref_c_arg(const void *a, const void *b);
int ref_c_conv(const void *a, const void *b);
int ref_c_iflag(const void *a, const void *b);
int ref_c_oflag(const void *a, const void *b);
uintmax_t ref_get_num(const char *val);
off_t ref_get_off_t(const char *val);
void ref_f_bs(char *arg);
void ref_f_cbs(char *arg);
void ref_f_count(char *arg);
void ref_f_files(char *arg);
void ref_f_fillchar(char *arg);
void ref_f_ibs(char *arg);
void ref_f_if(char *arg);
void ref_f_iflag(char *arg);
void ref_f_obs(char *arg);
void ref_f_of(char *arg);
void ref_f_oflag(char *arg);
void ref_f_seek(char *arg);
void ref_f_skip(char *arg);
void ref_f_speed(char *arg);
void ref_f_status(char *arg);
void ref_f_conv(char *arg);
void ref_jcl(char **argv);

void def(void);
void block(void);
void unblock(void);

extern uint64_t ddflags;
extern P::IO in, out;
extern size_t cbsz;
extern uintmax_t cpy_cnt, files_cnt;
extern char fill_char;
extern size_t speed;
extern const unsigned char *ctab;
extern void (*cfunc)(void);
}

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
};

static Stat stats[] = {
	{ "parity", 0, 0 },
	{ "swapbytes", 0, 0 },
	{ "postfix_to_mult", 0, 0 },
	{ "c_arg", 0, 0 },
	{ "c_conv", 0, 0 },
	{ "c_iflag", 0, 0 },
	{ "c_oflag", 0, 0 },
	{ "get_num", 0, 0 },
	{ "get_off_t", 0, 0 },
	{ "f_bs", 0, 0 },
	{ "f_cbs", 0, 0 },
	{ "f_count", 0, 0 },
	{ "f_files", 0, 0 },
	{ "f_fillchar", 0, 0 },
	{ "f_ibs", 0, 0 },
	{ "f_if", 0, 0 },
	{ "f_iflag", 0, 0 },
	{ "f_obs", 0, 0 },
	{ "f_of", 0, 0 },
	{ "f_oflag", 0, 0 },
	{ "f_seek", 0, 0 },
	{ "f_skip", 0, 0 },
	{ "f_speed", 0, 0 },
	{ "f_status", 0, 0 },
	{ "f_conv", 0, 0 },
	{ "jcl", 0, 0 },
};
static const int NSTAT = (int)(sizeof(stats) / sizeof(stats[0]));

static Stat &
S(const char *n)
{
	for (int i = 0; i < NSTAT; i++)
		if (std::strcmp(stats[i].name, n) == 0)
			return stats[i];
	std::fprintf(stderr, "bad stat %s\n", n);
	std::exit(2);
}

static void
fail(Stat &st, const char *msg)
{
	st.fails++;
	if (st.fails <= 8)
		std::printf("  FAIL %s: %s\n", st.name, msg);
}

static void
ok(Stat &st)
{
	st.cases++;
}

static struct Rng {
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

	std::uint32_t u32() { return (std::uint32_t)next(); }
} rng(0x00b0220ddULL);

static std::uint64_t
ctab_fp(const unsigned char *p)
{
	if (p == nullptr)
		return 0;
	std::uint64_t h = 0;
	for (int i = 0; i < 8; i++)
		h = h * 31 + p[i];
	h ^= (std::uint64_t)p[0x48] << 40;
	h ^= (std::uint64_t)p[0x80] << 24;
	h ^= (std::uint64_t)p[0xff] << 8;
	return h;
}

static int
cfunc_kind_ref(void (*f)(void))
{
	if (f == (void (*)(void))nullptr)
		return -1;
	if (f == def)
		return 0;
	if (f == block)
		return 1;
	if (f == unblock)
		return 2;
	return 99;
}

static int
cfunc_kind_port(void (*f)(void))
{
	if (f == nullptr)
		return -1;
	if (f == P::def)
		return 0;
	if (f == P::block)
		return 1;
	if (f == P::unblock)
		return 2;
	return 99;
}

struct Snap {
	std::uint64_t ddflags;
	std::size_t in_dbsz, out_dbsz, cbsz;
	std::uintmax_t cpy_cnt, files_cnt;
	char fill_char;
	std::size_t speed;
	const char *in_name, *out_name;
	off_t in_offset, out_offset;
	std::uint64_t ctab_fp_val;
	int cfunc_kind;
};

static Snap
snap_ref()
{
	Snap s{};
	s.ddflags = ddflags;
	s.in_dbsz = (std::size_t)in.dbsz;
	s.out_dbsz = (std::size_t)out.dbsz;
	s.cbsz = cbsz;
	s.cpy_cnt = cpy_cnt;
	s.files_cnt = files_cnt;
	s.fill_char = fill_char;
	s.speed = speed;
	s.in_name = in.name;
	s.out_name = out.name;
	s.in_offset = in.offset;
	s.out_offset = out.offset;
	s.ctab_fp_val = ctab_fp(ctab);
	s.cfunc_kind = cfunc_kind_ref(cfunc);
	return s;
}

static Snap
snap_port()
{
	Snap s{};
	s.ddflags = P::ddflags;
	s.in_dbsz = (std::size_t)P::in.dbsz;
	s.out_dbsz = (std::size_t)P::out.dbsz;
	s.cbsz = P::cbsz;
	s.cpy_cnt = P::cpy_cnt;
	s.files_cnt = P::files_cnt;
	s.fill_char = P::fill_char;
	s.speed = P::speed;
	s.in_name = P::in.name;
	s.out_name = P::out.name;
	s.in_offset = P::in.offset;
	s.out_offset = P::out.offset;
	s.ctab_fp_val = ctab_fp(P::ctab);
	s.cfunc_kind = cfunc_kind_port(P::cfunc);
	return s;
}

static bool
name_eq(const char *a, const char *b)
{
	if (a == b)
		return true;
	if (a == nullptr || b == nullptr)
		return a == b;
	return std::strcmp(a, b) == 0;
}

static bool
snap_eq(const Snap &a, const Snap &b)
{
	return a.ddflags == b.ddflags && a.in_dbsz == b.in_dbsz &&
	    a.out_dbsz == b.out_dbsz && a.cbsz == b.cbsz &&
	    a.cpy_cnt == b.cpy_cnt && a.files_cnt == b.files_cnt &&
	    a.fill_char == b.fill_char && a.speed == b.speed &&
	    name_eq(a.in_name, b.in_name) && name_eq(a.out_name, b.out_name) &&
	    a.in_offset == b.in_offset && a.out_offset == b.out_offset &&
	    a.ctab_fp_val == b.ctab_fp_val && a.cfunc_kind == b.cfunc_kind;
}

static void
reset_both()
{
	b0220_reset_state();
	P::reset_state();
}

static bool
guards_ok(const unsigned char *buf, std::size_t n)
{
	for (std::size_t i = 0; i < n; i++)
		if (buf[i] != GUARD)
			return false;
	return true;
}

/* ------------------------------------------------------------------ */
static void
test_parity_edges()
{
	Stat &st = S("parity");
	for (int v = 0; v < 256; v++) {
		int rr = ref_parity((unsigned char)v);
		int rp = P::parity((unsigned char)v);
		ok(st);
		if (rr != rp)
			fail(st, "mismatch");
	}
}

static void
test_parity_sweep(long n)
{
	Stat &st = S("parity");
	for (long i = 0; i < n; i++) {
		unsigned char v = (unsigned char)rng.u32();
		int rr = ref_parity(v);
		int rp = P::parity(v);
		ok(st);
		if (rr != rp)
			fail(st, "random");
	}
}

/* ------------------------------------------------------------------ */
static void
run_swapbytes(const unsigned char *in, std::size_t len, std::size_t cap)
{
	Stat &st = S("swapbytes");
	std::size_t total = cap + 2;
	auto *br = (unsigned char *)std::malloc(total);
	auto *bp = (unsigned char *)std::malloc(total);
	if (!br || !bp) {
		std::free(br);
		std::free(bp);
		std::fprintf(stderr, "oom\n");
		std::exit(2);
	}
	for (std::size_t i = 0; i < total; i++)
		br[i] = bp[i] = GUARD;
	std::memcpy(br + 1, in, len);
	std::memcpy(bp + 1, in, len);
	ref_swapbytes(br + 1, len);
	P::swapbytes(bp + 1, len);
	ok(st);
	if (std::memcmp(br, bp, total) != 0)
		fail(st, "buffer mismatch");
	if (!guards_ok(br, 1) || !guards_ok(br + 1 + len, total - 1 - len))
		fail(st, "ref guard");
	if (!guards_ok(bp, 1) || !guards_ok(bp + 1 + len, total - 1 - len))
		fail(st, "port guard");
	std::free(br);
	std::free(bp);
}

static void
test_swapbytes_edges()
{
	unsigned char empty[1] = { 0 };
	unsigned char one[] = { 0x80 };
	unsigned char two[] = { 0x01, 0xff };
	unsigned char three[] = { 0x7f, 0x80, 0xff };
	unsigned char nul[] = { 0, 0, 0, 0 };
	unsigned char hi[] = { 0xfe, 0x80, 0x7f, 0xff, 0x00, 0x81 };

	run_swapbytes(empty, 0, 0);
	run_swapbytes(one, 1, 4);
	run_swapbytes(two, 2, 4);
	run_swapbytes(three, 3, 8);
	run_swapbytes(nul, 4, 8);
	run_swapbytes(hi, 6, 16);
}

static void
test_swapbytes_sweep(long n)
{
	for (long i = 0; i < n; i++) {
		std::size_t len = (std::size_t)(rng.u32() % 64);
		unsigned char buf[64];
		for (std::size_t j = 0; j < len; j++)
			buf[j] = (unsigned char)rng.u32();
		run_swapbytes(buf, len, len + 8);
	}
}

/* ------------------------------------------------------------------ */
static void
test_postfix_edges()
{
	Stat &st = S("postfix_to_mult");
	const char *letters = "BbKkMmGgTtPpWw";
	for (const char *p = letters; *p; p++) {
		intmax_t rr = ref_postfix_to_mult(*p);
		intmax_t rp = P::postfix_to_mult(*p);
		ok(st);
		if (rr != rp)
			fail(st, "letter");
	}
	for (int c = 0; c < 256; c++) {
		if (std::strchr(letters, c) != nullptr)
			continue;
		intmax_t rr = ref_postfix_to_mult((char)c);
		intmax_t rp = P::postfix_to_mult((char)c);
		ok(st);
		if (rr != rp)
			fail(st, "non-letter");
	}
}

static void
test_postfix_sweep(long n)
{
	Stat &st = S("postfix_to_mult");
	for (long i = 0; i < n; i++) {
		char c = (char)(rng.u32() & 0xff);
		intmax_t rr = ref_postfix_to_mult(c);
		intmax_t rp = P::postfix_to_mult(c);
		ok(st);
		if (rr != rp)
			fail(st, "random");
	}
}

/* ------------------------------------------------------------------ */
struct NamePair {
	const char *a, *b;
};

static void
test_cmp(const char *stat, int (*ref)(const void *, const void *),
    int (*port)(const void *, const void *), const NamePair *pairs,
    int npairs)
{
	Stat &st = S(stat);
	for (int i = 0; i < npairs; i++) {
		int rr = ref(&pairs[i].a, &pairs[i].b);
		int rp = port(&pairs[i].a, &pairs[i].b);
		ok(st);
		if (rr != rp)
			fail(st, "cmp");
	}
}

static void
test_compare_edges()
{
	static const NamePair arg_pairs[] = {
		{ "bs", "cbs" }, { "cbs", "bs" }, { "conv", "conv" },
		{ "if", "of" }, { "seek", "skip" }, { "zzz", "bs" },
	};
	static const NamePair conv_pairs[] = {
		{ "ascii", "ebcdic" }, { "block", "unblock" },
		{ "sync", "osync" }, { "ibm", "ascii" },
	};
	static const NamePair iflag_pairs[] = {
		{ "direct", "fullblock" }, { "fullblock", "direct" },
		{ "direct", "direct" },
	};
	static const NamePair oflag_pairs[] = {
		{ "direct", "fsync" }, { "sync", "fsync" }, { "fsync", "direct" },
	};

	test_cmp("c_arg", ref_c_arg, P::c_arg, arg_pairs,
	    (int)(sizeof(arg_pairs) / sizeof(arg_pairs[0])));
	test_cmp("c_conv", ref_c_conv, P::c_conv, conv_pairs,
	    (int)(sizeof(conv_pairs) / sizeof(conv_pairs[0])));
	test_cmp("c_iflag", ref_c_iflag, P::c_iflag, iflag_pairs,
	    (int)(sizeof(iflag_pairs) / sizeof(iflag_pairs[0])));
	test_cmp("c_oflag", ref_c_oflag, P::c_oflag, oflag_pairs,
	    (int)(sizeof(oflag_pairs) / sizeof(oflag_pairs[0])));
}

static void
test_compare_sweep(long n)
{
	static const char *names[] = {
		"bs", "cbs", "conv", "ascii", "ebcdic", "block", "direct",
		"fsync", "if", "of", "skip", "seek", "sync", "unblock", "zzz",
	};
	const int nn = (int)(sizeof(names) / sizeof(names[0]));
	for (long i = 0; i < n; i++) {
		const char *a = names[rng.u32() % nn];
		const char *b = names[rng.u32() % nn];
		int rr = ref_c_arg(&a, &b);
		int rp = P::c_arg(&a, &b);
		ok(S("c_arg"));
		if (rr != rp)
			fail(S("c_arg"), "random");
		rr = ref_c_conv(&a, &b);
		rp = P::c_conv(&a, &b);
		ok(S("c_conv"));
		if (rr != rp)
			fail(S("c_conv"), "random");
		rr = ref_c_iflag(&a, &b);
		rp = P::c_iflag(&a, &b);
		ok(S("c_iflag"));
		if (rr != rp)
			fail(S("c_iflag"), "random");
		rr = ref_c_oflag(&a, &b);
		rp = P::c_oflag(&a, &b);
		ok(S("c_oflag"));
		if (rr != rp)
			fail(S("c_oflag"), "random");
	}
}

/* ------------------------------------------------------------------ */
static bool
run_get_num_ok(const char *val, uintmax_t *out)
{
	Stat &st = S("get_num");
	reset_both();
	oper = (char *)"test";
	P::oper = (char *)"test";
	uintmax_t rr = 0, rp = 0;
	int er = 0, ep = 0;
	if (setjmp(b0220_jmp) == 0)
		rr = ref_get_num(val);
	else
		er = b0220_err_exit;
	if (setjmp(P::b0220_jmp) == 0)
		rp = P::get_num(val);
	else
		ep = P::b0220_err_exit;
	ok(st);
	if (er != ep) {
		fail(st, "err exit");
		return false;
	}
	if (er != 0)
		return true;
	if (rr != rp) {
		fail(st, "value");
		return false;
	}
	if (out)
		*out = rr;
	return true;
}

static void
test_get_num_edges()
{
	uintmax_t v;
	(void)run_get_num_ok("0", &v);
	(void)run_get_num_ok("1", &v);
	(void)run_get_num_ok("512", &v);
	(void)run_get_num_ok("1023", &v);
	(void)run_get_num_ok("1b", &v);
	(void)run_get_num_ok("2B", &v);
	(void)run_get_num_ok("3k", &v);
	(void)run_get_num_ok("4K", &v);
	(void)run_get_num_ok("5m", &v);
	(void)run_get_num_ok("6M", &v);
	(void)run_get_num_ok("1g", &v);
	(void)run_get_num_ok("2G", &v);
	(void)run_get_num_ok("1t", &v);
	(void)run_get_num_ok("2T", &v);
	(void)run_get_num_ok("1p", &v);
	(void)run_get_num_ok("2P", &v);
	(void)run_get_num_ok("4w", &v);
	(void)run_get_num_ok("4W", &v);
	(void)run_get_num_ok("3x4", &v);
	(void)run_get_num_ok("3X4", &v);
	(void)run_get_num_ok("2*5", &v);
	(void)run_get_num_ok("10x20x30", &v);
	(void)run_get_num_ok("0x10", &v);
	(void)run_get_num_ok("010", &v);
	(void)run_get_num_ok("bad", nullptr);
	(void)run_get_num_ok("1q", nullptr);
	(void)run_get_num_ok("999999999999999999999999999999999999k", nullptr);
}

static char numbuf[64];

static void
test_get_num_sweep(long n)
{
	const char suffix[] = "bBkKmMgGtTpPwW";
	for (long i = 0; i < n; i++) {
		unsigned u = rng.u32() % 1000000u;
		int kind = (int)(rng.u32() % 8);
		switch (kind) {
		case 0:
			std::snprintf(numbuf, sizeof(numbuf), "%u", u);
			break;
		case 1:
			std::snprintf(numbuf, sizeof(numbuf), "%u%c",
			    u, suffix[rng.u32() % (sizeof(suffix) - 1)]);
			break;
		case 2:
			std::snprintf(numbuf, sizeof(numbuf), "%ux%u",
			    u % 1000 + 1, u % 500 + 1);
			break;
		case 3:
			std::snprintf(numbuf, sizeof(numbuf), "%u*%u",
			    u % 100 + 1, u % 50 + 1);
			break;
		default:
			std::snprintf(numbuf, sizeof(numbuf), "%u",
			    u % 7);
			break;
		}
		(void)run_get_num_ok(numbuf, nullptr);
	}
}

/* ------------------------------------------------------------------ */
static bool
run_get_off_ok(const char *val, off_t *out)
{
	Stat &st = S("get_off_t");
	reset_both();
	oper = (char *)"test";
	P::oper = (char *)"test";
	off_t rr = 0, rp = 0;
	int er = 0, ep = 0;
	if (setjmp(b0220_jmp) == 0)
		rr = ref_get_off_t(val);
	else
		er = b0220_err_exit;
	if (setjmp(P::b0220_jmp) == 0)
		rp = P::get_off_t(val);
	else
		ep = P::b0220_err_exit;
	ok(st);
	if (er != ep) {
		fail(st, "err exit");
		return false;
	}
	if (er != 0)
		return true;
	if (rr != rp) {
		fail(st, "value");
		return false;
	}
	if (out)
		*out = rr;
	return true;
}

static void
test_get_off_edges()
{
	off_t v;
	(void)run_get_off_ok("0", &v);
	(void)run_get_off_ok("1", &v);
	(void)run_get_off_ok("-1", &v);
	(void)run_get_off_ok("512", &v);
	(void)run_get_off_ok("2k", &v);
	(void)run_get_off_ok("-3m", &v);
	(void)run_get_off_ok("4x5", &v);
	(void)run_get_off_ok("bad", nullptr);
	(void)run_get_off_ok("1z", nullptr);
}

static void
test_get_off_sweep(long n)
{
	for (long i = 0; i < n; i++) {
		int sign = (rng.u32() & 1) ? -1 : 1;
		unsigned u = rng.u32() % 100000 + 1;
		const char sfx = "bBkKmM"[rng.u32() % 4];
		if (rng.u32() % 3 == 0)
			std::snprintf(numbuf, sizeof(numbuf), "%d%u", sign * (int)u,
			    (unsigned)(rng.u32() % 1000));
		else if (rng.u32() % 2 == 0)
			std::snprintf(numbuf, sizeof(numbuf), "%d%c",
			    sign * (int)(u % 10000 + 1), sfx);
		else
			std::snprintf(numbuf, sizeof(numbuf), "%u%c", u, sfx);
		(void)run_get_off_ok(numbuf, nullptr);
	}
}

/* ------------------------------------------------------------------ */
typedef void (*Farg)(char *);

static bool
run_f(const char *name, Farg ref, Farg port, const char *arg,
    std::uint64_t flags_before)
{
	Stat &st = S(name);
	char bufr[256];
	char bufp[256];
	std::strncpy(bufr, arg, sizeof(bufr) - 1);
	bufr[sizeof(bufr) - 1] = '\0';
	std::strncpy(bufp, arg, sizeof(bufp) - 1);
	bufp[sizeof(bufp) - 1] = '\0';
	reset_both();
	ddflags = flags_before;
	P::ddflags = flags_before;
	int er = 0, ep = 0;
	if (setjmp(b0220_jmp) == 0)
		ref(bufr);
	else
		er = b0220_err_exit;
	if (setjmp(P::b0220_jmp) == 0)
		port(bufp);
	else
		ep = P::b0220_err_exit;
	ok(st);
	if (er != ep) {
		fail(st, "err");
		return false;
	}
	if (er != 0)
		return true;
	Snap sr = snap_ref();
	Snap sp = snap_port();
	if (!snap_eq(sr, sp))
		fail(st, "state");
	return true;
}

static void
test_f_edges()
{
	(void)run_f("f_bs", ref_f_bs, P::f_bs, "512", 0);
	(void)run_f("f_cbs", ref_f_cbs, P::f_cbs, "80", 0);
	(void)run_f("f_count", ref_f_count, P::f_count, "0", 0);
	(void)run_f("f_count", ref_f_count, P::f_count, "10", 0);
	(void)run_f("f_files", ref_f_files, P::f_files, "3", 0);
	(void)run_f("f_fillchar", ref_f_fillchar, P::f_fillchar, "X", 0);
	(void)run_f("f_fillchar", ref_f_fillchar, P::f_fillchar, "\xff", 0);
	(void)run_f("f_ibs", ref_f_ibs, P::f_ibs, "1024", 0);
	(void)run_f("f_ibs", ref_f_ibs, P::f_ibs, "2048",
	    0x0000000000000004ULL);
	(void)run_f("f_if", ref_f_if, P::f_if, "/dev/zero", 0);
	(void)run_f("f_iflag", ref_f_iflag, P::f_iflag, "direct", 0);
	(void)run_f("f_iflag", ref_f_iflag, P::f_iflag, "direct,fullblock", 0);
	(void)run_f("f_obs", ref_f_obs, P::f_obs, "512", 0);
	(void)run_f("f_of", ref_f_of, P::f_of, "outfile", 0);
	(void)run_f("f_oflag", ref_f_oflag, P::f_oflag, "direct,fsync", 0);
	(void)run_f("f_seek", ref_f_seek, P::f_seek, "10", 0);
	(void)run_f("f_skip", ref_f_skip, P::f_skip, "5k", 0);
	(void)run_f("f_speed", ref_f_speed, P::f_speed, "1000", 0);
	(void)run_f("f_status", ref_f_status, P::f_status, "none", 0);
	(void)run_f("f_status", ref_f_status, P::f_status, "noxfer", 0);
	(void)run_f("f_status", ref_f_status, P::f_status, "progress", 0);
	(void)run_f("f_conv", ref_f_conv, P::f_conv, "ascii,swab,sync", 0);
	(void)run_f("f_conv", ref_f_conv, P::f_conv, "block", 0);
	(void)run_f("f_bs", ref_f_bs, P::f_bs, "0", 0);
	(void)run_f("f_fillchar", ref_f_fillchar, P::f_fillchar, "ab", 0);
}

/* ------------------------------------------------------------------ */
static bool
run_jcl(char **argv)
{
	Stat &st = S("jcl");
	reset_both();
	int er = 0, ep = 0;
	if (setjmp(b0220_jmp) == 0)
		ref_jcl(argv);
	else
		er = b0220_err_exit;
	if (setjmp(P::b0220_jmp) == 0)
		P::jcl(argv);
	else
		ep = P::b0220_err_exit;
	ok(st);
	if (er != ep) {
		fail(st, "err");
		return false;
	}
	if (er != 0)
		return true;
	Snap sr = snap_ref();
	Snap sp = snap_port();
	if (!snap_eq(sr, sp))
		fail(st, "state");
	return true;
}

static char arg0[] = "dd";
static char a_bs[] = "bs=512";
static char a_count[] = "count=1";
static char a_if[] = "if=/dev/null";
static char a_of[] = "of=/dev/null";
static char a_conv[] = "conv=ascii,swab";
static char a_cbs[] = "cbs=80";
static char a_block[] = "conv=block";
static char a_unblock[] = "conv=unblock";
static char a_ascii_cbs[] = "conv=ascii";
static char a_ebcdic[] = "conv=ebcdic";
static char a_ibs_obs[] = "ibs=1024";
static char a_obs[] = "obs=512";
static char a_bad[] = "notanoperand=1";
static char a_empty[] = "bs=";
static char a_fill[] = "fillchar=Z";
static char a_files[] = "files=2";
static char a_skip[] = "skip=1";
static char a_seek[] = "seek=2";
static char a_status[] = "status=progress";
static char a_iflag[] = "iflag=direct";
static char a_oflag[] = "oflag=sync";

static void
test_jcl_edges()
{
	char *av1[] = { arg0, a_bs, nullptr };
	char *av2[] = { arg0, a_bs, a_count, nullptr };
	char *av3[] = { arg0, a_if, a_of, a_conv, nullptr };
	char *av4[] = { arg0, a_cbs, a_block, nullptr };
	char *av5[] = { arg0, a_cbs, a_unblock, nullptr };
	char *av6[] = { arg0, a_cbs, a_ascii_cbs, nullptr };
	char *av7[] = { arg0, a_cbs, a_ebcdic, nullptr };
	char *av8[] = { arg0, a_bs, a_ibs_obs, a_obs, nullptr };
	char *av9[] = { arg0, a_bad, nullptr };
	char *av10[] = { arg0, a_empty, nullptr };
	char *av11[] = { arg0, a_fill, a_files, a_skip, a_seek, a_status,
	    a_iflag, a_oflag, nullptr };

	(void)run_jcl(av1);
	(void)run_jcl(av2);
	(void)run_jcl(av3);
	(void)run_jcl(av4);
	(void)run_jcl(av5);
	(void)run_jcl(av6);
	(void)run_jcl(av7);
	(void)run_jcl(av8);
	(void)run_jcl(av9);
	(void)run_jcl(av10);
	(void)run_jcl(av11);
}

static char jcl_bufs[8][64];
static char *jcl_argv[10];

static void
test_jcl_sweep(long n)
{
	const char *ops[] = {
		"bs", "ibs", "obs", "cbs", "count", "skip", "seek", "speed",
		"files", "fillchar", "status", "if", "of",
	};
	const char *vals[] = {
		"512", "1", "0", "80", "1024", "2k", "X", "none", "/dev/null",
		"progress",
	};
	const int nop = (int)(sizeof(ops) / sizeof(ops[0]));
	const int nval = (int)(sizeof(vals) / sizeof(vals[0]));

	for (long i = 0; i < n; i++) {
		int narg = (int)(rng.u32() % 5) + 1;
		jcl_argv[0] = arg0;
		for (int j = 0; j < narg; j++) {
			const char *op = ops[rng.u32() % nop];
			const char *val = vals[rng.u32() % nval];
			std::snprintf(jcl_bufs[j], sizeof(jcl_bufs[j]), "%s=%s",
			    op, val);
			jcl_argv[j + 1] = jcl_bufs[j];
		}
		jcl_argv[narg + 1] = nullptr;
		(void)run_jcl(jcl_argv);
	}
}

/* ------------------------------------------------------------------ */
int
main()
{
	test_parity_edges();
	test_swapbytes_edges();
	test_postfix_edges();
	test_compare_edges();
	test_get_num_edges();
	test_get_off_edges();
	test_f_edges();
	test_jcl_edges();

	test_parity_sweep(SWEEP / 10);
	test_swapbytes_sweep(SWEEP / 5);
	test_postfix_sweep(SWEEP / 10);
	test_compare_sweep(SWEEP / 10);
	test_get_num_sweep(SWEEP / 4);
	test_get_off_sweep(SWEEP / 5);
	test_jcl_sweep(SWEEP / 4);

	std::printf("\n%-20s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-20s %12s %12s\n", "--------", "-----", "--------");
	int bad = 0;
	for (int i = 0; i < NSTAT; i++) {
		std::printf("%-20s %12llu %12llu\n", stats[i].name,
		    stats[i].cases, stats[i].fails);
		if (stats[i].fails != 0)
			bad = 1;
	}
	return bad ? 1 : 0;
}
