module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.pipe;

export import pbsd.core;

/// pipe from hbsd/src/lib/libc/sys/pipe.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status pipe_fds(int fds[2]) noexcept {
    if (fds == nullptr) {
        return Status::Invalid;
    }
    fds[0] = -1;
    fds[1] = -1;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
