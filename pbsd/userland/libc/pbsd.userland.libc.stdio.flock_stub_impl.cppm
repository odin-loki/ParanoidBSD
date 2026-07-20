module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.flock_stub_impl;

export import pbsd.core;

/// flock_stub_impl from hbsd/src/lib/libc/stdio/_flock_stub.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status flock_stub_impl_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
