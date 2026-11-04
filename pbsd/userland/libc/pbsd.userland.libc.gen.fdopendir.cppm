module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.fdopendir;

export import pbsd.core;

/// fdopendir from hbsd/src/lib/libc/gen/fdopendir.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fdopendir_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
