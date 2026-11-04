module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.vwscanf;

export import pbsd.core;

/// vwscanf from hbsd/src/lib/libc/stdio/vwscanf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status vwscanf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
