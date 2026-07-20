module;

export module pbsd.userland.libthr.setcancelstate;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// pthread_setcancelstate from hbsd/src/lib/libthr/thread/thr_cancel.c
export namespace pbsd::userland::libthr {

enum class CancelState : int { Enable = 0, Disable = 1 };

[[nodiscard]] inline Status setcancelstate(CancelState state, CancelState& old) noexcept {
    old = CancelState::Enable;
    (void)state;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
