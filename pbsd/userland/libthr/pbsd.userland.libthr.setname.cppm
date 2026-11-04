module;

export module pbsd.userland.libthr.setname;

import pbsd.core;
import pbsd.userland.libthr.mutex;
import pbsd.userland.libthr.thread;

/// pthread_setname_np from hbsd/src/lib/libthr/thread/thr_setname.c
export namespace pbsd::userland::libthr {

inline constexpr int kMaxThreadName = 16;

[[nodiscard]] inline Status thread_setname(ThreadId tid, const char* name) noexcept {
    if (tid == kInvalidThread || name == nullptr) {
        return Status::Invalid;
    }
    (void)name;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
