module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gdtoa.hldtoa_impl;

export import pbsd.core;

/// hldtoa_impl from hbsd/src/lib/libc/gdtoa/_hldtoa.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status hldtoa_impl_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
