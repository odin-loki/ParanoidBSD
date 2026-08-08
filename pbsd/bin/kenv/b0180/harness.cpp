/*
 * harness.cpp -- differential test for PBSD batch b0180 (kenv.c).
 */

#define _GNU_SOURCE

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

import pbsd.bin.kenv.b0180;

namespace P = pbsd::bin_kenv::b0180;

extern "C" {
int ref_main(int argc, char **argv);
void ref_usage(void);
int ref_kdumpenv(int dump_type);
int ref_kgetenv(const char *env);
int ref_ksetenv(const char *env, char *val);
int ref_kunsetenv(const char *env);
extern int ref_hflag;
extern int ref_lflag;
extern int ref_Nflag;
extern int ref_qflag;
extern int ref_sflag;
extern int ref_uflag;
extern int ref_vflag;
}

namespace {

enum {
	KENV_GET = 0,
	KENV_SET = 1,
	KENV_UNSET = 2,
	KENV_DUMP = 3,
	KENV_DUMP_STATIC = 6,
	KENV_DUMP_LOADER = 7,
};

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

Rng rng(0xb0180faceULL);

Stat st_usage = { "usage", 0, 0, 0 };
Stat st_kdumpenv = { "kdumpenv", 0, 0, 0 };
Stat st_kgetenv = { "kgetenv", 0, 0, 0 };
Stat st_ksetenv = { "ksetenv", 0, 0, 0 };
Stat st_kunsetenv = { "kunsetenv", 0, 0, 0 };
Stat st_main = { "main", 0, 0, 0 };

struct DumpEntry {
	std::string name;
	std::string value;
};

struct MockState {
	std::vector<DumpEntry> dumps[3];
	std::vector<std::string> raw_blobs[3];
	std::vector<std::pair<std::string, std::string>> getvars;

	int fail_action;
	int fail_errno;
	bool undersize_once;
	bool fail_dump_fill;
	int calloc_fail;
};

MockState g_mock;

int
dump_bucket(int dump_type)
{
	if (dump_type == KENV_DUMP_STATIC)
		return 1;
	if (dump_type == KENV_DUMP_LOADER)
		return 2;
	return 0;
}

std::size_t
dump_total(const std::vector<DumpEntry> &entries,
    const std::vector<std::string> &raw)
{
	std::size_t total = 0;
	for (const auto &e : entries)
		total += e.name.size() + 1 + e.value.size() + 1;
	for (const auto &r : raw)
		total += r.size() + 1;
	return total;
}

std::size_t
serialize_dump(const std::vector<DumpEntry> &entries,
    const std::vector<std::string> &raw, char *buf, int buflen)
{
	std::size_t total = dump_total(entries, raw);
	if (buf == nullptr || buflen <= 0)
		return total;
	std::size_t pos = 0;
	for (const auto &e : entries) {
		std::string s = e.name + "=" + e.value;
		std::size_t need = s.size() + 1;
		if (pos + need > (std::size_t)buflen)
			return total;
		std::memcpy(buf + pos, s.c_str(), need);
		pos += need;
	}
	for (const auto &r : raw) {
		std::size_t need = r.size() + 1;
		if (pos + need > (std::size_t)buflen)
			return total;
		std::memcpy(buf + pos, r.c_str(), need);
		pos += need;
	}
	return total;
}

void
mock_reset()
{
	g_mock.dumps[0].clear();
	g_mock.dumps[1].clear();
	g_mock.dumps[2].clear();
	g_mock.raw_blobs[0].clear();
	g_mock.raw_blobs[1].clear();
	g_mock.raw_blobs[2].clear();
	g_mock.getvars.clear();
	g_mock.fail_action = -1;
	g_mock.fail_errno = ENOENT;
	g_mock.undersize_once = false;
	g_mock.fail_dump_fill = false;
	g_mock.calloc_fail = 0;
}

void
mock_set_get(const char *name, const char *val)
{
	g_mock.getvars.emplace_back(name, val);
}

void
mock_set_dump(int bucket, const char *name, const char *val)
{
	g_mock.dumps[bucket].push_back({ name, val });
}

void
reset_ref_flags()
{
	ref_hflag = 0;
	ref_lflag = 0;
	ref_Nflag = 0;
	ref_qflag = 0;
	ref_sflag = 0;
	ref_uflag = 0;
	ref_vflag = 0;
}

void
reset_port_flags()
{
	P::hflag = 0;
	P::lflag = 0;
	P::Nflag = 0;
	P::qflag = 0;
	P::sflag = 0;
	P::uflag = 0;
	P::vflag = 0;
}

void
reset_flags()
{
	reset_ref_flags();
	reset_port_flags();
}

extern "C" void *
__real_calloc(size_t nmemb, size_t size);

extern "C" void *
__wrap_calloc(size_t nmemb, size_t size)
{
	if (g_mock.calloc_fail > 0) {
		g_mock.calloc_fail--;
		return nullptr;
	}
	return __real_calloc(nmemb, size);
}

extern "C" int
__wrap_kenv(int action, const char *name, char *value, int len)
{
	if (action == KENV_GET) {
		if (g_mock.fail_action == KENV_GET) {
			errno = g_mock.fail_errno;
			return -1;
		}
		for (const auto &kv : g_mock.getvars) {
			if (kv.first == name) {
				std::size_t n = kv.second.size();
				if (len > 0 && value != nullptr) {
					std::size_t cp = n;
					if (cp >= (std::size_t)len)
						cp = (std::size_t)len - 1;
					std::memcpy(value, kv.second.c_str(), cp);
					value[cp] = '\0';
				}
				return (int)n;
			}
		}
		errno = ENOENT;
		return -1;
	}

	if (action == KENV_SET) {
		if (g_mock.fail_action == KENV_SET) {
			errno = g_mock.fail_errno;
			return -1;
		}
		if (name == nullptr || value == nullptr || len <= 0) {
			errno = EINVAL;
			return -1;
		}
		std::string v(value, (std::size_t)len - 1);
		g_mock.getvars.erase(
		    std::remove_if(g_mock.getvars.begin(), g_mock.getvars.end(),
			[&](const auto &kv) { return kv.first == name; }),
		    g_mock.getvars.end());
		g_mock.getvars.emplace_back(name, v);
		return 0;
	}

	if (action == KENV_UNSET) {
		if (g_mock.fail_action == KENV_UNSET) {
			errno = g_mock.fail_errno;
			return -1;
		}
		bool found = false;
		for (auto it = g_mock.getvars.begin(); it != g_mock.getvars.end();) {
			if (it->first == name) {
				it = g_mock.getvars.erase(it);
				found = true;
			} else
				it++;
		}
		if (!found) {
			errno = ENOENT;
			return -1;
		}
		return 0;
	}

	if (action == KENV_DUMP || action == KENV_DUMP_STATIC ||
	    action == KENV_DUMP_LOADER) {
		int bucket = dump_bucket(action);
		std::size_t total = dump_total(g_mock.dumps[bucket],
		    g_mock.raw_blobs[bucket]);
		if (value == nullptr || len == 0) {
			if (g_mock.fail_action == action) {
				errno = g_mock.fail_errno;
				return -1;
			}
			return (int)total;
		}
		if (g_mock.fail_dump_fill || g_mock.fail_action == action) {
			errno = g_mock.fail_errno;
			return -1;
		}
		if (g_mock.undersize_once) {
			g_mock.undersize_once = false;
			if (total <= (std::size_t)len)
				return (int)((std::size_t)len + 1);
			return (int)total;
		}
		std::size_t wrote = serialize_dump(g_mock.dumps[bucket],
		    g_mock.raw_blobs[bucket], value, len);
		if (wrote > (std::size_t)len)
			return (int)wrote;
		return (int)wrote;
	}

	errno = EINVAL;
	return -1;
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

bool
same_bytes(const std::vector<unsigned char> &a,
    const std::vector<unsigned char> &b)
{
	return a == b;
}

struct FnRun {
	int ret;
	std::vector<unsigned char> stdout_bytes;
	std::vector<unsigned char> stderr_bytes;
};

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

FnRun
run_ref_kdumpenv(int dump_type)
{
	FnRun res{};
	IoCapture out;
	IoCapture err;
	if (!out.begin(STDOUT_FILENO) || !err.begin(STDERR_FILENO))
		return res;
	res.ret = ref_kdumpenv(dump_type);
	res.stdout_bytes = out.end(STDOUT_FILENO);
	res.stderr_bytes = err.end(STDERR_FILENO);
	return res;
}

FnRun
run_port_kdumpenv(int dump_type)
{
	FnRun res{};
	IoCapture out;
	IoCapture err;
	if (!out.begin(STDOUT_FILENO) || !err.begin(STDERR_FILENO))
		return res;
	res.ret = P::kdumpenv(dump_type);
	res.stdout_bytes = out.end(STDOUT_FILENO);
	res.stderr_bytes = err.end(STDERR_FILENO);
	return res;
}

bool
check_kdumpenv(const char *label, int dump_type)
{
	st_kdumpenv.cases++;
	FnRun r = run_ref_kdumpenv(dump_type);
	FnRun p = run_port_kdumpenv(dump_type);
	if (r.ret != p.ret) {
		std::printf("  [%s] ret ref=%d port=%d\n", label, r.ret, p.ret);
		return fail(st_kdumpenv, label);
	}
	if (!same_bytes(r.stdout_bytes, p.stdout_bytes))
		return fail(st_kdumpenv, label);
	if (!same_bytes(r.stderr_bytes, p.stderr_bytes))
		return fail(st_kdumpenv, label);
	return true;
}

FnRun
run_ref_kgetenv(const char *env)
{
	FnRun res{};
	IoCapture out;
	IoCapture err;
	if (!out.begin(STDOUT_FILENO) || !err.begin(STDERR_FILENO))
		return res;
	res.ret = ref_kgetenv(env);
	res.stdout_bytes = out.end(STDOUT_FILENO);
	res.stderr_bytes = err.end(STDERR_FILENO);
	return res;
}

FnRun
run_port_kgetenv(const char *env)
{
	FnRun res{};
	IoCapture out;
	IoCapture err;
	if (!out.begin(STDOUT_FILENO) || !err.begin(STDERR_FILENO))
		return res;
	res.ret = P::kgetenv(env);
	res.stdout_bytes = out.end(STDOUT_FILENO);
	res.stderr_bytes = err.end(STDERR_FILENO);
	return res;
}

bool
check_kgetenv(const char *label, const char *env)
{
	st_kgetenv.cases++;
	FnRun r = run_ref_kgetenv(env);
	FnRun p = run_port_kgetenv(env);
	if (r.ret != p.ret) {
		std::printf("  [%s] ret ref=%d port=%d\n", label, r.ret, p.ret);
		return fail(st_kgetenv, label);
	}
	if (!same_bytes(r.stdout_bytes, p.stdout_bytes))
		return fail(st_kgetenv, label);
	if (!same_bytes(r.stderr_bytes, p.stderr_bytes))
		return fail(st_kgetenv, label);
	return true;
}

FnRun
run_ref_ksetenv(const char *env, const char *val)
{
	FnRun res{};
	GuardBuf gb(val);
	IoCapture out;
	IoCapture err;
	if (!out.begin(STDOUT_FILENO) || !err.begin(STDERR_FILENO))
		return res;
	res.ret = ref_ksetenv(env, gb.data());
	res.stdout_bytes = out.end(STDOUT_FILENO);
	res.stderr_bytes = err.end(STDERR_FILENO);
	if (gb.raw != GuardBuf(val).raw)
		res.ret = -999;
	return res;
}

FnRun
run_port_ksetenv(const char *env, const char *val)
{
	FnRun res{};
	GuardBuf gb(val);
	IoCapture out;
	IoCapture err;
	if (!out.begin(STDOUT_FILENO) || !err.begin(STDERR_FILENO))
		return res;
	res.ret = P::ksetenv(env, gb.data());
	res.stdout_bytes = out.end(STDOUT_FILENO);
	res.stderr_bytes = err.end(STDERR_FILENO);
	if (gb.raw != GuardBuf(val).raw)
		res.ret = -999;
	return res;
}

bool
check_ksetenv(const char *label, const char *env, const char *val)
{
	st_ksetenv.cases++;
	GuardBuf gr(val);
	GuardBuf gp(val);
	FnRun r, p;
	IoCapture out, err;
	if (!out.begin(STDOUT_FILENO) || !err.begin(STDERR_FILENO))
		return fail(st_ksetenv, label);
	r.ret = ref_ksetenv(env, gr.data());
	r.stdout_bytes = out.end(STDOUT_FILENO);
	r.stderr_bytes = err.end(STDERR_FILENO);
	out.begin(STDOUT_FILENO);
	err.begin(STDERR_FILENO);
	p.ret = P::ksetenv(env, gp.data());
	p.stdout_bytes = out.end(STDOUT_FILENO);
	p.stderr_bytes = err.end(STDERR_FILENO);
	if (gr.raw != gp.raw) {
		std::printf("  [%s] valbuf mismatch\n", label);
		return fail(st_ksetenv, label);
	}
	if (r.ret != p.ret) {
		std::printf("  [%s] ret ref=%d port=%d\n", label, r.ret, p.ret);
		return fail(st_ksetenv, label);
	}
	if (!same_bytes(r.stdout_bytes, p.stdout_bytes))
		return fail(st_ksetenv, label);
	if (!same_bytes(r.stderr_bytes, p.stderr_bytes))
		return fail(st_ksetenv, label);
	return true;
}

bool
check_kunsetenv_pair(const char *label, const char *env)
{
	st_kunsetenv.cases++;
	int rr = ref_kunsetenv(env);
	int pr = P::kunsetenv(env);
	if (rr != pr) {
		std::printf("  [%s] ret ref=%d port=%d\n", label, rr, pr);
		return fail(st_kunsetenv, label);
	}
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
		optind = 1;
		opterr = 1;
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
		optind = 1;
		opterr = 1;
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
check_main_argv(const char *label, const std::vector<std::string> &args)
{
	st_main.cases++;
	std::vector<char *> argv1;
	std::vector<char *> argv2;
	argv1.push_back(const_cast<char *>("kenv"));
	argv2.push_back(const_cast<char *>("kenv"));
	std::vector<std::string> storage;
	for (const auto &a : args) {
		storage.push_back(a);
		argv1.push_back(const_cast<char *>(storage.back().c_str()));
		argv2.push_back(const_cast<char *>(storage.back().c_str()));
	}
	argv1.push_back(nullptr);
	argv2.push_back(nullptr);
	int argc = (int)args.size() + 1;
	ExitRun r = run_ref_main(argc, argv1.data());
	ExitRun p = run_port_main(argc, argv2.data());
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
rand_name(Rng &r, int maxlen, bool hint = false)
{
	int len = r.bits(0, maxlen);
	if (len == 0)
		len = 1;
	std::string s;
	if (hint)
		s = "hint.";
	for (int i = 0; i < len; i++) {
		unsigned char c = r.byte();
		if (c == '=' || c == '\0')
			c = 'a' + r.bits(0, 25);
		s.push_back((char)c);
	}
	return s;
}

std::string
rand_val(Rng &r, int maxlen)
{
	int len = r.bits(0, maxlen);
	std::string s;
	for (int i = 0; i < len; i++)
		s.push_back((char)r.byte());
	return s;
}

void
test_kdumpenv_edge()
{
	mock_reset();
	reset_flags();
	(void)check_kdumpenv("empty", KENV_DUMP);

	mock_reset();
	mock_set_dump(0, "a", "1");
	reset_flags();
	(void)check_kdumpenv("one", KENV_DUMP);

	mock_reset();
	mock_set_dump(0, "x", "y");
	mock_set_dump(0, "p", "q");
	reset_flags();
	(void)check_kdumpenv("two", KENV_DUMP);

	mock_reset();
	g_mock.raw_blobs[0].push_back("noequal");
	reset_flags();
	(void)check_kdumpenv("noeq", KENV_DUMP);

	mock_reset();
	mock_set_dump(0, "hint.foo", "bar");
	mock_set_dump(0, "other", "val");
	reset_flags();
	ref_hflag = 1;
	P::hflag = 1;
	(void)check_kdumpenv("hint_h", KENV_DUMP);

	mock_reset();
	mock_set_dump(0, "hint.x", "1");
	reset_flags();
	ref_hflag = 0;
	P::hflag = 0;
	(void)check_kdumpenv("hint_no", KENV_DUMP);

	mock_reset();
	mock_set_dump(0, "n", "v");
	reset_flags();
	ref_Nflag = 1;
	P::Nflag = 1;
	(void)check_kdumpenv("Nflag", KENV_DUMP);

	mock_reset();
	mock_set_dump(1, "s", "t");
	reset_flags();
	(void)check_kdumpenv("static", KENV_DUMP_STATIC);

	mock_reset();
	mock_set_dump(2, "l", "d");
	reset_flags();
	(void)check_kdumpenv("loader", KENV_DUMP_LOADER);

	mock_reset();
	g_mock.fail_action = KENV_DUMP;
	g_mock.fail_errno = ENOENT;
	reset_flags();
	(void)check_kdumpenv("fail_size", KENV_DUMP);

	mock_reset();
	mock_set_dump(0, "a", "b");
	g_mock.fail_dump_fill = true;
	g_mock.fail_errno = EIO;
	reset_flags();
	(void)check_kdumpenv("fail_fill", KENV_DUMP);

	mock_reset();
	mock_set_dump(0, "grow", "data");
	g_mock.undersize_once = true;
	reset_flags();
	(void)check_kdumpenv("undersize", KENV_DUMP);

	mock_reset();
	mock_set_dump(0, "c", "d");
	g_mock.calloc_fail = 1;
	reset_flags();
	(void)check_kdumpenv("calloc_fail", KENV_DUMP);

	mock_reset();
	mock_set_dump(0, "\xff", "\x80");
	reset_flags();
	(void)check_kdumpenv("highbit", KENV_DUMP);
}

void
test_kdumpenv_random()
{
	for (long i = 0; i < SWEEP; i++) {
		mock_reset();
		int bucket = rng.bits(0, 2);
		int nent = rng.bits(0, 5);
		for (int j = 0; j < nent; j++) {
			bool hint = rng.coin();
			std::string nm = rand_name(rng, 8, hint);
			std::string vl = rand_val(rng, 12);
			mock_set_dump(bucket, nm.c_str(), vl.c_str());
		}
		reset_flags();
		if (rng.coin())
			ref_hflag = P::hflag = 1;
		if (rng.coin())
			ref_Nflag = P::Nflag = 1;
		if (rng.bits(0, 20) == 0) {
			g_mock.undersize_once = true;
		}
		int dt = KENV_DUMP;
		if (bucket == 1)
			dt = KENV_DUMP_STATIC;
		else if (bucket == 2)
			dt = KENV_DUMP_LOADER;
		char label[64];
		std::snprintf(label, sizeof(label), "kd%ld", i);
		(void)check_kdumpenv(label, dt);
	}
}

void
test_kgetenv_edge()
{
	mock_reset();
	mock_set_get("VAR", "val");
	reset_flags();
	(void)check_kgetenv("ok", "VAR");

	mock_reset();
	reset_flags();
	(void)check_kgetenv("miss", "MISSING");

	mock_reset();
	mock_set_get("V", "");
	reset_flags();
	ref_vflag = P::vflag = 1;
	(void)check_kgetenv("empty_v", "V");

	mock_reset();
	mock_set_get("E", "x");
	reset_flags();
	ref_vflag = P::vflag = 0;
	(void)check_kgetenv("plain", "E");

	mock_reset();
	mock_set_get("\xff", "\x80\xfe");
	reset_flags();
	(void)check_kgetenv("highbit", "\xff");

	mock_reset();
	g_mock.fail_action = KENV_GET;
	g_mock.fail_errno = ENOENT;
	reset_flags();
	(void)check_kgetenv("fail", "X");
}

void
test_kgetenv_random()
{
	for (long i = 0; i < SWEEP; i++) {
		mock_reset();
		std::string nm = rand_name(rng, 10);
		if (rng.coin())
			mock_set_get(nm.c_str(), rand_val(rng, 20).c_str());
		reset_flags();
		if (rng.coin())
			ref_vflag = P::vflag = 1;
		char label[64];
		std::snprintf(label, sizeof(label), "kg%ld", i);
		(void)check_kgetenv(label, nm.c_str());
	}
}

void
test_ksetenv_edge()
{
	mock_reset();
	reset_flags();
	(void)check_ksetenv("ok", "A", "val");

	mock_reset();
	g_mock.fail_action = KENV_SET;
	g_mock.fail_errno = EACCES;
	reset_flags();
	(void)check_ksetenv("fail", "A", "v");

	mock_reset();
	reset_flags();
	(void)check_ksetenv("empty", "E", "");

	mock_reset();
	reset_flags();
	(void)check_ksetenv("high", "\xff", "\x80\xff");
}

void
test_ksetenv_random()
{
	for (long i = 0; i < SWEEP; i++) {
		mock_reset();
		reset_flags();
		std::string nm = rand_name(rng, 10);
		std::string vl = rand_val(rng, 20);
		if (rng.bits(0, 30) == 0)
			g_mock.fail_action = KENV_SET;
		char label[64];
		std::snprintf(label, sizeof(label), "ks%ld", i);
		(void)check_ksetenv(label, nm.c_str(), vl.c_str());
	}
}

void
test_kunsetenv_edge()
{
	mock_reset();
	mock_set_get("U", "v");
	reset_flags();
	(void)check_kunsetenv_pair("ok", "U");

	mock_reset();
	reset_flags();
	(void)check_kunsetenv_pair("miss", "NONE");

	mock_reset();
	mock_set_get("F", "x");
	g_mock.fail_action = KENV_UNSET;
	g_mock.fail_errno = EPERM;
	reset_flags();
	(void)check_kunsetenv_pair("fail", "F");
}

void
test_kunsetenv_random()
{
	for (long i = 0; i < SWEEP; i++) {
		mock_reset();
		std::string nm = rand_name(rng, 10);
		if (rng.coin())
			mock_set_get(nm.c_str(), "v");
		if (rng.bits(0, 30) == 0)
			g_mock.fail_action = KENV_UNSET;
		reset_flags();
		char label[64];
		std::snprintf(label, sizeof(label), "ku%ld", i);
		(void)check_kunsetenv_pair(label, nm.c_str());
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
	mock_reset();
	mock_set_dump(0, "a", "1");
	reset_flags();
	(void)check_main_argv("dump", {});

	mock_reset();
	mock_set_dump(1, "s", "2");
	reset_flags();
	(void)check_main_argv("dump_s", { "-s" });

	mock_reset();
	mock_set_dump(2, "l", "3");
	reset_flags();
	(void)check_main_argv("dump_l", { "-l" });

	mock_reset();
	g_mock.fail_action = KENV_DUMP;
	g_mock.fail_errno = ENOENT;
	reset_flags();
	(void)check_main_argv("dump_fail", {});

	mock_reset();
	g_mock.fail_action = KENV_DUMP;
	g_mock.fail_errno = ENOENT;
	reset_flags();
	(void)check_main_argv("dump_fail_q", { "-q" });

	mock_reset();
	mock_set_get("V", "hello");
	reset_flags();
	(void)check_main_argv("get", { "V" });

	mock_reset();
	mock_set_get("V", "hello");
	reset_flags();
	(void)check_main_argv("get_v", { "-v", "V" });

	mock_reset();
	reset_flags();
	(void)check_main_argv("get_miss", { "MISS" });

	mock_reset();
	reset_flags();
	(void)check_main_argv("set", { "K=val" });

	mock_reset();
	g_mock.fail_action = KENV_SET;
	reset_flags();
	(void)check_main_argv("set_fail", { "K=val" });

	mock_reset();
	mock_set_get("R", "x");
	reset_flags();
	(void)check_main_argv("unset", { "-u", "R" });

	mock_reset();
	reset_flags();
	(void)check_main_argv("unset_miss", { "-u", "NONE" });

	mock_reset();
	reset_flags();
	(void)check_main_argv("bad_opt", { "-z" });

	mock_reset();
	reset_flags();
	(void)check_main_argv("ls_both", { "-l", "-s" });

	mock_reset();
	reset_flags();
	(void)check_main_argv("extra", { "a", "b" });

	mock_reset();
	reset_flags();
	(void)check_main_argv("h_env", { "-h", "x=y" });

	mock_reset();
	reset_flags();
	(void)check_main_argv("N_env", { "-N", "x=y" });

	mock_reset();
	reset_flags();
	(void)check_main_argv("u_noenv", { "-u" });

	mock_reset();
	reset_flags();
	(void)check_main_argv("v_noenv", { "-v" });

	mock_reset();
	mock_set_dump(0, "hint.h", "1");
	mock_set_dump(0, "other", "2");
	reset_flags();
	(void)check_main_argv("dump_h", { "-h" });

	mock_reset();
	mock_set_dump(0, "n", "v");
	reset_flags();
	(void)check_main_argv("dump_N", { "-N" });
}

void
test_main_random()
{
	for (long i = 0; i < SWEEP; i++) {
		mock_reset();
		int mode = rng.bits(0, 8);
		std::vector<std::string> args;

		if (mode == 0) {
			int n = rng.bits(0, 4);
			for (int j = 0; j < n; j++)
				mock_set_dump(0, rand_name(rng, 6).c_str(),
				    rand_val(rng, 8).c_str());
			if (rng.coin())
				args.push_back("-q");
			if (rng.coin())
				args.push_back("-h");
			if (rng.coin())
				args.push_back("-N");
		} else if (mode == 1) {
			mock_set_dump(1, rand_name(rng, 6).c_str(),
			    rand_val(rng, 8).c_str());
			args.push_back("-s");
		} else if (mode == 2) {
			mock_set_dump(2, rand_name(rng, 6).c_str(),
			    rand_val(rng, 8).c_str());
			args.push_back("-l");
		} else if (mode == 3) {
			std::string nm = rand_name(rng, 8);
			mock_set_get(nm.c_str(), rand_val(rng, 12).c_str());
			if (rng.coin())
				args.push_back("-v");
			if (rng.coin())
				args.push_back("-q");
			args.push_back(nm);
		} else if (mode == 4) {
			std::string nm = rand_name(rng, 8);
			if (rng.coin())
				args.push_back("-q");
			args.push_back(nm);
		} else if (mode == 5) {
			std::string nm = rand_name(rng, 8);
			std::string vl = rand_val(rng, 12);
			if (rng.coin())
				args.push_back("-q");
			args.push_back(nm + "=" + vl);
		} else if (mode == 6) {
			std::string nm = rand_name(rng, 8);
			mock_set_get(nm.c_str(), "x");
			if (rng.coin())
				args.push_back("-q");
			args.push_back("-u");
			args.push_back(nm);
		} else {
			if (rng.coin())
				args.push_back("-l");
			else if (rng.coin())
				args.push_back("-s");
			if (rng.bits(0, 20) == 0)
				g_mock.fail_action = KENV_DUMP;
		}

		reset_flags();
		char label[64];
		std::snprintf(label, sizeof(label), "main%ld", i);
		(void)check_main_argv(label, args);
	}
}

} // namespace

int
main()
{
	test_kdumpenv_edge();
	test_kgetenv_edge();
	test_ksetenv_edge();
	test_kunsetenv_edge();
	test_usage_edge();
	test_main_edge();

	test_kdumpenv_random();
	test_kgetenv_random();
	test_ksetenv_random();
	test_kunsetenv_random();
	test_usage_random();
	test_main_random();

	Stat *all[] = {
		&st_usage,
		&st_kdumpenv,
		&st_kgetenv,
		&st_ksetenv,
		&st_kunsetenv,
		&st_main,
	};
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
