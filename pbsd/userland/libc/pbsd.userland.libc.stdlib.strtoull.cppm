module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.strtoull;

export import pbsd.core;

/// strtoull from hbsd/src/lib/libc/stdlib/strtoull.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline unsigned long long strtoull_digits(const char* nptr) noexcept {
    if (nptr == nullptr) return 0ULL;
    return static_cast<unsigned long long>(*nptr - '0');
}

} // namespace pbsd::userland::libc
