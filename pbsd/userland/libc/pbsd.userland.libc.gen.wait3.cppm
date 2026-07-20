module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.wait3;

export import pbsd.core;

/// wait3 from hbsd/src/lib/libc/gen/wait3.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int wait3_status(int status) noexcept { return status; }

} // namespace pbsd::userland::libc
