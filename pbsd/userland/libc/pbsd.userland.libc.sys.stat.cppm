module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.stat;

export import pbsd.core;

/// stat from hbsd/src/lib/libc/sys/stat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status stat_path(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
