module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.closefrom;

export import pbsd.core;

/// closefrom from hbsd/src/lib/libc/sys/closefrom.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status closefrom_lowfd(int lowfd) noexcept { return lowfd >= 0 ? Status::Ok : Status::Invalid; }

} // namespace pbsd::userland::libc
