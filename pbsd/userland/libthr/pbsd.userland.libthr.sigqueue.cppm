module;

export module pbsd.userland.libthr.sigqueue;

import pbsd.core;
export import pbsd.userland.libthr.mutex;

/// pthread_sigqueue from hbsd/src/lib/libthr/thread/thr_sigqueue.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status sigqueue_thread(ThreadId tid, int sig) noexcept {
    if (tid == kInvalidThread) {
        return Status::Invalid;
    }
    (void)sig;
    return Status::NotImplemented;
}

} // namespace pbsd::userland::libthr
