module;

export module pbsd.userland.libthr.sleepq;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_sleepq from hbsd/src/lib/libthr/thread/thr_sleepq.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status sleepq_enqueue(ThreadId tid) noexcept {
    if (tid == kInvalidThread) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
