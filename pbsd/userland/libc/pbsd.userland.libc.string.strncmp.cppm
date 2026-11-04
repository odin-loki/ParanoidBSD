module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.strncmp;

export import pbsd.core;

/// strncmp from hbsd/src/lib/libc/string/strncmp.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int strncmp_bytes(const char* a, const char* b, std::size_t n) noexcept {
    if (a == nullptr || b == nullptr || n == 0) return 0;
    for (std::size_t i = 0; i < n; ++i) {
        if (a[i] != b[i] || a[i] == '\0') {
            return static_cast<unsigned char>(a[i]) - static_cast<unsigned char>(b[i]);
        }
    }
    return 0;
}

} // namespace pbsd::userland::libc
