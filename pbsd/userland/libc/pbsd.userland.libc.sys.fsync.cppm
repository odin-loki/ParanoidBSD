module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.fsync;

export import pbsd.core;

/// fsync from hbsd/src/lib/libc/sys/fsync.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sys_fd(int fd) noexcept { return fd >= 0 ? Status::Ok : Status::Invalid; }

} // namespace pbsd::userland::libc
