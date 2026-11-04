module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.fdopen;

export import pbsd.core;

/// fdopen from hbsd/src/lib/libc/stdio/fdopen.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fdopen_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
