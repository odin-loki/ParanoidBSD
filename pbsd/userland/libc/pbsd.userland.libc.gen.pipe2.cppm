module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.pipe2;

export import pbsd.core;

/// pipe2 from hbsd/src/lib/libc/gen/pipe2.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status pipe2_flags(int flags) noexcept {
    (void)flags;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
