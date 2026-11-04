module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.inet_net_ntop;

export import pbsd.core;

/// inet_net_ntop from hbsd/src/lib/libc/inet/inet_net_ntop.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status inet_net_ntop_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
