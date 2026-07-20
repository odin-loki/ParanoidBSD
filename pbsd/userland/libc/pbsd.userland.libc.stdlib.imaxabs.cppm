module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.imaxabs;

export import pbsd.core;

/// imaxabs from hbsd/src/lib/libc/stdlib/imaxabs.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline long long imaxabs_val(long long j) noexcept { return j < 0 ? -j : j; }

} // namespace pbsd::userland::libc
