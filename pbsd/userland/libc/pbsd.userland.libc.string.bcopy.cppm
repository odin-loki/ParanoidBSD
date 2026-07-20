module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.bcopy;

export import pbsd.core;

/// bcopy from hbsd/src/lib/libc/string/bcopy.c
export namespace pbsd::userland::libc {

inline void bcopy_bytes(const void* src, void* dst, std::size_t n) noexcept {
    auto* d = static_cast<unsigned char*>(dst);
    const auto* s = static_cast<const unsigned char*>(src);
    if (d == nullptr || s == nullptr) return;
    if (d < s) {
        for (std::size_t i = 0; i < n; ++i) d[i] = s[i];
    } else if (d > s) {
        for (std::size_t i = n; i > 0; --i) d[i - 1] = s[i - 1];
    }
}

} // namespace pbsd::userland::libc
