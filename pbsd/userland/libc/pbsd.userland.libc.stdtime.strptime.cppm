module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdtime.strptime;

export import pbsd.core;

/// strptime from hbsd/src/lib/libc/stdtime/strptime.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status strptime_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
