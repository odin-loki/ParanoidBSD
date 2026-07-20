module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.strtoul;

export import pbsd.core;

/// strtoul from hbsd/src/lib/libc/stdlib/strtoul.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline unsigned long strtoul_digits(const char* nptr, char** endptr, int base) noexcept {
    (void)endptr;
    (void)base;
    if (nptr == nullptr) {
        return 0UL;
    }
    unsigned long acc = 0;
    while (*nptr == ' ') {
        ++nptr;
    }
    if (*nptr == '+') {
        ++nptr;
    }
    while (*nptr >= '0' && *nptr <= '9') {
        acc = acc * 10 + static_cast<unsigned long>(*nptr - '0');
        ++nptr;
    }
    return acc;
}

} // namespace pbsd::userland::libc
