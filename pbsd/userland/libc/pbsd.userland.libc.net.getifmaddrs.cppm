module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.getifmaddrs;

export import pbsd.core;

/// getifmaddrs from hbsd/src/lib/libc/net/getifmaddrs.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getifmaddrs_list(void** ifmap) noexcept { if (!ifmap) return Status::Invalid; *ifmap = nullptr; return Status::Ok; }

} // namespace pbsd::userland::libc
