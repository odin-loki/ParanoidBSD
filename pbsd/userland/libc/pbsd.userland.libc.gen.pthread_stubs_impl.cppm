module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.pthread_stubs_impl;

export import pbsd.core;

/// pthread_stubs_impl from hbsd/src/lib/libc/gen/_pthread_stubs.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status pthread_stubs_impl_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
