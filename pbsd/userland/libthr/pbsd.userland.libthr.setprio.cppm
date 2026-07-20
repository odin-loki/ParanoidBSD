module;

export module pbsd.userland.libthr.setprio;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_setprio from hbsd/src/lib/libthr/thread/thr_setprio.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status setprio_thread(ThreadId tid, int prio) noexcept {
    if (tid == kInvalidThread) {
        return Status::Invalid;
    }
    (void)prio;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
