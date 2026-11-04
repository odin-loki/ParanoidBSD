module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.wait;

export import pbsd.core;

/// wait from hbsd/src/lib/libc/sys/wait.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int wait_status(int status) noexcept { return status; }

} // namespace pbsd::userland::libc
