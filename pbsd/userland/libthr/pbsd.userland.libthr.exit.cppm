module;

export module pbsd.userland.libthr.exit;

import pbsd.core;

/// pthread_exit from hbsd/src/lib/libthr/thread/thr_exit.c
export namespace pbsd::userland::libthr {

[[nodiscard]] inline Status thread_exit(void* value) noexcept {
    (void)value;
    return Status::Ok;
}

} // namespace pbsd::userland::libthr
