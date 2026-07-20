module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.ungetwc;

export import pbsd.core;

/// ungetwc from hbsd/src/lib/libc/stdio/ungetwc.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status ungetwc_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
