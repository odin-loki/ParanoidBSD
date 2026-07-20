module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.strtol;

export import pbsd.core;

/// strtol from hbsd/src/lib/libc/stdlib/strtol.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline long strtol_digits(const char* nptr, char** endptr, int base) noexcept {
    (void)endptr;
    (void)base;
    if (nptr == nullptr) {
        return 0L;
    }
    long acc = 0;
    int sign = 1;
    while (*nptr == ' ') {
        ++nptr;
    }
    if (*nptr == '-') {
        sign = -1;
        ++nptr;
    } else if (*nptr == '+') {
        ++nptr;
    }
    while (*nptr >= '0' && *nptr <= '9') {
        acc = acc * 10 + (*nptr - '0');
        ++nptr;
    }
    return sign * acc;
}

} // namespace pbsd::userland::libc
