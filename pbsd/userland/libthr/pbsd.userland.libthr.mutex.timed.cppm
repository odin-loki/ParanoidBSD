module;

export module pbsd.userland.libthr.mutex.timed;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// timed mutex lock from hbsd/src/lib/libthr/thread/thr_mutex.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status mutex_timedlock(Mutex& m, ThreadId self,
                                            unsigned ticks) noexcept {
    (void)ticks;
    if (self == kInvalidThread) {
        return Status::Invalid;
    }
    if (m.owner == kInvalidThread) {
        m.owner = self;
        m.lock_count = 1;
        return Status::Ok;
    }
    if (m.owner == self && m.type == MutexType::Recursive) {
        ++m.lock_count;
        return Status::Ok;
    }
    return Status::Busy;
}

} // namespace pbsd::userland::libthr
