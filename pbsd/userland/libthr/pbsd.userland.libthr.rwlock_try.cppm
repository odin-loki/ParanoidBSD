module;

export module pbsd.userland.libthr.rwlock_try;

import pbsd.core;
import pbsd.userland.libthr.mutex;
import pbsd.userland.libthr.rwlock;

/// tryrdlock/trywrlock from hbsd/src/lib/libthr/thread/thr_rwlock.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status rwlock_tryrdlock(RwLock& l) noexcept {
    if (l.writer != 0) {
        return Status::Busy;
    }
    ++l.readers;
    return Status::Ok;
}

[[nodiscard]] inline Status rwlock_trywrlock(RwLock& l, ThreadId self) noexcept {
    if (l.writer != 0 || l.readers > 0) {
        return Status::Busy;
    }
    l.writer = self;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
