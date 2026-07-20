module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.basename_compat;

export import pbsd.core;

/// basename_compat from hbsd/src/lib/libc/gen/basename_compat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status basename_compat_path(const char* path) noexcept {
    if (path == nullptr) return Status::Invalid;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
