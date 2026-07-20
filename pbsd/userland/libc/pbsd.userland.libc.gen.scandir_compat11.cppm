module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.scandir_compat11;

export import pbsd.core;

/// scandir_compat11 from hbsd/src/lib/libc/gen/scandir_compat11.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status scandir_compat11_dir(const char* dir) noexcept {
    if (dir == nullptr) return Status::Invalid;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
