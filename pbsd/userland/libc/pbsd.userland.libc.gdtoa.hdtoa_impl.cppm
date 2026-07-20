module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gdtoa.hdtoa_impl;

export import pbsd.core;

/// hdtoa_impl from hbsd/src/lib/libc/gdtoa/_hdtoa.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status hdtoa_impl_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
