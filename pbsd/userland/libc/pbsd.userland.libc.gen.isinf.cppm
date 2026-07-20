module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.isinf;

export import pbsd.core;

/// isinf from hbsd/src/lib/libc/gen/isinf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool isinf_val(double x) noexcept { return x != x ? false : x == x * 2.0 && x != 0.0; }

} // namespace pbsd::userland::libc
