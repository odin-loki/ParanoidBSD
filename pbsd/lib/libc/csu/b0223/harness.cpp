/*
 * harness.cpp -- differential test for PBSD batch b0223.
 *
 * Every ported entry point is driven against the ref_ oracle in oracle.c
 * with hand-written edge cases and a fixed-seed randomised sweep.
 */

#include <cerrno>
#include <csetjmp>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

import pbsd.lib.libc.csu.b0223;

namespace port = pbsd::lib_libc_csu::b0223;

extern "C" {
typedef std::uint64_t Elf_Addr;
typedef std::int64_t Elf_Signed;

typedef struct {
	Elf_Addr a_type;
	union {
		Elf_Addr a_val;
	} a_un;
} Elf_Auxinfo;

typedef struct {
	Elf_Addr r_offset;
	Elf_Addr r_info;
	Elf_Signed r_addend;
} Elf_Rela;

extern char **environ;
extern const char *__progname;

extern int mock_iplt_n;
extern Elf_Rela __rela_iplt_start[64];
extern int mock_preinit_n;
extern void (*__preinit_array_start[64])(int, char **, char **);
extern int mock_init_n;
extern void (*__init_array_start[64])(int, char **, char **);
extern int mock_fini_n;
extern void (*__fini_array_start[64])(void);

extern int mock_crt1_handle_rela_calls;
extern const Elf_Rela *mock_crt1_handle_rela_args[256];
extern int mock_ifunc_init_calls;
extern const Elf_Auxinfo *mock_ifunc_init_args[64];
extern int mock_init_tls_calls;
extern int mock_init_calls;
extern int mock_fini_calls;
extern int mock_mcleanup_atexit;
extern int mock_monstartup_calls;
extern int *mock_monstartup_eprolp;
extern int *mock_monstartup_etextp;
extern int mock_atexit_count;
extern void (*mock_atexit_funcs[128])(void);
extern int mock_exit_called;
extern int mock_exit_status;
extern jmp_buf mock_exit_jmp;
extern int mock_preinit_calls;
extern int mock_init_array_calls;
extern int mock_fini_array_calls;
extern int mock_preinit_argc[64];
extern char **mock_preinit_argv[64];
extern char **mock_preinit_env[64];
extern int mock_init_array_argc[64];
extern char **mock_init_array_argv[64];
extern char **mock_init_array_env[64];

extern int pbsd_dynamic_storage;
extern int *pbsd_dynamic_ptr;

void ref_reset_mocks(void);
void ref_preinit_hook(int, char **, char **);
void ref_init_array_hook(int, char **, char **);
void ref_fini_array_hook(void);

void ref_test_process_irelocs(void);
void ref_test_finalizer(void);
void ref_test_handle_static_init(int, char **, char **);
void ref_test_handle_argv(int, char *[], char **);
void ref_test_handle_irelocs(char *[]);
void ref___libc_start1(int, char *[], char *[], void (*)(void),
    int (*)(int, char *[], char *[]));
void ref___libc_start1_gcrt(int, char *[], char *[], void (*)(void),
    int (*)(int, char *[], char *[]), int *, int *);
}

namespace {

constexpr int MAX_REPORT = 10;
constexpr long RANDOM_ITERATIONS = 200000;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

Stat st_process_irelocs{"process_irelocs", 0, 0, 0};
Stat st_finalizer{"finalizer", 0, 0, 0};
Stat st_handle_static_init{"handle_static_init", 0, 0, 0};
Stat st_handle_argv{"handle_argv", 0, 0, 0};
Stat st_handle_irelocs{"handle_irelocs", 0, 0, 0};
Stat st_libc_start1{"__libc_start1", 0, 0, 0};
Stat st_libc_start1_gcrt{"__libc_start1_gcrt", 0, 0, 0};

std::uint64_t prng_state;

void prng_seed(std::uint64_t seed) { prng_state = seed; }

std::uint64_t prng_next()
{
	std::uint64_t z = (prng_state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

std::uint32_t prng_u32() { return static_cast<std::uint32_t>(prng_next()); }

bool want_report(Stat &s) { return s.reported++ < MAX_REPORT; }

void note_fail(Stat &s) { s.fails++; }

bool dynamic_linked() { return pbsd_dynamic_ptr != nullptr; }

void set_link_mode(bool dynamic)
{
	if (dynamic)
		pbsd_dynamic_ptr = &pbsd_dynamic_storage;
	else
		pbsd_dynamic_ptr = nullptr;
}

struct Snapshot {
	char **environ;
	const char *progname;
	int rela_calls;
	const Elf_Rela *rela_args[256];
	int ifunc_calls;
	const Elf_Auxinfo *ifunc_args[64];
	int init_tls;
	int init;
	int fini;
	int atexit_count;
	void (*atexit_funcs[128])(void);
	int preinit_calls;
	int init_array_calls;
	int fini_array_calls;
	int preinit_argc[64];
	char **preinit_argv[64];
	char **preinit_env[64];
	int init_array_argc[64];
	char **init_array_argv[64];
	char **init_array_env[64];
	int monstartup_calls;
	int *mon_eprolp;
	int *mon_etextp;
	int errno_value;
};

void capture(Snapshot &s)
{
	s.environ = environ;
	s.progname = __progname;
	s.rela_calls = mock_crt1_handle_rela_calls;
	for (int i = 0; i < 256; i++)
		s.rela_args[i] = mock_crt1_handle_rela_args[i];
	s.ifunc_calls = mock_ifunc_init_calls;
	for (int i = 0; i < 64; i++)
		s.ifunc_args[i] = mock_ifunc_init_args[i];
	s.init_tls = mock_init_tls_calls;
	s.init = mock_init_calls;
	s.fini = mock_fini_calls;
	s.atexit_count = mock_atexit_count;
	for (int i = 0; i < 128; i++)
		s.atexit_funcs[i] = mock_atexit_funcs[i];
	s.preinit_calls = mock_preinit_calls;
	s.init_array_calls = mock_init_array_calls;
	s.fini_array_calls = mock_fini_array_calls;
	for (int i = 0; i < 64; i++) {
		s.preinit_argc[i] = mock_preinit_argc[i];
		s.preinit_argv[i] = mock_preinit_argv[i];
		s.preinit_env[i] = mock_preinit_env[i];
		s.init_array_argc[i] = mock_init_array_argc[i];
		s.init_array_argv[i] = mock_init_array_argv[i];
		s.init_array_env[i] = mock_init_array_env[i];
	}
	s.monstartup_calls = mock_monstartup_calls;
	s.mon_eprolp = mock_monstartup_eprolp;
	s.mon_etextp = mock_monstartup_etextp;
	s.errno_value = errno;
}

bool snap_same(const Snapshot &a, const Snapshot &b)
{
	if (a.environ != b.environ)
		return false;
	if ((a.progname == nullptr) != (b.progname == nullptr))
		return false;
	if (a.progname != nullptr && std::strcmp(a.progname, b.progname) != 0)
		return false;
	if (a.rela_calls != b.rela_calls)
		return false;
	for (int i = 0; i < a.rela_calls; i++) {
		std::ptrdiff_t oa = a.rela_args[i] - __rela_iplt_start;
		std::ptrdiff_t ob = b.rela_args[i] - __rela_iplt_start;
		if (oa != ob)
			return false;
	}
	if (a.ifunc_calls != b.ifunc_calls)
		return false;
	for (int i = 0; i < a.ifunc_calls; i++) {
		if (a.ifunc_args[i] != b.ifunc_args[i])
			return false;
	}
	if (a.init_tls != b.init_tls || a.init != b.init || a.fini != b.fini)
		return false;
	if (a.atexit_count != b.atexit_count)
		return false;
	if (a.preinit_calls != b.preinit_calls ||
	    a.init_array_calls != b.init_array_calls ||
	    a.fini_array_calls != b.fini_array_calls)
		return false;
	for (int i = 0; i < a.preinit_calls; i++) {
		if (a.preinit_argc[i] != b.preinit_argc[i] ||
		    a.preinit_argv[i] != b.preinit_argv[i] ||
		    a.preinit_env[i] != b.preinit_env[i])
			return false;
	}
	for (int i = 0; i < a.init_array_calls; i++) {
		if (a.init_array_argc[i] != b.init_array_argc[i] ||
		    a.init_array_argv[i] != b.init_array_argv[i] ||
		    a.init_array_env[i] != b.init_array_env[i])
			return false;
	}
	if (a.monstartup_calls != b.monstartup_calls ||
	    a.mon_eprolp != b.mon_eprolp || a.mon_etextp != b.mon_etextp)
		return false;
	if (a.errno_value != b.errno_value)
		return false;
	return true;
}

void setup_rela(int n, const Elf_Rela *entries)
{
	mock_iplt_n = n;
	if (entries != nullptr)
		std::memcpy(__rela_iplt_start, entries,
		    static_cast<std::size_t>(n) * sizeof(Elf_Rela));
}

void setup_arrays(int pre, int init, int fini)
{
	mock_preinit_n = pre;
	mock_init_n = init;
	mock_fini_n = fini;
	for (int i = 0; i < pre; i++)
		__preinit_array_start[i] = ref_preinit_hook;
	for (int i = 0; i < init; i++)
		__init_array_start[i] = ref_init_array_hook;
	for (int i = 0; i < fini; i++)
		__fini_array_start[i] = ref_fini_array_hook;
}

struct EnvStack {
	std::vector<char> storage;
	std::vector<char *> ptrs;
	std::vector<Elf_Auxinfo> aux;

	char *add_cstr(const char *s)
	{
		std::size_t off = storage.size();
		std::size_t len = std::strlen(s) + 1;
		storage.resize(off + len);
		std::memcpy(storage.data() + off, s, len);
		return storage.data() + off;
	}

	char **finish(const Elf_Auxinfo *auxv, int aux_count)
	{
		ptrs.clear();
		aux.clear();
		for (const char *const *p = ptrs_start(); p != nullptr && *p != nullptr;
		    ++p)
			ptrs.push_back(const_cast<char *>(*p));
		ptrs.push_back(nullptr);
		if (auxv != nullptr) {
			for (int i = 0; i < aux_count; i++)
				aux.push_back(auxv[i]);
		}
		aux.push_back({0, {0}});
		for (const auto &a : aux)
			ptrs.push_back(reinterpret_cast<char *>(
			    const_cast<Elf_Auxinfo *>(&a)));
		return ptrs.data();
	}

	void reset_env(const std::vector<const char *> &envs,
	    const Elf_Auxinfo *auxv, int aux_count)
	{
		storage.clear();
		ptrs.clear();
		aux.clear();
		for (const char *e : envs) {
			if (e == nullptr)
				break;
			ptrs.push_back(add_cstr(e));
		}
		ptrs.push_back(nullptr);
		if (auxv != nullptr) {
			for (int i = 0; i < aux_count; i++)
				aux.push_back(auxv[i]);
		}
		aux.push_back({0, {0}});
		for (const auto &a : aux)
			ptrs.push_back(reinterpret_cast<char *>(
			    const_cast<Elf_Auxinfo *>(&a)));
	}

	const char *const *ptrs_start() const { return nullptr; }

	char **data() { return ptrs.data(); }
};

void reset_all()
{
	ref_reset_mocks();
}

bool test_process_irelocs_case(Stat &st, int n, const Elf_Rela *entries)
{
	st.cases++;
	reset_all();
	setup_rela(n, entries);

	Snapshot ref_snap, port_snap;
	ref_test_process_irelocs();
	capture(ref_snap);

	reset_all();
	setup_rela(n, entries);
	port::test_process_irelocs();
	capture(port_snap);

	if (!snap_same(ref_snap, port_snap)) {
		note_fail(st);
		if (want_report(st))
			std::printf("  FAIL process_irelocs n=%d rela_calls ref=%d port=%d\n",
			    n, ref_snap.rela_calls, port_snap.rela_calls);
		return false;
	}
	return true;
}

bool test_finalizer_case(Stat &st, int fini_n, std::uintptr_t fn0,
    std::uintptr_t fn1)
{
	st.cases++;
	reset_all();
	setup_arrays(0, 0, fini_n);
	if (fini_n > 0)
		__fini_array_start[0] =
		    reinterpret_cast<void (*)(void)>(fn0);
	if (fini_n > 1)
		__fini_array_start[1] =
		    reinterpret_cast<void (*)(void)>(fn1);

	Snapshot ref_snap, port_snap;
	ref_test_finalizer();
	capture(ref_snap);

	reset_all();
	setup_arrays(0, 0, fini_n);
	if (fini_n > 0)
		__fini_array_start[0] =
		    reinterpret_cast<void (*)(void)>(fn0);
	if (fini_n > 1)
		__fini_array_start[1] =
		    reinterpret_cast<void (*)(void)>(fn1);
	port::test_finalizer();
	capture(port_snap);

	if (!snap_same(ref_snap, port_snap)) {
		note_fail(st);
		if (want_report(st))
			std::printf(
			    "  FAIL finalizer fini_n=%d fini_calls ref=%d port=%d\n",
			    fini_n, ref_snap.fini_array_calls,
			    port_snap.fini_array_calls);
		return false;
	}
	return true;
}

bool test_handle_static_init_case(Stat &st, int argc, char **argv, char **env,
    int pre, int init)
{
	st.cases++;
	reset_all();
	setup_arrays(pre, init, 0);

	Snapshot ref_snap, port_snap;
	ref_test_handle_static_init(argc, argv, env);
	capture(ref_snap);

	reset_all();
	setup_arrays(pre, init, 0);
	port::test_handle_static_init(argc, argv, env);
	capture(port_snap);

	if (!snap_same(ref_snap, port_snap)) {
		note_fail(st);
		if (want_report(st))
			std::printf(
			    "  FAIL handle_static_init pre=%d init=%d atexit ref=%d port=%d\n",
			    pre, init, ref_snap.atexit_count,
			    port_snap.atexit_count);
		return false;
	}
	return true;
}

bool test_handle_argv_case(Stat &st, int argc, char **argv, char **env,
    bool preset_environ)
{
	st.cases++;
	reset_all();
	if (preset_environ)
		environ = env;

	Snapshot ref_snap, port_snap;
	ref_test_handle_argv(argc, argv, env);
	capture(ref_snap);

	reset_all();
	if (preset_environ)
		environ = env;
	port::test_handle_argv(argc, argv, env);
	capture(port_snap);

	if (!snap_same(ref_snap, port_snap)) {
		note_fail(st);
		if (want_report(st))
			std::printf(
			    "  FAIL handle_argv argc=%d prog ref=%s port=%s\n",
			    argc,
			    ref_snap.progname ? ref_snap.progname : "(null)",
			    port_snap.progname ? port_snap.progname : "(null)");
		return false;
	}
	return true;
}

bool test_handle_irelocs_case(Stat &st, EnvStack &stack,
    const std::vector<const char *> &envs, const Elf_Auxinfo *auxv,
    int aux_count, int rela_n)
{
	st.cases++;
	reset_all();
	stack.reset_env(envs, auxv, aux_count);
	setup_rela(rela_n, __rela_iplt_start);

	Snapshot ref_snap, port_snap;
	ref_test_handle_irelocs(stack.data());
	capture(ref_snap);

	reset_all();
	stack.reset_env(envs, auxv, aux_count);
	setup_rela(rela_n, __rela_iplt_start);
	port::test_handle_irelocs(stack.data());
	capture(port_snap);

	if (!snap_same(ref_snap, port_snap)) {
		note_fail(st);
		if (want_report(st))
			std::printf(
			    "  FAIL handle_irelocs rela=%d ifunc ref=%d port=%d\n",
			    rela_n, ref_snap.ifunc_calls, port_snap.ifunc_calls);
		return false;
	}
	return true;
}

static int cleanup_called;
static void test_cleanup(void) { cleanup_called++; }

static int main_return;
static int test_main(int argc, char *argv[], char *env[])
{
	(void)argc;
	(void)argv;
	(void)env;
	return main_return;
}

bool test_libc_start1_case(Stat &st, int argc, char **argv, char **env,
    int pre, int init, int rela_n, int main_ret, bool dynamic)
{
	st.cases++;
	reset_all();
	set_link_mode(dynamic);
	cleanup_called = 0;
	main_return = main_ret;
	setup_arrays(pre, init, 0);
	setup_rela(rela_n, nullptr);

	Snapshot ref_snap, port_snap;
	int ref_j = setjmp(mock_exit_jmp);
	if (ref_j == 0)
		ref___libc_start1(argc, argv, env, test_cleanup, test_main);
	capture(ref_snap);
	int ref_exit = mock_exit_called ? mock_exit_status : -1;

	reset_all();
	set_link_mode(dynamic);
	cleanup_called = 0;
	main_return = main_ret;
	setup_arrays(pre, init, 0);
	setup_rela(rela_n, nullptr);
	int port_j = setjmp(mock_exit_jmp);
	if (port_j == 0)
		port::libc_start1(argc, argv, env, test_cleanup, test_main);
	capture(port_snap);
	int port_exit = mock_exit_called ? mock_exit_status : -1;

	if (ref_exit != port_exit || !snap_same(ref_snap, port_snap)) {
		note_fail(st);
		if (want_report(st))
			std::printf(
			    "  FAIL __libc_start1 exit ref=%d port=%d dynamic=%d\n",
			    ref_exit, port_exit, dynamic ? 1 : 0);
		return false;
	}
	return true;
}

bool test_libc_start1_gcrt_case(Stat &st, int argc, char **argv, char **env,
    int pre, int init, int main_ret, int eprol, int etext, bool dynamic)
{
	st.cases++;
	reset_all();
	set_link_mode(dynamic);
	cleanup_called = 0;
	main_return = main_ret;
	setup_arrays(pre, init, 0);
	int eprol_v = eprol;
	int etext_v = etext;

	Snapshot ref_snap, port_snap;
	int ref_j = setjmp(mock_exit_jmp);
	if (ref_j == 0)
		ref___libc_start1_gcrt(argc, argv, env, test_cleanup, test_main,
		    &eprol_v, &etext_v);
	capture(ref_snap);
	int ref_exit = mock_exit_called ? mock_exit_status : -1;

	reset_all();
	set_link_mode(dynamic);
	cleanup_called = 0;
	main_return = main_ret;
	setup_arrays(pre, init, 0);
	eprol_v = eprol;
	etext_v = etext;
	int port_j = setjmp(mock_exit_jmp);
	if (port_j == 0)
		port::libc_start1_gcrt(argc, argv, env, test_cleanup, test_main,
		    &eprol_v, &etext_v);
	capture(port_snap);
	int port_exit = mock_exit_called ? mock_exit_status : -1;

	if (ref_exit != port_exit || !snap_same(ref_snap, port_snap)) {
		note_fail(st);
		if (want_report(st))
			std::printf(
			    "  FAIL __libc_start1_gcrt exit ref=%d port=%d dynamic=%d\n",
			    ref_exit, port_exit, dynamic ? 1 : 0);
		return false;
	}
	return true;
}

void run_process_irelocs_tests()
{
	Elf_Rela zero{};
	test_process_irelocs_case(st_process_irelocs, 0, nullptr);
	test_process_irelocs_case(st_process_irelocs, 1, &zero);

	Elf_Rela multi[3] = {
	    {0, 37, 0},
	    {8, 38, -1},
	    {0x80, 0xFF, 0x7FFFFFFF},
	};
	test_process_irelocs_case(st_process_irelocs, 3, multi);

	prng_seed(0xB0223ULL);
	for (long i = 0; i < RANDOM_ITERATIONS; i++) {
		int n = static_cast<int>(prng_u32() % 9);
		Elf_Rela entries[9];
		for (int j = 0; j < n; j++) {
			entries[j].r_offset = prng_u32();
			entries[j].r_info = prng_u32();
			entries[j].r_addend = static_cast<Elf_Signed>(prng_next());
		}
		test_process_irelocs_case(st_process_irelocs, n, entries);
	}
}

void run_finalizer_tests()
{
	test_finalizer_case(st_finalizer, 0, 0, 0);
	test_finalizer_case(st_finalizer, 1,
	    reinterpret_cast<std::uintptr_t>(ref_fini_array_hook), 0);
	test_finalizer_case(st_finalizer, 2,
	    reinterpret_cast<std::uintptr_t>(ref_fini_array_hook),
	    reinterpret_cast<std::uintptr_t>(ref_fini_array_hook));
	test_finalizer_case(st_finalizer, 2, 0, 0);
	test_finalizer_case(st_finalizer, 2, 1, 1);
	test_finalizer_case(st_finalizer, 3,
	    reinterpret_cast<std::uintptr_t>(ref_fini_array_hook), 0);

	prng_seed(0xF1BA11EULL);
	for (long i = 0; i < RANDOM_ITERATIONS; i++) {
		int n = static_cast<int>(prng_u32() % 6);
		std::uintptr_t f0 = (prng_u32() & 3) == 0 ?
		    0 :
		    reinterpret_cast<std::uintptr_t>(ref_fini_array_hook);
		std::uintptr_t f1 = (prng_u32() & 3) == 0 ?
		    0 :
		    reinterpret_cast<std::uintptr_t>(ref_fini_array_hook);
		test_finalizer_case(st_finalizer, n, f0, f1);
	}
}

void run_handle_static_init_tests()
{
	char a0[] = "prog";
	char *argv1[] = {a0, nullptr};
	char *env0[] = {nullptr};

	test_handle_static_init_case(st_handle_static_init, 0, argv1, env0, 0, 0);
	test_handle_static_init_case(st_handle_static_init, 1, argv1, env0, 1, 0);
	test_handle_static_init_case(st_handle_static_init, 1, argv1, env0, 0, 1);
	test_handle_static_init_case(st_handle_static_init, 1, argv1, env0, 2, 2);
	test_handle_static_init_case(st_handle_static_init, 0, argv1, env0, 1, 1);

	prng_seed(0x57414943);
	for (long i = 0; i < RANDOM_ITERATIONS; i++) {
		int pre = static_cast<int>(prng_u32() % 5);
		int init = static_cast<int>(prng_u32() % 5);
		test_handle_static_init_case(st_handle_static_init, 1, argv1, env0,
		    pre, init);
	}
}

void run_handle_argv_tests()
{
	char empty[] = "";
	char slash[] = "/";
	char name[] = "prog";
	char path[] = "/usr/bin/prog";
	char trail[] = "/usr/bin/";
	char multi[] = "a/b/c/d";
	char high[] = "\xff/bin/\xfe";
	char *argv0[] = {empty, nullptr};
	char *argv1[] = {name, nullptr};
	char *argv2[] = {path, nullptr};
	char *argv3[] = {trail, nullptr};
	char *argv4[] = {multi, nullptr};
	char *argv5[] = {slash, nullptr};
	char *argv6[] = {high, nullptr};
	char *env[] = {(char *)"K=V", nullptr};

	test_handle_argv_case(st_handle_argv, 0, argv0, env, false);
	test_handle_argv_case(st_handle_argv, 1, argv0, env, false);
	test_handle_argv_case(st_handle_argv, 1, argv1, env, false);
	test_handle_argv_case(st_handle_argv, 1, argv2, env, false);
	test_handle_argv_case(st_handle_argv, 1, argv3, env, false);
	test_handle_argv_case(st_handle_argv, 1, argv4, env, false);
	test_handle_argv_case(st_handle_argv, 1, argv5, env, false);
	test_handle_argv_case(st_handle_argv, 1, argv6, env, false);
	test_handle_argv_case(st_handle_argv, 1, argv1, env, true);
	test_handle_argv_case(st_handle_argv, 2, argv2, env, false);

	char *nullargv[] = {nullptr, nullptr};
	test_handle_argv_case(st_handle_argv, 1, nullargv, env, false);
	test_handle_argv_case(st_handle_argv, 0, nullargv, env, false);

	prng_seed(0x41524756);
	for (long i = 0; i < RANDOM_ITERATIONS; i++) {
		std::vector<char> buf(1 + (prng_u32() % 64));
		for (auto &c : buf)
			c = static_cast<char>(prng_u32() & 0xFF);
		buf.back() = '\0';
		char *av[] = {buf.data(), nullptr};
		int ac = (prng_u32() & 1) ? 1 : 0;
		bool preset = (prng_u32() & 1) != 0;
		test_handle_argv_case(st_handle_argv, ac, av, env, preset);
	}
}

void run_handle_irelocs_tests()
{
	EnvStack stack;
	Elf_Auxinfo aux1[] = {{3, {0x1000}}, {4, {0x2000}}};

	test_handle_irelocs_case(st_handle_irelocs, stack, {}, aux1, 2, 0);
	test_handle_irelocs_case(st_handle_irelocs, stack, {"A=1", nullptr}, aux1,
	    2, 0);
	test_handle_irelocs_case(st_handle_irelocs, stack, {"A=1", "B=2", nullptr},
	    aux1, 2, 1);

	Elf_Rela rela1 = {16, 37, 8};
	setup_rela(1, &rela1);
	test_handle_irelocs_case(st_handle_irelocs, stack, {"X=1", nullptr},
	    aux1, 2, 1);

	prng_seed(0x1E3E0CULL);
	for (long i = 0; i < RANDOM_ITERATIONS; i++) {
		int envc = static_cast<int>(prng_u32() % 4);
		std::vector<const char *> envs;
		for (int e = 0; e < envc; e++) {
			static thread_local char es[4][32];
			std::snprintf(es[e], sizeof(es[e]), "%c=%u",
			    static_cast<char>('A' + e), prng_u32());
			envs.push_back(es[e]);
		}
		envs.push_back(nullptr);
		Elf_Auxinfo ax[3];
		int axc = static_cast<int>(prng_u32() % 3);
		for (int a = 0; a < axc; a++) {
			ax[a].a_type = prng_u32();
			ax[a].a_un.a_val = prng_next();
		}
		int rn = static_cast<int>(prng_u32() % 4);
		Elf_Rela rs[4];
		for (int r = 0; r < rn; r++) {
			rs[r].r_offset = prng_u32();
			rs[r].r_info = prng_u32();
			rs[r].r_addend = static_cast<Elf_Signed>(prng_next());
		}
		setup_rela(rn, rs);
		test_handle_irelocs_case(st_handle_irelocs, stack, envs, ax, axc, rn);
	}
}

void run_libc_start1_tests()
{
	EnvStack stack;
	Elf_Auxinfo aux1[] = {{3, {0x1000}}, {4, {0x2000}}};
	stack.reset_env({"Z=1", nullptr}, aux1, 2);
	char *name[] = {(char *)"tool", nullptr};
	char **argv = name;
	char **env = stack.data();

	test_libc_start1_case(st_libc_start1, 1, argv, env, 0, 0, 0, 0, false);
	test_libc_start1_case(st_libc_start1, 1, argv, env, 1, 1, 0, 42, false);
	test_libc_start1_case(st_libc_start1, 1, argv, env, 0, 0, 0, -1, false);
	test_libc_start1_case(st_libc_start1, 0, argv, env, 0, 0, 0, 7, false);
	test_libc_start1_case(st_libc_start1, 1, argv, env, 0, 0, 0, 0, true);
	test_libc_start1_case(st_libc_start1, 1, argv, env, 1, 1, 0, 42, true);

	prng_seed(0x5A471701ULL);
	for (long i = 0; i < RANDOM_ITERATIONS; i++) {
		bool dynamic = (prng_u32() & 1) != 0;
		int pre = dynamic ? 0 : static_cast<int>(prng_u32() % 3);
		int init = dynamic ? 0 : static_cast<int>(prng_u32() % 3);
		int ret = static_cast<int>(prng_u32() % 200) - 50;
		test_libc_start1_case(st_libc_start1, 1, argv, env, pre, init, 0,
		    ret, dynamic);
	}
}

void run_libc_start1_gcrt_tests()
{
	EnvStack stack;
	Elf_Auxinfo aux1[] = {{5, {0x3000}}};
	stack.reset_env({}, aux1, 1);
	char name[] = "gcrt";
	char *argv[] = {name, nullptr};
	char **env = stack.data();

	test_libc_start1_gcrt_case(st_libc_start1_gcrt, 1, argv, env, 0, 0, 0, 0,
	    0, false);
	test_libc_start1_gcrt_case(st_libc_start1_gcrt, 1, argv, env, 1, 1, 99,
	    0x1000, 0x2000, false);
	test_libc_start1_gcrt_case(st_libc_start1_gcrt, 1, argv, env, 0, 0, 0, 0,
	    0, true);

	prng_seed(0x6C127701ULL);
	for (long i = 0; i < RANDOM_ITERATIONS; i++) {
		bool dynamic = (prng_u32() & 1) != 0;
		int pre = dynamic ? 0 : static_cast<int>(prng_u32() % 3);
		int init = dynamic ? 0 : static_cast<int>(prng_u32() % 3);
		int ret = static_cast<int>(prng_u32() % 300) - 100;
		int ep = static_cast<int>(prng_u32());
		int et = static_cast<int>(prng_u32());
		test_libc_start1_gcrt_case(st_libc_start1_gcrt, 1, argv, env, pre,
		    init, ret, ep, et, dynamic);
	}
}

void print_table()
{
	const Stat *all[] = {&st_process_irelocs, &st_finalizer,
	    &st_handle_static_init, &st_handle_argv, &st_handle_irelocs,
	    &st_libc_start1, &st_libc_start1_gcrt};
	std::printf("\n%-24s %10s %10s\n", "function", "cases", "failures");
	std::printf("%-24s %10s %10s\n", "--------", "-----", "--------");
	long total_cases = 0;
	long total_fails = 0;
	for (const Stat *s : all) {
		std::printf("%-24s %10ld %10ld\n", s->name, s->cases, s->fails);
		total_cases += s->cases;
		total_fails += s->fails;
	}
	std::printf("%-24s %10ld %10ld\n", "TOTAL", total_cases, total_fails);
	std::printf("link mode default: static (pbsd_dynamic_ptr == NULL)\n");
}

} // namespace

int main()
{
	run_process_irelocs_tests();
	run_finalizer_tests();
	run_handle_static_init_tests();
	run_handle_argv_tests();
	run_handle_irelocs_tests();
	run_libc_start1_tests();
	run_libc_start1_gcrt_tests();
	print_table();

	long fails = st_process_irelocs.fails + st_finalizer.fails +
	    st_handle_static_init.fails + st_handle_argv.fails +
	    st_handle_irelocs.fails + st_libc_start1.fails +
	    st_libc_start1_gcrt.fails;
	return fails == 0 ? 0 : 1;
}
