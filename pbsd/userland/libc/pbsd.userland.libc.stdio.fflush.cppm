module;

export module pbsd.userland.libc.stdio.fflush;

import pbsd.userland.libc.stdio.file;

/// fflush from hbsd/src/lib/libc/stdio/fflush.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int fflush(IoFile* fp) noexcept {
    if (fp == nullptr) {
        return 0;
    }
    if (fp->fd < 0) {
        return -1;
    }
    return 0;
}

} // namespace pbsd::userland::libc::stdio
