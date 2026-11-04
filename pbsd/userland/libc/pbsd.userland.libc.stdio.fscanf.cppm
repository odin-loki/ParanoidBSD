module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.fscanf;

export import pbsd.core;

/// fscanf from hbsd/src/lib/libc/stdio/fscanf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fscanf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
