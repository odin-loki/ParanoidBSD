module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.erand48;

export import pbsd.core;

/// erand48 from hbsd/src/lib/libc/gen/erand48.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline double erand48_step(unsigned short xsubi[3]) noexcept {
    if (xsubi == nullptr) {
        return 0.0;
    }
    return static_cast<double>(xsubi[2]) / 65536.0;
}

} // namespace pbsd::userland::libc
