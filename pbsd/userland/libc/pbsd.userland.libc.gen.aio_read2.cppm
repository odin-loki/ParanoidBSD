module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.aio_read2;

export import pbsd.core;

/// aio_read2 from hbsd/src/lib/libc/gen/aio_read2.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status aio_read2_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
