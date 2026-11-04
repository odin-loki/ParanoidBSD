module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.fgetws;

export import pbsd.core;

/// fgetws from hbsd/src/lib/libc/stdio/fgetws.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fgetws_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
