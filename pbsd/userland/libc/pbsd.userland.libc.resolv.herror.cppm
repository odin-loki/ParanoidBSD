module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.resolv.herror;

export import pbsd.core;

/// herror from hbsd/src/lib/libc/resolv/herror.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status herror_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
