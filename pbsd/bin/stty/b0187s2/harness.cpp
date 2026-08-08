// Differential harness for PBSD batch b0187s2 (stty.c).

import pbsd.bin.stty.b0187s2;

#include <cerrno>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

namespace P = pbsd::bin_stty::b0187s2;

extern "C" {
struct pbsd_stty_hooks {
	unsigned print_calls;
	unsigned gprint_calls;
	unsigned gread_calls;
	unsigned checkredirect_calls;
	unsigned ksearch_hits;
	unsigned csearch_hits;
	unsigned msearch_hits;
	int last_print_fmt;
};
extern struct pbsd_stty_hooks pbsd_stty_hooks;

int ref_main(int argc, char **argv);
void ref_usage(void);

int __real_tcgetattr(int, struct termios *);
int __real_tcsetattr(int, int, const struct termios *);
int __real_ioctl(int, unsigned long, ...);
int __real_open(const char *, int, ...);
int __real_isatty(int);
int __real_fstat(int, struct stat *);
}

namespace {

constexpr long SWEEP = 200000L;
constexpr int MAX_SHOW = 8;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int shown;
};

struct HooksSnap {
	unsigned print_calls;
	unsigned gprint_calls;
	unsigned gread_calls;
	unsigned checkredirect_calls;
	unsigned ksearch_hits;
	unsigned csearch_hits;
	unsigned msearch_hits;
	int last_print_fmt;
};

struct RunOut {
	int status;
	std::vector<unsigned char> out;
	std::vector<unsigned char> err;
	HooksSnap hooks;
	int tcsetattr_calls;
	int ioctl_setwinsz_calls;
};

struct MockState {
	int active;
	int tcgetattr_ret;
	int tcsetattr_ret;
	int ioctl_getd_ret;
	int ioctl_winsz_ret;
	int ioctl_setwinsz_ret;
	int open_ret;
	int isatty_ret;
	int fstat_ret;
	int same_rdev;
	int tcsetattr_calls;
	int ioctl_setwinsz_calls;
} mock;

Stat st_main = { "main", 0, 0, 0 };
Stat st_usage = { "usage", 0, 0, 0 };

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
	bool bit() { return (u32() & 1u) != 0u; }
	int range(int lo, int hi)
	{
		if (hi <= lo)
			return lo;
		return lo + (int)(next() % (std::uint64_t)(hi - lo + 1));
	}
} rng(0xb0187a2ULL);

bool
fail(Stat &st, const char *msg)
{
	st.fails++;
	if (st.shown < MAX_SHOW) {
		st.shown++;
		std::printf("  FAIL %s: %s\n", st.name, msg);
	}
	return false;
}

void
reset_hooks()
{
	std::memset(&pbsd_stty_hooks, 0, sizeof(pbsd_stty_hooks));
}

HooksSnap
snap_hooks()
{
	HooksSnap h{};
	h.print_calls = pbsd_stty_hooks.print_calls;
	h.gprint_calls = pbsd_stty_hooks.gprint_calls;
	h.gread_calls = pbsd_stty_hooks.gread_calls;
	h.checkredirect_calls = pbsd_stty_hooks.checkredirect_calls;
	h.ksearch_hits = pbsd_stty_hooks.ksearch_hits;
	h.csearch_hits = pbsd_stty_hooks.csearch_hits;
	h.msearch_hits = pbsd_stty_hooks.msearch_hits;
	h.last_print_fmt = pbsd_stty_hooks.last_print_fmt;
	return h;
}

void
reset_mock(const MockState &m)
{
	mock = m;
	mock.active = 1;
}

RunOut
capture_run(bool port, int argc, char **argv)
{
	RunOut res{};
	int outpipe[2];
	int errpipe[2];

	if (pipe(outpipe) != 0 || pipe(errpipe) != 0)
		return res;

	pid_t pid = fork();
	if (pid < 0)
		return res;

	if (pid == 0) {
		dup2(outpipe[1], STDOUT_FILENO);
		dup2(errpipe[1], STDERR_FILENO);
		close(outpipe[0]);
		close(outpipe[1]);
		close(errpipe[0]);
		close(errpipe[1]);
		optind = 0;
		int st = port ? P::main(argc, argv) : ref_main(argc, argv);
		_exit(st);
	}

	close(outpipe[1]);
	close(errpipe[1]);
	unsigned char buf[4096];
	ssize_t nr;
	while ((nr = read(outpipe[0], buf, sizeof(buf))) > 0)
		res.out.insert(res.out.end(), buf, buf + nr);
	while ((nr = read(errpipe[0], buf, sizeof(buf))) > 0)
		res.err.insert(res.err.end(), buf, buf + nr);
	close(outpipe[0]);
	close(errpipe[0]);

	int wst = 0;
	if (waitpid(pid, &wst, 0) >= 0 && WIFEXITED(wst))
		res.status = WEXITSTATUS(wst);
	res.hooks = snap_hooks();
	res.tcsetattr_calls = mock.tcsetattr_calls;
	res.ioctl_setwinsz_calls = mock.ioctl_setwinsz_calls;
	return res;
}

bool
same_vec(const std::vector<unsigned char> &a,
    const std::vector<unsigned char> &b)
{
	return a == b;
}

bool
same_hooks(const HooksSnap &a, const HooksSnap &b)
{
	return a.print_calls == b.print_calls &&
	    a.gprint_calls == b.gprint_calls &&
	    a.gread_calls == b.gread_calls &&
	    a.checkredirect_calls == b.checkredirect_calls &&
	    a.ksearch_hits == b.ksearch_hits &&
	    a.csearch_hits == b.csearch_hits &&
	    a.msearch_hits == b.msearch_hits &&
	    a.last_print_fmt == b.last_print_fmt;
}

bool
cmp_main_run(const MockState &m, int argc, char **argv)
{
	Stat &st = st_main;
	st.cases++;

	reset_hooks();
	reset_mock(m);
	RunOut ref = capture_run(false, argc, argv);

	reset_hooks();
	reset_mock(m);
	RunOut port = capture_run(true, argc, argv);

	mock.active = 0;

	if (ref.status != port.status) {
		fail(st, "exit status mismatch");
		return false;
	}
	if (!same_vec(ref.out, port.out)) {
		fail(st, "stdout mismatch");
		return false;
	}
	if (!same_vec(ref.err, port.err)) {
		fail(st, "stderr mismatch");
		return false;
	}
	if (!same_hooks(ref.hooks, port.hooks)) {
		fail(st, "hook counters mismatch");
		return false;
	}
	if (ref.tcsetattr_calls != port.tcsetattr_calls) {
		fail(st, "tcsetattr call count mismatch");
		return false;
	}
	if (ref.ioctl_setwinsz_calls != port.ioctl_setwinsz_calls) {
		fail(st, "TIOCSWINSZ call count mismatch");
		return false;
	}
	return true;
}

MockState
ok_mock()
{
	MockState m{};
	m.tcgetattr_ret = 0;
	m.tcsetattr_ret = 0;
	m.ioctl_getd_ret = 0;
	m.ioctl_winsz_ret = 0;
	m.ioctl_setwinsz_ret = 0;
	m.open_ret = 5;
	m.isatty_ret = 1;
	m.fstat_ret = 0;
	m.same_rdev = 1;
	return m;
}

void
run_main_edge_cases()
{
	char prog[] = "stty";
	char empty[] = "";
	char a[] = "-a";
	char e[] = "-e";
	char g[] = "-g";
	char ae[] = "-ae";
	char aefg[] = "-aefg";
	char fz[] = "-fz";
	char fopt[] = "-f";
	char nofile[] = "/nonexistent_stty_b0187s2";
	char okfile[] = "/dev/null";
	char k[] = "__ksearch__";
	char c[] = "__csearch__";
	char m[] = "__msearch__";
	char sp9600[] = "9600";
	char sp0[] = "0";
	char spbad[] = "999999999999999999999";
	char gfmt[] = "gfmt1:cflag=0:";
	char ill[] = "not-an-option";
	char dig[] = "38400";
	char hb[] = "\x80\xFF";
	char zopt[] = "-z";
	char ax[] = "-ax";

	char *av_empty[] = { prog, nullptr };
	char *av_a[] = { prog, a, nullptr };
	char *av_e[] = { prog, e, nullptr };
	char *av_g[] = { prog, g, nullptr };
	char *av_ae[] = { prog, ae, nullptr };
	char *av_aefg[] = { prog, aefg, nullptr };
	char *av_f_bad[] = { prog, fopt, nofile, nullptr };
	char *av_f_ok[] = { prog, fopt, okfile, nullptr };
	char *av_k[] = { prog, k, nullptr };
	char *av_c[] = { prog, c, nullptr };
	char *av_m[] = { prog, m, nullptr };
	char *av_sp[] = { prog, sp9600, nullptr };
	char *av_sp0[] = { prog, sp0, nullptr };
	char *av_spbad[] = { prog, spbad, nullptr };
	char *av_gfmt[] = { prog, gfmt, nullptr };
	char *av_ill[] = { prog, ill, nullptr };
	char *av_dig[] = { prog, dig, nullptr };
	char *av_hb[] = { prog, hb, nullptr };
	char *av_z[] = { prog, zopt, nullptr };
	char *av_ax[] = { prog, ax, nullptr };
	char *av_fz[] = { prog, fz, nullptr };
	char *av_k_sp[] = { prog, k, sp9600, nullptr };
	char *av_g_k[] = { prog, g, k, nullptr };
	char *av_e_arg[] = { prog, e, dig, nullptr };

	cmp_main_run(ok_mock(), 1, av_empty);
	cmp_main_run(ok_mock(), 2, av_a);
	cmp_main_run(ok_mock(), 2, av_e);
	cmp_main_run(ok_mock(), 2, av_g);
	cmp_main_run(ok_mock(), 2, av_ae);
	cmp_main_run(ok_mock(), 2, av_aefg);
	cmp_main_run(ok_mock(), 3, av_f_ok);
	cmp_main_run(ok_mock(), 2, av_k);
	cmp_main_run(ok_mock(), 2, av_c);
	cmp_main_run(ok_mock(), 2, av_m);
	cmp_main_run(ok_mock(), 2, av_sp);
	cmp_main_run(ok_mock(), 2, av_sp0);
	cmp_main_run(ok_mock(), 2, av_gfmt);
	cmp_main_run(ok_mock(), 2, av_dig);
	cmp_main_run(ok_mock(), 2, av_z);
	cmp_main_run(ok_mock(), 2, av_ax);
	cmp_main_run(ok_mock(), 2, av_fz);
	cmp_main_run(ok_mock(), 3, av_k_sp);
	cmp_main_run(ok_mock(), 3, av_g_k);
	cmp_main_run(ok_mock(), 3, av_e_arg);

	MockState m = ok_mock();
	m.open_ret = -1;
	cmp_main_run(m, 3, av_f_bad);

	m = ok_mock();
	m.tcgetattr_ret = -1;
	cmp_main_run(m, 1, av_empty);

	m = ok_mock();
	m.ioctl_getd_ret = -1;
	cmp_main_run(m, 1, av_empty);

	m = ok_mock();
	m.ioctl_winsz_ret = -1;
	cmp_main_run(m, 1, av_empty);

	m = ok_mock();
	m.tcsetattr_ret = -1;
	cmp_main_run(m, 2, av_sp);

	m = ok_mock();
	m.ioctl_setwinsz_ret = -1;
	cmp_main_run(m, 2, av_m);

	m = ok_mock();
	m.isatty_ret = 1;
	m.same_rdev = 0;
	cmp_main_run(m, 1, av_empty);

	cmp_main_run(ok_mock(), 2, av_spbad);
	cmp_main_run(ok_mock(), 2, av_ill);
	cmp_main_run(ok_mock(), 2, av_hb);
}

void
run_main_sweep()
{
	std::vector<std::string> pool = {
		"", "-a", "-e", "-g", "-ae", "-af", "-ag", "-ef", "-fg",
		"-aefg", "-f", "/dev/null", "/nope", "__ksearch__",
		"__csearch__", "__msearch__", "0", "50", "9600", "38400",
		"115200", "gfmt1:", "gfmt1:cflag=1:", "illegal", "\x7f",
		"\x80", "\xff", "9", "4294967295", "999999999999999999999",
	};

	for (long i = 0; i < SWEEP; i++) {
		std::vector<std::string> args;
		args.emplace_back("stty");
		int n = rng.range(0, 6);
		for (int j = 0; j < n; j++) {
			const std::string &pick = pool[rng.range(0, (int)pool.size() - 1)];
			args.push_back(pick);
		}
		std::vector<char *> av;
		av.reserve(args.size() + 1);
		for (auto &s : args)
			av.push_back(s.data());
		av.push_back(nullptr);

		MockState m = ok_mock();
		switch (rng.range(0, 11)) {
		case 0:
			m.tcgetattr_ret = -1;
			break;
		case 1:
			m.ioctl_getd_ret = -1;
			break;
		case 2:
			m.ioctl_winsz_ret = -1;
			break;
		case 3:
			m.open_ret = -1;
			break;
		case 4:
			m.tcsetattr_ret = -1;
			break;
		case 5:
			m.ioctl_setwinsz_ret = -1;
			break;
		case 6:
			m.same_rdev = 0;
			break;
		case 7:
			m.isatty_ret = 0;
			break;
		default:
			break;
		}
		cmp_main_run(m, (int)av.size() - 1, av.data());
	}
}

bool
cmp_usage_run()
{
	Stat &st = st_usage;
	st.cases++;

	int errpipe[2];
	if (pipe(errpipe) != 0)
		return fail(st, "pipe");

	pid_t pr = fork();
	if (pr < 0)
		return fail(st, "fork ref");
	if (pr == 0) {
		dup2(errpipe[1], STDERR_FILENO);
		close(errpipe[0]);
		close(errpipe[1]);
		ref_usage();
		_exit(99);
	}
	close(errpipe[1]);
	std::vector<unsigned char> ref_err;
	unsigned char buf[4096];
	ssize_t nr;
	while ((nr = read(errpipe[0], buf, sizeof(buf))) > 0)
		ref_err.insert(ref_err.end(), buf, buf + nr);
	close(errpipe[0]);
	int wst = 0;
	int ref_status = -1;
	if (waitpid(pr, &wst, 0) >= 0 && WIFEXITED(wst))
		ref_status = WEXITSTATUS(wst);

	if (pipe(errpipe) != 0)
		return fail(st, "pipe2");

	pid_t pp = fork();
	if (pp < 0)
		return fail(st, "fork port");
	if (pp == 0) {
		dup2(errpipe[1], STDERR_FILENO);
		close(errpipe[0]);
		close(errpipe[1]);
		P::usage();
		_exit(99);
	}
	close(errpipe[1]);
	std::vector<unsigned char> port_err;
	while ((nr = read(errpipe[0], buf, sizeof(buf))) > 0)
		port_err.insert(port_err.end(), buf, buf + nr);
	close(errpipe[0]);
	int port_status = -1;
	if (waitpid(pp, &wst, 0) >= 0 && WIFEXITED(wst))
		port_status = WEXITSTATUS(wst);

	if (ref_status != port_status) {
		fail(st, "exit status mismatch");
		return false;
	}
	if (!same_vec(ref_err, port_err)) {
		fail(st, "stderr mismatch");
		return false;
	}
	return true;
}

void
run_usage_cases()
{
	for (int i = 0; i < 32; i++)
		cmp_usage_run();
}

} // namespace

extern "C" int
__wrap_tcgetattr(int fd, struct termios *tp)
{
	(void)fd;
	if (!mock.active)
		return __real_tcgetattr(fd, tp);
	if (mock.tcgetattr_ret < 0) {
		errno = ENOTTY;
		return -1;
	}
	std::memset(tp, 0, sizeof(*tp));
	tp->c_ospeed = B9600;
	tp->c_ispeed = B9600;
	return 0;
}

extern "C" int
__wrap_tcsetattr(int fd, int opt, const struct termios *tp)
{
	(void)fd;
	(void)opt;
	(void)tp;
	if (!mock.active)
		return __real_tcsetattr(fd, opt, tp);
	mock.tcsetattr_calls++;
	if (mock.tcsetattr_ret < 0) {
		errno = EIO;
		return -1;
	}
	return 0;
}

extern "C" int
__wrap_ioctl(int fd, unsigned long req, ...)
{
	va_list ap;
	void *arg = nullptr;
	va_start(ap, req);
	arg = va_arg(ap, void *);
	va_end(ap);

	if (!mock.active) {
		va_list ap2;
		va_start(ap2, req);
		void *a2 = va_arg(ap2, void *);
		va_end(ap2);
		return __real_ioctl(fd, req, a2);
	}
	(void)fd;
	if (req == TIOCGETD) {
		if (mock.ioctl_getd_ret < 0) {
			errno = EINVAL;
			return -1;
		}
		if (arg != nullptr)
			*(int *)arg = 0;
		return 0;
	}
	if (req == TIOCGWINSZ) {
		if (mock.ioctl_winsz_ret < 0) {
			errno = EINVAL;
			return -1;
		}
		if (arg != nullptr)
			std::memset(arg, 0, sizeof(struct winsize));
		return 0;
	}
	if (req == TIOCSWINSZ) {
		mock.ioctl_setwinsz_calls++;
		if (mock.ioctl_setwinsz_ret < 0) {
			errno = EINVAL;
			return -1;
		}
		return 0;
	}
	errno = EINVAL;
	return -1;
}

extern "C" int
__wrap_open(const char *path, int flags, ...)
{
	(void)path;
	(void)flags;
	if (!mock.active) {
		va_list ap;
		va_start(ap, flags);
		va_end(ap);
		return __real_open(path, flags);
	}
	if (mock.open_ret < 0) {
		errno = ENOENT;
		return -1;
	}
	return mock.open_ret;
}

extern "C" int
__wrap_isatty(int fd)
{
	(void)fd;
	if (!mock.active)
		return __real_isatty(fd);
	return mock.isatty_ret;
}

extern "C" int
__wrap_fstat(int fd, struct stat *sb)
{
	(void)fd;
	if (!mock.active)
		return __real_fstat(fd, sb);
	if (mock.fstat_ret < 0) {
		errno = EIO;
		return -1;
	}
	std::memset(sb, 0, sizeof(*sb));
	sb->st_rdev = mock.same_rdev ? 42 : (fd == STDOUT_FILENO ? 1 : 2);
	return 0;
}

int
main()
{
	run_usage_cases();
	run_main_edge_cases();
	run_main_sweep();

	std::printf("\n%-8s %8s %8s\n", "function", "cases", "fail");
	std::printf("%-8s %8ld %8ld\n", st_usage.name, st_usage.cases,
	    st_usage.fails);
	std::printf("%-8s %8ld %8ld\n", st_main.name, st_main.cases,
	    st_main.fails);

	long total_fails = st_usage.fails + st_main.fails;
	return total_fails == 0 ? 0 : 1;
}
