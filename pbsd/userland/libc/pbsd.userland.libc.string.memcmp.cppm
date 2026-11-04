module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.memcmp;

export import pbsd.core;

/// memcmp from hbsd/src/lib/libc/string/memcmp.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int memcmp_bytes(const void* a, const void* b, std::size_t n) noexcept {
    const auto* p = static_cast<const unsigned char*>(a);
    const auto* q = static_cast<const unsigned char*>(b);
    if (p == nullptr || q == nullptr) return 0;
    for (std::size_t i = 0; i < n; ++i) {
        if (p[i] != q[i]) return static_cast<int>(p[i]) - static_cast<int>(q[i]);
    }
    return 0;
}

} // namespace pbsd::userland::libc
