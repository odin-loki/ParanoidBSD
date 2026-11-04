module;

export module pbsd.userland.libc.stdio.rewind;

export import pbsd.core;

/// rewind from hbsd/src/lib/libc/stdio/rewind.c
export namespace pbsd::userland::libc {

struct FilePos { long offset{0}; bool error{false}; };

inline void rewind_pos(FilePos& f) noexcept {
    f.offset = 0;
    f.error = false;
}

} // namespace pbsd::userland::libc
