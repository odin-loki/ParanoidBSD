module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.termios;

export import pbsd.core;

/// termios from hbsd/src/lib/libc/gen/termios.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status termios_fd(int fd) noexcept { return fd >= 0 ? Status::Ok : Status::Invalid; }

} // namespace pbsd::userland::libc
