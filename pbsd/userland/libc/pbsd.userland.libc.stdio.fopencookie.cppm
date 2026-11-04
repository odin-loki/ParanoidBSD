module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.fopencookie;

export import pbsd.core;

/// fopencookie from hbsd/src/lib/libc/stdio/fopencookie.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fopencookie_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
