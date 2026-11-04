module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.memrchr;

export import pbsd.core;

/// memrchr from hbsd/src/lib/libc/string/memrchr.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline void* memrchr_find(const void* s, int c, std::size_t n) noexcept {
    const auto* p = static_cast<const unsigned char*>(s);
    if (p == nullptr) return nullptr;
    const unsigned char v = static_cast<unsigned char>(c);
    for (std::size_t i = n; i > 0; --i) {
        if (p[i - 1] == v) return const_cast<void*>(static_cast<const void*>(p + i - 1));
    }
    return nullptr;
}

} // namespace pbsd::userland::libc
