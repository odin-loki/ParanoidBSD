module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.memset;

export import pbsd.core;

/// memset from hbsd/src/lib/libc/string/memset.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline void* memset_fill(void* dst, int c, std::size_t n) noexcept {
    auto* d = static_cast<unsigned char*>(dst);
    if (d == nullptr) return dst;
    const unsigned char v = static_cast<unsigned char>(c);
    for (std::size_t i = 0; i < n; ++i) d[i] = v;
    return dst;
}

} // namespace pbsd::userland::libc
