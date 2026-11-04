module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.strtold;

export import pbsd.core;

/// strtold from hbsd/src/lib/libc/stdlib/strtold.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline long double strtold_digits(const char* nptr) noexcept {
    if (nptr == nullptr) return 0.0L;
    return static_cast<long double>(*nptr - '0');
}

} // namespace pbsd::userland::libc
