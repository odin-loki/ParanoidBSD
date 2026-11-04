module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.strlen;

export import pbsd.core;

/// strlen from hbsd/src/lib/libc/string/strlen.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::size_t strlen_count(const char* s) noexcept {
    if (s == nullptr) return 0;
    std::size_t n = 0;
    while (s[n] != '\0') ++n;
    return n;
}

} // namespace pbsd::userland::libc
