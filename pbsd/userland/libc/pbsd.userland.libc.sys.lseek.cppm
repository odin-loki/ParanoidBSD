module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.lseek;

export import pbsd.core;

/// lseek from hbsd/src/lib/libc/sys/lseek.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status lseek_fd(int fd, long off) noexcept { if (fd < 0) return Status::Invalid; (void)off; return Status::Ok; }

} // namespace pbsd::userland::libc
