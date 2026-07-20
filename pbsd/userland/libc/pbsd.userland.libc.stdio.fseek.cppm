module;

export module pbsd.userland.libc.stdio.fseek;

import pbsd.userland.libc.stdio.file;

/// fseek/ftell/rewind from hbsd/src/lib/libc/stdio/fseek.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int fseek(IoFile& fp, long offset, int whence) noexcept {
    (void)fp;
    (void)offset;
    (void)whence;
    return 0;
}

[[nodiscard]] inline long ftell(const IoFile& fp) noexcept {
    (void)fp;
    return 0;
}

inline void rewind(IoFile& fp) noexcept {
    (void)fp;
}

} // namespace pbsd::userland::libc::stdio
