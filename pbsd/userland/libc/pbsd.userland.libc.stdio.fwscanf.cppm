module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.fwscanf;

export import pbsd.core;

/// fwscanf from hbsd/src/lib/libc/stdio/fwscanf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fwscanf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
