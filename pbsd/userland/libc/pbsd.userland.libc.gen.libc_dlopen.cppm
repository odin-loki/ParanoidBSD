module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.libc_dlopen;

export import pbsd.core;

/// libc_dlopen from hbsd/src/lib/libc/gen/libc_dlopen.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status libc_dlopen_path(const char* path) noexcept {
    if (path == nullptr) return Status::Invalid;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
