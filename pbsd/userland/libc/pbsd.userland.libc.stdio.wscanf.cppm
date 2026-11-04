module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.wscanf;

export import pbsd.core;

/// wscanf from hbsd/src/lib/libc/stdio/wscanf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status wscanf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
