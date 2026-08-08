/*
 * harness.cpp -- differential test for PBSD batch b0195 (chmod.c).
 */

#define _GNU_SOURCE

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <fts.h>
#include <memory>
#include <signal.h>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

import pbsd.bin.chmod.b0195;

namespace P = pbsd::bin_chmod::b0195;

extern "C" {
int ref_main(int argc, char *argv[]);
void ref_siginfo_handler(int sig);
void ref_usage(void);
int ref_may_have_nfs4acl(const FTSENT *ent, int hflag);
extern volatile sig_atomic_t ref_siginfo;
void __real_exit(int status);
long __real_pathconf(const char *path, int name);
long __real_lpathconf(const char *path, int name);
}

#ifndef _PC_ACL_NFS4
#define _PC_ACL_NFS4 64
#endif

#ifndef SIGINFO
#define SIGINFO SIGUSR1
#endif

#define SWEEP 200000L
#define MAX_SHOW 8

namespace {

bool g_test_child = false;
bool g_nfs4_direct = false;

struct PcCfg {
	long ret;
	int err;
};

std::unordered_map<std::string, mode_t> g_modes;
std::unordered_set<std::string> g_fail_paths;
std::unordered_map<std::string, PcCfg> g_pathconf;
std::unordered_map<std::string, PcCfg> g_lpathconf;

extern "C" void
__wrap_exit(int status)
{
	if (g_test_child) {
		::_exit(status);
	}
	__real_exit(status);
}

extern "C" int
__wrap_fchmodat(int fd, const char *path, mode_t mode, int atflag)
{
	(void)fd;
	(void)atflag;

	if (!g_test_child) {
		errno = ENOSYS;
		return (-1);
	}
	if (g_fail_paths.count(path) != 0) {
		errno = EIO;
		return (-1);
	}
	g_modes[std::string(path)] = mode;
	return (0);
}

extern "C" long
__wrap_pathconf(const char *path, int name)
{
	if (g_test_child && (g_nfs4_direct || name == _PC_ACL_NFS4)) {
		auto it = g_pathconf.find(path);
		if (it != g_pathconf.end()) {
			if (it->second.ret < 0) {
				errno = it->second.err;
			}
			return (it->second.ret);
		}
		return (0);
	}
	return (__real_pathconf(path, name));
}

extern "C" long
__wrap_lpathconf(const char *path, int name)
{
	if (g_test_child && (g_nfs4_direct || name == _PC_ACL_NFS4)) {
		auto it = g_lpathconf.find(path);
		if (it != g_lpathconf.end()) {
			if (it->second.ret < 0) {
				errno = it->second.err;
			}
			return (it->second.ret);
		}
		return (0);
	}
	return (__real_lpathconf(path, name));
}

static mode_t
lookup_mode(const std::string &path)
{
	auto it = g_modes.find(path);
	if (it != g_modes.end()) {
		return (it->second);
	}
	return (0644);
}

struct FtsNode {
	FTSENT ent;
	struct stat statbuf;
	std::string accpath;
	std::string path;
	std::vector<char> namebuf;
};

struct FtsState {
	FTS pub;
	std::vector<std::unique_ptr<FtsNode>> nodes;
	size_t idx;
	int options;
	bool skip_pending;
	int skip_level;
};

static FtsState *
fts_state(FTS *ftsp)
{
	return (reinterpret_cast<FtsState *>(ftsp));
}

static FtsNode *
make_node(int info, int level, int fts_errno, const std::string &accpath,
    const std::string &path, const char *name, mode_t base_mode)
{
	auto node = std::make_unique<FtsNode>();
	node->accpath = accpath;
	node->path = path;
	node->namebuf.assign(name, name + strlen(name) + 1);

	std::memset(&node->statbuf, 0, sizeof(node->statbuf));
	node->statbuf.st_mode = base_mode;
	node->statbuf.st_dev = 42;
	node->statbuf.st_ino = 1;

	std::memset(&node->ent, 0, sizeof(node->ent));
	node->ent.fts_info = (unsigned short)info;
	node->ent.fts_level = (short)level;
	node->ent.fts_errno = fts_errno;
	node->ent.fts_accpath = const_cast<char *>(node->accpath.c_str());
	node->ent.fts_path = const_cast<char *>(node->path.c_str());
	node->ent.fts_namelen = (unsigned short)(strlen(name));
	node->ent.fts_statp = &node->statbuf;
	node->ent.fts_name = node->namebuf.data();
	return (node.release());
}

static int
path_stat(const char *path, struct stat *sb, int options, int level)
{
	if ((options & FTS_LOGICAL) ||
	    ((options & FTS_COMFOLLOW) && level == FTS_ROOTLEVEL)) {
		return (stat(path, sb));
	}
	return (lstat(path, sb));
}

static void
walk_tree(FtsState *st, const std::string &accpath, const std::string &tpath,
    int level, dev_t root_dev)
{
	struct stat sb;

	(void)root_dev;
	if (path_stat(accpath.c_str(), &sb, st->options, level) != 0) {
		const char *base = accpath.c_str();
		const char *slash = strrchr(base, '/');
		const char *name = slash != nullptr ? slash + 1 : base;
		st->nodes.emplace_back(make_node(FTS_NS, level, errno, accpath,
		    tpath, name, 0644));
		return;
	}

	const char *base = accpath.c_str();
	const char *slash = strrchr(base, '/');
	const char *name = slash != nullptr ? slash + 1 : base;
	mode_t m = lookup_mode(accpath);
	if (m != 0644 || sb.st_mode != 0) {
		m = (sb.st_mode & S_IFMT) | (m & ALLPERMS);
	} else {
		m = sb.st_mode;
	}

	if (S_ISDIR(sb.st_mode)) {
		st->nodes.emplace_back(
		    make_node(FTS_D, level, 0, accpath, tpath, name, m));
		if ((sb.st_mode & S_IXUSR) == 0) {
			st->nodes.emplace_back(make_node(FTS_DNR, level, EACCES,
			    accpath, tpath, name, m));
			st->nodes.emplace_back(
			    make_node(FTS_DP, level, 0, accpath, tpath, name, m));
			return;
		}
		DIR *dp = opendir(accpath.c_str());
		if (dp == nullptr) {
			st->nodes.emplace_back(make_node(FTS_DNR, level,
			    errno, accpath, tpath, name, m));
		} else {
			struct dirent *de;
			while ((de = readdir(dp)) != nullptr) {
				if (strcmp(de->d_name, ".") == 0 ||
				    strcmp(de->d_name, "..") == 0) {
					continue;
				}
				std::string child_acc = accpath + "/" +
				    de->d_name;
				std::string child_path = tpath + "/" +
				    de->d_name;
				walk_tree(st, child_acc, child_path,
				    level + 1, root_dev);
			}
			closedir(dp);
		}
		st->nodes.emplace_back(
		    make_node(FTS_DP, level, 0, accpath, tpath, name, m));
		return;
	}

	if (S_ISLNK(sb.st_mode) &&
	    !((st->options & FTS_LOGICAL) ||
	    ((st->options & FTS_COMFOLLOW) && level == FTS_ROOTLEVEL))) {
		st->nodes.emplace_back(
		    make_node(FTS_SL, level, 0, accpath, tpath, name, m));
		return;
	}

	st->nodes.emplace_back(
	    make_node(FTS_F, level, 0, accpath, tpath, name, m));
}

extern "C" FTS *
__wrap_fts_open(char *const *argv, int options,
    int (*compar)(const FTSENT *const *, const FTSENT *const *))
{
	(void)compar;

	if (!g_test_child || argv == nullptr) {
		return (nullptr);
	}

	auto *st = new FtsState();
	st->pub.fts_options = options;
	st->options = options;
	st->idx = 0;
	st->skip_pending = false;
	st->skip_level = 0;

	for (; argv[0] != nullptr; ++argv) {
		struct stat sb;
		dev_t root_dev = 0;
		if (stat(argv[0], &sb) == 0) {
			root_dev = sb.st_dev;
		}
		walk_tree(st, argv[0], argv[0], FTS_ROOTLEVEL, root_dev);
	}

	return (reinterpret_cast<FTS *>(st));
}

extern "C" FTSENT *
__wrap_fts_read(FTS *ftsp)
{
	FtsState *st = fts_state(ftsp);
	if (st == nullptr) {
		return (nullptr);
	}
	while (st->idx < st->nodes.size()) {
		FtsNode *node = st->nodes[st->idx].get();
		if (st->skip_pending) {
			if (node->ent.fts_level > st->skip_level) {
				st->idx++;
				continue;
			}
			if (node->ent.fts_info == FTS_DP &&
			    node->ent.fts_level == st->skip_level) {
				st->skip_pending = false;
			} else {
				st->idx++;
				continue;
			}
		}
		mode_t m = lookup_mode(node->accpath);
		node->statbuf.st_mode =
		    (node->statbuf.st_mode & S_IFMT) | (m & ALLPERMS);
		st->idx++;
		return (&node->ent);
	}
	return (nullptr);
}

extern "C" int
__wrap_fts_set(FTS *ftsp, FTSENT *p, int instr)
{
	FtsState *st = fts_state(ftsp);
	if (st == nullptr || p == nullptr) {
		return (-1);
	}
	if (instr == FTS_SKIP) {
		st->skip_pending = true;
		st->skip_level = p->fts_level;
	}
	return (0);
}

extern "C" int
__wrap_fts_close(FTS *ftsp)
{
	FtsState *st = fts_state(ftsp);
	if (st == nullptr) {
		return (-1);
	}
	delete st;
	return (0);
}

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
		if (hi <= lo) {
			return (lo);
		}
		return (lo + (int)(next() % (std::uint64_t)(hi - lo + 1)));
	}

	bool coin()
	{
		return ((next() & 1u) != 0);
	}

	unsigned char byte()
	{
		return ((unsigned char)(next() & 0xffu));
	}
};

Rng rng(0xb0195faceULL);

Stat st_main = { "main", 0, 0, 0 };
Stat st_siginfo_handler = { "siginfo_handler", 0, 0, 0 };
Stat st_usage = { "usage", 0, 0, 0 };
Stat st_may_have_nfs4acl = { "may_have_nfs4acl", 0, 0, 0 };

struct ExitRun {
	int status;
	std::vector<unsigned char> stdout_bytes;
	std::vector<unsigned char> stderr_bytes;
};

struct Nfs4Call {
	std::string accpath;
	std::string path;
	dev_t dev;
	int hflag;
	PcCfg pc;
	bool use_lpath;
};

struct Nfs4Run {
	std::vector<int> rets;
	std::vector<unsigned char> stderr_bytes;
};

bool
fail(Stat &st, const char *what)
{
	st.fails++;
	if (st.shown < MAX_SHOW) {
		st.shown++;
		std::printf("  FAIL %s: %s\n", st.name, what);
	}
	return (false);
}

bool
same_bytes(const std::vector<unsigned char> &a,
    const std::vector<unsigned char> &b)
{
	return (a == b);
}

void
reset_host()
{
	g_modes.clear();
	g_fail_paths.clear();
	g_pathconf.clear();
	g_lpathconf.clear();
	g_nfs4_direct = false;
	ref_siginfo = 0;
	P::siginfo = 0;
	optind = 1;
	opterr = 1;
}

std::string
make_temp_root()
{
	char tmpl[] = "/tmp/pbsd_b0195_XXXXXX";
	char *dir = mkdtemp(tmpl);
	if (dir == nullptr) {
		return ("");
	}
	return (std::string(dir));
}

bool
write_file(const std::string &path, const char *data)
{
	FILE *fp = fopen(path.c_str(), "wb");
	if (fp == nullptr) {
		return (false);
	}
	if (data != nullptr) {
		fputs(data, fp);
	}
	fclose(fp);
	return (true);
}

bool
mkpath(const std::string &path)
{
	if (mkdir(path.c_str(), 0755) == 0) {
		return (true);
	}
	if (errno == EEXIST) {
		return (true);
	}
	return (false);
}

ExitRun
capture_run(int (*fn)(int, char **), int argc, char **argv)
{
	ExitRun res{};
	int pipe_out[2];
	int pipe_err[2];

	if (pipe(pipe_out) != 0 || pipe(pipe_err) != 0) {
		return (res);
	}
	pid_t pid = fork();
	if (pid < 0) {
		return (res);
	}
	if (pid == 0) {
		dup2(pipe_out[1], STDOUT_FILENO);
		dup2(pipe_err[1], STDERR_FILENO);
		close(pipe_out[0]);
		close(pipe_out[1]);
		close(pipe_err[0]);
		close(pipe_err[1]);
		g_test_child = true;
		int ret = fn(argc, argv);
		::_exit(ret);
	}
	close(pipe_out[1]);
	close(pipe_err[1]);
	unsigned char buf[4096];
	ssize_t nr;
	while ((nr = read(pipe_out[0], buf, sizeof(buf))) > 0) {
		res.stdout_bytes.insert(res.stdout_bytes.end(), buf, buf + nr);
	}
	while ((nr = read(pipe_err[0], buf, sizeof(buf))) > 0) {
		res.stderr_bytes.insert(res.stderr_bytes.end(), buf, buf + nr);
	}
	close(pipe_out[0]);
	close(pipe_err[0]);
	int st = 0;
	if (waitpid(pid, &st, 0) >= 0 && WIFEXITED(st)) {
		res.status = WEXITSTATUS(st);
	}
	return (res);
}

ExitRun
run_ref_main(int argc, char **argv)
{
	return (capture_run(
	    reinterpret_cast<int (*)(int, char **)>(ref_main), argc, argv));
}

ExitRun
run_port_main(int argc, char **argv)
{
	return (capture_run(
	    reinterpret_cast<int (*)(int, char **)>(P::main), argc, argv));
}

bool
check_main(const char *label, int argc, char **argv,
    const std::unordered_map<std::string, mode_t> &initial_modes,
    const std::unordered_map<std::string, PcCfg> &pathconf_cfg = {})
{
	st_main.cases++;
	reset_host();
	for (const auto &kv : initial_modes) {
		g_modes[kv.first] = kv.second;
	}
	for (const auto &kv : pathconf_cfg) {
		g_pathconf[kv.first] = kv.second;
	}
	ExitRun r = run_ref_main(argc, argv);
	reset_host();
	for (const auto &kv : initial_modes) {
		g_modes[kv.first] = kv.second;
	}
	for (const auto &kv : pathconf_cfg) {
		g_pathconf[kv.first] = kv.second;
	}
	ExitRun p = run_port_main(argc, argv);
	if (r.status != p.status) {
		std::printf("  [%s] status ref=%d port=%d\n", label, r.status,
		    p.status);
		return (fail(st_main, label));
	}
	if (!same_bytes(r.stdout_bytes, p.stdout_bytes)) {
		return (fail(st_main, label));
	}
	if (!same_bytes(r.stderr_bytes, p.stderr_bytes)) {
		return (fail(st_main, label));
	}
	return (true);
}

bool
check_siginfo_handler(const char *label, int sig)
{
	st_siginfo_handler.cases++;
	ref_siginfo = 0;
	P::siginfo = 0;
	ref_siginfo_handler(sig);
	int r = (int)ref_siginfo;
	ref_siginfo = 0;
	P::siginfo_handler(sig);
	int p = (int)P::siginfo;
	if (r != p) {
		std::printf("  [%s] ref=%d port=%d\n", label, r, p);
		return (fail(st_siginfo_handler, label));
	}
	return (true);
}

ExitRun
capture_usage(void (*fn)())
{
	ExitRun res{};
	int pipe_out[2];
	int pipe_err[2];

	if (pipe(pipe_out) != 0 || pipe(pipe_err) != 0) {
		return (res);
	}
	pid_t pid = fork();
	if (pid < 0) {
		return (res);
	}
	if (pid == 0) {
		dup2(pipe_out[1], STDOUT_FILENO);
		dup2(pipe_err[1], STDERR_FILENO);
		close(pipe_out[0]);
		close(pipe_out[1]);
		close(pipe_err[0]);
		close(pipe_err[1]);
		g_test_child = true;
		fn();
		::_exit(99);
	}
	close(pipe_out[1]);
	close(pipe_err[1]);
	unsigned char buf[4096];
	ssize_t nr;
	while ((nr = read(pipe_out[0], buf, sizeof(buf))) > 0) {
		res.stdout_bytes.insert(res.stdout_bytes.end(), buf, buf + nr);
	}
	while ((nr = read(pipe_err[0], buf, sizeof(buf))) > 0) {
		res.stderr_bytes.insert(res.stderr_bytes.end(), buf, buf + nr);
	}
	close(pipe_out[0]);
	close(pipe_err[0]);
	int st = 0;
	if (waitpid(pid, &st, 0) >= 0 && WIFEXITED(st)) {
		res.status = WEXITSTATUS(st);
	}
	return (res);
}

bool
check_usage(const char *label)
{
	st_usage.cases++;
	ExitRun r = capture_usage(ref_usage);
	ExitRun p = capture_usage(P::usage);
	if (r.status != p.status) {
		return (fail(st_usage, label));
	}
	if (!same_bytes(r.stdout_bytes, p.stdout_bytes)) {
		return (fail(st_usage, label));
	}
	if (!same_bytes(r.stderr_bytes, p.stderr_bytes)) {
		return (fail(st_usage, label));
	}
	return (true);
}

struct Nfs4Ent {
	FTSENT ent;
	struct stat statbuf;
	std::string accpath;
	std::string path;
	std::vector<char> namebuf;
};

static Nfs4Ent
make_nfs4_ent(const Nfs4Call &c)
{
	Nfs4Ent node;
	node.accpath = c.accpath;
	node.path = c.path;
	const char *name = c.accpath.c_str();
	const char *slash = strrchr(name, '/');
	if (slash != nullptr) {
		name = slash + 1;
	}
	node.namebuf.assign(name, name + strlen(name) + 1);
	std::memset(&node.statbuf, 0, sizeof(node.statbuf));
	node.statbuf.st_dev = c.dev;
	node.statbuf.st_mode = S_IFREG | 0644;
	std::memset(&node.ent, 0, sizeof(node.ent));
	node.ent.fts_accpath = const_cast<char *>(node.accpath.c_str());
	node.ent.fts_path = const_cast<char *>(node.path.c_str());
	node.ent.fts_statp = &node.statbuf;
	node.ent.fts_name = node.namebuf.data();
	return (node);
}

Nfs4Run
run_nfs4_seq(bool use_port, const std::vector<Nfs4Call> &calls)
{
	Nfs4Run res{};
	int pipe_out[2];
	int pipe_err[2];
	if (pipe(pipe_out) != 0 || pipe(pipe_err) != 0) {
		return (res);
	}
	pid_t pid = fork();
	if (pid < 0) {
		return (res);
	}
	if (pid == 0) {
		dup2(pipe_out[1], STDOUT_FILENO);
		dup2(pipe_err[1], STDERR_FILENO);
		close(pipe_out[0]);
		close(pipe_out[1]);
		close(pipe_err[0]);
		close(pipe_err[1]);
		g_test_child = true;
		g_nfs4_direct = true;
		for (const auto &c : calls) {
			if (c.use_lpath) {
				g_lpathconf[c.accpath] = c.pc;
			} else {
				g_pathconf[c.accpath] = c.pc;
			}
			Nfs4Ent node = make_nfs4_ent(c);
			int v;
			if (use_port) {
				v = P::may_have_nfs4acl(&node.ent, c.hflag);
			} else {
				v = ref_may_have_nfs4acl(&node.ent, c.hflag);
			}
			if (write(STDOUT_FILENO, &v, sizeof(v)) != sizeof(v)) {
				::_exit(1);
			}
		}
		::_exit(0);
	}
	close(pipe_out[1]);
	close(pipe_err[1]);
	unsigned char buf[4096];
	ssize_t nr;
	while ((nr = read(pipe_out[0], buf, sizeof(buf))) > 0) {
		const int *vals = reinterpret_cast<const int *>(buf);
		size_t n = (size_t)nr / sizeof(int);
		for (size_t i = 0; i < n; i++) {
			res.rets.push_back(vals[i]);
		}
	}
	while ((nr = read(pipe_err[0], buf, sizeof(buf))) > 0) {
		res.stderr_bytes.insert(res.stderr_bytes.end(), buf, buf + nr);
	}
	close(pipe_out[0]);
	close(pipe_err[0]);
	int st = 0;
	(void)waitpid(pid, &st, 0);
	return (res);
}

bool
check_nfs4(const char *label, const std::vector<Nfs4Call> &calls)
{
	st_may_have_nfs4acl.cases++;
	reset_host();
	Nfs4Run r = run_nfs4_seq(false, calls);
	reset_host();
	Nfs4Run p = run_nfs4_seq(true, calls);
	if (r.rets != p.rets) {
		std::printf("  [%s] rets differ\n", label);
		return (fail(st_may_have_nfs4acl, label));
	}
	if (!same_bytes(r.stderr_bytes, p.stderr_bytes)) {
		return (fail(st_may_have_nfs4acl, label));
	}
	return (true);
}

bool
run_main_case(const char *label, const std::vector<std::string> &args,
    const std::unordered_map<std::string, mode_t> &initial_modes,
    const std::unordered_map<std::string, PcCfg> &pathconf_cfg = {})
{
	static char prog[] = "chmod";
	std::vector<std::string> pool;
	std::vector<char *> argv;
	argv.push_back(prog);
	for (const auto &a : args) {
		pool.push_back(a);
		argv.push_back(const_cast<char *>(pool.back().c_str()));
	}
	argv.push_back(nullptr);
	return (check_main(label, (int)argv.size() - 1, argv.data(),
	    initial_modes, pathconf_cfg));
}

void
test_siginfo_handler_edge()
{
	(void)check_siginfo_handler("siginfo", SIGINFO);
	(void)check_siginfo_handler("sigterm", SIGTERM);
	(void)check_siginfo_handler("zero", 0);
	(void)check_siginfo_handler("highbit", 0x80);
	(void)check_siginfo_handler("neg", -1);
}

void
test_siginfo_handler_random()
{
	for (long i = 0; i < SWEEP; i++) {
		int sig = rng.bits(-128, 255);
		(void)check_siginfo_handler("sweep", sig);
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
	for (long i = 0; i < SWEEP; i++) {
		(void)check_usage("sweep");
	}
}

void
test_may_have_nfs4acl_edge()
{
	std::vector<Nfs4Call> one;
	one.push_back({ "/a/f", "/a/f", 1, 0, { 1, 0 }, false });
	(void)check_nfs4("acl_yes", one);

	one.clear();
	one.push_back({ "/b/f", "/b/f", 2, 0, { 0, 0 }, false });
	(void)check_nfs4("acl_no", one);

	one.clear();
	one.push_back({ "/c/f", "/c/f", 3, 0, { -1, EINVAL }, false });
	(void)check_nfs4("einval", one);

	one.clear();
	one.push_back({ "/d/f", "/d/f", 4, 0, { -1, EIO }, false });
	(void)check_nfs4("warn", one);

	one.clear();
	one.push_back({ "/e/l", "/e/l", 5, 1, { 1, 0 }, true });
	(void)check_nfs4("hflag_lpath", one);

	std::vector<Nfs4Call> cache;
	cache.push_back({ "/f/a", "/f/a", 10, 0, { 1, 0 }, false });
	cache.push_back({ "/f/b", "/f/b", 10, 0, { 0, 0 }, false });
	(void)check_nfs4("same_dev_cache", cache);

	std::vector<Nfs4Call> diff;
	diff.push_back({ "/g/a", "/g/a", 20, 0, { 1, 0 }, false });
	diff.push_back({ "/h/b", "/h/b", 21, 0, { 0, 0 }, false });
	(void)check_nfs4("diff_dev", diff);

	std::vector<Nfs4Call> multi;
	multi.push_back({ "/i/a", "/i/a", 30, 0, { 2, 0 }, false });
	multi.push_back({ "/i/b", "/i/b", 30, 0, { -1, EIO }, false });
	multi.push_back({ "/j/c", "/j/c", 31, 1, { 1, 0 }, true });
	(void)check_nfs4("multi", multi);
}

void
test_may_have_nfs4acl_random()
{
	for (long i = 0; i < SWEEP; i++) {
		int n = rng.bits(1, 4);
		std::vector<Nfs4Call> calls;
		for (int j = 0; j < n; j++) {
			Nfs4Call c;
			c.accpath = "/z/p" + std::to_string(i) + "_" +
			    std::to_string(j);
			c.path = c.accpath;
			c.dev = (dev_t)rng.bits(1, 100);
			c.hflag = rng.coin() ? 1 : 0;
			c.use_lpath = c.hflag != 0;
			int kind = rng.bits(0, 3);
			if (kind == 0) {
				c.pc = { 1, 0 };
			} else if (kind == 1) {
				c.pc = { 0, 0 };
			} else if (kind == 2) {
				c.pc = { -1, EINVAL };
			} else {
				c.pc = { -1, EIO };
			}
			calls.push_back(c);
		}
		(void)check_nfs4("sweep", calls);
	}
}

void
test_main_edge()
{
	std::string root = make_temp_root();
	if (root.empty()) {
		std::fprintf(stderr, "mkdtemp failed\n");
		std::exit(2);
	}
	std::string f1 = root + "/f1";
	std::string f2 = root + "/f2";
	std::string sub = root + "/sub";
	std::string fsub = sub + "/inner";
	std::string link = root + "/alink";
	std::string nodir = root + "/missing";
	std::string badperm = root + "/badperm";

	(void)write_file(f1, "a");
	(void)write_file(f2, "b");
	(void)mkpath(sub);
	(void)write_file(fsub, "c");
	(void)symlink(f1.c_str(), link.c_str());
	(void)mkpath(badperm);
	(void)chmod(badperm.c_str(), 0000);

	std::unordered_map<std::string, mode_t> modes;
	modes[f1] = 0644;
	modes[f2] = 0600;
	modes[fsub] = 0755;
	modes[link] = 0777;
	modes[sub] = 0755;
	modes[root] = 0755;

	(void)run_main_case("usage_no_args", {}, {});
	(void)run_main_case("usage_mode_only", { "644" }, {});
	(void)run_main_case("rh_conflict", { "-R", "-h", "644", f1 }, {});
	(void)run_main_case("octal_644", { "644", f1 }, modes);
	(void)run_main_case("octal_755", { "755", f1 }, modes);
	(void)run_main_case("invalid_mode", { "9bad", f1 }, modes);
	(void)run_main_case("sym_ux", { "u+x", f1 }, modes);
	(void)run_main_case("sym_go_w", { "go-w", f2 }, modes);
	(void)run_main_case("sym_plus_x", { "+x", f1 }, modes);
	(void)run_main_case("sym_a_r", { "a+r", f1 }, modes);
	(void)run_main_case("mode_dash_r", { "-r", f1 }, modes);
	(void)run_main_case("mode_dash_w", { "-w", f2 }, modes);
	(void)run_main_case("mode_dash_x", { "-x", f1 }, modes);
	(void)run_main_case("verbose", { "-v", "644", f1 }, modes);
	(void)run_main_case("verbose2", { "-vv", "644", f1 }, modes);
	(void)run_main_case("force_fail", { "-f", "644", nodir }, modes);
	(void)run_main_case("noforce_fail", { "644", nodir }, modes);
	(void)run_main_case("physical_h", { "-h", "644", link }, modes);
	(void)run_main_case("recursive", { "-R", "644", root }, modes);
	(void)run_main_case("recursive_L", { "-R", "-L", "644", root }, modes);
	(void)run_main_case("recursive_H", { "-R", "-H", "644", root }, modes);
	(void)run_main_case("recursive_P", { "-R", "-P", "644", root }, modes);
	(void)run_main_case("unchanged", { "644", f1 },
	    { { f1, 0644 } });
	(void)run_main_case("two_files", { "755", f1, f2 }, modes);
	(void)run_main_case("dir_no_r", { "755", sub }, modes);
	(void)run_main_case("X_dir", { "X", sub }, modes);
	(void)run_main_case("nfs4_skip", { "644", f1 },
	    { { f1, 0644 } }, { { f1, { 1, 0 } } });

	(void)chmod(badperm.c_str(), 0755);
	(void)unlink(link.c_str());
	(void)unlink(fsub.c_str());
	(void)rmdir(sub.c_str());
	(void)unlink(f2.c_str());
	(void)unlink(f1.c_str());
	(void)rmdir(badperm.c_str());
	(void)rmdir(root.c_str());
}

std::string
rand_mode_spec(Rng &r)
{
	switch (r.bits(0, 9)) {
	case 0:
		return ("644");
	case 1:
		return ("755");
	case 2:
		return ("u+x");
	case 3:
		return ("go-w");
	case 4:
		return ("+x");
	case 5:
		return ("a+r");
	case 6:
		return ("-r");
	case 7:
		return ("-w");
	case 8:
		return ("9bad");
	default: {
		std::string s;
		int n = r.bits(1, 4);
		for (int i = 0; i < n; i++) {
			s.push_back((char)('0' + r.bits(0, 7)));
		}
		return (s);
	}
	}
}

std::string
rand_opts(Rng &r)
{
	std::string o;
	if (r.coin()) {
		o += "-";
	}
	if (r.coin()) {
		o += 'R';
	}
	if (r.coin()) {
		o += 'H';
	}
	if (r.coin()) {
		o += 'L';
	}
	if (r.coin()) {
		o += 'P';
	}
	if (r.coin()) {
		o += 'f';
	}
	if (r.coin()) {
		o += 'h';
	}
	if (r.bits(0, 2) > 0) {
		o.append(r.bits(0, 2), 'v');
	}
	if (o == "-") {
		return ("");
	}
	return (o);
}

void
test_main_random()
{
	for (long i = 0; i < SWEEP; i++) {
		std::string root = make_temp_root();
		if (root.empty()) {
			continue;
		}
		int nfiles = rng.bits(0, 4);
		std::vector<std::string> paths;
		std::unordered_map<std::string, mode_t> initial;
		std::unordered_map<std::string, PcCfg> pc;
		for (int j = 0; j < nfiles; j++) {
			std::string p = root + "/f" + std::to_string(j);
			char ch = (char)rng.byte();
			(void)write_file(p, std::string(1, ch));
			(void)chmod(p.c_str(), (mode_t)rng.bits(0, 0777));
			paths.push_back(p);
			initial[p] = (mode_t)rng.bits(0, 0777);
		}
		if (rng.coin()) {
			std::string sub = root + "/d";
			(void)mkpath(sub);
			std::string inner = sub + "/x";
			(void)write_file(inner, "z");
			paths.push_back(sub);
			initial[sub] = 0755;
			initial[inner] = (mode_t)rng.bits(0, 0777);
		}
		if (rng.coin() && !paths.empty()) {
			std::string lnk = root + "/l";
			(void)symlink(paths[0].c_str(), lnk.c_str());
			paths.push_back(lnk);
			initial[lnk] = 0777;
		}
		if (rng.coin()) {
			std::string p = root + "/nfs";
			(void)write_file(p, "n");
			paths.push_back(p);
			initial[p] = (mode_t)rng.bits(0, 0777);
			if (rng.coin()) {
				pc[p] = { 1, 0 };
			}
		}

		std::vector<std::string> args;
		std::string opt = rand_opts(rng);
		if (!opt.empty()) {
			args.push_back(opt);
		}
		args.push_back(rand_mode_spec(rng));
		if (paths.empty()) {
			args.push_back(root + "/none");
		} else {
			int pick = rng.bits(0, (int)paths.size() - 1);
			args.push_back(paths[pick]);
			if (rng.coin() && paths.size() > 1) {
				int pick2 = rng.bits(0, (int)paths.size() - 1);
				args.push_back(paths[pick2]);
			}
		}

		(void)run_main_case("sweep", args, initial, pc);
		for (const auto &p : paths) {
			(void)unlink(p.c_str());
		}
		(void)unlink((root + "/d/x").c_str());
		(void)rmdir((root + "/d").c_str());
		(void)rmdir(root.c_str());
	}
}

} // namespace

int
main()
{
	test_siginfo_handler_edge();
	test_usage_edge();
	test_may_have_nfs4acl_edge();
	test_main_edge();

	test_siginfo_handler_random();
	test_usage_random();
	test_may_have_nfs4acl_random();
	test_main_random();

	Stat *all[] = { &st_siginfo_handler, &st_usage, &st_may_have_nfs4acl,
	    &st_main };
	long total_cases = 0;
	long total_fails = 0;

	std::printf("\n%-18s %12s %12s\n", "function", "cases", "failures");
	for (Stat *st : all) {
		std::printf("%-18s %12ld %12ld\n", st->name, st->cases,
		    st->fails);
		total_cases += st->cases;
		total_fails += st->fails;
	}
	std::printf("%-18s %12ld %12ld\n", "TOTAL", total_cases, total_fails);

	return (total_fails == 0 ? 0 : 1);
}
