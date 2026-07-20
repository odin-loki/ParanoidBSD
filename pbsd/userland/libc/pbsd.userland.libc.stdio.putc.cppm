module;

export module pbsd.userland.libc.stdio.putc;

import pbsd.userland.libc.stdio.file;

/// putc concept from hbsd/src/lib/libc/stdio/putc.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int putc(int c, IoFile* fp) noexcept {
    if (fp == nullptr || fp->fd < 0) {
        return -1;
    }
    return c;
}

[[nodiscard]] inline int putc_unlocked(int c, IoFile* fp) noexcept {
    return putc(c, fp);
}

} // namespace pbsd::userland::libc::stdio
