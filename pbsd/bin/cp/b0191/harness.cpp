/*
 * harness.cpp -- differential test for PBSD batch b0191 (cp/utils.c, cp/cp.c).
 */

#define _GNU_SOURCE

#ifndef SIGINFO
#define SIGINFO SIGUSR1
#endif

#include <cerrno>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <fts.h>
#include <getopt.h>
#include <limits.h>
#include <sysexits.h>
#include <map>
#include <memory>
#include <signal.h>
#include <string>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#if defined(__linux__)
#include <bsd/string.h>
#endif

import pbsd.bin.cp.b0191;

namespace P = pbsd::bin_cp::b0191;

typedef void *acl_t;
typedef unsigned int acl_type_t;

extern "C" {
enum ref_op { RFILE_TO_FILE, RFILE_TO_DIR, RDIR_TO_DNE };

typedef struct {
	int dir;
	char base[PATH_MAX + 1];
	char *end;
	char path[PATH_MAX];
} PATH_T;

extern PATH_T ref_to;
extern bool ref_Nflag, ref_fflag, ref_iflag, ref_lflag, ref_nflag, ref_pflag,
    ref_sflag, ref_vflag;
extern volatile sig_atomic_t ref_info;

ssize_t ref_copy_fallback(int from_fd, int to_fd);
int ref_copy_file(const FTSENT *entp, bool dne, bool beneath);
int ref_copy_link(const FTSENT *p, bool dne, bool beneath);
int ref_copy_fifo(struct stat *from_stat, bool dne, bool beneath);
int ref_copy_special(struct stat *from_stat, bool dne, bool beneath);
int ref_setfile(struct stat *fs, int fd, bool beneath);
int ref_preserve_fd_acls(int source_fd, int dest_fd);
int ref_preserve_dir_acls(const char *source_dir, const char *dest_dir);
void ref_usage(void);
int ref_ftscmp(const FTSENT *const *a, const FTSENT *const *b);
void ref_siginfo(int sig);
int ref_ref_copy(char *argv[], enum ref_op type, int fts_options,
    struct stat *root_stat);
int ref_main(int argc, char *argv[]);

void __real_exit(int status);
long __real_sysconf(int name);
void *__real_malloc(size_t size);
int __real_fchflags(int, unsigned long);
int __real_chflagsat(int, const char *, unsigned long, int);
acl_t __real_acl_get_fd_np(int, acl_type_t);
int __real_acl_is_trivial_np(acl_t, int *);
int __real_acl_set_fd_np(int, acl_t, acl_type_t);
int __real_acl_free(acl_t);
}

#if defined(__linux__)
extern "C" int
__real_fchflags(int fd, unsigned long flags)
{
	(void)fd;
	(void)flags;
	errno = EOPNOTSUPP;
	return -1;
}

extern "C" int
__real_chflagsat(int dirfd, const char *path, unsigned long flags, int atflag)
{
	(void)dirfd;
	(void)path;
	(void)flags;
	(void)atflag;
	errno = EOPNOTSUPP;
	return -1;
}

extern "C" acl_t
__real_acl_get_fd_np(int fd, acl_type_t type)
{
	(void)fd;
	(void)type;
	errno = EOPNOTSUPP;
	return nullptr;
}

extern "C" int
__real_acl_is_trivial_np(acl_t acl, int *trivial)
{
	(void)acl;
	(void)trivial;
	errno = EINVAL;
	return -1;
}

extern "C" int
__real_acl_set_fd_np(int fd, acl_t acl, acl_type_t type)
{
	(void)fd;
	(void)acl;
	(void)type;
	errno = EOPNOTSUPP;
	return -1;
}

extern "C" int
__real_acl_free(acl_t acl)
{
	(void)acl;
	return 0;
}
#endif

#define SWEEP 200000L
#define MAX_SHOW 8
#define GUARD 0x7f
#define PHYSPAGES_THRESHOLD (32 * 1024)

namespace {

bool g_test_active = false;
bool g_test_child = false;

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

Rng rng(0x00b0191faceULL);

Stat st_copy_fallback = { "copy_fallback", 0, 0, 0 };
Stat st_copy_file = { "copy_file", 0, 0, 0 };
Stat st_copy_link = { "copy_link", 0, 0, 0 };
Stat st_copy_fifo = { "copy_fifo", 0, 0, 0 };
Stat st_copy_special = { "copy_special", 0, 0, 0 };
Stat st_setfile = { "setfile", 0, 0, 0 };
Stat st_preserve_fd_acls = { "preserve_fd_acls", 0, 0, 0 };
Stat st_preserve_dir_acls = { "preserve_dir_acls", 0, 0, 0 };
Stat st_usage = { "usage", 0, 0, 0 };
Stat st_ftscmp = { "ftscmp", 0, 0, 0 };
Stat st_siginfo = { "siginfo", 0, 0, 0 };
Stat st_copy = { "copy", 0, 0, 0 };
Stat st_main = { "main", 0, 0, 0 };

#ifndef ACL_TYPE_NFS4
#define ACL_TYPE_NFS4 0x00000004
#endif
#ifndef ACL_TYPE_ACCESS
#define ACL_TYPE_ACCESS 0x00000002
#endif
#ifndef _PC_ACL_NFS4
#define _PC_ACL_NFS4 64
#endif
#ifndef _PC_ACL_EXTENDED
#define _PC_ACL_EXTENDED 65
#endif

struct PathMock {
	bool exists = false;
	struct stat st {};
	std::string readlink_target;
	std::vector<unsigned char> data;
	int open_errno = 0;
	int stat_errno = 0;
	int unlink_errno = 0;
	int readlink_errno = 0;
};

struct FdMock {
	bool active = false;
	std::string path;
	bool is_dir = false;
	off_t pos = 0;
	struct stat st {};
	std::vector<unsigned char> data;
	int fpathconf_nfs4 = 0;
	int fpathconf_nfs4_errno = EINVAL;
	int fpathconf_ext = 0;
	int fpathconf_ext_errno = EINVAL;
	uintptr_t acl_handle = 0;
	int acl_get_errno = 0;
	int acl_trivial = 0;
	int acl_trivial_fail = 0;
	int acl_set_fail = 0;
};

struct IoOp {
	int fd = -1;
	int op = 0;
	ssize_t ret = 0;
	int err = 0;
	ssize_t partial = 0;
};

struct MockCtrl {
	std::map<std::string, PathMock> paths;
	std::map<int, FdMock> fds;
	std::vector<IoOp> ioq;
	size_t ioq_pos = 0;
	int next_fd = 100;
	long sysconf_pages = 65536;
	bool sysconf_fail = false;
	int getchar_val = 'n';
	int getchar_errno = 0;
	bool copy_file_range_fail = false;
	int copy_file_range_errno = EINVAL;
	std::map<std::string, struct stat> fstatat_results;
	std::map<std::string, int> fstatat_errno;
	std::map<std::string, int> futimens_fail;
	std::map<std::string, int> fchown_fail;
	std::map<std::string, int> fchmod_fail;
	std::map<std::string, unsigned long> chflags_val;
	std::map<std::string, int> chflags_fail;
	std::map<int, struct stat> fd_fstat;
	std::map<int, int> fd_fstat_fail;
	uintptr_t next_acl = 0x2000;
};

MockCtrl g_mock;

struct FtsNode {
	std::vector<char> blob;
	FTSENT *ent = nullptr;
	struct stat statbuf {};
	std::string accpath;
	std::string path;
};

struct FtsState {
	FTS pub {};
	std::vector<std::unique_ptr<FtsNode>> nodes;
	size_t idx = 0;
	bool skip_pending = false;
	int skip_level = 0;
};

std::unique_ptr<FtsState> g_fts_script;

extern "C" void
__wrap_exit(int status)
{
	if (g_test_child || g_test_active)
		::_exit(status);
	__real_exit(status);
}

static void
mock_reset()
{
	g_mock = MockCtrl{};
	g_fts_script.reset();
	g_mock.next_fd = 100;
}

static void
reset_globals()
{
	ref_to.dir = AT_FDCWD;
	ref_to.end = ref_to.path;
	ref_to.path[0] = '\0';
	ref_to.base[0] = '\0';

	P::to.dir = AT_FDCWD;
	P::to.end = P::to.path;
	P::to.path[0] = '\0';
	P::to.base[0] = '\0';

	ref_Nflag = ref_fflag = ref_iflag = ref_lflag = ref_nflag = false;
	ref_pflag = ref_sflag = ref_vflag = false;
	ref_info = 0;

	P::Nflag = P::fflag = P::iflag = P::lflag = P::nflag = false;
	P::pflag = P::sflag = P::vflag = false;
	P::Hflag = P::Lflag = P::Pflag = P::Rflag = false;
	P::rflag = P::Sflag = false;
	P::info = 0;

	optind = 1;
	opterr = 0;
#if defined(__GLIBC__) && defined(optreset)
	optreset = 1;
#endif
}

static bool
fail(Stat &st, const char *what)
{
	st.fails++;
	if (st.shown < MAX_SHOW) {
		st.shown++;
		std::printf("  FAIL %s: %s\n", st.name, what);
	}
	return false;
}

static int
silence_stderr()
{
	int saved = dup(STDERR_FILENO);
	int devnull = open("/dev/null", O_WRONLY);
	if (devnull >= 0) {
		dup2(devnull, STDERR_FILENO);
		close(devnull);
	}
	return saved;
}

static void
restore_stderr(int saved)
{
	if (saved >= 0) {
		dup2(saved, STDERR_FILENO);
		close(saved);
	}
}

static PathMock &
path_mock(const std::string &p)
{
	return g_mock.paths[p];
}

static int
alloc_fd(const std::string &path, bool is_dir)
{
	int fd = g_mock.next_fd++;
	FdMock &fm = g_mock.fds[fd];
	fm.active = true;
	fm.path = path;
	fm.is_dir = is_dir;
	fm.pos = 0;
	auto it = g_mock.paths.find(path);
	if (it != g_mock.paths.end()) {
		fm.st = it->second.st;
		fm.data = it->second.data;
	} else {
		std::memset(&fm.st, 0, sizeof(fm.st));
		fm.st.st_mode = is_dir ? (S_IFDIR | 0755) : (S_IFREG | 0644);
	}
	return fd;
}

static FdMock *
fd_mock(int fd)
{
	auto it = g_mock.fds.find(fd);
	if (it == g_mock.fds.end() || !it->second.active)
		return nullptr;
	return &it->second;
}

static std::string
join_path(int dirfd, const char *path)
{
	if (dirfd == AT_FDCWD || path[0] == '/')
		return std::string(path);
	FdMock *d = fd_mock(dirfd);
	if (d == nullptr)
		return std::string(path);
	if (d->path.empty())
		return std::string(path);
	if (d->path.back() == '/')
		return d->path + path;
	return d->path + "/" + path;
}

static IoOp *
next_io()
{
	if (g_mock.ioq_pos >= g_mock.ioq.size())
		return nullptr;
	return &g_mock.ioq[g_mock.ioq_pos++];
}

extern "C" long
__wrap_sysconf(int name)
{
	if (g_test_active && name == _SC_PHYS_PAGES) {
		if (g_mock.sysconf_fail) {
			errno = EINVAL;
			return -1;
		}
		return g_mock.sysconf_pages;
	}
	return __real_sysconf(name);
}

extern "C" void *
__wrap_malloc(size_t size)
{
	return __real_malloc(size);
}

extern "C" ssize_t
__wrap_read(int fd, void *buf, size_t nbytes)
{
	if (!g_test_active)
		return read(fd, buf, nbytes);

	IoOp *op = next_io();
	if (op != nullptr && (op->fd < 0 || op->fd == fd) && op->op == 1) {
		if (op->ret < 0) {
			errno = op->err ? op->err : EIO;
			return -1;
		}
		FdMock *fm = fd_mock(fd);
		if (fm == nullptr) {
			errno = EBADF;
			return -1;
		}
		size_t avail = fm->data.size() > (size_t)fm->pos ?
		    fm->data.size() - (size_t)fm->pos : 0;
		size_t n = op->partial > 0 ? (size_t)op->partial :
		    (nbytes < avail ? nbytes : avail);
		if (n > 0) {
			std::memcpy(buf, fm->data.data() + fm->pos, n);
			fm->pos += (off_t)n;
		}
		return (ssize_t)n;
	}

	FdMock *fm = fd_mock(fd);
	if (fm == nullptr) {
		errno = EBADF;
		return -1;
	}
	size_t avail = fm->data.size() > (size_t)fm->pos ?
	    fm->data.size() - (size_t)fm->pos : 0;
	size_t n = nbytes < avail ? nbytes : avail;
	if (n > 0) {
		std::memcpy(buf, fm->data.data() + fm->pos, n);
		fm->pos += (off_t)n;
	}
	return (ssize_t)n;
}

extern "C" ssize_t
__wrap_write(int fd, const void *buf, size_t nbytes)
{
	if (!g_test_active)
		return write(fd, buf, nbytes);

	IoOp *op = next_io();
	if (op != nullptr && (op->fd < 0 || op->fd == fd) && op->op == 2) {
		if (op->ret < 0) {
			errno = op->err ? op->err : EIO;
			return -1;
		}
		FdMock *fm = fd_mock(fd);
		if (fm == nullptr) {
			errno = EBADF;
			return -1;
		}
		size_t n = op->partial > 0 ? (size_t)op->partial : nbytes;
		if (fm->data.size() < fm->pos + (off_t)n)
			fm->data.resize((size_t)fm->pos + n);
		std::memcpy(fm->data.data() + fm->pos, buf, n);
		fm->pos += (off_t)n;
		return (ssize_t)n;
	}

	FdMock *fm = fd_mock(fd);
	if (fm == nullptr) {
		errno = EBADF;
		return -1;
	}
	if (fm->data.size() < fm->pos + (off_t)nbytes)
		fm->data.resize((size_t)fm->pos + nbytes);
	std::memcpy(fm->data.data() + fm->pos, buf, nbytes);
	fm->pos += (off_t)nbytes;
	return (ssize_t)nbytes;
}

extern "C" ssize_t
__wrap_copy_file_range(int infd, off_t *off_in, int outfd, off_t *off_out,
    size_t len, unsigned int flags)
{
	(void)off_in;
	(void)off_out;
	(void)len;
	(void)flags;

	if (!g_test_active) {
		return copy_file_range(infd, off_in, outfd, off_out, len, flags);
	}

	IoOp *op = next_io();
	if (op != nullptr && op->op == 3 && op->ret < 0) {
		errno = op->err ? op->err : EINVAL;
		return -1;
	}

	if (g_mock.copy_file_range_fail) {
		errno = g_mock.copy_file_range_errno;
		return -1;
	}
	errno = EINVAL;
	return -1;
}

extern "C" int
__wrap_close(int fd)
{
	if (!g_test_active)
		return close(fd);
	auto it = g_mock.fds.find(fd);
	if (it != g_mock.fds.end())
		it->second.active = false;
	return 0;
}

extern "C" int
__wrap_open(const char *path, int flags, ...)
{
	if (!g_test_active) {
		va_list ap;
		va_start(ap, flags);
		mode_t mode = (mode_t)va_arg(ap, int);
		va_end(ap);
		return open(path, flags, mode);
	}

	(void)flags;
	PathMock &pm = path_mock(path);
	if (!pm.exists) {
		errno = pm.open_errno ? pm.open_errno : ENOENT;
		return -1;
	}
	return alloc_fd(path, S_ISDIR(pm.st.st_mode));
}

extern "C" int
__wrap_openat(int dirfd, const char *path, int flags, ...)
{
	if (!g_test_active) {
		va_list ap;
		va_start(ap, flags);
		mode_t mode = (mode_t)va_arg(ap, int);
		va_end(ap);
		return openat(dirfd, path, flags, mode);
	}

	(void)flags;
	std::string full = join_path(dirfd, path);
	PathMock &pm = path_mock(full);
	if (!pm.exists && (flags & O_CREAT)) {
		pm.exists = true;
		pm.st.st_mode = S_IFREG | 0644;
	}
	if (!pm.exists) {
		errno = pm.open_errno ? pm.open_errno : ENOENT;
		return -1;
	}
	return alloc_fd(full, S_ISDIR(pm.st.st_mode));
}

extern "C" int
__wrap_fstat(int fd, struct stat *sb)
{
	if (!g_test_active)
		return fstat(fd, sb);

	auto fail_it = g_mock.fd_fstat_fail.find(fd);
	if (fail_it != g_mock.fd_fstat_fail.end()) {
		errno = fail_it->second;
		return -1;
	}
	auto st_it = g_mock.fd_fstat.find(fd);
	if (st_it != g_mock.fd_fstat.end()) {
		*sb = st_it->second;
		return 0;
	}
	FdMock *fm = fd_mock(fd);
	if (fm == nullptr) {
		errno = EBADF;
		return -1;
	}
	*sb = fm->st;
	return 0;
}

extern "C" int
__wrap_fstatat(int dirfd, const char *path, struct stat *sb, int atflags)
{
	(void)atflags;
	if (!g_test_active)
		return fstatat(dirfd, path, sb, atflags);

	std::string full = join_path(dirfd, path);
	auto eit = g_mock.fstatat_errno.find(full);
	if (eit != g_mock.fstatat_errno.end()) {
		errno = eit->second;
		return -1;
	}
	auto sit = g_mock.fstatat_results.find(full);
	if (sit != g_mock.fstatat_results.end()) {
		*sb = sit->second;
		return 0;
	}
	PathMock &pm = path_mock(full);
	if (!pm.exists) {
		errno = pm.stat_errno ? pm.stat_errno : ENOENT;
		return -1;
	}
	*sb = pm.st;
	return 0;
}

extern "C" int
__wrap_stat(const char *path, struct stat *sb)
{
	if (!g_test_active)
		return stat(path, sb);
	PathMock &pm = path_mock(path);
	if (!pm.exists) {
		errno = pm.stat_errno ? pm.stat_errno : ENOENT;
		return -1;
	}
	*sb = pm.st;
	return 0;
}

extern "C" int
__wrap_lstat(const char *path, struct stat *sb)
{
	return __wrap_stat(path, sb);
}

extern "C" ssize_t
__wrap_readlink(const char *path, char *buf, size_t bufsiz)
{
	if (!g_test_active)
		return readlink(path, buf, bufsiz);

	PathMock &pm = path_mock(path);
	if (!pm.exists) {
		errno = pm.readlink_errno ? pm.readlink_errno : ENOENT;
		return -1;
	}
	if (pm.readlink_target.empty()) {
		errno = EINVAL;
		return -1;
	}
	size_t n = pm.readlink_target.size();
	if (n >= bufsiz) {
		errno = ENAMETOOLONG;
		return -1;
	}
	std::memcpy(buf, pm.readlink_target.data(), n);
	return (ssize_t)n;
}

extern "C" int
__wrap_unlinkat(int dirfd, const char *path, int atflags)
{
	(void)atflags;
	if (!g_test_active)
		return unlinkat(dirfd, path, atflags);

	std::string full = join_path(dirfd, path);
	PathMock &pm = path_mock(full);
	if (!pm.exists) {
		errno = pm.unlink_errno ? pm.unlink_errno : ENOENT;
		return -1;
	}
	pm.exists = false;
	return 0;
}

extern "C" int
__wrap_linkat(int olddirfd, const char *oldpath, int newdirfd,
    const char *newpath, int flags)
{
	(void)olddirfd;
	(void)oldpath;
	(void)newdirfd;
	(void)flags;
	if (!g_test_active)
		return linkat(olddirfd, oldpath, newdirfd, newpath, flags);

	std::string dst = join_path(newdirfd, newpath);
	path_mock(dst).exists = true;
	path_mock(dst).st.st_mode = S_IFREG | 0644;
	return 0;
}

extern "C" int
__wrap_symlinkat(const char *target, int newdirfd, const char *newpath)
{
	if (!g_test_active)
		return symlinkat(target, newdirfd, newpath);

	std::string dst = join_path(newdirfd, newpath);
	PathMock &pm = path_mock(dst);
	pm.exists = true;
	pm.st.st_mode = S_IFLNK | 0777;
	pm.readlink_target = target;
	return 0;
}

extern "C" int
__wrap_mkfifoat(int dirfd, const char *path, mode_t mode)
{
	if (!g_test_active)
		return mkfifoat(dirfd, path, mode);

	std::string full = join_path(dirfd, path);
	PathMock &pm = path_mock(full);
	pm.exists = true;
	pm.st.st_mode = S_IFIFO | (mode & 0777);
	return 0;
}

extern "C" int
__wrap_mknodat(int dirfd, const char *path, mode_t mode, dev_t dev)
{
	if (!g_test_active)
		return mknodat(dirfd, path, mode, dev);

	std::string full = join_path(dirfd, path);
	PathMock &pm = path_mock(full);
	pm.exists = true;
	pm.st.st_mode = mode;
	pm.st.st_rdev = dev;
	return 0;
}

extern "C" int
__wrap_mkdir(const char *path, mode_t mode)
{
	if (!g_test_active)
		return mkdir(path, mode);

	PathMock &pm = path_mock(path);
	pm.exists = true;
	pm.st.st_mode = S_IFDIR | (mode & 0777);
	return 0;
}

extern "C" int
__wrap_mkdirat(int dirfd, const char *path, mode_t mode)
{
	if (!g_test_active)
		return mkdirat(dirfd, path, mode);

	std::string full = join_path(dirfd, path);
	PathMock &pm = path_mock(full);
	pm.exists = true;
	pm.st.st_mode = S_IFDIR | (mode & 0777);
	return 0;
}

extern "C" int
__wrap_rmdir(const char *path)
{
	if (!g_test_active)
		return rmdir(path);
	PathMock &pm = path_mock(path);
	if (!pm.exists) {
		errno = ENOENT;
		return -1;
	}
	pm.exists = false;
	return 0;
}

extern "C" int
__wrap_futimens(int fd, const struct timespec ts[2])
{
	if (!g_test_active)
		return futimens(fd, ts);

	FdMock *fm = fd_mock(fd);
	if (fm == nullptr) {
		errno = EBADF;
		return -1;
	}
	auto it = g_mock.futimens_fail.find(fm->path);
	if (it != g_mock.futimens_fail.end()) {
		errno = it->second;
		return -1;
	}
	fm->st.st_atim = ts[0];
	fm->st.st_mtim = ts[1];
	return 0;
}

extern "C" int
__wrap_utimensat(int dirfd, const char *path, const struct timespec ts[2],
    int atflags)
{
	(void)atflags;
	if (!g_test_active)
		return utimensat(dirfd, path, ts, atflags);

	std::string full = join_path(dirfd, path);
	auto it = g_mock.futimens_fail.find(full);
	if (it != g_mock.futimens_fail.end()) {
		errno = it->second;
		return -1;
	}
	PathMock &pm = path_mock(full);
	pm.st.st_atim = ts[0];
	pm.st.st_mtim = ts[1];
	return 0;
}

extern "C" int
__wrap_fchown(int fd, uid_t owner, gid_t group)
{
	if (!g_test_active)
		return fchown(fd, owner, group);

	FdMock *fm = fd_mock(fd);
	if (fm == nullptr) {
		errno = EBADF;
		return -1;
	}
	auto it = g_mock.fchown_fail.find(fm->path);
	if (it != g_mock.fchown_fail.end()) {
		errno = it->second;
		return -1;
	}
	fm->st.st_uid = owner;
	fm->st.st_gid = group;
	return 0;
}

extern "C" int
__wrap_fchownat(int dirfd, const char *path, uid_t owner, gid_t group,
    int atflags)
{
	(void)atflags;
	if (!g_test_active)
		return fchownat(dirfd, path, owner, group, atflags);

	std::string full = join_path(dirfd, path);
	auto it = g_mock.fchown_fail.find(full);
	if (it != g_mock.fchown_fail.end()) {
		errno = it->second;
		return -1;
	}
	PathMock &pm = path_mock(full);
	pm.st.st_uid = owner;
	pm.st.st_gid = group;
	return 0;
}

extern "C" int
__wrap_fchmod(int fd, mode_t mode)
{
	if (!g_test_active)
		return fchmod(fd, mode);

	FdMock *fm = fd_mock(fd);
	if (fm == nullptr) {
		errno = EBADF;
		return -1;
	}
	auto it = g_mock.fchmod_fail.find(fm->path);
	if (it != g_mock.fchmod_fail.end()) {
		errno = it->second;
		return -1;
	}
	fm->st.st_mode = (fm->st.st_mode & S_IFMT) | (mode & 07777);
	return 0;
}

extern "C" int
__wrap_fchmodat(int dirfd, const char *path, mode_t mode, int atflags)
{
	(void)atflags;
	if (!g_test_active)
		return fchmodat(dirfd, path, mode, atflags);

	std::string full = join_path(dirfd, path);
	auto it = g_mock.fchmod_fail.find(full);
	if (it != g_mock.fchmod_fail.end()) {
		errno = it->second;
		return -1;
	}
	PathMock &pm = path_mock(full);
	pm.st.st_mode = (pm.st.st_mode & S_IFMT) | (mode & 07777);
	return 0;
}

extern "C" int
__wrap_fchflags(int fd, unsigned long flags)
{
	if (!g_test_active)
		return __real_fchflags(fd, flags);

	FdMock *fm = fd_mock(fd);
	if (fm == nullptr) {
		errno = EBADF;
		return -1;
	}
	auto it = g_mock.chflags_fail.find(fm->path);
	if (it != g_mock.chflags_fail.end()) {
		errno = it->second;
		return -1;
	}
#if defined(__linux__)
	fm->st.st_blksize = (blksize_t)flags;
#else
	fm->st.st_flags = flags;
#endif
	g_mock.chflags_val[fm->path] = flags;
	return 0;
}

extern "C" int
__wrap_chflagsat(int dirfd, const char *path, unsigned long flags, int atflag)
{
	(void)atflag;
	if (!g_test_active)
		return __real_chflagsat(dirfd, path, flags, atflag);

	std::string full = join_path(dirfd, path);
	auto it = g_mock.chflags_fail.find(full);
	if (it != g_mock.chflags_fail.end()) {
		errno = it->second;
		return -1;
	}
	PathMock &pm = path_mock(full);
#if defined(__linux__)
	pm.st.st_blksize = (blksize_t)flags;
#else
	pm.st.st_flags = flags;
#endif
	g_mock.chflags_val[full] = flags;
	return 0;
}

extern "C" long
__wrap_fpathconf(int fd, int name)
{
	if (!g_test_active)
		return fpathconf(fd, name);

	FdMock *fm = fd_mock(fd);
	if (fm == nullptr) {
		errno = EBADF;
		return -1;
	}
	if (name == _PC_ACL_NFS4) {
		if (fm->fpathconf_nfs4 < 0) {
			errno = fm->fpathconf_nfs4_errno;
			return -1;
		}
		return fm->fpathconf_nfs4;
	}
	if (name == _PC_ACL_EXTENDED) {
		if (fm->fpathconf_ext < 0) {
			errno = fm->fpathconf_ext_errno;
			return -1;
		}
		return fm->fpathconf_ext;
	}
	errno = EINVAL;
	return -1;
}

extern "C" acl_t
__wrap_acl_get_fd_np(int fd, acl_type_t type)
{
	(void)type;
	if (!g_test_active)
		return __real_acl_get_fd_np(fd, type);

	FdMock *fm = fd_mock(fd);
	if (fm == nullptr) {
		errno = EBADF;
		return nullptr;
	}
	if (fm->acl_get_errno != 0) {
		errno = fm->acl_get_errno;
		return nullptr;
	}
	if (fm->acl_handle == 0)
		return nullptr;
	return (acl_t)fm->acl_handle;
}

extern "C" int
__wrap_acl_is_trivial_np(acl_t acl, int *trivial)
{
	if (!g_test_active)
		return __real_acl_is_trivial_np(acl, trivial);

	for (const auto &kv : g_mock.fds) {
		if (kv.second.acl_handle == (uintptr_t)acl) {
			if (kv.second.acl_trivial_fail) {
				errno = EINVAL;
				return -1;
			}
			*trivial = kv.second.acl_trivial;
			return 0;
		}
	}
	errno = EINVAL;
	return -1;
}

extern "C" int
__wrap_acl_set_fd_np(int fd, acl_t acl, acl_type_t type)
{
	(void)acl;
	(void)type;
	if (!g_test_active)
		return __real_acl_set_fd_np(fd, acl, type);

	FdMock *fm = fd_mock(fd);
	if (fm == nullptr) {
		errno = EBADF;
		return -1;
	}
	if (fm->acl_set_fail) {
		errno = EIO;
		return -1;
	}
	return 0;
}

extern "C" int
__wrap_acl_free(acl_t acl)
{
	if (!g_test_active)
		return __real_acl_free(acl);
	(void)acl;
	return 0;
}

extern "C" int
__wrap_getchar(void)
{
	if (!g_test_active)
		return getchar();
	if (g_mock.getchar_errno) {
		errno = g_mock.getchar_errno;
		return EOF;
	}
	return g_mock.getchar_val;
}

extern "C" sighandler_t
__wrap_signal(int sig, sighandler_t handler)
{
	if (!g_test_active)
		return signal(sig, handler);
	(void)sig;
	(void)handler;
	return SIG_DFL;
}

static FtsNode *
make_fts_node(int info, int level, int fts_errno, const std::string &accpath,
    const std::string &tpath, const char *name, const struct stat *st)
{
	auto node = std::make_unique<FtsNode>();
	node->accpath = accpath;
	node->path = tpath;
	size_t nlen = std::strlen(name) + 1;
	node->blob.resize(sizeof(FTSENT) + nlen);
	node->ent = reinterpret_cast<FTSENT *>(node->blob.data());
	std::memset(node->ent, 0, sizeof(FTSENT));
	node->ent->fts_info = (unsigned short)info;
	node->ent->fts_level = (short)level;
	node->ent->fts_errno = fts_errno;
	node->ent->fts_accpath = const_cast<char *>(node->accpath.c_str());
	node->ent->fts_path = const_cast<char *>(node->path.c_str());
	node->ent->fts_namelen = (unsigned short)std::strlen(name);
	if (st != nullptr)
		node->statbuf = *st;
	node->ent->fts_statp = &node->statbuf;
	std::memcpy(node->ent->fts_name, name, nlen);
	return node.release();
}

extern "C" FTS *
__wrap_fts_open(char *const *argv, int options,
    int (*compar)(const FTSENT *const *, const FTSENT *const *))
{
	(void)argv;
	(void)options;
	(void)compar;
	if (!g_test_active) {
		auto cmp = reinterpret_cast<int (*)(const FTSENT **,
		    const FTSENT **)>(compar);
		return fts_open(argv, options, cmp);
	}
	if (g_fts_script == nullptr)
		return nullptr;
	return (FTS *)g_fts_script.get();
}

extern "C" FTSENT *
__wrap_fts_read(FTS *ftsp)
{
	if (!g_test_active)
		return fts_read(ftsp);

	FtsState *st = (FtsState *)ftsp;
	while (st->idx < st->nodes.size()) {
		FtsNode *node = st->nodes[st->idx].get();
		if (st->skip_pending) {
			if (node->ent->fts_level > st->skip_level) {
				st->idx++;
				continue;
			}
			if (node->ent->fts_info == FTS_DP &&
			    node->ent->fts_level == st->skip_level) {
				st->skip_pending = false;
			} else {
				st->idx++;
				continue;
			}
		}
		st->idx++;
		return node->ent;
	}
	return nullptr;
}

extern "C" int
__wrap_fts_set(FTS *ftsp, FTSENT *p, int instr)
{
	if (!g_test_active)
		return fts_set(ftsp, p, instr);

	FtsState *st = (FtsState *)ftsp;
	if (st == nullptr || p == nullptr)
		return -1;
	if (instr == FTS_SKIP) {
		st->skip_pending = true;
		st->skip_level = p->fts_level;
	}
	return 0;
}

extern "C" int
__wrap_fts_close(FTS *ftsp)
{
	if (!g_test_active)
		return fts_close(ftsp);
	(void)ftsp;
	return 0;
}

extern "C" int
__wrap_asprintf(char **strp, const char *fmt, ...)
{
	if (!g_test_active) {
		va_list ap;
		va_start(ap, fmt);
		int r = vasprintf(strp, fmt, ap);
		va_end(ap);
		return r;
	}

	char buf[PATH_MAX];
	va_list ap;
	va_start(ap, fmt);
	int n = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	if (n < 0)
		return -1;
	*strp = strdup(buf);
	return (*strp != nullptr) ? n : -1;
}

static void
copy_path_t(P::PATH_T *dst, const PATH_T *src)
{
	dst->dir = src->dir;
	std::memcpy(dst->base, src->base, sizeof(dst->base));
	std::memcpy(dst->path, src->path, sizeof(dst->path));
	dst->end = dst->path + (src->end - src->path);
}

static void
setup_reg_file(const std::string &path, const std::vector<unsigned char> &data,
    mode_t mode)
{
	PathMock &pm = path_mock(path);
	pm.exists = true;
	pm.st.st_mode = S_IFREG | (mode & 0777);
	pm.st.st_size = (off_t)data.size();
	pm.data = data;
}

static bool
run_copy_fallback_case(const char *label)
{
	st_copy_fallback.cases++;
	mock_reset();
	reset_globals();
	g_test_active = true;

	g_mock.sysconf_pages = (long)rng.bits(1, PHYSPAGES_THRESHOLD * 2);

	std::vector<unsigned char> payload;
	int nbytes = rng.bits(0, 4096);
	for (int i = 0; i < nbytes; i++)
		payload.push_back(rng.byte());

	setup_reg_file("/in", payload, 0644);
	int in_fd = alloc_fd("/in", false);
	int out_fd = alloc_fd("/out", false);
	g_mock.fds[out_fd].data.clear();
	g_mock.fds[out_fd].pos = 0;
	g_mock.fds[in_fd].data = payload;
	g_mock.fds[in_fd].pos = 0;

	g_mock.ioq.clear();
	if (rng.coin()) {
		g_mock.ioq.push_back({ in_fd, 1, -1, EIO, 0 });
	} else if (payload.empty()) {
		g_mock.ioq.push_back({ in_fd, 1, 0, 0, 0 });
	} else if (rng.coin()) {
		ssize_t part = rng.bits(1, (int)payload.size());
		g_mock.ioq.push_back({ in_fd, 1, part, 0, part });
	}
	if (rng.coin() && !payload.empty())
		g_mock.ioq.push_back({ out_fd, 2, -1, EIO, 0 });

	g_mock.ioq_pos = 0;
	int saved = silence_stderr();
	ssize_t rr = ref_copy_fallback(in_fd, out_fd);
	g_mock.ioq_pos = 0;
	ssize_t rp = P::copy_fallback(in_fd, out_fd);
	restore_stderr(saved);
	g_test_active = false;

	if (rr != rp) {
		std::printf("    %s: ret %zd vs %zd\n", label, rr, rp);
		return fail(st_copy_fallback, label);
	}
	return true;
}

static FtsNode *
make_ent_node(const char *name, const char *path, mode_t mode, off_t size)
{
	struct stat st {};
	st.st_mode = mode;
	st.st_size = size;
	st.st_uid = 1000;
	st.st_gid = 1000;
	st.st_nlink = 1;
	return make_fts_node(FTS_F, FTS_ROOTLEVEL, 0, path, path, name, &st);
}

static bool
run_copy_file_case(const char *label, bool dne, bool beneath)
{
	st_copy_file.cases++;
	mock_reset();
	reset_globals();
	g_test_active = true;

	const char *src = "/mock/srcfile";
	const char *dstfull = "/mock/dstfile";
	setup_reg_file(src, { 'a', 'b', 'c', GUARD }, 0644);
	if (!dne)
		setup_reg_file(dstfull, { 'x' }, 0644);

	ref_to.dir = AT_FDCWD;
	strlcpy(ref_to.path, "dstfile", sizeof(ref_to.path));
	ref_to.end = ref_to.path + strlen(ref_to.path);
	ref_to.base[0] = '\0';
	copy_path_t(&P::to, &ref_to);

	ref_nflag = P::nflag = rng.coin() && !dne;
	ref_vflag = P::vflag = rng.coin();
	ref_fflag = P::fflag = rng.coin() && !dne;
	ref_iflag = P::iflag = false;
	ref_lflag = P::lflag = false;
	ref_sflag = P::sflag = false;
	ref_pflag = P::pflag = false;

	g_mock.copy_file_range_fail = rng.coin();

	FtsNode *node = make_ent_node("srcfile", src, S_IFREG | 0644, 3);

	int saved = silence_stderr();
	int rr = ref_copy_file(node->ent, dne, beneath);
	int rp = P::copy_file(node->ent, dne, beneath);
	restore_stderr(saved);

	delete node;
	g_test_active = false;

	if (rr != rp)
		return fail(st_copy_file, label);
	return true;
}

static bool
run_copy_link_case(const char *label, bool dne, bool beneath)
{
	st_copy_link.cases++;
	mock_reset();
	reset_globals();
	g_test_active = true;

	const char *src = "/mock/linksrc";
	const char *dstpath = "linkdst";
	path_mock(src).exists = true;
	path_mock(src).st.st_mode = S_IFLNK | 0777;
	path_mock(src).readlink_target = "target";
	if (!dne) {
		std::string full = join_path(AT_FDCWD, dstpath);
		path_mock(full).exists = true;
		path_mock(full).st.st_mode = S_IFLNK | 0777;
	}

	ref_to.dir = AT_FDCWD;
	strlcpy(ref_to.path, dstpath, sizeof(ref_to.path));
	ref_to.end = ref_to.path + strlen(ref_to.path);
	ref_to.base[0] = '\0';
	copy_path_t(&P::to, &ref_to);

	ref_nflag = P::nflag = rng.coin() && !dne;
	ref_vflag = P::vflag = rng.coin();
	ref_pflag = P::pflag = rng.coin();

	FtsNode *node = make_fts_node(FTS_SL, FTS_ROOTLEVEL, 0, src, src,
	    "linksrc", &path_mock(src).st);

	int saved = silence_stderr();
	int rr = ref_copy_link(node->ent, dne, beneath);
	int rp = P::copy_link(node->ent, dne, beneath);
	restore_stderr(saved);

	delete node;
	g_test_active = false;

	if (rr != rp)
		return fail(st_copy_link, label);
	return true;
}

static bool
run_copy_fifo_case(const char *label, bool dne, bool beneath)
{
	st_copy_fifo.cases++;
	mock_reset();
	reset_globals();
	g_test_active = true;

	struct stat fs {};
	fs.st_mode = S_IFIFO | 0600;
	fs.st_uid = 1000;
	fs.st_gid = 1000;

	const char *dst = "fifodst";
	if (!dne) {
		std::string full = join_path(AT_FDCWD, dst);
		path_mock(full).exists = true;
		path_mock(full).st.st_mode = S_IFIFO | 0600;
	}

	ref_to.dir = AT_FDCWD;
	strlcpy(ref_to.path, dst, sizeof(ref_to.path));
	ref_to.end = ref_to.path + strlen(ref_to.path);
	ref_to.base[0] = '\0';
	copy_path_t(&P::to, &ref_to);

	ref_nflag = P::nflag = rng.coin() && !dne;
	ref_vflag = P::vflag = rng.coin();
	ref_pflag = P::pflag = rng.coin();

	int saved = silence_stderr();
	int rr = ref_copy_fifo(&fs, dne, beneath);
	int rp = P::copy_fifo(&fs, dne, beneath);
	restore_stderr(saved);
	g_test_active = false;

	if (rr != rp)
		return fail(st_copy_fifo, label);
	return true;
}

static bool
run_copy_special_case(const char *label, bool dne, bool beneath, mode_t mode)
{
	st_copy_special.cases++;
	mock_reset();
	reset_globals();
	g_test_active = true;

	struct stat fs {};
	fs.st_mode = mode;
	fs.st_rdev = makedev(1, 3);
	fs.st_uid = 1000;
	fs.st_gid = 1000;

	const char *dst = "specdst";
	if (!dne) {
		std::string full = join_path(AT_FDCWD, dst);
		path_mock(full).exists = true;
		path_mock(full).st.st_mode = mode;
	}

	ref_to.dir = AT_FDCWD;
	strlcpy(ref_to.path, dst, sizeof(ref_to.path));
	ref_to.end = ref_to.path + strlen(ref_to.path);
	ref_to.base[0] = '\0';
	copy_path_t(&P::to, &ref_to);

	ref_nflag = P::nflag = rng.coin() && !dne;
	ref_vflag = P::vflag = rng.coin();
	ref_pflag = P::pflag = rng.coin();

	int saved = silence_stderr();
	int rr = ref_copy_special(&fs, dne, beneath);
	int rp = P::copy_special(&fs, dne, beneath);
	restore_stderr(saved);
	g_test_active = false;

	if (rr != rp)
		return fail(st_copy_special, label);
	return true;
}

static bool
run_setfile_case(const char *label, bool use_fd, bool beneath, bool islink)
{
	st_setfile.cases++;
	mock_reset();
	reset_globals();
	g_test_active = true;

	struct stat fs {};
	fs.st_mode = (islink ? S_IFLNK : S_IFREG) | 0644;
	fs.st_uid = rng.coin() ? 1000 : 1001;
	fs.st_gid = rng.coin() ? 1000 : 1001;
	fs.st_atim.tv_sec = 100;
	fs.st_mtim.tv_sec = 200;
#if defined(__linux__)
	fs.st_blksize = rng.coin() ? 1u : 2u;
#else
	fs.st_flags = rng.coin() ? 1u : 2u;
#endif

	const char *dst = "/mock/setfile";
	path_mock(dst).exists = true;
	path_mock(dst).st = fs;
	path_mock(dst).st.st_uid = 1000;
	path_mock(dst).st.st_gid = 1000;
	path_mock(dst).st.st_mode = (islink ? S_IFLNK : S_IFREG) | 0600;

	ref_to.dir = AT_FDCWD;
	strlcpy(ref_to.path, "setfile", sizeof(ref_to.path));
	ref_to.end = ref_to.path + strlen(ref_to.path);
	strlcpy(ref_to.base, "/mock/", sizeof(ref_to.base));
	copy_path_t(&P::to, &ref_to);

	ref_Nflag = P::Nflag = rng.coin();

	int fd = -1;
	if (use_fd) {
		fd = alloc_fd(dst, false);
		g_mock.fds[fd].st = path_mock(dst).st;
	}

	if (rng.coin())
		g_mock.fchown_fail["/mock/setfile"] = EPERM;
	if (rng.coin())
		g_mock.fchmod_fail["/mock/setfile"] = EIO;
	if (rng.coin())
		g_mock.chflags_fail["/mock/setfile"] = EOPNOTSUPP;

	int saved = silence_stderr();
	int rr = ref_setfile(&fs, fd, beneath);
	int rp = P::setfile(&fs, fd, beneath);
	restore_stderr(saved);
	g_test_active = false;

	if (rr != rp)
		return fail(st_setfile, label);
	return true;
}

static bool
run_preserve_fd_acls_case(const char *label)
{
	st_preserve_fd_acls.cases++;
	mock_reset();
	reset_globals();
	g_test_active = true;

	strlcpy(ref_to.base, "/dest/", sizeof(ref_to.base));
	strlcpy(ref_to.path, "file", sizeof(ref_to.path));
	copy_path_t(&P::to, &ref_to);

	int src_fd = alloc_fd("/src", false);
	int dst_fd = alloc_fd("/dst", false);
	FdMock &src = g_mock.fds[src_fd];

	switch (rng.bits(0, 5)) {
	case 0:
		src.fpathconf_nfs4 = 0;
		src.fpathconf_ext = 0;
		break;
	case 1:
		src.fpathconf_nfs4 = 1;
		src.acl_handle = g_mock.next_acl++;
		src.acl_trivial = 1;
		break;
	case 2:
		src.fpathconf_nfs4 = 1;
		src.acl_handle = g_mock.next_acl++;
		src.acl_trivial = 0;
		break;
	case 3:
		src.fpathconf_nfs4 = -1;
		src.fpathconf_nfs4_errno = EINVAL;
		src.fpathconf_ext = 1;
		src.acl_handle = g_mock.next_acl++;
		src.acl_trivial = 0;
		break;
	case 4:
		src.fpathconf_nfs4 = -1;
		src.fpathconf_nfs4_errno = EIO;
		break;
	default:
		src.acl_get_errno = EIO;
		break;
	}
	if (rng.coin())
		g_mock.fds[dst_fd].acl_set_fail = 1;
	if (rng.coin())
		src.acl_trivial_fail = 1;

	int saved = silence_stderr();
	int rr = ref_preserve_fd_acls(src_fd, dst_fd);
	int rp = P::preserve_fd_acls(src_fd, dst_fd);
	restore_stderr(saved);
	g_test_active = false;

	if (rr != rp)
		return fail(st_preserve_fd_acls, label);
	return true;
}

static bool
run_preserve_dir_acls_case(const char *label)
{
	st_preserve_dir_acls.cases++;
	mock_reset();
	reset_globals();
	g_test_active = true;

	const char *srcdir = "/srcdir";
	path_mock(srcdir).exists = true;
	path_mock(srcdir).st.st_mode = S_IFDIR | 0755;

	ref_to.dir = alloc_fd("/destdir", true);
	strlcpy(ref_to.base, "/destdir/", sizeof(ref_to.base));
	ref_to.path[0] = '\0';
	ref_to.end = ref_to.path;
	copy_path_t(&P::to, &ref_to);

	const char *destdir = rng.coin() ? "" : "subdir";
	if (destdir[0] != '\0') {
		std::string full = join_path(ref_to.dir, destdir);
		path_mock(full).exists = true;
		path_mock(full).st.st_mode = S_IFDIR | 0755;
	}

	int src_fd = alloc_fd(srcdir, true);
	g_mock.fds[src_fd].fpathconf_ext = 1;
	g_mock.fds[src_fd].acl_handle = g_mock.next_acl++;
	g_mock.fds[src_fd].acl_trivial = rng.coin();

	int saved = silence_stderr();
	int rr = ref_preserve_dir_acls(srcdir, destdir);
	int rp = P::preserve_dir_acls(srcdir, destdir);
	restore_stderr(saved);
	g_test_active = false;

	if (rr != rp)
		return fail(st_preserve_dir_acls, label);
	return true;
}

struct UsageResult {
	int status;
	std::vector<unsigned char> err;
};

static UsageResult
run_usage_child(bool use_port)
{
	int errpipe[2];
	if (pipe(errpipe) != 0)
		return {};

	pid_t pid = fork();
	if (pid < 0)
		return {};

	if (pid == 0) {
		close(errpipe[0]);
		dup2(errpipe[1], STDERR_FILENO);
		close(errpipe[1]);
		g_test_child = true;
		g_test_active = true;
		if (use_port)
			P::usage();
		else
			ref_usage();
		::_exit(99);
	}

	close(errpipe[1]);
	UsageResult res{};
	unsigned char buf[4096];
	ssize_t nr;
	while ((nr = read(errpipe[0], buf, sizeof(buf))) > 0)
		res.err.insert(res.err.end(), buf, buf + nr);
	close(errpipe[0]);

	int status = 0;
	if (waitpid(pid, &status, 0) >= 0 && WIFEXITED(status))
		res.status = WEXITSTATUS(status);
	return res;
}

static bool
run_usage_case(const char *label)
{
	st_usage.cases++;
	UsageResult ref = run_usage_child(false);
	UsageResult port = run_usage_child(true);
	if (ref.status != port.status || ref.status != EX_USAGE)
		return fail(st_usage, label);
	if (ref.err != port.err)
		return fail(st_usage, label);
	return true;
}

static FtsNode *
make_name_node(const char *name)
{
	struct stat st {};
	st.st_mode = S_IFREG | 0644;
	return make_fts_node(FTS_F, 0, 0, "/a", "/a", name, &st);
}

static bool
run_ftscmp_case(const char *label, const char *a, const char *b)
{
	st_ftscmp.cases++;
	FtsNode *na = make_name_node(a);
	FtsNode *nb = make_name_node(b);
	const FTSENT *pa = na->ent;
	const FTSENT *pb = nb->ent;
	int rr = ref_ftscmp(&pa, &pb);
	int rp = P::ftscmp(&pa, &pb);
	delete na;
	delete nb;
	if (rr != rp)
		return fail(st_ftscmp, label);
	return true;
}

static bool
run_siginfo_case(const char *label, int sig)
{
	st_siginfo.cases++;
	reset_globals();
	ref_siginfo(sig);
	int rr = (int)ref_info;
	reset_globals();
	P::siginfo(sig);
	int rp = (int)P::info;
	if (rr != rp || rr != 1)
		return fail(st_siginfo, label);
	return true;
}

static void
build_fts_script_simple()
{
	g_fts_script = std::make_unique<FtsState>();
	setup_reg_file("/src/a", { 'd', 'a', 't', 'a' }, 0644);
	g_fts_script->nodes.emplace_back(std::unique_ptr<FtsNode>(
	    make_fts_node(FTS_F, FTS_ROOTLEVEL, 0, "/src/a", "/src/a", "a",
	    &path_mock("/src/a").st)));
}

static bool
paths_equal(const PATH_T *a, const P::PATH_T *b)
{
	if (a->dir != b->dir)
		return false;
	if (std::strcmp(a->base, b->base) != 0)
		return false;
	if (std::strcmp(a->path, b->path) != 0)
		return false;
	return (a->end - a->path) == (b->end - b->path);
}

static bool
run_copy_case(const char *label, enum ref_op type)
{
	st_copy.cases++;
	mock_reset();
	reset_globals();
	g_test_active = true;

	build_fts_script_simple();

	if (type == RFILE_TO_FILE) {
		ref_to.dir = AT_FDCWD;
		strlcpy(ref_to.path, "out", sizeof(ref_to.path));
		ref_to.end = ref_to.path + 3;
		ref_to.base[0] = '\0';
	} else {
		ref_to.dir = alloc_fd("/dest", true);
		strlcpy(ref_to.base, "/dest/", sizeof(ref_to.base));
		ref_to.path[0] = '\0';
		ref_to.end = ref_to.path;
	}
	copy_path_t(&P::to, &ref_to);

	char arg0[] = "/src/a";
	char *argv[] = { arg0, nullptr };

	struct stat root_stat {};
	struct stat *rootp = nullptr;
	if (type == RFILE_TO_DIR) {
		root_stat.st_mode = S_IFDIR | 0755;
		root_stat.st_dev = 1;
		root_stat.st_ino = 1;
		rootp = &root_stat;
	}

	int fts_options = FTS_NOCHDIR | FTS_PHYSICAL;
	int saved = silence_stderr();
	int rr = ref_ref_copy(argv, type, fts_options, rootp);
	char *end_ref = ref_to.end;
	int rp = P::copy(argv, (P::op)type, fts_options, rootp);
	char *end_port = P::to.end;
	restore_stderr(saved);
	g_test_active = false;

	if (rr != rp || !paths_equal(&ref_to, &P::to))
		return fail(st_copy, label);
	if ((end_ref - ref_to.path) != (end_port - P::to.path))
		return fail(st_copy, label);
	return true;
}

static std::string
make_temp_root()
{
	char tmpl[] = "/tmp/pbsd_b0191_XXXXXX";
	char *dir = mkdtemp(tmpl);
	if (dir == nullptr)
		return "";
	return std::string(dir);
}

struct MainResult {
	int status;
	std::vector<unsigned char> out;
	std::vector<unsigned char> err;
};

static MainResult
run_main_child(bool use_port, int argc, char **argv)
{
	int pout[2], perr[2];
	if (pipe(pout) != 0 || pipe(perr) != 0)
		return {};

	pid_t pid = fork();
	if (pid < 0)
		return {};

	if (pid == 0) {
		dup2(pout[1], STDOUT_FILENO);
		dup2(perr[1], STDERR_FILENO);
		close(pout[0]);
		close(pout[1]);
		close(perr[0]);
		close(perr[1]);
		g_test_child = true;
		g_test_active = false;
		optind = 1;
#if defined(__GLIBC__) && defined(optreset)
		optreset = 1;
#endif
		int ret = use_port ? P::main(argc, argv) : ref_main(argc, argv);
		::_exit(ret);
	}

	close(pout[1]);
	close(perr[1]);
	MainResult res{};
	unsigned char buf[4096];
	ssize_t nr;
	while ((nr = read(pout[0], buf, sizeof(buf))) > 0)
		res.out.insert(res.out.end(), buf, buf + nr);
	while ((nr = read(perr[0], buf, sizeof(buf))) > 0)
		res.err.insert(res.err.end(), buf, buf + nr);
	close(pout[0]);
	close(perr[0]);

	int st = 0;
	if (waitpid(pid, &st, 0) >= 0 && WIFEXITED(st))
		res.status = WEXITSTATUS(st);
	return res;
}

static bool
run_main_case(const char *label, const std::vector<std::string> &args)
{
	st_main.cases++;
	static char prog[] = "cp";
	std::vector<std::string> pool = args;
	std::vector<char *> argv;
	argv.push_back(prog);
	for (auto &a : pool)
		argv.push_back(const_cast<char *>(a.c_str()));
	argv.push_back(nullptr);

	MainResult ref = run_main_child(false, (int)argv.size() - 1, argv.data());
	MainResult port = run_main_child(true, (int)argv.size() - 1, argv.data());

	if (ref.status != port.status)
		return fail(st_main, label);
	if (ref.out != port.out || ref.err != port.err)
		return fail(st_main, label);
	return true;
}

static void test_copy_fallback_hand()
{
	g_mock.sysconf_pages = PHYSPAGES_THRESHOLD + 1;
	(void)run_copy_fallback_case("large_buf");
	g_mock.sysconf_pages = 1;
	(void)run_copy_fallback_case("small_buf");
}

static void test_copy_fallback_sweep()
{
	for (long i = 0; i < 18000; i++)
		(void)run_copy_fallback_case("sweep");
}

static void test_copy_file_hand()
{
	(void)run_copy_file_case("dne", true, false);
	(void)run_copy_file_case("exists", false, false);
}

static void test_copy_file_sweep()
{
	for (long i = 0; i < 24500; i++)
		(void)run_copy_file_case("sweep", rng.coin(), rng.coin());
}

static void test_copy_link_hand()
{
	(void)run_copy_link_case("dne", true, false);
	(void)run_copy_link_case("exists", false, false);
}

static void test_copy_link_sweep()
{
	for (long i = 0; i < 14500; i++)
		(void)run_copy_link_case("sweep", rng.coin(), rng.coin());
}

static void test_copy_fifo_hand()
{
	(void)run_copy_fifo_case("dne", true, false);
	(void)run_copy_fifo_case("exists", false, false);
}

static void test_copy_fifo_sweep()
{
	for (long i = 0; i < 11500; i++)
		(void)run_copy_fifo_case("sweep", rng.coin(), rng.coin());
}

static void test_copy_special_hand()
{
	(void)run_copy_special_case("chr", true, false, S_IFCHR | 0600);
	(void)run_copy_special_case("blk", false, false, S_IFBLK | 0600);
}

static void test_copy_special_sweep()
{
	for (long i = 0; i < 11500; i++) {
		mode_t m = rng.coin() ? S_IFCHR : S_IFBLK;
		(void)run_copy_special_case("sweep", rng.coin(), rng.coin(), m | 0600);
	}
}

static void test_setfile_hand()
{
	(void)run_setfile_case("fd", true, false, false);
	(void)run_setfile_case("path", false, false, false);
	(void)run_setfile_case("symlink", false, false, true);
}

static void test_setfile_sweep()
{
	for (long i = 0; i < 24500; i++)
		(void)run_setfile_case("sweep", rng.coin(), rng.coin(), rng.coin());
}

static void test_preserve_fd_acls_hand()
{
	(void)run_preserve_fd_acls_case("hand");
}

static void test_preserve_fd_acls_sweep()
{
	for (long i = 0; i < 17500; i++)
		(void)run_preserve_fd_acls_case("sweep");
}

static void test_preserve_dir_acls_hand()
{
	(void)run_preserve_dir_acls_case("hand");
}

static void test_preserve_dir_acls_sweep()
{
	for (long i = 0; i < 11500; i++)
		(void)run_preserve_dir_acls_case("sweep");
}

static void test_usage_hand()
{
	(void)run_usage_case("hand");
}

static void test_usage_sweep()
{
	for (long i = 0; i < 7500; i++)
		(void)run_usage_case("sweep");
}

static void test_ftscmp_hand()
{
	(void)run_ftscmp_case("eq", "abc", "abc");
	(void)run_ftscmp_case("lt", "a", "b");
	(void)run_ftscmp_case("gt", "z", "a");
	(void)run_ftscmp_case("empty", "", "");
	(void)run_ftscmp_case("hi", "\x80", "\x7f");
}

static void test_ftscmp_sweep()
{
	for (long i = 0; i < 17500; i++) {
		char a[16], b[16];
		int la = rng.bits(0, 8);
		int lb = rng.bits(0, 8);
		for (int j = 0; j < la; j++)
			a[j] = (char)rng.byte();
		a[la] = '\0';
		for (int j = 0; j < lb; j++)
			b[j] = (char)rng.byte();
		b[lb] = '\0';
		(void)run_ftscmp_case("sweep", a, b);
	}
}

static void test_siginfo_hand()
{
	(void)run_siginfo_case("siginfo", SIGINFO);
	(void)run_siginfo_case("zero", 0);
}

static void test_siginfo_sweep()
{
	for (long i = 0; i < 7500; i++)
		(void)run_siginfo_case("sweep", rng.bits(-128, 255));
}

static void test_copy_hand()
{
	(void)run_copy_case("file_to_file", RFILE_TO_FILE);
	(void)run_copy_case("file_to_dir", RFILE_TO_DIR);
}

static void test_copy_sweep()
{
	for (long i = 0; i < 27500; i++) {
		enum ref_op t = rng.coin() ? RFILE_TO_FILE : RFILE_TO_DIR;
		(void)run_copy_case("sweep", t);
	}
}

static void test_main_hand()
{
	std::string root = make_temp_root();
	if (root.empty())
		return;

	std::string f1 = root + "/f1";
	std::string f2 = root + "/f2";
	std::string sub = root + "/sub";
	std::string dne = root + "/newdir";

	FILE *fp = fopen(f1.c_str(), "w");
	if (fp) {
		fputs("hello", fp);
		fclose(fp);
	}
	fp = fopen(f2.c_str(), "w");
	if (fp) {
		fputs("world", fp);
		fclose(fp);
	}
	mkdir(sub.c_str(), 0755);

	(void)run_main_case("usage_no_args", {});
	(void)run_main_case("usage_one_arg", { f1 });
	(void)run_main_case("bad_opt", { "-Z", f1, f2 });
	(void)run_main_case("R_r_conflict", { "-R", "-r", f1, sub });
	(void)run_main_case("l_s_conflict", { "-l", "-s", f1, f2 });
	(void)run_main_case("file_to_file", { f1, f2 });
	(void)run_main_case("to_dir", { f1, sub });
	(void)run_main_case("recursive", { "-R", f1, dne });
	(void)run_main_case("no_clobber", { "-n", f1, f2 });
	(void)run_main_case("verbose", { "-v", f1, f2 });

	unlink(f1.c_str());
	unlink(f2.c_str());
	rmdir(sub.c_str());
	rmdir(dne.c_str());
	rmdir(root.c_str());
}

static void test_main_sweep()
{
	static const char *opts[] = { "", "-f", "-n", "-v", "-p", "-H", "-L",
	    "-P", "-R", "-a", "-l", "-s", "-x", "-N" };

	for (long i = 0; i < 19500; i++) {
		std::string root = make_temp_root();
		if (root.empty())
			continue;
		std::string f1 = root + "/a";
		std::string f2 = root + "/b";
		std::string sub = root + "/d";
		FILE *fp = fopen(f1.c_str(), "w");
		if (fp) {
			fputc((char)rng.byte(), fp);
			fclose(fp);
		}
		fp = fopen(f2.c_str(), "w");
		if (fp) {
			fputc((char)rng.byte(), fp);
			fclose(fp);
		}
		mkdir(sub.c_str(), 0755);

		std::vector<std::string> args;
		if ((rng.next() & 3u) == 0u) {
			const char *o = opts[rng.bits(0, 13)];
			if (o[0] != '\0')
				args.emplace_back(o);
		}
		args.push_back(f1);
		if (rng.coin())
			args.push_back(f2);
		args.push_back(rng.coin() ? sub : f2);

		(void)run_main_case("sweep", args);

		unlink(f1.c_str());
		unlink(f2.c_str());
		rmdir(sub.c_str());
		rmdir(root.c_str());
	}
}

} // namespace

namespace pbsd::bin_cp::b0191 {

int fchflags(int fd, unsigned long flags)
{
	return ::fchflags(fd, flags);
}

int chflagsat(int dirfd, const char *path, unsigned long flags, int atflag)
{
	return ::chflagsat(dirfd, path, flags, atflag);
}

acl_t acl_get_fd_np(int fd, acl_type_t type)
{
	return ::acl_get_fd_np(fd, type);
}

int acl_is_trivial_np(acl_t acl, int *trivial)
{
	return ::acl_is_trivial_np(acl, trivial);
}

int acl_set_fd_np(int fd, acl_t acl, acl_type_t type)
{
	return ::acl_set_fd_np(fd, acl, type);
}

int acl_free(acl_t acl)
{
	return ::acl_free(acl);
}

} // namespace pbsd::bin_cp::b0191

extern "C" {
int fchflags(int fd, unsigned long flags);
int chflagsat(int dirfd, const char *path, unsigned long flags, int atflag);
acl_t acl_get_fd_np(int fd, acl_type_t type);
int acl_is_trivial_np(acl_t acl, int *trivial);
int acl_set_fd_np(int fd, acl_t acl, acl_type_t type);
int acl_free(acl_t acl);
}

namespace {
{
	test_copy_fallback_hand();
	test_copy_file_hand();
	test_copy_link_hand();
	test_copy_fifo_hand();
	test_copy_special_hand();
	test_setfile_hand();
	test_preserve_fd_acls_hand();
	test_preserve_dir_acls_hand();
	test_usage_hand();
	test_ftscmp_hand();
	test_siginfo_hand();
	test_copy_hand();
	test_main_hand();

	test_copy_fallback_sweep();
	test_copy_file_sweep();
	test_copy_link_sweep();
	test_copy_fifo_sweep();
	test_copy_special_sweep();
	test_setfile_sweep();
	test_preserve_fd_acls_sweep();
	test_preserve_dir_acls_sweep();
	test_usage_sweep();
	test_ftscmp_sweep();
	test_siginfo_sweep();
	test_copy_sweep();
	test_main_sweep();

	Stat *all[] = {
		&st_copy_fallback, &st_copy_file, &st_copy_link, &st_copy_fifo,
		&st_copy_special, &st_setfile, &st_preserve_fd_acls,
		&st_preserve_dir_acls, &st_usage, &st_ftscmp, &st_siginfo,
		&st_copy, &st_main,
	};
	long total_cases = 0;
	long total_fails = 0;

	std::printf("\n%-22s %12s %12s\n", "function", "cases", "failures");
	for (Stat *st : all) {
		std::printf("%-22s %12ld %12ld\n", st->name, st->cases,
		    st->fails);
		total_cases += st->cases;
		total_fails += st->fails;
	}
	std::printf("%-22s %12ld %12ld\n", "TOTAL", total_cases, total_fails);

	return (total_fails == 0 ? 0 : 1);
}
