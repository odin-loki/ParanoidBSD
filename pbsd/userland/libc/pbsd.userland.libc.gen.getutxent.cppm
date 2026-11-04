module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.getutxent;

export import pbsd.core;

/// getutxent from hbsd/src/lib/libc/gen/getutxent.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool getutxent_eof() noexcept { return true; }

} // namespace pbsd::userland::libc
