module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.openat;

export import pbsd.core;

/// openat from hbsd/src/lib/libc/sys/openat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status openat_dirfd(int dirfd) noexcept { return dirfd >= 0 ? Status::Ok : Status::Invalid; }

} // namespace pbsd::userland::libc
