module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.getttyent;

export import pbsd.core;

/// getttyent from hbsd/src/lib/libc/gen/getttyent.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool getttyent_eof() noexcept { return false; }

} // namespace pbsd::userland::libc
