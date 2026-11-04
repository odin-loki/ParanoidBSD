module;

export module pbsd.userland.libthr.rwlock_timed;

import pbsd.core;
import pbsd.userland.libthr.mutex;
import pbsd.userland.libthr.rwlock;
import pbsd.userland.libthr.rwlock_try;

/// pthread_rwlock_timed* from hbsd/src/lib/libthr/thread/thr_rwlock.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status rwlock_timedrdlock(RwLock& l, unsigned ticks) noexcept {
    (void)ticks;
    return rwlock_rdlock(l);
}

[[nodiscard]] inline Status rwlock_timedwrlock(RwLock& l, ThreadId self,
                                               unsigned ticks) noexcept {
    (void)ticks;
    return rwlock_trywrlock(l, self);
}

} // namespace pbsd::userland::libthr
