/*
 * PBSD batch b0146s4 -- reference oracle.
 *
 * Source: hbsd/src/sys/kern/kern_sema.c
 *
 * Every function is renamed with a "ref_" prefix.  Function bodies are
 * otherwise UNMODIFIED.  The kernel environment (types, constants, and the
 * mutex(9), condvar(9), KASSERT(9), and CTR* routines) is modelled below and
 * shared identically with the C++23 port under test.
 */

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef LONG_BIT
#define	LONG_BIT	(sizeof(long) * CHAR_BIT)
#endif

#define	__unused		__attribute__((__unused__))

#define	MTX_DEF			0x00000000
#define	MTX_SPIN		0x00000001
#define	MTX_NOWITNESS		0x00000008
#define	MTX_QUIET		0x00040000

#define	KTR_LOCK		0x00000004

#ifndef EWOULDBLOCK
#define	EWOULDBLOCK		35
#endif

struct mtx {
	unsigned long long	mtx_name_hash;
	unsigned long long	mtx_type_hash;
	int			mtx_flags;
	int			mtx_locked;
};

struct cv {
	unsigned long long	cv_desc_hash;
	int			cv_waits;
	int			cv_signals;
};

struct sema {
	struct mtx	sema_mtx;
	struct cv	sema_cv;
	int		sema_value;
	int		sema_waiters;
};

#define	MODEL_WAIT_RUNAWAY	256

int		oracle_cv_release_after = 1;
int		oracle_cv_post_amount = 1;
int		oracle_timedwait_post_after;
int		oracle_timedwait_fail_after = 1;
struct sema	*oracle_sema_target;

static int	oracle_total_waits;
static int	oracle_runaway;

static unsigned long long
oracle_strhash(const char *s)
{
	unsigned long long h = 1469598103934665603ULL;

	if (s == NULL)
		return (0);
	while (*s != '\0') {
		h ^= (unsigned long long)(unsigned char)*s++;
		h *= 1099511628211ULL;
	}
	return (h);
}

void
oracle_reset(void)
{

	oracle_total_waits = 0;
	oracle_runaway = 0;
}

void
oracle_set_sema_target(void *p)
{

	oracle_sema_target = (struct sema *)p;
}

void
oracle_configure_cv(int release_after, int post_amount,
    int timedwait_post_after, int timedwait_fail_after)
{

	oracle_cv_release_after = release_after;
	oracle_cv_post_amount = post_amount;
	oracle_timedwait_post_after = timedwait_post_after;
	oracle_timedwait_fail_after = timedwait_fail_after;
}

static void
oracle_assert(int cond)
{

	(void)cond;
}

static void
oracle_ctr(int arity, long long v1, long long v2, const char *s)
{

	(void)arity;
	(void)v1;
	(void)v2;
	(void)s;
}

static void
oracle_bzero(void *p, unsigned long len)
{

	memset(p, 0, (size_t)len);
}

static void
oracle_mtx_init(struct mtx *m, const char *name, const char *type, int opts)
{

	m->mtx_name_hash = oracle_strhash(name);
	m->mtx_type_hash = oracle_strhash(type);
	m->mtx_flags = opts;
	m->mtx_locked = 0;
}

static void
oracle_mtx_destroy(struct mtx *m)
{

	m->mtx_name_hash = 0;
	m->mtx_type_hash = 0;
	m->mtx_flags = 0;
}

static void
oracle_mtx_lock(struct mtx *m)
{

	m->mtx_locked++;
}

static void
oracle_mtx_unlock(struct mtx *m)
{

	m->mtx_locked--;
}

static void
oracle_cv_init(struct cv *cv, const char *desc)
{

	cv->cv_desc_hash = oracle_strhash(desc);
	cv->cv_waits = 0;
	cv->cv_signals = 0;
}

static void
oracle_cv_destroy(struct cv *cv)
{

	cv->cv_desc_hash = 0;
}

static const char *
oracle_cv_wmesg(struct cv *cv)
{

	static char buf[1];

	(void)cv;
	return (buf);
}

static void
oracle_cv_signal(struct cv *cv)
{

	cv->cv_signals++;
}

static void
oracle_cv_wait(struct cv *cv, struct mtx *m)
{

	cv->cv_waits++;
	oracle_total_waits++;
	m->mtx_locked--;
	if (oracle_total_waits > MODEL_WAIT_RUNAWAY) {
		oracle_runaway = 1;
		if (oracle_sema_target != NULL)
			oracle_sema_target->sema_value = oracle_total_waits & 1;
	} else if (cv->cv_waits >= oracle_cv_release_after &&
	    oracle_sema_target != NULL)
		oracle_sema_target->sema_value += oracle_cv_post_amount;
	m->mtx_locked++;
}

static int
oracle_cv_timedwait(struct cv *cv, struct mtx *m, int timo)
{

	cv->cv_waits++;
	oracle_total_waits++;
	m->mtx_locked--;
	m->mtx_locked++;
	if (oracle_total_waits > MODEL_WAIT_RUNAWAY) {
		oracle_runaway = 1;
		if (oracle_sema_target != NULL)
			oracle_sema_target->sema_value = oracle_total_waits & 1;
		return (EWOULDBLOCK);
	}
	if (oracle_timedwait_post_after > 0 &&
	    cv->cv_waits >= oracle_timedwait_post_after &&
	    oracle_sema_target != NULL)
		oracle_sema_target->sema_value += 1;
	if (oracle_timedwait_fail_after > 0 &&
	    cv->cv_waits >= oracle_timedwait_fail_after)
		return (EWOULDBLOCK);
	(void)timo;
	return (0);
}

#define	KASSERT(exp, msg)		oracle_assert((exp) ? 1 : 0)
#define	CTR3(m, f, a1, a2, a3)		oracle_ctr(3, 0, 0, (const char *)(a3))
#define	CTR4(m, f, a1, a2, a3, a4)	oracle_ctr(4, (long long)(a3), 0,	\
					    (const char *)(a4))
#define	CTR5(m, f, a1, a2, a3, a4, a5)	oracle_ctr(5, (long long)(a5), 0,	\
					    (const char *)(a3))
#define	CTR6(m, f, a1, a2, a3, a4, a5, a6)					\
					oracle_ctr(6, (long long)(a4),		\
					    (long long)(a6), (const char *)(a3))

#define	bzero(p, n)			oracle_bzero((p), (unsigned long)(n))
#define	mtx_init(m, n, t, o)		oracle_mtx_init((m), (n), (t), (o))
#define	mtx_destroy(m)			oracle_mtx_destroy(m)
#define	mtx_lock(m)			oracle_mtx_lock(m)
#define	mtx_unlock(m)			oracle_mtx_unlock(m)
#define	cv_init(c, d)			oracle_cv_init((c), (d))
#define	cv_destroy(c)			oracle_cv_destroy(c)
#define	cv_wmesg(c)			oracle_cv_wmesg(c)
#define	cv_signal(c)			oracle_cv_signal(c)
#define	cv_wait(c, m)			oracle_cv_wait((c), (m))
#define	cv_timedwait(c, m, t)		oracle_cv_timedwait((c), (m), (t))

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
