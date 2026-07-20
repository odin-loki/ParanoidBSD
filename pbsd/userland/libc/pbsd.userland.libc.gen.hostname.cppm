module;

#include <cstddef>

export module pbsd.userland.libc.gen.hostname;

/// get/set hostname/domainname from hbsd/src/lib/libc/gen/{get,set}*name.c
export namespace pbsd::userland::libc {

inline constexpr std::size_t kHostNameMax = 256;

[[nodiscard]] inline bool copy_host_string(char* dst, std::size_t dstlen, const char* src) noexcept {
    if (dstlen == 0) {
        return false;
    }
    std::size_t i = 0;
    while (src[i] != '\0' && i + 1 < dstlen) {
        dst[i] = src[i];
        ++i;
    }
    dst[i] = '\0';
    return src[i] == '\0';
}

} // namespace pbsd::userland::libc
