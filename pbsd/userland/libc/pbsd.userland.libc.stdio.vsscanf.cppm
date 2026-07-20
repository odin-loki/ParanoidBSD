module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.vsscanf;

export import pbsd.core;

/// vsscanf from hbsd/src/lib/libc/stdio/vsscanf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status vsscanf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
