module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.thread_init_impl;

export import pbsd.core;

/// thread_init_impl from hbsd/src/lib/libc/gen/_thread_init.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status thread_init_impl_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
