// Differential harness for PBSD batch b0227 (key.c, print.c, modes.c).

import pbsd.bin.stty.b0227;

#include <cerrno>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>

namespace P = pbsd::bin_stty::b0227;

static const unsigned char GUARD = 0x7f;
static const long SWEEP = 200000L;

#define TIOCEXT_VAL	0x54410096
#define TIOCSETD_VAL	0x54270027
#define TTYDISC_VAL	0

struct CInfo {
	int fd;
	int ldisc;
	int off;
	int set;
	int wset;
	const char *arg;
	P::termios t;
	P::winsize win;
};

static_assert(sizeof(CInfo) == sizeof(P::info), "info layout");

static CInfo *
as_cinfo(P::info *ip)
{
	return reinterpret_cast<CInfo *>(ip);
}

static const CInfo *
as_cinfo(const P::info *ip)
{
	return reinterpret_cast<const CInfo *>(ip);
}

struct OTermios {
	unsigned c_iflag;
	unsigned c_oflag;
	unsigned c_lflag;
	unsigned c_cflag;
	unsigned char c_cc[20];
	unsigned c_ispeed;
	unsigned c_ospeed;
};

struct OWin {
	unsigned short ws_row;
	unsigned short ws_col;
	unsigned short ws_xpixel;
	unsigned short ws_ypixel;
};

extern "C" {
int ref_msearch(char ***, CInfo *);
void ref_print(OTermios *, OWin *, int, int);
int ref_ksearch(char ***, CInfo *);
void ref_f_all(CInfo *);
void ref_f_cbreak(CInfo *);
void ref_f_columns(CInfo *);
void ref_f_dec(CInfo *);
void ref_f_ek(CInfo *);
void ref_f_everything(CInfo *);
void ref_f_extproc(CInfo *);
void ref_f_ispeed(CInfo *);
void ref_f_nl(CInfo *);
void ref_f_ospeed(CInfo *);
void ref_f_raw(CInfo *);
void ref_f_rows(CInfo *);
void ref_f_sane(CInfo *);
void ref_f_size(CInfo *);
void ref_f_speed(CInfo *);
void ref_f_tty(CInfo *);
}

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
};

static Stat stats[] = {
	{ "msearch", 0, 0 },
	{ "print", 0, 0 },
	{ "ksearch", 0, 0 },
	{ "f_cbreak", 0, 0 },
	{ "f_columns", 0, 0 },
	{ "f_dec", 0, 0 },
	{ "f_ek", 0, 0 },
	{ "f_extproc", 0, 0 },
	{ "f_ispeed", 0, 0 },
	{ "f_nl", 0, 0 },
	{ "f_ospeed", 0, 0 },
	{ "f_raw", 0, 0 },
	{ "f_rows", 0, 0 },
	{ "f_sane", 0, 0 },
	{ "f_size", 0, 0 },
	{ "f_speed", 0, 0 },
	{ "f_tty", 0, 0 },
	{ "f_all", 0, 0 },
	{ "f_everything", 0, 0 },
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

static int g_test_active = 0;
static int g_usage_count = 0;
static int g_warnx_count = 0;
static int g_err_count = 0;
static int g_err_eval = 0;
static int g_ioctl_ret = 0;
static unsigned long g_last_ioctl_req = 0;
static int g_ioctl_arg = 0;
static int g_ioctl_count = 0;

extern "C" int __real_ioctl(int, unsigned long, ...);

extern "C" void
__wrap_usage(void)
{
	if (g_test_active) {
		g_usage_count++;
		longjmp(g_usage_jmp, 1);
	}
	std::abort();
}

extern "C" void
__wrap_warnx(const char *fmt, ...)
{
	if (g_test_active) {
		g_warnx_count++;
		return;
	}
	va_list ap;
	va_start(ap, fmt);
	std::vfprintf(stderr, fmt, ap);
	va_end(ap);
	std::fputc('\n', stderr);
}

extern "C" void
__wrap_err(int eval, const char *fmt, ...)
{
	if (g_test_active) {
		g_err_count++;
		g_err_eval = eval;
		return;
	}
	va_list ap;
	va_start(ap, fmt);
	std::vfprintf(stderr, fmt, ap);
	va_end(ap);
	std::fputc('\n', stderr);
	std::exit(eval);
}

extern "C" int
__wrap_ioctl(int fd, unsigned long req, ...)
{
	va_list ap;
	void *arg = nullptr;
	va_start(ap, req);
	arg = va_arg(ap, void *);
	va_end(ap);

	if (g_test_active) {
		g_ioctl_count++;
		g_last_ioctl_req = req;
		if (arg != nullptr)
			g_ioctl_arg = *(int *)arg;
		return g_ioctl_ret;
	}
	return __real_ioctl(fd, req, arg);
}

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

	std::uint32_t u32() { return (std::uint32_t)next(); }
	std::uint8_t u8() { return (std::uint8_t)next(); }
	int i32() { return (int)u32(); }
	bool bit() { return (u32() & 1) != 0; }
} rng(0x00b0227ULL);

static void
reset_hooks()
{
	g_usage_count = 0;
	g_warnx_count = 0;
	g_err_count = 0;
	g_err_eval = 0;
	g_ioctl_ret = 0;
	g_last_ioctl_req = 0;
	g_ioctl_arg = 0;
	g_ioctl_count = 0;
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

static void
fill_termios(P::termios &t, Rng &r, bool hi)
{
	t.c_iflag = r.u32();
	t.c_oflag = r.u32();
	t.c_lflag = r.u32();
	t.c_cflag = r.u32() & 0x00ffffffu;
	for (int i = 0; i < 20; i++) {
		if (hi)
			t.c_cc[i] = (P::cc_t)(0x80u | (r.u8() & 0x7fu));
		else
			t.c_cc[i] = (P::cc_t)r.u8();
	}
	t.c_ispeed = r.u32();
	t.c_ospeed = r.u32();
}

static void
fill_info(P::info &ip, Rng &r, bool hi)
{
	ip.fd = r.i32();
	ip.ldisc = (int)(r.u32() % 8u);
	ip.off = r.bit() ? 1 : 0;
	ip.set = r.bit() ? 1 : 0;
	ip.wset = r.bit() ? 1 : 0;
	ip.arg = nullptr;
	fill_termios(ip.t, r, hi);
	ip.win.ws_row = (unsigned short)(r.u32() & 0xffffu);
	ip.win.ws_col = (unsigned short)(r.u32() & 0xffffu);
	ip.win.ws_xpixel = (unsigned short)(r.u32() & 0xffffu);
	ip.win.ws_ypixel = (unsigned short)(r.u32() & 0xffffu);
}

static void
dup_info(P::info &ip_r, P::info &ip_p, Rng &r, bool hi)
{
	fill_info(ip_r, r, hi);
	ip_p = ip_r;
}

static bool
same_info(const P::info &a, const P::info &b)
{
	if (a.fd != b.fd || a.ldisc != b.ldisc || a.off != b.off ||
	    a.set != b.set || a.wset != b.wset)
		return false;
	if (a.arg != b.arg &&
	    (a.arg == nullptr || b.arg == nullptr ||
		std::strcmp(a.arg, b.arg) != 0))
		return false;
	return std::memcmp(&a.t, &b.t, sizeof(a.t)) == 0 &&
	    std::memcmp(&a.win, &b.win, sizeof(a.win)) == 0;
}

static std::size_t
capture_stdout(void (*fn)(void *), void *ctx, char *buf, std::size_t cap)
{
	int pipefd[2];
	if (pipe(pipefd) < 0)
		return 0;
	int saved = dup(STDOUT_FILENO);
	dup2(pipefd[1], STDOUT_FILENO);
	std::fflush(stdout);
	close(pipefd[1]);
	fn(ctx);
	std::fflush(stdout);
	dup2(saved, STDOUT_FILENO);
	close(saved);
	std::size_t n = 0;
	ssize_t r;
	while (n < cap - 1) {
		r = read(pipefd[0], buf + n, cap - 1 - n);
		if (r <= 0)
			break;
		n += (std::size_t)r;
	}
	close(pipefd[0]);
	buf[n] = '\0';
	return n;
}

struct print_ctx {
	P::termios *tp;
	P::winsize *wp;
	int ldisc;
	P::FMT fmt;
	bool ref;
};

static void
run_print(void *vp)
{
	print_ctx *c = (print_ctx *)vp;
	if (c->ref)
		ref_print(reinterpret_cast<OTermios *>(c->tp),
		    reinterpret_cast<OWin *>(c->wp), c->ldisc,
		    static_cast<int>(c->fmt));
	else
		P::print(c->tp, c->wp, c->ldisc, c->fmt);
}

static void
cmp_print(P::termios &t, P::winsize &w, int ldisc, P::FMT fmt)
{
	Stat &st = S("print");
	char out_r[65536];
	char out_p[65536];
	print_ctx cr{ &t, &w, ldisc, fmt, true };
	print_ctx cp{ &t, &w, ldisc, fmt, false };
	std::size_t nr = capture_stdout(run_print, &cr, out_r, sizeof(out_r));
	std::size_t np = capture_stdout(run_print, &cp, out_p, sizeof(out_p));
	if (nr != np || std::memcmp(out_r, out_p, nr) != 0)
		fail(st, "stdout mismatch");
	ok(st);
}

static const char *mode_names[] = {
	"cs5", "cs6", "cs7", "cs8", "cstopb", "-cstopb", "cread", "-cread",
	"parenb", "-parenb", "parodd", "-parodd", "parity", "-parity",
	"evenp", "-evenp", "oddp", "-oddp", "pass8", "-pass8", "hupcl",
	"-hupcl", "hup", "-hup", "clocal", "-clocal", "crtscts", "-crtscts",
	"ctsflow", "-ctsflow", "dsrflow", "-dsrflow", "dtrflow", "-dtrflow",
	"rtsflow", "-rtsflow", "mdmbuf", "-mdmbuf", "rtsdtr", "-rtsdtr",
	"ignbrk", "-ignbrk", "brkint", "-brkint", "ignpar", "-ignpar",
	"parmrk", "-parmrk", "inpck", "-inpck", "istrip", "-istrip",
	"inlcr", "-inlcr", "igncr", "-igncr", "icrnl", "-icrnl", "ixon",
	"-ixon", "flow", "-flow", "ixoff", "-ixoff", "tandem", "-tandem",
	"ixany", "-ixany", "decctlq", "-decctlq", "imaxbel", "-imaxbel",
	"iutf8", "-iutf8", "echo", "-echo", "echoe", "-echoe", "crterase",
	"-crterase", "crtbs", "-crtbs", "echok", "-echok", "echoke",
	"-echoke", "crtkill", "-crtkill", "altwerase", "-altwerase",
	"iexten", "-iexten", "echonl", "-echonl", "echoctl", "-echoctl",
	"ctlecho", "-ctlecho", "echoprt", "-echoprt", "prterase",
	"-prterase", "isig", "-isig", "icanon", "-icanon", "noflsh",
	"-noflsh", "tostop", "-tostop", "flusho", "-flusho", "pendin",
	"-pendin", "crt", "-crt", "newcrt", "-newcrt", "nokerninfo",
	"-nokerninfo", "kerninfo", "-kerninfo", "opost", "-opost",
	"litout", "-litout", "onlcr", "-onlcr", "ocrnl", "-ocrnl",
	"tabs", "-tabs", "oxtabs", "-oxtabs", "tab0", "tab3", "onocr",
	"-onocr", "onlret", "-onlret",
	"", "x", "zz", "rawmode", "\xff", "cs9",
};
static const int NMODES =
    (int)(sizeof(mode_names) / sizeof(mode_names[0]));

static void
run_msearch_one(const char *name)
{
	Stat &st = S("msearch");
	char buf_r[64];
	char buf_p[64];
	std::memcpy(buf_r, name, std::strlen(name) + 1);
	std::memcpy(buf_p, name, std::strlen(name) + 1);
	char *av_r[2] = { buf_r, nullptr };
	char *av_p[2] = { buf_p, nullptr };
	char **argv_r = av_r;
	char **argv_p = av_p;
	P::info ip_r{};
	P::info ip_p{};
	fill_info(ip_r, rng, false);
	fill_info(ip_p, rng, false);
	ip_r.set = 0;
	ip_p.set = 0;
	g_test_active = 1;
	reset_hooks();
	int ret_r = ref_msearch(&argv_r, as_cinfo(&ip_r));
	int ret_p = P::msearch(&argv_p, &ip_p);
	g_test_active = 0;
	if (ret_r != ret_p)
		fail(st, "return mismatch");
	else if (!same_info(ip_r, ip_p))
		fail(st, "info mismatch");
	ok(st);
}

static const char *key_names[] = {
	"all", "cbreak", "-cbreak", "cols", "columns", "cooked", "dec", "ek",
	"everything", "extproc", "-extproc", "ispeed", "new", "nl", "-nl",
	"old", "ospeed", "raw", "-raw", "rows", "sane", "size", "speed",
	"tty", "-cols", "-speed", "bogus", "", "x",
};
static const int NKEYS =
    (int)(sizeof(key_names) / sizeof(key_names[0]));

static char argbuf[8][32];

static void
run_ksearch_one(const char *key, const char *arg, bool need_arg_path)
{
	Stat &st = S("ksearch");
	char k_r[64];
	char k_p[64];
	std::snprintf(k_r, sizeof(k_r), "%s", key);
	std::snprintf(k_p, sizeof(k_p), "%s", key);
	char *av_r[4];
	char *av_p[4];
	int n = 0;
	av_r[n] = k_r;
	av_p[n] = k_p;
	n++;
	if (arg != nullptr && *arg) {
		std::snprintf(argbuf[0], sizeof(argbuf[0]), "%s", arg);
		std::snprintf(argbuf[1], sizeof(argbuf[1]), "%s", arg);
		av_r[n] = argbuf[0];
		av_p[n] = argbuf[1];
		n++;
	}
	av_r[n] = nullptr;
	av_p[n] = nullptr;
	char **argv_r = av_r;
	char **argv_p = av_p;
	P::info ip_r{};
	P::info ip_p{};
	fill_info(ip_r, rng, true);
	fill_info(ip_p, rng, true);
	ip_r.off = 0;
	ip_p.off = 0;
	ip_r.set = 0;
	ip_p.set = 0;
	ip_r.wset = 0;
	ip_p.wset = 0;
	if (arg != nullptr && *arg) {
		ip_r.arg = argbuf[0];
		ip_p.arg = argbuf[1];
	}
	g_test_active = 1;
	reset_hooks();
	g_ioctl_ret = 0;
	int ret_r = ref_ksearch(&argv_r, as_cinfo(&ip_r));
	int u_r = g_usage_count;
	int w_r = g_warnx_count;
	int ret_p = P::ksearch(&argv_p, &ip_p);
	int u_p = g_usage_count;
	int w_p = g_warnx_count;
	g_test_active = 0;
	if (ret_r != ret_p)
		fail(st, "return mismatch");
	else if (u_r != u_p)
		fail(st, "usage count mismatch");
	else if (w_r != w_p)
		fail(st, "warnx count mismatch");
	else if (ret_r == 1 && !same_info(ip_r, ip_p))
		fail(st, "info mismatch");
	else if (ret_r == 0 && !same_info(ip_r, ip_p))
		fail(st, "info mismatch on miss");
	(void)need_arg_path;
	ok(st);
}

static void
cmp_f_void(const char *name, void (*ref_fn)(CInfo *), void (*port_fn)(P::info *),
    bool hi, int off)
{
	Stat &st = S(name);
	P::info ip_r{};
	P::info ip_p{};
	fill_info(ip_r, rng, hi);
	fill_info(ip_p, rng, hi);
	ip_r.off = off;
	ip_p.off = off;
	ip_r.set = 0;
	ip_p.set = 0;
	ip_r.wset = 0;
	ip_p.wset = 0;
	g_test_active = 1;
	reset_hooks();
	g_ioctl_ret = 0;
	ref_fn(as_cinfo(&ip_r));
	int ioctl_r = g_ioctl_count;
	int ioctl_arg_r = g_ioctl_arg;
	int err_r = g_err_count;
	int err_eval_r = g_err_eval;
	reset_hooks();
	g_ioctl_ret = 0;
	port_fn(&ip_p);
	int ioctl_p = g_ioctl_count;
	int ioctl_arg_p = g_ioctl_arg;
	int err_p = g_err_count;
	int err_eval_p = g_err_eval;
	g_test_active = 0;
	if (!same_info(ip_r, ip_p))
		fail(st, "info mismatch");
	else if (ioctl_r != ioctl_p)
		fail(st, "ioctl count mismatch");
	else if (ioctl_r > 0 && ioctl_arg_r != ioctl_arg_p)
		fail(st, "ioctl arg mismatch");
	else if (err_r != err_p)
		fail(st, "err count mismatch");
	else if (err_r > 0 && err_eval_r != err_eval_p)
		fail(st, "err eval mismatch");
	ok(st);
}

struct f_stdout_ctx {
	bool ref;
	P::info *ip;
	void (*ref_fn)(CInfo *);
	void (*port_fn)(P::info *);
};

static void
run_f_stdout(void *vp)
{
	f_stdout_ctx *c = (f_stdout_ctx *)vp;
	if (c->ref)
		c->ref_fn(as_cinfo(c->ip));
	else
		c->port_fn(c->ip);
}

static void
cmp_f_stdout(const char *stat_name, void (*ref_fn)(CInfo *),
    void (*port_fn)(P::info *))
{
	Stat &st = S(stat_name);
	P::info ip_r{};
	P::info ip_p{};
	fill_info(ip_r, rng, false);
	fill_info(ip_p, rng, false);
	char out_r[65536];
	char out_p[65536];
	f_stdout_ctx cr{ true, &ip_r, ref_fn, port_fn };
	f_stdout_ctx cp{ false, &ip_p, ref_fn, port_fn };
	std::size_t nr = capture_stdout(run_f_stdout, &cr, out_r, sizeof(out_r));
	std::size_t np = capture_stdout(run_f_stdout, &cp, out_p, sizeof(out_p));
	if (nr != np || std::memcmp(out_r, out_p, nr) != 0)
		fail(st, "stdout mismatch");
	ok(st);
}

static void
run_f_columns(const char *arg)
{
	Stat &st = S("f_columns");
	char ab_r[32];
	char ab_p[32];
	std::snprintf(ab_r, sizeof(ab_r), "%s", arg);
	std::snprintf(ab_p, sizeof(ab_p), "%s", arg);
	P::info ip_r{};
	P::info ip_p{};
	fill_info(ip_r, rng, false);
	fill_info(ip_p, rng, false);
	ip_r.arg = ab_r;
	ip_p.arg = ab_p;
	ip_r.wset = 0;
	ip_p.wset = 0;
	ref_f_columns(as_cinfo(&ip_r));
	P::f_columns(&ip_p);
	if (!same_info(ip_r, ip_p))
		fail(st, "info mismatch");
	ok(st);
}

static void
run_f_rows(const char *arg)
{
	Stat &st = S("f_rows");
	char ab_r[32];
	char ab_p[32];
	std::snprintf(ab_r, sizeof(ab_r), "%s", arg);
	std::snprintf(ab_p, sizeof(ab_p), "%s", arg);
	P::info ip_r{};
	P::info ip_p{};
	fill_info(ip_r, rng, false);
	fill_info(ip_p, rng, false);
	ip_r.arg = ab_r;
	ip_p.arg = ab_p;
	ip_r.wset = 0;
	ip_p.wset = 0;
	ref_f_rows(as_cinfo(&ip_r));
	P::f_rows(&ip_p);
	if (!same_info(ip_r, ip_p))
		fail(st, "info mismatch");
	ok(st);
}

static void
run_f_ispeed(const char *arg)
{
	Stat &st = S("f_ispeed");
	char ab_r[32];
	char ab_p[32];
	std::snprintf(ab_r, sizeof(ab_r), "%s", arg);
	std::snprintf(ab_p, sizeof(ab_p), "%s", arg);
	P::info ip_r{};
	P::info ip_p{};
	fill_info(ip_r, rng, true);
	fill_info(ip_p, rng, true);
	ip_r.arg = ab_r;
	ip_p.arg = ab_p;
	ip_r.set = 0;
	ip_p.set = 0;
	ref_f_ispeed(as_cinfo(&ip_r));
	P::f_ispeed(&ip_p);
	if (!same_info(ip_r, ip_p))
		fail(st, "info mismatch");
	ok(st);
}

static void
run_f_ospeed(const char *arg)
{
	Stat &st = S("f_ospeed");
	char ab_r[32];
	char ab_p[32];
	std::snprintf(ab_r, sizeof(ab_r), "%s", arg);
	std::snprintf(ab_p, sizeof(ab_p), "%s", arg);
	P::info ip_r{};
	P::info ip_p{};
	dup_info(ip_r, ip_p, rng, true);
	ip_r.arg = ab_r;
	ip_p.arg = ab_p;
	ip_r.set = 0;
	ip_p.set = 0;
	ref_f_ospeed(as_cinfo(&ip_r));
	P::f_ospeed(&ip_p);
	if (!same_info(ip_r, ip_p))
		fail(st, "info mismatch");
	ok(st);
}

int
main()
{
	static const int ldiscs[] = { 0, 4, 5, 1, 7, 99, -1, 0x7f, 0x80 };
	static const P::FMT fmts[] = {
		P::FMT::NOTSET, P::FMT::GFLAG, P::FMT::BSD, P::FMT::POSIX,
	};
	static const char *col_args[] = {
		"0", "1", "24", "80", "9999", "-1", "077", "0x80", "x", "",
	};
	static const char *speed_args[] = {
		"0", "50", "9600", "115200", "38400", "999999", "-5", "077",
		"0xFF",
	};

	for (int i = 0; i < NMODES; i++)
		run_msearch_one(mode_names[i]);
	for (long i = 0; i < SWEEP; i++) {
		int idx = (int)(rng.u32() % (unsigned)(NMODES + 20));
		if (idx < NMODES)
			run_msearch_one(mode_names[idx]);
		else {
			char tmp[16];
			std::snprintf(tmp, sizeof(tmp), "m%lu", (unsigned long)i);
			run_msearch_one(tmp);
		}
	}

	for (int hi = 0; hi < 2; hi++) {
		for (int li = 0; li < (int)(sizeof(ldiscs) / sizeof(ldiscs[0])); li++) {
			for (int fi = 0; fi < (int)(sizeof(fmts) / sizeof(fmts[0])); fi++) {
				P::termios t{};
				P::winsize w{};
				fill_termios(t, rng, hi != 0);
				w.ws_row = (unsigned short)rng.u32();
				w.ws_col = (unsigned short)rng.u32();
				cmp_print(t, w, ldiscs[li], fmts[fi]);
			}
		}
	}
	for (long i = 0; i < SWEEP; i++) {
		P::termios t{};
		P::winsize w{};
		fill_termios(t, rng, rng.bit());
		w.ws_row = (unsigned short)rng.u32();
		w.ws_col = (unsigned short)rng.u32();
		cmp_print(t, w, rng.i32(), fmts[rng.u32() % 4u]);
	}

	for (int i = 0; i < NKEYS; i++) {
		const char *arg = nullptr;
		if (std::strcmp(key_names[i], "cols") == 0 ||
		    std::strcmp(key_names[i], "columns") == 0 ||
		    std::strcmp(key_names[i], "rows") == 0)
			arg = "80";
		else if (std::strcmp(key_names[i], "ispeed") == 0 ||
		    std::strcmp(key_names[i], "ospeed") == 0)
			arg = "9600";
		run_ksearch_one(key_names[i], arg, false);
	}
	for (long i = 0; i < SWEEP; i++) {
		int ki = (int)(rng.u32() % (unsigned)NKEYS);
		const char *arg = nullptr;
		if (key_needs_arg(key_names[ki]) || rng.bit()) {
			std::snprintf(argbuf[2], sizeof(argbuf[2]), "%d", rng.i32());
			arg = argbuf[2];
		}
		run_ksearch_one(key_names[ki], arg, false);
	}

	for (int off = 0; off < 2; off++)
		cmp_f_void("f_cbreak", ref_f_cbreak, P::f_cbreak, true, off);
	for (long i = 0; i < SWEEP; i++)
		cmp_f_void("f_cbreak", ref_f_cbreak, P::f_cbreak, rng.bit(), rng.bit() ? 1 : 0);

	cmp_f_void("f_dec", ref_f_dec, P::f_dec, true, 0);
	cmp_f_void("f_ek", ref_f_ek, P::f_ek, false, 0);
	for (long i = 0; i < SWEEP; i++) {
		cmp_f_void("f_dec", ref_f_dec, P::f_dec, rng.bit(), 0);
		cmp_f_void("f_ek", ref_f_ek, P::f_ek, rng.bit(), 0);
	}

	for (int off = 0; off < 2; off++)
		cmp_f_void("f_extproc", ref_f_extproc, P::f_extproc, false, off);
	for (long i = 0; i < SWEEP; i++)
		cmp_f_void("f_extproc", ref_f_extproc, P::f_extproc, rng.bit(),
		    rng.bit() ? 1 : 0);

	for (int off = 0; off < 2; off++)
		cmp_f_void("f_nl", ref_f_nl, P::f_nl, true, off);
	for (long i = 0; i < SWEEP; i++)
		cmp_f_void("f_nl", ref_f_nl, P::f_nl, rng.bit(), rng.bit() ? 1 : 0);

	for (int off = 0; off < 2; off++)
		cmp_f_void("f_raw", ref_f_raw, P::f_raw, true, off);
	for (long i = 0; i < SWEEP; i++)
		cmp_f_void("f_raw", ref_f_raw, P::f_raw, rng.bit(), rng.bit() ? 1 : 0);

	cmp_f_void("f_sane", ref_f_sane, P::f_sane, true, 0);
	for (long i = 0; i < SWEEP; i++)
		cmp_f_void("f_sane", ref_f_sane, P::f_sane, rng.bit(), 0);

	for (int i = 0; i < (int)(sizeof(col_args) / sizeof(col_args[0])); i++)
		run_f_columns(col_args[i]);
	for (long i = 0; i < SWEEP; i++) {
		std::snprintf(argbuf[3], sizeof(argbuf[3]), "%d", rng.i32());
		run_f_columns(argbuf[3]);
	}

	for (int i = 0; i < (int)(sizeof(col_args) / sizeof(col_args[0])); i++)
		run_f_rows(col_args[i]);
	for (long i = 0; i < SWEEP; i++) {
		std::snprintf(argbuf[4], sizeof(argbuf[4]), "%d", rng.i32());
		run_f_rows(argbuf[4]);
	}

	for (int i = 0; i < (int)(sizeof(speed_args) / sizeof(speed_args[0])); i++)
		run_f_ispeed(speed_args[i]);
	for (long i = 0; i < SWEEP; i++) {
		std::snprintf(argbuf[5], sizeof(argbuf[5]), "%d", rng.i32());
		run_f_ispeed(argbuf[5]);
	}

	for (int i = 0; i < (int)(sizeof(speed_args) / sizeof(speed_args[0])); i++)
		run_f_ospeed(speed_args[i]);
	for (long i = 0; i < SWEEP; i++) {
		std::snprintf(argbuf[6], sizeof(argbuf[6]), "%d", rng.i32());
		run_f_ospeed(argbuf[6]);
	}

	cmp_f_stdout("f_size", ref_f_size, P::f_size);
	cmp_f_stdout("f_speed", ref_f_speed, P::f_speed);
	cmp_f_stdout("f_all", ref_f_all, P::f_all);
	cmp_f_stdout("f_everything", ref_f_everything, P::f_everything);
	for (long i = 0; i < SWEEP; i++) {
		cmp_f_stdout("f_size", ref_f_size, P::f_size);
		cmp_f_stdout("f_speed", ref_f_speed, P::f_speed);
		cmp_f_stdout("f_all", ref_f_all, P::f_all);
		cmp_f_stdout("f_everything", ref_f_everything, P::f_everything);
	}

	g_test_active = 1;
	reset_hooks();
	g_ioctl_ret = 0;
	cmp_f_void("f_tty", ref_f_tty, P::f_tty, false, 0);
	g_ioctl_ret = -1;
	cmp_f_void("f_tty", ref_f_tty, P::f_tty, false, 0);
	g_test_active = 0;
	for (long i = 0; i < SWEEP; i++) {
		g_test_active = 1;
		reset_hooks();
		g_ioctl_ret = rng.bit() ? 0 : -1;
		cmp_f_void("f_tty", ref_f_tty, P::f_tty, rng.bit(), 0);
		g_test_active = 0;
	}

	unsigned long long total_fail = 0;
	std::printf("function            cases     failures\n");
	std::printf("----------------------------------------\n");
	for (int i = 0; i < NSTAT; i++) {
		std::printf("%-18s %9llu %9llu\n", stats[i].name, stats[i].cases,
		    stats[i].fails);
		total_fail += stats[i].fails;
	}
	return total_fail ? 1 : 0;
}
