module;

export module pbsd.userland.libthr.clean;

import pbsd.core;
import pbsd.userland.libthr.mutex;

/// thr_clean from hbsd/src/lib/libthr/thread/thr_clean.c
export namespace pbsd::userland::libthr {

using CleanupFn = void (*)() noexcept;

[[nodiscard]] inline Status push_cleanup(CleanupFn fn) noexcept {
    (void)fn;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
