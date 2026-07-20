module;

export module pbsd.userland.libc.gen.thread_init;

export import pbsd.core;
import pbsd.userland.libc.gen.thread_init_impl;

/// thread_init from hbsd/src/lib/libc/gen/_thread_init.c
export namespace pbsd::userland::libc::gen {

[[nodiscard]] inline bool thread_init_ready() noexcept {
    return pbsd::userland::libc::thread_init_impl_ok() == Status::Ok;
}

} // namespace pbsd::userland::libc::gen
