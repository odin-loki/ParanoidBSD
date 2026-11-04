module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.timespec_getres;

export import pbsd.core;

/// timespec_getres from hbsd/src/lib/libc/gen/timespec_getres.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int timespec_getres_base(int base) noexcept { return base; }

} // namespace pbsd::userland::libc
