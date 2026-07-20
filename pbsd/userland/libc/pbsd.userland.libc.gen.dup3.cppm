module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.dup3;

export import pbsd.core;

/// dup3 from hbsd/src/lib/libc/gen/dup3.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status dup3_validate(int oldfd, int newfd, int flags) noexcept {
    if (oldfd < 0 || newfd < 0) {
        return Status::Invalid;
    }
    (void)flags;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
