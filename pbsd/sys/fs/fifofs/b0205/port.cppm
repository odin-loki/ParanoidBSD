/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993, 1995
 *	The Regents of the University of California.
 * Copyright (c) 2005 Robert N. M. Watson
 * Copyright (c) 2012 Giovanni Trematerra
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

/*
 * PBSD batch b0205: sys/fs/fifofs/fifo_vnops.c ported to C++23.
 *
 * The port is behaviourally identical to the C original: the same
 * expressions in the same order, the same integer types (long reader and
 * writer counts, u_int generation counters, int pipe_wgen), the same
 * long -> int narrowing in the f_pipegen computation, and the same calls to
 * the kernel primitives in the same sequence.  Nothing is tidied up.
 *
 * The kernel primitives are supplied by the pbsd_* hooks below, which are the
 * hooks the reference oracle uses as well.
 */

export module pbsd.sys.fs.fifofs.b0205;

extern "C" {
int	pbsd_kprintf(const char *fmt, ...);
void	*pbsd_kmalloc(unsigned long size, void *mtp, int flags);
void	pbsd_kfree(void *addr, void *mtp);
int	pbsd_pipe_named_ctor(void **pp, void *td);
void	pbsd_pipe_dtor(void *cpipe);
void	pbsd_pipe_lock(void *cpipe);
void	pbsd_pipe_unlock(void *cpipe);
void	pbsd_wakeup(void *chan);
void	pbsd_pipeselwakeup(void *cpipe);
void	pbsd_assert_vop_elocked(void *vp, const char *str);
void	pbsd_vop_unlock(void *vp);
void	pbsd_vn_lock(void *vp, int flags);
int	pbsd_sigdeferstop(int mode);
void	pbsd_sigallowstop(int prev);
int	pbsd_msleep(void *chan, void *mtx, int pri, const char *wmesg,
	    int timo);
void	pbsd_finit(void *fp, unsigned int flag, int type, void *data,
	    const void *ops);
int	pbsd_vop_stdadvlock(void *ap);

extern int	pbsd_M_VNODE_obj;
extern int	pbsd_pipeops_obj;
extern int	pbsd_badfileops_obj;
}

#define NULL			nullptr

#ifndef LONG_BIT
#define LONG_BIT		(8 * (int)sizeof(long))
#endif

/* Error numbers (sys/errno.h). */
#define EINVAL			22
#define ENXIO			6
#define EINTR			4

/* Kernel file flags (sys/fcntl.h). */
#define FREAD			0x0001
#define FWRITE			0x0002
#define O_NONBLOCK		0x0004
#define O_EXEC			0x00040000
#define FEXEC			O_EXEC
#define F_FLOCK			0x020

/* struct pipe pipe_state bits (sys/pipe.h). */
#define PIPE_ASYNC		0x004
#define PIPE_WANTR		0x008
#define PIPE_WANTW		0x010
#define PIPE_WANT		0x020
#define PIPE_SEL		0x040
#define PIPE_EOF		0x080
#define PIPE_LOCKFL		0x100
#define PIPE_LWANT		0x200
#define PIPE_DIRECTW		0x400
#define PIPE_DIRECTOK		0x800

/* malloc(9) flags (sys/malloc.h). */
#define M_NOWAIT		0x0001
#define M_WAITOK		0x0002
#define M_ZERO			0x0100

/* lockmgr(9) flags (sys/lockmgr.h). */
#define LK_RETRY		0x000400
#define LK_EXCLUSIVE		0x080000

/* msleep(9) priority modifiers (sys/param.h, sys/priority.h). */
#define PSOCK			24
#define PCATCH			0x100
#define PDROP			0x200

/* sigdeferstop(9) modes (sys/signalvar.h). */
#define SIGDEFERSTOP_OFF	0

/* struct file descriptor types (sys/file.h). */
#define DTYPE_FIFO		8

#define M_VNODE			(&pbsd_M_VNODE_obj)
#define pipeops			pbsd_pipeops_obj
#define badfileops		pbsd_badfileops_obj

#define printf			pbsd_kprintf
#define kmalloc(sz, mt, fl)	pbsd_kmalloc((unsigned long)(sz), (mt), (fl))
#define free(p, mt)		pbsd_kfree((void *)(p), (mt))
#define pipe_named_ctor(pp, td)	pbsd_pipe_named_ctor((void **)(pp), (void *)(td))
#define pipe_dtor(p)		pbsd_pipe_dtor((void *)(p))
#define PIPE_LOCK(p)		pbsd_pipe_lock((void *)(p))
#define PIPE_UNLOCK(p)		pbsd_pipe_unlock((void *)(p))
#define PIPE_MTX(p)		((void *)(p))
#define wakeup(c)		pbsd_wakeup((void *)(c))
#define pipeselwakeup(p)	pbsd_pipeselwakeup((void *)(p))
#define ASSERT_VOP_ELOCKED(vp, s) pbsd_assert_vop_elocked((void *)(vp), (s))
#define VOP_UNLOCK(vp)		pbsd_vop_unlock((void *)(vp))
#define vn_lock(vp, f)		pbsd_vn_lock((void *)(vp), (f))
#define sigdeferstop(m)		pbsd_sigdeferstop(m)
#define sigallowstop(p)		pbsd_sigallowstop(p)
#define msleep(c, m, p, w, t)	pbsd_msleep((void *)(c), (void *)(m), (p), (w), (t))
#define finit(fp, fl, ty, d, o)	pbsd_finit((void *)(fp), (fl), (ty), (void *)(d), \
				    (const void *)(o))
#define vop_stdadvlock(ap)	pbsd_vop_stdadvlock((void *)(ap))

/* KASSERT is a no-op in a kernel built without INVARIANTS. */
#define KASSERT(exp, msg)	((void)0)

export namespace pbsd::sys_fs_fifofs::b0205 {

using u_int = unsigned int;

struct fifoinfo;

struct pipe {
	u_int	pipe_state;
	int	pipe_wgen;
};

struct file {
	u_int		f_flag;
	int		f_pipegen;
	const void	*f_ops;
};

struct vnode {
	struct fifoinfo	*v_fifoinfo;
};

struct thread {
	int	td_tid;
};

struct flock {
	long	l_start;
	long	l_len;
	int	l_pid;
	short	l_type;
	short	l_whence;
};

struct vop_open_args {
	struct vnode	*a_vp;
	int		a_mode;
	struct thread	*a_td;
	struct file	*a_fp;
};

struct vop_close_args {
	struct vnode	*a_vp;
	int		a_fflag;
	struct thread	*a_td;
};

struct vop_print_args {
	struct vnode	*a_vp;
};

struct vop_advlock_args {
	struct vnode	*a_vp;
	void		*a_id;
	int		a_op;
	struct flock	*a_fl;
	int		a_flags;
};

/*
 * This structure is associated with the FIFO vnode and stores
 * the state associated with the FIFO.
 * Notes about locking:
 *   - fi_pipe is invariant since init time.
 *   - fi_readers and fi_writers are protected by the vnode lock.
 */
struct fifoinfo {
	struct pipe *fi_pipe;
	long	fi_readers;
	long	fi_writers;
	u_int	fi_rgen;
	u_int	fi_wgen;
};

/*
 * Dispose of fifo resources.
 */
void
fifo_cleanup(struct vnode *vp)
{
	struct fifoinfo *fip;

	ASSERT_VOP_ELOCKED(vp, "fifo_cleanup");
	fip = vp->v_fifoinfo;
	if (fip->fi_readers == 0 && fip->fi_writers == 0) {
		vp->v_fifoinfo = NULL;
		pipe_dtor(fip->fi_pipe);
		free(fip, M_VNODE);
	}
}

/*
 * Open called to set up a new instance of a fifo or
 * to find an active instance of a fifo.
 */
/* ARGSUSED */
int
fifo_open(struct vop_open_args *ap)
{
	struct vnode *vp;
	struct file *fp;
	struct thread *td;
	struct fifoinfo *fip;
	struct pipe *fpipe;
	u_int gen;
	int error, stops_deferred;

	vp = ap->a_vp;
	fp = ap->a_fp;
	td = ap->a_td;
	ASSERT_VOP_ELOCKED(vp, "fifo_open");
	if (fp == NULL || (ap->a_mode & FEXEC) != 0)
		return (EINVAL);
	if ((fip = vp->v_fifoinfo) == NULL) {
		error = pipe_named_ctor(&fpipe, td);
		if (error != 0)
			return (error);
		fip = (struct fifoinfo *)kmalloc(sizeof(*fip), M_VNODE,
		    M_WAITOK | M_ZERO);
		fip->fi_pipe = fpipe;
		fpipe->pipe_wgen = 0;
		KASSERT(vp->v_fifoinfo == NULL, ("fifo_open: v_fifoinfo race"));
		vp->v_fifoinfo = fip;
	}
	fpipe = fip->fi_pipe;
	KASSERT(fpipe != NULL, ("fifo_open: pipe is NULL"));

	/*
	 * Use the pipe mutex here, in addition to the vnode lock,
	 * in order to allow vnode lock dropping before msleep() calls
	 * and still avoiding missed wakeups.
	 */
	PIPE_LOCK(fpipe);
	if (ap->a_mode & FREAD) {
		fip->fi_readers++;
		fip->fi_rgen++;
		if (fip->fi_readers == 1) {
			fpipe->pipe_state &= ~PIPE_EOF;
			if (fip->fi_writers > 0) {
				wakeup(&fip->fi_writers);
				pipeselwakeup(fpipe);
			}
		}
		fp->f_pipegen = fpipe->pipe_wgen - fip->fi_writers;
	}
	if (ap->a_mode & FWRITE) {
		if ((ap->a_mode & O_NONBLOCK) && fip->fi_readers == 0) {
			PIPE_UNLOCK(fpipe);
			if (fip->fi_writers == 0)
				fifo_cleanup(vp);
			return (ENXIO);
		}
		fip->fi_writers++;
		fip->fi_wgen++;
		if (fip->fi_writers == 1) {
			fpipe->pipe_state &= ~PIPE_EOF;
			if (fip->fi_readers > 0) {
				wakeup(&fip->fi_readers);
				pipeselwakeup(fpipe);
			}
		}
	}
	if ((ap->a_mode & O_NONBLOCK) == 0) {
		if ((ap->a_mode & FREAD) && fip->fi_writers == 0) {
			gen = fip->fi_wgen;
			VOP_UNLOCK(vp);
			stops_deferred = sigdeferstop(SIGDEFERSTOP_OFF);
			error = msleep(&fip->fi_readers, PIPE_MTX(fpipe),
			    PDROP | PCATCH | PSOCK, "fifoor", 0);
			sigallowstop(stops_deferred);
			vn_lock(vp, LK_EXCLUSIVE | LK_RETRY);
			if (error != 0 && gen == fip->fi_wgen) {
				fip->fi_readers--;
				if (fip->fi_readers == 0) {
					PIPE_LOCK(fpipe);
					fpipe->pipe_state |= PIPE_EOF;
					if (fpipe->pipe_state & PIPE_WANTW)
						wakeup(fpipe);
					pipeselwakeup(fpipe);
					PIPE_UNLOCK(fpipe);
					fifo_cleanup(vp);
				}
				return (error);
			}
			PIPE_LOCK(fpipe);
			/*
			 * We must have got woken up because we had a writer.
			 * That (and not still having one) is the condition
			 * that we must wait for.
			 */
		}
		if ((ap->a_mode & FWRITE) && fip->fi_readers == 0) {
			gen = fip->fi_rgen;
			VOP_UNLOCK(vp);
			stops_deferred = sigdeferstop(SIGDEFERSTOP_OFF);
			error = msleep(&fip->fi_writers, PIPE_MTX(fpipe),
			    PDROP | PCATCH | PSOCK, "fifoow", 0);
			sigallowstop(stops_deferred);
			vn_lock(vp, LK_EXCLUSIVE | LK_RETRY);
			if (error != 0 && gen == fip->fi_rgen) {
				fip->fi_writers--;
				if (fip->fi_writers == 0) {
					PIPE_LOCK(fpipe);
					fpipe->pipe_state |= PIPE_EOF;
					if (fpipe->pipe_state & PIPE_WANTR)
						wakeup(fpipe);
					fpipe->pipe_wgen++;
					pipeselwakeup(fpipe);
					PIPE_UNLOCK(fpipe);
					fifo_cleanup(vp);
				}
				return (error);
			}
			/*
			 * We must have got woken up because we had
			 * a reader.  That (and not still having one)
			 * is the condition that we must wait for.
			 */
			PIPE_LOCK(fpipe);
		}
	}
	PIPE_UNLOCK(fpipe);
	KASSERT(fp != NULL, ("can't fifo/vnode bypass"));
	KASSERT(fp->f_ops == &badfileops, ("not badfileops in fifo_open"));
	finit(fp, fp->f_flag, DTYPE_FIFO, fpipe, &pipeops);
	return (0);
}

/*
 * Device close routine
 */
/* ARGSUSED */
int
fifo_close(struct vop_close_args *ap)
{
	struct vnode *vp;
	struct fifoinfo *fip;
	struct pipe *cpipe;

	vp = ap->a_vp;
	ASSERT_VOP_ELOCKED(vp, "fifo_close");
	fip = vp->v_fifoinfo;

	/*
	 * During open, it is possible that the fifo vnode is relocked
	 * after the vnode is instantiated but before VOP_OPEN() is
	 * done.  For instance, vn_open_vnode() might need to upgrade
	 * vnode lock, or ffs_vput_pair() needs to unlock vp to sync
	 * dvp.  In this case, reclaim can observe us with v_fifoinfo
	 * equal to NULL.
	 */
	if (fip == NULL)
		return (0);

	cpipe = fip->fi_pipe;
	if (ap->a_fflag & FREAD) {
		fip->fi_readers--;
		if (fip->fi_readers == 0) {
			PIPE_LOCK(cpipe);
			cpipe->pipe_state |= PIPE_EOF;
			if ((cpipe->pipe_state & PIPE_WANTW)) {
				cpipe->pipe_state &= ~PIPE_WANTW;
				wakeup(cpipe);
			}
			pipeselwakeup(cpipe);
			PIPE_UNLOCK(cpipe);
		}
	}
	if (ap->a_fflag & FWRITE) {
		fip->fi_writers--;
		if (fip->fi_writers == 0) {
			PIPE_LOCK(cpipe);
			cpipe->pipe_state |= PIPE_EOF;
			if ((cpipe->pipe_state & PIPE_WANTR)) {
				cpipe->pipe_state &= ~PIPE_WANTR;
				wakeup(cpipe);
			}
			cpipe->pipe_wgen++;
			pipeselwakeup(cpipe);
			PIPE_UNLOCK(cpipe);
		}
	}
	fifo_cleanup(vp);
	return (0);
}

/*
 * Print out internal contents of a fifo vnode.
 */
int
fifo_printinfo(struct vnode *vp)
{
	struct fifoinfo *fip = vp->v_fifoinfo;

	if (fip == NULL){
		printf(", NULL v_fifoinfo");
		return (0);
	}
	printf(", fifo with %ld readers and %ld writers",
		fip->fi_readers, fip->fi_writers);
	return (0);
}

/*
 * Print out the contents of a fifo vnode.
 */
int
fifo_print(struct vop_print_args *ap)
{
	printf("    ");
	fifo_printinfo(ap->a_vp);
	printf("\n");
	return (0);
}

/*
 * Fifo advisory byte-level locks.
 */
/* ARGSUSED */
int
fifo_advlock(struct vop_advlock_args *ap)
{

	if ((ap->a_flags & F_FLOCK) == 0)
		return (EINVAL);
	return (vop_stdadvlock(ap));
}

} /* namespace pbsd::sys_fs_fifofs::b0205 */
