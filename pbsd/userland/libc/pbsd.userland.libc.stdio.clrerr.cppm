module;

export module pbsd.userland.libc.stdio.clrerr;

export import pbsd.core;

/// clearerr from hbsd/src/lib/libc/stdio/clrerr.c
export namespace pbsd::userland::libc {

struct FileFlags { bool eof{false}; bool error{false}; };

inline void clearerr_flags(FileFlags& f) noexcept {
    f.eof = false;
    f.error = false;
}

} // namespace pbsd::userland::libc
