module;

export module pbsd.userland.libc.gen.pthread_stubs;

export import pbsd.core;
import pbsd.userland.libc.gen.pthread_stubs_impl;

/// pthread_stubs from hbsd/src/lib/libc/gen/_pthread_stubs.c
export namespace pbsd::userland::libc::gen {

[[nodiscard]] inline bool pthread_stubs_active() noexcept {
    return pbsd::userland::libc::pthread_stubs_impl_ok() == Status::Ok;
}

} // namespace pbsd::userland::libc::gen
