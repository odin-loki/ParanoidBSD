module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.timespec_get;

export import pbsd.core;

/// timespec_get from hbsd/src/lib/libc/gen/timespec_get.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int timespec_get_base(int base) noexcept { return base; }

} // namespace pbsd::userland::libc
