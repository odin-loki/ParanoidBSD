/*
 * harness.cpp -- differential test for PBSD batch b0183 (cpuset.c).
 */

#define _GNU_SOURCE

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

import pbsd.bin.cpuset.b0183;

namespace P = pbsd::bin_cpuset::b0183;

extern int optind;

extern "C" {
void ref_printset(struct bitset *mask, int size);
void ref_printaffinity(void);
void ref_printsetid(void);
int ref_main(int argc, char **argv);
void ref_usage(void);
void ref_reset_globals(void);

extern int ref_Cflag;
extern int ref_cflag;
extern int ref_dflag;
extern int ref_gflag;
extern int ref_iflag;
extern int ref_jflag;
extern int ref_lflag;
extern int ref_nflag;
extern int ref_pflag;
extern int ref_rflag;
extern int ref_sflag;
extern int ref_tflag;
extern int ref_xflag;
extern id_t ref_id;
extern int ref_level;
extern int ref_which;

extern char jail_errmsg[];

#define _BITSET_BITS (sizeof(unsigned long) * 8)
#define __howmany(x, y) (((x) + ((y) - 1)) / (y))
#define __bitset_words(_s) (__howmany(_s, _BITSET_BITS))
#define __BITSET_DEFINE(_t, _s)                                        \
	struct _t {                                                    \
		unsigned long __bits[__bitset_words((_s))];            \
	}
#define __bitset_mask(_s, n)                                           \
	(1UL << (__builtin_constant_p(__bitset_words((_s)) == 1) ?     \
		    (size_t)(n)                                          \
					       : ((n) % _BITSET_BITS)))
#define __bitset_word(_s, n)                                           \
	(__builtin_constant_p(__bitset_words((_s)) == 1) ? 0           \
							    : ((n) / _BITSET_BITS))
#define __BIT_SET(_s, n, p)                                            \
	((p)->__bits[__bitset_word(_s, n)] |= __bitset_mask((_s), (n)))
#define __BIT_ZERO(_s, p)                                              \
	do {                                                           \
		size_t __i;                                            \
		for (__i = 0; __i < __bitset_words((_s)); __i++)       \
			(p)->__bits[__i] = 0L;                         \
	} while (0)
#define __BIT_ISSET(_s, n, p)                                          \
	((((p)->__bits[__bitset_word(_s, n)] &                          \
	   __bitset_mask((_s), (n))) != 0))

__BITSET_DEFINE(bitset, 1);

#define CPU_SETSIZE 1024
__BITSET_DEFINE(_cpuset, CPU_SETSIZE);
typedef struct _cpuset cpuset_t;
#define CPU_SET(n, p) __BIT_SET(CPU_SETSIZE, n, p)
#define CPU_ZERO(p) __BIT_ZERO(CPU_SETSIZE, p)

#define DOMAINSET_SETSIZE 256
__BITSET_DEFINE(_domainset, DOMAINSET_SETSIZE);
typedef struct _domainset domainset_t;
#define DOMAINSET_SET(n, p) __BIT_SET(DOMAINSET_SETSIZE, n, p)
#define DOMAINSET_ZERO(p) __BIT_ZERO(DOMAINSET_SETSIZE, p)

#define CPU_LEVEL_ROOT 1
#define CPU_LEVEL_CPUSET 2
#define CPU_LEVEL_WHICH 3
#define CPU_WHICH_TID 1
#define CPU_WHICH_PID 2
#define CPU_WHICH_CPUSET 3
#define CPU_WHICH_IRQ 4
#define CPU_WHICH_JAIL 5
#define CPU_WHICH_DOMAIN 6

typedef int cpulevel_t;
typedef int cpuwhich_t;
typedef int cpusetid_t;
#ifndef _LWPID_T_DECLARED
typedef int lwpid_t;
#define _LWPID_T_DECLARED
#endif

int cpuset(cpusetid_t *);
int cpuset_setid(cpuwhich_t, id_t, cpusetid_t);
int cpuset_getid(cpulevel_t, cpuwhich_t, id_t, cpusetid_t *);
int cpuset_getaffinity(cpulevel_t, cpuwhich_t, id_t, size_t, cpuset_t *);
int cpuset_setaffinity(cpulevel_t, cpuwhich_t, id_t, size_t,
    const cpuset_t *);
int cpuset_getdomain(cpulevel_t, cpuwhich_t, id_t, size_t, domainset_t *,
    int *);
int cpuset_setdomain(cpulevel_t, cpuwhich_t, id_t, size_t,
    const domainset_t *, int);
int jail_getid(const char *name);
int execvp(const char *, char *const *);
}

constexpr std::size_t JAIL_ERRMSG_SZ = 1024;

namespace {

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
};

Rng rng(0xb0183faceULL);

Stat st_printset = { "printset", 0, 0, 0 };
Stat st_printaffinity = { "printaffinity", 0, 0, 0 };
Stat st_printsetid = { "printsetid", 0, 0, 0 };
Stat st_main = { "main", 0, 0, 0 };
Stat st_usage = { "usage", 0, 0, 0 };

struct MockCtl {
	int getaffinity_fail;
	int getdomain_fail;
	int getid_fail;
	int getid_ret;
	int cpuset_fail;
	int setid_fail;
	int setaffinity_fail;
	int setdomain_fail;
	int jail_fail;
	int jail_ret;
	int exec_fail;
	int exec_errno;
	int root_policy;
};

MockCtl mock{};

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

struct ExitRun {
	int status;
	std::vector<unsigned char> stdout_bytes;
	std::vector<unsigned char> stderr_bytes;
};

void
mock_reset()
{
	mock = MockCtl{};
	mock.getid_ret = 42;
	mock.jail_ret = 7;
	mock.root_policy = 1;
	mock.exec_errno = ENOENT;
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

bool
same_bytes(const std::vector<unsigned char> &a,
    const std::vector<unsigned char> &b)
{
	return a == b;
}

void
fill_mask(cpuset_t *mask, cpulevel_t level, cpuwhich_t which, id_t id)
{
	int seed = (int)(level + which + id + 17);
	CPU_ZERO(mask);
	for (int i = 0; i < 8; i++) {
		int bit = (seed + i * 3) & 0x3f;
		if (bit >= 0 && bit < CPU_SETSIZE)
			CPU_SET(bit, mask);
	}
}

void
fill_domain(domainset_t *dom, cpulevel_t level, cpuwhich_t which, id_t id)
{
	int seed = (int)(level * 3 + which + id);
	DOMAINSET_ZERO(dom);
	for (int i = 0; i < 4; i++) {
		int bit = (seed + i * 5) & 0x1f;
		if (bit >= 0 && bit < DOMAINSET_SETSIZE)
			DOMAINSET_SET(bit, dom);
	}
}

} // namespace

extern "C" int
__wrap_cpuset_getaffinity(cpulevel_t level, cpuwhich_t which, id_t id,
    size_t size, cpuset_t *mask)
{
	if (mock.getaffinity_fail) {
		errno = EIO;
		return (-1);
	}
	if (mask == nullptr || size < sizeof(cpuset_t)) {
		errno = EINVAL;
		return (-1);
	}
	fill_mask(mask, level, which, id);
	return (0);
}

extern "C" int
__wrap_cpuset_getdomain(cpulevel_t level, cpuwhich_t which, id_t id,
    size_t size, domainset_t *domain, int *policy)
{
	if (mock.getdomain_fail) {
		errno = EIO;
		return (-1);
	}
	if (domain == nullptr || policy == nullptr ||
	    size < sizeof(domainset_t)) {
		errno = EINVAL;
		return (-1);
	}
	fill_domain(domain, level, which, id);
	*policy = mock.root_policy;
	return (0);
}

extern "C" int
__wrap_cpuset_getid(cpulevel_t level, cpuwhich_t which, id_t id,
    cpusetid_t *setid)
{
	(void)level;
	(void)which;
	(void)id;
	if (mock.getid_fail) {
		errno = ESRCH;
		return (-1);
	}
	if (setid == nullptr) {
		errno = EINVAL;
		return (-1);
	}
	*setid = (cpusetid_t)mock.getid_ret;
	return (0);
}

extern "C" int
__wrap_cpuset(cpusetid_t *setid)
{
	if (mock.cpuset_fail) {
		errno = ENOMEM;
		return (-1);
	}
	if (setid == nullptr) {
		errno = EINVAL;
		return (-1);
	}
	*setid = 99;
	return (0);
}

extern "C" int
__wrap_cpuset_setid(cpuwhich_t which, id_t id, cpusetid_t setid)
{
	(void)which;
	(void)id;
	(void)setid;
	if (mock.setid_fail) {
		errno = EPERM;
		return (-1);
	}
	return (0);
}

extern "C" int
__wrap_cpuset_setaffinity(cpulevel_t level, cpuwhich_t which, id_t id,
    size_t size, const cpuset_t *mask)
{
	(void)level;
	(void)which;
	(void)id;
	(void)size;
	(void)mask;
	if (mock.setaffinity_fail) {
		errno = EPERM;
		return (-1);
	}
	return (0);
}

extern "C" int
__wrap_cpuset_setdomain(cpulevel_t level, cpuwhich_t which, id_t id,
    size_t size, const domainset_t *domains, int policy)
{
	(void)level;
	(void)which;
	(void)id;
	(void)size;
	(void)domains;
	(void)policy;
	if (mock.setdomain_fail) {
		errno = EPERM;
		return (-1);
	}
	return (0);
}

extern "C" int
__wrap_jail_getid(const char *name)
{
	if (mock.jail_fail || name == nullptr || name[0] == '\0') {
		std::snprintf(jail_errmsg, JAIL_ERRMSG_SZ, "bad jail");
		return (-1);
	}
	std::snprintf(jail_errmsg, JAIL_ERRMSG_SZ, "%s", name);
	return (mock.jail_ret);
}

extern "C" int
__wrap_execvp(const char *file, char *const argv[])
{
	(void)file;
	(void)argv;
	if (mock.exec_fail) {
		errno = mock.exec_errno;
		return (-1);
	}
	errno = 0;
	return (0);
}

namespace {

std::vector<unsigned char>
capture_ref_printset(cpuset_t *mask, int size)
{
	IoCapture cap;
	if (!cap.begin(STDOUT_FILENO))
		return {};
	ref_printset((struct bitset *)mask, size);
	return cap.end(STDOUT_FILENO);
}

std::vector<unsigned char>
capture_port_printset(cpuset_t *mask, int size)
{
	IoCapture cap;
	if (!cap.begin(STDOUT_FILENO))
		return {};
	P::printset(reinterpret_cast<P::bitset_view *>(mask), size);
	return cap.end(STDOUT_FILENO);
}

bool
check_printset(const char *label, cpuset_t *mask, int size)
{
	st_printset.cases++;
	auto r = capture_ref_printset(mask, size);
	auto p = capture_port_printset(mask, size);
	if (!same_bytes(r, p))
		return fail(st_printset, label);
	return true;
}

void
set_ref_globals(cpulevel_t lv, cpuwhich_t wh, id_t ident, int df, int xf,
    int sf)
{
	ref_reset_globals();
	ref_level = lv;
	ref_which = wh;
	ref_id = ident;
	ref_dflag = df;
	ref_xflag = xf;
	ref_sflag = sf;
}

void
set_port_globals(cpulevel_t lv, cpuwhich_t wh, id_t ident, int df, int xf,
    int sf)
{
	P::reset_globals();
	P::level = lv;
	P::which = wh;
	P::id = ident;
	P::dflag = df;
	P::xflag = xf;
	P::sflag = sf;
}

ExitRun
run_ref_printaffinity(cpulevel_t lv, cpuwhich_t wh, id_t ident, int df,
    int xf)
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
		mock_reset();
		set_ref_globals(lv, wh, ident, df, xf, 0);
		dup2(pipe_out[1], STDOUT_FILENO);
		dup2(pipe_err[1], STDERR_FILENO);
		close(pipe_out[0]);
		close(pipe_out[1]);
		close(pipe_err[0]);
		close(pipe_err[1]);
		ref_printaffinity();
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
run_port_printaffinity(cpulevel_t lv, cpuwhich_t wh, id_t ident, int df,
    int xf)
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
		mock_reset();
		set_port_globals(lv, wh, ident, df, xf, 0);
		dup2(pipe_out[1], STDOUT_FILENO);
		dup2(pipe_err[1], STDERR_FILENO);
		close(pipe_out[0]);
		close(pipe_out[1]);
		close(pipe_err[0]);
		close(pipe_err[1]);
		P::printaffinity();
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
check_printaffinity(const char *label, cpulevel_t lv, cpuwhich_t wh, id_t ident,
    int df, int xf)
{
	st_printaffinity.cases++;
	ExitRun r = run_ref_printaffinity(lv, wh, ident, df, xf);
	ExitRun p = run_port_printaffinity(lv, wh, ident, df, xf);
	if (r.status != p.status)
		return fail(st_printaffinity, label);
	if (!same_bytes(r.stdout_bytes, p.stdout_bytes))
		return fail(st_printaffinity, label);
	if (!same_bytes(r.stderr_bytes, p.stderr_bytes))
		return fail(st_printaffinity, label);
	return true;
}

ExitRun
run_ref_printsetid(cpulevel_t lv, cpuwhich_t wh, id_t ident, int sf)
{
	ExitRun res{};
	IoCapture out;
	IoCapture err;
	mock_reset();
	set_ref_globals(lv, wh, ident, 0, 0, sf);
	if (!out.begin(STDOUT_FILENO) || !err.begin(STDERR_FILENO))
		return res;
	ref_printsetid();
	res.stdout_bytes = out.end(STDOUT_FILENO);
	res.stderr_bytes = err.end(STDERR_FILENO);
	res.status = 0;
	return res;
}

ExitRun
run_port_printsetid(cpulevel_t lv, cpuwhich_t wh, id_t ident, int sf)
{
	ExitRun res{};
	IoCapture out;
	IoCapture err;
	mock_reset();
	set_port_globals(lv, wh, ident, 0, 0, sf);
	if (!out.begin(STDOUT_FILENO) || !err.begin(STDERR_FILENO))
		return res;
	P::printsetid();
	res.stdout_bytes = out.end(STDOUT_FILENO);
	res.stderr_bytes = err.end(STDERR_FILENO);
	res.status = 0;
	return res;
}

bool
check_printsetid(const char *label, cpulevel_t lv, cpuwhich_t wh, id_t ident,
    int sf)
{
	st_printsetid.cases++;
	ExitRun r = run_ref_printsetid(lv, wh, ident, sf);
	ExitRun p = run_port_printsetid(lv, wh, ident, sf);
	if (r.status != p.status)
		return fail(st_printsetid, label);
	if (!same_bytes(r.stdout_bytes, p.stdout_bytes))
		return fail(st_printsetid, label);
	if (!same_bytes(r.stderr_bytes, p.stderr_bytes))
		return fail(st_printsetid, label);
	return true;
}

ExitRun
run_ref_main(int argc, char **argv)
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
		mock_reset();
		ref_reset_globals();
		optind = 1;
		opterr = 0;
		dup2(pipe_out[1], STDOUT_FILENO);
		dup2(pipe_err[1], STDERR_FILENO);
		close(pipe_out[0]);
		close(pipe_out[1]);
		close(pipe_err[0]);
		close(pipe_err[1]);
		int ret = ref_main(argc, argv);
		_exit(ret);
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
run_port_main(int argc, char **argv)
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
		mock_reset();
		P::reset_globals();
		optind = 1;
		opterr = 0;
		dup2(pipe_out[1], STDOUT_FILENO);
		dup2(pipe_err[1], STDERR_FILENO);
		close(pipe_out[0]);
		close(pipe_out[1]);
		close(pipe_err[0]);
		close(pipe_err[1]);
		int ret = P::main(argc, argv);
		_exit(ret);
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
check_main(const char *label, int argc, char **argv)
{
	st_main.cases++;
	ExitRun r = run_ref_main(argc, argv);
	ExitRun p = run_port_main(argc, argv);
	if (r.status != p.status)
		return fail(st_main, label);
	if (!same_bytes(r.stdout_bytes, p.stdout_bytes))
		return fail(st_main, label);
	if (!same_bytes(r.stderr_bytes, p.stderr_bytes))
		return fail(st_main, label);
	return true;
}

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
	if (r.status != p.status)
		return fail(st_usage, label);
	if (!same_bytes(r.stdout_bytes, p.stdout_bytes))
		return fail(st_usage, label);
	if (!same_bytes(r.stderr_bytes, p.stderr_bytes))
		return fail(st_usage, label);
	return true;
}

void
test_printset_edge()
{
	cpuset_t mask;

	CPU_ZERO(&mask);
	check_printset("empty", &mask, 0);
	check_printset("empty_size8", &mask, 8);

	CPU_ZERO(&mask);
	CPU_SET(0, &mask);
	check_printset("single0", &mask, 16);

	CPU_ZERO(&mask);
	CPU_SET(7, &mask);
	check_printset("single7", &mask, 8);

	CPU_ZERO(&mask);
	CPU_SET(0, &mask);
	CPU_SET(2, &mask);
	CPU_SET(5, &mask);
	check_printset("sparse", &mask, 8);

	CPU_ZERO(&mask);
	for (int i = 0; i < 4; i++)
		CPU_SET(i, &mask);
	check_printset("dense4", &mask, 4);

	CPU_ZERO(&mask);
	CPU_SET(63, &mask);
	check_printset("bit63", &mask, 64);
}

void
test_printaffinity_edge()
{
	check_printaffinity("pid_which", CPU_LEVEL_WHICH, CPU_WHICH_PID, 42, 0, 0);
	check_printaffinity("tid", CPU_LEVEL_CPUSET, CPU_WHICH_TID, 3, 0, 0);
	check_printaffinity("dflag_skip_domain", CPU_LEVEL_ROOT, CPU_WHICH_DOMAIN,
	    1, 1, 0);
	check_printaffinity("xflag_skip_domain", CPU_LEVEL_WHICH, CPU_WHICH_IRQ,
	    9, 0, 1);
	check_printaffinity("jail", CPU_LEVEL_CPUSET, CPU_WHICH_JAIL, 5, 0, 0);
	check_printaffinity("cpuset", CPU_LEVEL_ROOT, CPU_WHICH_CPUSET, 2, 0, 0);
}

void
test_printsetid_edge()
{
	check_printsetid("which_pid", CPU_LEVEL_WHICH, CPU_WHICH_PID, 10, 0);
	check_printsetid("which_cpuset_nosflag", CPU_LEVEL_WHICH,
	    CPU_WHICH_CPUSET, 4, 0);
	check_printsetid("which_cpuset_sflag", CPU_LEVEL_WHICH, CPU_WHICH_CPUSET,
	    4, 1);
	check_printsetid("root_tid", CPU_LEVEL_ROOT, CPU_WHICH_TID, 8, 0);
	check_printsetid("cpuset_level", CPU_LEVEL_CPUSET, CPU_WHICH_PID, 1, 0);
}

void
test_main_edge()
{
	static char prog[] = "cpuset";
	static char dash_g[] = "-g";
	static char dash_i[] = "-i";
	static char dash_p[] = "-p";
	static char dash_t[] = "-t";
	static char dash_s[] = "-s";
	static char dash_l[] = "-l";
	static char dash_n[] = "-n";
	static char dash_C[] = "-C";
	static char dash_c[] = "-c";
	static char dash_d[] = "-d";
	static char dash_r[] = "-r";
	static char dash_j[] = "-j";
	static char dash_x[] = "-x";
	static char pid5[] = "5";
	static char tid3[] = "3";
	static char set2[] = "2";
	static char dom1[] = "1";
	static char irq7[] = "7";
	static char list01[] = "0,1";
	static char list_range[] = "0-2,5";
	static char policy[] = "round-robin:0-1";
	static char jailnm[] = "myjail";
	static char cmd[] = "/bin/true";
	static char badopt[] = "-Z";
	static char extra[] = "x";

	char *av_usage[] = { prog, nullptr };
	check_main("no_args_usage", 1, av_usage);

	char *av_g[] = { prog, dash_g, nullptr };
	check_main("g_affinity", 2, av_g);

	char *av_gi[] = { prog, dash_g, dash_i, dash_p, pid5, nullptr };
	check_main("g_i_pid", 5, av_gi);

	char *av_g_conflict[] = { prog, dash_g, dash_p, pid5, dash_t, tid3,
	    nullptr };
	check_main("g_two_id", 6, av_g_conflict);

	char *av_g_C[] = { prog, dash_g, dash_C, nullptr };
	check_main("g_C_usage", 3, av_g_C);

	char *av_d[] = { prog, dash_d, dom1, nullptr };
	check_main("d_no_g", 3, av_d);

	char *av_r[] = { prog, dash_r, nullptr };
	check_main("r_usage", 2, av_r);

	char *av_ls[] = { prog, dash_l, list01, dash_s, set2, nullptr };
	check_main("l_s_modify", 5, av_ls);

	char *av_lp[] = { prog, dash_l, list01, dash_p, pid5, nullptr };
	check_main("l_p_modify", 5, av_lp);

	char *av_Clp[] = { prog, dash_C, dash_l, list01, dash_p, pid5, nullptr };
	check_main("C_l_p", 6, av_Clp);

	char *av_cmd[] = { prog, dash_l, list01, cmd, nullptr };
	check_main("exec_cmd", 4, av_cmd);

	char *av_scmd[] = { prog, dash_s, set2, dash_l, list_range, cmd,
	    nullptr };
	check_main("s_l_cmd", 6, av_scmd);

	char *av_t[] = { prog, dash_t, tid3, dash_l, list01, nullptr };
	check_main("t_l", 5, av_t);

	char *av_x[] = { prog, dash_x, irq7, dash_l, list01, nullptr };
	check_main("x_l", 5, av_x);

	char *av_tp[] = { prog, dash_t, tid3, dash_p, pid5, nullptr };
	check_main("t_p_usage", 5, av_tp);

	char *av_xp[] = { prog, dash_x, irq7, dash_p, pid5, nullptr };
	check_main("x_p_usage", 5, av_xp);

	char *av_j[] = { prog, dash_j, jailnm, dash_l, list01, nullptr };
	check_main("j_l", 5, av_j);

	char *av_n[] = { prog, dash_n, policy, dash_p, pid5, nullptr };
	check_main("n_p", 5, av_n);

	char *av_c_only[] = { prog, dash_c, nullptr };
	check_main("c_only_usage", 2, av_c_only);

	char *av_l_only[] = { prog, dash_l, list01, nullptr };
	check_main("l_only_usage", 3, av_l_only);

	char *av_bad[] = { prog, badopt, nullptr };
	check_main("bad_opt", 2, av_bad);

	char *av_g_extra[] = { prog, dash_g, extra, nullptr };
	check_main("g_extra_arg", 3, av_g_extra);

	char *av_g_l[] = { prog, dash_g, dash_l, list01, nullptr };
	check_main("g_l_usage", 4, av_g_l);

	char *av_g_n[] = { prog, dash_g, dash_n, policy, nullptr };
	check_main("g_n_usage", 4, av_g_n);

	char *av_C_j[] = { prog, dash_C, dash_j, jailnm, nullptr };
	check_main("C_j_usage", 4, av_C_j);
}

void
sweep_printset()
{
	for (long i = 0; i < SWEEP; i++) {
		cpuset_t mask;
		int size = rng.bits(0, 128);
		CPU_ZERO(&mask);
		int nbits = rng.bits(0, size > 0 ? size : 1);
		for (int b = 0; b < nbits; b++) {
			int bit = rng.bits(0, size > 0 ? size - 1 : 0);
			if (size > 0)
				CPU_SET(bit, &mask);
		}
		char label[64];
		std::snprintf(label, sizeof(label), "rand_%ld", i);
		check_printset(label, &mask, size);
	}
}

void
sweep_printaffinity()
{
	for (long i = 0; i < SWEEP / 5; i++) {
		cpulevel_t lv = (cpulevel_t)rng.bits(1, 3);
		cpuwhich_t wh = (cpuwhich_t)rng.bits(1, 6);
		id_t ident = (id_t)rng.bits(-20, 200);
		int df = rng.coin() ? 1 : 0;
		int xf = rng.coin() ? 1 : 0;
		if (df)
			xf = 0;
		char label[64];
		std::snprintf(label, sizeof(label), "rand_%ld", i);
		check_printaffinity(label, lv, wh, ident, df, xf);
	}
}

void
sweep_printsetid()
{
	for (long i = 0; i < SWEEP / 5; i++) {
		cpulevel_t lv = (cpulevel_t)rng.bits(1, 3);
		cpuwhich_t wh = (cpuwhich_t)rng.bits(1, 6);
		id_t ident = (id_t)rng.bits(-5, 100);
		int sf = rng.coin() ? 1 : 0;
		char label[64];
		std::snprintf(label, sizeof(label), "rand_%ld", i);
		check_printsetid(label, lv, wh, ident, sf);
	}
}

void
sweep_main()
{
	static char prog[] = "cpuset";
	static char dash_g[] = "-g";
	static char dash_i[] = "-i";
	static char dash_p[] = "-p";
	static char dash_l[] = "-l";
	static char dash_s[] = "-s";
	static char dash_t[] = "-t";
	static char dash_x[] = "-x";
	static char dash_C[] = "-C";
	static char dash_n[] = "-n";
	static char cmd[] = "/bin/false";
	static char pidbuf[16];
	static char setbuf[16];
	static char listbuf[32];
	static char polbuf[48];

	for (long i = 0; i < SWEEP; i++) {
		int mode = rng.bits(0, 12);
		std::snprintf(pidbuf, sizeof(pidbuf), "%d", rng.bits(1, 500));
		std::snprintf(setbuf, sizeof(setbuf), "%d", rng.bits(0, 50));
		std::snprintf(listbuf, sizeof(listbuf), "%d,%d",
		    rng.bits(0, 7), rng.bits(0, 7));
		std::snprintf(polbuf, sizeof(polbuf), "rr:%d-%d",
		    rng.bits(0, 3), rng.bits(2, 5));

		char *argv[12];
		int argc = 0;
		argv[argc++] = prog;

		switch (mode) {
		case 0:
			argv[argc++] = dash_g;
			break;
		case 1:
			argv[argc++] = dash_g;
			argv[argc++] = dash_i;
			argv[argc++] = dash_p;
			argv[argc++] = pidbuf;
			break;
		case 2:
			argv[argc++] = dash_l;
			argv[argc++] = listbuf;
			argv[argc++] = dash_s;
			argv[argc++] = setbuf;
			break;
		case 3:
			argv[argc++] = dash_l;
			argv[argc++] = listbuf;
			argv[argc++] = dash_p;
			argv[argc++] = pidbuf;
			break;
		case 4:
			argv[argc++] = dash_C;
			argv[argc++] = dash_l;
			argv[argc++] = listbuf;
			argv[argc++] = dash_p;
			argv[argc++] = pidbuf;
			break;
		case 5:
			argv[argc++] = dash_l;
			argv[argc++] = listbuf;
			argv[argc++] = cmd;
			break;
		case 6:
			argv[argc++] = dash_s;
			argv[argc++] = setbuf;
			argv[argc++] = dash_l;
			argv[argc++] = listbuf;
			argv[argc++] = cmd;
			break;
		case 7:
			argv[argc++] = dash_t;
			argv[argc++] = pidbuf;
			argv[argc++] = dash_l;
			argv[argc++] = listbuf;
			break;
		case 8:
			argv[argc++] = dash_x;
			argv[argc++] = pidbuf;
			argv[argc++] = dash_l;
			argv[argc++] = listbuf;
			break;
		case 9:
			argv[argc++] = dash_n;
			argv[argc++] = polbuf;
			argv[argc++] = dash_p;
			argv[argc++] = pidbuf;
			break;
		case 10:
			argv[argc++] = dash_g;
			argv[argc++] = dash_p;
			argv[argc++] = pidbuf;
			break;
		default:
			break;
		}
		argv[argc] = nullptr;
		char label[64];
		std::snprintf(label, sizeof(label), "rand_%ld", i);
		check_main(label, argc, argv);
	}
}

void
sweep_usage()
{
	for (long i = 0; i < 1000; i++) {
		char label[64];
		std::snprintf(label, sizeof(label), "rand_%ld", i);
		check_usage(label);
	}
}

void
print_table()
{
	Stat *all[] = { &st_printset, &st_printaffinity, &st_printsetid,
		&st_main, &st_usage };
	std::printf("\n%-16s %8s %8s\n", "function", "cases", "failures");
	std::printf("%-16s %8s %8s\n", "--------", "-----", "--------");
	long total_cases = 0;
	long total_fails = 0;
	for (Stat *st : all) {
		std::printf("%-16s %8ld %8ld\n", st->name, st->cases,
		    st->fails);
		total_cases += st->cases;
		total_fails += st->fails;
	}
	std::printf("%-16s %8ld %8ld\n", "TOTAL", total_cases, total_fails);
}

} // namespace

int
main()
{
	mock_reset();
	test_printset_edge();
	test_printaffinity_edge();
	test_printsetid_edge();
	test_main_edge();
	check_usage("once");
	sweep_printset();
	sweep_printaffinity();
	sweep_printsetid();
	sweep_main();
	sweep_usage();
	print_table();
	long total_fails = st_printset.fails + st_printaffinity.fails +
	    st_printsetid.fails + st_main.fails + st_usage.fails;
	return total_fails == 0 ? 0 : 1;
}
