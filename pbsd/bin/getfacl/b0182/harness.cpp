/*
 * harness.cpp -- differential test for PBSD batch b0182 (getfacl.c).
 */

#define _GNU_SOURCE

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <grp.h>
#include <map>
#include <pwd.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

import pbsd.bin.getfacl.b0182;

namespace P = pbsd::bin_getfacl::b0182;

extern "C" {
void ref_usage(void);
char *ref_getuname(uid_t uid);
char *ref_getgname(gid_t gid);
int ref_print_acl(char *path, unsigned int type, int hflag, int iflag,
    int nflag, int qflag, int vflag, int sflag);
int ref_print_acl_from_stdin(unsigned int type, int hflag, int iflag,
    int nflag, int qflag, int vflag, int sflag);
int ref_main(int argc, char **argv);
extern int ref_more_than_one;
}

namespace {

typedef void *acl_t;
typedef unsigned int acl_type_t;

enum {
	ACL_TYPE_ACCESS = 0x00000002,
	ACL_TYPE_DEFAULT = 0x00000003,
	ACL_TYPE_NFS4 = 0x00000004,
	_PC_ACL_NFS4 = 64,
};

constexpr long SWEEP = 200000L;
constexpr int MAX_SHOW = 8;
constexpr unsigned char GUARD = 0x7f;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int shown;
};

struct PathCfg {
	bool present = true;
	int stat_errno = ENOENT;
	uid_t st_uid = 1000;
	gid_t st_gid = 1000;
	mode_t st_mode = 0644;
	int pathconf_ret = 0;
	int pathconf_errno = 0;
	uintptr_t acl = 0;
	int acl_errno = EOPNOTSUPP;
	int trivial = 0;
	int trivial_fail = 0;
	int from_mode_fail = 0;
	std::string acl_text = "user::rw-\ngroup::r--\nother::r--\n";
	int acl_text_fail = 0;
};

struct MockState {
	std::map<std::string, PathCfg> paths;
	std::map<uid_t, std::string> users;
	std::map<gid_t, std::string> groups;
	std::vector<std::string> stdin_lines;
	std::size_t stdin_pos = 0;
	uintptr_t next_acl = 0x1000;
};

MockState g_mock;
Rng {
} // placeholder - define Rng below

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

Rng rng(0xb0182faceULL);

Stat st_usage = { "usage", 0, 0, 0 };
Stat st_getuname = { "getuname", 0, 0, 0 };
Stat st_getgname = { "getgname", 0, 0, 0 };
Stat st_print_acl = { "print_acl", 0, 0, 0 };
Stat st_print_acl_from_stdin = { "print_acl_from_stdin", 0, 0, 0 };
Stat st_main = { "main", 0, 0, 0 };

PathCfg *
find_path(const char *path)
{
	auto it = g_mock.paths.find(path);
	if (it == g_mock.paths.end())
		return nullptr;
	return &it->second;
}

void
mock_reset()
{
	g_mock.paths.clear();
	g_mock.users.clear();
	g_mock.groups.clear();
	g_mock.stdin_lines.clear();
	g_mock.stdin_pos = 0;
	g_mock.next_acl = 0x1000;
	ref_more_than_one = 0;
	P::more_than_one = 0;
}

void
reset_getopt()
{
	optind = 1;
	opterr = 0;
#ifdef __GLIBC__
	optreset = 1;
#endif
}

PathCfg &
add_path(const char *path)
{
	return g_mock.paths[path];
}

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

struct IoCap {
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
		if (dup2(pipefd[1], fd) < 0)
			return false;
		close(pipefd[1]);
		pipefd[1] = -1;
		return true;
	}

	std::vector<unsigned char> finish()
	{
		unsigned char buf[4096];
		ssize_t nr;

		fflush(stdout);
		fflush(stderr);
		if (pipefd[1] >= 0)
			close(pipefd[1]);
		while ((nr = read(pipefd[0], buf, sizeof(buf))) > 0)
			bytes.insert(bytes.end(), buf, buf + nr);
		close(pipefd[0]);
		dup2(saved, STDOUT_FILENO);
		close(saved);
		return bytes;
	}
};

struct IoCapErr {
	int saved;
	int pipefd[2];
	std::vector<unsigned char> bytes;

	bool begin()
	{
		saved = dup(STDERR_FILENO);
		if (saved < 0)
			return false;
		if (pipe(pipefd) != 0) {
			close(saved);
			return false;
		}
		if (dup2(pipefd[1], STDERR_FILENO) < 0)
			return false;
		close(pipefd[1]);
		pipefd[1] = -1;
		return true;
	}

	std::vector<unsigned char> finish()
	{
		unsigned char buf[4096];
		ssize_t nr;

		fflush(stderr);
		if (pipefd[1] >= 0)
			close(pipefd[1]);
		while ((nr = read(pipefd[0], buf, sizeof(buf))) > 0)
			bytes.insert(bytes.end(), buf, buf + nr);
		close(pipefd[0]);
		dup2(saved, STDERR_FILENO);
		close(saved);
		return bytes;
	}
};

bool
same_bytes(const std::vector<unsigned char> &a,
    const std::vector<unsigned char> &b)
{
	return a == b;
}

bool
same_cstr(const char *a, const char *b)
{
	if (a == nullptr || b == nullptr)
		return a == b;
	return std::strcmp(a, b) == 0;
}

} /* namespace */

extern "C" {

int
__real_stat(const char *path, struct stat *sb);

int
__wrap_stat(const char *path, struct stat *sb)
{
	PathCfg *cfg = find_path(path);
	if (cfg == nullptr || !cfg->present) {
		errno = ENOENT;
		return -1;
	}
	if (cfg->stat_errno != 0) {
		errno = cfg->stat_errno;
		return -1;
	}
	std::memset(sb, 0, sizeof(*sb));
	sb->st_uid = cfg->st_uid;
	sb->st_gid = cfg->st_gid;
	sb->st_mode = cfg->st_mode;
	return 0;
}

int
__real_lstat(const char *path, struct stat *sb);

int
__wrap_lstat(const char *path, struct stat *sb)
{
	return __wrap_stat(path, sb);
}

long
__real_pathconf(const char *path, int name);

long
__wrap_pathconf(const char *path, int name)
{
	PathCfg *cfg = find_path(path);
	if (cfg == nullptr || !cfg->present) {
		errno = ENOENT;
		return -1;
	}
	if (name != _PC_ACL_NFS4)
		return __real_pathconf(path, name);
	if (cfg->pathconf_ret < 0) {
		errno = cfg->pathconf_errno ? cfg->pathconf_errno : EINVAL;
		return -1;
	}
	errno = 0;
	return cfg->pathconf_ret;
}

long
__wrap_lpathconf(const char *path, int name)
{
	return __wrap_pathconf(path, name);
}

acl_t
__wrap_acl_get_file(const char *path, acl_type_t type)
{
	(void)type;
	PathCfg *cfg = find_path(path);
	if (cfg == nullptr || !cfg->present) {
		errno = ENOENT;
		return nullptr;
	}
	if (cfg->acl == 0) {
		errno = cfg->acl_errno;
		return nullptr;
	}
	errno = 0;
	return (acl_t)cfg->acl;
}

acl_t
__wrap_acl_get_link_np(const char *path, acl_type_t type)
{
	return __wrap_acl_get_file(path, type);
}

int
__wrap_acl_is_trivial_np(acl_t acl, int *trivial)
{
	for (const auto &kv : g_mock.paths) {
		if (kv.second.acl == (uintptr_t)acl) {
			if (kv.second.trivial_fail) {
				errno = EIO;
				return -1;
			}
			*trivial = kv.second.trivial;
			return 0;
		}
	}
	errno = EINVAL;
	return -1;
}

acl_t
__wrap_acl_from_mode_np(mode_t mode)
{
	(void)mode;
	uintptr_t h = g_mock.next_acl++;
	for (auto &kv : g_mock.paths) {
		if (kv.second.from_mode_fail)
			continue;
	}
	/* failure when any path requests it via global flag encoded in mode */
	if ((mode & 0x80000000u) != 0) {
		errno = ENOMEM;
		return nullptr;
	}
	errno = 0;
	return (acl_t)h;
}

char *
__wrap_acl_to_text_np(acl_t acl, ssize_t len, int flags)
{
	(void)len;
	for (const auto &kv : g_mock.paths) {
		if (kv.second.acl == (uintptr_t)acl) {
			if (kv.second.acl_text_fail) {
				errno = ENOMEM;
				return nullptr;
			}
			std::string out = kv.second.acl_text;
			out += "flags=" + std::to_string(flags) + "\n";
			char *p = (char *)std::malloc(out.size() + 1);
			if (p == nullptr) {
				errno = ENOMEM;
				return nullptr;
			}
			std::memcpy(p, out.c_str(), out.size() + 1);
			return p;
		}
	}
	/* synthetic acl from acl_from_mode_np */
	if (acl != nullptr) {
		std::string out = "synthetic\nflags=" + std::to_string(flags) + "\n";
		char *p = (char *)std::malloc(out.size() + 1);
		if (p == nullptr) {
			errno = ENOMEM;
			return nullptr;
		}
		std::memcpy(p, out.c_str(), out.size() + 1);
		return p;
	}
	errno = EINVAL;
	return nullptr;
}

int
__wrap_acl_free(void *obj)
{
	(void)obj;
	return 0;
}

struct passwd *
__real_getpwuid(uid_t uid);

struct passwd *
__wrap_getpwuid(uid_t uid)
{
	static struct passwd pw;
	static std::string name_store;
	auto it = g_mock.users.find(uid);
	if (it == g_mock.users.end())
		return nullptr;
	name_store = it->second;
	pw.pw_name = const_cast<char *>(name_store.c_str());
	return &pw;
}

struct group *
__real_getgrgid(gid_t gid);

struct group *
__wrap_getgrgid(gid_t gid)
{
	static struct group gr;
	static std::string name_store;
	auto it = g_mock.groups.find(gid);
	if (it == g_mock.groups.end())
		return nullptr;
	name_store = it->second;
	gr.gr_name = const_cast<char *>(name_store.c_str());
	return &gr;
}

char *
__real_fgets(char *buf, int size, FILE *stream);

char *
__wrap_fgets(char *buf, int size, FILE *stream)
{
	if (stream != stdin)
		return __real_fgets(buf, size, stream);
	if (g_mock.stdin_pos >= g_mock.stdin_lines.size())
		return nullptr;
	const std::string &line = g_mock.stdin_lines[g_mock.stdin_pos++];
	std::size_t n = line.size();
	if (n >= (std::size_t)size)
		n = (std::size_t)size - 1;
	std::memcpy(buf, line.c_str(), n);
	buf[n] = '\0';
	return buf;
}

} /* extern "C" */

namespace {

struct RunOut {
	int ret;
	std::vector<unsigned char> stdout_bytes;
	std::vector<unsigned char> stderr_bytes;
	int more_flag;
};

RunOut
run_ref_print_acl(char *path, acl_type_t type, int hflag, int iflag, int nflag,
    int qflag, int vflag, int sflag)
{
	RunOut out{};
	IoCap cap_out;
	IoCapErr cap_err;
	ref_more_than_one = P::more_than_one;
	if (!cap_out.begin(STDOUT_FILENO) || !cap_err.begin())
		return out;
	out.ret = ref_print_acl(path, type, hflag, iflag, nflag, qflag, vflag,
	    sflag);
	out.stdout_bytes = cap_out.finish();
	out.stderr_bytes = cap_err.finish();
	out.more_flag = ref_more_than_one;
	return out;
}

RunOut
run_port_print_acl(char *path, acl_type_t type, int hflag, int iflag, int nflag,
    int qflag, int vflag, int sflag)
{
	RunOut out{};
	IoCap cap_out;
	IoCapErr cap_err;
	P::more_than_one = ref_more_than_one;
	if (!cap_out.begin(STDOUT_FILENO) || !cap_err.begin())
		return out;
	out.ret = P::print_acl(path, type, hflag, iflag, nflag, qflag, vflag,
	    sflag);
	out.stdout_bytes = cap_out.finish();
	out.stderr_bytes = cap_err.finish();
	out.more_flag = P::more_than_one;
	return out;
}

bool
check_print_acl(Stat &st, const char *label, RunOut ref_o, RunOut port_o)
{
	if (ref_o.ret != port_o.ret) {
		fail(st, label);
		return false;
	}
	if (!same_bytes(ref_o.stdout_bytes, port_o.stdout_bytes)) {
		fail(st, label);
		return false;
	}
	if (!same_bytes(ref_o.stderr_bytes, port_o.stderr_bytes)) {
		fail(st, label);
		return false;
	}
	if (ref_o.more_flag != port_o.more_flag) {
		fail(st, label);
		return false;
	}
	return true;
}

void
test_print_acl_case(Stat &st, const char *path, acl_type_t type, int hflag,
    int iflag, int nflag, int qflag, int vflag, int sflag)
{
	st.cases++;
	std::vector<char> p(path, path + std::strlen(path) + 1);
	RunOut ref_o = run_ref_print_acl(p.data(), type, hflag, iflag, nflag,
	    qflag, vflag, sflag);
	RunOut port_o = run_port_print_acl(p.data(), type, hflag, iflag, nflag,
	    qflag, vflag, sflag);
	(void)check_print_acl(st, path, ref_o, port_o);
}

void
test_getuname_case(Stat &st, uid_t uid)
{
	st.cases++;
	char *ra = ref_getuname(uid);
	char *rb = P::getuname(uid);
	if (!same_cstr(ra, rb))
		fail(st, "string mismatch");
}

void
test_getgname_case(Stat &st, gid_t gid)
{
	st.cases++;
	char *ra = ref_getgname(gid);
	char *rb = P::getgname(gid);
	if (!same_cstr(ra, rb))
		fail(st, "string mismatch");
}

void
test_usage_case(Stat &st)
{
	st.cases++;
	IoCapErr cap_a, cap_b;
	if (!cap_a.begin() || !cap_b.begin()) {
		fail(st, "io setup");
		return;
	}
	ref_usage();
	auto sa = cap_a.finish();
	cap_b.begin();
	P::usage();
	auto sb = cap_b.finish();
	if (!same_bytes(sa, sb))
		fail(st, "stderr mismatch");
}

RunOut
run_ref_stdin(unsigned int type, int hflag, int iflag, int nflag, int qflag,
    int vflag, int sflag)
{
	RunOut out{};
	IoCap cap_out;
	IoCapErr cap_err;
	ref_more_than_one = P::more_than_one;
	if (!cap_out.begin(STDOUT_FILENO) || !cap_err.begin())
		return out;
	out.ret = ref_print_acl_from_stdin(type, hflag, iflag, nflag, qflag,
	    vflag, sflag);
	out.stdout_bytes = cap_out.finish();
	out.stderr_bytes = cap_err.finish();
	out.more_flag = ref_more_than_one;
	return out;
}

RunOut
run_port_stdin(unsigned int type, int hflag, int iflag, int nflag, int qflag,
    int vflag, int sflag)
{
	RunOut out{};
	IoCap cap_out;
	IoCapErr cap_err;
	P::more_than_one = ref_more_than_one;
	if (!cap_out.begin(STDOUT_FILENO) || !cap_err.begin())
		return out;
	out.ret = P::print_acl_from_stdin(type, hflag, iflag, nflag, qflag,
	    vflag, sflag);
	out.stdout_bytes = cap_out.finish();
	out.stderr_bytes = cap_err.finish();
	out.more_flag = P::more_than_one;
	return out;
}

void
test_stdin_case(Stat &st, const std::vector<std::string> &lines,
    unsigned int type, int hflag, int iflag, int nflag, int qflag, int vflag,
    int sflag)
{
	st.cases++;
	g_mock.stdin_lines = lines;
	g_mock.stdin_pos = 0;
	RunOut ref_o = run_ref_stdin(type, hflag, iflag, nflag, qflag, vflag,
	    sflag);
	g_mock.stdin_lines = lines;
	g_mock.stdin_pos = 0;
	RunOut port_o = run_port_stdin(type, hflag, iflag, nflag, qflag, vflag,
	    sflag);
	(void)check_print_acl(st, "stdin", ref_o, port_o);
}

struct MainOut {
	int ret;
	std::vector<unsigned char> stdout_bytes;
	std::vector<unsigned char> stderr_bytes;
};

MainOut
run_ref_main(std::vector<std::string> args)
{
	MainOut out{};
	std::vector<char *> argv;
	argv.push_back(const_cast<char *>("getfacl"));
	for (auto &a : args)
		argv.push_back(a.data());
	argv.push_back(nullptr);
	reset_getopt();
	IoCap cap_out;
	IoCapErr cap_err;
	ref_more_than_one = 0;
	P::more_than_one = 0;
	if (!cap_out.begin(STDOUT_FILENO) || !cap_err.begin())
		return out;
	out.ret = ref_main((int)argv.size() - 1, argv.data());
	out.stdout_bytes = cap_out.finish();
	out.stderr_bytes = cap_err.finish();
	return out;
}

MainOut
run_port_main(std::vector<std::string> args)
{
	MainOut out{};
	std::vector<char *> argv;
	argv.push_back(const_cast<char *>("getfacl"));
	for (auto &a : args)
		argv.push_back(a.data());
	argv.push_back(nullptr);
	reset_getopt();
	IoCap cap_out;
	IoCapErr cap_err;
	ref_more_than_one = 0;
	P::more_than_one = 0;
	if (!cap_out.begin(STDOUT_FILENO) || !cap_err.begin())
		return out;
	out.ret = P::main((int)argv.size() - 1, argv.data());
	out.stdout_bytes = cap_out.finish();
	out.stderr_bytes = cap_err.finish();
	return out;
}

void
test_main_case(Stat &st, const std::vector<std::string> &args)
{
	st.cases++;
	MainOut ra = run_ref_main(args);
	MainOut rb = run_port_main(args);
	if (ra.ret != rb.ret)
		fail(st, "exit code");
	else if (!same_bytes(ra.stdout_bytes, rb.stdout_bytes))
		fail(st, "stdout");
	else if (!same_bytes(ra.stderr_bytes, rb.stderr_bytes))
		fail(st, "stderr");
}

void
setup_basic_path(const char *name, PathCfg cfg)
{
	add_path(name) = cfg;
}

void
handwritten_tests()
{
	mock_reset();

	/* usage */
	test_usage_case(st_usage);

	/* getuname / getgname edge cases */
	g_mock.users[0] = "root";
	g_mock.users[65534] = "nobody";
	g_mock.groups[0] = "wheel";
	g_mock.groups[65534] = "nogroup";
	test_getuname_case(st_getuname, 0);
	test_getuname_case(st_getuname, 65534);
	test_getuname_case(st_getuname, 4242);
	test_getgname_case(st_getgname, 0);
	test_getgname_case(st_getgname, 65534);
	test_getgname_case(st_getgname, 5151);

	/* stat failure */
	mock_reset();
	setup_basic_path("/bad", PathCfg{});
	g_mock.paths["/bad"].present = true;
	g_mock.paths["/bad"].stat_errno = EACCES;
	test_print_acl_case(st_print_acl, "/bad", ACL_TYPE_ACCESS, 0, 0, 0, 0,
	    0, 0);

	/* pathconf NFS4 + default type */
	mock_reset();
	setup_basic_path("/nfsdef", PathCfg{});
	g_mock.paths["/nfsdef"].pathconf_ret = 1;
	test_print_acl_case(st_print_acl, "/nfsdef", ACL_TYPE_DEFAULT, 0, 0, 0,
	    0, 0, 0);

	/* pathconf failure not EINVAL */
	mock_reset();
	setup_basic_path("/pcfail", PathCfg{});
	g_mock.paths["/pcfail"].pathconf_ret = -1;
	g_mock.paths["/pcfail"].pathconf_errno = EIO;
	test_print_acl_case(st_print_acl, "/pcfail", ACL_TYPE_ACCESS, 0, 0, 0,
	    0, 0, 0);

	/* pathconf EINVAL continues */
	mock_reset();
	setup_basic_path("/pcval", PathCfg{});
	g_mock.paths["/pcval"].pathconf_ret = -1;
	g_mock.paths["/pcval"].pathconf_errno = EINVAL;
	g_mock.paths["/pcval"].acl = 0x2001;
	test_print_acl_case(st_print_acl, "/pcval", ACL_TYPE_ACCESS, 0, 0, 0,
	    0, 0, 0);

	/* acl missing non-EOPNOTSUPP */
	mock_reset();
	setup_basic_path("/aclerr", PathCfg{});
	g_mock.paths["/aclerr"].acl = 0;
	g_mock.paths["/aclerr"].acl_errno = EIO;
	test_print_acl_case(st_print_acl, "/aclerr", ACL_TYPE_ACCESS, 0, 0, 0,
	    0, 0, 0);

	/* EOPNOTSUPP synthesize from mode */
	mock_reset();
	setup_basic_path("/eop", PathCfg{});
	g_mock.paths["/eop"].acl = 0;
	g_mock.paths["/eop"].acl_errno = EOPNOTSUPP;
	g_mock.paths["/eop"].st_mode = 0755;
	test_print_acl_case(st_print_acl, "/eop", ACL_TYPE_ACCESS, 0, 0, 0, 0,
	    0, 0);

	/* EOPNOTSUPP default type early return */
	mock_reset();
	setup_basic_path("/eopdef", PathCfg{});
	g_mock.paths["/eopdef"].acl = 0;
	g_mock.paths["/eopdef"].acl_errno = EOPNOTSUPP;
	test_print_acl_case(st_print_acl, "/eopdef", ACL_TYPE_DEFAULT, 0, 0, 0,
	    0, 0, 0);

	/* from_mode failure */
	mock_reset();
	setup_basic_path("/fmfail", PathCfg{});
	g_mock.paths["/fmfail"].acl = 0;
	g_mock.paths["/fmfail"].acl_errno = EOPNOTSUPP;
	g_mock.paths["/fmfail"].st_mode = 0x80000000u | 0644;
	test_print_acl_case(st_print_acl, "/fmfail", ACL_TYPE_ACCESS, 0, 0, 0,
	    0, 0, 0);

	/* sflag no acl */
	mock_reset();
	setup_basic_path("/sno", PathCfg{});
	g_mock.paths["/sno"].acl = 0;
	g_mock.paths["/sno"].acl_errno = EOPNOTSUPP;
	test_print_acl_case(st_print_acl, "/sno", ACL_TYPE_ACCESS, 0, 0, 0, 0,
	    0, 1);

	/* sflag trivial */
	mock_reset();
	setup_basic_path("/striv", PathCfg{});
	g_mock.paths["/striv"].acl = 0x3001;
	g_mock.paths["/striv"].trivial = 1;
	test_print_acl_case(st_print_acl, "/striv", ACL_TYPE_ACCESS, 0, 0, 0,
	    0, 0, 1);

	/* sflag trivial check failure continues */
	mock_reset();
	setup_basic_path("/strfail", PathCfg{});
	g_mock.paths["/strfail"].acl = 0x3002;
	g_mock.paths["/strfail"].trivial_fail = 1;
	test_print_acl_case(st_print_acl, "/strfail", ACL_TYPE_ACCESS, 0, 0, 0,
	    0, 0, 1);

	/* more_than_one separator */
	mock_reset();
	setup_basic_path("/a", PathCfg{});
	g_mock.paths["/a"].acl = 0x4001;
	setup_basic_path("/b", PathCfg{});
	g_mock.paths["/b"].acl = 0x4002;
	ref_more_than_one = 0;
	P::more_than_one = 0;
	test_print_acl_case(st_print_acl, "/a", ACL_TYPE_ACCESS, 0, 0, 0, 0,
	    0, 0);
	test_print_acl_case(st_print_acl, "/b", ACL_TYPE_ACCESS, 0, 0, 0, 0,
	    0, 0);

	/* qflag, iflag, nflag, vflag */
	mock_reset();
	setup_basic_path("/flags", PathCfg{});
	g_mock.paths["/flags"].acl = 0x5001;
	test_print_acl_case(st_print_acl, "/flags", ACL_TYPE_ACCESS, 0, 1, 1,
	    1, 1, 0);

	/* hflag lstat path */
	mock_reset();
	setup_basic_path("/link", PathCfg{});
	g_mock.paths["/link"].acl = 0x6001;
	test_print_acl_case(st_print_acl, "/link", ACL_TYPE_ACCESS, 1, 0, 0, 0,
	    0, 0);

	/* acl_to_text failure */
	mock_reset();
	setup_basic_path("/txtfail", PathCfg{});
	g_mock.paths["/txtfail"].acl = 0x7001;
	g_mock.paths["/txtfail"].acl_text_fail = 1;
	test_print_acl_case(st_print_acl, "/txtfail", ACL_TYPE_ACCESS, 0, 0, 0,
	    0, 0, 0);

	/* NFS4 conversion success */
	mock_reset();
	setup_basic_path("/nfs4", PathCfg{});
	g_mock.paths["/nfs4"].pathconf_ret = 2;
	g_mock.paths["/nfs4"].acl = 0x8001;
	test_print_acl_case(st_print_acl, "/nfs4", ACL_TYPE_ACCESS, 0, 0, 0,
	    0, 0, 0);

	/* print_acl_from_stdin */
	mock_reset();
	setup_basic_path("/stdin1", PathCfg{});
	g_mock.paths["/stdin1"].acl = 0x9001;
	setup_basic_path("/stdin2", PathCfg{});
	g_mock.paths["/stdin2"].stat_errno = EACCES;
	test_stdin_case(st_print_acl_from_stdin,
	    { "/stdin1\n", "/stdin2\n" }, ACL_TYPE_ACCESS, 0, 0, 0, 0, 0, 0);

	/* empty stdin */
	mock_reset();
	test_stdin_case(st_print_acl_from_stdin, {}, ACL_TYPE_ACCESS, 0, 0, 0,
	    0, 0, 0);

	/* main: bad option */
	mock_reset();
	test_main_case(st_main, { "-z" });

	/* main: file arg */
	mock_reset();
	setup_basic_path("/m1", PathCfg{});
	g_mock.paths["/m1"].acl = 0xa001;
	test_main_case(st_main, { "/m1" });

	/* main: two files */
	mock_reset();
	setup_basic_path("/m2", PathCfg{});
	g_mock.paths["/m2"].acl = 0xa002;
	setup_basic_path("/m3", PathCfg{});
	g_mock.paths["/m3"].acl = 0xa003;
	test_main_case(st_main, { "/m2", "/m3" });

	/* main: -d -q -n -i -v -s -h */
	mock_reset();
	setup_basic_path("/mall", PathCfg{});
	g_mock.paths["/mall"].acl = 0xa004;
	test_main_case(st_main,
	    { "-d", "-q", "-n", "-i", "-v", "-s", "-h", "/mall" });

	/* main: stdin via no args */
	mock_reset();
	setup_basic_path("/fromstdin", PathCfg{});
	g_mock.paths["/fromstdin"].acl = 0xb001;
	g_mock.stdin_lines = { "/fromstdin\n" };
	g_mock.stdin_pos = 0;
	test_main_case(st_main, {});

	/* main: dash arg */
	mock_reset();
	setup_basic_path("/dash", PathCfg{});
	g_mock.paths["/dash"].acl = 0xb002;
	g_mock.stdin_lines = { "/dash\n" };
	g_mock.stdin_pos = 0;
	test_main_case(st_main, { "-" });
}

void
random_sweep()
{
	for (long i = 0; i < SWEEP; ++i) {
		mock_reset();
		uid_t uid = (uid_t)rng.bits(0, 200000);
		gid_t gid = (gid_t)rng.bits(0, 200000);
		if (rng.coin())
			g_mock.users[uid] = "u" + std::to_string(uid);
		if (rng.coin())
			g_mock.groups[gid] = "g" + std::to_string(gid);
		test_getuname_case(st_getuname, uid);
		test_getgname_case(st_getgname, gid);

		char path[64];
		std::snprintf(path, sizeof(path), "/p%ld", i);
		PathCfg cfg;
		cfg.present = true;
		cfg.st_uid = uid;
		cfg.st_gid = gid;
		cfg.st_mode = (mode_t)rng.bits(0, 07777);
		int pc = rng.bits(-2, 2);
		if (pc < 0) {
			cfg.pathconf_ret = -1;
			cfg.pathconf_errno = rng.coin() ? EINVAL : EIO;
		} else
			cfg.pathconf_ret = pc;
		int acl_kind = rng.bits(0, 4);
		if (acl_kind == 0) {
			cfg.acl = 0;
			cfg.acl_errno = EOPNOTSUPP;
		} else if (acl_kind == 1) {
			cfg.acl = 0;
			cfg.acl_errno = EIO;
		} else {
			cfg.acl = (uintptr_t)(0x10000 + (unsigned long)i);
		}
		cfg.trivial = rng.coin() ? 1 : 0;
		cfg.trivial_fail = rng.bits(0, 10) == 0 ? 1 : 0;
		cfg.acl_text_fail = rng.bits(0, 20) == 0 ? 1 : 0;
		if (rng.bits(0, 50) == 0)
			cfg.stat_errno = EACCES;
		add_path(path) = cfg;

		acl_type_t type = rng.coin() ? ACL_TYPE_ACCESS : ACL_TYPE_DEFAULT;
		int hflag = rng.coin() ? 1 : 0;
		int iflag = rng.coin() ? 1 : 0;
		int nflag = rng.coin() ? 1 : 0;
		int qflag = rng.coin() ? 1 : 0;
		int vflag = rng.coin() ? 1 : 0;
		int sflag = rng.coin() ? 1 : 0;
		test_print_acl_case(st_print_acl, path, type, hflag, iflag, nflag,
		    qflag, vflag, sflag);

		if (i % 500 == 0) {
			mock_reset();
			setup_basic_path("/rs", PathCfg{});
			g_mock.paths["/rs"].acl = 0xc000 + (uintptr_t)i;
			std::string line = std::string("/rs") +
			    (rng.coin() ? "\n" : "");
			test_stdin_case(st_print_acl_from_stdin, { line },
			    type, hflag, iflag, nflag, qflag, vflag, sflag);
		}

		if (i % 1000 == 0) {
			mock_reset();
			setup_basic_path("/rm", PathCfg{});
			g_mock.paths["/rm"].acl = 0xd000 + (uintptr_t)i;
			std::vector<std::string> args;
			if (rng.coin())
				args.push_back("-d");
			if (rng.coin())
				args.push_back("-q");
			args.push_back("/rm");
			test_main_case(st_main, args);
		}
	}
}

} /* namespace */

int
main()
{
	handwritten_tests();
	random_sweep();

	Stat stats[] = {
		st_usage,
		st_getuname,
		st_getgname,
		st_print_acl,
		st_print_acl_from_stdin,
		st_main,
	};

	std::printf("PBSD batch b0182 differential test\n");
	std::printf("(oracle.c is the specification)\n\n");
	std::printf("%-32s %12s %12s %10s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");

	long total_cases = 0;
	long total_fails = 0;
	for (const auto &s : stats) {
		total_cases += s.cases;
		total_fails += s.fails;
		std::printf("%-32s %12ld %12ld %10s\n", s.name, s.cases,
		    s.fails, s.fails == 0 ? "PASS" : "FAIL");
	}
	std::printf("%-32s %12s %12s %10s\n",
	    "--------------------------------", "------------",
	    "------------", "----------");
	std::printf("%-32s %12ld %12ld %10s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0 ? "PASS" : "FAIL");
	std::printf("\n%zu function(s) tested, %ld case(s) run, "
	    "%ld failure(s).\n", sizeof(stats) / sizeof(stats[0]),
	    total_cases, total_fails);

	return total_fails == 0 ? 0 : 1;
}
