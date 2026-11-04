module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.ttyslot;

export import pbsd.core;

/// ttyslot from hbsd/src/lib/libc/gen/ttyslot.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int ttyslot_fd(int fd) noexcept { return fd >= 0 ? 0 : -1; }

} // namespace pbsd::userland::libc
