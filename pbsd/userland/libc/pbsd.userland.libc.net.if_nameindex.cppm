module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.if_nameindex;

export import pbsd.core;

/// if_nameindex from hbsd/src/lib/libc/net/if_nameindex.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool if_nameindex_empty() noexcept { return true; }

} // namespace pbsd::userland::libc
