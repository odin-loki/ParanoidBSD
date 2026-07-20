module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.fgetwln;

export import pbsd.core;

/// fgetwln from hbsd/src/lib/libc/stdio/fgetwln.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fgetwln_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
