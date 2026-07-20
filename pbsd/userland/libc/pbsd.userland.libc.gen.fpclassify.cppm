module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.fpclassify;

export import pbsd.core;

/// fpclassify from hbsd/src/lib/libc/gen/fpclassify.c
export namespace pbsd::userland::libc {

enum class FpClass : int { Normal, Zero, Subnormal, Infinite, Nan };

[[nodiscard]] inline FpClass fpclassify_zero(double x) noexcept {
    return x == 0.0 ? FpClass::Zero : FpClass::Normal;
}

} // namespace pbsd::userland::libc
