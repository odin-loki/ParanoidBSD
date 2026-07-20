module;

export module pbsd.userland.libc.stdio.tmpfile;

import pbsd.userland.libc.stdio.file;

/// tmpfile from hbsd/src/lib/libc/stdio/tmpfile.c (stub FILE*)
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline IoFile* tmpfile() noexcept {
    static IoFile fp{};
    fp.fd = -1;
    return &fp;
}

} // namespace pbsd::userland::libc::stdio
