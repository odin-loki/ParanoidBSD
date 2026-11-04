module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.ether_addr;

export import pbsd.core;

/// ether_addr from hbsd/src/lib/libc/net/ether_addr.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status ether_addr_parse(const char* asc) noexcept { return asc ? Status::Ok : Status::Invalid; }

} // namespace pbsd::userland::libc
