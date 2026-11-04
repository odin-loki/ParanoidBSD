module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.inet_net_pton;

export import pbsd.core;

/// inet_net_pton from hbsd/src/lib/libc/inet/inet_net_pton.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status inet_net_pton_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
