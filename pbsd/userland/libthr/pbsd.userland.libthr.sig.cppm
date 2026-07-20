module;

export module pbsd.userland.libthr.sig;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_sigmask from hbsd/src/lib/libthr/thread/thr_sig.c
export namespace pbsd::userland::libthr {

enum class SigMaskOp : int { Block = 0, Unblock, Set };

[[nodiscard]] inline Status sigmask_op(SigMaskOp op) noexcept {
    (void)op;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
