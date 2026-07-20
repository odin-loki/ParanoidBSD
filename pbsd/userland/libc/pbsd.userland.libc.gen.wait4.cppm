module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.wait4;

export import pbsd.core;

/// wait4 from hbsd/src/lib/libc/gen/wait4.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int wait4_status(int status) noexcept { return status; }

} // namespace pbsd::userland::libc
