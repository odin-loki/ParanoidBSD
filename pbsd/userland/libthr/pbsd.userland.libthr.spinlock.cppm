module;

export module pbsd.userland.libthr.spinlock;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread spinlock concepts from hbsd/src/lib/libthr/thread/thr_spinlock.c
export namespace pbsd::userland::libthr {

struct SpinLock {
    ThreadId owner{kInvalidThread};
    bool locked{false};
};

[[nodiscard]] inline Status spin_init(SpinLock& s) noexcept {
    s = SpinLock{};
    return Status::Ok;
}

[[nodiscard]] inline Status spin_lock(SpinLock& s, ThreadId self) noexcept {
    if (s.locked && s.owner != self) {
        return Status::Busy;
    }
    s.locked = true;
    s.owner = self;
    return Status::Ok;
}

[[nodiscard]] inline Status spin_unlock(SpinLock& s, ThreadId self) noexcept {
    if (!s.locked || s.owner != self) {
        return Status::Denied;
    }
    s.locked = false;
    s.owner = kInvalidThread;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
