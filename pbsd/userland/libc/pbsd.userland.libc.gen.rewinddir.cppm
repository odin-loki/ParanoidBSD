module;

export module pbsd.userland.libc.gen.rewinddir;

export import pbsd.core;

/// rewinddir from hbsd/src/lib/libc/gen/rewinddir.c
export namespace pbsd::userland::libc {

struct DirHandle { long tell{0}; };

inline void rewinddir_handle(DirHandle& d) noexcept { d.tell = 0; }

} // namespace pbsd::userland::libc
