module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.resolv.h_errno;

export import pbsd.core;

/// h_errno from hbsd/src/lib/libc/resolv/h_errno.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status h_errno_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
