module;

export module pbsd.userland.libthr.getprio;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_getprio from hbsd/src/lib/libthr/thread/thr_getprio.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Result<int> getprio_thread(ThreadId tid) noexcept {
    if (tid == kInvalidThread) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(0);
}

} // namespace pbsd::userland::libthr
