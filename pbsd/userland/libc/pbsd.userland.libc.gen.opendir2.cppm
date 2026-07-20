module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.opendir2;

export import pbsd.core;

/// opendir2 from hbsd/src/lib/libc/gen/opendir2.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status opendir2_path(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
