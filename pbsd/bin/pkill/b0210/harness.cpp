/*
 * PBSD batch b0210 - differential harness for bin/pkill/pkill.c
 */

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <paths.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

import pbsd.bin.pkill.b0210;

namespace P = pbsd::bin_pkill::b0210;

extern "C" {
struct kinfo_proc;
struct listhead;
enum listtype {
	LT_GENERIC, LT_USER, LT_GROUP, LT_TTY, LT_PGRP, LT_JAIL, LT_SID,
	LT_CLASS
};

void ref_usage_call(void);
void ref_show_process_call(const struct kinfo_proc *kp);
int ref_killact_call(const struct kinfo_proc *kp);
int ref_grepact_call(const struct kinfo_proc *kp);
void ref_makelist_call(struct listhead *, enum listtype, char *);
int ref_takepid_call(const char *, int);
void ref_set_pgrep(int);
void ref_set_quiet(int);
void ref_set_longfmt(int);
void ref_set_matchargs(int);
void ref_set_interactive(int);
void ref_set_signum(int);
void ref_set_kd(void *);
void ref_set_mypid(pid_t);
void ref_set_delim(const char *);
void ref_set_progname(const char *);
void ref_list_clear(struct listhead *);
int ref_list_count(struct listhead *);
long ref_list_number(struct listhead *, int);
const char *ref_list_name(struct listhead *, int);
struct listhead *ref_ppidlist(void);
struct listhead *ref_ruidlist(void);
struct listhead *ref_rgidlist(void);
struct listhead *ref_pgrplist(void);
struct listhead *ref_tdevlist(void);
struct listhead *ref_sidlist(void);
struct listhead *ref_jidlist(void);
struct listhead *ref_classlist(void);
}

static int mock_kill_fail;
static int mock_kill_errno = ESRCH;
static char mock_argv_storage[8][64];
static char *mock_argv_ptrs[9];
static int mock_argv_valid;
static int mock_jail_id = -1;
static char mock_jail_name[64];
static struct passwd mock_pw;
static char mock_pw_name[32];
static int mock_pw_valid;
static struct group mock_gr;
static char mock_gr_name[32];
static int mock_gr_valid;
static int mock_stat_mode;
static dev_t mock_stat_rdev;
static int mock_stat_fail;
static int mock_stat_errno = ENOENT;
static char mock_stat_path[256];
static int mock_stat_path_set;

extern "C" int __real_stat(const char *, struct stat *);

extern "C" int __wrap_kill(pid_t pid, int sig)
{
	(void)pid;
	(void)sig;
	if (mock_kill_fail) {
		errno = mock_kill_errno;
		return (-1);
	}
	return (0);
}

extern "C" char **__wrap_kvm_getargv(void *kd, const struct kinfo_proc *kp,
    int nchr)
{
	(void)kd;
	(void)kp;
	(void)nchr;
	return (mock_argv_valid ? mock_argv_ptrs : NULL);
}

extern "C" int __wrap_jail_getid(const char *name)
{
	if (mock_jail_id >= 0 && strcmp(name, mock_jail_name) == 0)
		return (mock_jail_id);
	return (-1);
}

extern "C" struct passwd *__wrap_getpwnam(const char *name)
{
	if (!mock_pw_valid || strcmp(name, mock_pw_name) != 0)
		return (NULL);
	return (&mock_pw);
}

extern "C" struct group *__wrap_getgrnam(const char *name)
{
	if (!mock_gr_valid || strcmp(name, mock_gr_name) != 0)
		return (NULL);
	return (&mock_gr);
}

extern "C" int __wrap_stat(const char *path, struct stat *st)
{
	if (mock_stat_path_set && strcmp(path, mock_stat_path) == 0) {
		if (mock_stat_fail) {
			errno = mock_stat_errno;
			return (-1);
		}
		memset(st, 0, sizeof(*st));
		st->st_mode = mock_stat_mode;
		st->st_rdev = mock_stat_rdev;
		return (0);
	}
	return (__real_stat(path, st));
}

enum Fn { F_USAGE, F_SHOW, F_KILL, F_GREP, F_MAKE, F_TAKE, F_N };

static struct {
	const char *name;
	unsigned long cases, fails, shown;
} stats[F_N] = {
	{ "usage", 0, 0, 0 }, { "show_process", 0, 0, 0 },
	{ "killact", 0, 0, 0 }, { "grepact", 0, 0, 0 },
	{ "makelist", 0, 0, 0 }, { "takepid", 0, 0, 0 },
};

static void fail(int f, const char *fmt, ...)
{
	va_list ap;
	stats[f].fails++;
	if (stats[f].shown >= 12)
		return;
	stats[f].shown++;
	fprintf(stderr, "FAIL %s: ", stats[f].name);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
}

static const unsigned long SWEEP_ITERS = 200000UL;

static uint64_t rng;

static void rng_seed(uint64_t s) { rng = s ? s : 0xb0210ULL; }
static uint64_t rng_next(void)
{
	uint64_t x = rng;
	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	rng = x;
	return (x * 0x2545f4914f6cdd1dULL);
}
static uint32_t rng_u32(void) { return (uint32_t)(rng_next() >> 32); }

#define	CAP_SZ 512
#define	CAP_WIN 256

struct Capture {
	unsigned char buf[CAP_SZ];
	long nread;
	int spawn_error, wait_error, status;
};

static void mock_reset(void)
{
	mock_kill_fail = 0;
	mock_kill_errno = ESRCH;
	mock_argv_valid = 0;
	mock_jail_id = -1;
	mock_pw_valid = 0;
	mock_gr_valid = 0;
	mock_stat_path_set = 0;
}

static void mock_argv_set(const char *a0, const char *a1)
{
	int n = 0;
	mock_argv_valid = 1;
	if (a0) {
		strncpy(mock_argv_storage[n], a0, 63);
		mock_argv_ptrs[n++] = mock_argv_storage[n - 1];
	}
	if (a1) {
		strncpy(mock_argv_storage[n], a1, 63);
		mock_argv_ptrs[n++] = mock_argv_storage[n - 1];
	}
	mock_argv_ptrs[n] = NULL;
}

static void mock_stat_set(const char *path, int mode, dev_t rdev)
{
	strncpy(mock_stat_path, path, sizeof(mock_stat_path) - 1);
	mock_stat_path_set = 1;
	mock_stat_fail = 0;
	mock_stat_mode = mode;
	mock_stat_rdev = rdev;
}

static void sync_globals(int pg, int q, int lf, int ma, int intr, int sig,
    const char *dl)
{
	P::set_pgrep(pg); ref_set_pgrep(pg);
	P::set_quiet(q); ref_set_quiet(q);
	P::set_longfmt(lf); ref_set_longfmt(lf);
	P::set_matchargs(ma); ref_set_matchargs(ma);
	P::set_interactive(intr); ref_set_interactive(intr);
	P::set_signum(sig); ref_set_signum(sig);
	P::set_delim(dl); ref_set_delim(dl);
	P::set_kd((P::kvm_t *)0x42); ref_set_kd((void *)0x42);
	P::set_mypid(getpid()); ref_set_mypid(getpid());
}

static P::kinfo_proc make_kp(pid_t pid, const char *comm)
{
	P::kinfo_proc kp;
	memset(&kp, 0, sizeof(kp));
	kp.ki_pid = pid;
	strncpy(kp.ki_comm, comm, sizeof(kp.ki_comm) - 1);
	return (kp);
}

static void run_child_stderr(void (*fn)(void), Capture *cap)
{
	int fds[2];
	pid_t pid, w;
	ssize_t n;
	size_t off;

	memset(cap->buf, 0x7f, sizeof(cap->buf));
	cap->nread = cap->spawn_error = cap->wait_error = 0;
	cap->status = -1;
	if (pipe(fds) == -1) { cap->spawn_error = errno; return; }
	fflush(NULL);
	pid = fork();
	if (pid == -1) {
		cap->spawn_error = errno;
		close(fds[0]); close(fds[1]);
		return;
	}
	if (pid == 0) {
		close(fds[0]);
		dup2(fds[1], STDERR_FILENO);
		close(fds[1]);
		fn();
		_exit(120);
	}
	close(fds[1]);
	for (off = 0;;) {
		if (off >= (size_t)CAP_WIN) {
			char s[128];
			n = read(fds[0], s, sizeof(s));
			if (n <= 0) break;
			cap->nread += n;
			continue;
		}
		n = read(fds[0], cap->buf + off, CAP_WIN - off);
		if (n <= 0) break;
		off += (size_t)n;
		cap->nread += n;
	}
	close(fds[0]);
	do { w = waitpid(pid, &cap->status, 0); } while (w == -1 && errno == EINTR);
	if (w == -1) cap->wait_error = errno;
}

static int caps_eq(const Capture *a, const Capture *b)
{
	size_t k;
	if (a->spawn_error != b->spawn_error || a->wait_error != b->wait_error)
		return (0);
	if (a->nread != b->nread || a->status != b->status)
		return (0);
	for (k = 0; k < CAP_SZ; k++)
		if (a->buf[k] != b->buf[k])
			return (0);
	return (1);
}

static void port_usage_fn(void) { P::usage(); }

static void check_usage(int pg, const char *prog)
{
	Capture cp, cr;
	stats[F_USAGE].cases++;
	P::set_pgrep(pg); ref_set_pgrep(pg);
	P::set_progname(prog); ref_set_progname(prog);
	run_child_stderr(port_usage_fn, &cp);
	run_child_stderr(ref_usage_call, &cr);
	if (!caps_eq(&cp, &cr))
		fail(F_USAGE, "pg=%d prog=%s", pg, prog);
}

static void test_usage(void)
{
	unsigned long i;
	check_usage(0, "pkill");
	check_usage(1, "pgrep");
	rng_seed(0xb02100001ULL);
	for (i = 0; i < SWEEP_ITERS; i++)
		check_usage((int)(rng_u32() & 1), (rng_u32() & 1) ? "pgrep" : "pkill");
}

static void capture_stdout(void (*fn)(void), Capture *cap)
{
	int fds[2];
	pid_t pid;
	ssize_t n;

	memset(cap->buf, 0x7f, sizeof(cap->buf));
	cap->nread = 0;
	cap->status = 0;
	if (pipe(fds) == -1) return;
	fflush(NULL);
	pid = fork();
	if (pid == 0) {
		dup2(fds[1], STDOUT_FILENO);
		close(fds[1]); close(fds[0]);
		fn();
		_exit(0);
	}
	close(fds[1]);
	n = read(fds[0], cap->buf, CAP_SZ);
	if (n > 0) cap->nread = n;
	close(fds[0]);
	waitpid(pid, &cap->status, 0);
}

struct ShowArgs { P::kinfo_proc kp; int port; };

static void show_fn(void)
{
	/* unused - use direct calls in capture_show */
}

static void capture_show(const P::kinfo_proc *kp, int port, Capture *cap)
{
	int fds[2];
	pid_t pid;
	ssize_t n;

	memset(cap->buf, 0x7f, sizeof(cap->buf));
	cap->nread = 0;
	if (pipe(fds) == -1) return;
	pid = fork();
	if (pid == 0) {
		dup2(fds[1], STDOUT_FILENO);
		close(fds[1]); close(fds[0]);
		if (port)
			P::show_process_call(kp);
		else
			ref_show_process_call((const struct kinfo_proc *)kp);
		_exit(0);
	}
	close(fds[1]);
	n = read(fds[0], cap->buf, CAP_SZ);
	if (n > 0) cap->nread = n;
	close(fds[0]);
	waitpid(pid, &cap->status, 0);
}

static void check_show(int pg, int q, int lf, int ma, pid_t pid,
    const char *comm, int argv)
{
	Capture cp, cr;
	P::kinfo_proc kp = make_kp(pid, comm);
	stats[F_SHOW].cases++;
	if (q)
		pg = 1;
	memset(&cp, 0, sizeof(cp));
	memset(&cr, 0, sizeof(cr));
	sync_globals(pg, q, lf, ma, 0, SIGTERM, "\n");
	if (argv) mock_argv_set(comm, "-a"); else mock_argv_valid = 0;
	capture_show(&kp, 1, &cp);
	capture_show(&kp, 0, &cr);
	if (!caps_eq(&cp, &cr))
		fail(F_SHOW, "pg=%d q=%d lf=%d ma=%d pid=%ld", pg, q, lf, ma,
		    (long)pid);
}

static void test_show_process(void)
{
	unsigned long i;
	check_show(1, 0, 0, 0, 42, "sh", 0);
	check_show(1, 0, 1, 0, 99, "vi", 0);
	check_show(0, 0, 1, 0, 7, "init", 0);
	check_show(1, 1, 0, 0, 1, "q", 0);
	check_show(0, 0, 0, 1, 100, "foo", 1);
	check_show(1, 0, 0, 0, 200, "\x80\xff", 0);
	rng_seed(0xb02100002ULL);
	for (i = 0; i < SWEEP_ITERS; i++) {
		char comm[20];
		size_t j, len = (rng_u32() % 18) + 1;
		int q = (int)(rng_u32() & 1);
		int pg = (int)(rng_u32() & 1);
		if (q)
			pg = 1;
		for (j = 0; j < len; j++) comm[j] = (char)(rng_u32() & 0xff);
		comm[len] = '\0';
		check_show(pg, q, (int)(rng_u32() & 1), (int)(rng_u32() & 1),
		    (pid_t)(rng_u32() % 100000), comm, (int)(rng_u32() & 1));
	}
}

static int run_kill(const P::kinfo_proc *kp, const char *in, int port)
{
	int st, rv = -1, fds[2], nullfd;
	pid_t pid;

	if (!in) {
		if (port)
			return (P::killact(kp));
		return (ref_killact_call((const struct kinfo_proc *)kp));
	}

	if (pipe(fds) == -1)
		return (-1);
	nullfd = open("/dev/null", O_WRONLY);
	pid = fork();
	if (pid == 0) {
		(void)write(fds[0], in, strlen(in));
		close(fds[0]);
		dup2(fds[1], STDIN_FILENO);
		close(fds[1]);
		if (nullfd != -1) {
			dup2(nullfd, STDOUT_FILENO);
			dup2(nullfd, STDERR_FILENO);
			close(nullfd);
		}
		_exit(port ? P::killact(kp) : ref_killact_call(
		    (const struct kinfo_proc *)kp));
	}
	close(fds[0]);
	close(fds[1]);
	if (nullfd != -1)
		close(nullfd);
	waitpid(pid, &st, 0);
	if (WIFEXITED(st))
		rv = WEXITSTATUS(st);
	return (rv);
}

static void check_kill(int intr, const char *in, int kf, int ke, pid_t pid)
{
	P::kinfo_proc kp = make_kp(pid, "k");
	int rp, rr;
	stats[F_KILL].cases++;
	sync_globals(0, 0, 0, 0, intr, SIGTERM, "\n");
	mock_kill_fail = kf;
	mock_kill_errno = ke;
	rp = run_kill(&kp, in, 1);
	rr = run_kill(&kp, in, 0);
	if (rp != rr) fail(F_KILL, "intr=%d rv %d vs %d", intr, rp, rr);
}

static void test_killact(void)
{
	unsigned long i;
	check_kill(0, NULL, 0, 0, 123);
	check_kill(0, NULL, 1, ESRCH, 124);
	check_kill(0, NULL, 1, EPERM, 125);
	check_kill(1, "y\n", 0, 0, 126);
	check_kill(1, "Y\n", 0, 0, 127);
	check_kill(1, "n\n", 0, 0, 128);
	check_kill(1, "N\n", 0, 0, 129);
	check_kill(1, "", 0, 0, 130);
	rng_seed(0xb02100003ULL);
	for (i = 0; i < SWEEP_ITERS; i++) {
		static const char *ans[] = { "y\n", "Y\n", "n\n", "N\n", "x\n", "" };
		int intr = (int)(rng_u32() & 1);
		int kf = (int)(rng_u32() % 3);
		check_kill(intr, intr ? ans[rng_u32() % 6] : NULL,
		    kf ? 1 : 0, kf == 2 ? EPERM : ESRCH,
		    (pid_t)(rng_u32() % 200000));
	}
}

static void capture_grep(int port, int pg, int q, const char *dl, pid_t pid,
    const char *comm, int n, Capture *cap)
{
	int fds[2];
	pid_t child;
	int k;

	memset(cap->buf, 0x7f, sizeof(cap->buf));
	cap->nread = 0;
	cap->status = 0;
	if (pipe(fds) == -1) return;
	fflush(NULL);
	child = fork();
	if (child == 0) {
		sync_globals(pg, q, 0, 0, 0, SIGTERM, dl);
		dup2(fds[1], STDOUT_FILENO);
		close(fds[1]); close(fds[0]);
		P::kinfo_proc kp = make_kp(pid, comm);
		for (k = 0; k < n; k++) {
			if (port) P::grepact(&kp);
			else ref_grepact_call((const struct kinfo_proc *)&kp);
		}
		fflush(stdout);
		_exit(0);
	}
	close(fds[1]);
	{
		ssize_t nread = read(fds[0], cap->buf, CAP_SZ);
		if (nread > 0)
			cap->nread = nread;
	}
	close(fds[0]);
	waitpid(child, &cap->status, 0);
}

static void check_grep(int pg, int q, const char *dl, pid_t pid,
    const char *comm, int n)
{
	Capture cp, cr;
	stats[F_GREP].cases++;
	if (q)
		pg = 1;
	memset(&cp, 0, sizeof(cp));
	memset(&cr, 0, sizeof(cr));
	capture_grep(1, pg, q, dl, pid, comm, n, &cp);
	capture_grep(0, pg, q, dl, pid, comm, n, &cr);
	if (!caps_eq(&cp, &cr))
		fail(F_GREP, "pg=%d q=%d n=%d", pg, q, n);
}

static void test_grepact(void)
{
	unsigned long i;
	check_grep(1, 0, "\n", 10, "a", 1);
	check_grep(1, 0, "\n", 11, "b", 2);
	check_grep(1, 0, "|", 12, "c", 2);
	check_grep(1, 1, "\n", 13, "d", 2);
	check_grep(1, 0, "", 14, "e", 2);
	rng_seed(0xb02100004ULL);
	for (i = 0; i < SWEEP_ITERS; i++) {
		char comm[12], dl[3];
		int q = (int)(rng_u32() & 1);
		int pg = (int)(rng_u32() & 1);
		size_t j;
		if (q)
			pg = 1;
		memset(comm, 0, sizeof(comm));
		for (j = 0; j < sizeof(comm) - 1; j++)
			comm[j] = (char)(rng_u32() & 0xff);
		dl[0] = (char)(rng_u32() & 0xff);
		dl[1] = '\0';
		check_grep(pg, q, dl, (pid_t)(rng_u32() % 50000),
		    comm, (int)(rng_u32() % 4) + 1);
	}
}

static void check_makelist_ok(enum listtype ty, const char *src,
    P::listhead *(*ph)(), struct listhead *(*rh)())
{
	char buf[256];
	int pc, rc, i;
	stats[F_MAKE].cases++;
	strncpy(buf, src, sizeof(buf) - 1); buf[sizeof(buf)-1] = '\0';
	P::list_clear(ph()); ref_list_clear(rh());
	P::set_mypid(getpid()); ref_set_mypid(getpid());
	P::makelist(ph(), (P::listtype)ty, buf);
	strncpy(buf, src, sizeof(buf) - 1); buf[sizeof(buf)-1] = '\0';
	ref_makelist_call(rh(), ty, buf);
	pc = P::list_count(ph()); rc = ref_list_count(rh());
	if (pc != rc) { fail(F_MAKE, "count %d vs %d", pc, rc); return; }
	for (i = 0; i < pc; i++) {
		long pn = P::list_number(ph(), i), rn = ref_list_number(rh(), i);
		const char *pna = P::list_name(ph(), i), *rna = ref_list_name(rh(), i);
		if (pn != rn || (pna == NULL) != (rna == NULL) ||
		    (pna && rna && strcmp(pna, rna) != 0))
			fail(F_MAKE, "idx %d num %ld/%ld", i, pn, rn);
	}
}

static void check_makelist_die(enum listtype ty, const char *src)
{
	char buf[256];
	pid_t p1, p2;
	int s1, s2;
	stats[F_MAKE].cases++;
	strncpy(buf, src, sizeof(buf) - 1);
	p1 = fork();
	if (p1 == 0) {
		P::list_clear(P::get_ppidlist());
		P::makelist(P::get_ppidlist(), (P::listtype)ty, buf);
		_exit(0);
	}
	waitpid(p1, &s1, 0);
	strncpy(buf, src, sizeof(buf) - 1);
	p2 = fork();
	if (p2 == 0) {
		ref_list_clear(ref_ppidlist());
		ref_makelist_call(ref_ppidlist(), ty, buf);
		_exit(0);
	}
	waitpid(p2, &s2, 0);
	if (s1 != s2) fail(F_MAKE, "die status 0x%x vs 0x%x", s1, s2);
}

static void test_makelist(void)
{
	unsigned long i;
	char path[128];

	mock_reset();
	check_makelist_ok(LT_GENERIC, "1,2,3", P::get_ppidlist, ref_ppidlist);
	check_makelist_ok(LT_GENERIC, "0x10,020", P::get_ppidlist, ref_ppidlist);
	check_makelist_ok(LT_PGRP, "0", P::get_pgrplist, ref_pgrplist);
	check_makelist_ok(LT_SID, "0", P::get_sidlist, ref_sidlist);
	mock_pw_valid = 1; strcpy(mock_pw_name, "testu"); mock_pw.pw_uid = 1234;
	check_makelist_ok(LT_USER, "testu", P::get_ruidlist, ref_ruidlist);
	mock_gr_valid = 1; strcpy(mock_gr_name, "testg"); mock_gr.gr_gid = 5678;
	check_makelist_ok(LT_GROUP, "testg", P::get_rgidlist, ref_rgidlist);
	check_makelist_ok(LT_JAIL, "none", P::get_jidlist, ref_jidlist);
	check_makelist_ok(LT_JAIL, "any", P::get_jidlist, ref_jidlist);
	mock_jail_id = 42; strcpy(mock_jail_name, "myjail");
	check_makelist_ok(LT_JAIL, "myjail", P::get_jidlist, ref_jidlist);
	check_makelist_ok(LT_JAIL, "7", P::get_jidlist, ref_jidlist);
	check_makelist_ok(LT_CLASS, "daemon,staff", P::get_classlist, ref_classlist);
	snprintf(path, sizeof(path), "%sconsole", _PATH_DEV);
	mock_stat_set(path, S_IFCHR, 0x0500);
	check_makelist_ok(LT_TTY, "co", P::get_tdevlist, ref_tdevlist);
	check_makelist_ok(LT_TTY, "-", P::get_tdevlist, ref_tdevlist);
	check_makelist_die(LT_GENERIC, "1,,2");
	check_makelist_die(LT_USER, "nosuchuser_x");
	check_makelist_die(LT_JAIL, "-5");
	rng_seed(0xb02100005ULL);
	for (i = 0; i < SWEEP_ITERS; i++) {
		char src[64];
		switch (rng_u32() % 5) {
		case 0:
			snprintf(src, sizeof(src), "%ld", (long)(rng_u32() % 10000));
			check_makelist_ok(LT_GENERIC, src, P::get_ppidlist, ref_ppidlist);
			break;
		case 1:
			check_makelist_ok(LT_JAIL, (rng_u32() & 1) ? "none" : "any",
			    P::get_jidlist, ref_jidlist);
			break;
		case 2:
			check_makelist_ok(LT_CLASS, "c", P::get_classlist, ref_classlist);
			break;
		case 3:
			mock_pw_valid = 1;
			snprintf(mock_pw_name, sizeof(mock_pw_name), "u%lu", i & 0xff);
			mock_pw.pw_uid = (uid_t)(rng_u32() % 60000);
			check_makelist_ok(LT_USER, mock_pw_name, P::get_ruidlist, ref_ruidlist);
			break;
		default:
			check_makelist_ok(LT_TTY, "-", P::get_tdevlist, ref_tdevlist);
			break;
		}
	}
}

static int write_pidfile(const char *path, const char *txt)
{
	FILE *f = fopen(path, "w");
	if (!f) return -1;
	fputs(txt, f);
	fclose(f);
	return 0;
}

static void check_takepid_ok(const char *txt)
{
	char path[] = "/tmp/pbsd_b0210_XXXXXX";
	int fd = mkstemp(path);
	int rp, rr;
	if (fd == -1) return;
	close(fd);
	write_pidfile(path, txt);
	stats[F_TAKE].cases++;
	rp = P::takepid(path, 0);
	rr = ref_takepid_call(path, 0);
	unlink(path);
	if (rp != rr) fail(F_TAKE, "rv %d vs %d for `%s`", rp, rr, txt);
}

static void check_takepid_die(const char *txt)
{
	char path[] = "/tmp/pbsd_b0210_XXXXXX";
	int fd = mkstemp(path);
	pid_t p1, p2;
	int s1, s2;
	if (fd == -1) return;
	close(fd);
	write_pidfile(path, txt);
	stats[F_TAKE].cases++;
	p1 = fork();
	if (p1 == 0) { P::takepid(path, 0); _exit(99); }
	waitpid(p1, &s1, 0);
	p2 = fork();
	if (p2 == 0) { ref_takepid_call(path, 0); _exit(99); }
	waitpid(p2, &s2, 0);
	unlink(path);
	if (s1 != s2) fail(F_TAKE, "die 0x%x vs 0x%x", s1, s2);
}

static void test_takepid(void)
{
	unsigned long i;
	char buf[32];
	check_takepid_ok("5\n");
	check_takepid_ok("99999\n");
	check_takepid_ok("12345\n");
	check_takepid_ok("42 \t\n");
	check_takepid_die("4\n");
	check_takepid_die("100000\n");
	check_takepid_die("abc\n");
	check_takepid_die("");
	rng_seed(0xb02100006ULL);
	for (i = 0; i < SWEEP_ITERS; i++) {
		long v = (long)(rng_u32() % 200000);
		int k = (int)(rng_u32() % 4);
		if (k == 0 && v >= 5 && v <= 99999)
			snprintf(buf, sizeof(buf), "%ld\n", v);
		else if (k == 1 && v >= 5 && v <= 99999)
			snprintf(buf, sizeof(buf), " %ld \n", v);
		else
			snprintf(buf, sizeof(buf), "%ld\n", v);
		if (v >= 5 && v <= 99999 && buf[0] >= '0' && buf[0] <= '9' && k <= 1)
			check_takepid_ok(buf);
		else
			check_takepid_die(buf);
	}
}

int main(void)
{
	unsigned long tc = 0, tf = 0;
	size_t f;

	mock_reset();
	printf("pbsd b0210 bin/pkill -- differential harness\n\n");
	test_usage();
	test_show_process();
	test_killact();
	test_grepact();
	test_makelist();
	test_takepid();
	printf("  %-20s %12s %12s  %s\n", "function", "cases", "failures", "result");
	printf("  %-20s %12s %12s  %s\n", "--------------------",
	    "------------", "------------", "------");
	for (f = 0; f < F_N; f++) {
		printf("  %-20s %12lu %12lu  %s\n", stats[f].name,
		    stats[f].cases, stats[f].fails,
		    stats[f].fails == 0 ? "ok" : "FAILED");
		tc += stats[f].cases;
		tf += stats[f].fails;
	}
	printf("\n  total cases   : %lu\n", tc);
	printf("  total failures: %lu\n", tf);
	printf(tf ? "\nRESULT: FAIL\n" : "\nRESULT: PASS\n");
	return (tf ? 1 : 0);
}
