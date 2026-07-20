module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.getusershell;

export import pbsd.core;

/// getusershell from hbsd/src/lib/libc/gen/getusershell.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline const char* getusershell_default() noexcept { return "/bin/sh"; }

} // namespace pbsd::userland::libc
