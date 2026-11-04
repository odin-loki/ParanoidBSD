module;

export module pbsd.userland.libc.stdio.feof;

export import pbsd.core;

/// feof from hbsd/src/lib/libc/stdio/feof.c
export namespace pbsd::userland::libc {

struct FileFlags { bool eof{false}; bool error{false}; };

[[nodiscard]] inline int feof_flag(const FileFlags& f) noexcept { return f.eof ? 1 : 0; }

} // namespace pbsd::userland::libc
