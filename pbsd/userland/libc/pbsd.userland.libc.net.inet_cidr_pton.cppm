module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.inet_cidr_pton;

export import pbsd.core;

/// inet_cidr_pton from hbsd/src/lib/libc/inet/inet_cidr_pton.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status inet_cidr_pton_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
