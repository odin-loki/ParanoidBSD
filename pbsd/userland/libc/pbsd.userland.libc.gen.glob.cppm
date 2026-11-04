module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.glob;

export import pbsd.core;

/// glob from hbsd/src/lib/libc/gen/glob.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status glob_pattern(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
