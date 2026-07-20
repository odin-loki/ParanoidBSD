module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.dlfcn;

export import pbsd.core;

/// dlfcn from hbsd/src/lib/libc/gen/dlfcn.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status dlfcn_path(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
