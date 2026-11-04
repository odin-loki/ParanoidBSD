module;

export module pbsd.userland.libc.stdio.fileno;

import pbsd.userland.libc.stdio.file;

/// fileno from hbsd/src/lib/libc/stdio/fileno.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline int fileno(IoFile* fp) noexcept {
    return fp != nullptr ? fp->fd : -1;
}

} // namespace pbsd::userland::libc::stdio
