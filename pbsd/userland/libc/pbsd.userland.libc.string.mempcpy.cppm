module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.mempcpy;

export import pbsd.core;

/// mempcpy from hbsd/src/lib/libc/string/mempcpy.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline void* mempcpy_copy(void* dst, const void* src, std::size_t n) noexcept {
    auto* d = static_cast<unsigned char*>(dst);
    const auto* s = static_cast<const unsigned char*>(src);
    if (d == nullptr || s == nullptr) return dst;
    for (std::size_t i = 0; i < n; ++i) d[i] = s[i];
    return d + n;
}

} // namespace pbsd::userland::libc
