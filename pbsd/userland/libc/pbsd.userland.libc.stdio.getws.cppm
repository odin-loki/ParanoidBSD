module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.getws;

export import pbsd.core;

/// getws from hbsd/src/lib/libc/stdio/getws.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getws_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
