module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.getdents;

export import pbsd.core;

/// getdents from hbsd/src/lib/libc/sys/getdents.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sys_fd(int fd) noexcept { return fd >= 0 ? Status::Ok : Status::Invalid; }

} // namespace pbsd::userland::libc
