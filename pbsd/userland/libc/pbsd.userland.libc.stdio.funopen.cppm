module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.funopen;

export import pbsd.core;

/// funopen from hbsd/src/lib/libc/stdio/funopen.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status funopen_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
