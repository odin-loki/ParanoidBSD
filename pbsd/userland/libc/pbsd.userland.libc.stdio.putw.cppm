module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.putw;

export import pbsd.core;

/// putw from hbsd/src/lib/libc/stdio/putw.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status putw_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
