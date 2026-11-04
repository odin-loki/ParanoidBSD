module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.strtoimax;

export import pbsd.core;

/// strtoimax from hbsd/src/lib/libc/stdlib/strtoimax.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline long long strtoimax_digits(const char* nptr) noexcept {
    if (nptr == nullptr) return 0LL;
    long long acc = 0;
    int sign = 1;
    while (*nptr == ' ') ++nptr;
    if (*nptr == '-') { sign = -1; ++nptr; } else if (*nptr == '+') ++nptr;
    while (*nptr >= '0' && *nptr <= '9') { acc = acc * 10 + (*nptr - '0'); ++nptr; }
    return sign * acc;
}

} // namespace pbsd::userland::libc
