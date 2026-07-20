module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.getdelim;

export import pbsd.core;

/// getdelim from hbsd/src/lib/libc/stdio/getdelim.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getdelim_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
