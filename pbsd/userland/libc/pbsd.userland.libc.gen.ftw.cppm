module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.ftw;

export import pbsd.core;

/// ftw from hbsd/src/lib/libc/gen/ftw.c
export namespace pbsd::userland::libc {

enum class FtwKind : int { File = 0, Dir, DirR, Slnk, SlnkNo, Unknown };

[[nodiscard]] inline bool ftw_skip(FtwKind kind) noexcept {
    return kind == FtwKind::Unknown;
}

} // namespace pbsd::userland::libc
