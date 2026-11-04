module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.fstab;

export import pbsd.core;

/// fstab from hbsd/src/lib/libc/gen/fstab.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool fstab_comment_line(const char* line) noexcept {
    return line != nullptr && line[0] == '#';
}

} // namespace pbsd::userland::libc
