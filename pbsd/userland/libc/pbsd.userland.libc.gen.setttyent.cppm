module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.setttyent;

export import pbsd.core;

/// setttyent from hbsd/src/lib/libc/gen/setttyent.c
export namespace pbsd::userland::libc {

inline void setttyent_rewind() noexcept {}

} // namespace pbsd::userland::libc
