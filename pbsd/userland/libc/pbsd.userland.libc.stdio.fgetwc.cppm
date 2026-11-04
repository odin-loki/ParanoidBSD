module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.fgetwc;

export import pbsd.core;

/// fgetwc from hbsd/src/lib/libc/stdio/fgetwc.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fgetwc_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
