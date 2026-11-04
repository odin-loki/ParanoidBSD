module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.statfs;

export import pbsd.core;

/// statfs from hbsd/src/lib/libc/gen/statfs.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status statfs_path(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
