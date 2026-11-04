module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.getnameinfo;

export import pbsd.core;

/// getnameinfo from hbsd/src/lib/libc/net/getnameinfo.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getnameinfo_flags(int flags) noexcept { (void)flags; return Status::Ok; }

} // namespace pbsd::userland::libc
