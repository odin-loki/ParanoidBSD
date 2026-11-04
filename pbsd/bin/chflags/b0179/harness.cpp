/*
 * harness.cpp -- differential test for PBSD batch b0179 (chflags.c).
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

import pbsd.bin.chflags.b0179;

namespace P = pbsd::bin_chflags::b0179;

extern "C" {
int ref_main(int argc, char *argv[]);
void ref_siginfo_handler(int sig);
void ref_usage(void);
extern volatile sig_atomic_t ref_siginfo;
void __real_exit(int status);
}

#if defined(__linux__)
struct pbsd_filestat {
	unsigned long st_flags;
};
#endif

#ifndef UF_NODUMP
#define UF_NODUMP	0x00000001
#define UF_ARCHIVE	0x00020000
#define UF_HIDDEN	0x00080000
#endif

#ifndef SIGINFO
#define SIGINFO SIGUSR1
#endif

#define SWEEP 200000L
#define MAX_SHOW 8

namespace {

bool g_test_child = false;

extern "C" void
__wrap_exit(int status)
{
	if (g_test_child) {
		::_exit(status);
	}
	__real_exit(status);
}

std::unordered_map<std::string, unsigned long> g_flags;
std::unordered_set<std::string> g_fail_paths;

extern "C" int
__wrap_chflagsat(int fd, const char *path, unsigned long flags, int atflag)
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
	g_flags[std::string(path)] = flags;
	return (0);
}

static unsigned long
lookup_flags(const std::string &path)
{
	auto it = g_flags.find(path);
	if (it != g_flags.end()) {
		return (it->second);
	}
	return (0);
}

struct FtsNode {
	FTSENT ent;
	pbsd_filestat statbuf;
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
    const std::string &path, const char *name)
{
	auto node = std::make_unique<FtsNode>();
	node->accpath = accpath;
	node->path = path;
	node->statbuf.st_flags = lookup_flags(accpath);
	node->namebuf.assign(name, name + strlen(name) + 1);

	std::memset(&node->ent, 0, sizeof(node->ent));
	node->ent.fts_info = (unsigned short)info;
	node->ent.fts_level = (short)level;
	node->ent.fts_errno = fts_errno;
	node->ent.fts_accpath = const_cast<char *>(node->accpath.c_str());
	node->ent.fts_path = const_cast<char *>(node->path.c_str());
	node->ent.fts_namelen = (unsigned short)(strlen(name));
	node->ent.fts_statp = reinterpret_cast<struct stat *>(&node->statbuf);
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

	if (path_stat(accpath.c_str(), &sb, st->options, level) != 0) {
		const char *base = accpath.c_str();
		const char *slash = strrchr(base, '/');
		const char *name = slash != nullptr ? slash + 1 : base;
		st->nodes.emplace_back(make_node(FTS_NS, level, errno, accpath,
		    tpath, name));
		return;
	}

	const char *base = accpath.c_str();
	const char *slash = strrchr(base, '/');
	const char *name = slash != nullptr ? slash + 1 : base;

	if (S_ISDIR(sb.st_mode)) {
		st->nodes.emplace_back(
		    make_node(FTS_D, level, 0, accpath, tpath, name));
		if ((sb.st_mode & S_IXUSR) == 0) {
			st->nodes.emplace_back(make_node(FTS_DNR, level, EACCES,
			    accpath, tpath, name));
			st->nodes.emplace_back(
			    make_node(FTS_DP, level, 0, accpath, tpath, name));
			return;
		}
		DIR *dp = opendir(accpath.c_str());
		if (dp == nullptr) {
			st->nodes.emplace_back(make_node(FTS_DNR, level,
			    errno, accpath, tpath, name));
		} else {
			if ((st->options & FTS_XDEV) &&
			    sb.st_dev != root_dev) {
				closedir(dp);
				st->nodes.emplace_back(make_node(FTS_DP,
				    level, 0, accpath, tpath, name));
				return;
			}
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
		    make_node(FTS_DP, level, 0, accpath, tpath, name));
		return;
	}

	if (S_ISLNK(sb.st_mode) &&
	    !((st->options & FTS_LOGICAL) ||
	    ((st->options & FTS_COMFOLLOW) && level == FTS_ROOTLEVEL))) {
		st->nodes.emplace_back(
		    make_node(FTS_SL, level, 0, accpath, tpath, name));
		return;
	}

	st->nodes.emplace_back(
	    make_node(FTS_F, level, 0, accpath, tpath, name));
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
		node->statbuf.st_flags = lookup_flags(node->accpath);
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

Rng rng(0xb0179faceULL);

Stat st_main = { "main", 0, 0, 0 };
Stat st_siginfo_handler = { "siginfo_handler", 0, 0, 0 };
Stat st_usage = { "usage", 0, 0, 0 };

struct ExitRun {
	int status;
	std::vector<unsigned char> stdout_bytes;
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
	g_flags.clear();
	g_fail_paths.clear();
	ref_siginfo = 0;
	P::siginfo = 0;
	optind = 1;
	opterr = 1;
}

std::string
make_temp_root()
{
	char tmpl[] = "/tmp/pbsd_b0179_XXXXXX";
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
run_ref_main(int argc, char **argv)
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
		int ret = ref_main(argc, argv);
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
run_port_main(int argc, char **argv)
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
		int ret = P::main(argc, argv);
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

bool
check_main(const char *label, int argc, char **argv,
    const std::unordered_map<std::string, unsigned long> &initial_flags)
{
	st_main.cases++;
	reset_host();
	for (const auto &kv : initial_flags) {
		g_flags[kv.first] = kv.second;
	}
	ExitRun r = run_ref_main(argc, argv);
	reset_host();
	for (const auto &kv : initial_flags) {
		g_flags[kv.first] = kv.second;
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
run_ref_usage()
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
		ref_usage();
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

ExitRun
run_port_usage()
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
		P::usage();
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
	ExitRun r = run_ref_usage();
	ExitRun p = run_port_usage();
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

bool
run_main_case(const char *label, const std::vector<std::string> &args,
    const std::unordered_map<std::string, unsigned long> &initial_flags)
{
	static char prog[] = "chflags";
	std::vector<std::string> pool;
	std::vector<char *> argv;
	argv.push_back(prog);
	for (const auto &a : args) {
		pool.push_back(a);
		argv.push_back(const_cast<char *>(pool.back().c_str()));
	}
	argv.push_back(nullptr);
	return (check_main(label, (int)argv.size() - 1, argv.data(),
	    initial_flags));
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

	std::unordered_map<std::string, unsigned long> fl;
	fl[f1] = UF_NODUMP;
	fl[f2] = UF_ARCHIVE;
	fl[fsub] = UF_HIDDEN;
	fl[link] = 0;

	(void)run_main_case("usage_no_args", {}, {});
	(void)run_main_case("usage_flags_only", { "nodump" }, {});
	(void)run_main_case("rh_conflict", { "-R", "-h", "nodump", f1 }, {});
	(void)run_main_case("octal_zero", { "0", f1 }, { { f1, 0 } });
	(void)run_main_case("octal_set", { "2", f1 }, { { f1, 0 } });
	(void)run_main_case("octal_invalid", { "8bad", f1 }, { { f1, 0 } });
	(void)run_main_case("octal_neg", { "-1", f1 }, { { f1, 0 } });
	(void)run_main_case("sym_nodump", { "nodump", f1 }, { { f1, 0 } });
	(void)run_main_case("sym_hidden", { "hidden", f2 }, { { f2, 0 } });
	(void)run_main_case("sym_bad", { "notaflag", f1 }, { { f1, 0 } });
	(void)run_main_case("sym_multi", { "nodump,hidden", f1 }, { { f1, 0 } });
	(void)run_main_case("sym_no", { "nohidden", f2 }, { { f2, UF_HIDDEN } });
	(void)run_main_case("verbose", { "-v", "nodump", f1 }, { { f1, 0 } });
	(void)run_main_case("verbose2", { "-vv", "2", f1 }, { { f1, 0 } });
	(void)run_main_case("force_fail", { "-f", "nodump", nodir }, {});
	(void)run_main_case("noforce_fail", { "nodump", nodir }, {});
	(void)run_main_case("physical_h", { "-h", "nodump", link }, fl);
	(void)run_main_case("recursive", { "-R", "nodump", root }, fl);
	(void)run_main_case("recursive_L", { "-R", "-L", "nodump", root }, fl);
	(void)run_main_case("recursive_H", { "-R", "-H", "nodump", root }, fl);
	(void)run_main_case("recursive_P", { "-R", "-P", "nodump", root }, fl);
	(void)run_main_case("xdev", { "-R", "-x", "nodump", root }, fl);
	(void)run_main_case("unchanged", { "nodump", f1 }, { { f1, UF_NODUMP } });
	(void)run_main_case("two_files", { "hidden", f1, f2 }, fl);
	(void)run_main_case("empty_flag_token", { ",", f1 }, { { f1, 0 } });
	(void)run_main_case("high_octal", { "177777", f1 }, { { f1, 0 } });

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
rand_flag_spec(Rng &r)
{
	switch (r.bits(0, 7)) {
	case 0:
		return ("0");
	case 1:
		return ("2");
	case 2:
		return ("nodump");
	case 3:
		return ("hidden");
	case 4:
		return ("nohidden");
	case 5:
		return ("nodump,hidden");
	case 6:
		return ("8bad");
	default: {
		std::string s;
		int n = r.bits(1, 6);
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
	if (r.coin()) {
		o += 'x';
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
		std::unordered_map<std::string, unsigned long> initial;
		for (int j = 0; j < nfiles; j++) {
			std::string p = root + "/f" + std::to_string(j);
			char ch = (char)rng.byte();
			(void)write_file(p, std::string(1, ch));
			paths.push_back(p);
			initial[p] = (unsigned long)rng.bits(0, 7) * UF_NODUMP;
		}
		if (rng.coin()) {
			std::string sub = root + "/d";
			(void)mkpath(sub);
			std::string inner = sub + "/x";
			(void)write_file(inner, "z");
			paths.push_back(sub);
			initial[inner] = UF_ARCHIVE;
		}
		if (rng.coin()) {
			std::string lnk = root + "/l";
			if (!paths.empty()) {
				(void)symlink(paths[0].c_str(), lnk.c_str());
				paths.push_back(lnk);
				initial[lnk] = 0;
			}
		}

		std::vector<std::string> args;
		std::string opt = rand_opts(rng);
		if (!opt.empty()) {
			args.push_back(opt);
		}
		args.push_back(rand_flag_spec(rng));
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

		(void)run_main_case("sweep", args, initial);
		for (const auto &p : paths) {
			(void)unlink(p.c_str());
		}
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
	test_main_edge();

	test_siginfo_handler_random();
	test_usage_random();
	test_main_random();

	Stat *all[] = { &st_siginfo_handler, &st_usage, &st_main };
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
