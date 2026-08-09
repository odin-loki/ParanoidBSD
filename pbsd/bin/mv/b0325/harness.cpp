// Differential test harness for PBSD batch b0325 (hbsd/src/bin/mv/mv.c).

#define _GNU_SOURCE

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <vector>

import pbsd.bin.mv.b0325;

namespace P = pbsd::bin_mv::b0325;

extern "C" {
extern int fflg;
extern int hflg;
extern int iflg;
extern int nflg;
extern int vflg;
int ref_main(int argc, char *argv[]);
int ref_do_move(const char *from, const char *to);
int ref_fastcopy(const char *from, const char *to, struct stat *sbp);
int ref_copy(const char *from, const char *to);
void ref_preserve_fd_acls(int, int, const char *, const char *);
void ref_usage(void);
}

#if defined(__linux__)
typedef void *acl_t;
typedef int acl_type_t;
extern "C" {
int fchflags(int fd, unsigned long flags)
{
	(void)fd;
	(void)flags;
	errno = EOPNOTSUPP;
	return -1;
}
acl_t acl_get_fd_np(int fd, acl_type_t type)
{
	(void)fd;
	(void)type;
	errno = EOPNOTSUPP;
	return nullptr;
}
int acl_is_trivial_np(acl_t acl, int *trivial)
{
	(void)acl;
	(void)trivial;
	errno = EINVAL;
	return -1;
}
int acl_set_fd_np(int fd, acl_t acl, acl_type_t type)
{
	(void)fd;
	(void)acl;
	(void)type;
	errno = EOPNOTSUPP;
	return -1;
}
int acl_free(acl_t acl)
{
	(void)acl;
	return 0;
}
}
#endif

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr std::size_t GUARD_PAD = 64;
constexpr long SWEEP = 50000L;
constexpr int MAX_SHOW = 8;

struct Stat {
	const char *name;
	long cases = 0;
	long fails = 0;
	int shown = 0;
};

struct Rng {
	std::uint64_t s;

	explicit Rng(std::uint64_t seed) : s(seed ? seed : 0xb0325faceULL) {}

	std::uint64_t next()
	{
		s += 0x9E3779B97F4A7C15ull;
		std::uint64_t z = s;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}

	int bits(int lo, int hi)
	{
		if (hi <= lo)
			return lo;
		return lo + static_cast<int>(next() % static_cast<std::uint64_t>(hi - lo + 1));
	}

	bool coin() { return (next() & 1u) != 0; }

	unsigned char byte() { return static_cast<unsigned char>(next() & 0xffu); }
};

Rng rng(0xb0325faceULL);

Stat st_preserve_fd_acls = { "preserve_fd_acls" };
Stat st_fastcopy = { "fastcopy" };
Stat st_copy = { "copy" };
Stat st_do_move = { "do_move" };
Stat st_main = { "main" };
Stat st_usage = { "usage" };

void
fail(Stat &st, const char *what)
{
	st.fails++;
	if (st.shown < MAX_SHOW) {
		st.shown++;
		std::printf("  FAIL %s: %s\n", st.name, what);
	}
}

struct GuardBuf {
	std::vector<unsigned char> raw;
	char *ptr;

	explicit GuardBuf(const char *s, std::size_t cap_extra = 0)
	{
		std::size_t n = s ? std::strlen(s) : 0;
		raw.assign(n + 2 * GUARD_PAD + cap_extra + 1, GUARD);
		ptr = reinterpret_cast<char *>(raw.data()) + GUARD_PAD;
		if (n > 0)
			std::memcpy(ptr, s, n);
		ptr[n] = '\0';
	}

	char *data() { return ptr; }

	const unsigned char *whole() const { return raw.data(); }

	std::size_t whole_size() const { return raw.size(); }
};

struct IoCapture {
	int saved = -1;
	int pipefd[2] = { -1, -1 };
	std::vector<unsigned char> bytes;

	bool begin(int fd)
	{
		saved = dup(fd);
		if (saved < 0)
			return false;
		if (pipe(pipefd) != 0) {
			close(saved);
			return false;
		}
		if (dup2(pipefd[1], fd) < 0) {
			close(pipefd[0]);
			close(pipefd[1]);
			close(saved);
			return false;
		}
		close(pipefd[1]);
		pipefd[1] = -1;
		return true;
	}

	std::vector<unsigned char> end(int fd)
	{
		unsigned char buf[4096];
		ssize_t nr;

		std::fflush(fd == STDOUT_FILENO ? stdout : stderr);
		while ((nr = read(pipefd[0], buf, sizeof(buf))) > 0)
			bytes.insert(bytes.end(), buf, buf + nr);
		close(pipefd[0]);
		dup2(saved, fd);
		close(saved);
		return bytes;
	}
};

struct TempRoot {
	char path[256];
	bool ok;

	TempRoot()
	{
		std::snprintf(path, sizeof(path), "/tmp/pbsd-mv-b0325-XXXXXX");
		ok = mkdtemp(path) != nullptr;
	}

	~TempRoot()
	{
		if (!ok)
			return;
		char cmd[512];
		std::snprintf(cmd, sizeof(cmd), "rm -rf '%s'", path);
		(void)std::system(cmd);
	}

	std::string join(const char *rel) const
	{
		return std::string(path) + "/" + rel;
	}
};

void
reset_flags()
{
	fflg = hflg = iflg = nflg = vflg = 0;
	P::fflg = P::hflg = P::iflg = P::nflg = P::vflg = 0;
}

void
set_flags(int f, int h, int i, int n, int v)
{
	fflg = f;
	hflg = h;
	iflg = i;
	nflg = n;
	vflg = v;
	P::fflg = f;
	P::hflg = h;
	P::iflg = i;
	P::nflg = n;
	P::vflg = v;
}

bool
write_file(const std::string &path, const void *data, std::size_t n, mode_t mode = 0644)
{
	int fd = open(path.c_str(), O_CREAT | O_TRUNC | O_WRONLY, mode);
	if (fd < 0)
		return false;
	ssize_t w = write(fd, data, n);
	close(fd);
	return w == static_cast<ssize_t>(n);
}

bool
read_file(const std::string &path, std::vector<unsigned char> &out)
{
	out.clear();
	int fd = open(path.c_str(), O_RDONLY);
	if (fd < 0)
		return false;
	unsigned char buf[8192];
	ssize_t nr;
	while ((nr = read(fd, buf, sizeof(buf))) > 0)
		out.insert(out.end(), buf, buf + nr);
	close(fd);
	return true;
}

bool
path_exists(const std::string &path)
{
	struct stat sb;
	return stat(path.c_str(), &sb) == 0;
}

bool
same_file_state(const std::string &a, const std::string &b)
{
	struct stat sa, sb;
	if (lstat(a.c_str(), &sa) != 0)
		return lstat(b.c_str(), &sb) != 0;
	if (lstat(b.c_str(), &sb) != 0)
		return false;
	if (!S_ISREG(sa.st_mode) || !S_ISREG(sb.st_mode))
		return sa.st_mode == sb.st_mode;
	std::vector<unsigned char> ca, cb;
	if (!read_file(a, ca) || !read_file(b, cb))
		return false;
	return ca == cb;
}

bool
compare_guard(const GuardBuf &a, const GuardBuf &b, Stat &st, const char *tag)
{
	if (std::memcmp(a.whole(), b.whole(), a.whole_size()) != 0) {
		fail(st, tag);
		return false;
	}
	return true;
}

bool
run_preserve_fd_acls_case(Stat &st, const char *label)
{
	st.cases++;
	TempRoot root;
	if (!root.ok)
		return fail(st, "mkdtemp"), false;

	std::string srcp = root.join("src");
	std::string dstp = root.join("dst");
	unsigned char payload[] = { 0x00, 0x7f, 0x80, 0xff, 'a' };
	if (!write_file(srcp, payload, sizeof(payload)))
		return fail(st, "setup"), false;
	if (!write_file(dstp, "z", 1))
		return fail(st, "setup"), false;

	int src_fd = open(srcp.c_str(), O_RDONLY);
	int dst_fd = open(dstp.c_str(), O_RDWR);
	if (src_fd < 0 || dst_fd < 0) {
		if (src_fd >= 0)
			close(src_fd);
		if (dst_fd >= 0)
			close(dst_fd);
		return fail(st, "open"), false;
	}

	IoCapture cap_r;
	if (!cap_r.begin(STDERR_FILENO))
		return fail(st, "capture"), false;
	ref_preserve_fd_acls(src_fd, dst_fd, srcp.c_str(), dstp.c_str());
	auto err_r = cap_r.end(STDERR_FILENO);
	close(src_fd);
	close(dst_fd);

	int src_fd2 = open(srcp.c_str(), O_RDONLY);
	int dst_fd2 = open(dstp.c_str(), O_RDWR);
	if (src_fd2 < 0 || dst_fd2 < 0) {
		if (src_fd2 >= 0)
			close(src_fd2);
		if (dst_fd2 >= 0)
			close(dst_fd2);
		return fail(st, "reopen"), false;
	}

	IoCapture cap_p;
	if (!cap_p.begin(STDERR_FILENO)) {
		close(src_fd2);
		close(dst_fd2);
		return fail(st, "capture"), false;
	}
	P::preserve_fd_acls(src_fd2, dst_fd2, srcp.c_str(), dstp.c_str());
	auto err_p = cap_p.end(STDERR_FILENO);
	close(src_fd2);
	close(dst_fd2);

	if (err_r != err_p) {
		fail(st, label);
		return false;
	}
	(void)label;
	return true;
}

bool
run_fastcopy_case(Stat &st, const char *label, int f, int h, int i, int n, int v,
    const std::vector<unsigned char> &content, bool dest_exists)
{
	st.cases++;
	reset_flags();
	set_flags(f, h, i, n, v);

	TempRoot ra, rb;
	if (!ra.ok || !rb.ok)
		return fail(st, "mkdtemp"), false;

	std::string from_a = ra.join("from");
	std::string to_a = ra.join("to");
	std::string from_b = rb.join("from");
	std::string to_b = rb.join("to");

	if (!write_file(from_a, content.data(), content.size()))
		return fail(st, "setup"), false;
	if (!write_file(from_b, content.data(), content.size()))
		return fail(st, "setup"), false;
	if (dest_exists) {
		write_file(to_a, "old", 3);
		write_file(to_b, "old", 3);
	}

	struct stat sba{}, sbb{};
	if (lstat(from_a.c_str(), &sba) != 0 || lstat(from_b.c_str(), &sbb) != 0)
		return fail(st, "lstat"), false;

	GuardBuf ga_from(from_a.c_str());
	GuardBuf gb_from(from_b.c_str());
	GuardBuf ga_to(to_a.c_str());
	GuardBuf gb_to(to_b.c_str());

	IoCapture cap_r;
	cap_r.begin(STDERR_FILENO);
	int rr = ref_fastcopy(ga_from.data(), ga_to.data(), &sba);
	auto er = cap_r.end(STDERR_FILENO);

	IoCapture cap_p;
	cap_p.begin(STDERR_FILENO);
	int rp = P::fastcopy(gb_from.data(), gb_to.data(), &sbb);
	auto ep = cap_p.end(STDERR_FILENO);

	if (!compare_guard(ga_from, gb_from, st, "from guard") ||
	    !compare_guard(ga_to, gb_to, st, "to guard"))
		return false;
	if (rr != rp) {
		fail(st, label);
		return false;
	}
	if (er != ep) {
		fail(st, "stderr");
		return false;
	}
	if (path_exists(to_a) && path_exists(to_b) && !same_file_state(to_a, to_b)) {
		fail(st, "dest content");
		return false;
	}
	if (path_exists(from_a) != path_exists(from_b)) {
		fail(st, "source existence");
		return false;
	}
	(void)label;
	return true;
}

bool
run_copy_case(Stat &st, const char *label, const char *kind)
{
	st.cases++;
	reset_flags();

	TempRoot ra, rb;
	if (!ra.ok || !rb.ok)
		return fail(st, "mkdtemp"), false;

	std::string from_a = ra.join("from");
	std::string to_a = ra.join("to");
	std::string from_b = rb.join("from");
	std::string to_b = rb.join("to");

	if (std::strcmp(kind, "file") == 0) {
		write_file(from_a, "payload", 7);
		write_file(from_b, "payload", 7);
	} else if (std::strcmp(kind, "symlink") == 0) {
		write_file(ra.join("target"), "t", 1);
		write_file(rb.join("target"), "t", 1);
		symlink("target", from_a.c_str());
		symlink("target", from_b.c_str());
	} else if (std::strcmp(kind, "dir") == 0) {
		mkdir(from_a.c_str(), 0755);
		mkdir(from_b.c_str(), 0755);
	} else {
		return fail(st, "kind"), false;
	}

	IoCapture cap_r;
	cap_r.begin(STDERR_FILENO);
	int rr = ref_copy(from_a.c_str(), to_a.c_str());
	auto er = cap_r.end(STDERR_FILENO);

	IoCapture cap_p;
	cap_p.begin(STDERR_FILENO);
	int rp = P::copy(from_b.c_str(), to_b.c_str());
	auto ep = cap_p.end(STDERR_FILENO);

	if (rr != rp) {
		fail(st, label);
		return false;
	}
	if (er != ep) {
		fail(st, "stderr");
		return false;
	}
	if (path_exists(from_a) != path_exists(from_b)) {
		fail(st, "source existence");
		return false;
	}
	if (path_exists(to_a) != path_exists(to_b)) {
		fail(st, "dest existence");
		return false;
	}
	(void)label;
	return true;
}

bool
feed_stdin(const char *bytes, std::size_t n)
{
	int fds[2];
	if (pipe(fds) != 0)
		return false;
	write(fds[1], bytes, n);
	close(fds[1]);
	dup2(fds[0], STDIN_FILENO);
	close(fds[0]);
	return true;
}

bool
run_do_move_case(Stat &st, const char *label, int f, int h, int i, int n, int v,
    bool dest_exists, const char *stdin_feed, std::size_t stdin_len)
{
	st.cases++;
	reset_flags();
	set_flags(f, h, i, n, v);

	TempRoot ra, rb;
	if (!ra.ok || !rb.ok)
		return fail(st, "mkdtemp"), false;

	std::string from_a = ra.join("from");
	std::string to_a = ra.join("to");
	std::string from_b = rb.join("from");
	std::string to_b = rb.join("to");

	write_file(from_a, "data\x80\xff", 6);
	write_file(from_b, "data\x80\xff", 6);
	if (dest_exists) {
		write_file(to_a, "old", 3);
		write_file(to_b, "old", 3);
	}

	int saved_in = dup(STDIN_FILENO);
	if (stdin_feed)
		feed_stdin(stdin_feed, stdin_len);

	IoCapture cap_r;
	cap_r.begin(STDERR_FILENO);
	int rr = ref_do_move(from_a.c_str(), to_a.c_str());
	auto er = cap_r.end(STDERR_FILENO);

	IoCapture cap_p;
	cap_p.begin(STDERR_FILENO);
	int rp = P::do_move(from_b.c_str(), to_b.c_str());
	auto ep = cap_p.end(STDERR_FILENO);

	dup2(saved_in, STDIN_FILENO);
	close(saved_in);

	if (rr != rp) {
		fail(st, label);
		return false;
	}
	if (er != ep) {
		fail(st, "stderr");
		return false;
	}
	if (path_exists(from_a) != path_exists(from_b)) {
		fail(st, "source existence");
		return false;
	}
	if (path_exists(to_a) != path_exists(to_b)) {
		fail(st, "dest existence");
		return false;
	}
	if (path_exists(to_a) && path_exists(to_b) && !same_file_state(to_a, to_b)) {
		fail(st, "dest mismatch");
		return false;
	}
	(void)label;
	return true;
}

int
run_main_child(bool use_ref, int argc, char **argv)
{
	if (use_ref)
		return ref_main(argc, argv);
	return P::main(argc, argv);
}

bool
run_main_case(Stat &st, const char *label, int f, int h, int i, int n, int v,
    const std::vector<std::string> &args)
{
	st.cases++;
	reset_flags();
	set_flags(f, h, i, n, v);

	TempRoot ra, rb;
	if (!ra.ok || !rb.ok)
		return fail(st, "mkdtemp"), false;

	std::vector<std::string> paths_a, paths_b;
	for (const auto &arg : args) {
		paths_a.push_back(ra.join(arg.c_str()));
		paths_b.push_back(rb.join(arg.c_str()));
	}

	for (std::size_t idx = 0; idx + 1 < args.size(); ++idx) {
		write_file(paths_a[idx], "mv-main", 7);
		write_file(paths_b[idx], "mv-main", 7);
	}
	if (args.size() >= 2 && args.back() == "dir") {
		mkdir(paths_a.back().c_str(), 0755);
		mkdir(paths_b.back().c_str(), 0755);
	}

	std::vector<char *> av_a, av_b;
	av_a.push_back(const_cast<char *>("mv"));
	av_b.push_back(const_cast<char *>("mv"));
	std::string opt;
	if (f)
		opt += 'f';
	if (h)
		opt += 'h';
	if (i)
		opt += 'i';
	if (n)
		opt += 'n';
	if (v)
		opt += 'v';
	static char obuf[16];
	if (!opt.empty()) {
		obuf[0] = '-';
		std::memcpy(obuf + 1, opt.c_str(), opt.size() + 1);
		av_a.push_back(obuf);
		av_b.push_back(obuf);
	}
	for (auto &p : paths_a)
		av_a.push_back(const_cast<char *>(p.c_str()));
	for (auto &p : paths_b)
		av_b.push_back(const_cast<char *>(p.c_str()));
	av_a.push_back(nullptr);
	av_b.push_back(nullptr);

	pid_t pa = fork();
	if (pa == 0)
		_exit(run_main_child(true, static_cast<int>(av_a.size()) - 1, av_a.data()));
	pid_t pb = fork();
	if (pb == 0)
		_exit(run_main_child(false, static_cast<int>(av_b.size()) - 1, av_b.data()));

	int status_a = 0, status_b = 0;
	waitpid(pa, &status_a, 0);
	waitpid(pb, &status_b, 0);
	int ea = WIFEXITED(status_a) ? WEXITSTATUS(status_a) : -1;
	int eb = WIFEXITED(status_b) ? WEXITSTATUS(status_b) : -1;

	if (ea != eb) {
		fail(st, label);
		return false;
	}
	(void)label;
	return true;
}

bool
run_usage_case(Stat &st)
{
	st.cases++;
	reset_flags();

	pid_t pa = fork();
	if (pa == 0)
		ref_usage();
	pid_t pb = fork();
	if (pb == 0)
		P::usage();

	int status_a = 0, status_b = 0;
	waitpid(pa, &status_a, 0);
	waitpid(pb, &status_b, 0);
	int ea = WIFEXITED(status_a) ? WEXITSTATUS(status_a) : -1;
	int eb = WIFEXITED(status_b) ? WEXITSTATUS(status_b) : -1;

	if (ea != eb) {
		fail(st, "exit status");
		return false;
	}
	return true;
}

void test_preserve_fd_acls_hand()
{
	(void)run_preserve_fd_acls_case(st_preserve_fd_acls, "hand");
}

void test_preserve_fd_acls_sweep()
{
	for (long i = 0; i < SWEEP; ++i)
		(void)run_preserve_fd_acls_case(st_preserve_fd_acls, "sweep");
}

void test_fastcopy_hand()
{
	(void)run_fastcopy_case(st_fastcopy, "empty", 0, 0, 0, 0, 0, {}, false);
	(void)run_fastcopy_case(st_fastcopy, "single", 0, 0, 0, 0, 0, { 'x' }, false);
	(void)run_fastcopy_case(st_fastcopy, "hibyte", 0, 0, 0, 0, 0,
	    { 0x80, 0xff, 0x7f, 0x00 }, false);
	(void)run_fastcopy_case(st_fastcopy, "dest-exists", 0, 0, 0, 0, 0,
	    { 'a', 'b', 'c' }, true);
	std::vector<unsigned char> big(90000, 0xab);
	(void)run_fastcopy_case(st_fastcopy, "large", 0, 0, 0, 0, 1, big, false);
}

void test_fastcopy_sweep()
{
	for (long i = 0; i < SWEEP; ++i) {
		std::vector<unsigned char> data(rng.bits(0, 4096));
		for (auto &c : data)
			c = rng.byte();
		(void)run_fastcopy_case(st_fastcopy, "sweep", rng.coin(), 0, 0,
		    rng.coin(), rng.coin(), data, rng.coin());
	}
}

void test_copy_hand()
{
	(void)run_copy_case(st_copy, "file", "file");
	(void)run_copy_case(st_copy, "symlink", "symlink");
	(void)run_copy_case(st_copy, "dir", "dir");
}

void test_copy_sweep()
{
	const char *kinds[] = { "file", "symlink", "dir" };
	for (long i = 0; i < SWEEP; ++i)
		(void)run_copy_case(st_copy, "sweep", kinds[rng.bits(0, 2)]);
}

void test_do_move_hand()
{
	(void)run_do_move_case(st_do_move, "rename", 0, 0, 0, 0, 0, false, nullptr, 0);
	(void)run_do_move_case(st_do_move, "force", 1, 0, 0, 0, 0, true, nullptr, 0);
	(void)run_do_move_case(st_do_move, "no-clobber", 0, 0, 0, 1, 1, true, nullptr, 0);
	(void)run_do_move_case(st_do_move, "interactive-y", 0, 0, 1, 0, 0, true, "y\n", 2);
	(void)run_do_move_case(st_do_move, "interactive-n", 0, 0, 1, 0, 0, true, "n\n", 2);
}

void test_do_move_sweep()
{
	for (long i = 0; i < SWEEP; ++i) {
		const char *stdin_feed = nullptr;
		std::size_t stdin_len = 0;
		char buf[4];
		if (rng.coin() && rng.coin()) {
			buf[0] = rng.coin() ? 'y' : 'n';
			buf[1] = '\n';
			stdin_feed = buf;
			stdin_len = 2;
		}
		(void)run_do_move_case(st_do_move, "sweep", rng.coin(), rng.coin(),
		    rng.coin(), rng.coin(), rng.coin(), rng.coin(), stdin_feed,
		    stdin_len);
	}
}

void test_main_hand()
{
	(void)run_main_case(st_main, "two-args", 0, 0, 0, 0, 0, { "from", "to" });
	(void)run_main_case(st_main, "to-dir", 0, 0, 0, 0, 0, { "from", "dir" });
	(void)run_main_case(st_main, "force", 1, 0, 0, 0, 0, { "from", "to" });
}

void test_main_sweep()
{
	for (long i = 0; i < SWEEP; ++i) {
		if (rng.coin())
			(void)run_main_case(st_main, "sweep", rng.coin(), 0, 0,
			    rng.coin(), rng.coin(), { "from", "dir" });
		else
			(void)run_main_case(st_main, "sweep", rng.coin(), 0, 0,
			    rng.coin(), rng.coin(), { "from", "to" });
	}
}

void test_usage_hand() { (void)run_usage_case(st_usage); }

void test_usage_sweep()
{
	for (long i = 0; i < SWEEP; ++i)
		(void)run_usage_case(st_usage);
}

} // namespace

int
main()
{
	test_preserve_fd_acls_hand();
	test_fastcopy_hand();
	test_copy_hand();
	test_do_move_hand();
	test_main_hand();
	test_usage_hand();

	test_preserve_fd_acls_sweep();
	test_fastcopy_sweep();
	test_copy_sweep();
	test_do_move_sweep();
	test_main_sweep();
	test_usage_sweep();

	Stat *all[] = {
		&st_preserve_fd_acls, &st_fastcopy, &st_copy, &st_do_move,
		&st_main, &st_usage,
	};
	long total_cases = 0;
	long total_fails = 0;

	std::printf("\n%-22s %12s %12s\n", "function", "cases", "failures");
	for (Stat *st : all) {
		std::printf("%-22s %12ld %12ld\n", st->name, st->cases, st->fails);
		total_cases += st->cases;
		total_fails += st->fails;
	}
	std::printf("%-22s %12ld %12ld\n", "TOTAL", total_cases, total_fails);

	return total_fails == 0 ? 0 : 1;
}
