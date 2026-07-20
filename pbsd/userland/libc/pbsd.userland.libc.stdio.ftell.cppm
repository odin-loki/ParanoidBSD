module;

export module pbsd.userland.libc.stdio.ftell;

export import pbsd.core;

/// ftell from hbsd/src/lib/libc/stdio/ftell.c
export namespace pbsd::userland::libc {

struct FilePos { long offset{0}; };

[[nodiscard]] inline long ftell_pos(const FilePos& f) noexcept { return f.offset; }

} // namespace pbsd::userland::libc
