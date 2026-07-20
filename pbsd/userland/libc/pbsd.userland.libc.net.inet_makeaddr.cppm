module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.inet_makeaddr;

export import pbsd.core;

/// inet_makeaddr from hbsd/src/lib/libc/inet/inet_makeaddr.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status inet_makeaddr_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
