/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (C) 2001 Jason Evans <jasone@freebsd.org>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice(s), this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified other than the possible 
 *    addition of one or more copyright notices.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice(s), this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/*
 * Counting semaphores.
 *
 * Priority propagation will not generally raise the priority of semaphore
 * "owners" (a misnomer in the context of semaphores), so should not be relied
 * upon in combination with semaphores.
 */

/*
 * PBSD batch b0146s4 oracle.
 *
 * Source: hbsd/src/sys/kern/kern_sema.c
 *
 * Every function of the original file appears below with a ref_ prefix and an
 * otherwise unmodified body.  The kernel services the file is written against
 * (mutexes, condition variables, KTR tracing, KASSERT and bzero) are not
 * available in userland, so a deterministic model of them is supplied here.
 * The model records every call it receives into an event trace and lets the
 * caller script the outcome of the blocking condition variable waits, which is
 * what makes the six functions observable.  The identical model is
 * reimplemented by the port; the harness compares the two traces.
 */

#include <stddef.h>
#include <string.h>
#include <setjmp.h>

/* sys/mutex.h */
#define	MTX_DEF		0x00000000
#define	MTX_NOWITNESS	0x00000008
#define	MTX_QUIET	0x00040000

/* sys/ktr.h */
#define	KTR_LOCK	0x00000004

/* Event opcodes; shared verbatim with the port and the harness. */
#define	EV_KASSERT	1
#define	EV_BZERO	2
#define	EV_MTX_INIT	3
#define	EV_CV_INIT	4
#define	EV_MTX_LOCK	5
#define	EV_MTX_UNLOCK	6
#define	EV_MTX_DESTROY	7
#define	EV_CV_DESTROY	8
#define	EV_CV_SIGNAL	9
#define	EV_CV_WMESG	10
#define	EV_CV_WAIT	11
#define	EV_CV_TIMEDWAIT	12
#define	EV_CTR3		13
#define	EV_CTR4		14
#define	EV_CTR5		15
#define	EV_CTR6		16

#define	PBSD_MAXEV	4096
#define	PBSD_NREG	8
#define	PBSD_EVARGS	8
#define	PBSD_WAITLIMIT	256

struct mtx {
	const char	*mtx_name;
	const char	*mtx_type;
	int		 mtx_opts;
	int		 mtx_inited;
	int		 mtx_locked;
	int		 mtx_lockcnt;
	int		 mtx_unlockcnt;
	int		 mtx_destroyed;
};

struct cv {
	const char	*cv_description;
	int		 cv_inited;
	int		 cv_signals;
	int		 cv_waits;
	int		 cv_destroyed;
};

struct sema {
	struct mtx	sema_mtx;
	struct cv	sema_cv;
	int		sema_value;
	int		sema_waiters;
};

struct ref_event {
	int	op;
	long	a[PBSD_EVARGS];
};

static struct ref_event	 ref_evbuf[PBSD_MAXEV];
static long		 ref_nev;
static long		 ref_evtotal;
static const void	*ref_reg[PBSD_NREG];
static int		 ref_nreg;
static const int	*ref_scr_delta;
static const int	*ref_scr_err;
static int		 ref_scr_n;
static int		 ref_scr_idx;
static int		 ref_scr_over;
static int		 ref_waitcalls;
static int		 ref_runaway;
static jmp_buf		 ref_jb;

static long
ref_ptr_id(const void *p)
{
	int i;

	if (p == NULL)
		return (0);
	for (i = 0; i < ref_nreg; i++) {
		if (ref_reg[i] == p)
			return (i + 1);
	}
	return (-1);
}

static long
ref_str_hash(const char *s)
{
	unsigned long h = 14695981039346656037UL;

	if (s == NULL)
		return (0);
	while (*s != '\0') {
		h ^= (unsigned long)(unsigned char)*s++;
		h *= 1099511628211UL;
	}
	return ((long)(h & 0x7fffffffUL));
}

static struct ref_event *
ref_rec(int op)
{
	static struct ref_event sink;
	struct ref_event *e;

	ref_evtotal++;
	if (ref_nev < PBSD_MAXEV)
		e = &ref_evbuf[ref_nev++];
	else
		e = &sink;
	memset(e, 0, sizeof(*e));
	e->op = op;
	return (e);
}

static void
ref_bzero(void *dst, size_t len)
{
	struct ref_event *e;

	e = ref_rec(EV_BZERO);
	e->a[0] = ref_ptr_id(dst);
	e->a[1] = (long)len;
	memset(dst, 0, len);
}

static void
ref_assert(int cond)
{
	struct ref_event *e;

	e = ref_rec(EV_KASSERT);
	e->a[0] = cond ? 1 : 0;
}

static void
mtx_init(struct mtx *m, const char *name, const char *type, int opts)
{
	struct ref_event *e;

	e = ref_rec(EV_MTX_INIT);
	e->a[0] = ref_ptr_id(m);
	e->a[1] = ref_ptr_id(name);
	e->a[2] = ref_str_hash(type);
	e->a[3] = opts;
	m->mtx_name = name;
	m->mtx_type = type;
	m->mtx_opts = opts;
	m->mtx_inited = 1;
}

static void
mtx_destroy(struct mtx *m)
{
	struct ref_event *e;

	e = ref_rec(EV_MTX_DESTROY);
	e->a[0] = ref_ptr_id(m);
	e->a[1] = m->mtx_locked;
	m->mtx_destroyed++;
	m->mtx_inited = 0;
}

static void
mtx_lock(struct mtx *m)
{
	struct ref_event *e;

	e = ref_rec(EV_MTX_LOCK);
	e->a[0] = ref_ptr_id(m);
	e->a[1] = m->mtx_locked;
	m->mtx_locked = 1;
	m->mtx_lockcnt++;
	e->a[2] = m->mtx_lockcnt;
}

static void
mtx_unlock(struct mtx *m)
{
	struct ref_event *e;

	e = ref_rec(EV_MTX_UNLOCK);
	e->a[0] = ref_ptr_id(m);
	e->a[1] = m->mtx_locked;
	m->mtx_locked = 0;
	m->mtx_unlockcnt++;
	e->a[2] = m->mtx_unlockcnt;
}

static void
cv_init(struct cv *cvp, const char *desc)
{
	struct ref_event *e;

	e = ref_rec(EV_CV_INIT);
	e->a[0] = ref_ptr_id(cvp);
	e->a[1] = ref_ptr_id(desc);
	cvp->cv_description = desc;
	cvp->cv_inited = 1;
}

static void
cv_destroy(struct cv *cvp)
{
	struct ref_event *e;

	e = ref_rec(EV_CV_DESTROY);
	e->a[0] = ref_ptr_id(cvp);
	cvp->cv_destroyed++;
	cvp->cv_inited = 0;
}

static void
cv_signal(struct cv *cvp)
{
	struct ref_event *e;

	e = ref_rec(EV_CV_SIGNAL);
	e->a[0] = ref_ptr_id(cvp);
	cvp->cv_signals++;
	e->a[1] = cvp->cv_signals;
}

static const char *
cv_wmesg(struct cv *cvp)
{
	struct ref_event *e;

	e = ref_rec(EV_CV_WMESG);
	e->a[0] = ref_ptr_id(cvp);
	e->a[1] = ref_ptr_id(cvp->cv_description);
	return (cvp->cv_description);
}

/*
 * The scripted wakeup.  Each wait consumes one entry of the script; the entry
 * says how much a notional producer changed sema_value by while the waiter was
 * asleep and, for the timed variant, what cv_timedwait() returns.  When the
 * script is exhausted the model forces progress so that a correct port always
 * terminates: an untimed wait observes a value bump of one, a timed wait fails.
 * A port whose loop condition has been broken can still spin, so the number of
 * waits per operation is capped, and the cap escapes through longjmp() rather
 * than hanging the harness.
 */
static int
ref_wait_common(struct ref_event *e, struct cv *cvp, struct mtx *m, int *errp)
{
	struct sema *s;
	int delta, err;

	s = (struct sema *)(void *)((char *)cvp -
	    offsetof(struct sema, sema_cv));
	if (++ref_waitcalls > PBSD_WAITLIMIT) {
		ref_runaway = 1;
		longjmp(ref_jb, 1);
	}
	e->a[0] = ref_ptr_id(cvp);
	e->a[1] = ref_ptr_id(m);
	e->a[2] = s->sema_value;
	e->a[3] = s->sema_waiters;
	if (ref_scr_idx < ref_scr_n) {
		delta = ref_scr_delta[ref_scr_idx];
		err = ref_scr_err[ref_scr_idx];
	} else {
		delta = (errp == NULL) ? 1 : 0;
		err = 35;			/* EWOULDBLOCK */
		ref_scr_over++;
	}
	ref_scr_idx++;
	e->a[4] = delta;
	e->a[5] = ref_scr_idx;
	e->a[6] = ref_scr_over;
	e->a[7] = m->mtx_locked;
	cvp->cv_waits++;
	m->mtx_locked = 0;
	s->sema_value += delta;
	m->mtx_locked = 1;
	if (errp != NULL)
		*errp = err;
	return (delta);
}

static void
cv_wait(struct cv *cvp, struct mtx *m)
{

	(void)ref_wait_common(ref_rec(EV_CV_WAIT), cvp, m, NULL);
}

static int
cv_timedwait(struct cv *cvp, struct mtx *m, int timo)
{
	struct ref_event *e;
	int err = 0;

	e = ref_rec(EV_CV_TIMEDWAIT);
	(void)ref_wait_common(e, cvp, m, &err);
	e->a[6] = timo;
	e->a[7] = err;
	return (err);
}

static void
ref_ctr3(int mask, const char *fmt, const void *p1, const void *p2)
{
	struct ref_event *e;

	e = ref_rec(EV_CTR3);
	e->a[0] = mask;
	e->a[1] = ref_str_hash(fmt);
	e->a[2] = ref_ptr_id(p1);
	e->a[3] = ref_ptr_id(p2);
}

static void
ref_ctr4(int mask, const char *fmt, const void *p1, int i2, const void *p3)
{
	struct ref_event *e;

	e = ref_rec(EV_CTR4);
	e->a[0] = mask;
	e->a[1] = ref_str_hash(fmt);
	e->a[2] = ref_ptr_id(p1);
	e->a[3] = i2;
	e->a[4] = ref_ptr_id(p3);
}

static void
ref_ctr5(int mask, const char *fmt, const void *p1, const void *p2,
    const void *p3, int i4)
{
	struct ref_event *e;

	e = ref_rec(EV_CTR5);
	e->a[0] = mask;
	e->a[1] = ref_str_hash(fmt);
	e->a[2] = ref_ptr_id(p1);
	e->a[3] = ref_ptr_id(p2);
	e->a[4] = ref_ptr_id(p3);
	e->a[5] = i4;
}

static void
ref_ctr6(int mask, const char *fmt, const void *p1, const void *p2, int i3,
    const void *p4, int i5)
{
	struct ref_event *e;

	e = ref_rec(EV_CTR6);
	e->a[0] = mask;
	e->a[1] = ref_str_hash(fmt);
	e->a[2] = ref_ptr_id(p1);
	e->a[3] = ref_ptr_id(p2);
	e->a[4] = i3;
	e->a[5] = ref_ptr_id(p4);
	e->a[6] = i5;
}

/*
 * KASSERT() is modelled after an INVARIANTS kernel except that a failed
 * assertion is recorded instead of panicking, so that both sides can be run to
 * completion and compared.  The message argument is not evaluated, exactly as
 * in a kernel where the assertion holds.
 */
#define	KASSERT(exp, msg)	ref_assert((exp) ? 1 : 0)
#define	bzero(dst, len)		ref_bzero((dst), (len))
#define	CTR3(mask, fmt, f, p1, p2)					\
	ref_ctr3((mask), (fmt), (p1), (p2))
#define	CTR4(mask, fmt, f, p1, i2, p3)					\
	ref_ctr4((mask), (fmt), (p1), (i2), (p3))
#define	CTR5(mask, fmt, f, p1, p2, p3, i4)				\
	ref_ctr5((mask), (fmt), (p1), (p2), (p3), (i4))
#define	CTR6(mask, fmt, f, p1, p2, i3, p4, i5)				\
	ref_ctr6((mask), (fmt), (p1), (p2), (i3), (p4), (i5))

/* ---------- original kern_sema.c bodies, unmodified ---------- */

void
ref_sema_init(struct sema *sema, int value, const char *description)
{

	KASSERT((value >= 0), ("%s(): negative value\n", __func__));

	bzero(sema, sizeof(*sema));
	mtx_init(&sema->sema_mtx, description, "sema backing lock",
	    MTX_DEF | MTX_NOWITNESS | MTX_QUIET);
	cv_init(&sema->sema_cv, description);
	sema->sema_value = value;

	CTR4(KTR_LOCK, "%s(%p, %d, \"%s\")", __func__, sema, value, description);
}

void
ref_sema_destroy(struct sema *sema)
{

	CTR3(KTR_LOCK, "%s(%p) \"%s\"", __func__, sema,
	    cv_wmesg(&sema->sema_cv));

	KASSERT((sema->sema_waiters == 0), ("%s(): waiters\n", __func__));

	mtx_destroy(&sema->sema_mtx);
	cv_destroy(&sema->sema_cv);
}

void
ref__sema_post(struct sema *sema, const char *file, int line)
{

	mtx_lock(&sema->sema_mtx);
	sema->sema_value++;
	if (sema->sema_waiters && sema->sema_value > 0)
		cv_signal(&sema->sema_cv);

	CTR6(KTR_LOCK, "%s(%p) \"%s\" v = %d at %s:%d", __func__, sema,
	    cv_wmesg(&sema->sema_cv), sema->sema_value, file, line);

	mtx_unlock(&sema->sema_mtx);
}

void
ref__sema_wait(struct sema *sema, const char *file, int line)
{

	mtx_lock(&sema->sema_mtx);
	while (sema->sema_value == 0) {
		sema->sema_waiters++;
		cv_wait(&sema->sema_cv, &sema->sema_mtx);
		sema->sema_waiters--;
	}
	sema->sema_value--;

	CTR6(KTR_LOCK, "%s(%p) \"%s\" v = %d at %s:%d", __func__, sema,
	    cv_wmesg(&sema->sema_cv), sema->sema_value, file, line);

	mtx_unlock(&sema->sema_mtx);
}

int
ref__sema_timedwait(struct sema *sema, int timo, const char *file, int line)
{
	int error;

	mtx_lock(&sema->sema_mtx);

	/*
	 * A spurious wakeup will cause the timeout interval to start over.
	 * This isn't a big deal as long as spurious wakeups don't occur
	 * continuously, since the timeout period is merely a lower bound on how
	 * long to wait.
	 */
	for (error = 0; sema->sema_value == 0 && error == 0;) {
		sema->sema_waiters++;
		error = cv_timedwait(&sema->sema_cv, &sema->sema_mtx, timo);
		sema->sema_waiters--;
	}
	if (sema->sema_value > 0) {
		/* Success. */
		sema->sema_value--;
		error = 0;

		CTR6(KTR_LOCK, "%s(%p) \"%s\" v = %d at %s:%d", __func__, sema,
		    cv_wmesg(&sema->sema_cv), sema->sema_value, file, line);
	} else {
		CTR5(KTR_LOCK, "%s(%p) \"%s\" fail at %s:%d", __func__, sema,
		    cv_wmesg(&sema->sema_cv), file, line);
	}

	mtx_unlock(&sema->sema_mtx);
	return (error);
}

int
ref__sema_trywait(struct sema *sema, const char *file, int line)
{
	int ret;

	mtx_lock(&sema->sema_mtx);

	if (sema->sema_value > 0) {
		/* Success. */
		sema->sema_value--;
		ret = 1;

		CTR6(KTR_LOCK, "%s(%p) \"%s\" v = %d at %s:%d", __func__, sema,
		    cv_wmesg(&sema->sema_cv), sema->sema_value, file, line);
	} else {
		ret = 0;

		CTR5(KTR_LOCK, "%s(%p) \"%s\" fail at %s:%d", __func__, sema,
		    cv_wmesg(&sema->sema_cv), file, line);
	}

	mtx_unlock(&sema->sema_mtx);
	return (ret);
}

int
ref_sema_value(struct sema *sema)
{
	int ret;

	mtx_lock(&sema->sema_mtx);
	ret = sema->sema_value;
	mtx_unlock(&sema->sema_mtx);
	return (ret);
}

/* ---------- observation interface used by the harness ---------- */

size_t
ref_obj_size(void)
{

	return (sizeof(struct sema));
}

size_t
ref_obj_align(void)
{

	return (_Alignof(struct sema));
}

void
ref_env_reset(void *obj, const void *desc, const void *file, const int *deltas,
    const int *errs, int nacts)
{
	struct sema *s = (struct sema *)obj;

	ref_reg[0] = obj;
	ref_reg[1] = desc;
	ref_reg[2] = file;
	ref_reg[3] = (const void *)&s->sema_mtx;
	ref_reg[4] = (const void *)&s->sema_cv;
	ref_nreg = 5;
	ref_nev = 0;
	ref_evtotal = 0;
	ref_scr_delta = deltas;
	ref_scr_err = errs;
	ref_scr_n = nacts;
	ref_scr_idx = 0;
	ref_scr_over = 0;
	ref_waitcalls = 0;
	ref_runaway = 0;
}

void *
ref_jmpbuf(void)
{

	return ((void *)&ref_jb);
}

int
ref_runaway_flag(void)
{

	return (ref_runaway);
}

void
ref_set_state(void *obj, int value, int waiters)
{
	struct sema *s = (struct sema *)obj;

	s->sema_value = value;
	s->sema_waiters = waiters;
}

long
ref_trace_count(void)
{

	return (ref_nev);
}

void
ref_trace_get(long i, int *op, long *a)
{
	int k;

	*op = ref_evbuf[i].op;
	for (k = 0; k < PBSD_EVARGS; k++)
		a[k] = ref_evbuf[i].a[k];
}

void
ref_snapshot(const void *obj, long *out)
{
	const struct sema *s = (const struct sema *)obj;

	out[0] = ref_ptr_id(s->sema_mtx.mtx_name);
	out[1] = ref_ptr_id(s->sema_mtx.mtx_type);
	out[2] = s->sema_mtx.mtx_opts;
	out[3] = s->sema_mtx.mtx_inited;
	out[4] = s->sema_mtx.mtx_locked;
	out[5] = s->sema_mtx.mtx_lockcnt;
	out[6] = s->sema_mtx.mtx_unlockcnt;
	out[7] = s->sema_mtx.mtx_destroyed;
	out[8] = ref_ptr_id(s->sema_cv.cv_description);
	out[9] = s->sema_cv.cv_inited;
	out[10] = s->sema_cv.cv_signals;
	out[11] = s->sema_cv.cv_waits;
	out[12] = s->sema_cv.cv_destroyed;
	out[13] = s->sema_value;
	out[14] = s->sema_waiters;
	out[15] = ref_scr_idx;
	out[16] = ref_scr_over;
	out[17] = ref_evtotal;
}
