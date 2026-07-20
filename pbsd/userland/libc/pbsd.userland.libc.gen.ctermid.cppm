module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.ctermid;

export import pbsd.core;

/// ctermid from hbsd/src/lib/libc/gen/ctermid.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline const char* ctermid_default() noexcept { return "/dev/tty"; }

} // namespace pbsd::userland::libc
