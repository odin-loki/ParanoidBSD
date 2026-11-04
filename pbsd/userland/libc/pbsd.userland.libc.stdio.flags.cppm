module;

export module pbsd.userland.libc.stdio.flags;

import pbsd.userland.libc.stdio.file;

/// feof/ferror/clearerr from hbsd/src/lib/libc/stdio/{feof,ferror,clrerr}.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int feof(IoFile* fp) noexcept {
    return fp != nullptr && is_eof(*fp);
}

[[nodiscard]] inline int ferror(IoFile* fp) noexcept {
    return fp != nullptr && is_err(*fp);
}

inline void clearerr(IoFile* fp) noexcept {
    if (fp != nullptr) {
        clear_flags(*fp);
    }
}

} // namespace pbsd::userland::libc::stdio
