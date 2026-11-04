module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.getnetgrent;

export import pbsd.core;

/// getnetgrent from hbsd/src/lib/libc/gen/getnetgrent.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool getnetgrent_eof() noexcept { return true; }

} // namespace pbsd::userland::libc
