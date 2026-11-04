module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.dirname_compat;

export import pbsd.core;

/// dirname_compat from hbsd/src/lib/libc/gen/dirname_compat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status dirname_compat_path(const char* path) noexcept {
    if (path == nullptr) return Status::Invalid;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
