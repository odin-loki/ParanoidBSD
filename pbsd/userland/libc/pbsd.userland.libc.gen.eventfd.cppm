module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.eventfd;

export import pbsd.core;

/// eventfd from hbsd/src/lib/libc/gen/eventfd.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status eventfd_initval(unsigned initval) noexcept {
    (void)initval;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
