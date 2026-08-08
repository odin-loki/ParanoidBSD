/*
 * oracle.c -- reference implementation for PBSD batch b0205.
 *
 * The original HardenedBSD kernel source is included below with every
 * function renamed with a "ref_" prefix.  Function bodies are UNMODIFIED.
 * Supporting types, macros, and shims are added only where the original file
 * obtained them from kernel headers.
 */

#define _KERNEL
#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#define EINVAL		22
#define ENXIO		6
#define ENOMEM		12

#define FREAD		0x00000001
#define FWRITE		0x00000002
#define FEXEC		0x00000020
#define O_NONBLOCK	0x00000004
#define F_FLOCK		0x020

#define DTYPE_FIFO	4

#define PIPE_WANTR	0x008
#define PIPE_WANTW	0x010
#define PIPE_EOF	0x080

#define M_WAITOK	0x0002
#define M_ZERO		0x0100

#define PDROP		0x200
#define PCATCH		0x100
#define PSOCK		(PRI_MIN_KERN + 6)
#define PRI_MIN_KERN	0

#define SIGDEFERSTOP_OFF	1

#define LK_EXCLUSIVE	0x0001
#define LK_RETRY	0x0004

typedef unsigned int u_int;

struct malloc_type {
	const char *ks_shortdesc;
};

struct mtx {
	pthread_mutex_t lock;
};

struct pipepair {
	struct mtx pp_mtx;
};

struct pipe {
	struct pipepair *pipe_pair;
	u_int pipe_state;
	int pipe_wgen;
};

struct fileops {
	int dummy;
};

struct file {
	u_int f_flag;
	const struct fileops *f_ops;
	int f_pipegen;
	short f_type;
	void *f_data;
};

struct thread {
	int dummy;
};

struct fifoinfo {
	struct pipe *fi_pipe;
	long fi_readers;
	long fi_writers;
	u_int fi_rgen;
	u_int fi_wgen;
};

struct vnode {
	struct fifoinfo *v_fifoinfo;
};

struct vop_open_args {
	struct vnode *a_vp;
	int a_mode;
	struct thread *a_td;
	struct file *a_fp;
};

struct vop_close_args {
	struct vnode *a_vp;
	int a_fflag;
};

struct vop_print_args {
	struct vnode *a_vp;
};

struct vop_advlock_args {
	struct vnode *a_vp;
	void *a_id;
	int a_op;
	void *a_fl;
	int a_flags;
};

#define PIPE_MTX(pipe)		(&(pipe)->pipe_pair->pp_mtx)
#define PIPE_LOCK(pipe)		pthread_mutex_lock(&PIPE_MTX(pipe)->lock)
#define PIPE_UNLOCK(pipe)	pthread_mutex_unlock(&PIPE_MTX(pipe)->lock)

#define ASSERT_VOP_ELOCKED(vp, msg)	do { (void)(vp); (void)(msg); } while (0)
#define KASSERT(cond, msg)		do { (void)(cond); (void)(msg); } while (0)
#define VOP_UNLOCK(vp)		do { (void)(vp); } while (0)
#define vn_lock(vp, flags)		do { (void)(vp); (void)(flags); } while (0)

static struct malloc_type M_VNODE[1];

const struct fileops badfileops = { 0 };
const struct fileops pipeops = { 1 };

static void *(*stdlib_malloc_fn)(size_t);
static void (*stdlib_free_fn)(void *);
static int stdlib_hooks_ready;

static void
oracle_init_stdlib_hooks(void)
{
	if (stdlib_hooks_ready)
		return;
	stdlib_malloc_fn = malloc;
	stdlib_free_fn = free;
	stdlib_hooks_ready = 1;
}

int ref_pipe_named_ctor_ret;
int ref_msleep_ret;
int ref_vop_stdadvlock_ret;

int ref_msleep_calls;
void *ref_msleep_chan;
int ref_msleep_pri;

int ref_wakeup_calls;
void *ref_wakeup_log[64];

int ref_pipeselwakeup_calls;
int ref_pipe_dtor_calls;

int ref_finit_calls;
u_int ref_finit_flag;
short ref_finit_type;
void *ref_finit_data;
const struct fileops *ref_finit_ops;

int ref_vop_stdadvlock_calls;
int ref_vop_stdadvlock_flags;

char ref_printf_buf[1024];
size_t ref_printf_len;

void
ref_stub_reset(void)
{
	oracle_init_stdlib_hooks();

	ref_pipe_named_ctor_ret = 0;
	ref_msleep_ret = 0;
	ref_vop_stdadvlock_ret = 0;

	ref_msleep_calls = 0;
	ref_msleep_chan = NULL;
	ref_msleep_pri = 0;

	ref_wakeup_calls = 0;
	memset(ref_wakeup_log, 0, sizeof(ref_wakeup_log));

	ref_pipeselwakeup_calls = 0;
	ref_pipe_dtor_calls = 0;

	ref_finit_calls = 0;
	ref_finit_flag = 0;
	ref_finit_type = 0;
	ref_finit_data = NULL;
	ref_finit_ops = NULL;

	ref_vop_stdadvlock_calls = 0;
	ref_vop_stdadvlock_flags = 0;

	ref_printf_buf[0] = '\0';
	ref_printf_len = 0;
}

int
printf(const char *fmt, ...)
{
	va_list ap;
	int n;

	va_start(ap, fmt);
	n = vsnprintf(ref_printf_buf + ref_printf_len,
	    sizeof(ref_printf_buf) - ref_printf_len, fmt, ap);
	va_end(ap);
	if (n > 0) {
		size_t add = (size_t)n;
		if (ref_printf_len + add >= sizeof(ref_printf_buf))
			add = sizeof(ref_printf_buf) - ref_printf_len - 1;
		ref_printf_len += add;
	}
	return (n);
}

int
pipe_named_ctor(struct pipe **ppipe, struct thread *td)
{
	struct pipepair *pp;
	struct pipe *p;

	(void)td;
	oracle_init_stdlib_hooks();
	if (ref_pipe_named_ctor_ret != 0)
		return (ref_pipe_named_ctor_ret);
	pp = stdlib_malloc_fn(sizeof(*pp));
	p = stdlib_malloc_fn(sizeof(*p));
	if (pp == NULL || p == NULL) {
		stdlib_free_fn(pp);
		stdlib_free_fn(p);
		return (ENOMEM);
	}
	memset(pp, 0, sizeof(*pp));
	memset(p, 0, sizeof(*p));
	pthread_mutex_init(&pp->pp_mtx.lock, NULL);
	p->pipe_pair = pp;
	*ppipe = p;
	return (0);
}

void
pipe_dtor(struct pipe *dpipe)
{

	ref_pipe_dtor_calls++;
	if (dpipe != NULL && dpipe->pipe_pair != NULL) {
		pthread_mutex_destroy(&dpipe->pipe_pair->pp_mtx.lock);
		stdlib_free_fn(dpipe->pipe_pair);
	}
	stdlib_free_fn(dpipe);
}

static void *
kern_malloc(unsigned long size, struct malloc_type *type, int flags)
{
	void *p;

	(void)type;
	oracle_init_stdlib_hooks();
	p = stdlib_malloc_fn((size_t)size);
	if (p != NULL && (flags & M_ZERO) != 0)
		memset(p, 0, (size_t)size);
	return (p);
}

static void
kern_free(void *addr, struct malloc_type *type)
{

	(void)type;
	if (addr != NULL)
		stdlib_free_fn(addr);
}

#define malloc(size, type, flags)	kern_malloc((size), (type), (flags))
#define free(addr, type)		kern_free((addr), (type))

int
msleep(void *chan, struct mtx *mtx, int pri, const char *wmesg, int timo)
{

	(void)wmesg;
	(void)timo;
	ref_msleep_calls++;
	ref_msleep_chan = chan;
	ref_msleep_pri = pri;
	if (mtx != NULL && (pri & PDROP) != 0)
		pthread_mutex_unlock(&mtx->lock);
	return (ref_msleep_ret);
}

void
wakeup(void *chan)
{

	if (ref_wakeup_calls < (int)(sizeof(ref_wakeup_log) / sizeof(ref_wakeup_log[0])))
		ref_wakeup_log[ref_wakeup_calls] = chan;
	ref_wakeup_calls++;
}

void
pipeselwakeup(struct pipe *cpipe)
{

	(void)cpipe;
	ref_pipeselwakeup_calls++;
}

void
finit(struct file *fp, u_int flag, short type, void *data,
    const struct fileops *ops)
{

	ref_finit_calls++;
	ref_finit_flag = flag;
	ref_finit_type = type;
	ref_finit_data = data;
	ref_finit_ops = ops;
	fp->f_flag = flag;
	fp->f_type = type;
	fp->f_data = data;
	fp->f_ops = ops;
}

int
vop_stdadvlock(struct vop_advlock_args *ap)
{

	ref_vop_stdadvlock_calls++;
	ref_vop_stdadvlock_flags = ap->a_flags;
	return (ref_vop_stdadvlock_ret);
}

int
sigdeferstop(int how)
{

	(void)how;
	return (0);
}

void
sigallowstop(int deferred)
{

	(void)deferred;
}

#define fifo_cleanup		ref_fifo_cleanup
#define fifo_printinfo		ref_fifo_printinfo

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

void
ref_fifo_cleanup(struct vnode *vp)
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

/* ARGSUSED */
int
ref_fifo_open(struct vop_open_args *ap)
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
		fip = malloc(sizeof(*fip), M_VNODE, M_WAITOK | M_ZERO);
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

/* ARGSUSED */
int
ref_fifo_close(struct vop_close_args *ap)
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

int
ref_fifo_printinfo(struct vnode *vp)
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

int
ref_fifo_print(struct vop_print_args *ap)
{
	printf("    ");
	fifo_printinfo(ap->a_vp);
	printf("\n");
	return (0);
}

/* ARGSUSED */
int
ref_fifo_advlock(struct vop_advlock_args *ap)
{

	if ((ap->a_flags & F_FLOCK) == 0)
		return (EINVAL);
	return (vop_stdadvlock(ap));
}

#undef free
#undef malloc
