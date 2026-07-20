module;

export module pbsd.userland.libthr.spinlock_try;

import pbsd.core;
import pbsd.userland.libthr.mutex;
import pbsd.userland.libthr.spinlock;

/// trylock from hbsd/src/lib/libthr/thread/thr_spinlock.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status spin_trylock(SpinLock& s, ThreadId self) noexcept {
    if (s.locked && s.owner != self) {
        return Status::Busy;
    }
    s.locked = true;
    s.owner = self;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
