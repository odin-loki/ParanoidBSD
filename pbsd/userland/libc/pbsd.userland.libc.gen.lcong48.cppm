module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.lcong48;

export import pbsd.core;

/// lcong48 from hbsd/src/lib/libc/gen/lcong48.c
export namespace pbsd::userland::libc {

inline void lcong48_param(unsigned short p[7]) noexcept { (void)p; }

} // namespace pbsd::userland::libc
