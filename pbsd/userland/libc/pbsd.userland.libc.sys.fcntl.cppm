module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.fcntl;

export import pbsd.core;

/// fcntl from hbsd/src/lib/libc/sys/fcntl.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fcntl_cmd(int fd, int cmd) noexcept { if (fd < 0) return Status::Invalid; (void)cmd; return Status::Ok; }

} // namespace pbsd::userland::libc
