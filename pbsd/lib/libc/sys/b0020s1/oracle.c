/*
 * Copyright 2026 The FreeBSD Foundation.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * This software was developed by Konstantin Belousov <kib@FreeBSD.org>
 * under sponsorship from the FreeBSD Foundation.
 */

/*
 * PBSD b0020s1 oracle: hbsd/src/lib/libc/sys/pdwait.c with every function
 * renamed with a ref_ prefix.  Function bodies are unmodified.
 *
 * <sys/procdesc.h> and "libc_private.h" are not available to this test, so the
 * declarations they supply -- struct __wrusage, struct __siginfo, the libc
 * interposition table and INTERPOS_SYS -- are reproduced here.  The table is
 * defined in this file and shared with the port and the harness, so that both
 * sides of the differential test dispatch through the same slots.
 */

#include <sys/types.h>
#include <sys/resource.h>

struct __wrusage {
	struct rusage	wru_self;
	struct rusage	wru_children;
};

struct __siginfo {
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

typedef int (*interpos_func_t)(void);

enum {
	INTERPOS_system,
	INTERPOS_tcdrain,
	INTERPOS_pdwait,
	INTERPOS_wait4,
	INTERPOS_MAX
};

interpos_func_t __pbsd_interposing[INTERPOS_MAX];

#define	__libc_interposing_slot(n)	(&__pbsd_interposing[(n)])

#define	INTERPOS_SYS(syscall, ...)					\
	(((__typeof__(syscall) *)*(__libc_interposing_slot(		\
	    INTERPOS_##syscall)))(__VA_ARGS__))

int	pdwait(int fd, int *status, int options, struct __wrusage *ru,
	    struct __siginfo *infop);

#pragma weak pdwait
int
ref_pdwait(int fd, int *status, int options, struct __wrusage *ru,
    struct __siginfo *infop)
{
	return (INTERPOS_SYS(pdwait, fd, status, options, ru, infop));
}
