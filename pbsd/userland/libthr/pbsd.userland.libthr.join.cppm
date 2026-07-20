module;

export module pbsd.userland.libthr.join;

import pbsd.core;
import pbsd.userland.libthr.mutex;
import pbsd.userland.libthr.thread;

/// pthread_join from hbsd/src/lib/libthr/thread/thr_join.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status thread_join(ThreadId tid, void** value_ptr) noexcept {
    if (tid == kInvalidThread) {
        return Status::Invalid;
    }
    if (value_ptr != nullptr) {
        *value_ptr = nullptr;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
