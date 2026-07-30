/*
 * Copyright 2026 The FreeBSD Foundation.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * This software was developed by Konstantin Belousov <kib@FreeBSD.org>
 * under sponsorship from the FreeBSD Foundation.
 */

/*
 * Original file: lib/libc/sys/pdwait.c
 *
 * The original included <sys/types.h>, <sys/procdesc.h> and the private
 * header "libc_private.h".  Those headers are not available outside of the
 * FreeBSD/HardenedBSD libc build, so the declarations and macros that the
 * function body depends on are reproduced verbatim below.  Nothing inside a
 * function body has been changed.
 */

struct __wrusage;
struct __siginfo;

typedef int (*interpos_func_t)(void);

extern interpos_func_t __libc_interposing[];

#define	__libc_interposing_slot(i)	(&__libc_interposing[i])

#define	INTERPOS_pdwait		11

int __sys_pdwait(int fd, int *status, int options, struct __wrusage *ru,
    struct __siginfo *infop);

#define	INTERPOS_SYS(syscall, ...)					\
	(((__typeof(__sys_ ## syscall) *)*				\
	    (__libc_interposing_slot(INTERPOS_ ## syscall)))(__VA_ARGS__))

#pragma weak ref_pdwait
int
ref_pdwait(int fd, int *status, int options, struct __wrusage *ru,
    struct __siginfo *infop)
{
	return (INTERPOS_SYS(pdwait, fd, status, options, ru, infop));
}
