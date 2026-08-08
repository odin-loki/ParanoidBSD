/*
 * harness.cpp -- differential test for PBSD batch b0172 (rmdir.c).
 */

#define _GNU_SOURCE

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

import pbsd.bin.rmdir.b0172;

namespace P = pbsd::bin_rmdir::b0172;

extern "C" {
int ref_main(int argc, char *argv[]);
int ref_rm_path(char *path);
void ref_usage(void);
extern int ref_pflag;
extern int ref_vflag;
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr std::size_t GUARD_PAD = 64;
constexpr long SWEEP = 200000L;
constexpr int MAX_SHOW = 8;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int shown;
};

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

	int bits(int lo, int hi)
	{
		if (hi <= lo)
			return lo;
		return lo + (int)(next() % (std::uint64_t)(hi - lo + 1));
	}

	bool coin()
	{
		return (next() & 1u) != 0;
	}

	unsigned char byte()
	{
		return (unsigned char)(next() & 0xffu);
	}
};

Rng rng(0xb0172faceULL);

Stat st_rm_path = { "rm_path", 0, 0, 0 };
Stat st_usage = { "usage", 0, 0, 0 };
Stat st_main = { "main", 0, 0, 0 };

bool
fail(Stat &st, const char *what)
{
	st.fails++;
	if (st.shown < MAX_SHOW) {
		st.shown++;
		std::printf("  FAIL %s: %s\n", st.name, what);
	}
	return false;
}

struct GuardBuf {
	std::vector<unsigned char> raw;
	char *ptr;

	explicit GuardBuf(const char *s)
	{
		std::size_t n = s ? std::strlen(s) : 0;
		raw.assign(n + 2 * GUARD_PAD + 1, GUARD);
		ptr = (char *)raw.data() + GUARD_PAD;
		if (n > 0)
			std::memcpy(ptr, s, n);
		ptr[n] = '\0';
	}

	char *
data()
	{
		return ptr;
	}

	const unsigned char *
whole() const
	{
		return raw.data();
	}

	std::size_t
	whole_size() const
	{
		return raw.size();
	}
};

struct IoCapture {
	int saved;
	int pipefd[2];
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

	std::vector<unsigned char>
	end(int fd)
	{
		unsigned char buf[4096];
		ssize_t nr;

		std::fflush(fd == STDOUT_FILENO ? stdout : stderr);
		if (pipefd[1] >= 0)
			close(pipefd[1]);
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
		std::snprintf(path, sizeof(path), "/tmp/pbsd-rmdir-XXXXXX");
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
};

bool
mkdir_p(const char *path)
{
	char buf[1024];
	std::size_t len = std::strlen(path);

	if (len == 0 || len >= sizeof(buf))
		return false;
	std::memcpy(buf, path, len + 1);
	for (char *p = buf + 1; *p != '\0'; p++) {
		if (*p != '/')
			continue;
		*p = '\0';
		if (mkdir(buf, 0755) != 0 && errno != EEXIST)
			return false;
		*p = '/';
	}
	return mkdir(buf, 0755) == 0 || errno == EEXIST;
}

bool
create_tree(const char *root, const char *relpath)
{
	char full[1024];

	if (relpath == nullptr || relpath[0] == '\0')
		return true;
	std::snprintf(full, sizeof(full), "%s", root);
	if (!mkdir_p(full))
		return false;

	const char *p = relpath;
	while (*p == '/')
		p++;
	while (*p != '\0') {
		const char *slash = std::strchr(p, '/');
		std::size_t n = slash ? (std::size_t)(slash - p) : std::strlen(p);
		if (n == 0) {
			p = slash ? slash + 1 : p + std::strlen(p);
			continue;
		}
		if (std::strlen(full) + 1 + n + 1 >= sizeof(full))
			return false;
		std::strcat(full, "/");
		std::strncat(full, p, n);
		if (!mkdir_p(full))
			return false;
		p = slash ? slash + 1 : p + n;
	}
	return true;
}

bool
path_exists(const char *path)
{
	struct stat sb;

	return stat(path, &sb) == 0;
}

bool
remove_leaf(const char *root, const char *relpath)
{
	char full[1024];
	const char *p = relpath;
	const char *last = nullptr;

	while (*p == '/')
		p++;
	for (const char *q = p; *q != '\0'; q++) {
		if (*q == '/')
			last = q;
	}
	if (last == nullptr)
		return true;
	std::snprintf(full, sizeof(full), "%s/%.*s", root, (int)(last - p), p);
	return rmdir(full) == 0 || errno == ENOENT;
}

std::string
abs_path(const char *root, const char *relpath)
{
	std::string s = root;
	const char *p = relpath;
	while (*p == '/')
		p++;
	if (*p != '\0') {
		s += '/';
		s += p;
	}
	return s;
}

struct RmPathResult {
	int ret;
	std::vector<unsigned char> pathbuf;
	std::vector<unsigned char> stdout_bytes;
	std::vector<unsigned char> stderr_bytes;
};

RmPathResult
run_ref_rm_path(const char *path, int vflag, bool remove_leaf_first,
    const char *tree_root, const char *relpath)
{
	RmPathResult res{};
	TempRoot tr;

	if (tree_root != nullptr) {
		if (!create_tree(tree_root, relpath))
			return res;
		if (remove_leaf_first)
			(void)remove_leaf(tree_root, relpath);
	} else if (tr.ok) {
		if (!create_tree(tr.path, path))
			return res;
		if (remove_leaf_first)
			(void)remove_leaf(tr.path, path);
	}

	ref_pflag = 0;
	ref_vflag = vflag;
	GuardBuf gb(path);
	IoCapture out;
	IoCapture err;

	if (!out.begin(STDOUT_FILENO) || !err.begin(STDERR_FILENO))
		return res;
	res.ret = ref_rm_path(gb.data());
	res.stdout_bytes = out.end(STDOUT_FILENO);
	res.stderr_bytes = err.end(STDERR_FILENO);
	res.pathbuf.assign(gb.whole(), gb.whole() + gb.whole_size());
	return res;
}

RmPathResult
run_port_rm_path(const char *path, int vflag, bool remove_leaf_first,
    const char *tree_root, const char *relpath)
{
	RmPathResult res{};
	TempRoot tr;

	if (tree_root != nullptr) {
		if (!create_tree(tree_root, relpath))
			return res;
		if (remove_leaf_first)
			(void)remove_leaf(tree_root, relpath);
	} else if (tr.ok) {
		if (!create_tree(tr.path, path))
			return res;
		if (remove_leaf_first)
			(void)remove_leaf(tr.path, path);
	}

	P::pflag = 0;
	P::vflag = vflag;
	GuardBuf gb(path);
	IoCapture out;
	IoCapture err;

	if (!out.begin(STDOUT_FILENO) || !err.begin(STDERR_FILENO))
		return res;
	res.ret = P::rm_path(gb.data());
	res.stdout_bytes = out.end(STDOUT_FILENO);
	res.stderr_bytes = err.end(STDERR_FILENO);
	res.pathbuf.assign(gb.whole(), gb.whole() + gb.whole_size());
	return res;
}

bool
same_bytes(const std::vector<unsigned char> &a, const std::vector<unsigned char> &b)
{
	return a == b;
}

bool
check_rm_path(const char *label, const char *path, int vflag,
    bool remove_leaf_first, const char *relpath_for_tree = nullptr)
{
	st_rm_path.cases++;
	TempRoot tr1;
	TempRoot tr2;
	const char *rel = relpath_for_tree ? relpath_for_tree : path;

	if (!tr1.ok || !tr2.ok)
		return fail(st_rm_path, "mkdtemp");

	RmPathResult r = run_ref_rm_path(path, vflag, remove_leaf_first, tr1.path, rel);
	RmPathResult p = run_port_rm_path(path, vflag, remove_leaf_first, tr2.path, rel);

	if (r.ret != p.ret) {
		std::printf("  [%s] ret ref=%d port=%d path=%s\n", label, r.ret, p.ret, path);
		return fail(st_rm_path, label);
	}
	if (!same_bytes(r.pathbuf, p.pathbuf)) {
		std::printf("  [%s] pathbuf mismatch path=%s\n", label, path);
		return fail(st_rm_path, label);
	}
	if (!same_bytes(r.stdout_bytes, p.stdout_bytes)) {
		std::printf("  [%s] stdout mismatch path=%s\n", label, path);
		return fail(st_rm_path, label);
	}
	if (!same_bytes(r.stderr_bytes, p.stderr_bytes)) {
		std::printf("  [%s] stderr mismatch path=%s\n", label, path);
		return fail(st_rm_path, label);
	}
	return true;
}

void
add_file_block_rmdir(const char *root, const char *relpath)
{
	char full[1024];
	std::snprintf(full, sizeof(full), "%s/%s", root, relpath);
	(void)mkdir_p(full);
	std::strcat(full, "/blocker");
	int fd = open(full, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd >= 0)
		close(fd);
}

struct ExitRun {
	int status;
	std::vector<unsigned char> stdout_bytes;
	std::vector<unsigned char> stderr_bytes;
};

ExitRun
run_ref_usage()
{
	ExitRun res{};
	int pipe_out[2];
	int pipe_err[2];

	if (pipe(pipe_out) != 0 || pipe(pipe_err) != 0)
		return res;
	pid_t pid = fork();
	if (pid < 0)
		return res;
	if (pid == 0) {
		dup2(pipe_out[1], STDOUT_FILENO);
		dup2(pipe_err[1], STDERR_FILENO);
		close(pipe_out[0]);
		close(pipe_out[1]);
		close(pipe_err[0]);
		close(pipe_err[1]);
		ref_usage();
		_exit(99);
	}
	close(pipe_out[1]);
	close(pipe_err[1]);
	unsigned char buf[4096];
	ssize_t nr;
	while ((nr = read(pipe_out[0], buf, sizeof(buf))) > 0)
		res.stdout_bytes.insert(res.stdout_bytes.end(), buf, buf + nr);
	while ((nr = read(pipe_err[0], buf, sizeof(buf))) > 0)
		res.stderr_bytes.insert(res.stderr_bytes.end(), buf, buf + nr);
	close(pipe_out[0]);
	close(pipe_err[0]);
	int st = 0;
	if (waitpid(pid, &st, 0) >= 0 && WIFEXITED(st))
		res.status = WEXITSTATUS(st);
	return res;
}

ExitRun
run_port_usage()
{
	ExitRun res{};
	int pipe_out[2];
	int pipe_err[2];

	if (pipe(pipe_out) != 0 || pipe(pipe_err) != 0)
		return res;
	pid_t pid = fork();
	if (pid < 0)
		return res;
	if (pid == 0) {
		dup2(pipe_out[1], STDOUT_FILENO);
		dup2(pipe_err[1], STDERR_FILENO);
		close(pipe_out[0]);
		close(pipe_out[1]);
		close(pipe_err[0]);
		close(pipe_err[1]);
		P::usage();
		_exit(99);
	}
	close(pipe_out[1]);
	close(pipe_err[1]);
	unsigned char buf[4096];
	ssize_t nr;
	while ((nr = read(pipe_out[0], buf, sizeof(buf))) > 0)
		res.stdout_bytes.insert(res.stdout_bytes.end(), buf, buf + nr);
	while ((nr = read(pipe_err[0], buf, sizeof(buf))) > 0)
		res.stderr_bytes.insert(res.stderr_bytes.end(), buf, buf + nr);
	close(pipe_out[0]);
	close(pipe_err[0]);
	int st = 0;
	if (waitpid(pid, &st, 0) >= 0 && WIFEXITED(st))
		res.status = WEXITSTATUS(st);
	return res;
}

bool
check_usage(const char *label)
{
	st_usage.cases++;
	ExitRun r = run_ref_usage();
	ExitRun p = run_port_usage();
	if (r.status != p.status) {
		std::printf("  [%s] status ref=%d port=%d\n", label, r.status, p.status);
		return fail(st_usage, label);
	}
	if (!same_bytes(r.stdout_bytes, p.stdout_bytes))
		return fail(st_usage, label);
	if (!same_bytes(r.stderr_bytes, p.stderr_bytes))
		return fail(st_usage, label);
	return true;
}

ExitRun
run_ref_main(int argc, char **argv, const char *tree_root,
    const std::vector<std::string> &rel_dirs)
{
	ExitRun res{};
	int pipe_out[2];
	int pipe_err[2];

	for (const auto &rel : rel_dirs) {
		if (!create_tree(tree_root, rel.c_str()))
			return res;
	}

	if (pipe(pipe_out) != 0 || pipe(pipe_err) != 0)
		return res;
	pid_t pid = fork();
	if (pid < 0)
		return res;
	if (pid == 0) {
		optind = 1;
		opterr = 1;
		dup2(pipe_out[1], STDOUT_FILENO);
		dup2(pipe_err[1], STDERR_FILENO);
		close(pipe_out[0]);
		close(pipe_out[1]);
		close(pipe_err[0]);
		close(pipe_err[1]);
		ref_main(argc, argv);
		_exit(99);
	}
	close(pipe_out[1]);
	close(pipe_err[1]);
	unsigned char buf[4096];
	ssize_t nr;
	while ((nr = read(pipe_out[0], buf, sizeof(buf))) > 0)
		res.stdout_bytes.insert(res.stdout_bytes.end(), buf, buf + nr);
	while ((nr = read(pipe_err[0], buf, sizeof(buf))) > 0)
		res.stderr_bytes.insert(res.stderr_bytes.end(), buf, buf + nr);
	close(pipe_out[0]);
	close(pipe_err[0]);
	int st = 0;
	if (waitpid(pid, &st, 0) >= 0 && WIFEXITED(st))
		res.status = WEXITSTATUS(st);
	return res;
}

ExitRun
run_port_main(int argc, char **argv, const char *tree_root,
    const std::vector<std::string> &rel_dirs)
{
	ExitRun res{};
	int pipe_out[2];
	int pipe_err[2];

	for (const auto &rel : rel_dirs) {
		if (!create_tree(tree_root, rel.c_str()))
			return res;
	}

	if (pipe(pipe_out) != 0 || pipe(pipe_err) != 0)
		return res;
	pid_t pid = fork();
	if (pid < 0)
		return res;
	if (pid == 0) {
		optind = 1;
		opterr = 1;
		dup2(pipe_out[1], STDOUT_FILENO);
		dup2(pipe_err[1], STDERR_FILENO);
		close(pipe_out[0]);
		close(pipe_out[1]);
		close(pipe_err[0]);
		close(pipe_err[1]);
		P::main(argc, argv);
		_exit(99);
	}
	close(pipe_out[1]);
	close(pipe_err[1]);
	unsigned char buf[4096];
	ssize_t nr;
	while ((nr = read(pipe_out[0], buf, sizeof(buf))) > 0)
		res.stdout_bytes.insert(res.stdout_bytes.end(), buf, buf + nr);
	while ((nr = read(pipe_err[0], buf, sizeof(buf))) > 0)
		res.stderr_bytes.insert(res.stderr_bytes.end(), buf, buf + nr);
	close(pipe_out[0]);
	close(pipe_err[0]);
	int st = 0;
	if (waitpid(pid, &st, 0) >= 0 && WIFEXITED(st))
		res.status = WEXITSTATUS(st);
	return res;
}

bool
check_main(const char *label, int argc, char **argv,
    const std::vector<std::string> &rel_dirs)
{
	st_main.cases++;
	TempRoot tr1;
	TempRoot tr2;
	if (!tr1.ok || !tr2.ok)
		return fail(st_main, "mkdtemp");

	ExitRun r = run_ref_main(argc, argv, tr1.path, rel_dirs);
	ExitRun p = run_port_main(argc, argv, tr2.path, rel_dirs);
	if (r.status != p.status) {
		std::printf("  [%s] status ref=%d port=%d\n", label, r.status, p.status);
		return fail(st_main, label);
	}
	if (!same_bytes(r.stdout_bytes, p.stdout_bytes))
		return fail(st_main, label);
	if (!same_bytes(r.stderr_bytes, p.stderr_bytes))
		return fail(st_main, label);
	return true;
}

std::string
rand_component(Rng &r, int maxlen)
{
	int len = r.bits(0, maxlen);
	if (len == 0)
		return "";
	std::string s;
	s.reserve((std::size_t)len);
	for (int i = 0; i < len; i++) {
		unsigned char c = r.byte();
		if (c == '/' || c == '\0')
			c = 'a' + (r.bits(0, 25));
		s.push_back((char)c);
	}
	return s;
}

std::string
rand_relpath(Rng &r, int depth, int maxlen)
{
	int d = r.bits(0, depth);
	if (d == 0)
		return rand_component(r, maxlen);
	std::string s;
	for (int i = 0; i < d; i++) {
		if (i > 0)
			s += '/';
		s += rand_component(r, maxlen);
	}
	int slashes = r.bits(0, 4);
	for (int i = 0; i < slashes; i++)
		s += '/';
	return s;
}

void
test_rm_path_edge()
{
	(void)check_rm_path("empty", "", 0, false);
	(void)check_rm_path("root", "/", 0, false);
	(void)check_rm_path("root_slashes", "///", 0, false);
	(void)check_rm_path("single", "a", 0, false);
	(void)check_rm_path("single_slash", "a/", 0, false);
	(void)check_rm_path("single_many_slash", "a////", 0, false);
	(void)check_rm_path("two_no_leaf", "a/b", 0, false);
	(void)check_rm_path("abs_two", "/a/b", 0, false);
	(void)check_rm_path("leaf_removed", "a/b/c", 0, true);
	(void)check_rm_path("leaf_removed_v", "a/b/c", 1, true);
	(void)check_rm_path("deep", "a/b/c/d/e", 0, true);
	(void)check_rm_path("deep_v", "a/b/c/d/e", 1, true);
	(void)check_rm_path("trail_many", "x/y/z////", 0, true);
	(void)check_rm_path("highbit", "a/\x80/b", 0, true);
	(void)check_rm_path("highbit_v", "\xff/\xfe/", 1, true);

	TempRoot tr;
	if (tr.ok) {
		add_file_block_rmdir(tr.path, "p/q");
		(void)remove_leaf(tr.path, "p/q/r");
		st_rm_path.cases++;
		TempRoot tr2;
		if (tr2.ok) {
			add_file_block_rmdir(tr.path, "p/q");
			(void)remove_leaf(tr.path, "p/q/r");
			create_tree(tr2.path, "p/q");
			(void)remove_leaf(tr2.path, "p/q/r");
			add_file_block_rmdir(tr2.path, "p/q");
			GuardBuf gb("p/q/r");
			GuardBuf gb2("p/q/r");
			ref_vflag = 0;
			P::vflag = 0;
			IoCapture o1, e1, o2, e2;
			(void)o1.begin(STDOUT_FILENO);
			(void)e1.begin(STDERR_FILENO);
			int rr = ref_rm_path(gb.data());
			auto ro = o1.end(STDOUT_FILENO);
			auto re = e1.end(STDERR_FILENO);
			(void)o2.begin(STDOUT_FILENO);
			(void)e2.begin(STDERR_FILENO);
			int pr = P::rm_path(gb2.data());
			auto po = o2.end(STDOUT_FILENO);
			auto pe = e2.end(STDERR_FILENO);
			if (rr != pr || gb.raw != gb2.raw || ro != po || re != pe)
				(void)fail(st_rm_path, "rmdir_fail");
		}
	}
}

void
test_rm_path_random()
{
	for (long i = 0; i < SWEEP; i++) {
		std::string rel = rand_relpath(rng, 6, 12);
		int v = rng.coin() ? 1 : 0;
		bool leaf = rng.coin();
		char label[64];
		std::snprintf(label, sizeof(label), "rand%ld", i);
		(void)check_rm_path(label, rel.c_str(), v, leaf);
	}
}

void
test_usage_edge()
{
	(void)check_usage("usage");
}

void
test_usage_random()
{
	for (long i = 0; i < SWEEP; i++)
		(void)check_usage("usage_rand");
}

bool
run_main_case(const char *label, const std::vector<const char *> &flags,
    const std::vector<std::string> &rel_dirs, bool create_dirs)
{
	st_main.cases++;
	TempRoot tr1;
	TempRoot tr2;
	if (!tr1.ok || !tr2.ok)
		return fail(st_main, "mkdtemp");

	std::vector<std::string> abs1;
	std::vector<std::string> abs2;
	for (const auto &rel : rel_dirs) {
		abs1.push_back(abs_path(tr1.path, rel.c_str()));
		abs2.push_back(abs_path(tr2.path, rel.c_str()));
	}

	std::vector<char *> argv1;
	std::vector<char *> argv2;
	argv1.push_back(const_cast<char *>("rmdir"));
	argv2.push_back(const_cast<char *>("rmdir"));
	for (const char *f : flags) {
		argv1.push_back(const_cast<char *>(f));
		argv2.push_back(const_cast<char *>(f));
	}
	for (auto &s : abs1)
		argv1.push_back(const_cast<char *>(s.c_str()));
	for (auto &s : abs2)
		argv2.push_back(const_cast<char *>(s.c_str()));
	argv1.push_back(nullptr);
	argv2.push_back(nullptr);

	std::vector<std::string> rels;
	if (create_dirs)
		rels = rel_dirs;

	ExitRun r = run_ref_main((int)argv1.size() - 1, argv1.data(), tr1.path, rels);
	std::vector<std::string> rels2;
	if (create_dirs)
		rels2 = rel_dirs;
	ExitRun p = run_port_main((int)argv2.size() - 1, argv2.data(), tr2.path, rels2);

	if (r.status != p.status) {
		std::printf("  [%s] status ref=%d port=%d\n", label, r.status, p.status);
		return fail(st_main, label);
	}
	if (!same_bytes(r.stdout_bytes, p.stdout_bytes))
		return fail(st_main, label);
	if (!same_bytes(r.stderr_bytes, p.stderr_bytes))
		return fail(st_main, label);
	return true;
}

void
test_main_edge()
{
	{
		char *argv[] = { (char *)"rmdir", nullptr };
		st_main.cases++;
		TempRoot t1, t2;
		ExitRun r = run_ref_main(1, argv, t1.path, {});
		ExitRun p = run_port_main(1, argv, t2.path, {});
		if (r.status != p.status || r.stdout_bytes != p.stdout_bytes ||
		    r.stderr_bytes != p.stderr_bytes)
			(void)fail(st_main, "no_args");
	}
	{
		char *argv[] = { (char *)"rmdir", (char *)"-z", nullptr };
		st_main.cases++;
		TempRoot t1, t2;
		ExitRun r = run_ref_main(2, argv, t1.path, {});
		ExitRun p = run_port_main(2, argv, t2.path, {});
		if (r.status != p.status || r.stdout_bytes != p.stdout_bytes ||
		    r.stderr_bytes != p.stderr_bytes)
			(void)fail(st_main, "bad_opt");
	}
	(void)run_main_case("single_ok", {}, { "d" }, true);
	(void)run_main_case("single_ok_v", { "-v" }, { "d" }, true);
	(void)run_main_case("single_ok_p", { "-p" }, { "a/b" }, true);
	(void)run_main_case("single_ok_pv", { "-p", "-v" }, { "a/b/c" }, true);
	(void)run_main_case("missing", {}, { "nope" }, false);
	(void)run_main_case("two_ok", {}, { "x", "y" }, true);
	(void)run_main_case("mix", { "-v" }, { "good", "bad" }, true);
}

void
test_main_random()
{
	for (long i = 0; i < SWEEP; i++) {
		std::vector<const char *> flags;
		if (rng.coin())
			flags.push_back("-p");
		if (rng.coin())
			flags.push_back("-v");
		int nd = rng.bits(1, 3);
		std::vector<std::string> rels;
		for (int j = 0; j < nd; j++)
			rels.push_back(rand_relpath(rng, 4, 8));
		bool create = rng.coin();
		char label[64];
		std::snprintf(label, sizeof(label), "main%ld", i);
		(void)run_main_case(label, flags, rels, create);
	}
}

} // namespace

int
main()
{
	test_rm_path_edge();
	test_usage_edge();
	test_main_edge();
	test_rm_path_random();
	test_usage_random();
	test_main_random();

	Stat *all[] = { &st_rm_path, &st_usage, &st_main };
	long total_cases = 0;
	long total_fails = 0;

	std::printf("\n%-10s %12s %12s\n", "function", "cases", "failures");
	for (Stat *st : all) {
		std::printf("%-10s %12ld %12ld\n", st->name, st->cases, st->fails);
		total_cases += st->cases;
		total_fails += st->fails;
	}
	std::printf("%-10s %12ld %12ld\n", "TOTAL", total_cases, total_fails);

	return total_fails == 0 ? 0 : 1;
}
