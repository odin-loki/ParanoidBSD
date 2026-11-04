module;

export module pbsd.userland.libc.stdio.fputc;

export import pbsd.userland.libc.stdio.file;
import pbsd.userland.libc.stdio.putc;

/// fputc from hbsd/src/lib/libc/stdio/fputc.c
export namespace pbsd::userland::libc::stdio {

using pbsd::userland::libc::stdio::IoFile;

[[nodiscard]] inline int fputc(int c, IoFile* fp) noexcept { return putc(c, fp); }

} // namespace pbsd::userland::libc::stdio
