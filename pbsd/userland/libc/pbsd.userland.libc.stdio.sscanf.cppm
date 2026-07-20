module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.sscanf;

export import pbsd.core;

/// sscanf from hbsd/src/lib/libc/stdio/sscanf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sscanf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
