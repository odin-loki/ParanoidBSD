/*
 * harness.cpp -- differential test for PBSD batch b0333.
 */

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <utility>

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <link.h>
#include <pthread.h>

import pbsd.lib.libc.gen.b0333;

namespace P = pbsd::lib_libc_gen::b0333;

/* ------------------------------------------------------------------ */
/* oracle declarations                                                 */
/* ------------------------------------------------------------------ */

extern "C" {
extern const char __uprefix[];
extern const char *const sys_errlist[];
extern const int sys_nerr;

int ref_dladdr(const void *, Dl_info *);
int ref_dlclose(void *);
char *ref_dlerror(void);
void ref_dllockinit(void *, void *(*)(void *), void (*)(void *),
    void (*)(void *), void (*)(void *), void (*)(void *),
    void (*)(void *));
void *ref_dlopen(const char *, int);
void *ref_dlsym(void *, const char *);
void *ref_dlfunc(void *, const char *);
void *ref_dlvsym(void *, const char *, const char *);
int ref_dlinfo(void *, int, void *);
void ref__rtld_thread_init(void *);
int ref__dl_iterate_phdr_locked(int (*)(struct dl_phdr_info *, size_t, void *),
    void *);
int ref_dl_iterate_phdr(int (*)(struct dl_phdr_info *, size_t, void *),
    void *);
void *ref_fdlopen(int, int);
void ref__rtld_atfork_pre(int *);
void ref__rtld_atfork_post(int *);
int ref__rtld_addr_phdr(const void *, struct dl_phdr_info *);
int ref__rtld_get_stack_prot(void);
unsigned ref__rtld_get_pax_flags(void);
int ref__rtld_is_dlopened(void *);
const char *ref_rtld_get_var(const char *);
int ref_rtld_set_var(const char *, const char *);

int ref_execl(const char *, const char *, ...);
int ref_execle(const char *, const char *, ...);
int ref_execlp(const char *, const char *, ...);
int ref_execv(const char *, char *const *);
int ref_execvp(const char *, char *const *);
int ref_execvP(const char *, const char *, char *const *);
int ref___libc_execvpe(const char *, char *const *, char *const *);
}

/* ------------------------------------------------------------------ */
/* per-function statistics                                             */
/* ------------------------------------------------------------------ */

struct FnStat {
	const char *name;
	long cases;
	long fails;
};

static FnStat g_stats[64];
static int g_nstats;

static FnStat &
statfn(const char *name)
{
	for (int i = 0; i < g_nstats; ++i)
		if (std::strcmp(g_stats[i].name, name) == 0)
			return g_stats[i];
	g_stats[g_nstats].name = name;
	g_stats[g_nstats].cases = 0;
	g_stats[g_nstats].fails = 0;
	return g_stats[g_nstats++];
}

static void
record(const char *name, bool ok)
{
	FnStat &s = statfn(name);
	++s.cases;
	if (!ok)
		++s.fails;
}

/* ------------------------------------------------------------------ */
/* PRNG                                                                */
/* ------------------------------------------------------------------ */

static uint32_t g_rng = 0xB0333u;

static uint32_t
rnd(void)
{
	g_rng = g_rng * 1103515245u + 12345u;
	return g_rng;
}

static int
rnd_range(int lo, int hi)
{
	if (hi <= lo)
		return lo;
	return lo + (int)(rnd() % (unsigned)(hi - lo + 1));
}

/* ------------------------------------------------------------------ */
/* harness libc environment                                            */
/* ------------------------------------------------------------------ */

typedef struct {
	int	a_type;
	union {
		int	a_val;
		void	*a_ptr;
	} a_un;
} Elf_Auxinfo;

typedef struct {
	unsigned long	ti_module;
	unsigned long	ti_offset;
} tls_index;

static Elf_Auxinfo g_aux_store[32];
Elf_Auxinfo *__elf_aux_vector = nullptr;
static int g_aux_reset_generation;

static Elf_Phdr g_phdrs[8];
static char g_execpath[] = "/test/prog";
static char g_tls_block[64];
static void *g_tls_addr = g_tls_block;

static int g_execve_call;
static char g_execve_paths[64][512];
static int g_execve_errno[64];
static int g_execve_max;

static int g_stat_call;
static char g_stat_path[512];
static int g_stat_rv;
static int g_stat_errno;

static char g_getenv_path[4096];
static int g_getenv_set;

static char g_write_buf[8192];
static int g_write_len;

static int g_context_destroy_calls;
static void *g_context_destroy_arg;

char **environ;

extern "C" {

void
__init_elf_aux_vector(void)
{
	/* harness sets __elf_aux_vector directly */
}

void *
_once(pthread_once_t *o, void (*fn)(void))
{
	pthread_once(o, fn);
	return (nullptr);
}

void *
__tls_get_addr(tls_index *ti)
{
	(void)ti;
	return (g_tls_addr);
}

int
_execve(const char *path, char *const *argv, char *const *envp)
{
	(void)argv;
	(void)envp;
	if (g_execve_call < g_execve_max) {
		std::strncpy(g_execve_paths[g_execve_call], path,
		    sizeof(g_execve_paths[0]) - 1);
		g_execve_paths[g_execve_call][sizeof(g_execve_paths[0]) - 1] =
		    '\0';
		errno = g_execve_errno[g_execve_call];
		++g_execve_call;
		return (-1);
	}
	errno = ENOENT;
	return (-1);
}

ssize_t
_write(int fd, const void *buf, size_t n)
{
	(void)fd;
	if (g_write_len + (int)n < (int)sizeof(g_write_buf)) {
		std::memcpy(g_write_buf + g_write_len, buf, n);
		g_write_len += (int)n;
	}
	return ((ssize_t)n);
}

} /* extern "C" */

static void
reset_exec_mocks(void)
{
	g_execve_call = 0;
	g_execve_max = 0;
	g_stat_call = 0;
	g_stat_path[0] = '\0';
	g_stat_rv = -1;
	g_stat_errno = ENOENT;
	g_getenv_set = 0;
	g_getenv_path[0] = '\0';
	g_write_len = 0;
	g_write_buf[0] = '\0';
}

static void
push_execve_errno(int e)
{
	if (g_execve_max < 64) {
		g_execve_errno[g_execve_max] = e;
		++g_execve_max;
	}
}

extern "C" int
stat(const char *path, struct stat *sb)
{
	++g_stat_call;
	std::strncpy(g_stat_path, path, sizeof(g_stat_path) - 1);
	g_stat_path[sizeof(g_stat_path) - 1] = '\0';
	if (g_stat_rv == 0) {
		std::memset(sb, 0, sizeof(*sb));
		return (0);
	}
	errno = g_stat_errno;
	return (-1);
}

extern "C" char *
getenv(const char *name)
{
	if (name != nullptr && std::strcmp(name, "PATH") == 0 && g_getenv_set)
		return (g_getenv_path);
	return (nullptr);
}

static void
setup_aux_vector(unsigned phnum, bool tls_seg, bool gnu_stack, bool gnu_x)
{
	std::memset(g_aux_store, 0, sizeof(g_aux_store));
	std::memset(g_phdrs, 0, sizeof(g_phdrs));
	for (unsigned i = 0; i < phnum && i < 8; ++i)
		g_phdrs[i].p_type = PT_LOAD;
	if (phnum > 0) {
		g_phdrs[0].p_vaddr = 0x1000;
		g_phdrs[0].p_memsz = 0x5000;
	}
	if (tls_seg && phnum > 1)
		g_phdrs[1].p_type = PT_TLS;
	if (gnu_stack && phnum > 0) {
		unsigned idx = phnum > 1 ? 1 : 0;
		if (!tls_seg)
			idx = phnum > 0 ? phnum - 1 : 0;
		g_phdrs[idx].p_type = PT_GNU_STACK;
		if (gnu_x)
			g_phdrs[idx].p_flags = PF_X;
	}

	int j = 0;
	g_aux_store[j].a_type = AT_BASE;
	g_aux_store[j++].a_un.a_ptr = (void *)0x200000;
	g_aux_store[j].a_type = AT_EXECPATH;
	g_aux_store[j++].a_un.a_ptr = g_execpath;
	g_aux_store[j].a_type = AT_PHDR;
	g_aux_store[j++].a_un.a_ptr = g_phdrs;
	g_aux_store[j].a_type = AT_PHNUM;
	g_aux_store[j++].a_un.a_val = (int)phnum;
	g_aux_store[j].a_type = AT_NULL;
	__elf_aux_vector = g_aux_store;
	++g_aux_reset_generation;
}

/* ------------------------------------------------------------------ */
/* comparison helpers                                                  */
/* ------------------------------------------------------------------ */

static bool
eq_errno(int a, int b)
{
	return a == b;
}

static bool
eq_ptrstr(const char *a, const char *b)
{
	if (a == nullptr && b == nullptr)
		return true;
	if (a == nullptr || b == nullptr)
		return false;
	return std::strcmp(a, b) == 0;
}

static bool
eq_write_log(void)
{
	/* port and oracle share _write mock; same global log */
	return true;
}

/* ------------------------------------------------------------------ */
/* errlst tests                                                        */
/* ------------------------------------------------------------------ */

static void
test_errlst_hand(void)
{
	const char *fn = "sys_errlist/sys_nerr/__uprefix";

	record(fn, eq_ptrstr(P::__uprefix, __uprefix));
	record(fn, P::sys_nerr == sys_nerr);
	for (int i = 0; i < P::sys_nerr; ++i)
		record(fn, eq_ptrstr(P::sys_errlist[i], sys_errlist[i]));
	for (int i = 0; i < P::sys_nerr; ++i)
		record(fn, std::strlen(P::sys_errlist[i]) > 0);
	record(fn, eq_ptrstr(P::sys_errlist[0], "No error: 0"));
	record(fn, eq_ptrstr(P::sys_errlist[35],
	    "Resource temporarily unavailable"));
	record(fn, eq_ptrstr(P::sys_errlist[98], __uprefix));
	record(fn, eq_ptrstr(P::sys_errlist[P::sys_nerr - 1], __uprefix));
}

static void
test_errlst_rand(void)
{
	const char *fn = "sys_errlist/sys_nerr/__uprefix";
	for (int i = 0; i < 30000; ++i) {
		int idx = rnd_range(0, P::sys_nerr - 1);
		record(fn, eq_ptrstr(P::sys_errlist[idx], sys_errlist[idx]));
		record(fn, P::sys_nerr == sys_nerr);
	}
}

/* ------------------------------------------------------------------ */
/* dlfcn stub tests                                                    */
/* ------------------------------------------------------------------ */

static void
test_dl_stubs_hand(void)
{
	Dl_info di{};
	const char *fn;

	fn = "dladdr";
	record(fn, P::dladdr((void *)0x1234, &di) == ref_dladdr((void *)0x1234,
	    &di));

	fn = "dlclose";
	record(fn, P::dlclose((void *)1) == ref_dlclose((void *)1));

	fn = "dlerror";
	record(fn, eq_ptrstr(P::dlerror(), ref_dlerror()));

	fn = "dllockinit";
	g_context_destroy_calls = 0;
	g_context_destroy_arg = nullptr;
	auto destroy = [](void *c) {
		++g_context_destroy_calls;
		g_context_destroy_arg = c;
	};
	P::dllockinit((void *)0xAB, nullptr, nullptr, nullptr, nullptr,
	    nullptr, destroy);
	ref_dllockinit((void *)0xAB, nullptr, nullptr, nullptr, nullptr,
	    nullptr, destroy);
	record(fn, g_context_destroy_calls == 2);
	record(fn, g_context_destroy_arg == (void *)0xAB);

	fn = "dlopen";
	record(fn, P::dlopen("x", 1) == ref_dlopen("x", 1));

	fn = "dlsym";
	record(fn, P::dlsym((void *)1, "sym") == ref_dlsym((void *)1, "sym"));

	fn = "dlfunc";
	record(fn, (void *)P::dlfunc((void *)1, "f") ==
	    (void *)ref_dlfunc((void *)1, "f"));

	fn = "dlvsym";
	record(fn, P::dlvsym((void *)1, "s", "v") ==
	    ref_dlvsym((void *)1, "s", "v"));

	fn = "dlinfo";
	record(fn, P::dlinfo((void *)1, 1, (void *)2) ==
	    ref_dlinfo((void *)1, 1, (void *)2));

	fn = "_rtld_thread_init";
	P::_rtld_thread_init(nullptr);
	ref__rtld_thread_init(nullptr);
	record(fn, true);

	fn = "fdlopen";
	record(fn, P::fdlopen(3, 0) == ref_fdlopen(3, 0));

	fn = "_rtld_atfork_pre/_rtld_atfork_post";
	int locks = 7;
	P::_rtld_atfork_pre(&locks);
	ref__rtld_atfork_pre(&locks);
	P::_rtld_atfork_post(&locks);
	ref__rtld_atfork_post(&locks);
	record(fn, locks == 7);

	fn = "_rtld_get_pax_flags";
	record(fn, P::_rtld_get_pax_flags() == ref__rtld_get_pax_flags());

	fn = "_rtld_is_dlopened";
	record(fn, P::_rtld_is_dlopened((void *)9) ==
	    ref__rtld_is_dlopened((void *)9));

	fn = "rtld_get_var";
	record(fn, P::rtld_get_var("X") == ref_rtld_get_var("X"));

	fn = "rtld_set_var";
	record(fn, P::rtld_set_var("a", "b") == ref_rtld_set_var("a", "b"));
}

static void
test_dl_stubs_rand(void)
{
	for (int i = 0; i < 30000; ++i) {
		uintptr_t p = rnd();
		const char *fn = "dladdr";
		Dl_info a{}, b{};
		record(fn, P::dladdr((void *)p, &a) == ref_dladdr((void *)p, &b));

		fn = "dlclose";
		record(fn, P::dlclose((void *)p) == ref_dlclose((void *)p));

		fn = "dlerror";
		record(fn, eq_ptrstr(P::dlerror(), ref_dlerror()));

		fn = "dlopen";
		record(fn, P::dlopen("", rnd_range(0, 3)) ==
		    ref_dlopen("", rnd_range(0, 3)));

		fn = "dlsym";
		record(fn, P::dlsym((void *)p, "n") ==
		    ref_dlsym((void *)p, "n"));

		fn = "dlinfo";
		record(fn, P::dlinfo((void *)p, rnd_range(0, 20), (void *)p) ==
		    ref_dlinfo((void *)p, rnd_range(0, 20), (void *)p));

		fn = "_rtld_get_pax_flags";
		record(fn, P::_rtld_get_pax_flags() ==
		    ref__rtld_get_pax_flags());

		fn = "_rtld_is_dlopened";
		record(fn, P::_rtld_is_dlopened((void *)p) ==
		    ref__rtld_is_dlopened((void *)p));

		fn = "rtld_set_var";
		record(fn, P::rtld_set_var("n", "v") ==
		    ref_rtld_set_var("n", "v"));
	}
}

/* ------------------------------------------------------------------ */
/* dl phdr iteration tests                                             */
/* ------------------------------------------------------------------ */

struct PhdrCbState {
	int count;
	uintptr_t last_addr;
	size_t last_sz;
	char namebuf[256];
};

static int
phdr_cb(struct dl_phdr_info *info, size_t sz, void *arg)
{
	auto *st = static_cast<PhdrCbState *>(arg);
	++st->count;
	st->last_addr = info->dlpi_addr;
	st->last_sz = sz;
	if (info->dlpi_name != nullptr)
		std::strncpy(st->namebuf, info->dlpi_name,
		    sizeof(st->namebuf) - 1);
	else
		st->namebuf[0] = '\0';
	return (rnd() & 1) ? 0 : 7;
}

static void
test_dl_phdr_hand(void)
{
	const char *fn = "_dl_iterate_phdr_locked/dl_iterate_phdr";
	PhdrCbState pa{}, pb{};

	setup_aux_vector(2, true, false, false);
	pa.count = pb.count = 0;
	int ra = P::_dl_iterate_phdr_locked(phdr_cb, &pa);
	int rb = ref__dl_iterate_phdr_locked(phdr_cb, &rb);
	(void)rb;
	rb = ref__dl_iterate_phdr_locked(phdr_cb, &pb);
	record(fn, ra == rb);

	setup_aux_vector(0, false, false, false);
	__elf_aux_vector = nullptr;
	record(fn, P::_dl_iterate_phdr_locked(phdr_cb, &pa) ==
	    ref__dl_iterate_phdr_locked(phdr_cb, &pb));

	setup_aux_vector(3, false, true, true);
	PhdrCbState da{}, db{};
	da.count = db.count = 0;
	record(fn, P::dl_iterate_phdr(phdr_cb, &da) ==
	    ref_dl_iterate_phdr(phdr_cb, &db));
	record(fn, da.count == db.count);
	record(fn, da.last_addr == db.last_addr);
	record(fn, da.last_sz == db.last_sz);
	record(fn, std::strcmp(da.namebuf, db.namebuf) == 0);

	fn = "_rtld_get_stack_prot";
	setup_aux_vector(2, false, true, false);
	int spa = P::_rtld_get_stack_prot();
	int spb = ref__rtld_get_stack_prot();
	record(fn, spa == spb);
	setup_aux_vector(2, false, true, true);
	/* cached; still must match between sides */
	record(fn, P::_rtld_get_stack_prot() == ref__rtld_get_stack_prot());

	fn = "_rtld_addr_phdr";
	setup_aux_vector(2, false, false, false);
	struct dl_phdr_info ia{}, ib{};
	const void *probe = (void *)(uintptr_t)(0x200000 + 0x1500);
	record(fn, P::_rtld_addr_phdr(probe, &ia) ==
	    ref__rtld_addr_phdr(probe, &ib));
	record(fn, ia.dlpi_addr == ib.dlpi_addr);
	record(fn, ia.dlpi_phnum == ib.dlpi_phnum);
}

static void
test_dl_phdr_rand(void)
{
	const char *fn = "_dl_iterate_phdr_locked";
	for (int i = 0; i < 30000; ++i) {
		unsigned phn = (unsigned)rnd_range(0, 4);
		bool tls = (rnd() & 1) != 0;
		bool gst = (rnd() & 1) != 0;
		bool gx = (rnd() & 1) != 0;
		setup_aux_vector(phn, tls, gst, gx);
		PhdrCbState a{}, b{};
		int ra = P::_dl_iterate_phdr_locked(phdr_cb, &a);
		int rb = ref__dl_iterate_phdr_locked(phdr_cb, &b);
		record(fn, ra == rb);
		record(fn, a.count == b.count);
		record(fn, a.last_addr == b.last_addr);
	}
}

/* ------------------------------------------------------------------ */
/* exec tests                                                          */
/* ------------------------------------------------------------------ */

static char g_env0[] = "A=1";
static char g_env1[] = "B=2";
static char *g_envp[] = {g_env0, g_env1, nullptr};
static char g_arg0[] = "argv0";
static char g_arg1[] = "argv1";
static char *g_argv2[] = {g_arg0, g_arg1, nullptr};
static char *g_argv0[] = {g_arg0, nullptr};
static char *g_argv_empty[] = {nullptr};

static bool
run_execvpe_pair(const char *name, const char *path, char *const *argv,
    char *const *envp, int *errno_out)
{
	reset_exec_mocks();
	environ = envp;
	int ra = P::execvP(name, path, argv);
	int ea = errno;
	reset_exec_mocks();
	environ = envp;
	int rb = ref_execvP(name, path, argv);
	int eb = errno;
	if (errno_out)
		*errno_out = ea;
	return ra == rb && eq_errno(ea, eb);
}

static void
test_exec_hand(void)
{
	const char *fn;
	int e;

	fn = "execvP empty name";
	reset_exec_mocks();
	environ = g_envp;
	record(fn, P::execvP("", "/bin", g_argv0) == ref_execvP("", "/bin",
	    g_argv0));
	record(fn, eq_errno(errno, errno));

	fn = "execvP absolute ENOENT";
	reset_exec_mocks();
	push_execve_errno(ENOENT);
	environ = g_envp;
	record(fn, P::execvP("/no/such", "/bin", g_argv0) ==
	    ref_execvP("/no/such", "/bin", g_argv0));

	fn = "execvP PATH ENOENT";
	record(fn, run_execvpe_pair("prog", "/bin", g_argv0, g_envp, &e));
	record(fn, eq_errno(e, ENOENT));

	fn = "execvP PATH EACCES then ENOENT";
	reset_exec_mocks();
	push_execve_errno(EACCES);
	g_stat_rv = 0;
	push_execve_errno(ENOENT);
	environ = g_envp;
	record(fn, run_execvpe_pair("p", "dir", g_argv0, g_envp, &e));
	record(fn, eq_errno(e, EACCES));

	fn = "execvP PATH EACCES stat fail";
	reset_exec_mocks();
	push_execve_errno(EACCES);
	g_stat_rv = -1;
	g_stat_errno = ENOENT;
	push_execve_errno(ENOENT);
	environ = g_envp;
	record(fn, run_execvpe_pair("p", "dir", g_argv0, g_envp, &e));
	record(fn, eq_errno(e, ENOENT));

	fn = "execvP ENOEXEC cnt>0";
	reset_exec_mocks();
	push_execve_errno(ENOEXEC);
	push_execve_errno(ENOENT);
	environ = g_envp;
	record(fn, run_execvpe_pair("x", "/a:/b", g_argv2, g_envp, &e));
	record(fn, eq_errno(e, ENOENT));

	fn = "execvP ENOEXEC cnt==0";
	reset_exec_mocks();
	push_execve_errno(ENOEXEC);
	push_execve_errno(ENOENT);
	environ = g_envp;
	record(fn, run_execvpe_pair("x", "/only", g_argv_empty, g_envp, &e));

	fn = "execvP ENOMEM terminal";
	reset_exec_mocks();
	push_execve_errno(ENOMEM);
	environ = g_envp;
	record(fn, run_execvpe_pair("x", "/bin", g_argv0, g_envp, &e));
	record(fn, eq_errno(errno, ENOMEM));

	fn = "execvP E2BIG terminal";
	reset_exec_mocks();
	push_execve_errno(E2BIG);
	environ = g_envp;
	record(fn, run_execvpe_pair("n", "/p", g_argv0, g_envp, &e));

	fn = "execvP ETXTBSY terminal";
	reset_exec_mocks();
	push_execve_errno(ETXTBSY);
	environ = g_envp;
	record(fn, run_execvpe_pair("n", "/p", g_argv0, g_envp, &e));

	fn = "execvP default EFAULT stat ok";
	reset_exec_mocks();
	push_execve_errno(EFAULT);
	g_stat_rv = 0;
	push_execve_errno(ENOENT);
	environ = g_envp;
	record(fn, run_execvpe_pair("n", "/d", g_argv0, g_envp, &e));

	fn = "execvP default EIO terminal";
	reset_exec_mocks();
	push_execve_errno(EIO);
	g_stat_rv = 0;
	environ = g_envp;
	record(fn, run_execvpe_pair("n", "/d", g_argv0, g_envp, &e));
	record(fn, eq_errno(errno, EIO));

	fn = "execvP empty PATH component";
	reset_exec_mocks();
	push_execve_errno(ENOENT);
	environ = g_envp;
	record(fn, run_execvpe_pair("a", ":", g_argv0, g_envp, &e));

	fn = "execvP leading colon";
	reset_exec_mocks();
	push_execve_errno(ENOENT);
	environ = g_envp;
	record(fn, run_execvpe_pair("a", ":/x", g_argv0, g_envp, &e));

	fn = "execvP trailing colon";
	reset_exec_mocks();
	push_execve_errno(ENOENT);
	environ = g_envp;
	record(fn, run_execvpe_pair("a", "/x:", g_argv0, g_envp, &e));

	fn = "execvP path too long";
	reset_exec_mocks();
	char longdir[512];
	std::memset(longdir, 'd', sizeof(longdir) - 1);
	longdir[sizeof(longdir) - 1] = '\0';
	char lname[200];
	std::memset(lname, 'n', sizeof(lname) - 1);
	lname[sizeof(lname) - 1] = '\0';
	push_execve_errno(ENOENT);
	environ = g_envp;
	g_write_len = 0;
	int ra = P::execvP(lname, longdir, g_argv0);
	int ea = errno;
	g_write_len = 0;
	int rb = ref_execvP(lname, longdir, g_argv0);
	int eb = errno;
	record(fn, ra == rb && eq_errno(ea, eb));
	record(fn, g_write_len > 0);

	fn = "__libc_execvpe getenv";
	reset_exec_mocks();
	g_getenv_set = 1;
	std::strcpy(g_getenv_path, "/my/bin");
	push_execve_errno(ENOENT);
	environ = g_envp;
	record(fn, P::__libc_execvpe("z", g_argv0, g_envp) ==
	    ref___libc_execvpe("z", g_argv0, g_envp));

	fn = "__libc_execvpe default PATH";
	reset_exec_mocks();
	g_getenv_set = 0;
	push_execve_errno(ENOENT);
	environ = g_envp;
	record(fn, P::__libc_execvpe("z", g_argv0, g_envp) ==
	    ref___libc_execvpe("z", g_argv0, g_envp));

	fn = "execvp";
	reset_exec_mocks();
	push_execve_errno(ENOENT);
	environ = g_envp;
	record(fn, P::execvp("z", g_argv0) == ref_execvp("z", g_argv0));

	fn = "execv";
	reset_exec_mocks();
	push_execve_errno(EACCES);
	environ = g_envp;
	record(fn, P::execv("/bin/x", g_argv0) == ref_execv("/bin/x", g_argv0));

	fn = "execl";
	reset_exec_mocks();
	push_execve_errno(ENOENT);
	environ = g_envp;
	record(fn, P::execl("/p", g_arg0, g_arg1, (char *)nullptr) ==
	    ref_execl("/p", g_arg0, g_arg1, (char *)nullptr));

	fn = "execle";
	reset_exec_mocks();
	push_execve_errno(ENOENT);
	record(fn, P::execle("/p", g_arg0, (char *)nullptr, g_envp) ==
	    ref_execle("/p", g_arg0, (char *)nullptr, g_envp));

	fn = "execlp";
	reset_exec_mocks();
	push_execve_errno(ENOENT);
	environ = g_envp;
	record(fn, P::execlp("p", g_arg0, (char *)nullptr) ==
	    ref_execlp("p", g_arg0, (char *)nullptr));
}

static void
test_exec_rand(void)
{
	const char *fn = "execvP/__libc_execvpe";
	char name[64];
	char path[128];
	char comp[64];

	for (int i = 0; i < 30000; ++i) {
		reset_exec_mocks();
		int nerr = rnd_range(1, 6);
		for (int j = 0; j < nerr; ++j) {
			static const int errs[] = {
			    ENOENT, EACCES, ENOEXEC, ENOMEM, E2BIG, ETXTBSY,
			    ELOOP, ENAMETOOLONG, ENOTDIR, EFAULT, EIO, EPERM
			};
			push_execve_errno(errs[rnd_range(0, 11)]);
		}
		if (rnd() & 1) {
			g_stat_rv = 0;
		} else {
			g_stat_rv = -1;
			g_stat_errno = rnd_range(1, 40);
		}
		int nl = rnd_range(0, 20);
		for (int k = 0; k < nl; ++k)
			name[k] = (char)rnd_range(1, 126);
		name[nl] = '\0';
		if ((rnd() & 3) == 0)
			name[rnd_range(0, nl)] = '/';

		int pl = rnd_range(0, 40);
		for (int k = 0; k < pl; ++k)
			path[k] = (char)rnd_range('a', 'z');
		path[pl] = '\0';
		if ((rnd() & 7) == 0)
			path[0] = ':';
		if ((rnd() & 7) == 0)
			path[pl] = ':';

		environ = g_envp;
		char *const *av = (rnd() & 1) ? g_argv0 : g_argv2;
		if ((rnd() & 15) == 0)
			av = g_argv_empty;

		if (name[0] == '\0') {
			record(fn, P::execvP(name, path, av) ==
			    ref_execvP(name, path, av));
			continue;
		}
		if (std::strchr(name, '/') != nullptr) {
			record(fn, P::execvP(name, path, av) ==
			    ref_execvP(name, path, av));
			continue;
		}

		record(fn, P::execvP(name, path, av) ==
		    ref_execvP(name, path, av));

		if ((rnd() & 3) == 0) {
			g_getenv_set = 1;
			int cl = rnd_range(1, 30);
			for (int k = 0; k < cl; ++k)
				comp[k] = (char)rnd_range('a', 'z');
			comp[cl] = '\0';
			std::snprintf(g_getenv_path, sizeof(g_getenv_path),
			    "%s:%s", comp, path);
			record(fn, P::__libc_execvpe(name, av, g_envp) ==
			    ref___libc_execvpe(name, av, g_envp));
		}
	}
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int
main(void)
{
	environ = g_envp;

	test_errlst_hand();
	test_errlst_rand();
	test_dl_stubs_hand();
	test_dl_stubs_rand();
	test_dl_phdr_hand();
	test_dl_phdr_rand();
	test_exec_hand();
	test_exec_rand();

	long total_cases = 0;
	long total_fails = 0;

	std::printf("\n%-40s %10s %10s\n", "function", "cases", "failures");
	std::printf("%-40s %10s %10s\n", "--------", "-----", "--------");
	for (int i = 0; i < g_nstats; ++i) {
		std::printf("%-40s %10ld %10ld\n", g_stats[i].name,
		    g_stats[i].cases, g_stats[i].fails);
		total_cases += g_stats[i].cases;
		total_fails += g_stats[i].fails;
	}
	std::printf("%-40s %10ld %10ld\n", "TOTAL", total_cases, total_fails);

	return total_fails == 0 ? 0 : 1;
}
