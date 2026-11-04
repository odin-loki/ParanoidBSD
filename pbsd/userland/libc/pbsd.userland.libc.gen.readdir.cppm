module;
#include <cstddef>

export module pbsd.userland.libc.gen.readdir;

export import pbsd.core;

/// readdir from hbsd/src/lib/libc/gen/readdir.c
export namespace pbsd::userland::libc {

struct Dirent { char d_name[256]{}; std::size_t d_namlen{0}; };

[[nodiscard]] inline bool readdir_end(const Dirent* ent) noexcept {
    return ent == nullptr || ent->d_name[0] == '\0';
}

} // namespace pbsd::userland::libc
