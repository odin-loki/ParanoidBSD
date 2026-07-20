module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.wait6;

export import pbsd.core;

/// wait6 from hbsd/src/lib/libc/sys/wait6.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int wait6_status(int status) noexcept { return status; }

} // namespace pbsd::userland::libc
