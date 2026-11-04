module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.getifaddrs;

export import pbsd.core;

/// getifaddrs from hbsd/src/lib/libc/net/getifaddrs.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getifaddrs_list(void** ifap) noexcept { if (!ifap) return Status::Invalid; *ifap = nullptr; return Status::Ok; }

} // namespace pbsd::userland::libc
