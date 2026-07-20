module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.scandir_b;

export import pbsd.core;

/// scandir_b from hbsd/src/lib/libc/gen/scandir_b.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status scandir_b_dir(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
