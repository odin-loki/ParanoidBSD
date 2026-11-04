module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.statvfs;

export import pbsd.core;

/// statvfs from hbsd/src/lib/libc/gen/statvfs.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status statvfs_path(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
