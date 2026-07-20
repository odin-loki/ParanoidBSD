module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.getservent;

export import pbsd.core;

/// getservent from hbsd/src/lib/libc/net/getservent.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool getservent_eof() noexcept { return true; }

} // namespace pbsd::userland::libc
