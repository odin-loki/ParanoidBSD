module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.inet_cidr_ntop;

export import pbsd.core;

/// inet_cidr_ntop from hbsd/src/lib/libc/inet/inet_cidr_ntop.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status inet_cidr_ntop_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
