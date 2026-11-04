module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.accept4;

export import pbsd.core;

/// accept4 from hbsd/src/lib/libc/sys/accept4.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sys_fd(int fd) noexcept { return fd >= 0 ? Status::Ok : Status::Invalid; }

} // namespace pbsd::userland::libc
