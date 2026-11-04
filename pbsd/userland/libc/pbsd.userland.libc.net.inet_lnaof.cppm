module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.inet_lnaof;

export import pbsd.core;

/// inet_lnaof from hbsd/src/lib/libc/inet/inet_lnaof.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status inet_lnaof_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
