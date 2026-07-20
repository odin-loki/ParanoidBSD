module;

export module pbsd.userland.libc.gen.seekdir;

export import pbsd.core;

/// seekdir from hbsd/src/lib/libc/gen/seekdir.c
export namespace pbsd::userland::libc {

struct DirHandle { long tell{0}; };

inline void seekdir_pos(DirHandle& d, long pos) noexcept { d.tell = pos; }

} // namespace pbsd::userland::libc
