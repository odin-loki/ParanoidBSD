module;

#include <cstddef>

export module pbsd.userland.libc.string.secure;

/// timingsafe_bcmp/timingsafe_memcmp from hbsd/src/lib/libc/string/timingsafe_*.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int timingsafe_bcmp(const void* b1, const void* b2, std::size_t n) noexcept {
    const auto* p1 = static_cast<const unsigned char*>(b1);
    const auto* p2 = static_cast<const unsigned char*>(b2);
    int ret = 0;
    for (; n > 0; --n) {
        ret |= *p1++ ^ *p2++;
    }
    return ret != 0;
}

[[nodiscard]] inline int timingsafe_memcmp(const void* b1, const void* b2, std::size_t n) noexcept {
    const auto* p1 = static_cast<const unsigned char*>(b1);
    const auto* p2 = static_cast<const unsigned char*>(b2);
    int ret = 0;
    for (; n > 0; --n) {
        ret |= *p1++ ^ *p2++;
    }
    return ret != 0 ? 1 : 0;
}

} // namespace pbsd::userland::libc
