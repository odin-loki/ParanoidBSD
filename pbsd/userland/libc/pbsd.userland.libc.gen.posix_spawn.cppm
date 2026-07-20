module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.posix_spawn;

export import pbsd.core;

/// posix_spawn from hbsd/src/lib/libc/gen/posix_spawn.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status posix_spawn_path(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
