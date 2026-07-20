module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.memmove;

export import pbsd.core;

/// memmove from hbsd/src/lib/libc/string/memmove.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline void* memmove_copy(void* dst, const void* src, std::size_t n) noexcept {
    auto* d = static_cast<unsigned char*>(dst);
    const auto* s = static_cast<const unsigned char*>(src);
    if (d == nullptr || s == nullptr) return dst;
    if (d < s) {
        for (std::size_t i = 0; i < n; ++i) d[i] = s[i];
    } else if (d > s) {
        for (std::size_t i = n; i > 0; --i) d[i - 1] = s[i - 1];
    }
    return dst;
}

} // namespace pbsd::userland::libc
