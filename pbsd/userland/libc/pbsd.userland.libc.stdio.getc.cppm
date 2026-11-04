module;

export module pbsd.userland.libc.stdio.getc;

import pbsd.userland.libc.stdio.file;

/// getc concept from hbsd/src/lib/libc/stdio/getc.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int getc(IoFile* fp) noexcept {
    if (fp == nullptr || fp->fd < 0 || is_eof(*fp)) {
        return -1;
    }
    return 0;
}

[[nodiscard]] inline int getc_unlocked(IoFile* fp) noexcept {
    return getc(fp);
}

} // namespace pbsd::userland::libc::stdio
