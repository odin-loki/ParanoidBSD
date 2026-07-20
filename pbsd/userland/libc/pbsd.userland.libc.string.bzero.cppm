module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.bzero;

export import pbsd.core;

/// bzero from hbsd/src/lib/libc/string/bzero.c
export namespace pbsd::userland::libc {

inline void bzero_bytes(void* s, std::size_t n) noexcept {
    auto* p = static_cast<unsigned char*>(s);
    if (p == nullptr) return;
    for (std::size_t i = 0; i < n; ++i) p[i] = 0;
}

} // namespace pbsd::userland::libc
