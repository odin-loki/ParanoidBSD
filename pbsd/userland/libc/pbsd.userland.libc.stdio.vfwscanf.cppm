module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.vfwscanf;

export import pbsd.core;

/// vfwscanf from hbsd/src/lib/libc/stdio/vfwscanf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status vfwscanf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
