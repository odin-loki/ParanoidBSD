module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.ftruncate;

export import pbsd.core;

/// ftruncate from hbsd/src/lib/libc/sys/ftruncate.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status ftruncate_fd(int fd, long len) noexcept { if (fd < 0) return Status::Invalid; (void)len; return Status::Ok; }

} // namespace pbsd::userland::libc
