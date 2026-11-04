module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.getprotoent;

export import pbsd.core;

/// getprotoent from hbsd/src/lib/libc/net/getprotoent.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool getprotoent_eof() noexcept { return true; }

} // namespace pbsd::userland::libc
