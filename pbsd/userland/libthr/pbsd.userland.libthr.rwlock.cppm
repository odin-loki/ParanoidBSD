module;

export module pbsd.userland.libthr.rwlock;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_rwlock concepts from hbsd/src/lib/libthr/thread/thr_rwlock*.c
export namespace pbsd::userland::libthr {

struct RwLock {
    ThreadId writer{kInvalidThread};
    unsigned readers{0};
};

[[nodiscard]] inline Status rwlock_init(RwLock& l) noexcept {
    l = RwLock{};
    return Status::Ok;
}

[[nodiscard]] inline Status rwlock_rdlock(RwLock& l) noexcept {
    if (l.writer != kInvalidThread) {
        return Status::Busy;
    }
    ++l.readers;
    return Status::Ok;
}

[[nodiscard]] inline Status rwlock_wrlock(RwLock& l, ThreadId self) noexcept {
    if (l.writer != kInvalidThread || l.readers > 0) {
        return Status::Busy;
    }
    l.writer = self;
    return Status::Ok;
}

[[nodiscard]] inline Status rwlock_unlock(RwLock& l, ThreadId self) noexcept {
    if (l.writer == self) {
        l.writer = kInvalidThread;
        return Status::Ok;
    }
    if (l.readers > 0) {
        --l.readers;
        return Status::Ok;
    }
    return Status::Denied;
}

} // namespace pbsd::userland::libthr
