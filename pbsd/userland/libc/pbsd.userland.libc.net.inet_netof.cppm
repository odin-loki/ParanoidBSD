module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.inet_netof;

export import pbsd.core;

/// inet_netof from hbsd/src/lib/libc/inet/inet_netof.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status inet_netof_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
