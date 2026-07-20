module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.inet_ntoa;

export import pbsd.core;

/// inet_ntoa from hbsd/src/lib/libc/inet/inet_ntoa.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status inet_ntoa_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
