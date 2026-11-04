module;

export module pbsd.userland.libthr.resume_np;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_resume_np from hbsd/src/lib/libthr/thread/thr_resume_np.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status resume_np_thread(ThreadId tid) noexcept {
    if (tid == kInvalidThread) {
        return Status::Invalid;
    }
    return Status::NotImplemented;
}

} // namespace pbsd::userland::libthr
