module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.isnan;

export import pbsd.core;

/// isnan from hbsd/src/lib/libc/gen/isnan.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool isnan_val(double x) noexcept { return x != x; }

} // namespace pbsd::userland::libc
