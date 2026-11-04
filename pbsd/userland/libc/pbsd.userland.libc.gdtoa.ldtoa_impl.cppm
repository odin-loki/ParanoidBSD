module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gdtoa.ldtoa_impl;

export import pbsd.core;

/// ldtoa_impl from hbsd/src/lib/libc/gdtoa/_ldtoa.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status ldtoa_impl_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
