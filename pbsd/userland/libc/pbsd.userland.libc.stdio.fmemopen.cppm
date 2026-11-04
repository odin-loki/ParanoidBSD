module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.fmemopen;

export import pbsd.core;

/// fmemopen from hbsd/src/lib/libc/stdio/fmemopen.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fmemopen_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
