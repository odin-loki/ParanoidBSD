/*
 * oracle.c -- reference implementation for PBSD batch b0223.
 *
 * The original HardenedBSD C source
 *
 *	src/lib/libc/csu/libc_start1.c
 *
 * concatenated, with each ported function renamed with a ref_ prefix.
 * The function bodies are UNMODIFIED.  This file is the specification.
 *
 * Scaffolding reproduces the headers and runtime symbols the original
 * depends on so that the bodies compile verbatim.
 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#ifndef LONG_BIT
#define LONG_BIT	(sizeof(long) * 8)
#endif

#ifndef __hidden
#define __hidden
#endif

#ifndef __weak_symbol
#define __weak_symbol
#endif

typedef uint64_t Elf_Addr;
typedef int64_t Elf_Signed;

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

typedef struct {
	Elf_Addr r_offset;
	Elf_Addr r_info;
} Elf_Rel;

/* ------------------------------------------------------------------ */
/* libc_private.h globals                                             */
/* ------------------------------------------------------------------ */

char **environ;
const char *__progname;

/* ------------------------------------------------------------------ */
/* Mock runtime state (observable from harness.cpp)                   */
/* ------------------------------------------------------------------ */

int mock_iplt_n;
Elf_Rela __rela_iplt_start[64];
#define __rela_iplt_end \
	((const Elf_Rela *)(__rela_iplt_start + mock_iplt_n))

int mock_rel_iplt_n;
Elf_Rel __rel_iplt_start[64];
#define __rel_iplt_end \
	((const Elf_Rel *)(__rel_iplt_start + mock_rel_iplt_n))

int mock_preinit_n;
void (*__preinit_array_start[64])(int, char **, char **);
#define __preinit_array_end \
	((void (**)(int, char **, char **))(__preinit_array_start + \
	    mock_preinit_n))

int mock_init_n;
void (*__init_array_start[64])(int, char **, char **);
#define __init_array_end \
	((void (**)(int, char **, char **))(__init_array_start + mock_init_n))

int mock_fini_n;
void (*__fini_array_start[64])(void);
#define __fini_array_end \
	((void (**)(void))(__fini_array_start + mock_fini_n))

/*
 * Scrt1.o always defines _DYNAMIC for C++ links, which would force the
 * dynamic startup path.  Redirect the token while keeping bodies verbatim.
 */
static int pbsd_dynamic_storage;
int *pbsd_dynamic_ptr;

int mock_crt1_handle_rela_calls;
const Elf_Rela *mock_crt1_handle_rela_args[256];

int mock_crt1_handle_rel_calls;
const Elf_Rel *mock_crt1_handle_rel_args[256];

int mock_ifunc_init_calls;
const Elf_Auxinfo *mock_ifunc_init_args[64];

int mock_init_tls_calls;
int mock_init_calls;
int mock_fini_calls;
int mock_mcleanup_atexit;
int mock_monstartup_calls;
int *mock_monstartup_eprolp;
int *mock_monstartup_etextp;

int mock_atexit_count;
void (*mock_atexit_funcs[128])(void);

int mock_exit_called;
int mock_exit_status;
jmp_buf mock_exit_jmp;

int mock_preinit_calls;
int mock_init_array_calls;
int mock_fini_array_calls;
int mock_preinit_argc[64];
char **mock_preinit_argv[64];
char **mock_preinit_env[64];
int mock_init_array_argc[64];
char **mock_init_array_argv[64];
char **mock_init_array_env[64];

void
ref_reset_mocks(void)
{
	mock_iplt_n = 0;
	mock_rel_iplt_n = 0;
	mock_preinit_n = 0;
	mock_init_n = 0;
	mock_fini_n = 0;
	mock_crt1_handle_rela_calls = 0;
	mock_crt1_handle_rel_calls = 0;
	mock_ifunc_init_calls = 0;
	mock_init_tls_calls = 0;
	mock_init_calls = 0;
	mock_fini_calls = 0;
	mock_mcleanup_atexit = 0;
	mock_monstartup_calls = 0;
	mock_monstartup_eprolp = NULL;
	mock_monstartup_etextp = NULL;
	mock_atexit_count = 0;
	mock_exit_called = 0;
	mock_exit_status = 0;
	mock_preinit_calls = 0;
	mock_init_array_calls = 0;
	mock_fini_array_calls = 0;
	environ = NULL;
	__progname = NULL;
	pbsd_dynamic_ptr = NULL;
	memset(__rela_iplt_start, 0, sizeof(__rela_iplt_start));
	memset(__rel_iplt_start, 0, sizeof(__rel_iplt_start));
	memset(__preinit_array_start, 0, sizeof(__preinit_array_start));
	memset(__init_array_start, 0, sizeof(__init_array_start));
	memset(__fini_array_start, 0, sizeof(__fini_array_start));
}

void
ref_preinit_hook(int argc, char **argv, char **env)
{
	if (mock_preinit_calls < 64) {
		mock_preinit_argc[mock_preinit_calls] = argc;
		mock_preinit_argv[mock_preinit_calls] = argv;
		mock_preinit_env[mock_preinit_calls] = env;
	}
	mock_preinit_calls++;
}

void
ref_init_array_hook(int argc, char **argv, char **env)
{
	if (mock_init_array_calls < 64) {
		mock_init_array_argc[mock_init_array_calls] = argc;
		mock_init_array_argv[mock_init_array_calls] = argv;
		mock_init_array_env[mock_init_array_calls] = env;
	}
	mock_init_array_calls++;
}

void
ref_fini_array_hook(void)
{
	mock_fini_array_calls++;
}

void
crt1_handle_rela(const Elf_Rela *r)
{
	if (mock_crt1_handle_rela_calls < 256)
		mock_crt1_handle_rela_args[mock_crt1_handle_rela_calls] = r;
	mock_crt1_handle_rela_calls++;
}

void
crt1_handle_rel(const Elf_Rel *r)
{
	if (mock_crt1_handle_rel_calls < 256)
		mock_crt1_handle_rel_args[mock_crt1_handle_rel_calls] = r;
	mock_crt1_handle_rel_calls++;
}

void
ifunc_init(const Elf_Auxinfo *aux)
{
	if (mock_ifunc_init_calls < 64)
		mock_ifunc_init_args[mock_ifunc_init_calls] = aux;
	mock_ifunc_init_calls++;
}

void
_init_tls(void)
{
	mock_init_tls_calls++;
}

void
mock__init(void)
{
	mock_init_calls++;
}

void
mock__fini(void)
{
	mock_fini_calls++;
}

void
_mcleanup(void)
{
}

void
monstartup(void *eprolp, void *etextp)
{
	mock_monstartup_calls++;
	mock_monstartup_eprolp = (int *)eprolp;
	mock_monstartup_etextp = (int *)etextp;
}

int
atexit(void (*fn)(void))
{
	if (mock_atexit_count < 128)
		mock_atexit_funcs[mock_atexit_count] = fn;
	mock_atexit_count++;
	return 0;
}

void
exit(int status)
{
	mock_exit_called = 1;
	mock_exit_status = status;
	longjmp(mock_exit_jmp, status ? status : 1);
}

/* ------------------------------------------------------------------ */
/* Internal name mapping for unmodified cross-calls                     */
/* ------------------------------------------------------------------ */

#define handle_argv ref_handle_argv
#define handle_irelocs ref_handle_irelocs
#define handle_static_init ref_handle_static_init
#define process_irelocs ref_process_irelocs
#define finalizer ref_finalizer
#define _init mock__init
#define _fini mock__fini
#define _DYNAMIC (*pbsd_dynamic_ptr)

/* ------------------------------------------------------------------ */
/* libc_start1.c bodies (UNMODIFIED)                                  */
/* ------------------------------------------------------------------ */

#if defined(CRT_IRELOC_RELA)

static void
ref_process_irelocs(void)
{
	const Elf_Rela *r;

	for (r = &__rela_iplt_start[0]; r < &__rela_iplt_end[0]; r++)
		crt1_handle_rela(r);
}
#elif defined(CRT_IRELOC_REL)

static void
ref_process_irelocs(void)
{
	const Elf_Rel *r;

	for (r = &__rel_iplt_start[0]; r < &__rel_iplt_end[0]; r++)
		crt1_handle_rel(r);
}
#elif defined(CRT_IRELOC_SUPPRESS)
#else
#error "Define platform reloc type"
#endif

#ifndef PIC
static void
ref_finalizer(void)
{
	void (*fn)(void);
	size_t array_size, n;

	array_size = __fini_array_end - __fini_array_start;
	for (n = array_size; n > 0; n--) {
		fn = __fini_array_start[n - 1];
		if ((uintptr_t)fn != 0 && (uintptr_t)fn != 1)
			(fn)();
	}
	_fini();
}
#endif

static void
ref_handle_static_init(int argc, char **argv, char **env)
{
#ifndef PIC
	void (*fn)(int, char **, char **);
	size_t array_size, n;

	if (&_DYNAMIC != NULL)
		return;

	atexit(finalizer);

	array_size = __preinit_array_end - __preinit_array_start;
	for (n = 0; n < array_size; n++) {
		fn = __preinit_array_start[n];
		if ((uintptr_t)fn != 0 && (uintptr_t)fn != 1)
			fn(argc, argv, env);
	}
	_init();
	array_size = __init_array_end - __init_array_start;
	for (n = 0; n < array_size; n++) {
		fn = __init_array_start[n];
		if ((uintptr_t)fn != 0 && (uintptr_t)fn != 1)
			fn(argc, argv, env);
	}
#endif
}

static void
ref_handle_argv(int argc, char *argv[], char **env)
{
	const char *s;

	if (environ == NULL)
		environ = env;
	if (argc > 0 && argv[0] != NULL) {
		__progname = argv[0];
		for (s = __progname; *s != '\0'; s++) {
			if (*s == '/')
				__progname = s + 1;
		}
	}
}

static void
ref_handle_irelocs(char *env[])
{
#ifndef CRT_IRELOC_SUPPRESS
	const Elf_Auxinfo *aux;

	/* Find the auxiliary vector on the stack. */
	while (*env++ != 0)	/* Skip over environment, and NULL terminator */
		;
	aux = (const Elf_Auxinfo *)env;

	ifunc_init(aux);
	process_irelocs();
#else
	(void)env;
#endif
}

void
ref___libc_start1(int argc, char *argv[], char *env[], void (*cleanup)(void),
    int (*mainX)(int, char *[], char *[]))
{
	handle_argv(argc, argv, env);

	if (&_DYNAMIC != NULL) {
		atexit(cleanup);
	} else {
		handle_irelocs(env);
		_init_tls();
	}

	handle_static_init(argc, argv, env);

	/*
	 * C17 4.3 paragraph 3:
	 * The value of errno in the initial thread is zero at program
	 * startup.
	 */
	errno = 0;
	exit(mainX(argc, argv, env));
}

void
ref___libc_start1_gcrt(int argc, char *argv[], char *env[],
    void (*cleanup)(void), int (*mainX)(int, char *[], char *[]),
    int *eprolp, int *etextp)
{
	handle_argv(argc, argv, env);

	if (&_DYNAMIC != NULL) {
		atexit(cleanup);
	} else {
		handle_irelocs(env);
		_init_tls();
	}

	atexit(_mcleanup);
	monstartup(eprolp, etextp);

	handle_static_init(argc, argv, env);
	errno = 0;
	exit(mainX(argc, argv, env));
}

/* Test entry points for static helpers */
void
ref_test_process_irelocs(void)
{
	ref_process_irelocs();
}

void
ref_test_finalizer(void)
{
#ifndef PIC
	ref_finalizer();
#endif
}

void
ref_test_handle_static_init(int argc, char **argv, char **env)
{
	ref_handle_static_init(argc, argv, env);
}

void
ref_test_handle_argv(int argc, char *argv[], char **env)
{
	ref_handle_argv(argc, argv, env);
}

void
ref_test_handle_irelocs(char *env[])
{
	ref_handle_irelocs(env);
}

#undef handle_argv
#undef handle_irelocs
#undef handle_static_init
#undef process_irelocs
#undef finalizer
#undef _init
#undef _fini
#undef _DYNAMIC
