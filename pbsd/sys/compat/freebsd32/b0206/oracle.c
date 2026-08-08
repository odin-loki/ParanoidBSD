/*
 * PBSD batch b0206 -- reference ("oracle") build of the C sources:
 *
 *	sys/compat/freebsd32/freebsd32_abort2.c
 *	sys/compat/freebsd32/freebsd32_capability.c
 *	sys/compat/freebsd32/freebsd32_syscalls.c
 *
 * The sources are concatenated below.  Every function is renamed with a ref_
 * prefix; the function bodies are byte-for-byte the originals.  The kernel
 * environment the bodies talk to (types, error numbers, nitems(), MIN(),
 * malloc()/free(), the copyin/fueword/suword accessors and the kern_*
 * helpers) is declared here and supplied by harness.cpp, so this file is the
 * specification port.cppm is diffed against.
 *
 * Only #include lines that cannot exist outside the kernel were dropped;
 * nothing inside a function body was touched.
 */

#include <stddef.h>
#include <stdint.h>
#include <limits.h>

/* ------------------------------------------------------------------ */
/* modelled kernel environment					      */
/* ------------------------------------------------------------------ */

#define	CAPABILITIES	1		/* opt_capsicum.h */

#define	nitems(x)	(sizeof((x)) / sizeof((x)[0]))
#define	MIN(a, b)	(((a) < (b)) ? (a) : (b))

#define	EBADF		9
#define	EFAULT		14
#define	EINVAL		22
#define	ENOSYS		78

#define	M_WAITOK	0x0002
#define	MALLOC_DECLARE(type)	extern void *type
#define	malloc(size, type, flags)	kmock_malloc((size), (type), (flags))
#define	free(addr, type)		kmock_free((addr), (type))

#define	AUDIT_ARG_FD(fd)		kmock_audit_arg_fd(fd)
#define	FILEDESC_SLOCK(fdp)		kmock_filedesc_slock(fdp)
#define	FILEDESC_SUNLOCK(fdp)		kmock_filedesc_sunlock(fdp)

typedef unsigned int u_int;
typedef unsigned long u_long;
typedef long register_t;

struct file {
	int	f_dummy;
};

struct filedescent {
	struct file	*fde_file;
	u_long		*fde_ioctls;	/* fde_caps.fc_ioctls */
	int16_t		 fde_nioctls;	/* fde_caps.fc_nioctls */
};

struct filedesc {
	struct filedescent	*fd_ofiles;
	int			 fd_nfiles;
};

struct proc {
	struct filedesc	*p_fd;
};

struct thread {
	struct proc	*td_proc;
	register_t	 td_retval[2];
};

struct freebsd32_abort2_args {
	const char	*why;
	int		 nargs;
	uint32_t	*args;
};

struct freebsd32_cap_ioctls_limit_args {
	int		 fd;
	const uint32_t	*cmds;
	uint32_t	 ncmds;
};

struct freebsd32_cap_ioctls_get_args {
	int		 fd;
	uint32_t	*cmds;
	uint32_t	 maxcmds;
};

extern void	*kmock_malloc(size_t size, void *type, int flags);
extern void	 kmock_free(void *addr, void *type);
extern void	 kmock_audit_arg_fd(int fd);
extern void	 kmock_filedesc_slock(struct filedesc *fdp);
extern void	 kmock_filedesc_sunlock(struct filedesc *fdp);

extern int	 fueword32(const void *base, uint32_t *val);
extern int	 suword32(void *base, int word);
extern int	 copyin(const void *uaddr, void *kaddr, size_t len);
extern struct file *fget_noref(struct filedesc *fdp, int fd);
extern int	 kern_abort2(struct thread *td, const char *why, int nargs,
		     void **uargs);
extern int	 kern_cap_ioctls_limit(struct thread *td, int fd, u_long *cmds,
		     size_t ncmds);

/* ================================================================== */
/* sys/compat/freebsd32/freebsd32_abort2.c			      */
/* ================================================================== */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2005 Wojciech A. Koszek
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

int
ref_freebsd32_abort2(struct thread *td, struct freebsd32_abort2_args *uap)
{
	void *uargs[16];
	void *uargsp;
	uint32_t *uargsptr;
	uint32_t ptr;
	int i, nargs;

	nargs = uap->nargs;
	if (nargs < 0 || nargs > nitems(uargs))
		nargs = -1;
	uargsp = NULL;
	if (nargs > 0) {
		if (uap->args != NULL) {
			uargsptr = uap->args;
			for (i = 0; i < nargs; i++) {
				if (fueword32(uargsptr + i, &ptr) != 0) {
					nargs = -1;
					break;
				} else
					uargs[i] = (void *)(uintptr_t)ptr;
			}
			if (nargs > 0)
				uargsp = &uargs;
		} else
			nargs = -1;
	}
	return (kern_abort2(td, uap->why, nargs, uargsp));
}

/* ================================================================== */
/* sys/compat/freebsd32/freebsd32_capability.c			      */
/* ================================================================== */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2013 The FreeBSD Foundation
 *
 * This software was developed by Pawel Jakub Dawidek under sponsorship from
 * the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifdef CAPABILITIES

MALLOC_DECLARE(M_FILECAPS);

int
ref_freebsd32_cap_ioctls_limit(struct thread *td,
    struct freebsd32_cap_ioctls_limit_args *uap)
{
	u_long *cmds;
	uint32_t *cmds32;
	size_t ncmds;
	u_int i;
	int error;

	ncmds = uap->ncmds;

	if (ncmds > 256)	/* XXX: Is 256 sane? */
		return (EINVAL);

	if (ncmds == 0) {
		cmds = NULL;
	} else {
		cmds32 = malloc(sizeof(cmds32[0]) * ncmds, M_FILECAPS, M_WAITOK);
		error = copyin(uap->cmds, cmds32, sizeof(cmds32[0]) * ncmds);
		if (error != 0) {
			free(cmds32, M_FILECAPS);
			return (error);
		}
		cmds = malloc(sizeof(cmds[0]) * ncmds, M_FILECAPS, M_WAITOK);
		for (i = 0; i < ncmds; i++)
			cmds[i] = cmds32[i];
		free(cmds32, M_FILECAPS);
	}

	return (kern_cap_ioctls_limit(td, uap->fd, cmds, ncmds));
}

int
ref_freebsd32_cap_ioctls_get(struct thread *td,
    struct freebsd32_cap_ioctls_get_args *uap)
{
	struct filedesc *fdp;
	struct filedescent *fdep;
	uint32_t *cmds32;
	u_long *cmds;
	size_t maxcmds;
	int error, fd;
	u_int i;

	fd = uap->fd;
	cmds32 = uap->cmds;
	maxcmds = uap->maxcmds;

	AUDIT_ARG_FD(fd);

	fdp = td->td_proc->p_fd;
	FILEDESC_SLOCK(fdp);

	if (fget_noref(fdp, fd) == NULL) {
		error = EBADF;
		goto out;
	}

	/*
	 * If all ioctls are allowed (fde_nioctls == -1 && fde_ioctls == NULL)
	 * the only sane thing we can do is to not populate the given array and
	 * return CAP_IOCTLS_ALL (actually, INT_MAX).
	 */

	fdep = &fdp->fd_ofiles[fd];
	cmds = fdep->fde_ioctls;
	if (cmds32 != NULL && cmds != NULL) {
		for (i = 0; i < MIN(fdep->fde_nioctls, maxcmds); i++) {
			if (suword32(&cmds32[i], cmds[i]) != 0) {
				error = EFAULT;
				goto out;
			}
		}
	}
	if (fdep->fde_nioctls == -1)
		td->td_retval[0] = INT_MAX;
	else
		td->td_retval[0] = fdep->fde_nioctls;

	error = 0;
out:
	FILEDESC_SUNLOCK(fdp);
	return (error);
}

#else /* !CAPABILITIES */

int
ref_freebsd32_cap_ioctls_limit(struct thread *td,
    struct freebsd32_cap_ioctls_limit_args *uap)
{

	return (ENOSYS);
}

int
ref_freebsd32_cap_ioctls_get(struct thread *td,
    struct freebsd32_cap_ioctls_get_args *uap)
{

	return (ENOSYS);
}

#endif /* CAPABILITIES */

/* ================================================================== */
/* sys/compat/freebsd32/freebsd32_syscalls.c			      */
/* ================================================================== */

const size_t ref_freebsd32_syscallnames_count = 603;

