/*
 * harness.cpp -- differential test for PBSD batch b0178 (mkdir.c).
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

import pbsd.bin.mkdir.b0178;

namespace P = pbsd::bin_mkdir::b0178;

extern "C" {
int ref_main(int argc, char *argv[]);
int ref_build(char *path, mode_t omode);
void ref_usage(void);
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

Rng rng(0xb0178faceULL);

Stat st_build = { "build", 0, 0, 0 };
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
		std::snprintf(path, sizeof(path), "/tmp/pbsd-mkdir-XXXXXX");
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
mkdir_p(const char *path, mode_t mode = 0755)
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
		if (mkdir(buf, mode) != 0 && errno != EEXIST)
			return false;
		*p = '/';
	}
	return mkdir(buf, mode) == 0 || errno == EEXIST;
}

bool
create_tree(const char *root, const char *relpath, mode_t mode = 0755)
{
	char full[1024];

	if (relpath == nullptr || relpath[0] == '\0')
		return true;
	std::snprintf(full, sizeof(full), "%s", root);
	if (!mkdir_p(full, mode))
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
		if (!mkdir_p(full, mode))
			return false;
		p = slash ? slash + 1 : p + n;
	}
	return true;
}

bool
create_partial_tree(const char *root, const char *relpath)
{
	const char *slash = std::strchr(relpath, '/');
	if (slash == nullptr)
		return true;
	char parent[1024];
	std::snprintf(parent, sizeof(parent), "%.*s", (int)(slash - relpath),
	    relpath);
	if (parent[0] == '\0')
		return true;
	return create_tree(root, parent);
}

void
add_file_block(const char *root, const char *relpath)
{
	char full[1024];
	std::snprintf(full, sizeof(full), "%s/%s", root, relpath);
	char *slash = std::strrchr(full, '/');
	if (slash != nullptr) {
		*slash = '\0';
		(void)mkdir_p(full);
		*slash = '/';
	}
	int fd = open(full, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd >= 0)
		close(fd);
}

std::string
abs_path(const char *root, const char *relpath)
{
	if (relpath == nullptr)
		return root;
	if (relpath[0] == '\0')
		return root;
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

enum class Prep {
	NONE,
	FULL,
	PARTIAL,
	FILE_BLOCK,
};

struct BuildResult {
	int ret;
	std::vector<unsigned char> pathbuf;
	std::vector<unsigned char> stdout_bytes;
	std::vector<unsigned char> stderr_bytes;
};

BuildResult
run_ref_build(const char *fullpath, mode_t omode, int vflag, Prep prep,
    const char *root, const char *relpath)
{
	BuildResult res{};

	switch (prep) {
	case Prep::FULL:
		if (!create_tree(root, relpath))
			return res;
		break;
	case Prep::PARTIAL:
		if (!create_partial_tree(root, relpath))
			return res;
		break;
	case Prep::FILE_BLOCK:
		add_file_block(root, relpath);
		break;
	default:
		break;
	}

	ref_vflag = vflag;
	GuardBuf gb(fullpath);
	IoCapture out;
	IoCapture err;

	if (!out.begin(STDOUT_FILENO) || !err.begin(STDERR_FILENO))
		return res;
	res.ret = ref_build(gb.data(), omode);
	res.stdout_bytes = out.end(STDOUT_FILENO);
	res.stderr_bytes = err.end(STDERR_FILENO);
	res.pathbuf.assign(gb.whole(), gb.whole() + gb.whole_size());
	return res;
}

BuildResult
run_port_build(const char *fullpath, mode_t omode, int vflag, Prep prep,
    const char *root, const char *relpath)
{
	BuildResult res{};

	switch (prep) {
	case Prep::FULL:
		if (!create_tree(root, relpath))
			return res;
		break;
	case Prep::PARTIAL:
		if (!create_partial_tree(root, relpath))
			return res;
		break;
	case Prep::FILE_BLOCK:
		add_file_block(root, relpath);
		break;
	default:
		break;
	}

	P::vflag = vflag;
	GuardBuf gb(fullpath);
	IoCapture out;
	IoCapture err;

	if (!out.begin(STDOUT_FILENO) || !err.begin(STDERR_FILENO))
		return res;
	res.ret = P::build(gb.data(), omode);
	res.stdout_bytes = out.end(STDOUT_FILENO);
	res.stderr_bytes = err.end(STDERR_FILENO);
	res.pathbuf.assign(gb.whole(), gb.whole() + gb.whole_size());
	return res;
}

bool
same_bytes(const std::vector<unsigned char> &a,
    const std::vector<unsigned char> &b)
{
	return a == b;
}

bool
check_build(const char *label, const char *relpath, mode_t omode, int vflag,
    Prep prep)
{
	st_build.cases++;
	TempRoot tr1;
	TempRoot tr2;
	if (!tr1.ok || !tr2.ok)
		return fail(st_build, "mkdtemp");

	std::string full1 = abs_path(tr1.path, relpath);
	std::string full2 = abs_path(tr2.path, relpath);

	BuildResult r = run_ref_build(full1.c_str(), omode, vflag, prep, tr1.path,
	    relpath);
	BuildResult p = run_port_build(full2.c_str(), omode, vflag, prep, tr2.path,
	    relpath);

	if (r.ret != p.ret) {
		std::printf("  [%s] ret ref=%d port=%d path=%s\n", label, r.ret,
		    p.ret, relpath);
		return fail(st_build, label);
	}
	if (!same_bytes(r.pathbuf, p.pathbuf)) {
		std::printf("  [%s] pathbuf mismatch path=%s\n", label, relpath);
		return fail(st_build, label);
	}
	if (!same_bytes(r.stdout_bytes, p.stdout_bytes)) {
		std::printf("  [%s] stdout mismatch path=%s\n", label, relpath);
		return fail(st_build, label);
	}
	if (!same_bytes(r.stderr_bytes, p.stderr_bytes)) {
		std::printf("  [%s] stderr mismatch path=%s\n", label, relpath);
		return fail(st_build, label);
	}
	return true;
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
		std::printf("  [%s] status ref=%d port=%d\n", label, r.status,
		    p.status);
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
    const std::vector<std::string> &prep_rels, Prep prep)
{
	ExitRun res{};
	int pipe_out[2];
	int pipe_err[2];

	for (const auto &rel : prep_rels) {
		switch (prep) {
		case Prep::FULL:
			if (!create_tree(tree_root, rel.c_str()))
				return res;
			break;
		case Prep::PARTIAL:
			if (!create_partial_tree(tree_root, rel.c_str()))
				return res;
			break;
		case Prep::FILE_BLOCK:
			add_file_block(tree_root, rel.c_str());
			break;
		default:
			break;
		}
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
    const std::vector<std::string> &prep_rels, Prep prep)
{
	ExitRun res{};
	int pipe_out[2];
	int pipe_err[2];

	for (const auto &rel : prep_rels) {
		switch (prep) {
		case Prep::FULL:
			if (!create_tree(tree_root, rel.c_str()))
				return res;
			break;
		case Prep::PARTIAL:
			if (!create_partial_tree(tree_root, rel.c_str()))
				return res;
			break;
		case Prep::FILE_BLOCK:
			add_file_block(tree_root, rel.c_str());
			break;
		default:
			break;
		}
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
run_main_case(const char *label, const std::vector<const char *> &flags,
    const std::vector<std::string> &rel_dirs, Prep prep)
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
	argv1.push_back(const_cast<char *>("mkdir"));
	argv2.push_back(const_cast<char *>("mkdir"));
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

	ExitRun r = run_ref_main((int)argv1.size() - 1, argv1.data(), tr1.path,
	    rel_dirs, prep);
	ExitRun p = run_port_main((int)argv2.size() - 1, argv2.data(), tr2.path,
	    rel_dirs, prep);

	if (r.status != p.status) {
		std::printf("  [%s] status ref=%d port=%d\n", label, r.status,
		    p.status);
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

mode_t
rand_mode(Rng &r)
{
	static const mode_t modes[] = {
		(mode_t)0000,
		(mode_t)0100,
		(mode_t)0200,
		(mode_t)0400,
		(mode_t)0700,
		(mode_t)0755,
		(mode_t)0777,
		(mode_t)02755,
	};
	return modes[r.bits(0, (int)(sizeof(modes) / sizeof(modes[0])) - 1)];
}

const char *
rand_mode_str(Rng &r)
{
	static const char *modes[] = {
		"755",
		"0755",
		"700",
		"777",
		"644",
		"u+rwx,go+rx",
		"go-w",
		"u=rwX,g=rX,o=rX",
	};
	return modes[r.bits(0, (int)(sizeof(modes) / sizeof(modes[0])) - 1)];
}

void
test_build_edge()
{
	(void)check_build("empty", "", 0755, 0, Prep::NONE);
	(void)check_build("single", "a", 0755, 0, Prep::NONE);
	(void)check_build("single_v", "a", 0755, 1, Prep::NONE);
	(void)check_build("two", "a/b", 0755, 0, Prep::NONE);
	(void)check_build("two_v", "a/b", 0700, 1, Prep::NONE);
	(void)check_build("deep", "a/b/c/d/e", 0755, 0, Prep::NONE);
	(void)check_build("deep_v", "a/b/c/d/e", 0777, 1, Prep::NONE);
	(void)check_build("trail_slash", "a/b/", 0755, 0, Prep::NONE);
	(void)check_build("trail_many", "x/y/z////", 0755, 0, Prep::NONE);
	(void)check_build("dbl_slash", "a//b", 0755, 0, Prep::NONE);
	(void)check_build("exists", "a/b/c", 0755, 0, Prep::FULL);
	(void)check_build("exists_v", "a/b/c", 0755, 1, Prep::FULL);
	(void)check_build("partial", "a/b/c", 0755, 0, Prep::PARTIAL);
	(void)check_build("partial_v", "a/b/c/d", 0755, 1, Prep::PARTIAL);
	(void)check_build("mode_700", "m/n", 0700, 0, Prep::NONE);
	(void)check_build("mode_777", "p/q", 0777, 0, Prep::NONE);
	(void)check_build("mode_0", "z", (mode_t)0, 0, Prep::NONE);
	(void)check_build("highbit", "a/\x80/b", 0755, 0, Prep::NONE);
	(void)check_build("highbit_v", "\xff/\xfe", 0755, 1, Prep::NONE);
	(void)check_build("file_block", "p/q/r", 0755, 0, Prep::FILE_BLOCK);
	(void)check_build("file_block_mid", "a/b/c", 0755, 0, Prep::FILE_BLOCK);

	TempRoot tr;
	if (tr.ok) {
		add_file_block(tr.path, "blk");
		st_build.cases++;
		TempRoot tr2;
		if (tr2.ok) {
			add_file_block(tr2.path, "blk");
			std::string full1 = abs_path(tr.path, "blk/x");
			std::string full2 = abs_path(tr2.path, "blk/x");
			GuardBuf gb(full1.c_str());
			GuardBuf gb2(full2.c_str());
			ref_vflag = 0;
			P::vflag = 0;
			IoCapture o1, e1, o2, e2;
			(void)o1.begin(STDOUT_FILENO);
			(void)e1.begin(STDERR_FILENO);
			int rr = ref_build(gb.data(), 0755);
			auto ro = o1.end(STDOUT_FILENO);
			auto re = e1.end(STDERR_FILENO);
			(void)o2.begin(STDOUT_FILENO);
			(void)e2.begin(STDERR_FILENO);
			int pr = P::build(gb2.data(), 0755);
			auto po = o2.end(STDOUT_FILENO);
			auto pe = e2.end(STDERR_FILENO);
			if (rr != pr || gb.raw != gb2.raw || ro != po || re != pe)
				(void)fail(st_build, "blocker");
		}
	}
}

void
test_build_random()
{
	for (long i = 0; i < SWEEP; i++) {
		std::string rel = rand_relpath(rng, 6, 12);
		mode_t omode = rand_mode(rng);
		int v = rng.coin() ? 1 : 0;
		Prep prep = Prep::NONE;
		switch (rng.bits(0, 3)) {
		case 1:
			prep = Prep::FULL;
			break;
		case 2:
			prep = Prep::PARTIAL;
			break;
		case 3:
			prep = Prep::FILE_BLOCK;
			break;
		default:
			break;
		}
		char label[64];
		std::snprintf(label, sizeof(label), "rand%ld", i);
		(void)check_build(label, rel.c_str(), omode, v, prep);
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

void
test_main_edge()
{
	{
		char *argv[] = { (char *)"mkdir", nullptr };
		st_main.cases++;
		TempRoot t1, t2;
		ExitRun r = run_ref_main(1, argv, t1.path, {}, Prep::NONE);
		ExitRun p = run_port_main(1, argv, t2.path, {}, Prep::NONE);
		if (r.status != p.status || r.stdout_bytes != p.stdout_bytes ||
		    r.stderr_bytes != p.stderr_bytes)
			(void)fail(st_main, "no_args");
	}
	{
		char *argv[] = { (char *)"mkdir", (char *)"-z", nullptr };
		st_main.cases++;
		TempRoot t1, t2;
		ExitRun r = run_ref_main(2, argv, t1.path, {}, Prep::NONE);
		ExitRun p = run_port_main(2, argv, t2.path, {}, Prep::NONE);
		if (r.status != p.status || r.stdout_bytes != p.stdout_bytes ||
		    r.stderr_bytes != p.stderr_bytes)
			(void)fail(st_main, "bad_opt");
	}
	{
		char *argv[] = { (char *)"mkdir", (char *)"-m", (char *)"bogus!!!",
		    (char *)"d", nullptr };
		st_main.cases++;
		TempRoot t1, t2;
		ExitRun r = run_ref_main(4, argv, t1.path, { "d" }, Prep::NONE);
		ExitRun p = run_port_main(4, argv, t2.path, { "d" }, Prep::NONE);
		if (r.status != p.status || r.stdout_bytes != p.stdout_bytes ||
		    r.stderr_bytes != p.stderr_bytes)
			(void)fail(st_main, "bad_mode");
	}
	(void)run_main_case("single_ok", {}, { "d" }, Prep::FULL);
	(void)run_main_case("single_ok_v", { "-v" }, { "d" }, Prep::FULL);
	(void)run_main_case("single_new", {}, { "newdir" }, Prep::NONE);
	(void)run_main_case("single_new_v", { "-v" }, { "nd" }, Prep::NONE);
	(void)run_main_case("nested_fail", {}, { "a/b/c" }, Prep::NONE);
	(void)run_main_case("nested_p", { "-p" }, { "a/b/c" }, Prep::NONE);
	(void)run_main_case("nested_pv", { "-p", "-v" }, { "a/b/c" }, Prep::NONE);
	(void)run_main_case("exists_p", { "-p" }, { "x/y" }, Prep::FULL);
	(void)run_main_case("exists_pv", { "-p", "-v" }, { "x/y" }, Prep::FULL);
	(void)run_main_case("mode_755", { "-m", "755" }, { "m755" }, Prep::NONE);
	(void)run_main_case("mode_pv", { "-p", "-v", "-m", "700" }, { "a/b" },
	    Prep::NONE);
	(void)run_main_case("two_ok", {}, { "t1", "t2" }, Prep::FULL);
	(void)run_main_case("two_p", { "-p" }, { "u/v", "w/x" }, Prep::NONE);
	(void)run_main_case("file_block_p", { "-p" }, { "blk/s" }, Prep::FILE_BLOCK);
	(void)run_main_case("highbit_p", { "-p" }, { "a/\x80/b" }, Prep::NONE);
}

void
test_main_random()
{
	for (long i = 0; i < SWEEP; i++) {
		std::vector<const char *> flags;
		bool use_p = rng.coin();
		bool use_v = rng.coin();
		bool use_m = rng.coin();
		if (use_p)
			flags.push_back("-p");
		if (use_v)
			flags.push_back("-v");
		if (use_m) {
			flags.push_back("-m");
			flags.push_back(rand_mode_str(rng));
		}
		int nd = rng.bits(1, 3);
		std::vector<std::string> rels;
		for (int j = 0; j < nd; j++)
			rels.push_back(rand_relpath(rng, 4, 8));
		Prep prep = Prep::NONE;
		switch (rng.bits(0, 3)) {
		case 1:
			prep = Prep::FULL;
			break;
		case 2:
			prep = Prep::PARTIAL;
			break;
		case 3:
			prep = Prep::FILE_BLOCK;
			break;
		default:
			break;
		}
		if (!use_p && prep == Prep::NONE && rng.coin())
			prep = Prep::FULL;
		char label[64];
		std::snprintf(label, sizeof(label), "main%ld", i);
		(void)run_main_case(label, flags, rels, prep);
	}
}

} // namespace

int
main()
{
	test_build_edge();
	test_usage_edge();
	test_main_edge();
	test_build_random();
	test_usage_random();
	test_main_random();

	Stat *all[] = { &st_build, &st_usage, &st_main };
	long total_cases = 0;
	long total_fails = 0;

	std::printf("\n%-10s %12s %12s\n", "function", "cases", "failures");
	for (Stat *st : all) {
		std::printf("%-10s %12ld %12ld\n", st->name, st->cases,
		    st->fails);
		total_cases += st->cases;
		total_fails += st->fails;
	}
	std::printf("%-10s %12ld %12ld\n", "TOTAL", total_cases, total_fails);

	return total_fails == 0 ? 0 : 1;
}
