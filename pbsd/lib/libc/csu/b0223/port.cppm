/*-
 * SPDX-License-Identifier: BSD-1-Clause
 *
 * Copyright 2012 Konstantin Belousov <kib@FreeBSD.org>
 * Copyright (c) 2018, 2023 The FreeBSD Foundation
 *
 * Parts of this software was developed by Konstantin Belousov
 * <kib@FreeBSD.org> under sponsorship from the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

module;

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

export module pbsd.lib.libc.csu.b0223;

export namespace pbsd::lib_libc_csu::b0223 {

using Elf_Addr = std::uint64_t;
using Elf_Signed = std::int64_t;

struct Elf_Auxinfo {
	Elf_Addr a_type;
	union {
		Elf_Addr a_val;
	} a_un;
};

struct Elf_Rela {
	Elf_Addr r_offset;
	Elf_Addr r_info;
	Elf_Signed r_addend;
};

struct Elf_Rel {
	Elf_Addr r_offset;
	Elf_Addr r_info;
};

extern "C" {
extern char **environ;
extern const char *__progname;

extern int mock_iplt_n;
extern Elf_Rela __rela_iplt_start[64];
extern int mock_rel_iplt_n;
extern Elf_Rel __rel_iplt_start[64];
extern int mock_preinit_n;
extern void (*__preinit_array_start[64])(int, char **, char **);
extern int mock_init_n;
extern void (*__init_array_start[64])(int, char **, char **);
extern int mock_fini_n;
extern void (*__fini_array_start[64])(void);

extern int _DYNAMIC;

void crt1_handle_rela(const Elf_Rela *r);
void crt1_handle_rel(const Elf_Rel *r);
void ifunc_init(const Elf_Auxinfo *aux);
void _init_tls();
void _init();
void _fini();
void _mcleanup();
void monstartup(void *eprolp, void *etextp);
int atexit(void (*fn)(void));
void exit(int status);
}

namespace {

#if defined(CRT_IRELOC_RELA)
#define __rela_iplt_end \
	((const Elf_Rela (*)[1])(__rela_iplt_start + mock_iplt_n))
#elif defined(CRT_IRELOC_REL)
#define __rel_iplt_end \
	((const Elf_Rel (*)[1])(__rel_iplt_start + mock_rel_iplt_n))
#endif

#define __preinit_array_end \
	((void (*(*)[1])(int, char **, char **))(__preinit_array_start + \
	    mock_preinit_n))

#define __init_array_end \
	((void (*(*)[1])(int, char **, char **))(__init_array_start + \
	    mock_init_n))

#define __fini_array_end \
	((void (*(*)[1])(void))(__fini_array_start + mock_fini_n))

#if defined(CRT_IRELOC_RELA)

void
process_irelocs(void)
{
	const Elf_Rela *r;

	for (r = &__rela_iplt_start[0]; r < &__rela_iplt_end[0]; r++)
		crt1_handle_rela(r);
}
#elif defined(CRT_IRELOC_REL)

void
process_irelocs(void)
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

void
finalizer(void)
{
	void (*fn)(void);
	std::size_t array_size, n;

	array_size = __fini_array_end - __fini_array_start;
	for (n = array_size; n > 0; n--) {
		fn = __fini_array_start[n - 1];
		if ((std::uintptr_t)fn != 0 && (std::uintptr_t)fn != 1)
			(fn)();
	}
	_fini();
}
#endif

void
handle_static_init(int argc, char **argv, char **env)
{
#ifndef PIC
	void (*fn)(int, char **, char **);
	std::size_t array_size, n;

	if (&_DYNAMIC != NULL)
		return;

	atexit(finalizer);

	array_size = __preinit_array_end - __preinit_array_start;
	for (n = 0; n < array_size; n++) {
		fn = __preinit_array_start[n];
		if ((std::uintptr_t)fn != 0 && (std::uintptr_t)fn != 1)
			fn(argc, argv, env);
	}
	_init();
	array_size = __init_array_end - __init_array_start;
	for (n = 0; n < array_size; n++) {
		fn = __init_array_start[n];
		if ((std::uintptr_t)fn != 0 && (std::uintptr_t)fn != 1)
			fn(argc, argv, env);
	}
#endif
}

void
handle_argv(int argc, char *argv[], char **env)
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

void
handle_irelocs(char *env[])
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

} // namespace

void
libc_start1(int argc, char *argv[], char *env[], void (*cleanup)(void),
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
libc_start1_gcrt(int argc, char *argv[], char *env[],
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

void
test_process_irelocs(void)
{
	process_irelocs();
}

void
test_finalizer(void)
{
#ifndef PIC
	finalizer();
#endif
}

void
test_handle_static_init(int argc, char **argv, char **env)
{
	handle_static_init(argc, argv, env);
}

void
test_handle_argv(int argc, char *argv[], char **env)
{
	handle_argv(argc, argv, env);
}

void
test_handle_irelocs(char *env[])
{
	handle_irelocs(env);
}

} // namespace pbsd::lib_libc_csu::b0223
