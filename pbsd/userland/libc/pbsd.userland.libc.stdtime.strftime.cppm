module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdtime.strftime;

export import pbsd.core;

/// strftime from hbsd/src/lib/libc/stdtime/strftime.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status strftime_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
