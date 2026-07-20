module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.endusershell;

export import pbsd.core;

/// endusershell from hbsd/src/lib/libc/gen/endusershell.c
export namespace pbsd::userland::libc {

inline void endusershell_close() noexcept {}

} // namespace pbsd::userland::libc
