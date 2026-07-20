module;

export module pbsd.userland.libc.stdio.ferror;

export import pbsd.core;

/// ferror from hbsd/src/lib/libc/stdio/ferror.c
export namespace pbsd::userland::libc {

struct FileFlags { bool eof{false}; bool error{false}; };

[[nodiscard]] inline int ferror_flag(const FileFlags& f) noexcept { return f.error ? 1 : 0; }

} // namespace pbsd::userland::libc
