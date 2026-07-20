module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.creat;

export import pbsd.core;

/// creat from hbsd/src/lib/libc/sys/creat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status creat_path(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
