module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.strtouq;

export import pbsd.core;

/// strtouq from hbsd/src/lib/libc/stdlib/strtouq.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline unsigned long long strtouq_digits(const char* nptr) noexcept {
    if (nptr == nullptr) return 0ULL;
    return static_cast<unsigned long long>(*nptr - '0');
}

} // namespace pbsd::userland::libc
