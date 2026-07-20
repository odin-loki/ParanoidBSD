module;

export module pbsd.userland.libc.stdio.fgetc;

export import pbsd.userland.libc.stdio.file;
import pbsd.userland.libc.stdio.getc;

/// fgetc from hbsd/src/lib/libc/stdio/fgetc.c
export namespace pbsd::userland::libc::stdio {

using pbsd::userland::libc::stdio::IoFile;

[[nodiscard]] inline int fgetc(IoFile* fp) noexcept { return getc(fp); }

} // namespace pbsd::userland::libc::stdio
