module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.inet_network;

export import pbsd.core;

/// inet_network from hbsd/src/lib/libc/inet/inet_network.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status inet_network_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
