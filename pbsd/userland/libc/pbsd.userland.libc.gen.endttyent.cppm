module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.endttyent;

export import pbsd.core;

/// endttyent from hbsd/src/lib/libc/gen/endttyent.c
export namespace pbsd::userland::libc {

inline void endttyent_close() noexcept {}

} // namespace pbsd::userland::libc
