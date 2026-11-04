module;

export module pbsd.userland.libthr.pspinlock;

import pbsd.core;
import pbsd.userland.libthr.mutex;
import pbsd.userland.libthr.spinlock;

/// thr_pspinlock from hbsd/src/lib/libthr/thread/thr_pspinlock.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status pspinlock_init(SpinLock& s) noexcept {
    s.locked = false;
    s.owner = kInvalidThread;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
