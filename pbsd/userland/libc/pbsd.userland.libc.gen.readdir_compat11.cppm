module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.readdir_compat11;

export import pbsd.core;

/// readdir_compat11 from hbsd/src/lib/libc/gen/readdir_compat11.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status readdir_compat11_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
