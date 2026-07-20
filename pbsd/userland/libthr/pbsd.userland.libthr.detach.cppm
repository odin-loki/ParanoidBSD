module;

export module pbsd.userland.libthr.detach;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_detach from hbsd/src/lib/libthr/thread/thr_detach.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status thread_detach(ThreadId tid) noexcept {
    if (tid == kInvalidThread) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
