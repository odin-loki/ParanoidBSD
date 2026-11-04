module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.nsap_addr;

export import pbsd.core;

/// nsap_addr from hbsd/src/lib/libc/inet/nsap_addr.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status nsap_addr_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
