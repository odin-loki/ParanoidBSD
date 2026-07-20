module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.getpwent;

export import pbsd.core;

/// getpwent from hbsd/src/lib/libc/gen/getpwent.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool getpwent_eof() noexcept { return true; }

} // namespace pbsd::userland::libc
