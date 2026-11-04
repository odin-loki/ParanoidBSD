module;

export module pbsd.userland.libthr.suspend_np;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_suspend_np from hbsd/src/lib/libthr/thread/thr_suspend_np.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status suspend_np_thread(ThreadId tid) noexcept {
    if (tid == kInvalidThread) {
        return Status::Invalid;
    }
    return Status::NotImplemented;
}

} // namespace pbsd::userland::libthr
