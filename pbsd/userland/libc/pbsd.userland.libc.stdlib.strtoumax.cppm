module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.strtoumax;

export import pbsd.core;

/// strtoumax from hbsd/src/lib/libc/stdlib/strtoumax.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline unsigned long long strtoumax_digits(const char* nptr) noexcept {
    if (nptr == nullptr) return 0ULL;
    return static_cast<unsigned long long>(*nptr - '0');
}

} // namespace pbsd::userland::libc
