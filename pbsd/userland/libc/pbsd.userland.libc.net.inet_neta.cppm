module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.inet_neta;

export import pbsd.core;

/// inet_neta from hbsd/src/lib/libc/inet/inet_neta.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status inet_neta_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
