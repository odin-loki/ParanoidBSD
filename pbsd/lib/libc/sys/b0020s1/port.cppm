/*
 * Copyright 2026 The FreeBSD Foundation.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * This software was developed by Konstantin Belousov <kib@FreeBSD.org>
 * under sponsorship from the FreeBSD Foundation.
 */

/*
 * PBSD b0020s1: C++23 port of hbsd/src/lib/libc/sys/pdwait.c.
 *
 * <sys/procdesc.h> and "libc_private.h" are libc-private FreeBSD headers with
 * no PBSD module yet, so the declarations they supply are reproduced here.
 * The interposition table itself is defined by the oracle translation unit and
 * shared, so both sides of the differential test dispatch through the same
 * slots.
 */

module;

#include <sys/types.h>
#include <sys/resource.h>

export module pbsd.lib.libc.sys.b0020s1;

export struct __wrusage {
	struct rusage	wru_self;
	struct rusage	wru_children;
};

export struct __siginfo {
	int		si_signo;
	int		si_errno;
	int		si_code;
	int		si_pid;
	unsigned int	si_uid;
	int		si_status;
	void		*si_addr;
	long		si_value;
	long		si_band;
	int		__spare__[7];
};

extern "C" {

typedef int (*interpos_func_t)(void);

enum {
	INTERPOS_system,
	INTERPOS_tcdrain,
	INTERPOS_pdwait,
	INTERPOS_wait4,
	INTERPOS_MAX
};

extern interpos_func_t __pbsd_interposing[INTERPOS_MAX];

}

#define	__libc_interposing_slot(n)	(&__pbsd_interposing[(n)])

#define	INTERPOS_SYS(syscall, ...)					\
	(((__typeof__(syscall) *)*(__libc_interposing_slot(		\
	    INTERPOS_##syscall)))(__VA_ARGS__))

export namespace pbsd::lib_libc_sys::b0020s1 {

/* The original carries "#pragma weak pdwait". */
[[gnu::weak]] int
pdwait(int fd, int *status, int options, struct __wrusage *ru,
    struct __siginfo *infop)
{
	return (INTERPOS_SYS(pdwait, fd, status, options, ru, infop));
}

}
