module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.memchr;

export import pbsd.core;

/// memchr from hbsd/src/lib/libc/string/memchr.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline void* memchr_find(const void* s, int c, std::size_t n) noexcept {
    const auto* p = static_cast<const unsigned char*>(s);
    if (p == nullptr) return nullptr;
    const unsigned char v = static_cast<unsigned char>(c);
    for (std::size_t i = 0; i < n; ++i) {
        if (p[i] == v) return const_cast<void*>(static_cast<const void*>(p + i));
    }
    return nullptr;
}

} // namespace pbsd::userland::libc
