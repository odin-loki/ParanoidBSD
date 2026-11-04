module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.getgrent;

export import pbsd.core;

/// getgrent from hbsd/src/lib/libc/gen/getgrent.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool getgrent_eof() noexcept { return true; }

} // namespace pbsd::userland::libc
