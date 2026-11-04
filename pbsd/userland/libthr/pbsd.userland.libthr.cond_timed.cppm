module;

export module pbsd.userland.libthr.cond_timed;

import pbsd.core;
import pbsd.userland.libthr.cond;
import pbsd.userland.libthr.mutex;

/// pthread_cond_timedwait from hbsd/src/lib/libthr/thread/thr_cond.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status cond_timedwait(Cond& c, Mutex& m, ThreadId self,
                                         unsigned ticks) noexcept {
    (void)ticks;
    return cond_wait(c, m, self);
}

} // namespace pbsd::userland::libthr
