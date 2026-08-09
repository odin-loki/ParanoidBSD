/*
 * Differential harness for PBSD batch b0291 (ps fmt/keyword/print).
 */

#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <clocale>
#include <initializer_list>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>
#include <setjmp.h>

import pbsd.bin.ps.b0291;

namespace P = pbsd::bin_ps::b0291;

typedef int fixpt_t;

#define STAILQ_HEAD(name, type) \
	struct name { struct type *stqh_first; struct type **stqh_last; }
#define STAILQ_ENTRY(type) struct { struct type *stqe_next; }
#define STAILQ_FIRST(head) ((head)->stqh_first)
#define STAILQ_NEXT(elm, field) ((elm)->field.stqe_next)
#define STAILQ_INIT(head) do { \
	(head)->stqh_first = nullptr; \
	(head)->stqh_last = &(head)->stqh_first; \
} while (0)
#define STAILQ_INSERT_TAIL(head, elm, field) do { \
	(elm)->field.stqe_next = nullptr; \
	*(head)->stqh_last = (elm); \
	(head)->stqh_last = &(elm)->field.stqe_next; \
} while (0)

STAILQ_HEAD(velisthead, varent);

extern "C" {

fixpt_t ccpu = 0;
int cflag = 0;
int eval = 0;
int fscale = 2048;
int nlistread = 0;
int rawcpu = 0;
unsigned long mempages = 1024;
time_t now = 1700000000;
int showthreads = 0;
int sumrusage = 0;
int termwidth = 80;

struct velisthead varlist;

jmp_buf b0291_err_jmp;
int b0291_err_jmp_set = 0;
int b0291_errx_code = 0;

struct velisthead *
b0291_get_varlist(void)
{
	return &varlist;
}

} // extern "C"

static jmp_buf exit_jmp;
static int exit_jmp_set = 0;
static int exit_code = 0;

static int malloc_fail_at = 0;
static int malloc_calls = 0;

static char xo_log[65536];
static std::size_t xo_log_len = 0;

static int devname_mode = 0;
static char devname_buf[64];
static char jailname_buf[64];
static int jail_getname_fail = 0;
static int mac_prepare_fail = 0;
static int mac_get_pid_fail = 0;
static int mac_to_text_fail = 0;
static char mac_text_buf[128];

extern "C" {

struct var;
typedef struct var VAR;

struct var {
	const char *name;
	union {
		const char *aliased;
		const VAR *final_kw;
	};
	const char *header;
	const char *field;
	unsigned int flag;
	char *(*oproc)(struct kinfo *, struct varent *);
	std::size_t off;
	int type;
	const char *fmt;
};

typedef struct varent {
	STAILQ_ENTRY(varent) next_ve;
	const char *header;
	const VAR *var;
	unsigned int width;
	std::uint16_t flags;
} VARENT;

struct priority {
	unsigned char pri_class;
	unsigned char pri_level;
	unsigned char pri_native;
	unsigned char pri_user;
};

struct kinfo_proc {
	int ki_structsize;
	int ki_layout;
	void *ki_args;
	void *ki_paddr;
	void *ki_addr;
	void *ki_tracep;
	void *ki_textvp;
	void *ki_fd;
	void *ki_vmspace;
	const void *ki_wchan;
	pid_t ki_pid;
	pid_t ki_ppid;
	pid_t ki_pgid;
	pid_t ki_tpgid;
	pid_t ki_sid;
	pid_t ki_tsid;
	short ki_jobc;
	short ki_spare_short1;
	std::uint32_t ki_tdev_freebsd11;
	std::uint32_t ki_siglist[4];
	std::uint32_t ki_sigmask[4];
	std::uint32_t ki_sigignore[4];
	std::uint32_t ki_sigcatch[4];
	uid_t ki_uid;
	uid_t ki_ruid;
	uid_t ki_svuid;
	gid_t ki_rgid;
	gid_t ki_svgid;
	short ki_ngroups;
	short ki_spare_short2;
	gid_t ki_groups[16];
	unsigned long ki_size;
	long ki_rssize;
	long ki_swrss;
	long ki_tsize;
	long ki_dsize;
	long ki_ssize;
	unsigned short ki_xstat;
	unsigned short ki_acflag;
	int ki_pctcpu;
	unsigned int ki_estcpu;
	unsigned int ki_slptime;
	unsigned int ki_swtime;
	unsigned int ki_cow;
	std::uint64_t ki_runtime;
	struct timeval ki_start;
	struct timeval ki_childtime;
	long ki_flag;
	long ki_kiflag;
	int ki_traceflag;
	char ki_stat;
	signed char ki_nice;
	char ki_lock;
	char ki_rqindex;
	unsigned char ki_oncpu_old;
	unsigned char ki_lastcpu_old;
	char ki_tdname[17];
	char ki_wmesg[9];
	char ki_login[18];
	char ki_lockname[9];
	char ki_comm[20];
	char ki_emul[17];
	char ki_loginclass[18];
	char ki_moretdname[17];
	char ki_sparestrings[38];
	int ki_spareints[2];
	pid_t ki_reaper;
	pid_t ki_reapsubtree;
	std::uint64_t ki_tdev;
	int ki_oncpu;
	int ki_lastcpu;
	int ki_tracer;
	int ki_flag2;
	int ki_fibnum;
	unsigned int ki_cr_flags;
	int ki_jid;
	int ki_numthreads;
	std::uint32_t ki_tid;
	long ki_tdflags;
	struct priority ki_pri;
	struct rusage ki_rusage;
	struct timeval ki_childutime;
	struct timeval ki_childstime;
};

typedef struct kinfo {
	struct kinfo_proc *ki_p;
	const char *ki_args;
	const char *ki_env;
	int ki_valid;
	double ki_pcpu;
	long ki_memsize;
	union {
		int level;
		char *prefix;
	} ki_d;
} KINFO;

extern VAR keywords[];
extern const size_t known_keywords_nb;

const char *ref_fmt_argv(char **, char *, char *, size_t);
size_t ref_aliased_keyword_index(const VAR *);
void ref_check_keywords(void);
void ref_resolve_aliases(void);
void ref_showkey(void);
void ref_parsefmt(const char *, struct velisthead *, int);
void ref_printheader(void);
char *ref_arguments(KINFO *, VARENT *);
char *ref_command(KINFO *, VARENT *);
char *ref_ucomm(KINFO *, VARENT *);
char *ref_tdnam(KINFO *, VARENT *);
char *ref_logname(KINFO *, VARENT *);
char *ref_state(KINFO *, VARENT *);
char *ref_pri(KINFO *, VARENT *);
char *ref_upr(KINFO *, VARENT *);
char *ref_username(KINFO *, VARENT *);
char *ref_egroupname(KINFO *, VARENT *);
char *ref_rgroupname(KINFO *, VARENT *);
char *ref_runame(KINFO *, VARENT *);
char *ref_tdev(KINFO *, VARENT *);
char *ref_tname(KINFO *, VARENT *);
char *ref_longtname(KINFO *, VARENT *);
char *ref_started(KINFO *, VARENT *);
char *ref_lstarted(KINFO *, VARENT *);
char *ref_lockname(KINFO *, VARENT *);
char *ref_wchan(KINFO *, VARENT *);
char *ref_nwchan(KINFO *, VARENT *);
char *ref_mwchan(KINFO *, VARENT *);
char *ref_vsize(KINFO *, VARENT *);
char *ref_cputime(KINFO *, VARENT *);
char *ref_cpunum(KINFO *, VARENT *);
char *ref_systime(KINFO *, VARENT *);
char *ref_usertime(KINFO *, VARENT *);
char *ref_elapsed(KINFO *, VARENT *);
char *ref_elapseds(KINFO *, VARENT *);
double ref_getpcpu(const KINFO *);
char *ref_pcpu(KINFO *, VARENT *);
char *ref_pmem(KINFO *, VARENT *);
char *ref_pagein(KINFO *, VARENT *);
char *ref_maxrss(KINFO *, VARENT *);
char *ref_priorityr(KINFO *, VARENT *);
char *ref_kvar(KINFO *, VARENT *);
char *ref_rvar(KINFO *, VARENT *);
char *ref_emulname(KINFO *, VARENT *);
char *ref_label(KINFO *, VARENT *);
char *ref_loginclass(KINFO *, VARENT *);
char *ref_jailname(KINFO *, VARENT *);
int ref_donlist(void);

void *__real_malloc(size_t);
void __real_exit(int);

void *
__wrap_malloc(size_t sz)
{
	malloc_calls++;
	if (malloc_fail_at != 0 && malloc_calls == malloc_fail_at)
		return (nullptr);
	return __real_malloc(sz);
}

void
__wrap_exit(int code)
{
	if (exit_jmp_set) {
		exit_code = code;
		longjmp(exit_jmp, 1);
	}
	__real_exit(code);
}

void
xo_reset(void)
{
	xo_log[0] = '\0';
	xo_log_len = 0;
}

void
xo_warnx(const char *fmt, ...)
{
	(void)fmt;
}

void
xo_warn(const char *fmt, ...)
{
	(void)fmt;
}

void
xo_err(int eval, const char *fmt, ...)
{
	(void)eval;
	(void)fmt;
}

void
xo_errx(int eval, const char *fmt, ...)
{
	(void)fmt;
	b0291_errx_code = eval;
	if (b0291_err_jmp_set)
		longjmp(b0291_err_jmp, eval);
	__real_exit(eval);
}

void
xo_open_list(const char *name)
{
	(void)name;
}

int
xo_emit(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	if (xo_log_len < sizeof(xo_log) - 1) {
		int n = vsnprintf(xo_log + xo_log_len,
		    sizeof(xo_log) - xo_log_len, fmt, ap);
		if (n > 0)
			xo_log_len += (std::size_t)n;
	}
	va_end(ap);
	return 0;
}

void
xo_close_list(const char *name)
{
	(void)name;
}

int
xo_finish(void)
{
	return 0;
}

char *
user_from_uid(uid_t uid, int nogroup)
{
	(void)nogroup;
	static char buf[32];
	snprintf(buf, sizeof(buf), "user%u", (unsigned)uid);
	return buf;
}

char *
group_from_gid(gid_t gid, int nogroup)
{
	(void)nogroup;
	static char buf[32];
	snprintf(buf, sizeof(buf), "grp%u", (unsigned)gid);
	return buf;
}

char *
devname(dev_t dev, mode_t type)
{
	(void)type;
	if (devname_mode == 0)
		return nullptr;
	snprintf(devname_buf, sizeof(devname_buf), "tty%llu",
	    (unsigned long long)dev);
	return strdup(devname_buf);
}

char *
jail_getname(int jid)
{
	(void)jid;
	if (jail_getname_fail)
		return nullptr;
	snprintf(jailname_buf, sizeof(jailname_buf), "jail%d", jid);
	return strdup(jailname_buf);
}

int
donlist(void)
{
	ccpu = 128;
	fscale = 2048;
	mempages = 4096;
	nlistread = 1;
	return 0;
}

int
ref_donlist(void)
{
	return donlist();
}

typedef void *mac_t;

int
mac_prepare_process_label(mac_t *label)
{
	if (mac_prepare_fail)
		return -1;
	*label = (mac_t)0x1;
	return 0;
}

int
mac_get_pid(pid_t pid, mac_t label)
{
	(void)pid;
	(void)label;
	return mac_get_pid_fail ? -1 : 0;
}

int
mac_to_text(mac_t label, char **text)
{
	(void)label;
	if (mac_to_text_fail) {
		*text = nullptr;
		return -1;
	}
	*text = strdup(mac_text_buf);
	snprintf(mac_text_buf, sizeof(mac_text_buf), "labeltext");
	return 0;
}

void
mac_free(mac_t label)
{
	(void)label;
}

} // extern "C"

/* ------------------------------------------------------------------ */
/* Test bookkeeping                                                   */
/* ------------------------------------------------------------------ */

enum FnIdx {
	FN_FMT_ARGV,
	FN_ALIASED_KW_IDX,
	FN_CHECK_KEYWORDS,
	FN_RESOLVE_ALIASES,
	FN_SHOWKEY,
	FN_PARSEFMT,
	FN_PRINTHEADER,
	FN_ARGUMENTS,
	FN_COMMAND,
	FN_UCOMM,
	FN_TDNAM,
	FN_LOGNAME,
	FN_STATE,
	FN_PRI,
	FN_UPR,
	FN_USERNAME,
	FN_EGROUPNAME,
	FN_RGROUPNAME,
	FN_RUNAME,
	FN_TDEV,
	FN_TNAME,
	FN_LONGTNAME,
	FN_STARTED,
	FN_LSTARTED,
	FN_LOCKNAME,
	FN_WCHAN,
	FN_NWCHAN,
	FN_MWCHAN,
	FN_VSIZE,
	FN_CPUTIME,
	FN_CPUNUM,
	FN_SYSTIME,
	FN_USERTIME,
	FN_ELAPSED,
	FN_ELAPSEDS,
	FN_GETPCPU,
	FN_PCPU,
	FN_PMEM,
	FN_PAGEIN,
	FN_MAXRSS,
	FN_PRIORITYR,
	FN_KVAR,
	FN_RVAR,
	FN_EMULNAME,
	FN_LABEL,
	FN_LOGINCLASS,
	FN_JAILNAME,
	NFUNC
};

static const char *const kFnName[NFUNC] = {
	"fmt_argv",
	"aliased_keyword_index",
	"check_keywords",
	"resolve_aliases",
	"showkey",
	"parsefmt",
	"printheader",
	"arguments",
	"command",
	"ucomm",
	"tdnam",
	"logname",
	"state",
	"pri",
	"upr",
	"username",
	"egroupname",
	"rgroupname",
	"runame",
	"tdev",
	"tname",
	"longtname",
	"started",
	"lstarted",
	"lockname",
	"wchan",
	"nwchan",
	"mwchan",
	"vsize",
	"cputime",
	"cpunum",
	"systime",
	"usertime",
	"elapsed",
	"elapseds",
	"getpcpu",
	"pcpu",
	"pmem",
	"pagein",
	"maxrss",
	"priorityr",
	"kvar",
	"rvar",
	"emulname",
	"label",
	"loginclass",
	"jailname",
};

static unsigned long long g_cases[NFUNC];
static unsigned long long g_fails[NFUNC];

static constexpr int MAX_REPORT = 20;
static int g_reported = 0;

static void
fail(FnIdx f, const char *ctx)
{
	g_fails[f]++;
	if (g_reported < MAX_REPORT) {
		g_reported++;
		std::printf("FAIL %-22s %s\n", kFnName[f], ctx);
	} else if (g_reported == MAX_REPORT) {
		g_reported++;
		std::printf("... further failures suppressed\n");
	}
}

static void
casebump(FnIdx f)
{
	g_cases[f]++;
}

/* ------------------------------------------------------------------ */
/* PRNG                                                               */
/* ------------------------------------------------------------------ */

static std::uint64_t rng = 0xb0291c0ffeeULL;

static std::uint64_t
rand64(void)
{
	std::uint64_t x = rng;
	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	rng = x;
	return x * 0x2545f4914f6cdd1dULL;
}

static std::uint32_t
randu32(void)
{
	return (std::uint32_t)rand64();
}

static int
randint(int lo, int hi)
{
	if (hi <= lo)
		return lo;
	return lo + (int)(randu32() % (std::uint32_t)(hi - lo + 1));
}

static void
fill_random_bytes(unsigned char *p, std::size_t n)
{
	for (std::size_t i = 0; i < n; i++)
		p[i] = (unsigned char)(randu32() & 0xffu);
}

static void
fill_random_cstr(char *p, std::size_t cap)
{
	std::size_t n = cap > 0 ? (std::size_t)randint(0, (int)cap - 1) : 0;
	for (std::size_t i = 0; i < n; i++)
		p[i] = (char)(1 + (randu32() % 254));
	p[n] = '\0';
}

/* ------------------------------------------------------------------ */
/* String compare helpers                                             */
/* ------------------------------------------------------------------ */

static bool
streq(const char *a, const char *b)
{
	if (a == nullptr && b == nullptr)
		return true;
	if (a == nullptr || b == nullptr)
		return false;
	return std::strcmp(a, b) == 0;
}

static bool
dbl_eq(double a, double b)
{
	return std::fabs(a - b) <= 1e-9 * (1.0 + std::fabs(a) + std::fabs(b));
}

/* ------------------------------------------------------------------ */
/* Fixture builders                                                   */
/* ------------------------------------------------------------------ */

struct Fixture {
	kinfo_proc kp{};
	KINFO ki{};
	VARENT ve{};
	VARENT ve_next{};
	char args[256];
	char env[256];
	char prefix[32];
};

static void
fixture_reset(Fixture &fx)
{
	std::memset(&fx, 0, sizeof(fx));
	fx.ki.ki_p = &fx.kp;
	fx.ki.ki_args = fx.args;
	fx.ki.ki_env = fx.env;
	fx.ki.ki_valid = 1;
	fx.ve.var = &keywords[0];
	fx.ve.header = "HDR";
	fx.ve.width = 3;
	STAILQ_INIT(&varlist);
}

static void
fixture_randomize(Fixture &fx, bool last_field)
{
	fill_random_cstr(fx.args, sizeof(fx.args) - 1);
	fill_random_cstr(fx.env, sizeof(fx.env) - 1);
	fill_random_cstr(fx.prefix, sizeof(fx.prefix) - 1);
	fill_random_cstr(fx.kp.ki_comm, sizeof(fx.kp.ki_comm) - 1);
	fill_random_cstr(fx.kp.ki_tdname, sizeof(fx.kp.ki_tdname) - 1);
	fill_random_cstr(fx.kp.ki_moretdname, sizeof(fx.kp.ki_moretdname) - 1);
	fill_random_cstr(fx.kp.ki_login, sizeof(fx.kp.ki_login) - 1);
	fill_random_cstr(fx.kp.ki_lockname, sizeof(fx.kp.ki_lockname) - 1);
	fill_random_cstr(fx.kp.ki_wmesg, sizeof(fx.kp.ki_wmesg) - 1);
	fill_random_cstr(fx.kp.ki_emul, sizeof(fx.kp.ki_emul) - 1);
	fill_random_cstr(fx.kp.ki_loginclass, sizeof(fx.kp.ki_loginclass) - 1);

	fx.ki.ki_d.prefix = (randu32() & 1) ? fx.prefix : nullptr;
	fx.ki.ki_valid = (int)(randu32() & 1);
	fx.ki.ki_env = (randu32() & 3) == 0 ? nullptr : fx.env;

	fx.kp.ki_stat = (char)randint(1, 7);
	fx.kp.ki_nice = (signed char)randint(-40, 40);
	fx.kp.ki_pri.pri_class = (unsigned char)randint(1, 4);
	fx.kp.ki_pri.pri_level = (unsigned char)randint(0, 255);
	fx.kp.ki_pri.pri_user = (unsigned char)randint(0, 255);
	fx.kp.ki_flag = (long)randu32();
	fx.kp.ki_kiflag = (long)randu32();
	fx.kp.ki_tdflags = (long)randu32();
	fx.kp.ki_cr_flags = randu32() & 1u;
	fx.kp.ki_lock = (char)randint(0, 3);
	fx.kp.ki_pgid = randint(1, 50000);
	fx.kp.ki_tpgid = randint(1, 50000);
	fx.kp.ki_slptime = (unsigned)randint(0, 100);
	fx.kp.ki_uid = randint(0, 60000);
	fx.kp.ki_ruid = randint(0, 60000);
	fx.kp.ki_groups[0] = randint(0, 60000);
	fx.kp.ki_rgid = randint(0, 60000);
	fx.kp.ki_tdev = (std::uint64_t)randu32();
	fx.kp.ki_start.tv_sec = now - randint(0, 14 * 86400);
	fx.kp.ki_wchan = (const void *)(std::uintptr_t)randu32();
	fx.kp.ki_wmesg[0] = (char)randint(0, 1);
	fx.kp.ki_lockname[0] = (char)randint(0, 1);
	fx.kp.ki_numthreads = (int)randint(1, 8);
	fx.kp.ki_size = randu32();
	fx.kp.ki_rssize = (long)randint(0, 100000);
	fx.kp.ki_runtime = (long)randint(0, 50000000);
	fx.kp.ki_rusage.ru_stime.tv_sec = randint(0, 10000);
	fx.kp.ki_rusage.ru_stime.tv_usec = randint(0, 999999);
	fx.kp.ki_rusage.ru_utime.tv_sec = randint(0, 10000);
	fx.kp.ki_rusage.ru_utime.tv_usec = randint(0, 999999);
	fx.kp.ki_rusage.ru_majflt = randint(0, 10000);
	fx.kp.ki_childtime.tv_sec = randint(0, 1000);
	fx.kp.ki_childtime.tv_usec = randint(0, 999999);
	fx.kp.ki_childutime.tv_sec = randint(0, 1000);
	fx.kp.ki_childutime.tv_usec = randint(0, 999999);
	fx.kp.ki_childstime.tv_sec = randint(0, 1000);
	fx.kp.ki_childstime.tv_usec = randint(0, 999999);
	fx.kp.ki_swtime = (unsigned)randint(1, 10000);
	fx.kp.ki_pctcpu = (int)randint(0, fscale);
	fx.kp.ki_oncpu = randint(-1, 8);
	fx.kp.ki_lastcpu = randint(0, 8);
	fx.kp.ki_jid = randint(0, 100);
	fx.kp.ki_pid = randint(1, 50000);
	fx.kp.ki_flag &= ~0x200; /* clear P_SYSTEM sometimes */
	if (randu32() & 1)
		fx.kp.ki_flag |= 0x200;

	const int stats[] = {1, 2, 3, 4, 5, 6, 7};
	fx.kp.ki_stat = (char)stats[randint(0, 6)];

	if (last_field) {
		STAILQ_INIT(&varlist);
		STAILQ_INSERT_TAIL(&varlist, &fx.ve, next_ve);
	} else {
		STAILQ_INIT(&varlist);
		STAILQ_INSERT_TAIL(&varlist, &fx.ve, next_ve);
		STAILQ_INSERT_TAIL(&varlist, &fx.ve_next, next_ve);
	}
}

static char *
dupstr(const char *s)
{
	if (s == nullptr)
		return nullptr;
	char *p = (char *)std::malloc(std::strlen(s) + 1);
	if (p)
		std::strcpy(p, s);
	return p;
}

typedef char *(*PrintFn)(KINFO *, VARENT *);

static void
test_print_pair(FnIdx idx, PrintFn ref_fn, PrintFn port_fn, Fixture &fx)
{
	casebump(idx);
	char *rs = ref_fn(&fx.ki, &fx.ve);
	char *ps = port_fn(&fx.ki, &fx.ve);
	if (!streq(rs, ps))
		fail(idx, "string mismatch");
	std::free(rs);
	std::free(ps);
}

/* ------------------------------------------------------------------ */
/* fmt_argv tests                                                     */
/* ------------------------------------------------------------------ */

static bool
fmt_result_owned(char **argv, char *cmd)
{
	if (argv == nullptr || argv[0] == nullptr)
		return cmd != nullptr;
	return true;
}

static void
test_fmt_argv_case(char **argv, char *cmd, char *thread, size_t maxlen)
{
	casebump(FN_FMT_ARGV);
	const char *rs = ref_fmt_argv(argv, cmd, thread, maxlen);
	const char *ps = P::fmt_argv(argv, cmd, thread, maxlen);
	if (!streq(rs, ps))
		fail(FN_FMT_ARGV, "result mismatch");
	if (fmt_result_owned(argv, cmd)) {
		std::free((void *)rs);
		std::free((void *)ps);
	}
}

static void
run_fmt_argv_tests(void)
{
	char a0[] = "";
	char a1[] = "one";
	char a2[] = "two words";
	char a3[] = "/bin/ls";
	char a4[] = "\xff\xfe";
	char *av0[] = {nullptr};
	char *av1[] = {a1, nullptr};
	char *av2[] = {a3, a2, nullptr};
	char *av3[] = {a4, a0, nullptr};
	char cmd[] = "ls";
	char thr[] = "main";
	char cmd2[] = "other";

	test_fmt_argv_case(nullptr, nullptr, nullptr, 10);
	test_fmt_argv_case(nullptr, cmd, nullptr, 10);
	test_fmt_argv_case(nullptr, cmd, thr, 3);
	test_fmt_argv_case(av0, cmd, nullptr, 10);
	test_fmt_argv_case(av1, cmd, nullptr, 10);
	test_fmt_argv_case(av2, cmd, nullptr, 0);
	test_fmt_argv_case(av2, cmd2, nullptr, 4);
	test_fmt_argv_case(av3, cmd, nullptr, 20);

	for (int i = 0; i < 200000; i++) {
		char pool[512];
		fill_random_bytes((unsigned char *)pool, sizeof(pool));
		pool[sizeof(pool) - 1] = '\0';
		char *argbuf = pool;
		char *pool_end = pool + sizeof(pool) - 1;
		char *argv_local[4];
		int argc = randint(0, 3);
		int actual = 0;
		for (int j = 0; j < argc; j++) {
			if (argbuf >= pool_end)
				break;
			argv_local[actual++] = argbuf;
			size_t room = (size_t)(pool_end - argbuf);
			size_t len = (size_t)(1 + (randu32() % 40));
			if (len >= room)
				len = room > 0 ? room - 1 : 0;
			argbuf[len] = '\0';
			argbuf += len + 1;
		}
		argv_local[actual] = nullptr;
		char cbuf[48], tbuf[48];
		fill_random_cstr(cbuf, sizeof(cbuf) - 1);
		fill_random_cstr(tbuf, sizeof(tbuf) - 1);
		size_t ml = (size_t)randint(0, 40);
		char *cmdp = (randu32() & 3) == 0 ? nullptr : cbuf;
		char *thrp = (randu32() & 3) == 0 ? nullptr : tbuf;
		char **avp = actual == 0 ? nullptr : argv_local;
		if (avp != nullptr && cmdp == nullptr)
			cmdp = cbuf;
		test_fmt_argv_case(avp, cmdp, thrp, ml);
	}
}

/* ------------------------------------------------------------------ */
/* keyword tests                                                      */
/* ------------------------------------------------------------------ */

static void
run_keyword_tests(void)
{
	ref_resolve_aliases();
	P::resolve_aliases();

	for (size_t i = 0; i < known_keywords_nb; i++) {
		casebump(FN_ALIASED_KW_IDX);
		size_t ri = ref_aliased_keyword_index(&keywords[i]);
		size_t pi = P::aliased_keyword_index(&P::keywords[i]);
		if (ri != pi)
			fail(FN_ALIASED_KW_IDX, "index mismatch");
	}

	casebump(FN_CHECK_KEYWORDS);
	ref_check_keywords();
	P::check_keywords();

	casebump(FN_RESOLVE_ALIASES);
	ref_resolve_aliases();
	P::resolve_aliases();

	for (int tw : {0, 40, 80, 200}) {
		casebump(FN_SHOWKEY);
		termwidth = tw;
		xo_reset();
		ref_showkey();
		char reflog[sizeof(xo_log)];
		std::strcpy(reflog, xo_log);
		xo_reset();
		P::showkey();
		if (std::strcmp(reflog, xo_log) != 0)
			fail(FN_SHOWKEY, "xo output mismatch");
	}

	static const char *fmts[] = {
		"pid",
		"pid,comm",
		"pid,state,uid",
		"args,command",
		"pcpu,pmem,time",
		"pid,comm,vsz,rss",
	};
	for (auto fmt : fmts) {
		casebump(FN_PARSEFMT);
		struct velisthead rl, pl;
		STAILQ_INIT(&rl);
		STAILQ_INIT(&pl);
		eval = 0;
		ref_parsefmt(fmt, &rl, 0);
		eval = 0;
		P::parsefmt(fmt, reinterpret_cast<P::velisthead *>(&pl), 0);
		/* lists should have same length */
		int rc = 0, pc = 0;
		for (auto *v = rl.stqh_first; v; v = v->next_ve.stqe_next)
			rc++;
		for (auto *v = pl.stqh_first; v; v = v->next_ve.stqe_next)
			pc++;
		if (rc != pc)
			fail(FN_PARSEFMT, "list length mismatch");
		while (rl.stqh_first) {
			auto *v = rl.stqh_first;
			rl.stqh_first = v->next_ve.stqe_next;
			if (v->header != v->var->header)
				std::free((void *)v->header);
			std::free(v);
		}
		while (pl.stqh_first) {
			auto *v = pl.stqh_first;
			pl.stqh_first = v->next_ve.stqe_next;
			if (v->header != reinterpret_cast<const P::VAR *>(v->var)->header)
				std::free((void *)v->header);
			std::free(v);
		}
	}

	for (int i = 0; i < 200000; i++) {
		casebump(FN_ALIASED_KW_IDX);
		size_t idx = (size_t)randint(0, (int)known_keywords_nb - 1);
		size_t ri = ref_aliased_keyword_index(&keywords[idx]);
		size_t pi = P::aliased_keyword_index(&P::keywords[idx]);
		if (ri != pi)
			fail(FN_ALIASED_KW_IDX, "random index mismatch");
	}
}

/* ------------------------------------------------------------------ */
/* print tests                                                        */
/* ------------------------------------------------------------------ */

static void
setup_varlist_for_header(void)
{
	STAILQ_INIT(&varlist);
	static VARENT v1, v2, v3;
	v1.header = "PID";
	v1.width = 3;
	v1.var = &keywords[60]; /* pid */
	v2.header = "COMM";
	v2.width = 8;
	v2.var = &keywords[20]; /* comm */
	v3.header = "";
	v3.width = 0;
	v3.var = &keywords[60];
	STAILQ_INSERT_TAIL(&varlist, &v1, next_ve);
	STAILQ_INSERT_TAIL(&varlist, &v2, next_ve);
	STAILQ_INSERT_TAIL(&varlist, &v3, next_ve);
}

static void
run_print_tests(void)
{
	nlistread = 0;
	donlist();

	casebump(FN_PRINTHEADER);
	setup_varlist_for_header();
	xo_reset();
	ref_printheader();
	char rhdr[sizeof(xo_log)];
	std::strcpy(rhdr, xo_log);
	xo_reset();
	P::printheader();
	if (std::strcmp(rhdr, xo_log) != 0)
		fail(FN_PRINTHEADER, "header mismatch");

	Fixture fx;
	fixture_reset(fx);

	struct TestEntry {
		FnIdx idx;
		PrintFn ref_fn;
		PrintFn port_fn;
	};

	const TestEntry table[] = {
		{FN_ARGUMENTS, ref_arguments, [](KINFO *k, VARENT *v) {
			return P::arguments(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_COMMAND, ref_command, [](KINFO *k, VARENT *v) {
			return P::command(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_UCOMM, ref_ucomm, [](KINFO *k, VARENT *v) {
			return P::ucomm(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_TDNAM, ref_tdnam, [](KINFO *k, VARENT *v) {
			return P::tdnam(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_LOGNAME, ref_logname, [](KINFO *k, VARENT *v) {
			return P::logname(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_STATE, ref_state, [](KINFO *k, VARENT *v) {
			return P::state(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_PRI, ref_pri, [](KINFO *k, VARENT *v) {
			return P::pri(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_UPR, ref_upr, [](KINFO *k, VARENT *v) {
			return P::upr(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_USERNAME, ref_username, [](KINFO *k, VARENT *v) {
			return P::username(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_EGROUPNAME, ref_egroupname, [](KINFO *k, VARENT *v) {
			return P::egroupname(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_RGROUPNAME, ref_rgroupname, [](KINFO *k, VARENT *v) {
			return P::rgroupname(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_RUNAME, ref_runame, [](KINFO *k, VARENT *v) {
			return P::runame(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_TDEV, ref_tdev, [](KINFO *k, VARENT *v) {
			return P::tdev(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_TNAME, ref_tname, [](KINFO *k, VARENT *v) {
			return P::tname(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_LONGTNAME, ref_longtname, [](KINFO *k, VARENT *v) {
			return P::longtname(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_STARTED, ref_started, [](KINFO *k, VARENT *v) {
			return P::started(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_LSTARTED, ref_lstarted, [](KINFO *k, VARENT *v) {
			return P::lstarted(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_LOCKNAME, ref_lockname, [](KINFO *k, VARENT *v) {
			return P::lockname(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_WCHAN, ref_wchan, [](KINFO *k, VARENT *v) {
			return P::wchan(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_NWCHAN, ref_nwchan, [](KINFO *k, VARENT *v) {
			return P::nwchan(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_MWCHAN, ref_mwchan, [](KINFO *k, VARENT *v) {
			return P::mwchan(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_VSIZE, ref_vsize, [](KINFO *k, VARENT *v) {
			return P::vsize(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_CPUTIME, ref_cputime, [](KINFO *k, VARENT *v) {
			return P::cputime(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_CPUNUM, ref_cpunum, [](KINFO *k, VARENT *v) {
			return P::cpunum(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_SYSTIME, ref_systime, [](KINFO *k, VARENT *v) {
			return P::systime(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_USERTIME, ref_usertime, [](KINFO *k, VARENT *v) {
			return P::usertime(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_ELAPSED, ref_elapsed, [](KINFO *k, VARENT *v) {
			return P::elapsed(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_ELAPSEDS, ref_elapseds, [](KINFO *k, VARENT *v) {
			return P::elapseds(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_PCPU, ref_pcpu, [](KINFO *k, VARENT *v) {
			return P::pcpu(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_PMEM, ref_pmem, [](KINFO *k, VARENT *v) {
			return P::pmem(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_PAGEIN, ref_pagein, [](KINFO *k, VARENT *v) {
			return P::pagein(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_MAXRSS, ref_maxrss, [](KINFO *k, VARENT *v) {
			return P::maxrss(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_PRIORITYR, ref_priorityr, [](KINFO *k, VARENT *v) {
			return P::priorityr(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_EMULNAME, ref_emulname, [](KINFO *k, VARENT *v) {
			return P::emulname(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_LABEL, ref_label, [](KINFO *k, VARENT *v) {
			return P::label(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_LOGINCLASS, ref_loginclass, [](KINFO *k, VARENT *v) {
			return P::loginclass(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
		{FN_JAILNAME, ref_jailname, [](KINFO *k, VARENT *v) {
			return P::jailname(reinterpret_cast<P::KINFO *>(k),
			    reinterpret_cast<P::VARENT *>(v));
		}},
	};

	for (bool last : {false, true}) {
		for (bool c : {false, true}) {
			for (bool st : {false, true}) {
				for (bool sr : {false, true}) {
					cflag = c ? 1 : 0;
					showthreads = st ? 1 : 0;
					sumrusage = sr ? 1 : 0;
					fixture_reset(fx);
					fixture_randomize(fx, last);
					for (auto &te : table)
						test_print_pair(te.idx, te.ref_fn, te.port_fn, fx);
					casebump(FN_GETPCPU);
					double rd = ref_getpcpu(&fx.ki);
					double pd = P::getpcpu(reinterpret_cast<const P::KINFO *>(&fx.ki));
					if (!dbl_eq(rd, pd))
						fail(FN_GETPCPU, "double mismatch");
				}
			}
		}
	}

	/* kvar / rvar on numeric keywords */
	for (size_t ki = 0; ki < known_keywords_nb; ki++) {
		if (keywords[ki].oproc != ref_kvar && keywords[ki].oproc != ref_rvar)
			continue;
		fixture_reset(fx);
		fixture_randomize(fx, true);
		fx.ve.var = &keywords[ki];
		if (keywords[ki].oproc == ref_kvar) {
			test_print_pair(FN_KVAR, ref_kvar, [](KINFO *k, VARENT *v) {
				return P::kvar(reinterpret_cast<P::KINFO *>(k),
				    reinterpret_cast<P::VARENT *>(v));
			}, fx);
		} else {
			test_print_pair(FN_RVAR, ref_rvar, [](KINFO *k, VARENT *v) {
				return P::rvar(reinterpret_cast<P::KINFO *>(k),
				    reinterpret_cast<P::VARENT *>(v));
			}, fx);
		}
	}

	for (int i = 0; i < 200000; i++) {
		cflag = (int)(randu32() & 1);
		showthreads = (int)(randu32() & 1);
		sumrusage = (int)(randu32() & 1);
		rawcpu = (int)(randu32() & 1);
		devname_mode = (int)(randu32() & 1);
		jail_getname_fail = (int)(randu32() & 3) == 0 ? 1 : 0;
		mac_prepare_fail = (int)(randu32() & 7) == 0 ? 1 : 0;
		mac_get_pid_fail = (int)(randu32() & 7) == 0 ? 1 : 0;
		mac_to_text_fail = (int)(randu32() & 7) == 0 ? 1 : 0;
		nlistread = 0;
		if (randu32() & 1)
			donlist();

		bool last = (randu32() & 1) != 0;
		fixture_reset(fx);
		fixture_randomize(fx, last);

		for (auto &te : table)
			test_print_pair(te.idx, te.ref_fn, te.port_fn, fx);

		casebump(FN_GETPCPU);
		double rd = ref_getpcpu(&fx.ki);
		double pd = P::getpcpu(reinterpret_cast<const P::KINFO *>(&fx.ki));
		if (!dbl_eq(rd, pd))
			fail(FN_GETPCPU, "random double mismatch");

		size_t kidx = (size_t)randint(0, (int)known_keywords_nb - 1);
		if (keywords[kidx].oproc == ref_kvar) {
			fx.ve.var = &keywords[kidx];
			test_print_pair(FN_KVAR, ref_kvar, [](KINFO *k, VARENT *v) {
				return P::kvar(reinterpret_cast<P::KINFO *>(k),
				    reinterpret_cast<P::VARENT *>(v));
			}, fx);
		} else if (keywords[kidx].oproc == ref_rvar) {
			fx.ve.var = &keywords[kidx];
			test_print_pair(FN_RVAR, ref_rvar, [](KINFO *k, VARENT *v) {
				return P::rvar(reinterpret_cast<P::KINFO *>(k),
				    reinterpret_cast<P::VARENT *>(v));
			}, fx);
		}
	}
}

/* ------------------------------------------------------------------ */
/* main                                                               */
/* ------------------------------------------------------------------ */

int
main(void)
{
	setlocale(LC_ALL, "C");
	ref_resolve_aliases();
	P::resolve_aliases();

	run_fmt_argv_tests();
	run_keyword_tests();
	run_print_tests();

	std::printf("\n%-24s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-24s %12s %12s\n", "--------", "-----", "--------");
	unsigned long long tc = 0, tf = 0;
	for (int i = 0; i < NFUNC; i++) {
		std::printf("%-24s %12llu %12llu\n",
		    kFnName[i], g_cases[i], g_fails[i]);
		tc += g_cases[i];
		tf += g_fails[i];
	}
	std::printf("%-24s %12llu %12llu\n", "TOTAL", tc, tf);
	return tf == 0 ? 0 : 1;
}
