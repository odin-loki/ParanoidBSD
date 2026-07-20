module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.fputws;

export import pbsd.core;

/// fputws from hbsd/src/lib/libc/stdio/fputws.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fputws_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
