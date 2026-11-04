module;

#include <cstddef>

export module pbsd.userland.libc.string.memset_explicit;

/// memset_explicit from hbsd/src/lib/libc/string/memset_explicit.c
export namespace pbsd::userland::libc {

inline void memset_explicit(void* dst, int c, std::size_t len) noexcept {
    volatile unsigned char* p = static_cast<volatile unsigned char*>(dst);
    while (len-- != 0) {
        *p++ = static_cast<unsigned char>(c);
    }
}

} // namespace pbsd::userland::libc
