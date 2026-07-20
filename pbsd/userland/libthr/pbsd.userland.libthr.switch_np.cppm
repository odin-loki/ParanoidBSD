module;

export module pbsd.userland.libthr.switch_np;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_switch_np from hbsd/src/lib/libthr/thread/thr_switch_np.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status switch_np_to(ThreadId from, ThreadId to) noexcept {
    if (from == kInvalidThread || to == kInvalidThread) {
        return Status::Invalid;
    }
    return Status::NotImplemented;
}

} // namespace pbsd::userland::libthr
