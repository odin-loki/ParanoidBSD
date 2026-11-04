module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.setusershell;

export import pbsd.core;

/// setusershell from hbsd/src/lib/libc/gen/setusershell.c
export namespace pbsd::userland::libc {

inline void setusershell_rewind() noexcept {}

} // namespace pbsd::userland::libc
