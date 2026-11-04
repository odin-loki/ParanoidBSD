module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.putwc;

export import pbsd.core;

/// putwc from hbsd/src/lib/libc/stdio/putwc.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status putwc_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
