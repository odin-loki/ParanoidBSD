/*
 * oracle.c -- PBSD batch b0333 reference (specification) implementation.
 *
 * Original C sources, concatenated, with every function renamed with a
 * "ref_" prefix.  Function bodies are UNMODIFIED.
 *
 * Sources:
 *   hbsd/src/lib/libc/gen/errlst.c
 *   hbsd/src/lib/libc/gen/dlfcn.c
 *   hbsd/src/lib/libc/gen/exec.c
 *
 * Omitted (see skipped.txt):
 *   hbsd/src/lib/libc/gen/nlist.c
 */

#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>
#include <alloca.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <pthread.h>
#include <dlfcn.h>
#include <link.h>

#include <paths.h>
#include <sys/param.h>
#include <sys/stat.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef ELAST
#define ELAST 150
#endif

#define __unused	__attribute__((__unused__))
#define __DECONST(type, var)	((type)(uintptr_t)(const void *)(var))
#define __weak_reference(sym, alias)					\
	__asm__(".weak " #alias);					\
	__asm__(".equ "  #alias ", " #sym)
#define __strong_reference(sym, alias)					\
	__asm__(".globl " #alias);					\
	__asm__(".set " #alias ", " #sym)

/* from machine/atomic.h */
#define atomic_load_int(p) (*(volatile int *)(p))
#define atomic_store_int(p, v) (*(volatile int *)(p) = (v))

/* from rtld.h */
typedef struct {
	unsigned long	ti_module;
	unsigned long	ti_offset;
} tls_index;

#ifndef TLS_DTV_OFFSET
#define TLS_DTV_OFFSET 0
#endif

/* from reentrant.h / libc spinlock */
typedef pthread_mutex_t mutex_t;
#define MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER

static void
mutex_lock(mutex_t *m)
{
	(void)pthread_mutex_lock(m);
}

static void
mutex_unlock(mutex_t *m)
{
	(void)pthread_mutex_unlock(m);
}

/* harness-provided libc internals */
typedef struct {
	int	a_type;
	union {
		int	a_val;
		void	*a_ptr;
	} a_un;
} Elf_Auxinfo;

extern Elf_Auxinfo *__elf_aux_vector;
extern void __init_elf_aux_vector(void);
extern void *_once(pthread_once_t *, void (*)(void));
extern void *__tls_get_addr(tls_index *ti);

extern int _execve(const char *, char *const *, char *const *);
extern ssize_t _write(int, const void *, size_t);

extern char **environ;

/* ===================================================================== */
/* hbsd/src/lib/libc/gen/errlst.c					 */
/* ===================================================================== */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1982, 1985, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

const char __uprefix[] = "Unknown error";

const char *const sys_errlist[] = {
	"No error: 0",
	"Operation not permitted",
	"No such file or directory",
	"No such process",
	"Interrupted system call",
	"Input/output error",
	"Device not configured",
	"Argument list too long",
	"Exec format error",
	"Bad file descriptor",
	"No child processes",
	"Resource deadlock avoided",
	"Cannot allocate memory",
	"Permission denied",
	"Bad address",
	"Block device required",
	"Device busy",
	"File exists",
	"Cross-device link",
	"Operation not supported by device",
	"Not a directory",
	"Is a directory",
	"Invalid argument",
	"Too many open files in system",
	"Too many open files",
	"Inappropriate ioctl for device",
	"Text file busy",
	"File too large",
	"No space left on device",
	"Illegal seek",
	"Read-only file system",
	"Too many links",
	"Broken pipe",
	"Numerical argument out of domain",
	"Result too large",
	"Resource temporarily unavailable",
	"Operation now in progress",
	"Operation already in progress",
	"Socket operation on non-socket",
	"Destination address required",
	"Message too long",
	"Protocol wrong type for socket",
	"Protocol not available",
	"Protocol not supported",
	"Socket type not supported",
	"Operation not supported",
	"Protocol family not supported",
	"Address family not supported by protocol family",
	"Address already in use",
	"Can't assign requested address",
	"Network is down",
	"Network is unreachable",
	"Network dropped connection on reset",
	"Software caused connection abort",
	"Connection reset by peer",
	"No buffer space available",
	"Socket is already connected",
	"Socket is not connected",
	"Can't send after socket shutdown",
	"Too many references: can't splice",
	"Operation timed out",
	"Connection refused",
	"Too many levels of symbolic links",
	"File name too long",
	"Host is down",
	"No route to host",
	"Directory not empty",
	"Too many processes",
	"Too many users",
	"Disc quota exceeded",
	"Stale NFS file handle",
	"Too many levels of remote in path",
	"RPC struct is bad",
	"RPC version wrong",
	"RPC prog. not avail",
	"Program version wrong",
	"Bad procedure for program",
	"No locks available",
	"Function not implemented",
	"Inappropriate file type or format",
	"Authentication error",
	"Need authenticator",
	"Identifier removed",
	"No message of desired type",
	"Value too large to be stored in data type",
	"Operation canceled",
	"Illegal byte sequence",
	"Attribute not found",
	"Programming error",
	"Bad message",
	"Multihop attempted",
	"Link has been severed",
	"Protocol error",
	"Capabilities insufficient",
	"Not permitted in capability mode",
	"State not recoverable",
	"Previous owner died",
	"Integrity check failed",
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
	__uprefix,
};
const int sys_nerr = ELAST + 1;

/* ===================================================================== */
/* hbsd/src/lib/libc/gen/dlfcn.c					 */
/* ===================================================================== */

#if !defined(IN_LIBDL) || defined(PIC)

static const char sorry[] = "Service unavailable";

void ref__rtld_thread_init(void *);
void ref__rtld_atfork_pre(int *);
void ref__rtld_atfork_post(int *);

void
ref__rtld_error(const char *fmt __unused, ...)
{
}

int
ref_dladdr(const void *addr __unused, Dl_info *dlip __unused)
{

	ref__rtld_error(sorry);
	return (0);
}

int
ref_dlclose(void *handle __unused)
{

	ref__rtld_error(sorry);
	return (-1);
}

char *
ref_dlerror(void)
{

	return (__DECONST(char *, sorry));
}

void
ref_dllockinit(void *context,
    void *(*lock_create)(void *context) __unused,
    void (*rlock_acquire)(void *lock) __unused,
    void (*wlock_acquire)(void *lock) __unused,
    void (*lock_release)(void *lock) __unused,
    void (*lock_destroy)(void *lock) __unused,
    void (*context_destroy)(void *context) __unused)
{

	if (context_destroy != NULL)
		context_destroy(context);
}

void *
ref_dlopen(const char *name __unused, int mode __unused)
{

	ref__rtld_error(sorry);
	return (NULL);
}

void *
ref_dlsym(void * __restrict handle __unused, const char * __restrict name __unused)
{

	ref__rtld_error(sorry);
	return (NULL);
}

dlfunc_t
ref_dlfunc(void * __restrict handle __unused, const char * __restrict name __unused)
{

	ref__rtld_error(sorry);
	return (NULL);
}

void *
ref_dlvsym(void * __restrict handle __unused, const char * __restrict name __unused,
    const char * __restrict version __unused)
{

	ref__rtld_error(sorry);
	return (NULL);
}

int
ref_dlinfo(void * __restrict handle __unused, int request __unused,
    void * __restrict p __unused)
{

	ref__rtld_error(sorry);
	return (0);
}

void
ref__rtld_thread_init(void *li __unused)
{

	ref__rtld_error(sorry);
}

#ifndef IN_LIBDL
static pthread_once_t dl_phdr_info_once = PTHREAD_ONCE_INIT;
static struct dl_phdr_info phdr_info;
#ifndef PIC
static mutex_t dl_phdr_info_lock = MUTEX_INITIALIZER;
#endif

static void
ref_dl_init_phdr_info(void)
{
	Elf_Auxinfo *auxp;
	unsigned int i;

	for (auxp = __elf_aux_vector; auxp->a_type != AT_NULL; auxp++) {
		switch (auxp->a_type) {
		case AT_BASE:
			phdr_info.dlpi_addr = (Elf_Addr)auxp->a_un.a_ptr;
			break;
		case AT_EXECPATH:
			phdr_info.dlpi_name = (const char *)auxp->a_un.a_ptr;
			break;
		case AT_PHDR:
			phdr_info.dlpi_phdr =
			    (const Elf_Phdr *)auxp->a_un.a_ptr;
			break;
		case AT_PHNUM:
			phdr_info.dlpi_phnum = (Elf_Half)auxp->a_un.a_val;
			break;
		}
	}
	for (i = 0; i < phdr_info.dlpi_phnum; i++) {
		if (phdr_info.dlpi_phdr[i].p_type == PT_TLS) {
			phdr_info.dlpi_tls_modid = 1;
		}
	}
	phdr_info.dlpi_adds = 1;
}
#endif

int ref__dl_iterate_phdr_locked(int (*callback)(struct dl_phdr_info *,
    size_t, void *), void *data);
int
ref__dl_iterate_phdr_locked(
    int (*callback)(struct dl_phdr_info *, size_t, void *) __unused,
    void *data __unused)
{
#if defined IN_LIBDL
	return (0);
#elif defined PIC
	int (*r)(int (*)(struct dl_phdr_info *, size_t, void *), void *);

	r = dlsym(RTLD_DEFAULT, "dl_iterate_phdr");
	if (r == NULL)
		return (0);
	return (r(callback, data));
#else
	tls_index ti;
	int ret;

	__init_elf_aux_vector();
	if (__elf_aux_vector == NULL)
		return (1);
	_once(&dl_phdr_info_once, ref_dl_init_phdr_info);
	ti.ti_module = 1;
	ti.ti_offset = -TLS_DTV_OFFSET;
	phdr_info.dlpi_tls_data = __tls_get_addr(&ti);
	ret = callback(&phdr_info, sizeof(phdr_info), data);
	return (ret);
#endif
}

int
ref_dl_iterate_phdr(int (*callback)(struct dl_phdr_info *, size_t, void *) __unused,
    void *data __unused)
{
	int error;

#if !defined(IN_LIBDL) && !defined(PIC)
	mutex_lock(&dl_phdr_info_lock);
#endif
	error = ref__dl_iterate_phdr_locked(callback, data);
#if !defined(IN_LIBDL) && !defined(PIC)
	mutex_unlock(&dl_phdr_info_lock);
#endif
	return (error);
}

void *
ref_fdlopen(int fd __unused, int mode __unused)
{

	ref__rtld_error(sorry);
	return (NULL);
}

void
ref__rtld_atfork_pre(int *locks __unused)
{
}

void
ref__rtld_atfork_post(int *locks __unused)
{
}

#ifndef IN_LIBDL
struct _rtld_addr_phdr_cb_data {
	const void *addr;
	struct dl_phdr_info *dli;
};

static int
ref__rtld_addr_phdr_cb(struct dl_phdr_info *dli, size_t sz, void *arg)
{
	struct _rtld_addr_phdr_cb_data *rd;
	const Elf_Phdr *ph;
	unsigned i;

	rd = arg;
	for (i = 0; i < dli->dlpi_phnum; i++) {
		ph = &dli->dlpi_phdr[i];
		if (ph->p_type == PT_LOAD &&
		    dli->dlpi_addr + ph->p_vaddr <= (uintptr_t)rd->addr &&
		    (uintptr_t)rd->addr < dli->dlpi_addr + ph->p_vaddr +
		    ph->p_memsz) {
			memcpy(rd->dli, dli, sz);
			return (1);
		}
	}
	return (0);
}
#endif

int
ref__rtld_addr_phdr(const void *addr __unused,
    struct dl_phdr_info *phdr_info_a __unused)
{
#ifndef IN_LIBDL
	struct _rtld_addr_phdr_cb_data rd;

	rd.addr = addr;
	rd.dli = phdr_info_a;
	return (ref_dl_iterate_phdr(ref__rtld_addr_phdr_cb, &rd));
#else
	return (0);
#endif
}

int
ref__rtld_get_stack_prot(void)
{
#ifndef IN_LIBDL
	unsigned i;
	int r;
	static int ret;

	r = atomic_load_int(&ret);
	if (r != 0)
		return (r);

	_once(&dl_phdr_info_once, ref_dl_init_phdr_info);
	r = PROT_EXEC | PROT_READ | PROT_WRITE;
	for (i = 0; i < phdr_info.dlpi_phnum; i++) {
		if (phdr_info.dlpi_phdr[i].p_type != PT_GNU_STACK)
			continue;
		r = PROT_READ | PROT_WRITE;
		if ((phdr_info.dlpi_phdr[i].p_flags & PF_X) != 0)
			r |= PROT_EXEC;
		break;
	}
	atomic_store_int(&ret, r);
	return (r);
#else
	return (0);
#endif
}

Elf_Word
ref__rtld_get_pax_flags(void)
{

	return (0);
}

int
ref__rtld_is_dlopened(void *arg __unused)
{

	return (0);
}

const char *
ref_rtld_get_var(const char *name __unused)
{
	ref__rtld_error(sorry);
	return (NULL);
}

int
ref_rtld_set_var(const char *name __unused, const char *val __unused)
{
	ref__rtld_error(sorry);
	return (EINVAL);
}

#endif /* !defined(IN_LIBDL) || defined(PIC) */

/* ===================================================================== */
/* hbsd/src/lib/libc/gen/exec.c						 */
/* ===================================================================== */

static const char execvPe_err_preamble[] = "execvP: ";
static const char execvPe_err_trailer[] = ": path too long\n";

int ref_execvp(const char *, char * const *);
int ref___libc_execvpe(const char *, char * const *, char * const *);

int
ref_execl(const char *name, const char *arg, ...)
{
	va_list ap;
	const char **argv;
	int n;

	va_start(ap, arg);
	n = 1;
	while (va_arg(ap, char *) != NULL)
		n++;
	va_end(ap);
	argv = alloca((n + 1) * sizeof(*argv));
	if (argv == NULL) {
		errno = ENOMEM;
		return (-1);
	}
	va_start(ap, arg);
	n = 1;
	argv[0] = arg;
	while ((argv[n] = va_arg(ap, char *)) != NULL)
		n++;
	va_end(ap);
	return (_execve(name, __DECONST(char **, argv), environ));
}

int
ref_execle(const char *name, const char *arg, ...)
{
	va_list ap;
	const char **argv;
	char **envp;
	int n;

	va_start(ap, arg);
	n = 1;
	while (va_arg(ap, char *) != NULL)
		n++;
	va_end(ap);
	argv = alloca((n + 1) * sizeof(*argv));
	if (argv == NULL) {
		errno = ENOMEM;
		return (-1);
	}
	va_start(ap, arg);
	n = 1;
	argv[0] = arg;
	while ((argv[n] = va_arg(ap, char *)) != NULL)
		n++;
	envp = va_arg(ap, char **);
	va_end(ap);
	return (_execve(name, __DECONST(char **, argv), envp));
}

int
ref_execlp(const char *name, const char *arg, ...)
{
	va_list ap;
	const char **argv;
	int n;

	va_start(ap, arg);
	n = 1;
	while (va_arg(ap, char *) != NULL)
		n++;
	va_end(ap);
	argv = alloca((n + 1) * sizeof(*argv));
	if (argv == NULL) {
		errno = ENOMEM;
		return (-1);
	}
	va_start(ap, arg);
	n = 1;
	argv[0] = arg;
	while ((argv[n] = va_arg(ap, char *)) != NULL)
		n++;
	va_end(ap);
	return (ref_execvp(name, __DECONST(char **, argv)));
}

int
ref_execv(const char *name, char * const *argv)
{
	(void)_execve(name, argv, environ);
	return (-1);
}

int
ref_execvp(const char *name, char * const *argv)
{
	return (ref___libc_execvpe(name, argv, environ));
}

static int
ref_execvPe_prog(const char *path, char * const *argv, char * const *envp)
{
	struct stat sb;
	const char **memp;
	size_t cnt;
	int save_errno;

	(void)_execve(path, argv, envp);
	switch (errno) {
	case ELOOP:
	case ENAMETOOLONG:
	case ENOENT:
	case ENOTDIR:
		break;
	case ENOEXEC:
		for (cnt = 0; argv[cnt] != NULL; ++cnt)
			;

		memp = alloca(MAX(3, cnt + 2) * sizeof(char *));
		assert(memp != NULL);
		if (cnt > 0) {
			memp[0] = argv[0];
			memp[1] = path;
			memcpy(&memp[2], &argv[1], cnt * sizeof(char *));
		} else {
			memp[0] = "sh";
			memp[1] = path;
			memp[2] = NULL;
		}

		(void)_execve(_PATH_BSHELL, __DECONST(char **, memp), envp);
		return (-1);
	case ENOMEM:
	case E2BIG:
		return (-1);
	case ETXTBSY:
		return (-1);
	default:
		save_errno = errno;
		if (stat(path, &sb) == -1) {
			if (save_errno == EACCES)
				errno = ENOENT;
			break;
		}

		errno = save_errno;

		if (errno == EACCES)
			break;

		return (-1);
	}

	return (0);
}

static int
ref_execvPe(const char *name, const char *path, char * const *argv,
    char * const *envp)
{
	char buf[MAXPATHLEN];
	size_t ln, lp;
	const char *np, *op, *p;
	bool eacces;

	eacces = false;

	if (strchr(name, '/') != NULL) {
		(void)ref_execvPe_prog(name, argv, envp);
		return (-1);
	}

	if (*name == '\0') {
		errno = ENOENT;
		return (-1);
	}

	op = path;
	ln = strlen(name);
	while (op != NULL) {
		np = strchrnul(op, ':');

		if (np == op) {
			p = ".";
			lp = 1;
		} else {
			p = op;
			lp = np - op;
		}

		if (*np == '\0')
			op = NULL;
		else
			op = np + 1;

		if (lp + ln + 2 > sizeof(buf)) {
			(void)_write(STDERR_FILENO, execvPe_err_preamble,
			    sizeof(execvPe_err_preamble) - 1);
			(void)_write(STDERR_FILENO, p, lp);
			(void)_write(STDERR_FILENO, execvPe_err_trailer,
			    sizeof(execvPe_err_trailer) - 1);

			continue;
		}

		memcpy(&buf[0], p, lp);
		buf[lp] = '/';
		memcpy(&buf[lp + 1], name, ln);
		buf[lp + ln + 1] = '\0';

		if (ref_execvPe_prog(buf, argv, envp) == -1)
			return (-1);
		if (errno == EACCES)
			eacces = true;
	}

	if (eacces)
		errno = EACCES;
	else
		errno = ENOENT;

	return (-1);
}

int
ref_execvP(const char *name, const char *path, char * const argv[])
{
	return ref_execvPe(name, path, argv, environ);
}

int
ref___libc_execvpe(const char *name, char * const argv[], char * const envp[])
{
	const char *path;

	if ((path = getenv("PATH")) == NULL)
		path = _PATH_DEFPATH;

	return (ref_execvPe(name, path, argv, envp));
}

__weak_reference(ref___libc_execvpe, execvpe);
