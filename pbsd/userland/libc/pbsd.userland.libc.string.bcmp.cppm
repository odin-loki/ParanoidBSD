module;

#include <cstddef>

export module pbsd.userland.libc.string.bcmp;

/// bcmp from hbsd/src/lib/libc/string/bcmp.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int bcmp(const void* b1, const void* b2, std::size_t length) noexcept {
    if (length == 0) {
        return 0;
    }
    const auto* p1 = static_cast<const unsigned char*>(b1);
    const auto* p2 = static_cast<const unsigned char*>(b2);
    do {
        if (*p1++ != *p2++) {
            return 1;
        }
    } while (--length != 0);
    return 0;
}

} // namespace pbsd::userland::libc
