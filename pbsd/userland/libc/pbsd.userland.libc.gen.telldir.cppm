module;

export module pbsd.userland.libc.gen.telldir;

export import pbsd.core;

/// telldir from hbsd/src/lib/libc/gen/telldir.c
export namespace pbsd::userland::libc {

struct DirHandle { long tell{0}; };

[[nodiscard]] inline long telldir_pos(const DirHandle& d) noexcept { return d.tell; }

} // namespace pbsd::userland::libc
