// PBSD port of HardenedBSD sys/kern/kern_sema.c (batch b0146s4).

module;

#include <cstddef>
#include <cstdint>
#include <cstring>

export module pbsd.sys.kern.b0146s4;

namespace pbsd::sys_kern::b0146s4::detail {

#define KASSERT(cond, msg) ((void)0)

#define KTR_LOCK 0

constexpr int MTX_DEF = 0;
constexpr int MTX_NOWITNESS = 0;
constexpr int MTX_QUIET = 0;

constexpr int EWOULDBLOCK = 35;

constexpr int MODEL_WAIT_RUNAWAY = 256;

struct mtx {
	unsigned long long mtx_name_hash;
	unsigned long long mtx_type_hash;
	int mtx_flags;
	int mtx_locked;
};

struct cv {
	unsigned long long cv_desc_hash;
	int cv_waits;
	int cv_signals;
};

struct sema {
	mtx sema_mtx;
	cv sema_cv;
	int sema_value;
	int sema_waiters;
};

inline int g_cv_release_after = 1;
inline int g_cv_post_amount = 1;
inline int g_timedwait_post_after;
inline int g_timedwait_fail_after = 1;
inline sema *g_sema_target;

inline int g_total_waits;
inline int g_runaway;

inline unsigned long long
strhash(const char *s)
{
	unsigned long long h = 1469598103934665603ULL;

	if (s == nullptr)
		return (0);
	while (*s != '\0') {
		h ^= static_cast<unsigned long long>(static_cast<unsigned char>(*s++));
		h *= 1099511628211ULL;
	}
	return (h);
}

inline void
model_reset() noexcept
{
	g_total_waits = 0;
	g_runaway = 0;
}

inline void
model_set_sema_target(sema *p) noexcept
{
	g_sema_target = p;
}

inline void
model_configure_cv(int release_after, int post_amount,
    int timedwait_post_after, int timedwait_fail_after) noexcept
{
	g_cv_release_after = release_after;
	g_cv_post_amount = post_amount;
	g_timedwait_post_after = timedwait_post_after;
	g_timedwait_fail_after = timedwait_fail_after;
}

inline void
bzero(void *p, std::size_t n)
{
	std::memset(p, 0, n);
}

inline void
mtx_init(mtx *m, const char *name, const char *type, int opts)
{
	m->mtx_name_hash = strhash(name);
	m->mtx_type_hash = strhash(type);
	m->mtx_flags = opts;
	m->mtx_locked = 0;
}

inline void
mtx_destroy(mtx *m)
{
	m->mtx_name_hash = 0;
	m->mtx_type_hash = 0;
	m->mtx_flags = 0;
}

inline void
mtx_lock(mtx *m)
{
	m->mtx_locked++;
}

inline void
mtx_unlock(mtx *m)
{
	m->mtx_locked--;
}

inline void
cv_init(cv *c, const char *desc)
{
	c->cv_desc_hash = strhash(desc);
	c->cv_waits = 0;
	c->cv_signals = 0;
}

inline void
cv_destroy(cv *c)
{
	c->cv_desc_hash = 0;
}

inline const char *
cv_wmesg(cv *c)
{
	static char buf[1];

	(void)c;
	return (buf);
}

inline void
cv_signal(cv *c)
{
	c->cv_signals++;
}

inline void
cv_wait(cv *c, mtx *m)
{
	c->cv_waits++;
	g_total_waits++;
	m->mtx_locked--;
	if (g_total_waits > MODEL_WAIT_RUNAWAY) {
		g_runaway = 1;
		if (g_sema_target != nullptr)
			g_sema_target->sema_value = g_total_waits & 1;
	} else if (c->cv_waits >= g_cv_release_after && g_sema_target != nullptr)
		g_sema_target->sema_value += g_cv_post_amount;
	m->mtx_locked++;
}

inline int
cv_timedwait(cv *c, mtx *m, int timo)
{
	c->cv_waits++;
	g_total_waits++;
	m->mtx_locked--;
	m->mtx_locked++;
	if (g_total_waits > MODEL_WAIT_RUNAWAY) {
		g_runaway = 1;
		if (g_sema_target != nullptr)
			g_sema_target->sema_value = g_total_waits & 1;
		return (EWOULDBLOCK);
	}
	if (g_timedwait_post_after > 0 && c->cv_waits >= g_timedwait_post_after &&
	    g_sema_target != nullptr)
		g_sema_target->sema_value += 1;
	if (g_timedwait_fail_after > 0 && c->cv_waits >= g_timedwait_fail_after)
		return (EWOULDBLOCK);
	(void)timo;
	return (0);
}

} // namespace pbsd::sys_kern::b0146s4::detail

export namespace pbsd::sys_kern::b0146s4 {

using detail::sema;

#define KASSERT(cond, msg) ((void)0)
#define KTR_LOCK 0
#define CTR3(...) ((void)0)
#define CTR4(...) ((void)0)
#define CTR5(...) ((void)0)
#define CTR6(...) ((void)0)

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
sema_init(sema *sema, int value, const char *description)
{

KASSERT((value >= 0), ("%s(): negative value\n", __func__));

	detail::bzero(sema, sizeof(*sema));
	detail::mtx_init(&sema->sema_mtx, description, "sema backing lock",
	    detail::MTX_DEF | detail::MTX_NOWITNESS | detail::MTX_QUIET);
	detail::cv_init(&sema->sema_cv, description);
	sema->sema_value = value;

	CTR4(KTR_LOCK, "%s(%p, %d, \"%s\")", __func__, sema, value, description);
}

void
sema_destroy(sema *sema)
{

	CTR3(KTR_LOCK, "%s(%p) \"%s\"", __func__, sema,
	    detail::cv_wmesg(&sema->sema_cv));

KASSERT((sema->sema_waiters == 0), ("%s(): waiters\n", __func__));

	detail::mtx_destroy(&sema->sema_mtx);
	detail::cv_destroy(&sema->sema_cv);
}

void
_sema_post(sema *sema, const char *file, int line)
{

	detail::mtx_lock(&sema->sema_mtx);
	sema->sema_value++;
	if (sema->sema_waiters && sema->sema_value > 0)
		detail::cv_signal(&sema->sema_cv);

	CTR6(KTR_LOCK, "%s(%p) \"%s\" v = %d at %s:%d", __func__, sema,
	    detail::cv_wmesg(&sema->sema_cv), sema->sema_value, file, line);

	detail::mtx_unlock(&sema->sema_mtx);
}

void
_sema_wait(sema *sema, const char *file, int line)
{

	detail::mtx_lock(&sema->sema_mtx);
	while (sema->sema_value == 0) {
		sema->sema_waiters++;
		detail::cv_wait(&sema->sema_cv, &sema->sema_mtx);
		sema->sema_waiters--;
	}
	sema->sema_value--;

	CTR6(KTR_LOCK, "%s(%p) \"%s\" v = %d at %s:%d", __func__, sema,
	    detail::cv_wmesg(&sema->sema_cv), sema->sema_value, file, line);

	detail::mtx_unlock(&sema->sema_mtx);
}

int
_sema_timedwait(sema *sema, int timo, const char *file, int line)
{
	int error;

	detail::mtx_lock(&sema->sema_mtx);

	/*
	 * A spurious wakeup will cause the timeout interval to start over.
	 * This isn't a big deal as long as spurious wakeups don't occur
	 * continuously, since the timeout period is merely a lower bound on how
	 * long to wait.
	 */
	for (error = 0; sema->sema_value == 0 && error == 0;) {
		sema->sema_waiters++;
		error = detail::cv_timedwait(&sema->sema_cv, &sema->sema_mtx, timo);
		sema->sema_waiters--;
	}
	if (sema->sema_value > 0) {
		/* Success. */
		sema->sema_value--;
		error = 0;

		CTR6(KTR_LOCK, "%s(%p) \"%s\" v = %d at %s:%d", __func__, sema,
		    detail::cv_wmesg(&sema->sema_cv), sema->sema_value, file, line);
	} else {
		CTR5(KTR_LOCK, "%s(%p) \"%s\" fail at %s:%d", __func__, sema,
		    detail::cv_wmesg(&sema->sema_cv), file, line);
	}

	detail::mtx_unlock(&sema->sema_mtx);
	return (error);
}

int
_sema_trywait(sema *sema, const char *file, int line)
{
	int ret;

	detail::mtx_lock(&sema->sema_mtx);

	if (sema->sema_value > 0) {
		/* Success. */
		sema->sema_value--;
		ret = 1;

		CTR6(KTR_LOCK, "%s(%p) \"%s\" v = %d at %s:%d", __func__, sema,
		    detail::cv_wmesg(&sema->sema_cv), sema->sema_value, file, line);
	} else {
		ret = 0;

		CTR5(KTR_LOCK, "%s(%p) \"%s\" fail at %s:%d", __func__, sema,
		    detail::cv_wmesg(&sema->sema_cv), file, line);
	}

	detail::mtx_unlock(&sema->sema_mtx);
	return (ret);
}

int
sema_value(sema *sema)
{
	int ret;

	detail::mtx_lock(&sema->sema_mtx);
	ret = sema->sema_value;
	detail::mtx_unlock(&sema->sema_mtx);
	return (ret);
}

inline void model_reset() noexcept
{
	detail::model_reset();
}

inline void model_set_sema_target(sema *p) noexcept
{
	detail::model_set_sema_target(p);
}

inline void model_configure_cv(int release_after, int post_amount,
    int timedwait_post_after, int timedwait_fail_after) noexcept
{
	detail::model_configure_cv(release_after, post_amount,
	    timedwait_post_after, timedwait_fail_after);
}

} // namespace pbsd::sys_kern::b0146s4
