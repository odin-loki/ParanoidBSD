module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.putws;

export import pbsd.core;

/// putws from hbsd/src/lib/libc/stdio/putws.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status putws_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
