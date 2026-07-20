module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.drand48;

export import pbsd.core;

/// drand48 from hbsd/src/lib/libc/gen/drand48.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline double drand48_unit(unsigned seed) noexcept {
    return static_cast<double>(seed % 10000u) / 10000.0;
}

} // namespace pbsd::userland::libc
