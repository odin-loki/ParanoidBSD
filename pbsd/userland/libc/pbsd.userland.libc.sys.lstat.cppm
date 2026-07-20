module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.lstat;

export import pbsd.core;

/// lstat from hbsd/src/lib/libc/sys/lstat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status lstat_path(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
