module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.scandir;

export import pbsd.core;

/// scandir from hbsd/src/lib/libc/gen/scandir.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status scandir_path(const char* dir) noexcept {
    if (dir == nullptr || dir[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
