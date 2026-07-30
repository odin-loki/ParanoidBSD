/*
 * Copyright 2026 The FreeBSD Foundation.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * This software was developed by Konstantin Belousov <kib@FreeBSD.org>
 * under sponsorship from the FreeBSD Foundation.
 */

export module pbsd.lib.libc.sys.b0020s1;

/*
 * Declarations the original translation unit obtained from <sys/types.h>,
 * <sys/procdesc.h> and the private header "libc_private.h".  The two opaque
 * kernel structures are exported so that importers name the same entities
 * that appear in pdwait()'s signature.
 */

export struct __wrusage;
export struct __siginfo;

extern "C" {

typedef int (*interpos_func_t)(void);

extern interpos_func_t __libc_interposing[];

int __sys_pdwait(int fd, int *status, int options, struct __wrusage *ru,
    struct __siginfo *infop);

}

#define	INTERPOS_pdwait		11

#define	__libc_interposing_slot(i)	(&__libc_interposing[i])

#define	INTERPOS_SYS(syscall, ...)					\
	((reinterpret_cast<decltype(&::__sys_ ## syscall)>(*		\
	    (__libc_interposing_slot(INTERPOS_ ## syscall))))(__VA_ARGS__))

export namespace pbsd::lib_libc_sys::b0020s1 {

int
pdwait(int fd, int *status, int options, struct __wrusage *ru,
    struct __siginfo *infop)
{
	return (INTERPOS_SYS(pdwait, fd, status, options, ru, infop));
}

}
