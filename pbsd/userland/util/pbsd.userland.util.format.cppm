module;

#include <cstdio>

export module pbsd.userland.util.format;

/// hexdump(3) from hbsd/src/lib/libutil/hexdump.c
export namespace pbsd::userland::util {

inline constexpr int HD_COLUMN_MASK = 0xff;
inline constexpr int HD_DELIM_MASK = 0xff00;
inline constexpr int HD_OMIT_COUNT  = (1 << 16);
inline constexpr int HD_OMIT_HEX    = (1 << 17);
inline constexpr int HD_OMIT_CHARS  = (1 << 18);

inline void hexdump(const void* ptr, int length, const char* hdr,
                    int flags) noexcept {
    int i, j, k;
    int cols;
    const unsigned char* cp;
    char delim;

    if ((flags & HD_DELIM_MASK) != 0) {
        delim = static_cast<char>((flags & HD_DELIM_MASK) >> 8);
    } else {
        delim = ' ';
    }

    if ((flags & HD_COLUMN_MASK) != 0) {
        cols = flags & HD_COLUMN_MASK;
    } else {
        cols = 16;
    }

    cp = static_cast<const unsigned char*>(ptr);
    for (i = 0; i < length; i += cols) {
        if (hdr != nullptr) {
            std::printf("%s", hdr);
        }

        if ((flags & HD_OMIT_COUNT) == 0) {
            std::printf("%04x  ", i);
        }

        if ((flags & HD_OMIT_HEX) == 0) {
            for (j = 0; j < cols; ++j) {
                k = i + j;
                if (k < length) {
                    std::printf("%c%02x", delim, cp[k]);
                } else {
                    std::printf("   ");
                }
            }
        }

        if ((flags & HD_OMIT_CHARS) == 0) {
            std::printf("  |");
            for (j = 0; j < cols; ++j) {
                k = i + j;
                if (k >= length) {
                    std::printf(" ");
                } else if (cp[k] >= ' ' && cp[k] <= '~') {
                    std::printf("%c", cp[k]);
                } else {
                    std::printf(".");
                }
            }
            std::printf("|");
        }
        std::printf("\n");
    }
}

} // namespace pbsd::userland::util
