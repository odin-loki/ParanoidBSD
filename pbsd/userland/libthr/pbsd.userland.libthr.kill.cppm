module;

export module pbsd.userland.libthr.kill;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_kill from hbsd/src/lib/libthr/thread/thr_kill.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status thread_kill(ThreadId tid, int sig) noexcept {
    if (tid == kInvalidThread || sig < 0) {
        return Status::Invalid;
    }
    return Status::NotImplemented;
}

} // namespace pbsd::userland::libthr
