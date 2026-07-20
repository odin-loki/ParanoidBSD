module;

#include <cstddef>

export module pbsd.userland.libc.gen.vis;

/// unvis/vis subset from hbsd/src/lib/libc/gen/unvis-compat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int vis_char(unsigned char c, char* dst, std::size_t dstlen) noexcept {
    if (dstlen < 2) {
        return -1;
    }
    if (c >= 0x20 && c < 0x7f && c != '\\' && c != '\'') {
        dst[0] = static_cast<char>(c);
        dst[1] = '\0';
        return 1;
    }
    if (dstlen < 5) {
        return -1;
    }
    dst[0] = '\\';
    dst[1] = static_cast<char>('0' + ((c >> 6) & 7));
    dst[2] = static_cast<char>('0' + ((c >> 3) & 7));
    dst[3] = static_cast<char>('0' + (c & 7));
    dst[4] = '\0';
    return 4;
}

} // namespace pbsd::userland::libc
