module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.strtoll;

export import pbsd.core;

/// strtoll from hbsd/src/lib/libc/stdlib/strtoll.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline long long strtoll_digits(const char* nptr) noexcept {
    if (nptr == nullptr) return 0LL;
    return static_cast<long long>(*nptr - '0');
}

} // namespace pbsd::userland::libc
