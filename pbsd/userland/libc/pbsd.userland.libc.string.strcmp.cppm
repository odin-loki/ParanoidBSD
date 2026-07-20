module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.strcmp;

export import pbsd.core;

/// strcmp from hbsd/src/lib/libc/string/strcmp.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int strcmp_bytes(const char* a, const char* b) noexcept {
    if (a == nullptr || b == nullptr) return 0;
    while (*a && *a == *b) { ++a; ++b; }
    return static_cast<unsigned char>(*a) - static_cast<unsigned char>(*b);
}

} // namespace pbsd::userland::libc
