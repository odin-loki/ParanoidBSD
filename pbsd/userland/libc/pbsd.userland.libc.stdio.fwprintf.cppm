module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.fwprintf;

export import pbsd.core;

/// fwprintf from hbsd/src/lib/libc/stdio/fwprintf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fwprintf_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
