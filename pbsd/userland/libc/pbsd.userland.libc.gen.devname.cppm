module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.devname;

export import pbsd.core;

/// devname from hbsd/src/lib/libc/gen/devname.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status devname_validate(std::uint64_t dev, unsigned mode) noexcept {
    (void)dev;
    (void)mode;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
