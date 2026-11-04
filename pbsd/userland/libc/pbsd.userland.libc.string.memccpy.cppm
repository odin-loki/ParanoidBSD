module;

#include <cstddef>

export module pbsd.userland.libc.string.memccpy;

/// memccpy from hbsd/src/lib/libc/string/memccpy.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline void* memccpy(void* dst, const void* src, int c, std::size_t n) noexcept {
    auto* d = static_cast<unsigned char*>(dst);
    const auto* s = static_cast<const unsigned char*>(src);
    const auto ch = static_cast<unsigned char>(c);

    if (n != 0) {
        do {
            if ((*d++ = *s++) == ch) {
                return d;
            }
        } while (--n != 0);
    }
    return nullptr;
}

} // namespace pbsd::userland::libc
