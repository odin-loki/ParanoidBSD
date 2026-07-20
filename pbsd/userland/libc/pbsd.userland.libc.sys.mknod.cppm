module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.mknod;

export import pbsd.core;

/// mknod from hbsd/src/lib/libc/sys/mknod.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status mknod_path(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
