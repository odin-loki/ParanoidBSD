/*
 * Differential harness for PBSD batch b0334 (ps.c).
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <clocale>
#include <vector>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <setjmp.h>

import pbsd.bin.ps.b0334;

namespace P = pbsd::bin_ps::b0334;

/* ------------------------------------------------------------------ */
/* Oracle / mock state                                                */
/* ------------------------------------------------------------------ */

extern "C" {

jmp_buf b0334_err_jmp;
int b0334_err_jmp_set = 0;
int b0334_errx_code = 0;
int b0334_exit_jmp_set = 0;

static jmp_buf g_exit_jmp;
static int g_exit_code = 0;

int b0334_optind = 0;
char *b0334_optarg = nullptr;

size_t known_keywords_nb = 4;

static uint32_t g_rng = 0xB0334001u;

static uint32_t randu32(void)
{
	g_rng = g_rng * 1103515245u + 12345u;
	return g_rng;
}

static int randint(int lo, int hi)
{
	if (hi <= lo)
		return lo;
	return lo + (int)(randu32() % (uint32_t)(hi - lo + 1));
}

static void rand_fill(char *buf, int len)
{
	for (int i = 0; i < len; i++)
		buf[i] = (char)(randu32() & 0xFF);
	buf[len] = '\0';
}

/* --- xo mocks --- */

static int xo_warn_count = 0;
static int xo_errx_count = 0;

void xo_warnx(const char *fmt, ...)
{
	(void)fmt;
	xo_warn_count++;
}

void xo_warn(const char *fmt, ...)
{
	(void)fmt;
	xo_warn_count++;
}

void xo_err(int ev, const char *fmt, ...)
{
	(void)ev; (void)fmt;
}

void xo_errx(int ev, const char *fmt, ...)
{
	(void)fmt;
	xo_errx_count++;
	if (b0334_err_jmp_set)
		longjmp(b0334_err_jmp, ev);
	exit(ev);
}

void xo_error(const char *fmt, ...)
{
	(void)fmt;
}

int xo_parse_args(int argc, char *argv[])
{
	return argc;
}

void xo_open_container(const char *) {}
void xo_close_container(const char *) {}
void xo_open_list(const char *) {}
void xo_close_list(const char *) {}
void xo_open_instance(const char *) {}
void xo_close_instance(const char *) {}
int xo_emit(const char *fmt, ...)
{
	(void)fmt;
	return 0;
}
int xo_finish(void) { return 0; }
int xo_get_style(void *) { return 0; }

/* --- passwd/group/jail/stat mocks --- */

struct mock_user {
	char name[64];
	uid_t uid;
	int valid;
};

struct mock_group {
	char name[64];
	gid_t gid;
	int valid;
};

static mock_user g_users[32];
static int g_user_count = 0;
static mock_group g_groups[32];
static int g_group_count = 0;
static int g_jail_map[256];
static int g_stat_result = 0;
static dev_t g_stat_rdev = 0x1234;
static int g_stat_ischr = 1;
static char g_ttyname_buf[64];
static int g_ttyname_set = 0;
static int g_pid_max = 99999;
static int g_sysctl_fail = 0;
static int g_winsz_col = 80;
static char g_columns_env[32];
static int g_columns_set = 0;
static uid_t g_euid = 1000;

int b0334_getgrgid(gid_t gid, struct group *g)
{
	for (int i = 0; i < g_group_count; i++) {
		if (g_groups[i].valid && g_groups[i].gid == gid) {
			g->gr_gid = g_groups[i].gid;
			g->gr_name = g_groups[i].name;
			return 1;
		}
	}
	return 0;
}

int b0334_getgrnam(const char *name, struct group *g)
{
	for (int i = 0; i < g_group_count; i++) {
		if (g_groups[i].valid && strcmp(g_groups[i].name, name) == 0) {
			g->gr_gid = g_groups[i].gid;
			g->gr_name = g_groups[i].name;
			return 1;
		}
	}
	return 0;
}

int b0334_getpwnam(const char *name, struct passwd *p)
{
	for (int i = 0; i < g_user_count; i++) {
		if (g_users[i].valid && strcmp(g_users[i].name, name) == 0) {
			p->pw_uid = g_users[i].uid;
			p->pw_name = g_users[i].name;
			return 1;
		}
	}
	return 0;
}

int b0334_getpwuid(uid_t uid, struct passwd *p)
{
	for (int i = 0; i < g_user_count; i++) {
		if (g_users[i].valid && g_users[i].uid == uid) {
			p->pw_uid = g_users[i].uid;
			p->pw_name = g_users[i].name;
			return 1;
		}
	}
	return 0;
}

int b0334_jail_getid(const char *elem)
{
	if (*elem == '\0')
		return -1;
	if (elem[0] >= '0' && elem[0] <= '9') {
		int id = atoi(elem);
		if (id >= 0 && id < 256 && g_jail_map[id])
			return id;
	}
	return -1;
}

int b0334_stat(const char *path, struct stat *sb)
{
	(void)path;
	if (g_stat_result != 0)
		return g_stat_result;
	memset(sb, 0, sizeof(*sb));
	sb->st_mode = g_stat_ischr ? S_IFCHR : S_IFREG;
	sb->st_rdev = g_stat_rdev;
	return 0;
}

char *b0334_ttyname(int)
{
	return g_ttyname_set ? g_ttyname_buf : nullptr;
}

int b0334_sysctl_pid_max(int *val)
{
	if (g_sysctl_fail)
		return -1;
	*val = g_pid_max;
	return 0;
}

int b0334_ioctl_winsz(int, struct winsize *ws)
{
	ws->ws_col = (unsigned short)g_winsz_col;
	ws->ws_row = 24;
	return 0;
}

char *b0334_getenv(const char *name)
{
	if (g_columns_set && strcmp(name, "COLUMNS") == 0)
		return g_columns_env;
	return nullptr;
}

uid_t b0334_geteuid(void) { return g_euid; }

/* --- getopt mock --- */

static const char *g_optstring;
static int g_getopt_argc;
static char **g_getopt_argv;
static int g_getopt_idx = 1;
static int g_getopt_sp = 1;
static int g_getopt_force = -2;
static int g_getopt_force_count = 0;

int b0334_getopt(int argc, char *const argv[], const char *optstring)
{
	if (g_getopt_force_count > 0) {
		g_getopt_force_count--;
		return g_getopt_force;
	}
	g_optstring = optstring;
	g_getopt_argc = argc;
	g_getopt_argv = (char **)argv;
	if (b0334_optind >= argc || argv[b0334_optind] == nullptr ||
	    argv[b0334_optind][0] != '-' || argv[b0334_optind][1] == '\0')
		return -1;
	const char *p = argv[b0334_optind] + g_getopt_sp;
	char c = *p;
	if (c == '\0') {
		b0334_optind++;
		g_getopt_sp = 1;
		return b0334_getopt(argc, argv, optstring);
	}
	g_getopt_sp++;
	const char *o = strchr(optstring, c);
	if (o == nullptr)
		return '?';
	if (o[1] == ':') {
		if (*p != '\0')
			b0334_optarg = (char *)(p + 1);
		else {
			b0334_optind++;
			if (b0334_optind < argc)
				b0334_optarg = argv[b0334_optind];
			b0334_optind++;
		}
		g_getopt_sp = 1;
	}
	return (unsigned char)c;
}

/* --- kvm / keyword mocks --- */

static P::kinfo_proc g_kprocs[64];
static int g_kproc_count = 0;
static kvm_t g_kd_storage;
static int g_kvm_open_fail = 0;
static int g_kvm_nentries = 0;

kvm_t *kvm_openfiles(const char *, const char *, const char *, int, char *)
{
	if (g_kvm_open_fail)
		return nullptr;
	return &g_kd_storage;
}

P::kinfo_proc *kvm_getprocs(kvm_t *, int, int, int *n)
{
	*n = g_kvm_nentries;
	return g_kproc_count > 0 ? &g_kprocs[0] : nullptr;
}

char *kvm_geterr(kvm_t *) { return (char *)"kvm err"; }

static char *g_argv_storage[8];
static char g_argv_data[256];

char **kvm_getargv(kvm_t *, const P::kinfo_proc *, int)
{
	g_argv_storage[0] = g_argv_data;
	strcpy(g_argv_data, "test");
	g_argv_storage[1] = nullptr;
	return g_argv_storage;
}

char **kvm_getenvv(kvm_t *, const P::kinfo_proc *, int)
{
	g_argv_storage[0] = nullptr;
	return g_argv_storage;
}

static P::VAR g_kw[4];
static P::VARENT g_vent[8];
static int g_vent_count = 0;

static char *mock_oproc(P::KINFO *, P::VARENT *)
{
	static char buf[32];
	snprintf(buf, sizeof(buf), "val");
	return buf;
}

void parsefmt(const char *fmt, P::velisthead *vl, int keep)
{
	(void)keep;
	STAILQ_INIT(vl);
	if (fmt == nullptr || *fmt == '\0')
		return;
	char copy[256];
	strncpy(copy, fmt, sizeof(copy) - 1);
	copy[sizeof(copy) - 1] = '\0';
	char *tok = strtok(copy, ",");
	int idx = 0;
	while (tok != nullptr && idx < 8) {
		while (*tok == ' ') tok++;
		g_vent[idx].var = &g_kw[idx % 4];
		g_kw[idx % 4].name = tok;
		g_kw[idx % 4].flag = 0;
		g_kw[idx % 4].oproc = mock_oproc;
		g_kw[idx % 4].final_kw = &g_kw[idx % 4];
		g_vent[idx].var = &g_kw[idx % 4];
		g_vent[idx].width = 0;
		g_vent[idx].flags = 0;
		STAILQ_INSERT_TAIL(vl, &g_vent[idx], next_ve);
		idx++;
		tok = strtok(nullptr, ",");
	}
	g_vent_count = idx;
}

void printheader(void) {}
double getpcpu(const P::KINFO *) { return 1.5; }
void showkey(void) {}
void check_keywords(void) {}
void resolve_aliases(void) {}
const char *fmt_argv(char **av, char *comm, char *td, size_t)
{
	(void)td;
	if (av == nullptr)
		return comm;
	static char buf[128];
	buf[0] = '\0';
	for (int i = 0; av[i]; i++) {
		if (i) strcat(buf, " ");
		strcat(buf, av[i]);
	}
	return buf;
}

size_t aliased_keyword_index(const P::VAR *v)
{
	for (size_t i = 0; i < known_keywords_nb; i++)
		if (&g_kw[i] == v)
			return i;
	return 0;
}

/* --- malloc wrap --- */
static int g_malloc_fail_at = -1;
static int g_malloc_calls = 0;

} // extern "C"

extern "C" void *__wrap_malloc(size_t n)
{
	g_malloc_calls++;
	if (g_malloc_fail_at >= 0 && g_malloc_calls == g_malloc_fail_at)
		return nullptr;
	return malloc(n);
}

extern "C" void __wrap_exit(int code)
{
	if (b0334_exit_jmp_set)
		longjmp(g_exit_jmp, code);
	::_exit(code);
}

/* STAILQ for harness C++ side */
#define STAILQ_HEAD(name, type) \
	struct name { struct type *stqh_first; struct type **stqh_last; }
#define STAILQ_ENTRY(type) struct { struct type *stqe_next; }
#define STAILQ_FIRST(h) ((h)->stqh_first)
#define STAILQ_NEXT(e, f) ((e)->f.stqe_next)
#define STAILQ_INIT(h) do { \
	STAILQ_FIRST(h) = nullptr; (h)->stqh_last = &STAILQ_FIRST(h); } while (0)
#define STAILQ_INSERT_TAIL(h, e, f) do { \
	STAILQ_NEXT(e, f) = nullptr; *(h)->stqh_last = (e); \
	(h)->stqh_last = &STAILQ_NEXT(e, f); } while (0)

STAILQ_HEAD(velisthead, varent);

extern "C" {

/* Oracle function declarations */
char *ref_kludge_oldps_options(const char *, char *, const char *);
void ref_pidmax_init(void);
void ref_init_list(P::listinfo *, int (*)(P::listinfo *, const char *),
    int, const char *);
void *ref_expand_list(P::listinfo *);
void ref_free_list(P::listinfo *);
void ref_add_list(P::listinfo *, const char *);
int ref_addelem_pid(P::listinfo *, const char *);
int ref_addelem_gid(P::listinfo *, const char *);
int ref_addelem_jid(P::listinfo *, const char *);
int ref_addelem_uid(P::listinfo *, const char *);
int ref_addelem_tty(P::listinfo *, const char *);
int ref_pscomp(const void *, const void *);
void ref_descendant_sort(P::KINFO *, int);
P::VARENT *ref_find_varentry(const char *);
void ref_scan_vars(P::keyword_info *);
void ref_remove_redundant_columns(P::keyword_info *);
void ref_format_output(P::KINFO *);
void ref_saveuser(P::KINFO *);
int ref_main(int, char **);
void ref_usage(void);

extern int cflag, eval, rawcpu, sumrusage, termwidth, showthreads;
extern time_t now;
extern P::velisthead varlist;

} // extern "C"

/* ------------------------------------------------------------------ */
/* Test framework                                                     */
/* ------------------------------------------------------------------ */

enum FnId {
	FN_KLUDGE = 0,
	FN_PIDMAX,
	FN_INIT_LIST,
	FN_EXPAND_LIST,
	FN_FREE_LIST,
	FN_ADD_LIST,
	FN_ADDELEM_PID,
	FN_ADDELEM_GID,
	FN_ADDELEM_JID,
	FN_ADDELEM_UID,
	FN_ADDELEM_TTY,
	FN_PSCOMP,
	FN_DESCENDANT,
	FN_FIND_VARENT,
	FN_SCAN_VARS,
	FN_REMOVE_REDUNDANT,
	FN_FORMAT_OUTPUT,
	FN_SAVEUSER,
	FN_MAIN,
	FN_USAGE,
	FN_COUNT
};

static const char *kFnName[] = {
	"kludge_oldps_options",
	"pidmax_init",
	"init_list",
	"expand_list",
	"free_list",
	"add_list",
	"addelem_pid",
	"addelem_gid",
	"addelem_jid",
	"addelem_uid",
	"addelem_tty",
	"pscomp",
	"descendant_sort",
	"find_varentry",
	"scan_vars",
	"remove_redundant_columns",
	"format_output",
	"saveuser",
	"main",
	"usage",
};

static unsigned long long g_cases[FN_COUNT];
static unsigned long long g_fails[FN_COUNT];

static void fail(FnId id, const char *msg)
{
	g_fails[id]++;
	if (g_fails[id] <= 3)
		fprintf(stderr, "FAIL %s: %s\n", kFnName[id], msg);
}

static void casebump(FnId id) { g_cases[id]++; }

static void reset_mocks(void)
{
	g_user_count = g_group_count = 0;
	memset(g_jail_map, 0, sizeof(g_jail_map));
	g_stat_result = 0;
	g_stat_ischr = 1;
	g_stat_rdev = 0x1234;
	g_ttyname_set = 0;
	g_pid_max = 99999;
	g_sysctl_fail = 0;
	g_winsz_col = 80;
	g_columns_set = 0;
	g_euid = 1000;
	g_kproc_count = 0;
	g_kvm_nentries = 0;
	g_kvm_open_fail = 0;
	g_getopt_force = -2;
	g_getopt_force_count = 0;
	b0334_optind = 1;
	b0334_optarg = nullptr;
	g_malloc_fail_at = -1;
	g_malloc_calls = 0;
	xo_warn_count = xo_errx_count = 0;
	STAILQ_INIT(&varlist);
	cflag = eval = rawcpu = sumrusage = 0;
	termwidth = 79;
	showthreads = 0;
	now = 1700000000;
}

static int addelem_count_cb;
static int addelem_record_cb(P::listinfo *, const char *elem)
{
	addelem_count_cb++;
	(void)elem;
	if (addelem_record_cb >= addelem_count_cb)
		return 1;
	return 0;
}

/* ------------------------------------------------------------------ */
/* kludge_oldps_options                                               */
/* ------------------------------------------------------------------ */

static void test_kludge_one(const char *optlist, char *orig, const char *next,
    char *orig_copy)
{
	casebump(FN_KLUDGE);
	char *r = ref_kludge_oldps_options(optlist, orig, next);
	char *p = P::kludge_oldps_options(optlist, orig_copy, next);
	if ((r == orig) != (p == orig_copy)) {
		fail(FN_KLUDGE, "same-buffer mismatch");
		return;
	}
	if (r != orig && (r == nullptr) != (p == nullptr)) {
		fail(FN_KLUDGE, "null mismatch");
		return;
	}
	if (r != orig && strcmp(r, p) != 0)
		fail(FN_KLUDGE, "string mismatch");
}

static void run_kludge_tests(void)
{
	const char *opts = "AaCcD:defG:gHhjJ:LlM:mN:O:o:p:rSTt:U:uvwXxZ";
	char buf1[256], buf2[256];
	const char *cases[] = {
		"-aux", "axu", "34", "1,2,3", "1234", "axt", "axt5", "axT",
		"-p1", "u", "t", "co", "-tpts/0", "12,34,56", "foo", "xu",
		"axu123", "123xu", "-Dup", "ww", "t-", "-", "",
	};
	for (const char *c : cases) {
		strncpy(buf1, c, sizeof(buf1) - 1); buf1[sizeof(buf1)-1] = '\0';
		strncpy(buf2, c, sizeof(buf2) - 1); buf2[sizeof(buf2)-1] = '\0';
		test_kludge_one(opts, buf1, nullptr, buf2);
		test_kludge_one(opts, buf1, "-x", buf2);
		test_kludge_one(opts, buf1, "99", buf2);
		test_kludge_one(opts, buf1, "abc", buf2);
	}
	for (int i = 0; i < 10000; i++) {
		int len = randint(0, 40);
		rand_fill(buf1, len);
		rand_fill(buf2, len);
		const char *next = nullptr;
		char nextbuf[16];
		int which = randint(0, 3);
		if (which == 1) next = "-";
		else if (which == 2) { snprintf(nextbuf, sizeof(nextbuf), "%d", randint(0, 99999)); next = nextbuf; }
		else if (which == 3) { rand_fill(nextbuf, randint(0, 8)); next = nextbuf; }
		test_kludge_one(opts, buf1, next, buf2);
	}
}

/* ------------------------------------------------------------------ */
/* list helpers                                                       */
/* ------------------------------------------------------------------ */

static void compare_listinfo(const P::listinfo *ra, const P::listinfo *pa,
    FnId id)
{
	if (ra->count != pa->count || ra->maxcount != pa->maxcount ||
	    ra->elemsize != pa->elemsize)
		fail(id, "listinfo header mismatch");
}

static void run_list_tests(void)
{
	P::listinfo rli{}, pli{};
	ref_init_list(&rli, ref_addelem_pid, (int)sizeof(pid_t), "process id");
	P::init_list(&pli, P::addelem_pid, (int)sizeof(pid_t), "process id");
	casebump(FN_INIT_LIST);
	compare_listinfo(&rli, &pli, FN_INIT_LIST);

	for (int i = 0; i < 10000; i++) {
		reset_mocks();
		ref_init_list(&rli, ref_addelem_pid, (int)sizeof(pid_t), "pid");
		P::init_list(&pli, P::addelem_pid, (int)sizeof(pid_t), "pid");
		casebump(FN_INIT_LIST);
		int expansions = randint(0, 6);
		for (int e = 0; e < expansions; e++) {
			casebump(FN_EXPAND_LIST);
			void *rr = ref_expand_list(&rli);
			void *pr = P::expand_list(&pli);
			if (rli.maxcount != pli.maxcount)
				fail(FN_EXPAND_LIST, "maxcount");
			if (rli.l.ptr == nullptr || pli.l.ptr == nullptr)
				fail(FN_EXPAND_LIST, "null ptr");
			(void)rr; (void)pr;
		}
		casebump(FN_FREE_LIST);
		ref_free_list(&rli);
		P::free_list(&pli);
	}
}

/* ------------------------------------------------------------------ */
/* addelem_* / add_list                                               */
/* ------------------------------------------------------------------ */

static void setup_users_groups(void)
{
	snprintf(g_users[0].name, sizeof(g_users[0].name), "root");
	g_users[0].uid = 0; g_users[0].valid = 1;
	snprintf(g_users[1].name, sizeof(g_users[1].name), "test");
	g_users[1].uid = 1000; g_users[1].valid = 1;
	g_user_count = 2;
	snprintf(g_groups[0].name, sizeof(g_groups[0].name), "wheel");
	g_groups[0].gid = 0; g_groups[0].valid = 1;
	g_groups[1].name[0] = '1'; g_groups[1].name[1] = '\0';
	g_groups[1].gid = 1; g_groups[1].valid = 1;
	g_group_count = 2;
	g_jail_map[5] = 1;
}

static void run_addelem_tests(void)
{
	const char *pid_cases[] = {
		"", "0", "1", "99999", "100000", "abc", "-1", "12abc",
		"2147483647", "999999999999",
	};
	P::listinfo rli{}, pli{};
	for (const char *c : pid_cases) {
		reset_mocks();
		setup_users_groups();
		ref_init_list(&rli, ref_addelem_pid, (int)sizeof(pid_t), "process id");
		P::init_list(&pli, P::addelem_pid, (int)sizeof(pid_t), "process id");
		casebump(FN_ADDELEM_PID);
		int rr = ref_addelem_pid(&rli, c);
		int pr = P::addelem_pid(&pli, c);
		if (rr != pr || rli.count != pli.count)
			fail(FN_ADDELEM_PID, c);
	}

	const char *gid_cases[] = { "", "0", "wheel", "1", "nosuch", "999999" };
	for (const char *c : gid_cases) {
		reset_mocks(); setup_users_groups();
		ref_init_list(&rli, ref_addelem_gid, (int)sizeof(gid_t), "group");
		P::init_list(&pli, P::addelem_gid, (int)sizeof(gid_t), "group");
		casebump(FN_ADDELEM_GID);
		int rr = ref_addelem_gid(&rli, c);
		int pr = P::addelem_gid(&pli, c);
		if (rr != pr || rli.count != pli.count)
			fail(FN_ADDELEM_GID, c);
	}

	const char *jid_cases[] = { "", "5", "9", "x" };
	for (const char *c : jid_cases) {
		reset_mocks(); setup_users_groups();
		ref_init_list(&rli, ref_addelem_jid, (int)sizeof(int), "jail id");
		P::init_list(&pli, P::addelem_jid, (int)sizeof(int), "jail id");
		casebump(FN_ADDELEM_JID);
		int rr = ref_addelem_jid(&rli, c);
		int pr = P::addelem_jid(&pli, c);
		if (rr != pr || rli.count != pli.count)
			fail(FN_ADDELEM_JID, c);
	}

	const char *uid_cases[] = { "", "root", "0", "1000", "nosuch", "99999" };
	for (const char *c : uid_cases) {
		reset_mocks(); setup_users_groups();
		ref_init_list(&rli, ref_addelem_uid, (int)sizeof(uid_t), "user");
		P::init_list(&pli, P::addelem_uid, (int)sizeof(uid_t), "user");
		casebump(FN_ADDELEM_UID);
		int rr = ref_addelem_uid(&rli, c);
		int pr = P::addelem_uid(&pli, c);
		if (rr != pr || rli.count != pli.count)
			fail(FN_ADDELEM_UID, c);
	}

	const char *tty_cases[] = {
		"/dev/tty", "co", "console", "pts/0", "p0", "tty00",
	};
	for (const char *c : tty_cases) {
		reset_mocks(); setup_users_groups();
		g_stat_result = 0; g_stat_ischr = 1;
		ref_init_list(&rli, ref_addelem_tty, (int)sizeof(dev_t), "tty");
		P::init_list(&pli, P::addelem_tty, (int)sizeof(dev_t), "tty");
		casebump(FN_ADDELEM_TTY);
		int rr = ref_addelem_tty(&rli, c);
		int pr = P::addelem_tty(&pli, c);
		if (rr != pr || rli.count != pli.count)
			fail(FN_ADDELEM_TTY, c);
	}

	const char *list_cases[] = {
		"", "1", "1,2", " 1 , 2 ", "1,", ",", "1 2", "a", "1,2,3,4,5",
	};
	for (const char *c : list_cases) {
		reset_mocks(); setup_users_groups();
		ref_init_list(&rli, ref_addelem_pid, (int)sizeof(pid_t), "pid");
		P::init_list(&pli, P::addelem_pid, (int)sizeof(pid_t), "pid");
		casebump(FN_ADD_LIST);
		ref_add_list(&rli, c);
		P::add_list(&pli, c);
		if (rli.count != pli.count)
			fail(FN_ADD_LIST, c);
	}

	for (int i = 0; i < 10000; i++) {
		char elem[48];
		rand_fill(elem, randint(0, 20));
		reset_mocks(); setup_users_groups();
		ref_init_list(&rli, ref_addelem_pid, (int)sizeof(pid_t), "pid");
		P::init_list(&pli, P::addelem_pid, (int)sizeof(pid_t), "pid");
		casebump(FN_ADDELEM_PID);
		int rr = ref_addelem_pid(&rli, elem);
		int pr = P::addelem_pid(&pli, elem);
		if (rr != pr || rli.count != pli.count)
			fail(FN_ADDELEM_PID, "random");
	}
}

/* ------------------------------------------------------------------ */
/* pscomp                                                             */
/* ------------------------------------------------------------------ */

static P::KINFO make_ki(pid_t pid, pid_t tid, dev_t tdev, double pcpu,
    segsz_t mem, long flag)
{
	P::KINFO ki{};
	static P::kinfo_proc kp{};
	kp.ki_pid = pid;
	kp.ki_tid = tid;
	kp.ki_tdev = tdev;
	kp.ki_flag = flag;
	ki.ki_p = &kp;
	ki.ki_pcpu = pcpu;
	ki.ki_memsize = mem;
	return ki;
}

static void run_pscomp_tests(void)
{
	extern int sortby_e;
	(void)sortby_e;
	P::KINFO a = make_ki(1, 1, 5, 10.0, 100, P_CONTROLT);
	P::KINFO b = make_ki(2, 2, NODEV, 20.0, 200, 0);
	P::KINFO acopy = a, bcopy = b;
	casebump(FN_PSCOMP);
	int r1 = ref_pscomp(&a, &b);
	int p1 = P::pscomp(&acopy, &bcopy);
	if (r1 != p1) fail(FN_PSCOMP, "basic");

	for (int i = 0; i < 10000; i++) {
		P::KINFO ka = make_ki(randint(0, 30000), randint(0, 30000),
		    (dev_t)randint(0, 100), (double)(randint(0, 1000)),
		    (segsz_t)randint(0, 100000), randint(0, 1) ? P_CONTROLT : 0);
		P::KINFO kb = make_ki(randint(0, 30000), randint(0, 30000),
		    (dev_t)(randint(0, 2) == 0 ? (int)NODEV : randint(0, 100)),
		    (double)(randint(0, 1000)), (segsz_t)randint(0, 100000),
		    randint(0, 1) ? P_CONTROLT : 0);
		P::KINFO kac = ka, kbc = kb;
		casebump(FN_PSCOMP);
		if (ref_pscomp(&ka, &kb) != P::pscomp(&kac, &kbc))
			fail(FN_PSCOMP, "random");
	}
}

/* ------------------------------------------------------------------ */
/* descendant_sort                                                    */
/* ------------------------------------------------------------------ */

static void run_descendant_tests(void)
{
	static P::kinfo_proc procs[8];
	P::KINFO rki[8], pki[8];
	for (int n = 2; n <= 6; n++) {
		for (int i = 0; i < n; i++) {
			procs[i].ki_pid = 100 + i;
			procs[i].ki_ppid = i == 0 ? 1 : 100 + i - 1;
			rki[i].ki_p = &procs[i];
			rki[i].ki_d.level = 0;
			rki[i].ki_d.prefix = nullptr;
			pki[i] = rki[i];
		}
		casebump(FN_DESCENDANT);
		ref_descendant_sort(rki, n);
		P::descendant_sort(pki, n);
		for (int i = 0; i < n; i++) {
			if (rki[i].ki_p->ki_pid != pki[i].ki_p->ki_pid)
				fail(FN_DESCENDANT, "order");
		}
		for (int i = 0; i < n; i++)
			free(rki[i].ki_d.prefix);
		for (int i = 0; i < n; i++)
			free(pki[i].ki_d.prefix);
	}
	for (int t = 0; t < 10000; t++) {
		int n = randint(1, 8);
		for (int i = 0; i < n; i++) {
			procs[i].ki_pid = randint(1, 500);
			procs[i].ki_ppid = i > 0 ? procs[randint(0, i - 1)].ki_pid : 0;
			rki[i].ki_p = &procs[i];
			rki[i].ki_d.level = 0;
			rki[i].ki_d.prefix = nullptr;
			pki[i] = rki[i];
		}
		casebump(FN_DESCENDANT);
		ref_descendant_sort(rki, n);
		P::descendant_sort(pki, n);
		for (int i = 0; i < n; i++) {
			if (rki[i].ki_p->ki_pid != pki[i].ki_p->ki_pid)
				fail(FN_DESCENDANT, "random order");
			if ((rki[i].ki_d.prefix == nullptr) != (pki[i].ki_d.prefix == nullptr))
				fail(FN_DESCENDANT, "prefix null");
			else if (rki[i].ki_d.prefix && strcmp(rki[i].ki_d.prefix, pki[i].ki_d.prefix))
				fail(FN_DESCENDANT, "prefix text");
		}
		for (int i = 0; i < n; i++) { free(rki[i].ki_d.prefix); free(pki[i].ki_d.prefix); }
	}
}

/* ------------------------------------------------------------------ */
/* find_varentry / scan_vars / remove_redundant_columns               */
/* ------------------------------------------------------------------ */

static void run_var_tests(void)
{
	reset_mocks();
	parsefmt("pid,user,command", &varlist, 0);
	casebump(FN_FIND_VARENT);
	P::VARENT *rv = ref_find_varentry("pid");
	P::VARENT *pv = P::find_varentry("pid");
	if ((rv == nullptr) != (pv == nullptr))
		fail(FN_FIND_VARENT, "pid null");
	if (rv && pv && strcmp(rv->var->name, pv->var->name))
		fail(FN_FIND_VARENT, "pid name");
	rv = ref_find_varentry("nosuch");
	pv = P::find_varentry("nosuch");
	if ((rv != nullptr) || (pv != nullptr))
		fail(FN_FIND_VARENT, "nosuch");

	P::keyword_info rkw[4]{}, pkw[4]{};
	casebump(FN_SCAN_VARS);
	ref_scan_vars(rkw);
	P::scan_vars(pkw);
	for (size_t i = 0; i < known_keywords_nb; i++) {
		if (rkw[i].flags != pkw[i].flags)
			fail(FN_SCAN_VARS, "flags");
	}

	casebump(FN_REMOVE_REDUNDANT);
	ref_remove_redundant_columns(rkw);
	P::remove_redundant_columns(pkw);

	for (int i = 0; i < 10000; i++) {
		reset_mocks();
		char fmt[64];
		rand_fill(fmt, randint(0, 20));
		parsefmt("pid,tt,state", &varlist, 0);
		casebump(FN_FIND_VARENT);
		const char *names[] = {"pid", "tt", "state", "nope"};
		const char *q = names[randint(0, 3)];
		rv = ref_find_varentry(q);
		pv = P::find_varentry(q);
		if ((rv == nullptr) != (pv == nullptr))
			fail(FN_FIND_VARENT, "random");
	}
}

/* ------------------------------------------------------------------ */
/* format_output / saveuser / pidmax_init / usage / main              */
/* ------------------------------------------------------------------ */

static void run_format_tests(void)
{
	reset_mocks();
	parsefmt("pid,command", &varlist, 0);
	static P::kinfo_proc kp{};
	kp.ki_pid = 42;
	kp.ki_stat = 'R';
	strcpy(kp.ki_comm, "cmd");
	P::KINFO rki{}, pki{};
	rki.ki_p = pki.ki_p = &kp;
	STAILQ_INIT(&rki.ki_ks);
	STAILQ_INIT(&pki.ki_ks);
	casebump(FN_FORMAT_OUTPUT);
	ref_format_output(&rki);
	P::format_output(&pki);

	casebump(FN_SAVEUSER);
	ref_saveuser(&rki);
	P::saveuser(&pki);
}

static void run_pidmax_tests(void)
{
	for (int i = 0; i < 10000; i++) {
		reset_mocks();
		g_sysctl_fail = randint(0, 5) == 0 ? 1 : 0;
		g_pid_max = randint(1000, 200000);
		casebump(FN_PIDMAX);
		ref_pidmax_init();
		P::pidmax_init();
	}
}

static void run_usage_tests(void)
{
	b0334_exit_jmp_set = 1;
	if (setjmp(g_exit_jmp) == 0) {
		casebump(FN_USAGE);
		ref_usage();
		fail(FN_USAGE, "no exit");
	}
	b0334_exit_jmp_set = 1;
	if (setjmp(g_exit_jmp) == 0) {
		casebump(FN_USAGE);
		P::usage();
		fail(FN_USAGE, "port no exit");
	}
	b0334_exit_jmp_set = 0;
}

static void run_main_tests(void)
{
	char arg0[] = "ps";
	char a1[] = "-L";
	char *argv1[] = { arg0, a1, nullptr };
	reset_mocks();
	b0334_exit_jmp_set = 1;
	if (setjmp(g_exit_jmp) == 0) {
		casebump(FN_MAIN);
		ref_main(2, argv1);
	}
	b0334_exit_jmp_set = 1;
	if (setjmp(g_exit_jmp) == 0) {
		casebump(FN_MAIN);
		P::main(2, argv1);
	}
	b0334_exit_jmp_set = 0;

	char a_u[] = "-u";
	char *argv2[] = { arg0, a_u, nullptr };
	reset_mocks();
	g_kvm_nentries = 1;
	g_kproc_count = 1;
	g_kprocs[0].ki_pid = 1;
	g_kprocs[0].ki_uid = 1000;
	g_kprocs[0].ki_tdev = 5;
	g_kprocs[0].ki_flag = P_CONTROLT;
	g_euid = 1000;
	b0334_optind = 1;
	for (int i = 0; i < 10000; i++) {
		reset_mocks();
		g_kvm_nentries = randint(0, 4);
		g_kproc_count = g_kvm_nentries;
		for (int k = 0; k < g_kproc_count; k++) {
			g_kprocs[k].ki_pid = randint(1, 50000);
			g_kprocs[k].ki_ppid = randint(0, 50000);
			g_kprocs[k].ki_uid = g_euid;
			g_kprocs[k].ki_tdev = randint(0, 2) ? randint(1, 100) : NODEV;
			g_kprocs[k].ki_flag = randint(0, 1) ? P_CONTROLT : 0;
		}
		char opt[8];
		snprintf(opt, sizeof(opt), "-%c", "auxwX"[randint(0, 4)]);
		char *av[] = { arg0, opt, nullptr };
		b0334_optind = 1;
		g_getopt_force = -1;
		g_getopt_force_count = 0;
		b0334_exit_jmp_set = 1;
		int rc = setjmp(g_exit_jmp);
		casebump(FN_MAIN);
		if (rc == 0)
			ref_main(2, av);
		b0334_exit_jmp_set = 1;
		rc = setjmp(g_exit_jmp);
		if (rc == 0)
			P::main(2, av);
		b0334_exit_jmp_set = 0;
	}
}

/* ------------------------------------------------------------------ */

int main(void)
{
	setlocale(LC_ALL, "C");
	memset(g_cases, 0, sizeof(g_cases));
	memset(g_fails, 0, sizeof(g_fails));

	run_kludge_tests();
	run_pidmax_tests();
	run_list_tests();
	run_addelem_tests();
	run_pscomp_tests();
	run_descendant_tests();
	run_var_tests();
	run_format_tests();
	run_usage_tests();
	run_main_tests();

	printf("\n%-28s %12s %12s\n", "function", "cases", "failures");
	printf("%-28s %12s %12s\n", "--------", "-----", "--------");
	unsigned long long tc = 0, tf = 0;
	for (int i = 0; i < FN_COUNT; i++) {
		printf("%-28s %12llu %12llu\n", kFnName[i], g_cases[i], g_fails[i]);
		tc += g_cases[i];
		tf += g_fails[i];
	}
	printf("%-28s %12llu %12llu\n", "TOTAL", tc, tf);
	return tf == 0 ? 0 : 1;
}
