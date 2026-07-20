module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.fputwc;

export import pbsd.core;

/// fputwc from hbsd/src/lib/libc/stdio/fputwc.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fputwc_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
