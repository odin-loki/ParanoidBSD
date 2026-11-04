module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.lockf;

export import pbsd.core;

/// lockf from hbsd/src/lib/libc/sys/lockf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status lockf_fd(int fd, int cmd) noexcept { if (fd < 0) return Status::Invalid; (void)cmd; return Status::Ok; }

} // namespace pbsd::userland::libc
