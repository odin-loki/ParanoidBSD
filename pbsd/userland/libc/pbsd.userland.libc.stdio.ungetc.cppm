module;

export module pbsd.userland.libc.stdio.ungetc;

import pbsd.userland.libc.stdio.file;

/// ungetc from hbsd/src/lib/libc/stdio/unget.c (one-char pushback stub)
export namespace pbsd::userland::libc::stdio {

inline int g_unget_char{-1};

[[nodiscard]] inline int ungetc(int c, IoFile& fp) noexcept {
    if (c == -1 || fp.fd < 0) {
        return -1;
    }
    g_unget_char = c;
    return c;
}

} // namespace pbsd::userland::libc::stdio
